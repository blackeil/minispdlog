#pragma once

#include "common.h"
#include "level.h"
#include "logger.h"
#include "registry.h"
#include "sinks/console_sink.h"
#include "sinks/color_console_sink.h"
#include "sinks/file_sink.h"
#include "sinks/rotating_file_sink.h"
#include <fmt/format.h>
#include <memory>
#include <string>

// minispdlog.h:主头文件 - 同步日志功能
// 参考 spdlog 设计:
//   - _mt 后缀:多线程安全版本(使用 std::mutex)
//   - _st 后缀:单线程版本(使用 null_mutex,性能更高)
//
// 使用建议:
//   - 多线程环境:使用 _mt 版本
//   - 单线程环境:使用 _st 版本(性能更好)
//   - 异步日志:包含 "minispdlog/async.h"

namespace minispdlog {

// ============================================================================
// Registry 便捷访问
// ============================================================================

// 获取 logger(如果不存在返回 nullptr)
inline std::shared_ptr<logger> get(const std::string& name) {
    return registry::instance().get(name);
}

// 注册 logger
inline void register_logger(std::shared_ptr<logger> logger) {
    registry::instance().register_logger(std::move(logger));
}

// 移除 logger
inline void drop(const std::string& name) {
    registry::instance().drop(name);
}

// 移除所有 logger
inline void drop_all() {
    registry::instance().drop_all();
}

// 获取默认 logger
inline std::shared_ptr<logger> default_logger() {
    return registry::instance().default_logger();
}

// 设置默认 logger
inline void set_default_logger(std::shared_ptr<logger> new_default_logger) {
    registry::instance().set_default_logger(std::move(new_default_logger));
}

// 设置所有 logger 的级别
inline void set_level(level log_level) {
    registry::instance().set_level(log_level);
}

// 刷新所有 logger
inline void flush_all() {
    registry::instance().flush_all();
}

// ============================================================================
// 工厂函数:快速创建并注册 logger (多线程安全版本 _mt)
// ============================================================================

// 创建彩色控制台 logger(多线程安全)
inline std::shared_ptr<logger> stdout_color_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::color_console_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建彩色 stderr logger(多线程安全)
inline std::shared_ptr<logger> stderr_color_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::color_stderr_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建普通控制台 logger(多线程安全)
inline std::shared_ptr<logger> stdout_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::console_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建普通 stderr logger(多线程安全)
inline std::shared_ptr<logger> stderr_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::stderr_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建文件 logger(多线程安全)
inline std::shared_ptr<logger> basic_logger_mt(
    const std::string& logger_name,
    const std::string& filename,
    bool truncate = false
) {
    auto sink = std::make_shared<sinks::file_sink_mt>(filename, truncate);
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建滚动文件 logger(多线程安全)
inline std::shared_ptr<logger> rotating_logger_mt(
    const std::string& logger_name,
    const std::string& filename,
    size_t max_size,
    size_t max_files
) {
    auto sink = std::make_shared<sinks::rotating_file_sink_mt>(filename, max_size, max_files);
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// ============================================================================
// 工厂函数:单线程版本 (_st) - 性能更高,但不支持多线程
// ============================================================================

// 创建彩色控制台 logger(单线程)
inline std::shared_ptr<logger> stdout_color_st(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::color_console_sink_st>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建彩色 stderr logger(单线程)
inline std::shared_ptr<logger> stderr_color_st(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::color_stderr_sink_st>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建普通控制台 logger(单线程)
inline std::shared_ptr<logger> stdout_st(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::console_sink_st>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建普通 stderr logger(单线程)
inline std::shared_ptr<logger> stderr_st(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::stderr_sink_st>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建文件 logger(单线程)
inline std::shared_ptr<logger> basic_logger_st(
    const std::string& logger_name,
    const std::string& filename,
    bool truncate = false
) {
    auto sink = std::make_shared<sinks::file_sink_st>(filename, truncate);
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建滚动文件 logger(单线程)
inline std::shared_ptr<logger> rotating_logger_st(
    const std::string& logger_name,
    const std::string& filename,
    size_t max_size,
    size_t max_files
) {
    auto sink = std::make_shared<sinks::rotating_file_sink_st>(filename, max_size, max_files);
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// ============================================================================
// 全局日志接口:直接使用默认 logger
// ============================================================================

template<typename... Args>
inline void trace(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->trace(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void debug(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->debug(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void info(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->info(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void warn(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->warn(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void error(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->error(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void critical(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->critical(fmt, std::forward<Args>(args)...);
}

} // namespace minispdlog

#if 0
#pragma once

#include "common.h"
#include "level.h"
#include "logger.h"
#include "registry.h"
#include "sinks/console_sink.h"
#include "sinks/color_console_sink.h"
#include "sinks/file_sink.h"
#include "sinks/rotating_file_sink.h"
#include <fmt/format.h>
#include <memory>
#include <string>

namespace minispdlog {

// ============================================================================
// Registry 便捷访问
// ============================================================================

// 获取 logger(如果不存在返回 nullptr)
inline std::shared_ptr<logger> get(const std::string& name) {
    return registry::instance().get(name);
}

// 注册 logger
inline void register_logger(std::shared_ptr<logger> logger) {
    registry::instance().register_logger(std::move(logger));
}

// 移除 logger
inline void drop(const std::string& name) {
    registry::instance().drop(name);
}

// 移除所有 logger
inline void drop_all() {
    registry::instance().drop_all();
}

// 获取默认 logger
inline std::shared_ptr<logger> default_logger() {
    return registry::instance().default_logger();
}

// 设置默认 logger
inline void set_default_logger(std::shared_ptr<logger> new_default_logger) {
    registry::instance().set_default_logger(std::move(new_default_logger));
}

// 设置所有 logger 的级别
inline void set_level(level log_level) {
    registry::instance().set_level(log_level);
}

// 刷新所有 logger
inline void flush_all() {
    registry::instance().flush_all();
}

// ============================================================================
// 工厂函数:快速创建并注册 logger
// ============================================================================

// 创建彩色控制台 logger(多线程安全)
inline std::shared_ptr<logger> stdout_color_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::color_console_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建彩色 stderr logger(多线程安全)
inline std::shared_ptr<logger> stderr_color_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::color_stderr_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建普通控制台 logger(多线程安全)
inline std::shared_ptr<logger> stdout_mt(const std::string& logger_name) {
    auto sink = std::make_shared<sinks::console_sink_mt>();
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建文件 logger(多线程安全)
inline std::shared_ptr<logger> basic_logger_mt(
    const std::string& logger_name,
    const std::string& filename,
    bool truncate = false
) {
    auto sink = std::make_shared<sinks::file_sink_mt>(filename, truncate);
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// 创建滚动文件 logger(多线程安全)
// max_size: 单文件最大字节数(如 1048576 * 5 = 5MB)
// max_files: 最多保留文件数(如 3)
inline std::shared_ptr<logger> rotating_logger_mt(
    const std::string& logger_name,
    const std::string& filename,
    size_t max_size,
    size_t max_files
) {
    auto sink = std::make_shared<sinks::rotating_file_sink_mt>(filename, max_size, max_files);
    auto new_logger = std::make_shared<logger>(logger_name, sink);
    register_logger(new_logger);
    return new_logger;
}

// ============================================================================
// 全局日志接口:直接使用默认 logger
// ============================================================================

template<typename... Args>
inline void trace(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->trace(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void debug(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->debug(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void info(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->info(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void warn(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->warn(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void error(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->error(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void critical(fmt::format_string<Args...> fmt, Args&&... args) {
    default_logger()->critical(fmt, std::forward<Args>(args)...);
}

} // namespace minispdlog

#endif