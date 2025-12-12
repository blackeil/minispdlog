#include "minispdlog/async_logger.h"
#include <stdexcept>

namespace minispdlog {

async_logger::async_logger(
    std::string name,
    sinks::sink_ptr single_sink,
    std::weak_ptr<details::thread_pool> tp,
    async_overflow_policy policy
)
    : logger(std::move(name), std::move(single_sink))
    , thread_pool_(std::move(tp))
    , overflow_policy_(policy)
{}

async_logger::async_logger(
    std::string name,
    std::vector<sinks::sink_ptr> sinks,
    std::weak_ptr<details::thread_pool> tp,
    async_overflow_policy policy
)
    : logger(std::move(name), std::move(sinks))
    , thread_pool_(std::move(tp))
    , overflow_policy_(policy)
{}

// sink_it_:用户线程调用
// 关键:这个方法会立即返回,不会阻塞太久(除非队列满且策略是 block)
void async_logger::sink_it_(const details::log_msg& msg) {
    // 尝试获取线程池的 shared_ptr
    // weak_ptr::lock() 是线程安全的
    if (auto pool_ptr = thread_pool_.lock()) {
        // 根据溢出策略选择 post 方式
        if (overflow_policy_ == async_overflow_policy::block) {
            // 阻塞模式:队列满时等待
            pool_ptr->post_log(shared_from_this(), msg);
        } else {
            // 覆盖模式:队列满时覆盖最旧消息
            pool_ptr->post_log_nowait(shared_from_this(), msg);
        }
    } else {
        // 线程池已销毁,抛出异常
        throw std::runtime_error(
            "async_logger::sink_it_: thread pool doesn't exist anymore"
        );
    }
}

// flush:用户线程调用
// 向队列 post 刷新请求,后台线程会处理
void async_logger::flush_() {
    if (auto pool_ptr = thread_pool_.lock()) {
        pool_ptr->post_flush(shared_from_this());
    } else {
        throw std::runtime_error(
            "async_logger::flush: thread pool doesn't exist anymore"
        );
    }
}

// backend_sink_it_:后台线程调用
// 这是真正执行日志输出的地方
void async_logger::backend_sink_it_(const details::log_msg& msg) {
    // 遍历所有 sink,执行输出
    for (auto& sink : sinks_) {
        if (sink->should_log(msg.lvl)) {
            sink->log(msg);
        }
    }
    
    // 检查是否需要自动刷新
    if (msg.lvl >= flush_level_) {
        backend_flush_();
    }
}

// backend_flush_:后台线程调用
// 刷新所有 sink
void async_logger::backend_flush_() {
    for (auto& sink : sinks_) {
        sink->flush();
    }
}

} // namespace minispdlog