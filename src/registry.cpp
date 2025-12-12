#include "minispdlog/registry.h"
#include "minispdlog/sinks/color_console_sink.h"
#include "minispdlog/details/thread_pool.h"
#include <stdexcept>

namespace minispdlog {

registry::registry() {
    // 创建默认 logger(彩色控制台输出)
    auto console_sink = std::make_shared<sinks::color_console_sink_mt>();
    default_logger_ = std::make_shared<logger>("", console_sink);
    default_logger_->set_level(level::info);  // 默认级别 info
    
    // 注意:线程池延迟初始化,首次调用 thread_pool() 时才创建
}

registry& registry::instance() {
    // C++11 保证局部静态变量的线程安全初始化
    static registry s_instance;
    return s_instance;
}

void registry::register_logger(std::shared_ptr<logger> new_logger) {
    std::lock_guard<std::mutex> lock(mutex_);
    throw_if_exists_(new_logger->name());
    loggers_[new_logger->name()] = std::move(new_logger);
}

std::shared_ptr<logger> registry::get(const std::string& logger_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loggers_.find(logger_name);
    if (it != loggers_.end()) {
        return it->second;
    }
    return nullptr;  // 未找到返回 nullptr
}

void registry::drop(const std::string& logger_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto is_default_logger = default_logger_ && default_logger_->name() == logger_name;
    loggers_.erase(logger_name);
    if (is_default_logger) {
        default_logger_.reset();
    }
}

void registry::drop_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.clear();
    default_logger_.reset();
}

std::shared_ptr<logger> registry::default_logger() {
    std::lock_guard<std::mutex> lock(mutex_);
    return default_logger_;
}

void registry::set_default_logger(std::shared_ptr<logger> new_default_logger) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (new_default_logger != nullptr) {
        loggers_[new_default_logger->name()] = new_default_logger; 
    }
    default_logger_ = std::move(new_default_logger);
}

void registry::set_level(level log_level) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 设置默认 logger 的级别
    default_logger_->set_level(log_level);
    
    // 设置所有注册 logger 的级别
    for (auto& pair : loggers_) {
        pair.second->set_level(log_level);
    }
}

void registry::flush_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    default_logger_->flush();
    
    for (auto& pair : loggers_) {
        pair.second->flush();
    }
}

void registry::throw_if_exists_(const std::string& logger_name) {
    if (loggers_.find(logger_name) != loggers_.end()) {
        throw std::runtime_error("Logger with name '" + logger_name + "' already exists");
    }
}

// ========== 线程池管理 ==========

void registry::init_thread_pool(size_t queue_size, size_t threads_n) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 创建新的线程池(会销毁旧的)
    thread_pool_ = std::make_shared<details::thread_pool>(queue_size, threads_n);
}

std::shared_ptr<details::thread_pool> registry::thread_pool() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 延迟初始化:首次调用时创建默认线程池
    if (!thread_pool_) {
        create_default_thread_pool_();
    }
    
    return thread_pool_;
}

void registry::set_thread_pool(std::shared_ptr<details::thread_pool> tp) {
    std::lock_guard<std::mutex> lock(mutex_);
    thread_pool_ = std::move(tp);
}

void registry::create_default_thread_pool_() {
    // 默认配置:队列大小 8192,1 个工作线程
    // 参考 spdlog 的默认配置
    constexpr size_t default_queue_size = 8192;
    constexpr size_t default_threads = 1;
    
    thread_pool_ = std::make_shared<details::thread_pool>(
        default_queue_size, 
        default_threads
    );
}

} // namespace minispdlog
