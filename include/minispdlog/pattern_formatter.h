#pragma once

#include "formatter.h"
#include "level.h"
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <ctime>

namespace minispdlog {

// pattern_formatter:基于 pattern 字符串的格式化器
// 支持类似 strftime 的占位符语法
class pattern_formatter : public formatter {
public:
    // 构造函数:接受 pattern 字符串
    // pattern 示例: "[%Y-%m-%d %H:%M:%S] [%l] [%n] %v"
    explicit pattern_formatter(
        std::string pattern = "[%Y-%m-%d %H:%M:%S] [%l] %v"
    );
    
    ~pattern_formatter() override = default;
    
    // 实现 formatter 接口
    void format(const details::log_msg& msg, fmt::memory_buffer& dest) override;
    std::unique_ptr<formatter> clone() const override;
    
    // 设置新的 pattern(重新编译)
    void set_pattern(std::string pattern);
    
public:
    // flag_formatter 抽象基类:处理单个占位符
    class flag_formatter {
    public:
        virtual ~flag_formatter() = default;
        virtual void format(const details::log_msg& msg, 
                          const std::tm& tm_time, 
                          fmt::memory_buffer& dest) = 0;
        virtual std::unique_ptr<flag_formatter> clone() const = 0;
    };

private:    
    // 编译 pattern 字符串为 flag_formatter 向量
    void compile_pattern();
    
    // 获取格式化后的时间结构
    std::tm get_time(const details::log_msg& msg);
    
    std::string pattern_;                               // pattern 字符串
    std::vector<std::unique_ptr<flag_formatter>> formatters_;  // flag_formatter 向量
    
    // 性能优化:时间缓存
    std::chrono::seconds last_log_secs_{0};            // 上次日志的秒数
    std::tm cached_tm_{};                               // 缓存的 tm 结构
};

} // namespace minispdlog