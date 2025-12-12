#include "minispdlog/details/log_msg.h"
#include "minispdlog/sinks/console_sink.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace minispdlog;

void test_log_msg_creation() {
    std::cout << "\n========== 测试1:log_msg 创建 ==========\n";
    
    // 创建日志消息
    details::log_msg msg1("TestLogger", level::info, "Hello, MiniSpdlog!");
    
    std::cout << "Logger 名称: " << msg1.logger_name << "\n";
    std::cout << "日志级别: " << level_to_string(msg1.lvl) << "\n";
    std::cout << "线程 ID: " << msg1.thread_id << "\n";
    std::cout << "消息内容: " << msg1.payload << "\n";
    std::cout << "源码位置为空: " << (msg1.source.empty() ? "是" : "否") << "\n";
}

void test_source_loc() {
    std::cout << "\n========== 测试2:源码位置信息 ==========\n";
    
    details::source_loc loc("test_sink.cpp", 42, "test_function");
    details::log_msg msg(loc, "TestLogger", level::debug, "Debug message with source info");
    
    std::cout << "文件名: " << (msg.source.filename ? msg.source.filename : "null") << "\n";
    std::cout << "行号: " << msg.source.line << "\n";
    std::cout << "函数名: " << (msg.source.funcname ? msg.source.funcname : "null") << "\n";
}

void test_console_sink_mt() {
    std::cout << "\n========== 测试3:多线程控制台 Sink ==========\n";
    
    // 创建多线程安全的控制台 Sink
    auto sink = std::make_shared<sinks::console_sink_mt>();
    
    // 设置日志级别
    sink->set_level(level::trace);
    
    // 测试不同级别的日志
    std::vector<level> levels = {
        level::trace, level::debug, level::info,
        level::warn, level::error, level::critical
    };
    
    for (auto lvl : levels) {
        std::string msg_content = std::string("This is a ") + 
                                  level_to_string(lvl) + " message";
        details::log_msg msg("TestLogger", lvl, msg_content);
        
        if (sink->should_log(lvl)) {
            sink->log(msg);
        }
    }
    
    sink->flush();
}

void test_level_filtering() {
    std::cout << "\n========== 测试4:日志级别过滤 ==========\n";
    
    auto sink = std::make_shared<sinks::console_sink_mt>();
    
    // 设置级别为 warn(只输出 warn 及以上)
    sink->set_level(level::warn);
    std::cout << "Sink 级别设置为: " << level_to_string(sink->get_level()) << "\n\n";
    
    std::vector<level> test_levels = {
        level::trace, level::debug, level::info,
        level::warn, level::error, level::critical
    };
    
    for (auto lvl : test_levels) {
        std::string msg_content = std::string("Testing ") + level_to_string(lvl);
        details::log_msg msg("FilterTest", lvl, msg_content);
        
        bool should = sink->should_log(lvl);
        std::cout << level_to_string(lvl) << " - "
                  << (should ? "✓ 会输出" : "✗ 被过滤") << "\n";
        
        if (should) {
            sink->log(msg);
        }
    }
}

void test_stderr_sink() {
    std::cout << "\n========== 测试5:stderr Sink ==========\n";
    
    auto err_sink = std::make_shared<sinks::stderr_sink_mt>();
    err_sink->set_level(level::error);
    
    std::cout << "(以下消息应该输出到 stderr)\n";
    
    details::log_msg error_msg("ErrorLogger", level::error, "This is an error message");
    details::log_msg critical_msg("ErrorLogger", level::critical, "This is a critical message");
    
    err_sink->log(error_msg);
    err_sink->log(critical_msg);
    err_sink->flush();
}

void test_performance_hint() {
    std::cout << "\n========== 测试6:性能对比提示 ==========\n";
    
    std::cout << "💡 性能提示:\n";
    std::cout << "  - console_sink_mt: 多线程安全(使用 std::mutex)\n";
    std::cout << "  - console_sink_st: 单线程版本(无锁,性能更高)\n";
    std::cout << "  - 如果确定只在单线程使用,推荐使用 _st 版本\n";
    
    auto sink_mt = std::make_shared<sinks::console_sink_mt>();
    auto sink_st = std::make_shared<sinks::console_sink_st>();
    
    std::cout << "\n使用 _mt 版本输出:\n";
    details::log_msg msg1("MTLogger", level::info, "Thread-safe message");
    sink_mt->log(msg1);
    
    std::cout << "\n使用 _st 版本输出:\n";
    details::log_msg msg2("STLogger", level::info, "Single-thread message (faster)");
    sink_st->log(msg2);
}

int main() {
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   MiniSpdlog 第2天测试 - Sink系统   ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    
    try {
        test_log_msg_creation();
        test_source_loc();
        test_console_sink_mt();
        test_level_filtering();
        test_stderr_sink();
        test_performance_hint();
        
        std::cout << "\n✅ 所有测试通过!\n\n";
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 测试失败: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}