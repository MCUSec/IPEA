/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fuzz.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_SIZE 16

char of_global[BUFFER_SIZE];

struct object {
    int a;
    char buf[10];
} __attribute__((packed));

unsigned char DeviceTestCaseBuffer[512] __attribute__((section(".noinit")));
unsigned int TestCaseLen __attribute__((section(".noinit")));


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef ASAN
extern void __usan_trace_basicblock(uint16_t);
extern void __usan_trace_func_entry_stack(uint32_t);
int main(void);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

__attribute__((annotate("interruptHandler")))
void HardFault_Handler(void)
{
#ifdef ASAN
    __usan_trace_func_entry_stack((uint32_t)main);
    __usan_trace_basicblock(65335);
#endif
	FuzzAbort();
	while (1);
}

/*!
 * @brief Main function
 */
int main(void)
{
    struct object o;
    char of_stack[BUFFER_SIZE];
    char *of_heap = NULL;
    char flag = '\0';

#ifdef ASAN
    __usan_trace_basicblock(65534);
    __usan_trace_func_entry((uint32_t)main);
#endif

    while (1) {
#ifdef ASAN
        __usan_trace_basicblock(65533);
#endif
        FuzzStart();

        if (!of_heap) {
#ifdef ASAN
            __usan_trace_basicblock(65532);
#endif
            of_heap = malloc(BUFFER_SIZE);
        }

        flag = DeviceTestCaseBuffer[0];

        switch (flag) {
        case 'a':
#ifdef ASAN
            __usan_trace_basicblock(65531);
#endif
            // stack buffer overflow
            memset(of_stack, 0, BUFFER_SIZE + 1);
            break;
        case 'e':
#ifdef ASAN
            __usan_trace_basicblock(65530);
#endif
            // heap buffer overflow
            memset(of_heap, 0, BUFFER_SIZE + 1);
            break;
        case 'i':
#ifdef ASAN
            __usan_trace_basicblock(65529);
#endif
            // global buffer overflow
            memset(of_global, 0, BUFFER_SIZE + 1);
            break;
        case 'o':
#ifdef ASAN
            __usan_trace_basicblock(65528);
#endif
            // double free
            free(of_heap);
            free(of_heap);
            break;
        case 'u':
#ifdef ASAN
            __usan_trace_basicblock(65527);
#endif
            // use after free
            free(of_heap);
            memset(of_heap, 0, BUFFER_SIZE);
            break;
        case 'x':
#ifdef ASAN
            __usan_trace_basicblock(65526);
#endif
            // null pointer dereference
            of_heap = NULL;
            memset(of_heap, 0, BUFFER_SIZE);
            break;
        case 'y':
#ifdef ASAN
            __usan_trace_basicblock(65525);
#endif
            // free non-heap object
            free(of_stack);
            break;
        case 'z':
#ifdef ASAN
            __usan_trace_basicblock(65524);
#endif
            // sub-object overflow
            o.buf[10] = 100;
            break;
        default:
#ifdef ASAN
            __usan_trace_basicblock(65523);
#endif
            // normal case
            break;
        }
        
        FuzzFinish();
    }

    while (1);
}
