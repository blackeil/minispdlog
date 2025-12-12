#pragma once

#include "common.h"
#include "details/log_msg.h"
#include <fmt/format.h>
#include <memory>

namespace minispdlog {

// formatter 抽象接口
// 参考 spdlog 设计:每个 sink 拥有一个 formatter 实例
class formatter {
public:
    virtual ~formatter() = default;
    
    // 格式化日志消息到缓冲区
    // 使用 fmt::memory_buffer 以获得最佳性能
    virtual void format(const details::log_msg& msg, fmt::memory_buffer& dest) = 0;
    
    // 创建 formatter 的副本
    // 每个 sink 需要独立的 formatter 实例,避免多线程竞争
    virtual std::unique_ptr<formatter> clone() const = 0;
};

} // namespace minispdlog