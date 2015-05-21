/**
 * @file tm_time.h
 * @date 2014
 * @copyright (c) ЗАО "Голлард"
 * @author Роман В. Косинский [armowl]
 * @brief
 */
#ifndef TM_TIME_H_
#define TM_TIME_H_

#include <sys/time.h>
#include <inttypes.h>

/**
 * Структура NTP времени (32-битные элементы)
 */
typedef struct _tm_time_ntp {
	uint32_t seconds; /*!< Секунды */
	uint32_t fraction; /*!< Fraction */
} tm_time_ntp_t;

/**
 * Структура NTP времени (16-битные элементы)
 */
typedef struct _tm_time16 {
	uint16_t seconds; /*!< Секунды */
	uint16_t fraction; /*!< Fraction */
} tm_time16_t;


struct timeval *tm_time_ntp_to_timeval(struct timeval *ut, tm_time_ntp_t *ntp);
tm_time_ntp_t *tm_time_timeval_to_ntp(tm_time_ntp_t *ntp, struct timeval *ut);

/* Десятичное время */
double tm_time_get_current_time(void);
double tm_time_get_current_ntime(void);
double tm_time_ntp_to_double(tm_time_ntp_t *ntp);
tm_time_ntp_t *tm_time_double_to_ntp(tm_time_ntp_t *ntp, double tm);
double tm_time_timeval_to_double(struct timeval *tv);
struct timeval *tm_time_double_to_timeval(struct timeval *tv, double tm);
double tm_time_time16_to_double(tm_time16_t *t16);
tm_time16_t *tm_time_double_to_time16(tm_time16_t *t16, double tm);
double tm_time_double_to_double1970(double tm);
void tm_time_double_to_string_with_days(char *str, size_t strsize, double tim);

double tm_time_get_bitrate(double t2, double t1, int block_size);

#endif /* TM_TIME_H_ */
