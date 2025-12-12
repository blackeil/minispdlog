#include "minispdlog/pattern_formatter.h"
#include "minispdlog/details/utils.h"
#include <iomanip>
#include <sstream>
#include <cctype>

namespace minispdlog {

    namespace details {

// ============================================================================
// 高性能辅助函数(基于 spdlog 的 fmt_helper)
// ============================================================================

// 快速整数到字符串转换
inline void fast_uint_to_str(uint32_t n, char* buffer) {
    // 使用查表法优化小数字
    static constexpr char digits_table[] = 
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";
    
    if (n < 100) {
        if (n < 10) {
            buffer[0] = '0' + n;
            buffer[1] = '\0';
        } else {
            const char* d = digits_table + n * 2;
            buffer[0] = d[0];
            buffer[1] = d[1];
            buffer[2] = '\0';
        }
        return;
    }
    
    // 对于大数字，回退到标准方法
    fmt::format_to(buffer, "{}", n);
}

// 快速两位数转换(用于时间格式化)
inline void fast_two_digits(uint32_t n, char* buffer) {
    if (n < 100) {
        const char* d = "0001020304050607080910111213141516171819"
                       "2021222324252627282930313233343536373839"
                       "4041424344454647484950515253545556575859"
                       "6061626364656667686970717273747576777879"
                       "8081828384858687888990919293949596979899" + n * 2;
        buffer[0] = d[0];
        buffer[1] = d[1];
    } else {
        buffer[0] = '0';
        buffer[1] = '0';
    }
}

// 聚合文本formatter - 关键优化！
class aggregate_formatter : public pattern_formatter::flag_formatter {
public:
    explicit aggregate_formatter(std::string str) : str_(std::move(str)) {}
    
    void format(const details::log_msg&, const std::tm&, fmt::memory_buffer& dest) override {
        // 直接内存拷贝，避免逐字符追加
        dest.append(str_.data(), str_.data() + str_.size());
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<aggregate_formatter>(str_);
    }
    
    void add_ch(char ch) { str_ += ch; }
    void add_str(const std::string& str) { str_ += str; }
    
private:
    std::string str_;
};

// ============================================================================
// 高性能 Flag Formatter 实现
// ============================================================================

// %Y - 年份(4位) - 优化版本
class year_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        char buffer[5];
        int year = tm_time.tm_year + 1900;
        // 手动展开避免 sprintf 开销
        buffer[0] = '0' + (year / 1000);
        buffer[1] = '0' + ((year / 100) % 10);
        buffer[2] = '0' + ((year / 10) % 10);
        buffer[3] = '0' + (year % 10);
        buffer[4] = '\0';
        dest.append(buffer, buffer + 4);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<year_formatter>();
    }
};

// %m - 月份(01-12) - 优化版本
class month_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        char buffer[3];
        fast_two_digits(tm_time.tm_mon + 1, buffer);
        dest.append(buffer, buffer + 2);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<month_formatter>();
    }
};

// %d - 日期(01-31) - 优化版本
class day_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        char buffer[3];
        fast_two_digits(tm_time.tm_mday, buffer);
        dest.append(buffer, buffer + 2);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<day_formatter>();
    }
};

// %H - 小时(00-23) - 优化版本
class hour_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        char buffer[3];
        fast_two_digits(tm_time.tm_hour, buffer);
        dest.append(buffer, buffer + 2);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<hour_formatter>();
    }
};

// %M - 分钟(00-59) - 优化版本
class minute_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        char buffer[3];
        fast_two_digits(tm_time.tm_min, buffer);
        dest.append(buffer, buffer + 2);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<minute_formatter>();
    }
};

// %S - 秒(00-59) - 优化版本
class second_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        char buffer[3];
        fast_two_digits(tm_time.tm_sec, buffer);
        dest.append(buffer, buffer + 2);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<second_formatter>();
    }
};

