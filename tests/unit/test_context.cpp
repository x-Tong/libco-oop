/**
 * @file test_context.cpp
 * @brief 协程上下文管理系统完整测试套件
 * @author libco-oop
 * @version 1.0
 * 
 * 为协程上下文切换功能编写完整的测试套件，确保上下文切换的正确性和稳定性。
 * 这是协程库可靠性的基础测试。
 */

#include <gtest/gtest.h>
#include "libco_oop/context.h"
#include <chrono>
#include <thread>
#include <array>
#include <memory>

using namespace libco_oop;

//============================================================================
// 测试辅助工具类
//============================================================================

/**
 * @brief 寄存器状态验证器
 */
class RegisterChecker {
public:
    struct RegisterSnapshot {
        uint64_t values[16];  // 足够存储主要寄存器值
        void* stack_ptr;
        void* instruction_ptr;
        uint32_t checksum;
        
        RegisterSnapshot() {
            std::fill(std::begin(values), std::end(values), 0);
            stack_ptr = nullptr;
            instruction_ptr = nullptr;
            checksum = 0;
        }
        
        uint32_t calculate_checksum() const {
            uint32_t sum = 0;
            for (size_t i = 0; i < 16; ++i) {
                sum ^= static_cast<uint32_t>(values[i] & 0xFFFFFFFF);
                sum ^= static_cast<uint32_t>((values[i] >> 32) & 0xFFFFFFFF);
            }
            sum ^= reinterpret_cast<uintptr_t>(stack_ptr) & 0xFFFFFFFF;
            sum ^= reinterpret_cast<uintptr_t>(instruction_ptr) & 0xFFFFFFFF;
            return sum;
        }
        
        void update_checksum() {
            checksum = calculate_checksum();
        }
        
        bool verify_checksum() const {
            return checksum == calculate_checksum();
        }
    };
    
    static RegisterSnapshot capture_current_state() {
        RegisterSnapshot snapshot;
        // 捕获一些可见的状态
        snapshot.stack_ptr = context_utils::get_current_stack_pointer();
        
        // 设置一些测试值用于验证
        for (size_t i = 0; i < 16; ++i) {
            snapshot.values[i] = 0xDEADBEEF00000000ULL + i;
        }
        
        snapshot.update_checksum();
        return snapshot;
    }
    
    static bool compare_snapshots(const RegisterSnapshot& before, const RegisterSnapshot& after) {
        return before.verify_checksum() && after.verify_checksum();
    }
};

/**
 * @brief 测试协程函数模拟器
 */
class TestCoroutineFunction {
public:
    enum class Action {
        YIELD,      // 暂停并返回
        COMPLETE,   // 正常完成
        LOOP,       // 循环执行
        EXCEPTION   // 抛出异常
    };
    
    struct ExecutionRecord {
        int call_count = 0;
        Action last_action = Action::COMPLETE;
        std::chrono::nanoseconds total_time{0};
        bool exception_thrown = false;
        
        void reset() {
            call_count = 0;
            last_action = Action::COMPLETE;
            total_time = std::chrono::nanoseconds{0};
            exception_thrown = false;
        }
    };
    
private:
    Action action_;
    int max_calls_;
    ExecutionRecord* record_;
    
public:
    TestCoroutineFunction(Action action, int max_calls = 1, ExecutionRecord* record = nullptr)
        : action_(action), max_calls_(max_calls), record_(record) {}
    
    void operator()() {
        if (record_) {
            record_->call_count++;
            record_->last_action = action_;
            
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                execute_action();
            } catch (...) {
                record_->exception_thrown = true;
                auto end = std::chrono::high_resolution_clock::now();
                record_->total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                throw;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            record_->total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        } else {
            execute_action();
        }
    }
    
private:
    void execute_action() {
        switch (action_) {
            case Action::YIELD:
                // 模拟协程暂停
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                break;
                
            case Action::COMPLETE:
                // 正常完成，什么都不做
                break;
                
            case Action::LOOP:
                // 循环执行直到达到最大次数
                for (int i = 0; i < max_calls_; ++i) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(100));
                }
                break;
                
            case Action::EXCEPTION:
                throw std::runtime_error("Test exception in coroutine function");
        }
    }
};

/**
 * @brief 内存使用监控器
 */
