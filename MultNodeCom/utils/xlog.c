// log.c - 日志系统实现
#include "xlog.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#endif

// 线程安全的局部缓冲区
static char log_buffer[1024]; 

// 获取时间戳字符串 (线程安全实现)
static const char* get_timestamp(void) {
    static __thread char time_buf[20];
    
    time_t now;
    time(&now);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return time_buf;
}

// 获取简约文件名 (去除路径)
static const char* shorten_filename(const char* full_path) {
    const char* p = strrchr(full_path, '/');
    if (p) return p + 1;
    
    p = strrchr(full_path, '\\');
    if (p) return p + 1;
    
    return full_path;
}

// 实际的日志输出函数
void log_message(LogLevel level, const char* file, const char* function, int line, const char* fmt, ...) {
    // 静态检查 (编译时优化)
    if (default_logger.min_level == LOG_LEVEL_NONE || default_logger.callback == NULL) {
        return;
    }
    
    // 缓冲区的安全访问
    char* buf = log_buffer;
    size_t pos = 0;
    size_t max_size = sizeof(log_buffer);
    
    // 添加时间戳
    if (default_logger.options & LOG_OPT_TIMESTAMP) {
        pos += snprintf(buf + pos, max_size - pos, "[%s] ", get_timestamp());
    }
    
    // 添加日志级别标签
    const char* level_str = "UNKNOWN";
    switch (level) {
        case LOG_LEVEL_DEBUG:    level_str = "DEBUG"; break;
        case LOG_LEVEL_INFO:     level_str = "INFO"; break;
        case LOG_LEVEL_WARNING:  level_str = "WARN"; break;
        case LOG_LEVEL_ERROR:    level_str = "ERROR"; break;
        case LOG_LEVEL_CRITICAL: level_str = "CRITICAL"; break;
        default: break;
    }
    pos += snprintf(buf + pos, max_size - pos, "[%s] ", level_str);
    
    // 添加位置信息
    if (default_logger.options & LOG_OPT_FILENAME) {
        pos += snprintf(buf + pos, max_size - pos, "[%s", shorten_filename(file));
        
        if (default_logger.options & LOG_OPT_LINENUM) {
            pos += snprintf(buf + pos, max_size - pos, ":%d", line);
        }
        
        if (default_logger.options & LOG_OPT_FUNCTION) {
            pos += snprintf(buf + pos, max_size - pos, " %s", function);
        }
        
        pos += snprintf(buf + pos, max_size - pos, "] ");
    }
    
    // 处理用户消息
    va_list args;
    va_start(args, fmt);
    pos += vsnprintf(buf + pos, max_size - pos, fmt, args);
    va_end(args);
    
    // 确保以换行符结束
    if (buf[pos-1] != '\n' && pos < max_size - 1) {
        buf[pos++] = '\n';
        buf[pos] = '\0';
    } else if (pos == max_size - 1) {
        // 确保缓冲区截断后有结束符
        buf[sizeof(log_buffer) - 2] = '\n';
        buf[sizeof(log_buffer) - 1] = '\0';
    }
    
    // 调用用户指定的日志回调
    default_logger.callback(level, log_buffer, default_logger.user_data);
}

/* API 实现 */

void log_init(LogLevel min_level, uint32_t options, LogCallback callback, void* user_data) {
    default_logger.min_level = min_level;
    default_logger.options = options;
    default_logger.callback = callback;
    default_logger.user_data = user_data;
}

void log_configure(LogLevel min_level, uint32_t options) {
    default_logger.min_level = min_level;
    default_logger.options = options;
}

void log_set_callback(LogCallback callback, void* user_data) {
    default_logger.callback = callback;
    default_logger.user_data = user_data;
}

LogLevel log_get_level(void) {
    return default_logger.min_level;
}

void log_set_level(LogLevel level) {
    default_logger.min_level = level;
}

uint32_t log_get_options(void) {
    return default_logger.options;
}

void log_set_options(uint32_t options) {
    default_logger.options = options;
}