#pragma once

#include "base_sink.h"
#include <iostream>
#include <mutex>

namespace minispdlog {
namespace sinks {

// 控制台 Sink
template<typename ConsoleMutex>
class console_sink : public base_sink<ConsoleMutex> {
public:
    console_sink() = default;
    ~console_sink() override = default;
    
protected:
    void sink_it_(const details::log_msg& msg) override {
        fmt::memory_buffer formatted;
        this->format_message(msg, formatted);
        
        // 输出到 stdout
        std::cout.write(formatted.data(), formatted.size());
    }
    
    void flush_() override {
        std::cout << std::flush;
    }
};

// 多线程安全版本(使用 std::mutex)
using console_sink_mt = console_sink<std::mutex>;

// 单线程版本(使用 null_mutex,性能更高)
using console_sink_st = console_sink<null_mutex>;

// stderr 版本
template<typename ConsoleMutex>
class stderr_sink : public base_sink<ConsoleMutex> {
public:
    stderr_sink() = default;
    ~stderr_sink() override = default;
    
protected:
    void sink_it_(const details::log_msg& msg) override {
        fmt::memory_buffer formatted;
        this->format_message(msg, formatted);
        
        std::cerr.write(formatted.data(), formatted.size());
    }
    
    void flush_() override {
        std::cerr << std::flush;
    }
};

using stderr_sink_mt = stderr_sink<std::mutex>;
using stderr_sink_st = stderr_sink<null_mutex>;

} // namespace sinks
} // namespace minispdlog