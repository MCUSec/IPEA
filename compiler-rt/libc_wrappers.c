/**
 * @file libc_wrappers.c
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Implementation of libc wrappers
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>

#include "libipeasan_conf.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern void __ipeasan_trace_load_store(uint32_t tar_id, uint32_t addr, uint32_t size);

/**
 * @brief Wrapper of memcpy()
 * 
 * @param dest
 * @param src
 * @param n
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of memcpy() 
 */
EXPORT void *__ipeasan_memcpy(void *dest, const void *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __ipeasan_trace_load_store(src_id, (uint32_t)src, n);
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, n);
    return memcpy(dest, src, n);
}

/**
 * @brief Wapper of memmove()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of memmove()
 */
EXPORT void *__ipeasan_memmove(void *dest, const void *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __ipeasan_trace_load_store(src_id, (uint32_t)src, n);
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, n);
    return memmove(dest, src, n);
}

/**
 * @brief Wrapper of memset()
 * 
 * @param s 
 * @param c 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @return return value of memset()
 */
EXPORT void *__ipeasan_memset(void *s, int c, size_t n, uint32_t dest_id)
{
    __ipeasan_trace_load_store(dest_id, (uint32_t)s, n);
    return memset(s, c, n);
}

/**
 * @brief Wrapper of strcat()
 * 
 * @param dest 
 * @param src 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of strcat()
 */
EXPORT char *__ipeasan_strcat(char *dest, const char *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = strlen(src) + 1;
    const size_t dst_len = strlen(dest);
    __ipeasan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strcat(dest, src);
}

/**
 * @brief Wrapper of strncat()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of strncat()
 */
EXPORT char *__ipeasan_strncat(char *dest, const char *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n, strlen(src) + 1);
    const size_t dst_len = strlen(dest);
    __ipeasan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strncat(dest, src, n);
}

/**
 * @brief Wrapper of strcpy()
 * 
 * @param dest 
 * @param src 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of strcpy()
 */
EXPORT char *__ipeasan_strcpy(char *dest, const char *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = strlen(src) + 1;
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strcpy(dest, src);
}

/**
 * @brief Wrapper of strncpy()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of strncpy()
 */
EXPORT char *__ipeasan_strncpy(char *dest, const char *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n, strlen(src) + 1);
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return strncpy(dest, src, n);
}

/**
 * @brief Wrapper of strlen()
 * 
 * @param str 
 * @param ptr_id Pointer ID of str
 * @return return value of strlen()
 */
EXPORT size_t __ipeasan_strlen(const char *str, uint32_t ptr_id)
{
    size_t len = strlen(str);
    __ipeasan_trace_load_store(ptr_id, (uint32_t)str, len);
    return len;
}

/**
 * @brief Wrapper of wcscat()
 * 
 * @param dest 
 * @param src 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of wcscat()
 */
EXPORT wchar_t *__ipeasan_wcscat(wchar_t *dest, const wchar_t *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = wcslen(src) + sizeof(wchar_t);
    const size_t dst_len = wcslen(dest);
    __ipeasan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcscat(dest, src);
}

/**
 * @brief Wrapper of wcsncat()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of wcsncat()
 */
EXPORT wchar_t *__ipeasan_wcsncat(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n * sizeof(wchar_t), wcslen(src) + sizeof(wchar_t));
    const size_t dst_len = wcslen(dest);
    __ipeasan_trace_load_store(dest_id, (uint32_t)(dest + dst_len), src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcsncat(dest, src, n);
}

/**
 * @brief Wrapper of wcscpy()
 * 
 * @param dest 
 * @param src 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of wcscpy()
 */
EXPORT wchar_t *__ipeasan_wcscpy(wchar_t *dest, const wchar_t *src, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = wcslen(src) + sizeof(wchar_t);
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcscpy(dest, src);
}

/**
 * @brief Wrapper of wcsncpy()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of wcsncpy()
 */
EXPORT wchar_t *__ipeasan_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    const size_t src_len = MIN(n * sizeof(wchar_t), wcslen(src) + sizeof(wchar_t));
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, src_len);
    __ipeasan_trace_load_store(src_id, (uint32_t)src, src_len);
    return wcsncpy(dest, src, n);
}

/**
 * @brief Wrapper of wcslen()
 * 
 * @param str 
 * @param ptr_id Pointer ID of str
 * @return return value of wcslen()
 */
EXPORT size_t __ipeasan_wcslen(const wchar_t *str, uint32_t ptr_id)
{
    size_t len = wcslen(str);
    __ipeasan_trace_load_store(ptr_id, (uint32_t)str, len);
    return len;
}

/**
 * @brief Wrapper of wmemcpy()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of wmemcpy()
 */
EXPORT wchar_t *__ipeasan_wmemcpy(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __ipeasan_trace_load_store(src_id, (uint32_t)src, n);
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, n);
    return wmemcpy(dest, src, n);
}

/**
 * @brief Wrapper of wmemmove()
 * 
 * @param dest 
 * @param src 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @param src_id Pointer ID of src
 * @return return value of wmemmove()
 */
EXPORT wchar_t *__ipeasan_wmemmove(wchar_t *dest, const wchar_t *src, size_t n, uint32_t dest_id, uint32_t src_id)
{
    __ipeasan_trace_load_store(src_id, (uint32_t)src, n);
    __ipeasan_trace_load_store(dest_id, (uint32_t)dest, n);
    return wmemmove(dest, src, n);
}

/**
 * @brief Wrapper of wmemset()
 * 
 * @param wcs 
 * @param wc 
 * @param n 
 * @param dest_id Pointer ID of dest
 * @return return value of wmemset()
 */
EXPORT wchar_t *__ipeasan_wmemset(wchar_t *wcs, wchar_t wc, size_t n, uint32_t dest_id)
{
    __ipeasan_trace_load_store(dest_id, (uint32_t)wcs, n);
    return wmemset(wcs, wc, n);
}