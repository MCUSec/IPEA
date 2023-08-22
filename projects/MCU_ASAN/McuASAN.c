/*
 * Copyright (c) 2023, anonymous author
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Based on Erich's code
 * 
 */

#include "McuASANconfig.h"
#if McuASAN_CONFIG_IS_ENABLED
// #include "McuASAN.h"
// #include "McuLog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define MIN(a, b) (a) > (b) ? (b) : (a)
#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)

#define __round_mask(x, y) ((typeof(x))((y) - 1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y)) + 1)


/* The layout of struct dictated by compiler */
struct kasan_source_location {
	const char *filename;
	int line_no;
	int column_no;
};

struct kasan_global {
  const void *beg;
  size_t size;
  size_t size_with_redzone;
  const void *name;
  const void *module_name;
  unsigned long has_dynamic_init;
#if KASAN_ABI_VERSION >= 4
  struct kasan_source_location *location;
#endif
#if KASAN_ABI_VERSION >= 5
  char *odr_indicator;
#endif
};

typedef enum {
  kIsWrite, /* write access */
  kIsRead,  /* read access */
} rw_mode_e;

#if McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE > 0
static void *freeQuarantineList[McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE];
/*!< list of free'd blocks in quarantine */
static int freeQuarantineListIdx; /* index in list (ring buffer), points to free element in list */
#endif

static bool asan_enabled = 0;

extern void *__real_malloc(size_t);
extern void __real_free(void *);

/* see https://github.com/gcc-mirror/gcc/blob/master/libsanitizer/asan/asan_interface_internal.h */
// static uint8_t shadow[McuASAN_CONFIG_APP_MEM_SIZE/8]; /* one shadow byte for 8 application memory bytes. A 1 means that the memory address is poisoned */
static inline uint8_t *kasan_mem_to_shadow(const void *address) 
{
  unsigned long offset = (unsigned long)address - McuASAN_CONFIG_APP_MEM_START;
  return shadow + (offset >> McuASAN_SHADOW_SCALE_SHIFT); /* divided by 8: every byte has a shadow bit */
}

static inline const void *kasan_shadow_to_mem(const void *shadow_addr)
{
  unsigned long shadow_offset = (unsigned long)(shadow_addr - (void *)shadow);
  return (void *)((shadow_offset << McuASAN_SHADOW_SCALE_SHIFT) + McuASAN_CONFIG_APP_MEM_START);
}


static inline bool inside_mem_range(const void *address)
{
  return address >= (void *)__DATA_RAM && address < (void *)shadow;
}

// static inline bool inside_shadow_range(const void *address)
// {
//   return address >= (void *)shadow && address < (void *)__ShadowLimit;
// }


static inline void __asan_abort()
{
  __asm volatile("bkpt #0x47");
  while (1);
}

static void ReportError(unsigned long address, size_t kAccessSize, rw_mode_e mode) {
  // McuLog_fatal("ASAN ptr failure: addr 0x%x, %s, size: %d", address, mode==kIsRead?"read":"write", kAccessSize);
  __asan_abort();
}

/*
 * Poisons the shadow memory for 'size' bytes starting from 'addr'.
 * Memory addresses should be aligned to KASAN_SHADOW_SCALE_SIZE.
 */
static void kasan_poison_shadow(const void *address, size_t size, uint8_t value)
{
	void *shadow_start, *shadow_end;

	shadow_start = kasan_mem_to_shadow(address);
	shadow_end = kasan_mem_to_shadow(address + size);

	memset(shadow_start, value, shadow_end - shadow_start);
}

void kasan_unpoison_shadow(const void *address, size_t size)
{
	kasan_poison_shadow(address, size, 0);

	if (size & McuASAN_SHADOW_MASK) {
		uint8_t *shadow = (uint8_t *)kasan_mem_to_shadow(address + size);
		*shadow = size & McuASAN_SHADOW_MASK;
	}
}

static void register_global(struct kasan_global *global)
{
  if (likely(inside_mem_range(global->beg))) {
    size_t aligned_size = round_up(global->size, McuASAN_SHADOW_SCALE_SIZE);
    kasan_unpoison_shadow(global->beg, global->size);
    kasan_poison_shadow(global->beg + aligned_size, global->size_with_redzone - aligned_size, 0xf9);
  }
}

__attribute__((visibility("default")))
void __asan_register_globals(struct kasan_global *globals, size_t size) 
{ 
  int i;
  
  for (i = 0; i < size; i++) {
    register_global(&globals[i]);
  } 
}

__attribute__((visibility("default")))
void __asan_unregister_globals()
{
}

/*
 * All functions below always inlined so compiler could
 * perform better optimizations in each of __asan_loadX/__assn_storeX
 * depending on memory access size X.
 */
