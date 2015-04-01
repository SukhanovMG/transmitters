/**
 * @file common/transcoder_compat.h
 * @date 2014
 * @copyright (c) ЗАО "Голлард"
 * @author Роман В. Косинский [armowl]
 * @brief Функции работы со строками
 */

#ifndef _TRANSCODER_COMPAT_H
#define _TRANSCODER_COMPAT_H

#include <stdlib.h>
#include <string.h>

#if !defined(HAVE_STRLCAT)
extern size_t transcoder_strlcat(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRLCPY)
extern size_t transcoder_strlcpy(char *dst, const char *src, size_t siz);
#endif

size_t transcoder_strlcpyn(char *dst, const char *src, size_t len, size_t siz);
size_t transcoder_strlen(const char *str);

#endif /* _TRANSCODER_COMPAT_H */
