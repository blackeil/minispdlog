#include "minispdlog/details/thread_pool.h"
#include "minispdlog/async_logger.h"
#include <iostream>

namespace minispdlog {
namespace details {

thread_pool::thread_pool(size_t queue_size, size_t threads_n)
    : q_(queue_size)
{
    if (threads_n == 0 || threads_n > 1000) {
        throw std::invalid_argument("thread_pool: threads_n must be 1-1000");
    }
    
    // 创建工作线程
    for (size_t i = 0; i < threads_n; ++i) {
        threads_.emplace_back([this] { this->worker_loop_(); });
    }
}

thread_pool::~thread_pool() {
    try {
        // 为每个工作线程发送终止消息
        for (size_t i = 0; i < threads_.size(); ++i) {
            async_msg terminate_msg(async_msg_type::terminate);
            q_.enqueue(std::move(terminate_msg));
        }
        
        // 等待所有线程结束
        for (auto& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    } catch (...) {
        // 析构函数不应抛出异常
    }
}

// 投递日志消息(阻塞模式)
void thread_pool::post_log(std::shared_ptr<async_logger> &&async_logger_ptr, const log_msg& msg) {
    async_msg async_m(async_msg_type::log, std::move(async_logger_ptr), msg);
    q_.enqueue(std::move(async_m));
}

// 投递日志消息(非阻塞模式,队列满时覆盖)
void thread_pool::post_log_nowait(std::shared_ptr<async_logger> &&async_logger_ptr, const log_msg& msg) {
    async_msg async_m(async_msg_type::log, std::move(async_logger_ptr), msg);
    q_.enqueue_nowait(std::move(async_m));
}

// 投递刷新请求
void thread_pool::post_flush(std::shared_ptr<async_logger> &&async_logger_ptr) {
    async_msg flush_msg(async_msg_type::flush, std::move(async_logger_ptr));
    q_.enqueue(std::move(flush_msg));
}

size_t thread_pool::overrun_counter() {
    return q_.overrun_counter();
}

void thread_pool::worker_loop_() {
    while (process_next_msg_()) {
        // 继续处理消息
    }
}

bool thread_pool::process_next_msg_() {
    async_msg incoming_async_msg;
    
    // 从队列中取出消息(带超时)
    if (!q_.dequeue_for(incoming_async_msg, std::chrono::seconds(10))) {
        return true;  // 超时,继续等待
    }
    
    switch (incoming_async_msg.msg_type) {
        case async_msg_type::log: {
            // 处理日志消息
            // 关键:这里可以直接调用 async_logger 的 backend_sink_it_()
            if (incoming_async_msg.worker_ptr) {
                incoming_async_msg.worker_ptr->backend_sink_it_(incoming_async_msg);
            }
            return true;
        }
        
        case async_msg_type::flush: {
            // 处理刷新请求
            if (incoming_async_msg.worker_ptr) {
                incoming_async_msg.worker_ptr->backend_flush_();
            }
            return true;
        }
        
        case async_msg_type::terminate: {
            // 收到终止消息,退出循环
            return false;
        }
    }
    
    return true;
}

} // namespace details
} // namespace minispdlog


#if 0
#include "minispdlog/details/thread_pool.h"
#include "minispdlog/logger.h"

namespace minispdlog {
namespace details {

thread_pool::thread_pool(size_t queue_size, size_t threads_n)
    : q_(queue_size)
{
    if (threads_n == 0 || threads_n > 1000) {
        throw std::invalid_argument("thread_pool: threads_n must be 1-1000");
    }
    
    // 创建工作线程
    for (size_t i = 0; i < threads_n; ++i) {
        threads_.emplace_back([this] { this->worker_loop_(); });
    }
}

thread_pool::~thread_pool() {
    try {
        // 为每个工作线程发送终止消息
        for (size_t i = 0; i < threads_.size(); ++i) {
            async_msg terminate_msg;
            terminate_msg.msg_type = async_msg_type::terminate;
            q_.enqueue(std::move(terminate_msg));
        }
        
        // 等待所有线程结束
        for (auto& t : threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    } catch (...) {
        // 析构函数不应抛出异常
    }
}

void thread_pool::post_log(std::shared_ptr<async_logger> &&logger_ptr, const log_msg& msg) {
    async_msg async_m(async_msg_type::log, std::move(logger_ptr), msg);
    q_.enqueue(std::move(async_m));
}

void thread_pool::post_log_nowait(std::shared_ptr<async_logger> &&logger_ptr, const log_msg& msg) {
    async_msg async_m(async_msg_type::log, std::move(logger_ptr), msg);
    q_.enqueue_nowait(std::move(async_m));
}

void thread_pool::post_flush(std::shared_ptr<async_logger> &&logger_ptr) {
    async_msg flush_msg(async_msg_type::flush, std::move(logger_ptr));
    q_.enqueue(std::move(flush_msg));
}

size_t thread_pool::overrun_counter() {
    return q_.overrun_counter();
}

void thread_pool::worker_loop_() {
    while (process_next_msg_()) {
        // 继续处理消息
    }
}
#if 1
bool thread_pool::process_next_msg_() {
    async_msg incoming_async_msg;
    
    if (!q_.dequeue_for(incoming_async_msg, std::chrono::seconds(10))) {
        return true;  // 超时继续
    }
    
    switch (incoming_async_msg.msg_type) {
        case async_msg_type::log: {
            // 直接使用 worker_ptr (shared_ptr)
            if (incoming_async_msg.worker_ptr) {
                incoming_async_msg.worker_ptr->sink_it_(incoming_async_msg);
                // async_msg 继承自 log_msg_buffer，可直接传入
            }
            return true;
        }
        
        case async_msg_type::flush: {
            if (incoming_async_msg.worker_ptr) {
                incoming_async_msg.worker_ptr->flush();
            }
            return true;
        }
        
        case async_msg_type::terminate: {
            return false;
        }
    }
    
    return true;
}
#endif

} // namespace details
} // namespace minispdlog

#endif

#if 0
void thread_pool::post_flush(std::shared_ptr<logger> &&logger_ptr) {
    async_msg flush_msg;
    flush_msg.msg_type = async_msg_type::flush;
    flush_msg.logger_ptr = logger_ptr;
    q_.enqueue(std::move(flush_msg));
}
#endif

#if 0
bool thread_pool::process_next_msg_() {
    async_msg incoming_async_msg;
    
    // 从队列取消息(最多等待10秒)
    bool dequeued = q_.dequeue_for(incoming_async_msg, std::chrono::seconds(10));
    
    if (!dequeued) {
        // 超时,继续循环
        return true;
    }
    
    switch (incoming_async_msg.msg_type) {
        case async_msg_type::log: {
            // 尝试获取 logger(可能已经被销毁)
            auto logger_ptr = incoming_async_msg.logger_ptr.lock();
            if (!logger_ptr) {
                // Logger 已销毁,跳过此消息
                return true;
            }
            
            // 调用 logger 的后端方法
            auto msg = incoming_async_msg.to_log_msg();
            logger_ptr->sink_it_(msg);
            return true;
        }
        
        case async_msg_type::flush: {
            auto logger_ptr = incoming_async_msg.logger_ptr.lock();
            if (logger_ptr) {
                logger_ptr->flush();
            }
            return true;
        }
        
        case async_msg_type::terminate: {
            // 收到终止信号,退出循环
            return false;
        }
    }
    
    return true;
}
#endif