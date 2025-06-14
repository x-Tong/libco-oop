#pragma once

#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <cstddef>

namespace libco_oop {
namespace test {

/**
 * @brief 性能测试计时器
 * 
 * 用于测量代码执行时间，支持纳秒级精度
 */
class PerformanceTimer {
public:
    PerformanceTimer() : start_time_(std::chrono::high_resolution_clock::now()) {}
    
    /**
     * @brief 重置计时器
     */
    void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    /**
     * @brief 获取经过的时间（纳秒）
     */
    int64_t elapsed_ns() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time_).count();
    }
    
    /**
     * @brief 获取经过的时间（微秒）
     */
    int64_t elapsed_us() const {
        return elapsed_ns() / 1000;
    }
    
    /**
     * @brief 获取经过的时间（毫秒）
     */
    int64_t elapsed_ms() const {
        return elapsed_ns() / 1000000;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * @brief 内存使用监控器
 * 
 * 用于检测内存泄漏和监控内存使用情况
 */
class MemoryMonitor {
public:
    /**
     * @brief 获取当前进程的内存使用量（字节）
     */
    static size_t get_memory_usage();
    
    /**
     * @brief 检查内存泄漏
     * @param before_usage 操作前的内存使用量
     * @param after_usage 操作后的内存使用量
     * @param tolerance 允许的内存增长容忍度（字节）
     * @return 是否存在内存泄漏
     */
    static bool check_memory_leak(size_t before_usage, size_t after_usage, 
                                  size_t tolerance = 1024);
};

/**
 * @brief 测试数据生成器
 * 
 * 生成各种测试用的数据
 */
class TestDataGenerator {
public:
    /**
     * @brief 生成随机字符串
     * @param length 字符串长度
     * @return 随机字符串
     */
    static std::string random_string(size_t length);
    
    /**
     * @brief 生成随机整数
     * @param min 最小值
     * @param max 最大值
     * @return 随机整数
     */
    static int random_int(int min, int max);
    
    /**
     * @brief 生成测试用的协程函数
     * @param iterations 迭代次数
     * @return 协程函数
     */
    static std::function<void()> create_test_coroutine_function(int iterations = 10);
};

/**
 * @brief 协程测试辅助类
 * 
 * 提供协程测试的通用功能
 */
class CoroutineTestHelper {
public:
    /**
     * @brief 简单的协程函数，用于测试基本功能
     */
    static void simple_coroutine_function();
    
    /**
     * @brief 会yield的协程函数
     * @param yield_count yield的次数
     */
    static void yielding_coroutine_function(int yield_count = 3);
    
    /**
     * @brief 计算密集型协程函数
     * @param iterations 计算迭代次数
     */
    static void compute_intensive_function(int iterations = 1000000);
    
    /**
     * @brief 抛出异常的协程函数
     */
    static void exception_throwing_function();
};

/**
 * @brief 测试断言宏扩展
 */

// 性能测试断言
#define EXPECT_PERFORMANCE_LT(expr, max_time_ns) \
    do { \
        PerformanceTimer timer; \
        expr; \
        EXPECT_LT(timer.elapsed_ns(), max_time_ns) \
            << "Performance test failed: execution took " \
            << timer.elapsed_ns() << "ns, expected < " << max_time_ns << "ns"; \
    } while(0)

// 内存泄漏检测断言
#define EXPECT_NO_MEMORY_LEAK(expr) \
    do { \
        size_t before = MemoryMonitor::get_memory_usage(); \
        expr; \
        size_t after = MemoryMonitor::get_memory_usage(); \
        EXPECT_FALSE(MemoryMonitor::check_memory_leak(before, after)) \
            << "Memory leak detected: " << (after - before) << " bytes leaked"; \
    } while(0)

// 异常测试断言
#define EXPECT_EXCEPTION_TYPE(expr, exception_type) \
    EXPECT_THROW(expr, exception_type)

#define EXPECT_NO_EXCEPTION(expr) \
    EXPECT_NO_THROW(expr)

} // namespace test
} // namespace libco_oop 