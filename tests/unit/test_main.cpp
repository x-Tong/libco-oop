#include <gtest/gtest.h>
#include <iostream>

/**
 * @brief LibCo-OOP 单元测试主函数
 * 
 * 初始化Google Test框架，设置测试环境，运行所有测试用例
 */
int main(int argc, char** argv) {
    // 初始化Google Test框架
    ::testing::InitGoogleTest(&argc, argv);
    
    // 设置测试输出格式
    ::testing::FLAGS_gtest_color = "yes";
    ::testing::FLAGS_gtest_print_time = true;
    
    // 输出测试开始信息
    std::cout << "=== LibCo-OOP Unit Tests ===" << std::endl;
    std::cout << "Running tests with Google Test framework" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // 运行所有测试用例
    int result = RUN_ALL_TESTS();
    
    // 输出测试结束信息
    std::cout << "=================================" << std::endl;
    if (result == 0) {
        std::cout << "All tests passed successfully!" << std::endl;
    } else {
        std::cout << "Some tests failed. Check output above." << std::endl;
    }
    
    return result;
} 