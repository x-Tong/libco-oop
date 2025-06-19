/**
 * @file context.h
 * @brief 协程上下文管理系统
 * @author libco-oop
 * @version 1.0
 * 
 * 实现协程的核心上下文切换机制，这是协程库的基础。
 * 结合libco的稳定性和libaco的高性能，设计现代C++风格的上下文管理接口。
 */

#ifndef LIBCO_OOP_CONTEXT_H
#define LIBCO_OOP_CONTEXT_H

#include <cstdint>
#include <cstddef>

namespace libco_oop {

/**
 * @brief 上下文保存模式
 */
enum class ContextMode {
    MINIMAL,    ///< 精简模式 - 只保存必要寄存器 (类似libaco)
    COMPLETE    ///< 完整模式 - 保存所有寄存器 (类似libco)
};

/**
 * @brief 上下文配置选项
 */
struct ContextConfig {
    ContextMode mode = ContextMode::COMPLETE;   ///< 保存模式
    bool save_fpu = true;                       ///< 是否保存FPU/SSE状态
    bool enable_debugging = false;              ///< 是否启用调试信息
    
    ContextConfig() = default;
    
    explicit ContextConfig(ContextMode m, bool fpu = true, bool debug = false)
        : mode(m), save_fpu(fpu), enable_debugging(debug) {}
};

/**
 * @brief CPU寄存器状态结构 (x86_64)
 * 
 * 根据System V ABI和libaco的优化经验设计：
 * - 保存被调用者保存的寄存器 (callee-saved registers)
 * - 栈指针和基址指针
 * - 返回地址
 * - 可选的FPU/SSE状态
 */
struct alignas(16) RegisterState {
#ifdef __x86_64__
    // 被调用者保存的通用寄存器 (按libaco优化顺序)
    void* r12;      ///< r12 通用寄存器
    void* r13;      ///< r13 通用寄存器  
    void* r14;      ///< r14 通用寄存器
    void* r15;      ///< r15 通用寄存器
    void* rip;      ///< 指令指针 (返回地址)
    void* rsp;      ///< 栈指针
    void* rbx;      ///< rbx 通用寄存器
    void* rbp;      ///< 基址指针
    
    // FPU/SSE状态 (可选，通过配置控制)
    uint16_t fpucw;     ///< FPU控制字
    uint32_t mxcsr;     ///< SSE控制和状态寄存器
    uint16_t _padding;  ///< 内存对齐填充
#else
    #error "Currently only x86_64 is supported. Other platforms will be added in future versions."
#endif
};

// 静态断言确保寄存器状态结构大小正确
// x86_64: 8个指针(8*8=64字节) + uint16_t(2字节) + uint32_t(4字节) + uint16_t(2字节) = 72字节
static_assert(sizeof(RegisterState) >= 72, "RegisterState size must be at least 72 bytes for x86_64");
static_assert(alignof(RegisterState) == 16, "RegisterState must be 16-byte aligned");

/**
 * @brief 协程上下文类
 * 
 * 封装协程的CPU状态，提供现代C++风格的上下文切换接口。
 * 设计原则：
 * - RAII资源管理
 * - 异常安全
 * - 高性能
 * - 类型安全
 */
class Context {
public:
    /**
     * @brief 默认构造函数，初始化空上下文
     * @param config 上下文配置选项
     */
    explicit Context(const ContextConfig& config = ContextConfig{});
    
    /**
     * @brief 析构函数，清理资源
     */
    ~Context() noexcept;
    
    /**
     * @brief 移动构造函数
     * @param other 要移动的上下文对象
     */
    Context(Context&& other) noexcept;
    
    /**
     * @brief 移动赋值运算符
     * @param other 要移动的上下文对象
     * @return Context& 当前对象引用
     */
    Context& operator=(Context&& other) noexcept;
    
    // 禁用拷贝构造和拷贝赋值 (上下文应该是唯一的)
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    
    /**
     * @brief 保存当前执行上下文
     * @return bool 保存是否成功
     * 
     * 将当前CPU状态保存到此上下文对象中。
     * 通常在协程切换时调用。
     */
    bool save() noexcept;
    