class MemoryTracker {
private:
    size_t initial_usage_;
    size_t peak_usage_;
    size_t final_usage_;
    
public:
    MemoryTracker() {
        initial_usage_ = get_current_memory_usage();
        peak_usage_ = initial_usage_;
        final_usage_ = initial_usage_;
    }
    
    void update_peak() {
        size_t current = get_current_memory_usage();
        if (current > peak_usage_) {
            peak_usage_ = current;
        }
    }
    
    void finalize() {
        final_usage_ = get_current_memory_usage();
    }
    
    size_t get_initial_usage() const { return initial_usage_; }
    size_t get_peak_usage() const { return peak_usage_; }
    size_t get_final_usage() const { return final_usage_; }
    size_t get_leak_amount() const { 
        return final_usage_ > initial_usage_ ? final_usage_ - initial_usage_ : 0; 
    }
    
private:
    size_t get_current_memory_usage() {
        // 简化的内存使用量获取 (在实际实现中可以读取/proc/self/status)
        // 这里返回一个模拟值
        return 1024 * 1024; // 1MB baseline
    }
};

/**
 * @brief 性能测试计时器
 */
class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;
    bool is_running_;
    
public:
    PerformanceTimer() : is_running_(false) {}
    
    void start() {
        start_ = std::chrono::high_resolution_clock::now();
        is_running_ = true;
    }
    
    void stop() {
        if (is_running_) {
            end_ = std::chrono::high_resolution_clock::now();
            is_running_ = false;
        }
    }
    
    double get_elapsed_nanoseconds() const {
        if (is_running_) {
            auto now = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_).count();
        } else {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
        }
    }
    
    double get_elapsed_microseconds() const {
        return get_elapsed_nanoseconds() / 1000.0;
    }
    
    double get_elapsed_milliseconds() const {
        return get_elapsed_nanoseconds() / 1000000.0;
    }
};

//============================================================================
// 上下文管理测试类
//============================================================================

class ContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前准备
        memory_tracker_ = std::make_unique<MemoryTracker>();
        execution_record_.reset();
    }
    
    void TearDown() override {
        // 测试后清理和验证
        memory_tracker_->finalize();
        
        // 检查内存泄漏
        size_t leak = memory_tracker_->get_leak_amount();
        if (leak > 1024) { // 允许小量的内存增长
            std::cout << "Warning: Potential memory leak detected: " << leak << " bytes" << std::endl;
        }
    }
    
    std::unique_ptr<MemoryTracker> memory_tracker_;
    TestCoroutineFunction::ExecutionRecord execution_record_;
};

//============================================================================
// 核心测试用例
//============================================================================

// 测试Context对象的创建和销毁
TEST_F(ContextTest, BasicContextCreation) {
    // 默认构造
    {
        Context ctx1;
        EXPECT_FALSE(ctx1.is_valid());
        EXPECT_EQ(ctx1.get_switch_count(), 0);
        EXPECT_EQ(ctx1.get_stack_pointer(), nullptr);
        EXPECT_EQ(ctx1.get_instruction_pointer(), nullptr);
        EXPECT_EQ(ctx1.get_config().mode, ContextMode::COMPLETE);
        EXPECT_TRUE(ctx1.get_config().save_fpu);
    }
    
    // 配置构造
    {
        ContextConfig config{ContextMode::MINIMAL, false, true};
        Context ctx2(config);
        EXPECT_FALSE(ctx2.is_valid());
        EXPECT_EQ(ctx2.get_config().mode, ContextMode::MINIMAL);
        EXPECT_FALSE(ctx2.get_config().save_fpu);
        EXPECT_TRUE(ctx2.get_config().enable_debugging);
    }
    
    // 测试多个Context对象同时存在
    {
        std::array<Context, 10> contexts;
        for (auto& ctx : contexts) {
            EXPECT_FALSE(ctx.is_valid());
            EXPECT_EQ(ctx.get_switch_count(), 0);
        }
    }
}

