#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>

/* 当前输出模式由应用层 Log_SetLevel() 设置，不是单条日志自己决定。 */
typedef enum {
	LOG_ERROR = 0,  /* 错误日志，只输出文本 */
	LOG_INFO = 1,  /* 全部日志以文本输出 */
	LOG_VOFA = 2,  /* 丢掉 INFO，DEBUG 以 JustFloat 输出给 VOFA+ */
} LogLevel;

#define LOG_VOFA_MAX  8U
#define LOG_MESSAGE_MAX 128U

void Log_SetLevel(LogLevel level);
LogLevel Log_GetLevel(void);

/*
 * 一条日志。fmt 与 printf 相同。
 * level == LOG_INFO  ：按 fmt 输出文本。
 * level == LOG_VOFA ：取出 fmt 里的浮点参数，以 JustFloat 发给 VOFA+。
 * 应用等级为 LOG_INFO 时，两种 level 都按文本输出。
 * 应用等级为 LOG_VOFA 时，INFO 被丢掉，只发 DEBUG 的 JustFloat。
 *
 * 例：Log(LOG_VOFA, "%.3f,%.3f,%.3f,%.3f", set, now, gyro, out);
 */
void Log(LogLevel level, const char *fmt, ...);

#endif /* LOG_H_ */
