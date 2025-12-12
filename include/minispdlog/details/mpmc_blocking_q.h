#pragma once

#include "circular_q.h"
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace minispdlog {
namespace details {

// mpmc_blocking_queue: 多生产者多消费者阻塞队列
// 参考 spdlog 设计:使用 circular_q + mutex + condition_variable
//
// 特性:
//   - enqueue: 队列满时阻塞
//   - enqueue_nowait: 队列满时覆盖最旧消息
//   - dequeue_for: 队列空时带超时阻塞
//   - 线程安全
template<typename T>
class mpmc_blocking_queue {
public:
    using item_type = T;
    
    explicit mpmc_blocking_queue(size_t max_items)
        : q_(max_items)
    {}
    
    mpmc_blocking_queue(const mpmc_blocking_queue&) = delete;
    mpmc_blocking_queue& operator=(const mpmc_blocking_queue&) = delete;
    
    // 入队(阻塞模式):队列满时阻塞等待
    void enqueue(T&& item) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // 等待队列非满
            pop_cv_.wait(lock, [this] { return !this->q_.full(); });
            q_.push_back(std::move(item));
        }
        // 通知一个等待的消费者
        push_cv_.notify_one();
    }
    
    // 入队(非阻塞模式):队列满时覆盖最旧消息
    void enqueue_nowait(T&& item) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            q_.push_back(std::move(item));
        }
        push_cv_.notify_one();
    }
    
    // 出队(带超时):成功返回 true,超时返回 false
    // wait_duration: 最长等待时间
    bool dequeue_for(T& popped_item, std::chrono::milliseconds wait_duration) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 等待队列非空或超时
        if (!push_cv_.wait_for(lock, wait_duration, [this] { return !this->q_.empty(); })) {
            return false;  // 超时
        }
        
        popped_item = std::move(q_.front());
        q_.pop_front();
        
        // 通知一个等待的生产者
        pop_cv_.notify_one();
        
        return true;
    }
    
    // 获取溢出计数(被覆盖的消息数)
    size_t overrun_counter() {
        std::unique_lock<std::mutex> lock(mutex_);
        return q_.overrun_counter();
    }
    
    // 当前队列大小
    size_t size() {
        std::unique_lock<std::mutex> lock(mutex_);
        return q_.size();
    }
    
private:
    std::mutex mutex_;                      // 保护队列
    std::condition_variable push_cv_;       // 通知消费者:有新元素
    std::condition_variable pop_cv_;        // 通知生产者:有空闲槽位
    circular_q<T> q_;                       // 循环队列
};

} // namespace details
} // namespace minispdlog