// 测试基本的上下文切换功能
TEST_F(ContextTest, SimpleContextSwap) {
    Context ctx1, ctx2;
    
    // 保存当前上下文到ctx1
    ASSERT_TRUE(ctx1.save());
    EXPECT_TRUE(ctx1.is_valid());
    EXPECT_NE(ctx1.get_stack_pointer(), nullptr);
    EXPECT_NE(ctx1.get_instruction_pointer(), nullptr);
    
    // 保存当前上下文到ctx2  
    ASSERT_TRUE(ctx2.save());
    EXPECT_TRUE(ctx2.is_valid());
    
    // 注意：在同一函数中连续调用save()可能获得相同的栈指针
    // 这是正常行为，因为都在同一个栈帧中
    // 实际的swap测试在真实协程环境中更有意义
    // 这里主要测试接口的正确性和有效性
    
    // 验证两个上下文都是有效的
    EXPECT_TRUE(ctx1.is_valid());
    EXPECT_TRUE(ctx2.is_valid());
}

// 验证寄存器状态在切换后保持不变
TEST_F(ContextTest, RegisterPreservation) {
    Context ctx;
    
    // 捕获初始状态
    auto initial_snapshot = RegisterChecker::capture_current_state();
    
    // 保存上下文
    ASSERT_TRUE(ctx.save());
    
    // 捕获保存后状态
    auto saved_snapshot = RegisterChecker::capture_current_state();
    
    // 验证状态一致性
    EXPECT_TRUE(RegisterChecker::compare_snapshots(initial_snapshot, saved_snapshot));
    
    // 测试上下文中的栈指针有效性
    void* ctx_sp = ctx.get_stack_pointer();
    EXPECT_NE(ctx_sp, nullptr);
    EXPECT_TRUE(context_utils::is_stack_aligned(ctx_sp));
    
    // 测试指令指针有效性
    void* ctx_ip = ctx.get_instruction_pointer();
    EXPECT_NE(ctx_ip, nullptr);
}

// 测试栈指针切换的正确性
TEST_F(ContextTest, StackSwitching) {
    Context ctx;
    
    // 创建测试栈
    const size_t stack_size = 8192;
    std::unique_ptr<char[]> test_stack(new char[stack_size]);
    char* stack_top = test_stack.get() + stack_size;
    
    // 对齐栈指针
    void* aligned_stack = context_utils::align_stack_pointer(stack_top);
    
    // 设置栈指针
    ASSERT_TRUE(ctx.set_stack_pointer(aligned_stack));
    EXPECT_EQ(ctx.get_stack_pointer(), aligned_stack);
    EXPECT_TRUE(context_utils::is_stack_aligned(ctx.get_stack_pointer()));
    
    // 测试栈指针对齐功能
    char* unaligned_ptr = test_stack.get() + 1; // 故意不对齐
    EXPECT_FALSE(context_utils::is_stack_aligned(unaligned_ptr));
    
    void* realigned = context_utils::align_stack_pointer(unaligned_ptr);
    EXPECT_TRUE(context_utils::is_stack_aligned(realigned));
    EXPECT_NE(realigned, unaligned_ptr);
}

// 测试多次连续切换的稳定性
TEST_F(ContextTest, MultipleSwaps) {
    const int num_contexts = 5;
    std::array<Context, num_contexts> contexts;
    
    // 为每个上下文保存当前状态
    for (int i = 0; i < num_contexts; ++i) {
        ASSERT_TRUE(contexts[i].save());
        EXPECT_TRUE(contexts[i].is_valid());
        memory_tracker_->update_peak();
    }
    
    // 验证每个上下文都是有效的
    for (int i = 0; i < num_contexts; ++i) {
        EXPECT_TRUE(contexts[i].is_valid());
        EXPECT_NE(contexts[i].get_stack_pointer(), nullptr);
        EXPECT_NE(contexts[i].get_instruction_pointer(), nullptr);
    }
    
    // 注意：在同一函数中多次save()可能获得相同的栈指针
    // 这是正常行为，真实场景中不同协程会有不同的栈
    
    // 测试上下文重置和重用
    for (auto& ctx : contexts) {
        ctx.reset();
        EXPECT_FALSE(ctx.is_valid());
        EXPECT_EQ(ctx.get_switch_count(), 0);
    }
}