static inline bool memory_is_poisoned_1(unsigned long addr)
{
	int8_t shadow_value = *(int8_t *)kasan_mem_to_shadow((void *)addr);
	if (unlikely(shadow_value)) {
		int8_t last_accessible_byte = addr & McuASAN_SHADOW_MASK;
		return unlikely(last_accessible_byte >= shadow_value);
	}
	return false;
}

static inline bool memory_is_poisoned_2(unsigned long addr)
{
	uint16_t *shadow_addr = (uint16_t *)kasan_mem_to_shadow((void *)addr);
	
  if (unlikely(*shadow_addr)) {
		if (memory_is_poisoned_1(addr + 1))
			return true;
		
    /*
		 * If single shadow byte covers 2-byte access, we don't
		 * need to do anything more. Otherwise, test the first
		 * shadow byte.
		 */
		if (likely(((addr + 1) & McuASAN_SHADOW_MASK) != 0))
			return false;

		return unlikely(*(uint8_t *)shadow_addr);
	}

	return false;
}

static inline bool memory_is_poisoned_4(unsigned long addr)
{
	uint16_t *shadow_addr = (uint16_t *)kasan_mem_to_shadow((void *)addr);
	
  if (unlikely(*shadow_addr)) {
		if (memory_is_poisoned_1(addr + 3))
			return true;
	
  	/*
		 * If single shadow byte covers 4-byte access, we don't
		 * need to do anything more. Otherwise, test the first
		 * shadow byte.
		 */
		if (likely(((addr + 3) & McuASAN_SHADOW_MASK) >= 3))
			return false;
	
  	return unlikely(*(uint8_t *)shadow_addr);
	}
	
  return false;
}

static inline bool memory_is_poisoned_8(unsigned long addr)
{
	uint16_t *shadow_addr = (uint16_t *)kasan_mem_to_shadow((void *)addr);
	
  if (unlikely(*shadow_addr)) {
		if (memory_is_poisoned_1(addr + 7))
			return true;
	
  	/*
		 * If single shadow byte covers 8-byte access, we don't
		 * need to do anything more. Otherwise, test the first
		 * shadow byte.
		 */
		if (likely(IS_ALIGNED(addr, McuASAN_SHADOW_SCALE_SIZE)))
			return false;
	
  	return unlikely(*(uint8_t *)shadow_addr);
	}
	
  return false;
}

static inline unsigned long bytes_is_zero(const uint8_t *start, size_t size)
{
	while (size) {
		if (unlikely(*start))
			return (unsigned long)start;
		start++;
		size--;
	}

	return 0;
}

static inline unsigned long memory_is_zero(const void *start, const void *end)
{
	unsigned int words;
	unsigned long ret;
	unsigned int prefix = (unsigned long)start % 4;
	
  if (end - start <= 8)
		return bytes_is_zero(start, end - start);
	
  if (prefix) {
		prefix = 8 - prefix;
		ret = bytes_is_zero(start, prefix);
		if (unlikely(ret))
			return ret;
		start += prefix;
	}
	
  words = (end - start) / 4;
	
  while (words) {
		if (unlikely(*(uint32_t *)start))
			return bytes_is_zero(start, 4);
    start += 4;
		words--;
	}
	
  return bytes_is_zero(start, (end - start) % 4);
}

static __always_inline bool memory_is_poisoned_n(unsigned long addr, size_t size)
{
	unsigned long ret;

	ret = memory_is_zero(kasan_mem_to_shadow((void *)addr), 
    kasan_mem_to_shadow((void *)addr + size - 1) + 1);
	
  if (unlikely(ret)) {
		unsigned long last_byte = addr + size - 1;
		int8_t *last_shadow = (int8_t *)kasan_mem_to_shadow((void *)last_byte);
		
    if (unlikely(ret != (unsigned long)last_shadow ||
			((long)(last_byte & McuASAN_SHADOW_MASK) >= *last_shadow)))
			return true;
	}

	return false;
}

static inline bool memory_is_poisoned(unsigned long addr, size_t size)
{
  switch (size) {
  case 1:
    return memory_is_poisoned_1(addr);
  case 2:
    return memory_is_poisoned_2(addr);
  case 4:
    return memory_is_poisoned_4(addr);
  case 8:
    return memory_is_poisoned_8(addr);
  default:
    return memory_is_poisoned_n(addr, size);
  }
}

