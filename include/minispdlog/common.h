#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <chrono>

namespace minispdlog {

// 版本信息
constexpr const char* VERSION = "0.1.0";

// 平台相关定义
#ifdef _WIN32
    #define MINISPDLOG_WINDOWS
#elif defined(__linux__)
    #define MINISPDLOG_LINUX
#elif defined(__APPLE__)
    #define MINISPDLOG_MACOS
#endif

// 导出符号定义(为未来动态库做准备)
#if defined(_WIN32) && defined(MINISPDLOG_SHARED)
    #ifdef MINISPDLOG_BUILD
        #define MINISPDLOG_API __declspec(dllexport)
    #else
        #define MINISPDLOG_API __declspec(dllimport)
    #endif
#else
    #define MINISPDLOG_API
#endif

// 便捷类型别名
//using string_view_t = std::string;  // C++11 兼容,后续可升级为 std::string_view
using string_view_t = std::string_view;

// 时钟类型定义(参考 spdlog 设计)
using log_clock = std::chrono::system_clock;


} // namespace minispdlog