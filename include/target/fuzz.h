/**
 * @file fuzz.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Interfaces of fuzz triggers
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __FUZZ_H__
#define __FUZZ_H__

#include <stdbool.h>
#include "trace.h"

#ifdef __GNUC__
#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif
#else
#ifndef likely
#define likely(x)	(x)
#endif
#ifndef unlikely
#define unlikely(x)	(x)
#endif
#endif

// extern volatile unsigned int TestCaseIdx;
// extern volatile unsigned int TestCaseLen __attribute__ ((section (".noinit")));
// extern volatile unsigned char DeviceTestCaseBuffer[MAXDeviceTestCaseBufferSize + sizeof(unsigned int) * 2] __attribute__ ((section (".noinit")));
// extern volatile unsigned int rtt_total_bytes __attribute__ ((section (".data.rtt")));
extern unsigned int TestCaseIdx;
extern unsigned int TestCaseLen;
extern unsigned char DeviceTestCaseBuffer[];

extern void FuzzFinish();
extern void FuzzStart();
extern void FuzzAbort();

#endif
