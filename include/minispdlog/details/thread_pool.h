#pragma once

#include "../common.h"
#include "mpmc_blocking_q.h"
#include "async_msg.h"
#include <thread>
#include <vector>
#include <functional>
#include <memory>

namespace minispdlog {

// 前向声明
class async_logger;

namespace details {

// thread_pool: 异步日志的线程池
// 参考 spdlog 设计:管理工作线程 + MPMC 队列
//
// 关键修正:
//   - post_log/post_flush 接受 std::shared_ptr<async_logger> (而不是 logger)
//   - 这样可以直接调用 async_logger 的 backend_sink_it_() 方法
class MINISPDLOG_API thread_pool {
public:
    using item_type = async_msg;
    using q_type = mpmc_blocking_queue<item_type>;
    
    // 构造函数
    // queue_size: 队列容量
    // threads_n: 工作线程数量
    thread_pool(size_t queue_size, size_t threads_n);
    
    // 禁止拷贝
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    
    ~thread_pool();
    
    // 投递日志消息(阻塞模式)
    // 注意:参数类型是 std::shared_ptr<async_logger> 而不是 logger
    void post_log(std::shared_ptr<async_logger> &&async_logger_ptr, const log_msg& msg);
    
    // 投递日志消息(非阻塞模式,队列满时覆盖)
    void post_log_nowait(std::shared_ptr<async_logger> &&async_logger_ptr, const log_msg& msg);
    
    // 投递刷新请求
    void post_flush(std::shared_ptr<async_logger> &&async_logger_ptr);
    
    // 获取溢出计数
    size_t overrun_counter();
    
private:
    // 工作线程主循环
    void worker_loop_();
    
    // 处理下一条消息(返回 false 表示应该退出)
    bool process_next_msg_();
    
    q_type q_;                          // MPMC 队列
    std::vector<std::thread> threads_;  // 工作线程
};

} // namespace details
} // namespace minispdlog

#if 0
#pragma once

#include "../common.h"
#include "mpmc_blocking_q.h"
#include "async_msg.h"
#include <thread>
#include <vector>
#include <functional>
#include "../async_logger.h"

namespace minispdlog {
namespace details {

// thread_pool: 异步日志的线程池
// 参考 spdlog 设计:管理工作线程 + MPMC 队列
//
// 特性:
//   - 创建指定数量的工作线程
//   - 持有 MPMC 队列用于消息传递
//   - 支持阻塞/非阻塞两种 post 模式
//   - 支持优雅关闭
class MINISPDLOG_API thread_pool {
public:
    using item_type = async_msg;
    using q_type = mpmc_blocking_queue<item_type>;
    
    // 构造函数
    // queue_size: 队列容量
    // threads_n: 工作线程数量
    thread_pool(size_t queue_size, size_t threads_n);
    
    // 禁止拷贝
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    
    ~thread_pool();
    
    // 投递日志消息(阻塞模式)
    void post_log(std::shared_ptr<async_logger> &&logger_ptr, const log_msg& msg);
    
    // 投递日志消息(非阻塞模式,队列满时覆盖)
    void post_log_nowait(std::shared_ptr<async_logger> &&logger_ptr, const log_msg& msg);
    
    // 投递刷新请求
    void post_flush(std::shared_ptr<async_logger> &&logger_ptr);
    
    // 获取溢出计数 std::shared_ptr<spdlog::async_logger>
    size_t overrun_counter();
    
private:
    // 工作线程主循环
    void worker_loop_();
    
    // 处理下一条消息(返回 false 表示应该退出)
    bool process_next_msg_();
    
    q_type q_;                          // MPMC 队列
    std::vector<std::thread> threads_;  // 工作线程
};

} // namespace details
} // namespace minispdlog

#endif