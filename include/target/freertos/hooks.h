/**
 * @file hooks.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Hooks of FreeRTOS
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __FREERTOS_HOOKS_H__
#define __FREERTOS_HOOKS_H__

#include <stdint.h>

extern void __usan_trace_new_thread_on_success(uint32_t thread_id);

extern void __usan_trace_new_thread_on_fail(uint32_t thread_id);

extern void __usan_trace_switch_context(uint32_t thread_id);

#define traceTASK_CREATE( xTask ) __usan_trace_new_thread_on_success((uint32_t)xTask)

#define traceTASK_CREATE_FAILED( pxNewTCB ) __usan_trace_new_thread_on_fail()

#define traceTASK_SWITCHED_IN() __usan_trace_switch_context((uint32_t)pxCurrentTCB)


#endif /* __FREERTOS_TRACER_H__ */