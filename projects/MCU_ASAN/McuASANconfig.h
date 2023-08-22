/*
 * Copyright (c) 2023, anonymous author
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Based on Erich's code
 * 
 */

#ifndef MCUASANCONFIG_H_
#define MCUASANCONFIG_H_

#define GCC_VERSION (__GNUC__ * 10000          \
                      + __GNUC_MINOR__ * 100   \
                      + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 70000
# define KASAN_ABI_VERSION 5
#else
# define KASAN_ABI_VERSION 4
#endif

extern unsigned char __DATA_RAM[];
extern unsigned char __HeapBase[];
extern unsigned char __HeapLimit[];

extern unsigned char shadow[];
extern unsigned char __ShadowLimit[];

extern unsigned char __StackTop[];
extern unsigned char __StackLimit[];

#ifndef McuASAN_CONFIG_IS_ENABLED
  #define McuASAN_CONFIG_IS_ENABLED     (1)
  /*!< 1: ASAN is enabled; 0: ASAN is disabled */
#endif

#ifndef McuASAN_SHADOW_SCALE_SHIFT
  #define McuASAN_SHADOW_SCALE_SHIFT 3
#endif

#ifndef McuASAN_SHADOW_SCALE_SIZE
  #define McuASAN_SHADOW_SCALE_SIZE (1UL << McuASAN_SHADOW_SCALE_SHIFT)
#endif

#ifndef McuASAN_SHADOW_MASK
  #define McuASAN_SHADOW_MASK (McuASAN_SHADOW_SCALE_SIZE - 1)
#endif

#ifndef McuASAN_CONFIG_CHECK_MALLOC_FREE
  #define McuASAN_CONFIG_CHECK_MALLOC_FREE  (1)
  /*!< 1: check malloc() and free() */
#endif

#ifndef McuASAN_CONFIG_APP_MEM_START
  // #define McuASAN_CONFIG_APP_MEM_START RAM_REGION_BEGIN
  #define McuASAN_CONFIG_APP_MEM_START (unsigned long)__DATA_RAM
  /*!< base RAM address */
#endif

#ifndef McuASAN_CONFIG_APP_MEM_SIZE
  #define McuASAN_CONFIG_APP_MEM_SIZE  (unsigned long)(shadow - __DATA_RAM)
  // #define McuASAN_CONFIG_APP_MEM_SIZE (16 * 1024)
  /*!< Memory size in bytes */
#endif

#if McuASAN_CONFIG_CHECK_MALLOC_FREE
#ifndef McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER
  #define McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER  (8)
  /*!< red zone border in bytes around memory blocks. Must be larger than sizeof(size_t)! */
#endif

#ifndef McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE
  #define McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE  (8)
  /*!< list of free blocks in quarantine until they are released. Use 0 for no list. */
#endif

#endif /* McuASAN_CONFIG_CHECK_MALLOC_FREE */

#endif /* MCUASANCONFIG_H_ */