    /**
     * @brief 恢复到指定上下文
     * @return bool 恢复是否成功
     * 
     * 将CPU状态恢复到此上下文保存的状态。
     * 注意：此函数不会返回，会直接跳转到保存时的执行点。
     */
    [[noreturn]] void restore() noexcept;
    
    /**
     * @brief 原子性的上下文切换
     * @param other 要切换到的上下文
     * @return bool 切换是否成功
     * 
     * 保存当前上下文，然后切换到目标上下文。
     * 这是协程切换的核心操作。
     */
    bool swap(Context& other) noexcept;
    
    /**
     * @brief 检查上下文是否有效
     * @return bool 上下文是否已初始化且有效
     */
    bool is_valid() const noexcept;
    
    /**
     * @brief 重置上下文状态
     * 
     * 将上下文重置为初始状态，清除所有保存的寄存器状态。
     */
    void reset() noexcept;
    
    /**
     * @brief 获取栈指针
     * @return void* 当前保存的栈指针，如果上下文无效则返回nullptr
     */
    void* get_stack_pointer() const noexcept;
    
    /**
     * @brief 设置栈指针
     * @param sp 新的栈指针值
     * @return bool 设置是否成功
     */
    bool set_stack_pointer(void* sp) noexcept;
    
    /**
     * @brief 获取指令指针
     * @return void* 当前保存的指令指针，如果上下文无效则返回nullptr
     */
    void* get_instruction_pointer() const noexcept;
    
    /**
     * @brief 设置指令指针
     * @param ip 新的指令指针值
     * @return bool 设置是否成功
     */
    bool set_instruction_pointer(void* ip) noexcept;
    
    /**
     * @brief 获取上下文配置
     * @return const ContextConfig& 当前配置的引用
     */
    const ContextConfig& get_config() const noexcept;
    
    /**
     * @brief 获取上下文统计信息
     * @return size_t 上下文切换次数
     */
    size_t get_switch_count() const noexcept;

private:
    RegisterState registers_;           ///< CPU寄存器状态
    ContextConfig config_;              ///< 上下文配置
    bool is_valid_;                     ///< 上下文有效性标记
    size_t switch_count_;               ///< 上下文切换统计
    
    /**
     * @brief 验证上下文状态的完整性
     * @return bool 上下文状态是否完整
     */
    bool validate_state() const noexcept;
    
    /**
     * @brief 初始化寄存器状态
     */
    void initialize_registers() noexcept;
};

/**
 * @brief 上下文切换的汇编函数声明
 * 
 * 这些函数在 context_switch.S 中实现，提供底层的寄存器保存和恢复。
 */
extern "C" {
    /**
     * @brief 执行上下文切换的底层汇编函数
     * @param from_regs 源上下文的寄存器状态指针
     * @param to_regs 目标上下文的寄存器状态指针
     * @param save_fpu 是否保存FPU/SSE状态
     */
    void libco_oop_context_swap(RegisterState* from_regs, RegisterState* to_regs, bool save_fpu) noexcept;
    
    /**
     * @brief 保存当前上下文的底层汇编函数
     * @param regs 要保存到的寄存器状态指针
     * @param save_fpu 是否保存FPU/SSE状态
     * @return int 保存操作的返回值 (0=第一次调用, 1=从restore返回)
     */
    int libco_oop_context_save(RegisterState* regs, bool save_fpu) noexcept;
    
    /**
     * @brief 恢复上下文的底层汇编函数
     * @param regs 要恢复的寄存器状态指针
     * @param save_fpu 是否恢复FPU/SSE状态
     */
    [[noreturn]] void libco_oop_context_restore(RegisterState* regs, bool save_fpu) noexcept;
}

/**
 * @brief 全局上下文工具函数
 */
namespace context_utils {
    /**
     * @brief 获取当前栈指针
     * @return void* 当前栈指针值
     */
    void* get_current_stack_pointer() noexcept;
    
    /**
     * @brief 检查栈指针是否对齐
     * @param sp 要检查的栈指针
     * @return bool 是否按16字节对齐
     */
    bool is_stack_aligned(void* sp) noexcept;
    
    /**
     * @brief 对齐栈指针
     * @param sp 要对齐的栈指针
     * @return void* 对齐后的栈指针
     */
    void* align_stack_pointer(void* sp) noexcept;
}

} // namespace libco_oop

#endif // LIBCO_OOP_CONTEXT_H 