#include "fuzz.h"
#include "device_config.h"

// unsigned int TestCaseIdx = 0;
// unsigned int TestCaseLen __attribute__ ((section (".noinit")));
// unsigned char DeviceTestCaseBuffer[MAXDeviceTestCaseBufferSize + sizeof(unsigned int) * 2] __attribute__ ((section (".noinit")));

#define __BKPT(x)	__asm volatile("bkpt %0" : : "i"(x))

void FuzzStart(){
	__BKPT(FUZZ_START_FLAG);
}

void FuzzFinish(){
	__BKPT(FUZZ_STOP_FLAG);
}

void FuzzAbort(){
	__BKPT(FUZZ_ABORT_FLAG);
}
