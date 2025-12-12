#pragma once

#include "common.h"
#include "logger.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace minispdlog {

// 前向声明
namespace details {
class thread_pool;
}

// registry: Logger 注册表(单例模式)
// 参考 spdlog 设计:全局管理所有 logger + 全局线程池
//
// 第8天新增:
//   - 持有全局 thread_pool 的 shared_ptr
//   - 提供 thread_pool 的访问和初始化接口
class MINISPDLOG_API registry {
public:
    // 禁止拷贝
    registry(const registry&) = delete;
    registry& operator=(const registry&) = delete;
    
    // 获取单例实例(线程安全)
    static registry& instance();
    
    // ========== Logger 注册管理 ==========
    
    // 注册 logger(如果名称已存在则抛出异常)
    void register_logger(std::shared_ptr<logger> new_logger);
    
    // 获取 logger(如果不存在返回 nullptr)
    std::shared_ptr<logger> get(const std::string& logger_name);
    
    // 移除 logger
    void drop(const std::string& logger_name);
    
    // 移除所有 logger
    void drop_all();
    
    // ========== 默认 Logger ==========
    
    // 获取默认 logger
    std::shared_ptr<logger> default_logger();
    
    // 设置默认 logger
    void set_default_logger(std::shared_ptr<logger> new_default_logger);
    
    // ========== 全局设置 ==========
    
    // 设置所有 logger 的日志级别
    void set_level(level log_level);
    
    // 刷新所有 logger
    void flush_all();
    
    // ========== 线程池管理(第8天新增) ==========
    
    // 初始化全局线程池
    // queue_size: 队列大小(默认 8192)
    // threads_n: 工作线程数(默认 1)
    // 注意:必须在创建异步 logger 之前调用
    void init_thread_pool(size_t queue_size, size_t threads_n = 1);
    
    // 获取全局线程池(如果不存在则自动创建默认配置的线程池)
    std::shared_ptr<details::thread_pool> thread_pool();
    
    // 显式设置线程池(高级用法)
    void set_thread_pool(std::shared_ptr<details::thread_pool> tp);
    
private:
    registry();
    ~registry() = default;
    
    // 检查名称是否已存在(抛出异常)
    void throw_if_exists_(const std::string& logger_name);
    
    // 创建默认线程池(延迟初始化)
    void create_default_thread_pool_();
    
    std::mutex mutex_;                                              // 保护下面的成员
    std::unordered_map<std::string, std::shared_ptr<logger>> loggers_;  // Logger 映射表
    std::shared_ptr<logger> default_logger_;                        // 默认 logger
    
    // 第8天新增:全局线程池
    std::shared_ptr<details::thread_pool> thread_pool_;            // 全局线程池
};

} // namespace minispdlog