#include "tm_compat.h"


/**
 * Копирование одной строки в другую с проверкой на длину приемного буфера
 * @param dst Приемный буфер
 * @param src Оригинальная строка
 * @param siz Размер приемного буфера
 * @return Количество скопированных байт
 */
size_t tm_strlcpy(char *dst, const char *src, size_t siz)
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


/**
 * Копирование одной строки указанной длины в другую с проверкой на длину приемного буфера
 * @param dst Приемный буфер
 * @param src Оригинальная строка
 * @param len Длина копируемой строки
 * @param siz Размер приемного буфера
 * @return Количество скопированных байт
 */
size_t tm_strlcpyn(char *dst, const char *src, size_t len, size_t siz)
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
size_t tm_strlen(const char *str)
{
	if (!str)
		return 0;
	return strlen(str);
}
