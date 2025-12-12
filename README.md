# MiniSpdlog

MiniSpdlog 是一个从零实现的 轻量级、高可读性、高扩展性 的 C++ 日志库，核心思想参考成熟日志框架的设计理念，并结合自身需求进行结构化重构。项目旨在构建一个 模块化、易维护、可拓展 的日志组件，用于学习日志系统的底层机制，同时作为实际工程项目的可用基础设施。

该库覆盖一个日志系统的关键组成部分，包括：日志器（Logger）、输出端（Sink）、格式化器（Formatter）、日志级别控制、线程安全保障等。整体实现保持简洁，同时保留可扩展的框架结构，可作为进一步优化（异步日志、多 Sink、性能增强）的基础。

---

## 核心功能

1. 多级日志支持：Debug / Info / Warn / Error 等标准日志等级。
2. 模块化设计：Logger、Sink、Formatter 完全解耦，便于扩展新的输出端与格式化方式。
3. 可配置输出策略：控制台输出（Console Sink）、文件输出(File Sink）。
4. 格式化输出：基于互斥锁实现基本的同步日志写入，支持并发场景。
5. 轻量易用的 API：简洁的接口设计，使日志输出调用方式清晰流畅。

---

MiniSpdlog 采用分层结构设计，以便于扩展和维护。

### 1. Logger
负责：管理日志级别过滤逻辑、调用 Formatter 对消息进行格式化、将日志分发给多个 Sink。

### 2. Sink
输出端机制的核心抽象，支持：  
- ConsoleSink：输出到 stdout  
- FileSink：输出到文件  
- 可扩展更多 Sink（如滚动文件、网络输出等）

### 3. Formatter
用于将日志记录格式化为最终输出字符串，可自定义格式内容，如时间戳、线程 ID、日志等级等。

### 4. Level Filtering
Logger 与 Sink 均支持日志级别过滤，实现更灵活的控制策略。

---

## 项目架构

```
minispdlog/
│── include/
│   ├── logger.h
│   ├── sink.h
│   ├── formatter.h
│   └── level.h
│
│── src/
│   ├── logger.cpp
│   ├── sink.cpp
│   ├── formatter.cpp
│   └── file_sink.cpp
│
│── examples/
│   └── basic_usage.cpp
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

## Build & Run

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)

./test_performace
```

