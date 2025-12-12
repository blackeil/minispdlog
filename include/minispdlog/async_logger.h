#pragma once

#include "logger.h"
#include "details/thread_pool.h"
#include <memory>

namespace minispdlog {

// 前向声明
namespace details {
class thread_pool;
}

// 溢出策略枚举
// 参考 spdlog 设计:队列满时的两种处理方式
enum class async_overflow_policy {
    block,          // 阻塞调用者,等待队列有空闲槽位(默认,保证不丢消息)
    overrun_oldest  // 立即覆盖最旧消息(非阻塞,但可能丢消息)
};

// async_logger:异步日志记录器
// 参考 spdlog 设计:
//   1. 继承 logger,重写 sink_it_() 和 flush_()
//   2. 持有 weak_ptr<thread_pool>,避免循环引用
//   3. 继承 enable_shared_from_this,用于传递 shared_ptr 到队列
//   4. 支持溢出策略
//
// 关键理解:
//   - async_logger 不直接输出日志,而是将消息 post 到队列
//   - 后台工作线程从队列中取出消息,调用 backend_sink_it_() 真正输出
//   - 使用 weak_ptr 是因为 async_msg 持有 shared_ptr<logger>,避免循环引用
class MINISPDLOG_API async_logger final 
    : public logger
    , public std::enable_shared_from_this<async_logger> 
{
    // thread_pool 需要访问 backend_sink_it_()
    friend class details::thread_pool;

public:
    // 构造函数:迭代器版本
    template<typename It>
    async_logger(
        std::string name,
        It begin,
        It end,
        std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy policy = async_overflow_policy::block
    )
        : logger(std::move(name), begin, end)
        , thread_pool_(std::move(tp))
        , overflow_policy_(policy)
    {}

    // 构造函数:单个 Sink
    async_logger(
        std::string name,
        sinks::sink_ptr single_sink,
        std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy policy = async_overflow_policy::block
    );

    // 构造函数:多个 Sink
    async_logger(
        std::string name,
        std::vector<sinks::sink_ptr> sinks,
        std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy policy = async_overflow_policy::block
    );

    ~async_logger() override = default;

    // 禁止拷贝
    async_logger(const async_logger&) = delete;
    async_logger& operator=(const async_logger&) = delete;

protected:
    // 重写 logger 的虚函数:将消息 post 到队列(非阻塞返回)
    void sink_it_(const details::log_msg& msg) override;

    // 重写 flush:向队列 post 刷新请求
    void flush_() override;

    // 后台线程调用:真正执行日志输出
    // 注意:这个方法在工作线程中执行,不是用户线程
    void backend_sink_it_(const details::log_msg& msg);

    // 后台线程调用:真正执行刷新
    void backend_flush_();

private:
    std::weak_ptr<details::thread_pool> thread_pool_;  // 线程池(弱引用)
    async_overflow_policy overflow_policy_;             // 溢出策略
};

} // namespace minispdlog