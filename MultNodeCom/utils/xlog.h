#ifndef X_LOG_H
#define X_LOG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* 日志级别枚举 */
typedef enum {
    LOG_LEVEL_DEBUG,    // 调试信息，通常只在开发中使用
    LOG_LEVEL_INFO,     // 普通运行信息
    LOG_LEVEL_WARNING,  // 警告，不影响正常运行
    LOG_LEVEL_ERROR,    // 错误，需要关注
    LOG_LEVEL_CRITICAL, // 严重错误，可能导致崩溃
    LOG_LEVEL_NONE      // 无日志输出
} LogLevel;

/* 日志选项标志 */
typedef enum {
    LOG_OPT_TIMESTAMP    = (1 << 0),  // 包含时间戳
    LOG_OPT_FILENAME     = (1 << 1),  // 包含源文件名
    LOG_OPT_LINENUM      = (1 << 2),  // 包含行号
    LOG_OPT_FUNCTION     = (1 << 3),  // 包含函数名
    LOG_OPT_THREAD_SAFE  = (1 << 4)   // 线程安全输出
} LogOptions;

/* 日志回调类型 */
typedef void (*LogCallback)(LogLevel level, const char* message, void* user_data);

/* 日志上下文结构 */
typedef struct {
    LogLevel min_level;     // 最小日志级别 (只输出等于或高于此级别的日志)
    uint32_t options;       // 日志选项标志组合
    LogCallback callback;   // 日志输出函数
    void* user_data;        // 用户数据，传递给回调函数
} Logger;

/* 全局默认记录器 */
static Logger default_logger = {
    .min_level = LOG_LEVEL_INFO,
    .options = LOG_OPT_TIMESTAMP,
    .callback = NULL,
    .user_data = NULL
};

/* API 函数 */

// 初始化日志系统
void log_init(LogLevel min_level, uint32_t options, LogCallback callback, void* user_data);

// 配置日志选项
void log_configure(LogLevel min_level, uint32_t options);

// 设置日志回调
void log_set_callback(LogCallback callback, void* user_data);

// 获取当前日志级别
LogLevel log_get_level(void);

// 设置日志级别
void log_set_level(LogLevel level);

// 获取当前日志选项
uint32_t log_get_options(void);

// 设置日志选项
void log_set_options(uint32_t options);

// 核心日志函数 (实际实现应放在 .c 文件中)
void log_message(LogLevel level, const char* file, const char* function, int line, const char* fmt, ...);

/* 数组日志函数声明 */
void log_array(LogLevel level, const char* file, const char* function, int line,
               const void* arr, size_t count, size_t elem_size);

/* 简化调用的宏 */
#define LOG_ARRAY(level, arr, count, elem_size) \
    if (default_logger.min_level <= level) \
        log_array(level, __FILE__, __FUNCTION__, __LINE__, arr, count, elem_size)

/* 常用类型特化宏 */
#define LOG_BYTE_ARRAY(level, arr, count) \
    LOG_ARRAY(level, arr, count, sizeof(uint8_t))

#define LOG_INT_ARRAY(level, arr, count) \
    LOG_ARRAY(level, arr, count, sizeof(int))

#define LOG_FLOAT_ARRAY(level, arr, count) \
    LOG_ARRAY(level, arr, count, sizeof(float))

/* 日志宏 - 自动捕获位置信息 */
#define LOG_DEBUG(fmt, ...) \
    do { \
        if (default_logger.min_level <= LOG_LEVEL_DEBUG) \
            log_message(LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_INFO(fmt, ...) \
    do { \
        if (default_logger.min_level <= LOG_LEVEL_INFO) \
            log_message(LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_WARN(fmt, ...) \
    do { \
        if (default_logger.min_level <= LOG_LEVEL_WARNING) \
            log_message(LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_ERROR(fmt, ...) \
    do { \
        if (default_logger.min_level <= LOG_LEVEL_ERROR) \
            log_message(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
    } while(0)

#define LOG_CRITICAL(fmt, ...) \
    do { \
        if (default_logger.min_level <= LOG_LEVEL_CRITICAL) \
            log_message(LOG_LEVEL_CRITICAL, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
    } while(0)

#endif /* X_LOG_H */