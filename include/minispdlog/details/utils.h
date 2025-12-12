#pragma once

#include "../common.h"
#include <string>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif

namespace minispdlog {
namespace details {

// 格式化时间为字符串
MINISPDLOG_API std::string format_time(
    const log_clock::time_point& tp, 
    const char* format = "%Y-%m-%d %H:%M:%S"
);

// 获取当前时间戳(毫秒)
MINISPDLOG_API int64_t get_timestamp_ms();

// 获取当前线程 ID
inline size_t get_thread_id() {
#ifdef _WIN32
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__) || defined(__APPLE__)
    return static_cast<size_t>(pthread_self());
#else
    // 通用方案(C++11)
    std::hash<std::thread::id> hasher;
    return hasher(std::this_thread::get_id());
#endif
}

// 获取线程 ID
MINISPDLOG_API size_t get_thread_id();

// 字符串工具
MINISPDLOG_API std::string& ltrim(std::string& s);
MINISPDLOG_API std::string& rtrim(std::string& s);
MINISPDLOG_API std::string& trim(std::string& s);

} // namespace details
} // namespace minispdlog