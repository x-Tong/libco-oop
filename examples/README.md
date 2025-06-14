# 示例代码说明

本目录包含 libco-oop 协程库的使用示例。

## 示例分类

### 基础示例
- `basic_coroutine.cpp`: 基本的协程创建和使用
- `coroutine_yield_resume.cpp`: 协程的暂停和恢复
- `multiple_coroutines.cpp`: 多个协程的管理

### 调度器示例
- `simple_scheduler.cpp`: 简单调度器的使用
- `scheduler_policies.cpp`: 不同调度策略的对比

### IO示例
- `async_io.cpp`: 异步IO操作
- `echo_server.cpp`: 简单的echo服务器
- `http_client.cpp`: HTTP客户端实现

### 高级示例
- `coroutine_pool.cpp`: 协程池的使用
- `timer_example.cpp`: 定时器功能演示
- `performance_test.cpp`: 性能测试示例

## 编译和运行

### 编译示例
```bash
# 编译所有示例
xmake build examples

# 编译特定示例
xmake build basic_coroutine
```

### 运行示例
```bash
# 运行基础协程示例
xmake run basic_coroutine

# 运行echo服务器示例
xmake run echo_server
```

## 示例说明

每个示例都包含：
- 详细的代码注释
- 功能说明和使用场景
- 预期的输出结果
- 相关的性能数据

## 学习路径

建议按以下顺序学习示例：

1. **基础入门**
   - basic_coroutine.cpp
   - coroutine_yield_resume.cpp

2. **调度管理**
   - simple_scheduler.cpp
   - multiple_coroutines.cpp

3. **异步IO**
   - async_io.cpp
   - echo_server.cpp

4. **高级功能**
   - coroutine_pool.cpp
   - timer_example.cpp

## 贡献示例

欢迎贡献新的示例：
- 展示特定的使用场景
- 提供最佳实践演示
- 包含详细的注释说明
- 确保代码可以正常编译运行

## 常见问题

### 编译问题
- 确保已安装必要的依赖
- 检查编译器版本是否支持C++17
- 验证xmake配置是否正确

### 运行问题
- 检查系统权限设置
- 确认网络端口是否可用
- 查看错误日志获取详细信息 