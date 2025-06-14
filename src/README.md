# 源代码组织说明

本目录包含 libco-oop 协程库的所有源代码实现。

## 目录结构

### core/
核心协程实现，包含：
- 协程上下文管理 (Context)
- 协程栈管理 (Stack)
- 基础协程类 (Coroutine)
- 协程池管理 (CoroutinePool)

### scheduler/
调度器实现，包含：
- 基础调度器 (Scheduler)
- 简单轮询调度器 (SimpleScheduler)
- 定时器系统 (Timer, TimerManager)

### io/
IO事件处理实现，包含：
- IO事件管理器 (IOManager)
- epoll封装和事件循环
- 异步IO支持

### utils/
工具类和辅助函数，包含：
- 通用工具函数
- 调试和诊断工具
- 性能分析工具

## 编译说明

使用 xmake 构建系统：
```bash
# 编译静态库
xmake build libco_oop

# 编译所有目标
xmake build
```

## 代码规范

- 使用 C++17 标准
- 遵循现代 C++ 最佳实践
- 使用 RAII 进行资源管理
- 优先使用智能指针和移动语义 