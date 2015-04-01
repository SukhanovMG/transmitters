/**
 * @file common/transcoder_compat.c
 * @date 2014
 * @copyright (c) ЗАО "Голлард"
 * @author Роман В. Косинский [armowl]
 * @brief Функции работы со строками
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <src/multi-thread/copy-buffer/compat.h>

#if !defined(HAVE_STRLCAT)
/**
 * Дополнение одной строки другой
 * @param dst Приемный буфер
 * @param src Дополняющая строка строка
 * @param siz Размер приемного буфера
 * @return Количество скопированных байт
 */
size_t transcoder_strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	if (!dst || !src)
		return 0;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return (dlen + transcoder_strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return (dlen + (s - src)); /* count does not include NUL */
}
#else
#define transcoder_strlcat strlcat
#endif

#if !defined(HAVE_STRLCPY)
/**
 * Копирование одной строки в другую с проверкой на длину приемного буфера
 * @param dst Приемный буфер
 * @param src Оригинальная строка
 * @param siz Размер приемного буфера
 * @return Количество скопированных байт
 */
size_t transcoder_strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	if (!dst || !src)
		return 0;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0'; /* NUL-terminate dst */
	}

	return (s - src); /* count does not include NUL */
}
#else
#define transcoder_strlcpy strlcpy
#endif

/**
 * Копирование одной строки указанной длины в другую с проверкой на длину приемного буфера
 * @param dst Приемный буфер
 * @param src Оригинальная строка
 * @param len Длина копируемой строки
 * @param siz Размер приемного буфера
 * @return Количество скопированных байт
 */
size_t transcoder_strlcpyn(char *dst, const char *src, size_t len, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz < len ? siz : len + 1;

	if (!dst || !src)
		return 0;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0'; /* NUL-terminate dst */
	}

	return (s - src); /* count does not include NUL */
}

/**
 * Длина строки с бозопасной работой при нулевом указателе
 * @param str Указатель на строку
 * @return Длина строки (при str == NULL - 0)
 */
size_t transcoder_strlen(const char *str)
{
	if (!str)
		return 0;
	return strlen(str);
}
