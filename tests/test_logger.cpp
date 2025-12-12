#include "minispdlog/logger.h"
#include "minispdlog/sinks/console_sink.h"
#include "minispdlog/sinks/color_console_sink.h"
#include "minispdlog/sinks/file_sink.h"
#include "minispdlog/pattern_formatter.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace minispdlog;

void test_basic_logging() {
    std::cout << "\n========== 测试1:基础日志接口 ==========\n";
    
    auto console_sink = std::make_shared<sinks::console_sink_mt>();
    logger my_logger("TestLogger", console_sink);
    
    my_logger.trace("This is a trace message");
    my_logger.debug("This is a debug message");
    my_logger.info("This is an info message");
    my_logger.warn("This is a warning message");
    my_logger.error("This is an error message");
    my_logger.critical("This is a critical message");
}

void test_formatted_logging() {
    std::cout << "\n========== 测试2:格式化日志 ==========\n";
    
    auto console_sink = std::make_shared<sinks::console_sink_mt>();
    logger my_logger("FormatLogger", console_sink);
    
    int value = 42;
    std::string name = "World";
    double pi = 3.14159;
    
    my_logger.info("Integer: {}", value);
    my_logger.info("String: {}", name);
    my_logger.info("Float: {:.2f}", pi);
    my_logger.info("Multiple: {}, {}, {:.3f}", value, name, pi);
    my_logger.info("Named args: value={}, name={}", value, name);
}

void test_level_filtering() {
    std::cout << "\n========== 测试3:级别过滤 ==========\n";
    
    auto console_sink = std::make_shared<sinks::console_sink_mt>();
    logger my_logger("FilterLogger", console_sink);
    
    std::cout << "设置 logger 级别为 warn:\n";
    my_logger.set_level(level::warn);
    
    my_logger.trace("Trace (不会输出)");
    my_logger.debug("Debug (不会输出)");
    my_logger.info("Info (不会输出)");
    my_logger.warn("Warn (会输出)");
    my_logger.error("Error (会输出)");
    my_logger.critical("Critical (会输出)");
}

void test_multi_sink() {
    std::cout << "\n========== 测试4:多 Sink 输出 ==========\n";
    
    // 创建多个 sink
    auto console_sink = std::make_shared<sinks::console_sink_mt>();
    auto file_sink = std::make_shared<sinks::file_sink_mt>("logs/multi_sink.log", true);
    
    // 为不同 sink 设置不同格式
    console_sink->set_formatter(
        std::make_unique<pattern_formatter>("[控制台] [%H:%M:%S] [%l] %v")
    );
    
    file_sink->set_formatter(
        std::make_unique<pattern_formatter>("[文件] [%Y-%m-%d %H:%M:%S] [%L] [%n] %v")
    );
    
    // 创建 logger 并添加多个 sink
    logger my_logger("MultiSinkLogger");
    my_logger.add_sink(console_sink);
    my_logger.add_sink(file_sink);
    
    my_logger.info("This message goes to both console and file");
    my_logger.warn("Warning in multiple outputs");
    my_logger.error("Error logged everywhere");
    
    std::cout << "\n💡 同时查看控制台和 logs/multi_sink.log 文件\n";
}

void test_file_logging() {
    std::cout << "\n========== 测试5:文件日志 ==========\n";
    
    // 创建文件 sink
    auto file_sink = std::make_shared<sinks::file_sink_mt>("logs/test.log", true);
    logger file_logger("FileLogger", file_sink);
    
    file_logger.info("Writing to file");
    file_logger.debug("Debug information");
    file_logger.error("An error occurred");
    
    std::cout << "日志已写入 logs/test.log\n";
    
    // 测试 append 模式
    auto append_sink = std::make_shared<sinks::file_sink_mt>("logs/append.log", false);
    logger append_logger("AppendLogger", append_sink);
    
    append_logger.info("First message");
    append_logger.info("Second message");
    
    std::cout << "追加日志已写入 logs/append.log\n";
}

void test_color_logging() {
    std::cout << "\n========== 测试6:彩色日志 ==========\n";
    
    auto color_sink = std::make_shared<sinks::color_console_sink_mt>();
    logger color_logger("ColorLogger", color_sink);
    
    std::cout << "以下日志应该带有颜色:\n";
    color_logger.trace("Trace message (白色)");
    color_logger.debug("Debug message (青色)");
    color_logger.info("Info message (绿色)");
    color_logger.warn("Warning message (黄色)");
    color_logger.error("Error message (红色)");
    color_logger.critical("Critical message (红色加粗)");
}

void test_sink_level() {
    std::cout << "\n========== 测试7:Sink 级别控制 ==========\n";
    
    auto console_sink = std::make_shared<sinks::console_sink_mt>();
    auto file_sink = std::make_shared<sinks::file_sink_mt>("logs/errors_only.log", true);
    
    // 控制台显示所有级别
    console_sink->set_level(level::trace);
    
    // 文件只记录 error 及以上
    file_sink->set_level(level::error);
    
    logger my_logger("SinkLevelLogger");
    my_logger.add_sink(console_sink);
    my_logger.add_sink(file_sink);
    
    std::cout << "控制台显示所有级别,文件只记录 error:\n";
    my_logger.debug("Debug - 仅控制台");
    my_logger.info("Info - 仅控制台");
    my_logger.warn("Warn - 仅控制台");
    my_logger.error("Error - 控制台+文件");
    my_logger.critical("Critical - 控制台+文件");
    
    std::cout << "\n💡 查看 logs/errors_only.log,应该只有 error 和 critical\n";
}

