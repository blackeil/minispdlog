# 在文件开头添加线程库查找
find_package(Threads REQUIRED)

# 测试1:日志级别测试
add_executable(test_level test_level.cpp)
target_link_libraries(test_level PRIVATE minispdlog)

# 测试2:Sink 和日志消息测试(第2天)
add_executable(test_sink test_sink.cpp)
target_link_libraries(test_sink PRIVATE minispdlog)

# 测试3:Formatter 测试(第3天) - 这个使用了 std::thread，需要链接线程库
add_executable(test_formatter test_formatter.cpp)
target_link_libraries(test_formatter PRIVATE minispdlog Threads::Threads)

# 测试4:Logger 测试(第4天)
add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger PRIVATE minispdlog Threads::Threads)

# 测试5:Registry 测试(第5天)
add_executable(test_registry test_registry.cpp)
target_link_libraries(test_registry PRIVATE minispdlog Threads::Threads)

# 测试6:Rotating File 测试(第6天)
add_executable(test_rotating_file test_rotating_file.cpp)
target_link_libraries(test_rotating_file PRIVATE minispdlog Threads::Threads)

# 测试7:Thread Pool 测试(第7天)
# add_executable(test_thread_pool test_thread_pool.cpp)
# target_link_libraries(test_thread_pool PRIVATE minispdlog Threads::Threads)

add_executable(test_async_logger  test_async_logger.cpp)
target_link_libraries(test_async_logger PRIVATE minispdlog Threads::Threads)

add_executable(test_performance  test_performance.cpp)
target_link_libraries(test_performance PRIVATE minispdlog Threads::Threads)

add_executable(benchmark_spdlog  benchmark_spdlog.cpp)
target_link_libraries(benchmark_spdlog PRIVATE minispdlog Threads::Threads)