/**
 * @file tm_time.c
 * @date 2014
 * @copyright (c) ЗАО "Голлард"
 * @author Роман В. Косинский [armowl]
 * @brief Работа со временем
 * @note В десятичноv времени в файле не производится коррекции на эпоху, если не оговорено иное!
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>

#include "tm_logging.h"
#include "tm_configuration.h"

#include "tm_time.h"

#define TM_TIME_U32_MAX (1UL << 32)
#define TM_TIME_U16_MAX (1UL << 16)

#define	TM_TIME_JAN_1970	(2208988800UL)	/* 1970 - 1900 in seconds */
#define	TM_TIME_JAN_2030	(1893456000UL + TM_TIME_JAN_1970)	/* 1. 1. 2030 00:00:00 */

/**
 * Преобразование NTP аремени в Unixtime (timeval)
 * @param ut Указатель на структуру timeval
 * @param ntp Указатель на структуру NTP времени
 * @return Указатель на структуру timeval (передана в ut)
 */
struct timeval *tm_time_ntp_to_timeval(struct timeval *ut, tm_time_ntp_t *ntp)
{
	ut->tv_sec = ntp->seconds - TM_TIME_JAN_1970; // the seconds from Jan 1, 1900 to Jan 1, 1970
	ut->tv_usec = (uint32_t)((double)ntp->fraction * 1.0e6 / (double)TM_TIME_U32_MAX);
	return ut;
}

/**
 * Преобразование Unixtime (timeval) в NTP
 * @param ntp Указатель на структуру с NTP временем
 * @param ut Указатель на структуру timeval
 * @return Указатель на структуру NTP времени (передан в ntp)
 */
tm_time_ntp_t *tm_time_timeval_to_ntp(tm_time_ntp_t *ntp, struct timeval *ut)
{
	ntp->seconds = ut->tv_sec + TM_TIME_JAN_1970;
	ntp->fraction = (uint32_t)((double)(ut->tv_usec + 1) * (double)TM_TIME_U32_MAX * 1.0e-6);
	return ntp;
}

/**
 * Получение текущего времени с точностью до микросекунд
 * @return Текущее время (начало эпохи: 1 января 1900 00:00:00)
 */
double tm_time_get_current_time(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1)
		TM_LOG_ERROR("Get time (microseconds) failed");
	/* Т.к. в tv секунды с 1970 года, необходимо прибавить секунды с 1900 года */
	return ((double)tv.tv_sec + (double)TM_TIME_JAN_1970 + 1.0e-6 * tv.tv_usec);
}

/**
 * Получение текущего времени с точностью до наносекунд
 * @return Текущее время (начало эпохи: 1 января 1900 00:00:00)
 */
#define HAVE_CLOCKGETTIME 1
double tm_time_get_current_ntime(void)
{
#if HAVE_CLOCKGETTIME
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		TM_LOG_ERROR("Get time (nanoseconds) failed");
	/* Т.к. в tv секунды с 1970 года, необходимо прибавить секунды с 1900 года */
	return ((double)ts.tv_sec + (double)TM_TIME_JAN_1970 + 1.0e-9 * ts.tv_nsec);
#else
#warning "Build without 'clock_gettime'"
	return tm_time_get_current_time();
#endif
}

/**
 * Преобразование NTP времени в десятичное
 * @param ntp Указатель на структуру NTP времени
 * @return Десятичное время (начало эпохи: 1 января 1900 00:00:00)
 */
double tm_time_ntp_to_double(tm_time_ntp_t *ntp)
{
	return ((double)ntp->seconds + ((double)ntp->fraction / (double)TM_TIME_U32_MAX));
}

/**
 * Преобразование десятичного времени в NTP время
 * @param ntp Указатель на структуру для NTP времени
 * @param tm Десятичное время (должно быть с начала эпохи: 1 января 1900 00:00:00)
 * @return Указатель на структуру NTP времени
 */
tm_time_ntp_t *tm_time_double_to_ntp(tm_time_ntp_t *ntp, double tm)
{
	ntp->seconds = (uint32_t)tm;
	ntp->fraction = (uint32_t)((tm - ntp->seconds) * (double)TM_TIME_U32_MAX);
	return ntp;
}

/**
 * Преобразование timeval времени в десятичное
 * @param tv Указатель на структуру времени timeval
 * @return Десятичное время (начало эпохи: 1 января 1900 00:00:00)
 */
double tm_time_timeval_to_double(struct timeval *tv)
{
	return ((double)(tv->tv_sec + TM_TIME_JAN_1970) + 1.0e-6 * (double)tv->tv_usec);
}

/**
 * Преобразование десятичного времени в timeval
 * @param tv Указатель на структуру timeval
 * @param tm Десятичное время (должно быть с начала эпохи: 1 января 1900 00:00:00)
 * @return Указатель на структуру timeval
 */
struct timeval *tm_time_double_to_timeval(struct timeval *tv, double tm)
{
	tv->tv_sec = (uint32_t)tm;
	tv->tv_usec = (uint32_t)((tm - tv->tv_sec) * 1.0e6);
	tv->tv_sec -= TM_TIME_JAN_1970;
	return tv;
}

/**
 * Преобразование десятичного времени из эпохи 1900 в эпоху 1970
 * @param tm Десятичное время (начало эпохи: 1 января 1900 00:00:00)
 * @return Десятичное время (начало эпохи: 1 января 1970 00:00:00)
 */
double tm_time_double_to_double1970(double tm)
{
	return tm - (double)TM_TIME_JAN_1970;
}

/**
 * Преобразование tm_time16 в десятичное
 * @param t16 Указатель на структуру времени tm_time16
 * @return Десятичное время
 */
double tm_time_time16_to_double(tm_time16_t *t16)
{
	return ((double)t16->seconds + ((double)t16->fraction / (double)TM_TIME_U16_MAX));
}

/**
 * Преобразование десятичного времени в tm_time16
 * @param t16 Указатель на структуру tm_time16
 * @param tm Десятичное время
 * @return Указатель на структуру tm_time16
 */
tm_time16_t *tm_time_double_to_time16(tm_time16_t *t16, double tm)
{
	t16->seconds = (uint16_t)tm;
	t16->fraction = (uint16_t)((tm - t16->seconds) * (double)TM_TIME_U16_MAX);
	return t16;
}

/**
 * Конвертирование десятичного времени в формат с сутками (для отсчета времени работы)
 * @param str Строка для результата
 * @param strsize Размер строки для результата
 * @param tim Десятичное время (количество секунд)
 */
void tm_time_double_to_string_with_days(char *str, size_t strsize, double tim)
{
	uint64_t tm = (uint64_t)tim;
	int sutki = tm % 86400;
	snprintf(str, strsize, "%d,%02d:%02d:%02d.%06d", (int)(tm / 86400), sutki / 3600,  (sutki % 3600) / 60, sutki % 60,
			(int)((tim - (double)tm) * 10000000));
}
