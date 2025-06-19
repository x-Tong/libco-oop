/**
 * @file context.cpp
 * @brief 协程上下文管理实现
 * @author libco-oop
 * @version 1.0
 * 
 * 实现Context类的完整功能，提供现代C++风格的协程上下文管理接口。
 */

#include "libco_oop/context.h"
#include <cstring>
#include <cassert>
#include <exception>

namespace libco_oop {

// 声明外部汇编函数
extern "C" {
    void* libco_oop_get_stack_pointer() noexcept;
    bool libco_oop_is_stack_aligned(void* sp) noexcept;
    void* libco_oop_align_stack_pointer(void* sp) noexcept;
}

//============================================================================
// Context 类实现
//============================================================================

Context::Context(const ContextConfig& config)
    : config_(config)
    , is_valid_(false)
    , switch_count_(0)
{
    initialize_registers();
}

Context::~Context() noexcept
{
    // 上下文对象销毁时不需要特殊清理
    // RegisterState 是 POD 类型，会自动清理
}

Context::Context(Context&& other) noexcept
    : registers_(other.registers_)
    , config_(other.config_)
    , is_valid_(other.is_valid_)
    , switch_count_(other.switch_count_)
{
    // 移动后，原对象失效
    other.is_valid_ = false;
    other.switch_count_ = 0;
}

Context& Context::operator=(Context&& other) noexcept
{
    if (this != &other) {
        registers_ = other.registers_;
        config_ = other.config_;
        is_valid_ = other.is_valid_;
        switch_count_ = other.switch_count_;
        
        // 移动后，原对象失效
        other.is_valid_ = false;
        other.switch_count_ = 0;
    }
    return *this;
}

bool Context::save() noexcept
{
    try {
        // 调用底层汇编函数保存当前上下文
        int result = libco_oop_context_save(&registers_, config_.save_fpu);
        
        if (result == 0) {
            // 第一次调用，保存成功
            is_valid_ = true;
            return true;
        } else {
            // 从 restore 返回，上下文已经切换
            ++switch_count_;
            return true;
        }
    } catch (...) {
        // 捕获任何异常，确保 noexcept
        is_valid_ = false;
        return false;
    }
}

[[noreturn]] void Context::restore() noexcept
{
    // 验证上下文有效性
    if (!is_valid_ || !validate_state()) {
        // 如果上下文无效，终止程序（因为函数标记为 noreturn）
        std::terminate();
    }
    
    // 调用底层汇编函数恢复上下文
    // 注意：此函数不会返回
    libco_oop_context_restore(&registers_, config_.save_fpu);
}

bool Context::swap(Context& other) noexcept
{
    try {
        // 验证两个上下文都有效
        if (!this->validate_state() || !other.validate_state()) {
            return false;
        }
        
        // 确保两个上下文使用相同的配置（FPU保存策略）
        bool save_fpu = config_.save_fpu && other.config_.save_fpu;
        
        // 调用底层汇编函数进行原子性上下文切换
        libco_oop_context_swap(&this->registers_, &other.registers_, save_fpu);
        
        // 执行到这里说明切换成功返回
        ++this->switch_count_;
        this->is_valid_ = true;
        
        return true;
    } catch (...) {
        // 捕获任何异常，确保 noexcept
        return false;
    }
}

bool Context::is_valid() const noexcept
{
    return is_valid_ && validate_state();
}

void Context::reset() noexcept
{
    // 清零所有寄存器状态
    std::memset(&registers_, 0, sizeof(registers_));
    is_valid_ = false;
    switch_count_ = 0;
    initialize_registers();
}

void* Context::get_stack_pointer() const noexcept
{
    return registers_.rsp;
}

bool Context::set_stack_pointer(void* sp) noexcept
{
    if (sp == nullptr) {
        return false;
    }
    
    // 检查栈指针对齐
    if (!context_utils::is_stack_aligned(sp)) {
        sp = context_utils::align_stack_pointer(sp);
    }
    
    registers_.rsp = sp;
    // 如果之前无效，设置栈指针后可能变为有效
    if (!is_valid_) {
        is_valid_ = (registers_.rip != nullptr);
    }
    
    return true;
}

void* Context::get_instruction_pointer() const noexcept
{
    return registers_.rip;
}

bool Context::set_instruction_pointer(void* ip) noexcept
{
    if (ip == nullptr) {
        return false;
    }
    
    registers_.rip = ip;
    // 如果之前无效，设置指令指针后可能变为有效
    if (!is_valid_) {
        is_valid_ = (registers_.rsp != nullptr);
    }
    
    return true;
}

const ContextConfig& Context::get_config() const noexcept
{
    return config_;
}

size_t Context::get_switch_count() const noexcept
{
    return switch_count_;
}

bool Context::validate_state() const noexcept
{
    // 基本有效性检查
    if (!is_valid_) {
        return false;
    }
    
    // 检查关键寄存器是否合理
    // 栈指针不能为空且应该对齐
    if (registers_.rsp == nullptr) {
        return false;
    }
    
    // 检查栈指针对齐
    if (!context_utils::is_stack_aligned(registers_.rsp)) {
        return false;
    }
    
    // 指令指针不能为空 (除非是刚初始化的上下文)
    if (registers_.rip == nullptr && switch_count_ > 0) {
        return false;
    }
    
    return true;
}

void Context::initialize_registers() noexcept
{
    // 清零所有寄存器
    std::memset(&registers_, 0, sizeof(registers_));
    
    // 根据配置初始化FPU/SSE状态
    if (config_.save_fpu) {
        // 初始化为标准值
        registers_.fpucw = 0x037F;  // 标准FPU控制字
        registers_.mxcsr = 0x1F80;  // 标准SSE控制字
    }
}

//============================================================================
// 全局上下文工具函数实现
//============================================================================

namespace context_utils {

void* get_current_stack_pointer() noexcept
{
    return libco_oop_get_stack_pointer();
}

bool is_stack_aligned(void* sp) noexcept
{
    if (sp == nullptr) {
        return false;
    }
    return libco_oop_is_stack_aligned(sp);
}

void* align_stack_pointer(void* sp) noexcept
{
    if (sp == nullptr) {
        return nullptr;
    }
    return libco_oop_align_stack_pointer(sp);
}

} // namespace context_utils

} // namespace libco_oop 