void test_custom_pattern() {
    std::cout << "\n========== 测试8:自定义格式 ==========\n";
    
    auto sink = std::make_shared<sinks::color_console_sink_mt>();
    sink->set_formatter(
        std::make_unique<pattern_formatter>(">>> [%Y-%m-%d %H:%M:%S] [%n] [%L] %v <<<")
    );
    
    logger my_logger("CustomLogger", sink);
    my_logger.info("Custom formatted message");
    my_logger.warn("Another custom message");
}

void test_flush() {
    std::cout << "\n========== 测试9:手动刷新 ==========\n";
    
    auto file_sink = std::make_shared<sinks::file_sink_mt>("logs/flush_test.log", true);
    logger my_logger("FlushLogger", file_sink);
    
    my_logger.info("Message 1");
    my_logger.info("Message 2");
    my_logger.info("Message 3");
    
    std::cout << "写入 3 条消息,手动刷新...\n";
    my_logger.flush();
    std::cout << "刷新完成,日志已写入磁盘\n";
}

void test_auto_flush() {
    std::cout << "\n========== 测试10:自动刷新 ==========\n";
    
    auto file_sink = std::make_shared<sinks::file_sink_mt>("logs/auto_flush.log", true);
    logger my_logger("AutoFlushLogger", file_sink);
    
    // 设置在 error 级别自动刷新
    my_logger.flush_on(level::error);
    
    my_logger.info("Info message (不会立即刷新)");
    my_logger.warn("Warn message (不会立即刷新)");
    my_logger.error("Error message (自动刷新!)");
    
    std::cout << "💡 error 及以上级别会自动刷新到磁盘\n";
}

void test_performance() {
    std::cout << "\n========== 测试11:性能测试 ==========\n";
    
    auto console_sink = std::make_shared<sinks::console_sink_mt>();
    console_sink->set_level(level::off);  // 关闭输出,只测试开销
    
    logger my_logger("PerfLogger", console_sink);
    
    const int iterations = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        my_logger.info("Test message {}", i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "格式化 " << iterations << " 条日志耗时: " 
              << duration.count() << " 微秒\n";
    std::cout << "平均每条: " << (duration.count() / (double)iterations) << " 微秒\n";
    std::cout << "吞吐量: " << (iterations * 1000000.0 / duration.count()) << " 条/秒\n";
}

void test_multithread() {
    std::cout << "\n========== 测试12:多线程日志 ==========\n";
    
    auto color_sink = std::make_shared<sinks::color_console_sink_mt>();
    logger my_logger("MTLogger", color_sink);
    
    auto log_from_thread = [&my_logger](int thread_id) {
        for (int i = 0; i < 5; ++i) {
            my_logger.info("Thread {} - Message {}", thread_id, i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };
    
    std::thread t1(log_from_thread, 1);
    std::thread t2(log_from_thread, 2);
    std::thread t3(log_from_thread, 3);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "\n💡 多线程日志输出完成,顺序可能交错但线程安全\n";
}

void test_real_world_example() {
    std::cout << "\n========== 测试13:真实场景示例 ==========\n";
    
    // 创建一个典型的日志配置
    auto console_sink = std::make_shared<sinks::color_console_sink_mt>();
    auto file_sink = std::make_shared<sinks::file_sink_mt>("logs/application.log", false);
    auto error_sink = std::make_shared<sinks::file_sink_mt>("logs/errors.log", false);
    
    // 控制台:info 及以上
    console_sink->set_level(level::info);
    console_sink->set_formatter(
         std::make_unique<pattern_formatter>("[%H:%M:%S] [%l] %v")
        // std::make_unique<pattern_formatter>("[%H:%M:%S] [%^%l%$] %v")
    );
    
    // 普通日志文件:debug 及以上
    file_sink->set_level(level::debug);
    file_sink->set_formatter(
        std::make_unique<pattern_formatter>("[%Y-%m-%d %H:%M:%S] [%L] [%n] %v")
    );
    
    // 错误日志文件:error 及以上
    error_sink->set_level(level::error);
    error_sink->set_formatter(
        std::make_unique<pattern_formatter>("[%Y-%m-%d %H:%M:%S] [%L] [%n] [thread %t] %v")
    );
    
    logger app_logger("Application");
    app_logger.add_sink(console_sink);
    app_logger.add_sink(file_sink);
    app_logger.add_sink(error_sink);
    
    // 模拟应用程序日志
    app_logger.info("Application started");
    app_logger.debug("Loading configuration...");
    app_logger.debug("Configuration loaded successfully");
    app_logger.info("Connecting to database...");
    app_logger.info("Database connected");
    app_logger.warn("Connection pool size is low");
    app_logger.error("Failed to fetch user data");
    app_logger.critical("Database connection lost!");
    
    std::cout << "\n💡 查看 3 个文件的内容:\n";
    std::cout << "  - logs/application.log (debug及以上)\n";
    std::cout << "  - logs/errors.log (error及以上)\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║  MiniSpdlog 第4天测试 - Logger系统  ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    
    // 创建日志目录
    system("mkdir -p logs");
    
    try {
        test_basic_logging();
        test_formatted_logging();
        test_level_filtering();
        test_multi_sink();
        test_file_logging();
        test_color_logging();
        test_sink_level();
        test_custom_pattern();
        test_flush();
        test_auto_flush();
        test_performance();
        test_multithread();
        test_real_world_example();
        
        std::cout << "\n✅ 所有测试通过!\n\n";
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 测试失败: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}