static inline void check_memory_region(unsigned long addr, size_t size, bool write)
{
  if (unlikely(!asan_enabled))
    return;

	if (unlikely(size == 0))
		return;

  // if (unlikely(inside_shadow_range((void *)addr)))
  //   ReportError(addr, size, write ? kIsWrite : kIsRead);

  if (unlikely(!inside_mem_range((void *)addr)))
    return;
    
  if (unlikely(!inside_mem_range((void *)(addr + size - 1))))
    ReportError(addr + size - 1, 1, write ? kIsWrite : kIsRead);

	if (unlikely((void *)addr < kasan_shadow_to_mem((void *)shadow))) {
    ReportError(addr, size, write ? kIsWrite : kIsRead);
		return;
	}

	if (likely(!memory_is_poisoned(addr, size)))
		return;

	ReportError(addr, size, write ? kIsWrite : kIsRead);
}


void __asan_handle_no_return(void) {}

#define DEFINE_ASAN_LOAD_STORE(size)                         \
  __attribute__((visibility("default")))                     \
  void __asan_load##size##_noabort(unsigned long addr) {     \
    check_memory_region(addr, size, false);                  \
  }                                                          \
                                                             \
  __attribute__((visibility("default")))                     \
  void __asan_store##size##_noabort(unsigned long addr) {    \
    check_memory_region(addr, size, true);                   \
  }


DEFINE_ASAN_LOAD_STORE(1)
DEFINE_ASAN_LOAD_STORE(2)
DEFINE_ASAN_LOAD_STORE(4)
DEFINE_ASAN_LOAD_STORE(8)

__attribute__((visibility("default")))
void __asan_loadN_noabort(unsigned long addr, size_t n) {
  check_memory_region(addr, n, false);
}

__attribute__((visibility("default")))
void __asan_storeN_noabort(unsigned long addr, size_t n) {
  check_memory_region(addr, n, true);
}

#if McuASAN_CONFIG_CHECK_MALLOC_FREE
/* undo possible defines for malloc and free */
#ifdef malloc
  #undef malloc
  void *malloc(size_t);
#endif
#ifdef free
  #undef free
  void free(void*);
#endif

size_t __asan_heap_usage = 0;
size_t __asan_max_heap_usage = 0;

/*
 * rrrrrrrr  red zone border (incl. size below)
 * size
 * memory returned
 * rrrrrrrr  red zone boarder
 */
__attribute__((visibility("default")))
void *__wrap_malloc(size_t size) {
  /* malloc allocates the requested amount of memory with redzones around it.
   * The shadow values corresponding to the redzones are poisoned and the shadow values
   * for the memory region are cleared.
   */
  size_t aligned_size = round_up(size, McuASAN_SHADOW_SCALE_SIZE);
  void *p = __real_malloc(aligned_size + McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER); /* add size_t for the size of the block */
  if (!p)
    return NULL;

  __asan_heap_usage += aligned_size + McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER;
  if (__asan_heap_usage > __asan_max_heap_usage) {
    __asan_max_heap_usage = __asan_heap_usage;
  }

  kasan_poison_shadow(p, McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER, 0xfa); /* poison heap left redzone */
  kasan_unpoison_shadow(p + McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER, size);

  *((size_t *)(p + McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER - sizeof(size_t))) = aligned_size;

  return p + McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER; /* return pointer to valid memory */
}
#endif

#if McuASAN_CONFIG_CHECK_MALLOC_FREE
static inline size_t heap_region_size(void *p)
{
  return *(size_t *)(p - sizeof(size_t));
}

__attribute__((visibility("default")))
void __wrap_free(void *p) 
{
  if (unlikely(!(p >= (void *)__HeapBase && p < (void *)__HeapLimit))) {
    /* not a heap memory */
    __asan_abort();
  }

  uint8_t *sa = kasan_mem_to_shadow(p);
  if (unlikely(*sa != 0)) {
    /* not a valid heap memory (e.g., freed or unallocated memory) */
    __asan_abort();
  }

  sa = kasan_mem_to_shadow(p - 1);
  if (unlikely(*sa != 0xfa)) {
    /* attempt to free a pointer not at the start of a heap block */
    __asan_abort();
  }

  /* Poisons shadow values for the entire region and put the chunk of memory into a quarantine queue
   * (such that this chunk will not be returned again by malloc during some period of time).
   */
  size_t size = heap_region_size(p); /* get size */
  void *q = p - McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER; /* calculate beginning of malloc()ed block */;

#if McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE > 0
  kasan_poison_shadow(p, size, 0xfd); /* poison freed heap region */

  /* put the memory block into quarantine */
  freeQuarantineList[freeQuarantineListIdx] = q;
  freeQuarantineListIdx++;
  
  if (freeQuarantineListIdx >= McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE) {
    freeQuarantineListIdx = 0;
  }
  
  if (freeQuarantineList[freeQuarantineListIdx] != NULL) {
    size = heap_region_size(freeQuarantineList[freeQuarantineListIdx] + McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER);
    kasan_poison_shadow(freeQuarantineList[freeQuarantineListIdx], McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER + size, 0xfa);
    __real_free(freeQuarantineList[freeQuarantineListIdx]);
    __asan_heap_usage -= McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER + size;
    freeQuarantineList[freeQuarantineListIdx] = NULL;
  }
#else
  kasan_poison_shadow(q, McuASAN_CONFIG_MALLOC_RED_ZONE_BORDER + size, 0xfa);
  free(q); /* free block */
#endif
}
#endif /* McuASAN_CONFIG_CHECK_MALLOC_FREE */


