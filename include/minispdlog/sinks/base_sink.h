#pragma once

#include "../common.h"
#include "../details/log_msg.h"
#include "../formatter.h"
#include "../pattern_formatter.h"
#include <mutex>
#include <memory>

namespace minispdlog {
namespace sinks {

// Sink 接口类(纯虚)
class sink {
public:
    virtual ~sink() = default;
    
    // 输出日志(线程安全)
    virtual void log(const details::log_msg& msg) = 0;
    
    // 刷新缓冲区
    virtual void flush() = 0;
    
    // 设置日志级别
    virtual void set_level(level log_level) = 0;
    virtual level get_level() const = 0;
    
    // 判断是否应该输出
    virtual bool should_log(level msg_level) const = 0;
    
    // Formatter 相关接口
    virtual void set_formatter(std::unique_ptr<formatter> sink_formatter) = 0;
};

// base_sink:实现了线程安全的 Sink 基类
template<typename Mutex>
class base_sink : public sink {
public:
    base_sink() 
        : level_(level::trace)
        , formatter_(std::make_unique<pattern_formatter>())  // 默认 formatter
    {}
    
    base_sink(const base_sink&) = delete;
    base_sink& operator=(const base_sink&) = delete;
    
    void log(const details::log_msg& msg) override {
        std::lock_guard<Mutex> lock(mutex_);
        sink_it_(msg); // 调用的是子类的sink_it_方法
    }
    
    void flush() override {
        std::lock_guard<Mutex> lock(mutex_);
        flush_();
    }
    
    void set_level(level log_level) override {
        std::lock_guard<Mutex> lock(mutex_);
        level_ = log_level;
    }
    
    level get_level() const override {
        std::lock_guard<Mutex> lock(mutex_);
        return level_;
    }
    
    bool should_log(level msg_level) const override {
        return msg_level >= level_;
    }
    
    void set_formatter(std::unique_ptr<formatter> sink_formatter) override {
        std::lock_guard<Mutex> lock(mutex_);
        formatter_ = std::move(sink_formatter);
    }
    
protected:
    // 子类需要实现的核心方法
    virtual void sink_it_(const details::log_msg& msg) = 0;
    virtual void flush_() = 0;
    
    // 格式化日志消息
    void format_message(const details::log_msg& msg, fmt::memory_buffer& dest) {
        formatter_->format(msg, dest);
    }
    
    mutable Mutex mutex_;
    level level_;
    std::unique_ptr<formatter> formatter_;  // 每个 sink 拥有自己的 formatter
};

// null_mutex:用于单线程版本
struct null_mutex {
    void lock() {}
    void unlock() {}
};

using sink_ptr = std::shared_ptr<sink>;

} // namespace sinks
} // namespace minispdlog