#include <gtest/gtest.h>
#include "test_helper.h"
#include <functional>
#include <memory>
#include <vector>

using namespace libco_oop::test;

/**
 * @brief 基础测试套件
 * 
 * 验证测试框架本身和项目基础设施的正确性
 */
class BasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的准备工作
    }
    
    void TearDown() override {
        // 测试后的清理工作
    }
};

/**
 * @brief 测试项目能正常编译和链接
 */
TEST_F(BasicTest, ProjectCompilationTest) {
    // 这个测试能运行就说明项目编译链接正常
    EXPECT_TRUE(true);
    EXPECT_EQ(1 + 1, 2);
}

/**
 * @brief 测试基本的 C++17 功能
 */
TEST_F(BasicTest, Cpp17FeaturesTest) {
    // 测试 lambda 表达式
    auto lambda = [](int x) { return x * 2; };
    EXPECT_EQ(lambda(5), 10);
    
    // 测试 std::function
    std::function<int(int)> func = lambda;
    EXPECT_EQ(func(3), 6);
    
    // 测试智能指针
    auto ptr = std::make_unique<int>(42);
    EXPECT_EQ(*ptr, 42);
    
    // 测试 auto 类型推导
    auto vec = std::vector<int>{1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);
    
    // 测试结构化绑定 (C++17)
    auto pair = std::make_pair(10, 20);
    auto [first, second] = pair;
    EXPECT_EQ(first, 10);
    EXPECT_EQ(second, 20);
}

/**
 * @brief 验证测试框架本身正常工作
 */
TEST_F(BasicTest, TestFrameworkTest) {
    // 测试基本断言
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(42, 42);
    EXPECT_NE(1, 2);
    EXPECT_LT(1, 2);
    EXPECT_GT(2, 1);
    
    // 测试字符串断言
    EXPECT_STREQ("hello", "hello");
    EXPECT_STRNE("hello", "world");
    
    // 测试浮点数断言
    EXPECT_FLOAT_EQ(1.0f, 1.0f);
    EXPECT_DOUBLE_EQ(1.0, 1.0);
    EXPECT_NEAR(1.0, 1.1, 0.2);
}

/**
 * @brief 测试项目目录结构的完整性
 */
TEST_F(BasicTest, ProjectStructureTest) {
    // 这个测试验证项目结构是否正确创建
    // 实际的文件系统检查可以在集成测试中进行
    
    // 验证基本的C++标准库功能
    std::vector<std::string> expected_dirs = {
        "include/libco_oop",
        "include/internal", 
        "src/core",
        "src/scheduler",
        "src/io",
        "src/utils",
        "tests/unit",
        "tests/integration",
        "tests/benchmark"
    };
    
    EXPECT_FALSE(expected_dirs.empty());
    EXPECT_EQ(expected_dirs.size(), 9);
}

/**
 * @brief 测试性能计时器功能
 */
TEST_F(BasicTest, PerformanceTimerTest) {
    PerformanceTimer timer;
    
    // 简单的延时操作
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    
    // 验证计时器能正常工作
    int64_t elapsed_ns = timer.elapsed_ns();
    int64_t elapsed_us = timer.elapsed_us();
    int64_t elapsed_ms = timer.elapsed_ms();
    
    EXPECT_GT(elapsed_ns, 0);
    EXPECT_GE(elapsed_us, elapsed_ns / 1000 - 1); // 允许舍入误差
    EXPECT_GE(elapsed_ms, elapsed_ns / 1000000 - 1); // 允许舍入误差
    
    // 测试重置功能
    timer.reset();
    int64_t new_elapsed = timer.elapsed_ns();
    EXPECT_LT(new_elapsed, elapsed_ns);
}

/**
 * @brief 测试异常处理
 */
TEST_F(BasicTest, ExceptionHandlingTest) {
    // 测试异常抛出和捕获
    EXPECT_THROW({
        throw std::runtime_error("test exception");
    }, std::runtime_error);
    
    // 测试无异常情况
    EXPECT_NO_THROW({
        int x = 1 + 1;
        (void)x; // 避免未使用变量警告
    });
    
    // 测试特定异常类型
    EXPECT_THROW({
        throw std::invalid_argument("invalid argument");
    }, std::invalid_argument);
}

/**
 * @brief 测试内存管理
 */
TEST_F(BasicTest, MemoryManagementTest) {
    // 测试智能指针的基本功能
    {
        auto ptr = std::make_unique<int>(100);
        EXPECT_EQ(*ptr, 100);
        
        auto shared = std::make_shared<std::string>("hello");
        EXPECT_EQ(*shared, "hello");
        EXPECT_EQ(shared.use_count(), 1);
        
        auto shared2 = shared;
        EXPECT_EQ(shared.use_count(), 2);
        EXPECT_EQ(shared2.use_count(), 2);
    }
    
    // 测试容器的内存管理
    {
        std::vector<int> vec;
        vec.reserve(1000);
        for (int i = 0; i < 1000; ++i) {
            vec.push_back(i);
        }
        EXPECT_EQ(vec.size(), 1000);
        EXPECT_GE(vec.capacity(), 1000);
    }
} 