extern void *__real_memset(void *, int, size_t);
extern void *__real_memmove(void *, const void *, size_t);
extern void *__real_memcpy(void *, const void *, size_t);
extern char *__real_strcpy(char *, const char *);
extern char *__real_strncpy(char *, const char *, size_t);
extern char *__real_strcat(char *, const char *);
extern char *__real_strncat(char *, const char *, size_t);
extern int __real_sprintf(char *str, const char *format, ...);
extern int __real_snprintf(char *str, size_t size, const char *format, ...);

__attribute__((visibility("default")))
void *__wrap_memset(void *addr, int c, size_t len)
{
  __asan_storeN_noabort((unsigned long)addr, len);
  return __real_memset(addr, c, len);
}

__attribute__((visibility("default")))
void *__wrap_memmove(void *dest, const void *src, size_t len)
{
  __asan_loadN_noabort((unsigned long)src, len);
  __asan_storeN_noabort((unsigned long)dest, len);
  return __real_memmove(dest, src, len);
}

__attribute__((visibility("default")))
void *__wrap_memcpy(void *dest, const void *src, size_t len)
{
  __asan_loadN_noabort((unsigned long)src, len);
  __asan_storeN_noabort((unsigned long)dest, len);
  return __real_memcpy(dest, src, len);
}

__attribute__((visibility("default")))
char *__wrap_strcpy(char *dest, const char *src)
{
  size_t len = strlen(src) + 1;
  __asan_loadN_noabort((unsigned long)src, len);
  __asan_storeN_noabort((unsigned long)dest, len);
  return __real_strcpy(dest, src);
}

__attribute__((visibility("default")))
char *__wrap_strncpy(char *dest, const char *src, size_t n)
{
  size_t len = MIN(strlen(src) + 1, n);
  __asan_loadN_noabort((unsigned long)src, len);
  __asan_storeN_noabort((unsigned long)dest, len);
  return __real_strncpy(dest, src, n);
}

__attribute__((visibility("default")))
char *__wrap_strcat(char *dest, const char *src)
{
  size_t len = strlen(src) + 1;
  __asan_loadN_noabort((unsigned long)src, len);
  __asan_storeN_noabort((unsigned long)dest + strlen(dest), len);
  return __real_strcat(dest, src);
}

__attribute__((visibility("default")))
char *__wrap_strncat(char *dest, const char *src, size_t n)
{
  size_t len = MIN(strlen(src) + 1, n);
  __asan_loadN_noabort((unsigned long)src, len);
  __asan_storeN_noabort((unsigned long)dest + strlen(dest), len);
  return __real_strncat(dest, src, n);
}

__attribute__((visibility("default")))
int __wrap_sprintf(char *str, const char *format, ...)
{
  va_list pArgs;
  va_start(pArgs, format);
  
  int ret = vsprintf(str, format, pArgs);
  if (likely(ret > 0)) 
    __asan_storeN_noabort((unsigned long)str, ret + 1);

  return ret;
}

__attribute__((visibility("default")))
int __wrap_snprintf(char *str, size_t size, const char *format, ...)
{
  va_list pArgs;
  va_start(pArgs, format);

  int ret = vsnprintf(str, size, format, pArgs);
  if (likely(ret > 0))
    __asan_storeN_noabort((unsigned long)str, ret);

  return ret;
}


__attribute__((constructor(101)))
void McuASAN_Init(void) {
  memset(shadow, 0, __ShadowLimit - shadow);
  size_t heap_size = __HeapLimit - __HeapBase;
  size_t shadow_size = __ShadowLimit - shadow;

  kasan_poison_shadow(__HeapBase, heap_size, 0xfa);
  kasan_poison_shadow(shadow, shadow_size, -1);

#if McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE > 0
  for(int i = 0; i < McuASAN_CONFIG_FREE_QUARANTINE_LIST_SIZE; i++) {
    freeQuarantineList[i] = NULL;
  }
  freeQuarantineListIdx = 0;
#endif
  asan_enabled = true;
  __asm volatile("bkpt #0x99");
}

#endif /* McuASAN_CONFIG_IS_ENABLED */
