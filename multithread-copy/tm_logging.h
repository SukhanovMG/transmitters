/**
 * @file tm_logging.h
 * @date 2014-2015
 * @copyright (c) ЗАО "Голлард"
 * @author Олег В. Карпов
 * @author Роман В. Косинский [armowl]
 * @brief Функции работы с логированием
 */

#ifndef LOG4CEXT_H
#define LOG4CEXT_H

#include <syslog.h>
#include <string.h>
#include <pthread.h>

#ifndef TM_LOG_RAW_ENABLED
#define TM_LOG_RAW_ENABLED 1
#endif

/**
 * Перечислитель с кодами возврата функций логирования
 */
typedef enum _TMLogStatus {
	TMLogStatus_SUCCESS = 0, /*!< Успешное выполнение */
	TMLogStatus_ERROR /*!< Ошибка выполнения */
} TMLogStatus;

typedef enum _TMLogTypes {
	TMLogTypes_INFO,
	TMLogTypes_ERROR,
	TMLogTypes_CRIT,
	TMLogTypes_WARN,
	TMLogTypes_TRACE,
	TMLogTypes_DEBUG,
	TMLogTypes_STRACE,
	TMLogTypes_TSTRACE
} TMLogTypes;

TMLogStatus tm_log_init(const char *prefix);
TMLogStatus tm_log_destroy();
TMLogStatus tm_log_raw(const char *data, size_t datalen);
void _tm_log_string(const char *file, const char *line, const char *function, TMLogTypes facility, const char *prefix,
		const char *format, ...);

#define TM_LOG_STR(x) # x
#define TM_LOG_STRX(x) TM_LOG_STR(x)
#define TM_LOG_PREFIX ""

#define TM_LOG_INFO(...)    _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_INFO,    TM_LOG_PREFIX, "" __VA_ARGS__)
#define TM_LOG_ERROR(...)   _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_ERROR,   TM_LOG_PREFIX, "" __VA_ARGS__)
#define TM_LOG_CRIT(...)    _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_CRIT,    TM_LOG_PREFIX, "" __VA_ARGS__)
#define TM_LOG_WARN(...)    _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_WARN,    TM_LOG_PREFIX, "" __VA_ARGS__)
#define TM_LOG_TRACE(...)   _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_TRACE,   TM_LOG_PREFIX, "" __VA_ARGS__)
#define TM_LOG_STRACE()     _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_STRACE,  TM_LOG_PREFIX, NULL)
#define TM_LOG_TSTRACE(id)  _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_TSTRACE, TM_LOG_PREFIX, "%s%s", ((id) ? id : ""), ((id) ? ":" : ""))
#define TM_LOG_DEBUG(...)   _tm_log_string(__FILE__, TM_LOG_STRX(__LINE__), __FUNCTION__, TMLogTypes_DEBUG,   TM_LOG_PREFIX, "" __VA_ARGS__)

#if TM_LOG_RAW_ENABLED
#define TM_LOG_RAW(data, datalen) tm_log_raw((data), (datalen))
#else
#define TM_LOG_RAW(data, datalen)
#endif

#endif
