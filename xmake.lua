-- LibCo-OOP 协程库构建配置
-- 设置项目基本信息
set_project("libco-oop")
set_version("1.0.0")
set_languages("c++17")

-- 设置构建模式
add_rules("mode.debug", "mode.release", "mode.coverage")

-- 配置编译选项
if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
    add_defines("DEBUG")
    add_cxflags("-g", "-O0")
    -- 启用AddressSanitizer用于调试
    add_cxflags("-fsanitize=address")
    add_ldflags("-fsanitize=address")
elseif is_mode("release") then
    set_symbols("hidden")
    set_optimize("fastest")
    add_defines("NDEBUG")
    add_cxflags("-O2")
elseif is_mode("coverage") then
    set_symbols("debug")
    set_optimize("none")
    add_cxflags("-g", "-O0", "--coverage")
    add_ldflags("--coverage")
end

-- 设置警告选项
add_cxflags("-Wall", "-Wextra", "-Werror")
add_cxflags("-Wno-unused-parameter") -- 允许未使用的参数
add_cxflags("-Wno-cpp") -- 忽略预处理器警告（C++17兼容性）

-- 设置包含目录
add_includedirs("include", "include/internal")

-- 配置依赖包
add_requires("gtest", "benchmark")

-- 主静态库目标
target("libco_oop")
    set_kind("static")
    -- 添加核心源文件
    add_files("src/core/context.cpp")
    add_files("src/core/context_switch.S")
    -- 保留空文件确保编译
    add_files("src/core/empty.cpp")
    -- 后续会逐步添加：src/scheduler/*.cpp, src/io/*.cpp, src/utils/*.cpp
    
    -- 设置输出目录
    set_targetdir("build/lib")
    set_objectdir("build/obj")

-- 单元测试目标
target("unit_tests")
    set_kind("binary")
    add_deps("libco_oop")
    add_files("tests/unit/*.cpp")
    add_packages("gtest")
    add_links("pthread") -- gtest需要pthread
    
    -- 设置输出目录
    set_targetdir("build/bin")
    
    -- 测试运行配置
    after_build(function (target)
        print("Unit tests built successfully")
    end)

-- 性能基准测试目标
target("benchmark_tests")
    set_kind("binary")
    add_deps("libco_oop")
    add_files("tests/benchmark/*.cpp")
    add_packages("benchmark")
    add_links("pthread")
    
    set_targetdir("build/bin")

-- 自定义任务：运行单元测试
task("test")
    on_run(function ()
        os.exec("xmake run unit_tests")
    end)
    set_menu {
        usage = "xmake test",
        description = "Run unit tests"
    }

-- 自定义任务：性能基准
task("bench")
    on_run(function ()
        os.exec("xmake run benchmark_tests")
    end)
    set_menu {
        usage = "xmake bench",
        description = "Run performance benchmarks"
    }

-- 自定义任务：代码覆盖率
task("coverage")
    on_run(function ()
        os.exec("xmake config --mode=coverage")
        os.exec("xmake build")
        os.exec("xmake run unit_tests")
        print("Coverage report: run 'gcov build/obj/*.gcno' to generate coverage data")
    end)
    set_menu {
        usage = "xmake coverage",
        description = "Generate code coverage report"
    }

-- 清理任务增强
task("clean-all")
    on_run(function ()
        os.exec("xmake clean")
        os.rm("build")
        os.rm("coverage.info")
        os.rm("coverage_html")
        os.rm("*.gcov")
        print("All build artifacts cleaned")
    end)
    set_menu {
        usage = "xmake clean-all",
        description = "Clean all build artifacts and coverage data"
    }

-- 显示项目信息
task("info")
    on_run(function ()
        print("LibCo-OOP 协程库项目信息:")
        print("版本: 1.0.0")
        print("语言: C++17")
        print("当前阶段: TASK001 - 项目基础设施搭建 (已完成)")
        print("下一阶段: TASK002 - 协程上下文管理实现")
        print("")
        print("可用命令:")
        print("  xmake build          - 编译项目")
        print("  xmake test           - 运行单元测试")
        print("  xmake bench          - 运行性能基准测试")
        print("  xmake coverage       - 生成代码覆盖率报告")
        print("  xmake clean-all      - 清理所有构建产物")
    end)
    set_menu {
        usage = "xmake info",
        description = "Show project information"
    }
