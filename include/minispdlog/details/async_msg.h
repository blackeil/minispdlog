#pragma once

#include "log_msg.h"
#include <memory>
#include <string>
#include <iostream>
namespace minispdlog {

// 前向声明
class async_logger;

namespace details {

// 异步消息类型
enum class async_msg_type {
    log,        // 普通日志消息
    flush,      // 刷新请求
    terminate   // 终止线程池
};

// log_msg_buffer: 带缓冲的日志消息
// 参考 spdlog 设计:继承 log_msg 并深拷贝 payload
struct log_msg_buffer : log_msg {
    std::string buffer;  // 存储payload的深拷贝
    //std::string logger_name_buf; // logger_name 的深拷贝

    log_msg_buffer() = default;
    
    // 从 log_msg 构造(深拷贝)
    explicit log_msg_buffer(const log_msg& msg)
        : log_msg(msg)
        , buffer(msg.payload.data(), msg.payload.size())
       // , logger_name_buf(msg.logger_name.data(), msg.logger_name.size())
    {
        // 更新 payload 指向 buffer
         payload = string_view_t(buffer.data(), buffer.size());
    }

// ✅ 添加移动构造函数
    log_msg_buffer(log_msg_buffer&& other) noexcept
        : log_msg(other)  // 拷贝 log_msg 部分
        , buffer(std::move(other.buffer))  // 移动 buffer
    {
        // 关键：更新 payload 指向新的 buffer
        payload = string_view_t(buffer.data(), buffer.size());
    }
    
    // ✅ 添加移动赋值函数
    log_msg_buffer& operator=(log_msg_buffer&& other) noexcept {
        if (this != &other) {
            log_msg::operator=(other);  // 拷贝 log_msg 部分
            buffer = std::move(other.buffer);  // 移动 buffer
            // 关键：更新 payload 指向新的 buffer
            payload = string_view_t(buffer.data(), buffer.size());
        }
        return *this;
    }

};

// async_msg: 异步日志消息
// 参考 spdlog 设计:继承 log_msg_buffer + async_logger 的 shared_ptr
//
// 关键修正:
//   - worker_ptr 的类型改为 std::shared_ptr<async_logger> (而不是 logger)
//   - 这样 thread_pool 可以调用 async_logger 的 backend_sink_it_()
using async_logger_ptr = std::shared_ptr<minispdlog::async_logger>;

struct async_msg : log_msg_buffer {
    async_msg_type msg_type{async_msg_type::log};
    
    // Logger 的 shared_ptr (注意:这里是 async_logger,不是 logger)
    // 原因:thread_pool 需要调用 async_logger::backend_sink_it_()
    async_logger_ptr worker_ptr;
    
    // 默认构造
    async_msg() = default;
    ~async_msg() = default;

    // ✅ 添加移动构造函数
    async_msg(async_msg&& other) noexcept
        : log_msg_buffer(std::move(other))  // 调用父类移动构造
        , msg_type(other.msg_type)
        , worker_ptr(std::move(other.worker_ptr))
    {}
    
    // ✅ 添加移动赋值函数
    async_msg& operator=(async_msg&& other) noexcept {
        if (this != &other) {
            log_msg_buffer::operator=(std::move(other));  // 调用父类移动赋值
            msg_type = other.msg_type;
            worker_ptr = std::move(other.worker_ptr);
        }
        return *this;
    }

    // should only be moved in or out of the queue..
    async_msg(const async_msg&) = delete;
    
    // 从 log_msg 构造
    async_msg(async_msg_type type, async_logger_ptr &&worker, const log_msg& msg)
        : log_msg_buffer(msg)
        , msg_type(type)
        , worker_ptr(std::move(worker))
    {}

    async_msg(async_msg_type the_type, async_logger_ptr &&worker)
        : log_msg_buffer{}
        , msg_type{the_type}
        , worker_ptr{std::move(worker)}
    {}

    explicit async_msg(async_msg_type the_type)
        : async_msg{the_type, nullptr}
    {}
};

} // namespace details
} // namespace minispdlog

#if 0
#pragma once

#include "log_msg.h"
#include <memory>
#include <string>

namespace minispdlog {

// 前向声明
class logger;

namespace details {

// 异步消息类型
enum class async_msg_type {
    log,        // 普通日志消息
    flush,      // 刷新请求
    terminate   // 终止线程池
};

// log_msg_buffer: 带缓冲的日志消息
// 参考 spdlog 设计:继承 log_msg 并深拷贝 payload
struct log_msg_buffer : log_msg {
    std::string buffer;  // 存储payload的深拷贝
    
    log_msg_buffer() = default;
    
    // 从 log_msg 构造(深拷贝)
    explicit log_msg_buffer(const log_msg& msg)
        : log_msg(msg)
        , buffer(msg.payload.data(), msg.payload.size())
    {
        // 更新 payload 指向 buffer
        payload = string_view_t(buffer);
    }
};

// async_msg: 异步日志消息
// 参考 spdlog 设计:继承 log_msg_buffer + logger 的 shared_ptr
//
// 关键设计:
//   - 继承 log_msg_buffer 自动处理消息深拷贝
//   - 使用 shared_ptr<logger> 确保 logger 在队列中的消息处理完前不被销毁
//   - 支持多种消息类型
struct async_msg : log_msg_buffer {
    async_msg_type msg_type{async_msg_type::log};
    
    // Logger 的 shared_ptr
    // 注意:这里使用 shared_ptr 而非 weak_ptr
    // 原因:队列中的消息需要确保 logger 存活直到消息被处理
    std::shared_ptr<logger> worker_ptr;
    
    // 默认构造
    async_msg() = default;
    ~async_msg() = default;

    // should only be moved in or out of the queue..
    async_msg(const async_msg &) = delete;
    
    // 从 log_msg 构造
    async_msg(async_msg_type type, std::shared_ptr<logger> &&worker, const log_msg& msg)
        : log_msg_buffer(msg)
        , msg_type(type)
        , worker_ptr(std::move(worker))
    {}

    async_msg(async_msg_type the_type,std::shared_ptr<logger> &&worker)
        : log_msg_buffer{},
          msg_type{the_type},
          worker_ptr{std::move(worker)} {}

    explicit async_msg(async_msg_type the_type)
        : async_msg{the_type,nullptr} {}
};

} // namespace details
} // namespace minispdlog

#endif