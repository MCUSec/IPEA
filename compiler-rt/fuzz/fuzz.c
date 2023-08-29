/**
 * @file fuzz.c
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Implementation of fuzz triggers
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "fuzz.h"
#include "device_config.h"

// unsigned int TestCaseIdx = 0;
// unsigned int TestCaseLen __attribute__ ((section (".noinit")));
// unsigned char DeviceTestCaseBuffer[MAXDeviceTestCaseBufferSize + sizeof(unsigned int) * 2] __attribute__ ((section (".noinit")));

extern bool __ipeasan_bb_trace_enabled;

#define __BKPT(x)	__asm volatile("bkpt %0" : : "i"(x))

void FuzzStart(){
	__ipeasan_bb_trace_enabled = true;
	__BKPT(FUZZ_START_FLAG);
}

void FuzzFinish(){
	__ipeasan_bb_trace_enabled = false;
	__BKPT(FUZZ_STOP_FLAG);
}

void FuzzAbort(){
	__ipeasan_bb_trace_enabled = false;
	__BKPT(FUZZ_ABORT_FLAG);
}