// %l - 日志级别(短格式) - 优化版本
class level_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        // 预计算的级别字符串，避免函数调用
        static constexpr std::string_view level_strings[] = {
            "T", "D", "I", "W", "E", "C"
        };
        
        if (static_cast<size_t>(msg.lvl) < std::size(level_strings)) {
            auto level_str = level_strings[static_cast<size_t>(msg.lvl)];
            dest.append(level_str.data(), level_str.data() + level_str.size());
        }
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<level_formatter>();
    }
};

// %L - 日志级别(完整格式) - 优化版本
class level_full_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        // 预计算的级别字符串，避免函数调用
        static constexpr std::string_view level_strings[] = {
            "trace", "debug", "info", "warning", "error", "critical"
        };
        
        if (static_cast<size_t>(msg.lvl) < std::size(level_strings)) {
            auto level_str = level_strings[static_cast<size_t>(msg.lvl)];
            dest.append(level_str.data(), level_str.data() + level_str.size());
        }
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<level_full_formatter>();
    }
};

// %n - Logger 名称 - 优化版本
class name_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        dest.append(msg.logger_name.data(), msg.logger_name.data() + msg.logger_name.size());
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<name_formatter>();
    }
};

// %v - 实际日志内容 - 优化版本
class payload_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        dest.append(msg.payload.data(), msg.payload.data() + msg.payload.size());
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<payload_formatter>();
    }
};

// %t - 线程 ID - 优化版本
class thread_id_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        char buffer[32];
        fast_uint_to_str(static_cast<uint32_t>(msg.thread_id), buffer);
        size_t len = std::strlen(buffer);
        dest.append(buffer, buffer + len);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<thread_id_formatter>();
    }
};

} // namespace details

// ============================================================================
// pattern_formatter 实现
// ============================================================================

pattern_formatter::pattern_formatter(std::string pattern)
    : pattern_(std::move(pattern))
{
    compile_pattern();
}

void pattern_formatter::format(const details::log_msg& msg, fmt::memory_buffer& dest) {
    // 预分配空间避免多次重新分配
    dest.reserve(dest.size() + 256);
    
    // 时间缓存优化
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(
        msg.time.time_since_epoch()
    );
    
    if (secs != last_log_secs_) {
        cached_tm_ = get_time(msg);
        last_log_secs_ = secs;
    }
    
    // 遍历所有 formatter
    for (const auto& formatter : formatters_) {
        formatter->format(msg, cached_tm_, dest);
    }
    
    // 添加换行符
    dest.push_back('\n');
}

std::unique_ptr<formatter> pattern_formatter::clone() const {
    return std::make_unique<pattern_formatter>(pattern_);
}

void pattern_formatter::set_pattern(std::string pattern) {
    pattern_ = std::move(pattern);
    formatters_.clear();
    compile_pattern();
}

void pattern_formatter::compile_pattern() {
    auto it = pattern_.begin();
    auto end = pattern_.end();
    std::unique_ptr<details::aggregate_formatter> user_chars;
    
    while (it != end) {
        if (*it == '%') {
            // 保存聚合的普通文本
            if (user_chars) {
                formatters_.push_back(std::move(user_chars));
            }
            
            // 解析占位符
            ++it;
            if (it != end) {
                char flag = *it;
                ++it;
                
                // 根据 flag 创建对应的 formatter
                switch (flag) {
                    case 'Y': formatters_.push_back(std::make_unique<details::year_formatter>()); break;
                    case 'm': formatters_.push_back(std::make_unique<details::month_formatter>()); break;
                    case 'd': formatters_.push_back(std::make_unique<details::day_formatter>()); break;
                    case 'H': formatters_.push_back(std::make_unique<details::hour_formatter>()); break;
                    case 'M': formatters_.push_back(std::make_unique<details::minute_formatter>()); break;
                    case 'S': formatters_.push_back(std::make_unique<details::second_formatter>()); break;
                    case 'l': formatters_.push_back(std::make_unique<details::level_formatter>()); break;
                    case 'L': formatters_.push_back(std::make_unique<details::level_full_formatter>()); break;
                    case 'n': formatters_.push_back(std::make_unique<details::name_formatter>()); break;
                    case 'v': formatters_.push_back(std::make_unique<details::payload_formatter>()); break;
                    case 't': formatters_.push_back(std::make_unique<details::thread_id_formatter>()); break;
                    case '%': 
                        if (!user_chars) user_chars = std::make_unique<details::aggregate_formatter>("");
                        user_chars->add_ch('%'); 
                        break;
                    default:
                        // 未知占位符,创建聚合formatter
                        if (!user_chars) user_chars = std::make_unique<details::aggregate_formatter>("");
                        user_chars->add_ch('%');
                        user_chars->add_ch(flag);
                        break;
                }
            }
        } else {
            // 普通字符,累积到聚合formatter
            if (!user_chars) {
                user_chars = std::make_unique<details::aggregate_formatter>("");
            }
            user_chars->add_ch(*it);
            ++it;
        }
    }
    
    // 处理末尾的聚合文本
    if (user_chars) {
        formatters_.push_back(std::move(user_chars));
    }
}

