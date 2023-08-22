/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCUASAN_H_
#define MCUASAN_H_

#include <stddef.h>

  /* replace malloc and free calls */
#define malloc   __asan_malloc
#define free     __asan_free


#undef memset
#define memset __asan_memset

#undef memmove
#define memmove __asan_memmove

#undef memcpy
#define memcpy __asan_memcpy

#undef strcpy
#define strcpy __asan_strcpy

#undef strncpy
#define strncpy __asan_strncpy

#undef strcat
#define strcat __asan_strcat

#undef strncat
#define strncat __asan_strncat

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
void *__asan_malloc(size_t size);

/*!
 * \brief
 * Free a memory block
 */
void __asan_free(void *p);

/*! \brief
 * Call the init function first to initialize the module.
 */
void McuASAN_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif /* MCUASAN_H_ */