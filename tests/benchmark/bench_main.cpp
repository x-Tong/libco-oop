#include <benchmark/benchmark.h>
#include <iostream>

/**
 * @brief LibCo-OOP 性能基准测试主函数
 * 
 * 初始化Google Benchmark框架，运行所有性能测试
 */
int main(int argc, char** argv) {
    // 初始化 Google Benchmark
    ::benchmark::Initialize(&argc, argv);
    
    // 检查是否有错误
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    
    // 输出基准测试开始信息
    std::cout << "=== LibCo-OOP Performance Benchmarks ===" << std::endl;
    std::cout << "Running benchmarks with Google Benchmark framework" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // 运行所有基准测试
    ::benchmark::RunSpecifiedBenchmarks();
    
    // 输出基准测试结束信息
    std::cout << "=============================================" << std::endl;
    std::cout << "Benchmark tests completed!" << std::endl;
    
    // 关闭基准测试框架
    ::benchmark::Shutdown();
    
    return 0;
} 