
# 收集所有源文件
set(MINISPDLOG_SOURCES
    level.cpp
    formatter.cpp
    pattern_formatter.cpp
    logger.cpp
    async_logger.cpp              # 第8天新增
    registry.cpp
    details/utils.cpp
    details/thread_pool.cpp
    sinks/rotating_file_sink.cpp
)

# 创建静态库
add_library(minispdlog STATIC ${MINISPDLOG_SOURCES})

# 包含目录
target_include_directories(minispdlog PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 链接 fmt 库
target_link_libraries(minispdlog PUBLIC fmt::fmt)

# 设置编译特性
target_compile_features(minispdlog PUBLIC cxx_std_17)