std::tm pattern_formatter::get_time(const details::log_msg& msg) {
    auto time_t_val = log_clock::to_time_t(msg.time);
    std::tm tm_val;
    
#ifdef _WIN32
    localtime_s(&tm_val, &time_t_val);
#else
    localtime_r(&time_t_val, &tm_val);
#endif
    
    return tm_val;
}


#if 0
// ============================================================================
// 各种 Flag Formatter 实现
// ============================================================================

 namespace {

// 普通文本(非占位符)
class raw_string_formatter : public pattern_formatter::flag_formatter {
public:
    explicit raw_string_formatter(std::string str) : str_(std::move(str)) {}
    
    void format(const details::log_msg&, const std::tm&, fmt::memory_buffer& dest) override {
        dest.append(str_.data(), str_.data() + str_.size());
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<raw_string_formatter>(str_);
    }
    
private:
    std::string str_;
};

// %Y - 年份(4位)
class year_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{:04d}", tm_time.tm_year + 1900);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<year_formatter>();
    }
};

// %m - 月份(01-12)
class month_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{:02d}", tm_time.tm_mon + 1);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<month_formatter>();
    }
};

// %d - 日期(01-31)
class day_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{:02d}", tm_time.tm_mday);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<day_formatter>();
    }
};

// %H - 小时(00-23)
class hour_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{:02d}", tm_time.tm_hour);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<hour_formatter>();
    }
};

// %M - 分钟(00-59)
class minute_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{:02d}", tm_time.tm_min);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<minute_formatter>();
    }
};

// %S - 秒(00-59)
class second_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg&, const std::tm& tm_time, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{:02d}", tm_time.tm_sec);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<second_formatter>();
    }
};

// %l - 日志级别(短格式,如 I/W/E)
class level_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        const char* level_str = level_to_short_string(msg.lvl);
        dest.append(level_str, level_str + std::strlen(level_str));
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<level_formatter>();
    }
};

// %L - 日志级别(完整格式,如 info/warn/error)
class level_full_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        const char* level_str = level_to_string(msg.lvl);
        dest.append(level_str, level_str + std::strlen(level_str));
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<level_full_formatter>();
    }
};

// %n - Logger 名称
class name_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        dest.append(msg.logger_name.data(), msg.logger_name.data() + msg.logger_name.size());
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<name_formatter>();
    }
};

// %v - 实际日志内容
class payload_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        dest.append(msg.payload.data(), msg.payload.data() + msg.payload.size());
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<payload_formatter>();
    }
};

// %t - 线程 ID
class thread_id_formatter : public pattern_formatter::flag_formatter {
public:
    void format(const details::log_msg& msg, const std::tm&, fmt::memory_buffer& dest) override {
        fmt::format_to(std::back_inserter(dest), "{}", msg.thread_id);
    }
    
