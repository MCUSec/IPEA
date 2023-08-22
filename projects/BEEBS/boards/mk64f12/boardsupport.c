/* Copyright (C) 2014 Embecosm Limited and University of Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>

   This file is part of the Bristol/Embecosm Embedded Benchmark Suite.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

#include <support.h>
#include <stdint.h>
#include <stddef.h>

#if defined(ENABLE_PROFILE) && ENABLE_PROFILE && !defined(IPEA)

static uint32_t stack_base;
uint32_t __max_stack_usage;
uint32_t __max_heap_usage;
uint32_t __heap_usage;


__attribute__((no_instrument_function))
void __profile_stack(uint32_t sp)
{
    const uint32_t used = stack_base - sp;
    if (used > __max_stack_usage)
        __max_stack_usage = used;
}

__attribute__((naked, no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    __asm volatile(
        ".extern __profile_stack\n"
        "mov r0, sp\n"
        "b __profile_stack\n"
    );
}

__attribute__((naked, no_instrument_function))
void __cyg_profile_func_exit(void *this_func, void *call_site)  
{
    __asm volatile("bx lr");
}

__attribute__((no_instrument_function))
void *__wrap_malloc_beebs(size_t size)
{
#if !defined(ENABLE_ASAN) || ENABLE_ASAN == 0
    // extern void *__real_malloc_beebs(size_t);
    void *p = malloc(size + sizeof(size_t));
    if (p) {
        *((size_t *)p) = size;
        __heap_usage += size;
        if (__heap_usage > __max_heap_usage) {
            __max_heap_usage = __heap_usage;
        }
        return (void *)(p + sizeof(size_t));
    }
    return NULL;
#else
    extern size_t __asan_heap_usage, __asan_max_heap_usage;
    void *p = malloc(size);
    __heap_usage = __asan_heap_usage;
    __max_heap_usage = __asan_max_heap_usage;
    return p;    
#endif
}

__attribute__((no_instrument_function))
void __wrap_free_beebs(void *p)
{
#if !defined(ENABLE_ASAN) || ENABLE_ASAN == 0
    // extern void __real_free_beebs(void *);
    void *p_base = p - sizeof(size_t);
    __heap_usage -= *(size_t *)p_base;
    free(p_base);
#else
    extern size_t __asan_heap_usage, __asan_max_heap_usage;
    free(p);
    __heap_usage = __asan_heap_usage;
    __max_heap_usage = __asan_max_heap_usage;
#endif
}

__attribute__((naked, constructor, no_instrument_function))
void __init()
{
    __asm volatile(
        "mov %0, sp\n"
        "bkpt #0x99\n" 
        : "=r"(stack_base) : : "memory"
    );
}

#endif

#ifdef BASELINE
#include <stdlib.h>

uint32_t __baseline_heap_usage;
uint32_t __baseline_max_heap_usage;

__attribute__((no_instrument_function))
void *__baseline_malloc(size_t size)
{
    void *p = malloc(size + sizeof(size_t));
    if (p) {
        *((size_t *)p) = size;
        __baseline_heap_usage += size;
        if (__baseline_heap_usage > __baseline_max_heap_usage) {
            __baseline_max_heap_usage = __baseline_heap_usage;
        }
        return (void *)(p + sizeof(size_t));
    }
    return NULL;
}

__attribute__((no_instrument_function))
void __baseline_free(void *p)
{
    size_t size = *(size_t *)(p - sizeof(size_t));
    free(p - sizeof(size_t));
    __baseline_heap_usage -= size;
}
#endif

__attribute__((no_instrument_function))
void initialise_board()
{
}

__attribute__((no_instrument_function))
void start_trigger()
{
}

__attribute__((no_instrument_function))
void stop_trigger()
{
    __asm volatile("bkpt #0x20");
}
