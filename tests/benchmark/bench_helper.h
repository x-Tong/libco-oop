#pragma once

#include <benchmark/benchmark.h>
#include <functional>
#include <vector>

namespace libco_oop {
namespace benchmark {

/**
 * @brief 性能测试辅助工具类
 * 
 * 提供性能测试的通用功能和工具
 */
class BenchmarkHelper {
public:
    /**
     * @brief 生成随机数据用于性能测试
     * @param size 数据大小
     * @return 随机数据向量
     */
    static std::vector<int> generate_random_data(size_t size);
    
    /**
     * @brief 预热CPU，确保性能测试的准确性
     * @param iterations 预热迭代次数
     */
    static void warmup_cpu(int iterations = 1000000);
    
    /**
     * @brief 创建计算密集型工作负载
     * @param complexity 复杂度级别
     * @return 工作负载函数
     */
    static std::function<void()> create_cpu_intensive_workload(int complexity = 1000);
    
    /**
     * @brief 创建内存密集型工作负载
     * @param memory_size 内存大小（字节）
     * @return 工作负载函数
     */
    static std::function<void()> create_memory_intensive_workload(size_t memory_size = 1024 * 1024);
};

/**
 * @brief 协程性能测试辅助类
 * 
 * 专门用于协程相关的性能测试
 */
class CoroutineBenchmarkHelper {
public:
    /**
     * @brief 创建简单的协程函数用于性能测试
     * @return 协程函数
     */
    static std::function<void()> create_simple_coroutine();
    
    /**
     * @brief 创建会yield的协程函数
     * @param yield_count yield次数
     * @return 协程函数
     */
    static std::function<void()> create_yielding_coroutine(int yield_count = 10);
    
    /**
     * @brief 创建计算密集型协程函数
     * @param iterations 计算迭代次数
     * @return 协程函数
     */
    static std::function<void()> create_compute_coroutine(int iterations = 1000);
};

/**
 * @brief 性能测试宏定义
 */

// 基础性能测试宏
#define BENCHMARK_COROUTINE_CREATION(name) \
    static void BM_##name(::benchmark::State& state) { \
        for (auto _ : state) { \
            /* 协程创建性能测试代码 */ \
            ::benchmark::DoNotOptimize(state.iterations()); \
        } \
    } \
    BENCHMARK(BM_##name)

// 协程切换性能测试宏
#define BENCHMARK_COROUTINE_SWITCH(name) \
    static void BM_##name(::benchmark::State& state) { \
        for (auto _ : state) { \
            /* 协程切换性能测试代码 */ \
            ::benchmark::DoNotOptimize(state.iterations()); \
        } \
    } \
    BENCHMARK(BM_##name)

// 内存使用性能测试宏
#define BENCHMARK_MEMORY_USAGE(name) \
    static void BM_##name(::benchmark::State& state) { \
        for (auto _ : state) { \
            /* 内存使用性能测试代码 */ \
            ::benchmark::DoNotOptimize(state.iterations()); \
        } \
        state.SetBytesProcessed(state.iterations() * sizeof(int)); \
    } \
    BENCHMARK(BM_##name)

// 吞吐量性能测试宏
#define BENCHMARK_THROUGHPUT(name) \
    static void BM_##name(::benchmark::State& state) { \
        for (auto _ : state) { \
            /* 吞吐量性能测试代码 */ \
            ::benchmark::DoNotOptimize(state.iterations()); \
        } \
        state.SetItemsProcessed(state.iterations()); \
    } \
    BENCHMARK(BM_##name)

/**
 * @brief 性能测试配置常量
 */
namespace config {
    // 默认的性能测试迭代次数
    constexpr int DEFAULT_ITERATIONS = 1000000;
    
    // 协程创建性能目标（纳秒）
    constexpr int64_t COROUTINE_CREATION_TARGET_NS = 1000;
    
    // 协程切换性能目标（纳秒）
    constexpr int64_t COROUTINE_SWITCH_TARGET_NS = 20;
    
    // 内存使用目标（字节每协程）
    constexpr size_t MEMORY_USAGE_TARGET_BYTES = 4096;
    
    // 调度延迟目标（微秒）
    constexpr int64_t SCHEDULING_LATENCY_TARGET_US = 100;
}

} // namespace benchmark
} // namespace libco_oop 