    std::unique_ptr<flag_formatter> clone() const override {
        return std::make_unique<thread_id_formatter>();
    }
};

} // anonymous namespace

#endif

#if 0
// ============================================================================
// pattern_formatter 实现
// ============================================================================

pattern_formatter::pattern_formatter(std::string pattern)
    : pattern_(std::move(pattern))
{
    compile_pattern();
}

/*
========== 测试1:Pattern 编译 ==========
Pattern: [%Y-%m-%d %H:%M:%S] [%l] %v
Output:  [2025-09-30 03:36:39] [I] Hello, World!

*/
void pattern_formatter::format(const details::log_msg& msg, fmt::memory_buffer& dest) {
    // 性能优化:时间缓存
    // 只有当秒数变化时才重新获取 tm 结构
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(
        msg.time.time_since_epoch()
    );
    
    if (secs != last_log_secs_) {
        cached_tm_ = get_time(msg);
        last_log_secs_ = secs;
    }
    
    // 遍历所有 flag_formatter,完成格式化
    for (auto& formatter : formatters_) {
        formatter->format(msg, cached_tm_, dest);
    }
    
    // 添加换行符
    dest.push_back('\n');
}

std::unique_ptr<formatter> pattern_formatter::clone() const {
    return std::make_unique<pattern_formatter>(pattern_);
}

void pattern_formatter::set_pattern(std::string pattern) {
    pattern_ = std::move(pattern);
    formatters_.clear();
    compile_pattern();
}

void pattern_formatter::compile_pattern() {
    auto it = pattern_.begin();
    auto end = pattern_.end();
    std::string user_chars;
    
    while (it != end) {
        if (*it == '%') {
            // 遇到 %,先保存之前的普通文本
            if (!user_chars.empty()) {
                formatters_.push_back(
                    std::make_unique<raw_string_formatter>(std::move(user_chars))
                );
                user_chars.clear();
            }
            
            // 解析占位符
            ++it;
            if (it != end) {
                char flag = *it;
                ++it;
                
                // 根据 flag 创建对应的 formatter
                switch (flag) {
                    case 'Y': formatters_.push_back(std::make_unique<year_formatter>()); break;
                    case 'm': formatters_.push_back(std::make_unique<month_formatter>()); break;
                    case 'd': formatters_.push_back(std::make_unique<day_formatter>()); break;
                    case 'H': formatters_.push_back(std::make_unique<hour_formatter>()); break;
                    case 'M': formatters_.push_back(std::make_unique<minute_formatter>()); break;
                    case 'S': formatters_.push_back(std::make_unique<second_formatter>()); break;
                    case 'l': formatters_.push_back(std::make_unique<level_formatter>()); break;
                    case 'L': formatters_.push_back(std::make_unique<level_full_formatter>()); break;
                    case 'n': formatters_.push_back(std::make_unique<name_formatter>()); break;
                    case 'v': formatters_.push_back(std::make_unique<payload_formatter>()); break;
                    case 't': formatters_.push_back(std::make_unique<thread_id_formatter>()); break;
                    case '%': user_chars += '%'; break;  // %% 转义为 %
                    default:
                        // 未知占位符,按原样输出
                        user_chars += '%';
                        user_chars += flag;
                        break;
                }
            }
        } else {
            // 普通字符,累积到 user_chars
            user_chars += *it;
            ++it;
        }
    }
    
    // 处理末尾的普通文本
    if (!user_chars.empty()) {
        formatters_.push_back(
            std::make_unique<raw_string_formatter>(std::move(user_chars))
        );
    }
}

std::tm pattern_formatter::get_time(const details::log_msg& msg) {
    auto time_t_val = log_clock::to_time_t(msg.time);
    std::tm tm_val;
    
#ifdef _WIN32
    localtime_s(&tm_val, &time_t_val);
#else
    localtime_r(&time_t_val, &tm_val);
#endif
    
    return tm_val;
}

#endif

 } // namespace minispdlog