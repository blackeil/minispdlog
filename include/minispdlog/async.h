#pragma once

#include "common.h"
#include "async_logger.h"
#include "registry.h"
#include "sinks/file_sink.h"
#include "sinks/rotating_file_sink.h"
#include "sinks/console_sink.h"
#include "sinks/color_console_sink.h"
#include <memory>
#include <string>

// async.h:异步日志工厂函数
// 参考 spdlog 设计:提供便捷的异步 logger 创建接口
//
// 使用方式:
//   #include "minispdlog/async.h"
//   
//   // 方式1:使用默认线程池
//   auto logger = minispdlog::async_stdout_color_mt("async_console");
//   
//   // 方式2:自定义线程池配置
//   minispdlog::init_thread_pool(16384, 2);  // 队列16384,2个线程
//   auto logger = minispdlog::async_file_mt("async_file", "log.txt");

namespace minispdlog {

// ============================================================================
// 全局线程池管理
// ============================================================================

// 初始化全局线程池(必须在创建异步 logger 之前调用)
// 如果不调用,会使用默认配置(队列8192,1线程)
inline void init_thread_pool(size_t queue_size, size_t threads_n = 1) {
    registry::instance().init_thread_pool(queue_size, threads_n);
}

// 获取全局线程池
inline std::shared_ptr<details::thread_pool> thread_pool() {
    return registry::instance().thread_pool();
}

// ============================================================================
// 异步 Logger 工厂函数
// ============================================================================

// 创建异步彩色控制台 logger(多线程安全)
// overflow_policy: 溢出策略(默认 block)
inline std::shared_ptr<async_logger> async_stdout_color_mt(
    const std::string& logger_name,
    async_overflow_policy overflow_policy = async_overflow_policy::block
) {
    auto sink = std::make_shared<sinks::color_console_sink_mt>();
    auto tp = registry::instance().thread_pool();
    auto new_logger = std::make_shared<async_logger>(
        logger_name, 
        sink, 
        tp, 
        overflow_policy
    );
    registry::instance().register_logger(new_logger);
    return new_logger;
}

// 创建异步彩色 stderr logger(多线程安全)
inline std::shared_ptr<async_logger> async_stderr_color_mt(
    const std::string& logger_name,
    async_overflow_policy overflow_policy = async_overflow_policy::block
) {
    auto sink = std::make_shared<sinks::color_stderr_sink_mt>();
    auto tp = registry::instance().thread_pool();
    auto new_logger = std::make_shared<async_logger>(
        logger_name, 
        sink, 
        tp, 
        overflow_policy
    );
    registry::instance().register_logger(new_logger);
    return new_logger;
}

// 创建异步普通控制台 logger(多线程安全)
inline std::shared_ptr<async_logger> async_stdout_mt(
    const std::string& logger_name,
    async_overflow_policy overflow_policy = async_overflow_policy::block
) {
    auto sink = std::make_shared<sinks::console_sink_mt>();
    auto tp = registry::instance().thread_pool();
    auto new_logger = std::make_shared<async_logger>(
        logger_name, 
        sink, 
        tp, 
        overflow_policy
    );
    registry::instance().register_logger(new_logger);
    return new_logger;
}

// 创建异步文件 logger(多线程安全)
inline std::shared_ptr<async_logger> async_file_mt(
    const std::string& logger_name,
    const std::string& filename,
    bool truncate = false,
    async_overflow_policy overflow_policy = async_overflow_policy::block
) {
    auto sink = std::make_shared<sinks::file_sink_mt>(filename, truncate);
    auto tp = registry::instance().thread_pool();
    auto new_logger = std::make_shared<async_logger>(
        logger_name, 
        sink, 
        tp, 
        overflow_policy
    );
    registry::instance().register_logger(new_logger);
    return new_logger;
}

// 创建异步滚动文件 logger(多线程安全)
inline std::shared_ptr<async_logger> async_rotating_logger_mt(
    const std::string& logger_name,
    const std::string& filename,
    size_t max_size,
    size_t max_files,
    async_overflow_policy overflow_policy = async_overflow_policy::block
) {
    auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
        filename, 
        max_size, 
        max_files
    );
    auto tp = registry::instance().thread_pool();
    auto new_logger = std::make_shared<async_logger>(
        logger_name, 
        sink, 
        tp, 
        overflow_policy
    );
    registry::instance().register_logger(new_logger);
    return new_logger;
}

// ============================================================================
// 高级用法:手动创建异步 logger(不自动注册)
// ============================================================================

// 手动创建异步 logger(需要自己注册)
// 适用于需要自定义 sink 组合的场景
template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<async_logger> create_async(
    const std::string& logger_name,
    async_overflow_policy overflow_policy,
    SinkArgs&&... sink_args
) {
    auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(sink_args)...);
    auto tp = registry::instance().thread_pool();
    return std::make_shared<async_logger>(
        logger_name, 
        sink, 
        tp, 
        overflow_policy
    );
}

// 使用默认溢出策略(block)
template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<async_logger> create_async(
    const std::string& logger_name,
    SinkArgs&&... sink_args
) {
    return create_async<Sink>(
        logger_name, 
        async_overflow_policy::block,
        std::forward<SinkArgs>(sink_args)...
    );
}

} // namespace minispdlog