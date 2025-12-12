#pragma once

#include "common.h"
#include "level.h"
#include "sinks/base_sink.h"
#include "details/log_msg.h"
#include <fmt/format.h>
#include <vector>
#include <memory>
#include <string>
#include "minispdlog/details/thread_pool.h"

namespace minispdlog {

// logger 类:日志记录器的核心实现
// 参考 spdlog 设计:
// 1. 持有多个 sink,每次日志调用会遍历所有 sink
// 2. 支持日志级别过滤
// 3. 使用 fmt 库和变参模板提供灵活的日志接口
class MINISPDLOG_API logger {
public:
    // 构造函数
    explicit logger(std::string name);
    logger(std::string name, sinks::sink_ptr single_sink);
    logger(std::string name, std::vector<sinks::sink_ptr> sinks);
    
    virtual ~logger() = default;

    logger(const logger &other);
    logger(logger &&other);
    // 禁止拷贝
    // logger(const logger&) = delete;
    // logger& operator=(const logger&) = delete;
    
    // ========== 日志接口 ==========
    
    // 变参模板接口:支持 fmt 格式化
    // 示例: logger->info("Hello, {}!", "World")
    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        log(level::trace, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(level::debug, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(level::info, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        log(level::warn, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(level::error, fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        log(level::critical, fmt, std::forward<Args>(args)...);
    }
    
    // 核心日志方法
    template<typename... Args>
    void log(level lvl, fmt::format_string<Args...> fmt, Args&&... args) {
        if (!should_log(lvl)) {
            return;
        }
        
        // 格式化消息
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), fmt, std::forward<Args>(args)...);
        
        // 创建 log_msg
        details::log_msg msg(
            name_,
            lvl,
            string_view_t(buf.data(), buf.size())
        );
        
        // 输出到所有 sink
        sink_it_(msg);
    }
    
    // ========== Sink 管理 ==========
    
    void add_sink(sinks::sink_ptr sink);
    void remove_sink(sinks::sink_ptr sink);
    std::vector<sinks::sink_ptr>& sinks();
    const std::vector<sinks::sink_ptr>& sinks() const;
    
    // ========== 级别控制 ==========
    
    void set_level(level log_level);
    level get_level() const;
    bool should_log(level msg_level) const;
    
    // ========== 刷新 ==========
    
    void flush();
    void flush_on(level log_level);
    
    // ========== 名称 ==========
    
    const std::string& name() const;
    
protected:
    // 将消息输出到所有 sink
    virtual void sink_it_(const details::log_msg& msg);
    virtual void flush_();

    // 添加友元类声明
    friend class details::thread_pool;
    
    std::string name_;                          // Logger 名称
    std::vector<sinks::sink_ptr> sinks_;       // Sink 列表
    level level_{level::trace};                 // 日志级别
    level flush_level_{level::off};             // 自动刷新级别
};

} // namespace minispdlog