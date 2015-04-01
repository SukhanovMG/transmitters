#ifndef _TM_COMPAT_H
#define _TM_COMPAT_H

#include <stdlib.h>
#include <string.h>

#if !defined(HAVE_STRLCAT)
extern size_t tm_strlcat(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRLCPY)
extern size_t tm_strlcpy(char *dst, const char *src, size_t siz);
#endif

size_t tm_strlcpyn(char *dst, const char *src, size_t len, size_t siz);
size_t tm_strlen(const char *str);

#endif /* _TM_COMPAT_H */
