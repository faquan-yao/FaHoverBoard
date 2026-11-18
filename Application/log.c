#include "log.h"

#include "Vofa.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static LogLevel s_level = LOG_INFO;

void Log_SetLevel(LogLevel level)
{
	if (level == LOG_INFO || level == LOG_VOFA) {
		s_level = level;
	}
}

LogLevel Log_GetLevel(void)
{
	return s_level;
}

/* Walk a printf format and pull the floating arguments (promoted to double). */
static uint8_t Log_TakeFloats(const char *fmt, va_list ap, float *out, uint8_t max)
{
	uint8_t n = 0U;
	const char *p = fmt;

	while (*p != '\0') {
		int long_double = 0;
		int is_long = 0;
		int is_longlong = 0;
		char spec;

		if (*p != '%') {
			p++;
			continue;
		}
		p++;
		if (*p == '%') {
			p++;
			continue;
		}
		while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
			p++;
		}
		if (*p == '*') {
			(void)va_arg(ap, int);
			p++;
		} else {
			while (*p >= '0' && *p <= '9') {
				p++;
			}
		}
		if (*p == '.') {
			p++;
			if (*p == '*') {
				(void)va_arg(ap, int);
				p++;
			} else {
				while (*p >= '0' && *p <= '9') {
					p++;
				}
			}
		}
		if (*p == 'l') {
			p++;
			if (*p == 'l') {
				is_longlong = 1;
				p++;
			} else {
				is_long = 1;
			}
		} else if (*p == 'h') {
			p++;
			if (*p == 'h') {
				p++;
			}
		} else if (*p == 'L') {
			long_double = 1;
			p++;
		} else if (*p == 'z' || *p == 't' || *p == 'j') {
			is_long = 1;
			p++;
		}
		spec = *p;
		if (spec == '\0') {
			break;
		}
		p++;

		if (spec == 'f' || spec == 'F' || spec == 'g' || spec == 'G' ||
		    spec == 'e' || spec == 'E' || spec == 'a' || spec == 'A') {
			float value;
			if (long_double) {
				value = (float)va_arg(ap, long double);
			} else {
				value = (float)va_arg(ap, double);
			}
			if (n < max) {
				out[n++] = value;
			}
		} else if (spec == 's') {
			(void)va_arg(ap, char *);
		} else if (spec == 'c') {
			(void)va_arg(ap, int);
		} else if (spec == 'p') {
			(void)va_arg(ap, void *);
		} else if (spec == 'd' || spec == 'i' || spec == 'u' || spec == 'x' ||
		           spec == 'X' || spec == 'o') {
			if (is_longlong) {
				(void)va_arg(ap, long long);
			} else if (is_long) {
				(void)va_arg(ap, long);
			} else {
				(void)va_arg(ap, int);
			}
		}
	}
	return n;
}

static void Log_PrintText(LogLevel level, const char *fmt, va_list ap)
{
	char buf[LOG_MESSAGE_MAX];
	const char *prefix = "";
	size_t prefix_len;
	size_t room;
	int n;

	if (level == LOG_ERROR) {
		prefix = "ERROR: ";
	} else if (level == LOG_VOFA) {
		prefix = "VOFA: ";
	}
	prefix_len = strlen(prefix);
	room = sizeof(buf) - prefix_len;
	if (room < 2U) {
		return;
	}
	memcpy(buf, prefix, prefix_len);
	n = vsnprintf(buf + prefix_len, room, fmt, ap);
	if (n < 0) {
		return;
	}
	if ((size_t)n >= room) {
		n = (int)room - 1;
	}
	n += (int)prefix_len;
	if (n == 0 || buf[n - 1] != '\n') {
		if (n >= (int)sizeof(buf) - 1) {
			n = (int)sizeof(buf) - 2;
		}
		buf[n++] = '\n';
		buf[n] = '\0';
	}
	printf("%s", buf);
}

void Log(LogLevel level, const char *fmt, ...)
{
	va_list ap;

	if (fmt == NULL) {
		return;
	}
	if (s_level == LOG_ERROR || s_level == LOG_INFO) {
		va_start(ap, fmt);
		Log_PrintText(level, fmt, ap);
		va_end(ap);
	} else if (s_level == LOG_VOFA) {
		if (level == LOG_ERROR) {
			va_start(ap, fmt);
			Log_PrintText(level, fmt, ap);
			va_end(ap);
		} else if (level == LOG_VOFA) {
			float ch[LOG_VOFA_MAX];
			uint8_t n;

			va_start(ap, fmt);
			n = Log_TakeFloats(fmt, ap, ch, LOG_VOFA_MAX);
			va_end(ap);
			if (n > 0U) {
				Vofa_JustFloat_Send(n, ch);
			}
		} else {
			/* DEBUG 模式下丢掉 INFO */
		}
	}
}
