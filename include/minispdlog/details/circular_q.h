#pragma once

#include <vector>
#include <cassert>

namespace minispdlog {
namespace details {

// circular_q: 循环队列(环形缓冲区)
// 用于实现固定大小的高效队列,避免动态内存分配
// 参考 spdlog 设计:预分配所有槽位,使用 head/tail 指针管理
template<typename T>
class circular_q {
public:
    using value_type = T;
    
    // 构造函数:预分配 max_items 个槽位
    explicit circular_q(size_t max_items)
        : max_items_(max_items + 1)  // +1 for distinguishing full/empty
        , v_(max_items_)
        , head_(0)
        , tail_(0)
        , overrun_counter_(0)
    {}
    
    circular_q(const circular_q&) = delete;
    circular_q& operator=(const circular_q&) = delete;
    
    // 在队尾添加元素
    void push_back(T&& item) {
        v_[tail_] = std::move(item);
        tail_ = (tail_ + 1) % max_items_;
        
        // 如果队列满了,覆盖最旧的元素
        if (tail_ == head_) {
            head_ = (head_ + 1) % max_items_;
            ++overrun_counter_;
        }
    }
    
    // 获取队首元素
    const T& front() const {
        return v_[head_];
    }
    
    T& front() {
        return v_[head_];
    }
    
    // 移除队首元素
    void pop_front() {
        head_ = (head_ + 1) % max_items_;
    }
    
    // 队列是否为空
    bool empty() const {
        return head_ == tail_;
    }
    
    // 队列是否已满
    bool full() const {
        return ((tail_ + 1) % max_items_) == head_;
    }
    
    // 当前元素数量
    size_t size() const {
        if (tail_ >= head_) {
            return tail_ - head_;
        } else {
            return max_items_ - (head_ - tail_);
        }
    }
    
    // 最大容量(实际可用容量)
    size_t capacity() const {
        return max_items_ - 1;
    }
    
    // 溢出次数(覆盖旧元素的次数)
    size_t overrun_counter() const {
        return overrun_counter_;
    }
    
private:
    size_t max_items_;          // 数组大小(capacity + 1)
    std::vector<T> v_;          // 存储数据
    size_t head_;               // 队首索引
    size_t tail_;               // 队尾索引
    size_t overrun_counter_;    // 溢出计数
};

} // namespace details
} // namespace minispdlog