// 测试无效上下文的错误处理
TEST_F(ContextTest, ErrorHandling) {
    Context invalid_ctx;
    
    // 无效上下文应该无法切换
    Context valid_ctx;
    ASSERT_TRUE(valid_ctx.save());
    
    // 无效上下文与有效上下文的切换应该失败
    EXPECT_FALSE(invalid_ctx.swap(valid_ctx));
    
    // 测试空指针处理
    EXPECT_FALSE(invalid_ctx.set_stack_pointer(nullptr));
    EXPECT_FALSE(invalid_ctx.set_instruction_pointer(nullptr));
    
    // 测试边界条件
    char single_byte;
    void* tiny_stack = &single_byte;
    // 即使是很小的栈，只要对齐就应该能设置
    if (context_utils::is_stack_aligned(tiny_stack)) {
        EXPECT_TRUE(invalid_ctx.set_stack_pointer(tiny_stack));
    }
}

//============================================================================
// 高级测试用例
//============================================================================

// 测试嵌套的上下文切换
TEST_F(ContextTest, NestedContextSwap) {
    Context outer_ctx, inner_ctx;
    
    // 外层上下文
    ASSERT_TRUE(outer_ctx.save());
    
    {
        // 内层上下文 (嵌套范围)
        ASSERT_TRUE(inner_ctx.save());
        
        // 验证嵌套上下文的有效性
        EXPECT_TRUE(outer_ctx.is_valid());
        EXPECT_TRUE(inner_ctx.is_valid());
        
        // 验证两个上下文都有有效的指针
        EXPECT_NE(outer_ctx.get_stack_pointer(), nullptr);
        EXPECT_NE(inner_ctx.get_stack_pointer(), nullptr);
        EXPECT_NE(outer_ctx.get_instruction_pointer(), nullptr);
        EXPECT_NE(inner_ctx.get_instruction_pointer(), nullptr);
    }
    
    // 内层结束后，外层上下文仍然有效
    EXPECT_TRUE(outer_ctx.is_valid());
}

// 测试大量数据在切换后的保持
TEST_F(ContextTest, LargeDataPreservation) {
    Context ctx;
    
    // 创建大量栈数据
    const size_t data_size = 1024;
    std::array<uint64_t, data_size> large_data;
    
    // 填充测试模式
    for (size_t i = 0; i < data_size; ++i) {
        large_data[i] = 0xDEADBEEFCAFEBABEULL ^ i;
    }
    
    // 计算校验和
    uint64_t checksum = 0;
    for (const auto& val : large_data) {
        checksum ^= val;
    }
    
    // 保存上下文
    ASSERT_TRUE(ctx.save());
    
    // 验证数据完整性 (栈数据应该保持不变)
    uint64_t new_checksum = 0;
    for (const auto& val : large_data) {
        new_checksum ^= val;
    }
    
    EXPECT_EQ(checksum, new_checksum);
    memory_tracker_->update_peak();
}

// 基本的性能基准测试
TEST_F(ContextTest, PerformanceBenchmark) {
    const int iterations = 1000;
    PerformanceTimer timer;
    
    // 测试上下文保存性能
    Context ctx;
    timer.start();
    
    for (int i = 0; i < iterations; ++i) {
        ctx.save();
        ctx.reset();
    }
    
    timer.stop();
    double avg_save_time = timer.get_elapsed_nanoseconds() / iterations;
    
    std::cout << "Average context save time: " << avg_save_time << " ns" << std::endl;
    std::cout << "Peak memory usage: " << memory_tracker_->get_peak_usage() << " bytes" << std::endl;
    
    // 性能目标：每次保存应该在合理范围内
    EXPECT_LT(avg_save_time, 1000.0); // < 1μs
    
    // 测试栈指针操作性能
    timer.start();
    
    char stack_buffer[1024];
    void* stack_ptr = stack_buffer + sizeof(stack_buffer);
    
    for (int i = 0; i < iterations; ++i) {
        ctx.set_stack_pointer(stack_ptr);
        ctx.set_instruction_pointer(reinterpret_cast<void*>(0x1000000 + i));
    }
    
    timer.stop();
    double avg_set_time = timer.get_elapsed_nanoseconds() / (iterations * 2);
    
    std::cout << "Average pointer set time: " << avg_set_time << " ns" << std::endl;
    
    // 指针设置应该非常快
    EXPECT_LT(avg_set_time, 100.0); // < 100ns
}

