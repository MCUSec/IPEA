#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>

#include "libusan_conf.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern void __usan_trace_load_store(uint32_t tar_id, uint32_t addr, uint32_t size);

EXPORT void *__usan_memcpy(void *dest, const void *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __usan_trace_load_store(src_id, (uint32_t)src, n);
    __usan_trace_load_store(dest_id, (uint32_t)dest, n);
    return memcpy(dest, src, n);
}

EXPORT void *__usan_memmove(void *dest, const void *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __usan_trace_load_store(src_id, (uint32_t)src, n);
    __usan_trace_load_store(dest_id, (uint32_t)dest, n);
    return memmove(dest, src, n);
}

EXPORT void *__usan_memset(void *s, int c, size_t n, uint32_t dest_id)
{
    __usan_trace_load_store(dest_id, (uint32_t)s, n);
    return memset(s, c, n);
}

EXPORT char *__usan_strcat(char *dest, const char *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = strlen(src) + 1;
    const size_t dst_len = strlen(dest);
    __usan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strcat(dest, src);
}

EXPORT char *__usan_strncat(char *dest, const char *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n, strlen(src) + 1);
    const size_t dst_len = strlen(dest);
    __usan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strncat(dest, src, n);
}

EXPORT char *__usan_strcpy(char *dest, const char *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = strlen(src) + 1;
    __usan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strcpy(dest, src);
}

EXPORT char *__usan_strncpy(char *dest, const char *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n, strlen(src) + 1);
    __usan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strncpy(dest, src, n);
}

EXPORT size_t __usan_strlen(const char *str, uint32_t ptr_id)
{
    size_t len = strlen(str);
    __usan_trace_load_store(ptr_id, (uint32_t)str, len);
    return len;
}

EXPORT wchar_t *__usan_wcscat(wchar_t *dest, const wchar_t *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = wcslen(src) + sizeof(wchar_t);
    const size_t dst_len = wcslen(dest);
    __usan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcscat(dest, src);
}

EXPORT wchar_t *__usan_wcsncat(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n * sizeof(wchar_t), wcslen(src) + sizeof(wchar_t));
    const size_t dst_len = wcslen(dest);
    __usan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcsncat(dest, src, n);
}

EXPORT wchar_t *__usan_wcscpy(wchar_t *dest, const wchar_t *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = wcslen(src) + sizeof(wchar_t);
    __usan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcscpy(dest, src);
}

EXPORT wchar_t *__usan_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n * sizeof(wchar_t), wcslen(src) + sizeof(wchar_t));
    __usan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __usan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcsncpy(dest, src, n);
}

EXPORT size_t __usan_wcslen(const wchar_t *str, uint32_t ptr_id)
{
    size_t len = wcslen(str);
    __usan_trace_load_store(ptr_id, (uint32_t)str, len);
    return len;
}

EXPORT wchar_t *__usan_wmemcpy(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __usan_trace_load_store(src_id, (uint32_t)src, n);
    __usan_trace_load_store(dest_id, (uint32_t)dest, n);
    return wmemcpy(dest, src, n);
}

EXPORT wchar_t *__usan_wmemmove(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __usan_trace_load_store(src_id, (uint32_t)src, n);
    __usan_trace_load_store(dest_id, (uint32_t)dest, n);
    return wmemmove(dest, src, n);
}

EXPORT wchar_t *__usan_wmemset(wchar_t *wcs, wchar_t wc, size_t n, uint32_t dest_id)
{
    __usan_trace_load_store(dest_id, (uint32_t)wcs, n);
    return wmemset(wcs, wc, n);
}