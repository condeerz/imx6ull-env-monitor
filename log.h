#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

/**
 * @brief 日志级别枚举
 * 定义了不同严重程度的日志级别，数值越大表示越详细的日志信息
 */
typedef enum LogLevel {
    LOG_LEVEL_FATAL,    /**< 致命错误级别，系统无法继续运行 */
    LOG_LEVEL_ERROR,    /**< 错误级别，发生错误但不影响系统运行 */
    LOG_LEVEL_WARN,     /**< 警告级别，潜在的问题或异常情况 */
    LOG_LEVEL_INFO,     /**< 信息级别，一般的信息输出 */
    LOG_LEVEL_DEBUG,    /**< 调试级别，调试相关的信息 */
    LOG_LEVEL_TRACE,    /**< 追踪级别，最详细的追踪信息 */
}LogLevel_t;

extern int LogInit(const char *pLogPath);
extern int LogDeInit(void);
extern void LogSetLevel(LogLevel_t level);
extern int LogWrite(LogLevel_t level, char *pmsg);

#endif