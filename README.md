# MiniSpdlog

MiniSpdlog 是一个从零实现的 轻量级、高可读性、高扩展性 的 C++ 日志库，核心思想参考成熟日志框架的设计理念，并结合自身需求进行结构化重构。项目旨在构建一个 模块化、易维护、可拓展 的日志组件，用于学习日志系统的底层机制，同时作为实际工程项目的可用基础设施。

该库覆盖一个日志系统的关键组成部分，包括：日志器（Logger）、输出端（Sink）、格式化器（Formatter）、日志级别控制、线程安全保障等。整体实现保持简洁，同时保留可扩展的框架结构，可作为进一步优化（异步日志、多 Sink、性能增强）的基础。

---

## 核心功能

1. 多级日志支持：Debug / Info / Warn / Error 等标准日志等级。
2. 模块化设计：Logger、Sink、Formatter 完全解耦，便于扩展新的输出端与格式化方式。
3. 可配置输出策略：控制台输出（Console Sink）、文件输出(File Sink）、彩色控制台(Color Console Sink)、滚动文件(Rotating File Sink)等等
4. 格式化输出：基于互斥锁实现基本的同步日志写入，支持并发场景。
5. 轻量易用的 API：简洁的接口设计，使日志输出调用方式清晰流畅。

---

MiniSpdlog 采用分层结构设计，以便于扩展和维护。

### 1. Logger
日志记录器，负责：
1. Sink 向量管理: logger 持有一个 std::vector<sink_ptr>,每次日志调用会遍历所有 sink 进行输出
2. 级别过滤: logger 自己有一个级别,只有高于此级别的消息才会传递给 sink
3. 变参模板: 使用 C++11 变参模板和 fmt 库实现灵活的日志接口,如 info("val={}", 42)
4. 颜色支持: 使用 ANSI 转义码,在格式化时添加颜色前缀,终端自动识别并渲染
实现日志接口：trace(), debug(), info(), warn(), error(), critical()

### 2. Sink
输出端机制的核心抽象，支持：  
- console_sink：输出到 stdout  
- file_sink：输出到文件  
- color_console_sink：添加ANSI颜色支持的控制台 Sink（仅在终端输出时添加,不影响文件输出）
- rotating_file_sink：按文件大小自动滚动，输出到文件。（文件名：basename.N.ext 格式）
 Sink 使用模板方法模式,base_sink 类处理线程锁定,保证线程安全，子类只需实现 sink_it_ 和 flush_ 两个方法

### 3. Formatter
使用纯虚函数定义接口,允许不同的格式化实现(pattern、JSON、自定义等)。每个 sink 拥有自己的 formatter 实例
formatter 是策略接口，pattern_formatter 是具体策略实现，Sinks 使用不同的策略

### 4. Registry
实现 registry 单例模式设计（线程安全的全局日志管理器）
- 维护一个全局(每个进程)的 logger 注册表，目的是让 logger 能够从项目的任何地方轻松访问，而无需传递它们
- 支持 Logger 的注册、获取、删除，实现默认 Logger 机制。
- 使用局部静态变量实现线程安全的单例(C++11 保证)

### 5. minispdlog
- 全局便捷接口: 直接使用 minispdlog::info() 等函数。（使用默认 logger）
- 工厂函数: 快速创建常用类型的 logger，自动注册到 registry 并返回shared_ptr
- Registry 访问: 简化 registry 操作

### 6. Thread Pool + MPMC Queue
- 全局共享线程池，1个工作线程服务所有异步 logger。
- MPMC 阻塞队列：于 circular_q(循环队列) + mutex + condition_variable，实现多生产者多消费者阻塞队列
- 溢出策略：阻塞、非阻塞（覆盖旧消息）

### 6. async_logger
异步日志记录器：继承自logger
- 所有异步 logger 共享这个线程池
- registry 持有 shared_ptr<thread_pool>、async_logger 持有 weak_ptr<thread_pool> 避免循环引用、async_msg 持有 shared_ptr<async_logger> - 确保 logger 存活直到消息处理完

---

## 项目架构

```
minispdlog/
│── include/minispdlog
│   ├── details/
│   ├── sinks/
│   ├── logger.h
│   ├── formatter.h
│   ├── registry.h
│   ....
│   └── level.h
│
│── src/
│   ├── details/
│   ├── sinks/
│   ├── logger.cpp
│   ├── formatter.cpp
│   └── registry.cpp
│
│── tests/
│   ...
│   └── test_performance.cpp
│
└── CMakeLists.txt
└── README.md
```

---

## MiniSpdlog 的开发初衷包括

- 通过动手实现日志框架，深入理解日志系统的关键设计模式（观察者、策略模式、责任链）。
- 构建一个可以直接用于小型项目或嵌入式系统的轻量级日志库。
- 为进一步的研究或工程优化（异步化、无锁队列、环形缓冲区、多线程性能分析等）奠定基础。
- 提升 C++ 工程能力，包括模块化设计、内存管理、API 设计、高内聚低耦合的代码结构。

---

## 性能优化

MiniSpdlog 在最初实现中，日志格式化部分（`pattern_formatter`）是影响整体吞吐能力的核心瓶颈。通过使用 `perf` 对同步日志路径进行性能分析，热点集中在以下调用链中：

- `pattern_formatter::format`
- `fmt::vformat_to`
- `fmt::detail::write_int_noinline`
- `std::uninitialized_copy_n`

根据分析结果，性能瓶颈主要来自以下三个方面：

### 1. 碎片化的格式化逻辑
对于pattern [%Y-%m-%d %H:%M:%S] [%l] %v，需要调用8次 fmt::format_to！每次调用都有函数开销。

### 2. 缺少文本聚合策略
格式化过程中存在大量小块内容的 append 操作，造成大量函数调用，影响 CPU cache locality，降低内存复制效率。

### 3. 数字格式化开销较高
线程 ID、毫秒级时间戳等字段使用通用格式化(fmt::format_to)接口，会触发额外的类型推断和边界检查，导致不必要的性能损耗。

---

## 优化方案

基于上述问题，我对 Formatter 的实现进行了结构化重构，重点包括：

### • 日志模板预解析
将格式化 pattern（如 `%Y-%m-%d %H:%M:%S [%l] %t %v`）在初始化阶段解析为结构化片段，减少运行时逻辑分支与重复解析。

### • 连续缓冲区聚合输出
在格式化阶段使用连续内存缓冲区，将多段内容一次性聚合输出，减少 append 次数并降低内存分配频率。

### • 定制数值格式化函数
针对时间戳、线程 ID 等高频字段实现轻量级格式化逻辑，避免进入 fmt 的完整泛型路径。

---

## 优化效果概述

经过重构后：

- 日志格式化路径显著收敛，调用栈深度减少
- 热点函数中 fmt 相关占比明显下降
- 整体吞吐能力提升，同时保持接口简洁可扩展


## Build & Run

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)

./test_performace
```

