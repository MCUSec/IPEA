/*
 * Copyright (c) 2023, anonymous author
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Based on Erich's code
 * 
 */

#ifndef MCUASAN_H_
#define MCUASAN_H_

#include <stddef.h>
#include "McuASANconfig.h"

#if McuASAN_CONFIG_IS_ENABLED && McuASAN_CONFIG_CHECK_MALLOC_FREE
  /* replace malloc and free calls */
  #define malloc   __asan_malloc
  #define free     __asan_free
#endif

#undef memset
#define memset(addr, c, len) __asan_memset((addr), (c), (len))

#undef memmove
#define memmove(dest, src, len) __asan_memmove((dest), (src), (len))

#undef memcpy
#define memcpy(dest, src, len) __asan_memcpy((dest), (src), (len))

#undef strcpy
#define strcpy(dest, src) __asan_strcpy((dest), (src))

#undef strncpy
#define strncpy(dest, src, n) __asan_strncpy((dest), (src), (n))

#undef strcat
#define strcat(dest, src) __asan_strcat((dest), (src))

#undef strncat
#define strncat(dest, src, n) __asan_strncat((dest), (src), (n))

#ifdef __cplusplus
extern "C" {
#endif

void *__asan_memset(void *addr, int c, size_t len);

void *__asan_memmove(void *dest, const void *src, size_t len);

void *__asan_memcpy(void *dest, const void *src, size_t len);

void *__asan_strcpy(char *dest, const char *src);

void *__asan_strncpy(char *dest, const char *src, size_t n);

void *__asan_strcat(char *dest, const char *src);

void *__asan_strncat(char *dest, const char *src, size_t n);

int __asan_sprintf(char *str, const char *format, ...);

int __asan_snprintf(char *str, size_t size, const char *format, ...);

/*! \brief
 * Allocate a memory block
 */
void *__wrap_malloc(size_t size);

/*!
 * \brief
 * Free a memory block
 */
void __wrap_free(void *p);

/*! \brief
 * Call the init function first to initialize the module.
 */
void McuASAN_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif /* MCUASAN_H_ */