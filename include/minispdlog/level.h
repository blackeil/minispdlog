#pragma once

#include "common.h"
#include <string>

namespace minispdlog {

// 日志级别枚举
enum class level {
    trace = 0,      // 最详细的调试信息
    debug = 1,      // 调试信息
    info = 2,       // 普通信息
    warn = 3,       // 警告信息
    error = 4,      // 错误信息
    critical = 5,   // 严重错误
    off = 6         // 关闭日志
};

// 级别转字符串
MINISPDLOG_API const char* level_to_string(level lvl) noexcept;

// 级别转短字符串(用于格式化)
MINISPDLOG_API const char* level_to_short_string(level lvl) noexcept;

// 字符串转级别
MINISPDLOG_API level string_to_level(const std::string& str);

// 级别比较函数
inline bool should_log(level logger_level, level msg_level) noexcept {
    return msg_level >= logger_level;
}

} // namespace minispdlog