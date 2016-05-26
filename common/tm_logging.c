/**
 * Функции работы с логированием
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "tm_time.h"

#include "tm_logging.h"

/**
 * Инициализация логирования
 * @param prefix Префик перед сообщениями лога
 * @return @ref TMLogStatus
 */
TMLogStatus tm_log_init(const char *prefix)
{
	int logopt = LOG_NDELAY | LOG_PERROR | LOG_PID;
	int facility = LOG_DAEMON;
	openlog(prefix, logopt, facility);
	return TMLogStatus_SUCCESS;
}

/**
 * Уничтожение логирования
 * @return @ref TMLogStatus
 */
TMLogStatus tm_log_destroy()
{
	closelog();
	return TMLogStatus_SUCCESS;
}

/**
 * @brief вывод бинарной информации в лог
 * @param data данные для вывода
 * @param datalen длина данных
 * @return @ref TMLogStatus
 */
TMLogStatus tm_log_raw(const char *data, size_t datalen)
{
	char raw_buf[49];
	char char_buf[17];
	int i, j;

	syslog(LOG_DEBUG, "   PRINTED %d byte(s)", (int)datalen);
	for(i = 0; i < (datalen / 16) + ((datalen % 16) ? 1 : 0); i++) {
		memset(raw_buf, 0, 49);
		memset(char_buf, 0, 17);
		for(j = 0; j < (((datalen - (16 * i)) > 16) ? 16 : (datalen - (16 * i))); j++) {
			snprintf(raw_buf + (j * 3), sizeof(raw_buf), "%02X ", (unsigned char)*(data + j + i * 16));
			char_buf[j] = (isprint(*(data + j + i * 16)) ? (*(data + j + i * 16)) : '.');
		}
		syslog(LOG_DEBUG, "   %08x: %-48s| %s", i * 16, raw_buf, char_buf);
	}
	return TMLogStatus_SUCCESS;
}

/**
 * Печать строки в лог
 * @param file Имя файла для печати
 * @param line Номер строки в файле
 * @param function Название функции
 * @param facility Тип сообщения
 * @param prefix Префикс
 * @param format Строка формата сообщения
 */
void _tm_log_string(const char *file, const char *line, const char *function, TMLogTypes facility, const char *prefix,
		const char *format, ...)
{
	va_list vl;
	char *filen = NULL;
	struct tm tm;
	char ts[40] = "";
	char fmt[1024] = "";
	const char *lprefix = prefix ? prefix : "";
	struct timeval tv;
	pthread_t th = pthread_self();

	if(file && (filen = strrchr(file, '/')))
		filen++;

	tm_time_double_to_timeval(&tv, tm_time_get_current_time());
	localtime_r(&tv.tv_sec, &tm);
	strftime(ts, sizeof(ts), "%02d.%02m.%y-%02H:%02M:%02S", &tm);
	snprintf(ts + strlen(ts), sizeof(ts) - strlen(ts), ".%06d|", (int)tv.tv_usec);

	va_start(vl, format);
	switch(facility) {
		case TMLogTypes_INFO:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s %s", ts, format);
				vsyslog(LOG_INFO, fmt, vl);
			} else
				syslog(LOG_INFO, "%s", ts);
			break;
		case TMLogTypes_ERROR:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s  E %s%s", ts, lprefix, format);
				vsyslog(LOG_ERR, fmt, vl);
			} else
				syslog(LOG_ERR, "%s  E %s", ts, lprefix);
			syslog(LOG_DEBUG, "%s  T %s[%s]: ERROR in %s", ts, filen, line, function);
			break;
		case TMLogTypes_CRIT:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s CE %s%s", ts, lprefix, format);
				vsyslog(LOG_CRIT, fmt, vl);
			} else
				syslog(LOG_ERR, "%s CE %s", ts, lprefix);
			syslog(LOG_DEBUG, "%s  T %s[%s]: CRITICAL in %s", ts, filen, line, function);
			break;
		case TMLogTypes_WARN:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s  W %s%s", ts, lprefix, format);
				vsyslog(LOG_CRIT, fmt, vl);
			} else
				syslog(LOG_CRIT, "%s  W %s", ts, lprefix);
			syslog(LOG_DEBUG, "%s  T %s[%s]: WARNING in %s", ts, filen, line, function);
			break;
		case TMLogTypes_TRACE:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s  T %s[%s]: %s%s", ts, filen, line, lprefix, format);
				vsyslog(LOG_DEBUG, fmt, vl);
			} else
				syslog(LOG_DEBUG, "%s  T %s[%s]: %s", ts, filen, line, lprefix);
			break;
		case TMLogTypes_DEBUG:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s  D %s[%s]: %s%s", ts, filen, line, lprefix, format);
				vsyslog(LOG_DEBUG, fmt, vl);
			} else
				syslog(LOG_DEBUG, "%s  D %s[%s]: %s", ts, filen, line, lprefix);
			break;
		case TMLogTypes_STRACE:
			syslog(LOG_DEBUG, "%s ST %s[%s]:%x:%s", ts, filen, line, (unsigned int)th, function);
			break;
		case TMLogTypes_TSTRACE:
			if(format) {
				snprintf(fmt, sizeof(fmt), "%s ST %s[%s]:%x:%s%s", ts, filen, line, (unsigned int)th, format, function);
				vsyslog(LOG_DEBUG, fmt, vl);
			} else
				syslog(LOG_DEBUG, "%s ST %s[%s]:%x:%s", ts, filen, line, (unsigned int)th, function);
			break;
	}
	va_end(vl);
}