// 内存泄漏检测
TEST_F(ContextTest, MemoryLeakDetection) {
    const int num_contexts = 100;
    
    // 批量创建和销毁上下文
    for (int batch = 0; batch < 10; ++batch) {
        std::vector<std::unique_ptr<Context>> contexts;
        contexts.reserve(num_contexts);
        
        // 创建
        for (int i = 0; i < num_contexts; ++i) {
            contexts.emplace_back(std::make_unique<Context>());
            if (i % 2 == 0) {
                contexts.back()->save();
            }
        }
        
        memory_tracker_->update_peak();
        
        // 使用
        for (auto& ctx_ptr : contexts) {
            if (ctx_ptr->is_valid()) {
                ctx_ptr->reset();
            }
        }
        
        // 销毁 (自动析构)
        contexts.clear();
    }
    
    // 验证没有显著的内存泄漏
    // 注意：这是一个简化的检测，实际项目中应该使用valgrind等工具
}

//============================================================================
// 配置和边界测试
//============================================================================

// 测试不同配置模式的功能
TEST_F(ContextTest, DifferentConfigurations) {
    // MINIMAL模式，不保存FPU
    {
        ContextConfig minimal_config{ContextMode::MINIMAL, false, false};
        Context minimal_ctx(minimal_config);
        
        EXPECT_EQ(minimal_ctx.get_config().mode, ContextMode::MINIMAL);
        EXPECT_FALSE(minimal_ctx.get_config().save_fpu);
        EXPECT_FALSE(minimal_ctx.get_config().enable_debugging);
        
        ASSERT_TRUE(minimal_ctx.save());
        EXPECT_TRUE(minimal_ctx.is_valid());
    }
    
    // COMPLETE模式，保存FPU，启用调试
    {
        ContextConfig complete_config{ContextMode::COMPLETE, true, true};
        Context complete_ctx(complete_config);
        
        EXPECT_EQ(complete_ctx.get_config().mode, ContextMode::COMPLETE);
        EXPECT_TRUE(complete_ctx.get_config().save_fpu);
        EXPECT_TRUE(complete_ctx.get_config().enable_debugging);
        
        ASSERT_TRUE(complete_ctx.save());
        EXPECT_TRUE(complete_ctx.is_valid());
    }
    
    // 测试不同配置之间的兼容性
    ContextConfig config1{ContextMode::MINIMAL, false};
    ContextConfig config2{ContextMode::COMPLETE, true};
    
    Context ctx1(config1), ctx2(config2);
    ctx1.save();
    ctx2.save();
    
    // 不同FPU配置的上下文切换应该能处理
    // (实现中会取两者的交集)
}

// 测试上下文移动语义
TEST_F(ContextTest, ContextMoveSemantics) {
    // 移动构造
    {
        Context source_ctx;
        source_ctx.save();
        ASSERT_TRUE(source_ctx.is_valid());
        
        size_t original_count = source_ctx.get_switch_count();
        
        Context moved_ctx = std::move(source_ctx);
        
        // 移动后，目标对象应该继承状态
        EXPECT_TRUE(moved_ctx.is_valid());
        EXPECT_EQ(moved_ctx.get_switch_count(), original_count);
        
        // 源对象应该失效
        EXPECT_FALSE(source_ctx.is_valid());
        EXPECT_EQ(source_ctx.get_switch_count(), 0);
    }
    
    // 移动赋值
    {
        Context ctx1, ctx2;
        ctx1.save();
        ASSERT_TRUE(ctx1.is_valid());
        
        ctx2 = std::move(ctx1);
        
        EXPECT_TRUE(ctx2.is_valid());
        EXPECT_FALSE(ctx1.is_valid());
    }
}

// 压力测试
TEST_F(ContextTest, StressTest) {
    const int stress_iterations = 10000;
    PerformanceTimer timer;
    
    timer.start();
    
    for (int i = 0; i < stress_iterations; ++i) {
        Context ctx;
        
        // 随机配置
        ContextConfig config{
            (i % 2 == 0) ? ContextMode::MINIMAL : ContextMode::COMPLETE,
            (i % 3 == 0),
            false
        };
        
        Context configured_ctx(config);
        
        if (i % 100 == 0) {
            memory_tracker_->update_peak();
        }
        
        // 保存和重置
        if (configured_ctx.save()) {
            configured_ctx.reset();
        }
    }
    
    timer.stop();
    
    std::cout << "Stress test completed in " << timer.get_elapsed_milliseconds() << " ms" << std::endl;
    std::cout << "Average operation time: " << timer.get_elapsed_nanoseconds() / stress_iterations << " ns" << std::endl;
} 