/*
 * Copyright 2016-2023 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    PinLock.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "mbedtls/sha256.h"
/* TODO: insert other include files here. */

#define BOARD_LED_GPIO     BOARD_LED_RED_GPIO
#define BOARD_LED_GPIO_PIN BOARD_LED_RED_PIN

/* Size of Transmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define PINRXBUFFSIZE                   5
#define LOCKRXBUFFSIZE			2

/* Size of Test */
#define PINBUFFSIZE			  (COUNTOF(pin) - 1)
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */
#define STRSIZE				   2048

uint8_t aTxBuffer[] = "Please enter your password:\r\n";

unsigned char PinRxBuffer[PINRXBUFFSIZE];
unsigned char LockRxBuffer[LOCKRXBUFFSIZE];
unsigned char key[32];
unsigned char key_in[32];
unsigned char pin[] = "1995";

volatile uint32_t g_systickCounter;

#ifdef ENABLE_PROFILE

size_t stack_base, stack_top, stack_usage;

__attribute__((no_instrument_function, no_sanitize_address, annotate("no_instrument")))
void __cyg_profile_func_enter(void *this_fn, void *callsite)
{
    uint32_t sp;
    __asm volatile("mov %0, sp" : "=r"(sp) : : "memory");
    if (sp < stack_top)
        stack_top = sp;
}

__attribute__((no_instrument_function, no_sanitize_address, annotate("no_instrument")))
void __cyg_profile_func_exit(void *this_fn, void *callsite)
{
}

__attribute__((no_instrument_function, no_sanitize_address, annotate("no_instrument"), constructor))
void __profile_init(void)
{
    const uint32_t *vector = (const uint32_t *)*(volatile uint32_t *)0xe000ed08;
    stack_base = vector[0];
    stack_top = stack_base;
	stack_usage = stack_base - stack_top;
}

#endif


void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    }
}

void lock() {
    GPIO_PortSet(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
}

void unlock() {
    GPIO_PortClear(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
}

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    int len;
	int unlock_count = 0;

	unsigned int one = 1;
	unsigned int exp;
	unsigned int ms;
	static char locked[]="System Locked\r\n";
	static char enter[] = "Enter Pin:\r\n";
	static char unlocked[] = "Unlocked\r\n";
	static char incorrect[] = "Incorrect Pin\r\n";
	static char waiting[] = "waiting...\r\n";
	static char lockout[] = "System Lockout\r\n";

	/* Board pin init */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();

	/* Set systick reload value to generate 1ms interrupt */
	if (SysTick_Config(SystemCoreClock / 1000U))
	{
		while (1)
		{
		}
	}

	mbedtls_sha256((unsigned char*)pin,PINBUFFSIZE,key,0);
	__BKPT(0x99);
	while (1) {
		lock();
		PRINTF("%s\n", locked);

		unsigned int failures = 0;
	// In Locked State
		while(1) {
			// print(enter,sizeof(enter));
			PRINTF("%s\r\n", enter);
//			SCANF("%s", PinRxBuffer);
			snprintf(PinRxBuffer, sizeof(PinRxBuffer), "1995");
			//hash password received from uart
			mbedtls_sha256((unsigned char*)PinRxBuffer,PINRXBUFFSIZE,key_in,0);
			int i;
			for(i = 0; i < 32; i++) {
				if(key[i]!=key_in[i]) break;
			}
			if (i == 32) {
				PRINTF("%s\n", unlocked);
				unlock_count++;
				// if (unlock_count >= 100){
				//     STOP_TIMING;
				// }
				break;
			}

			failures++; //increment number of failures
			PRINTF("%s\n", incorrect);
//			#ifdef ENABLE_PROFILE
//			stack_usage = stack_base - stack_top;
//			#else
//			FuzzFinish();
//			#endif
			__BKPT(0x99);

			if (failures > 5 && failures <= 10) {

				exp = one << failures;  // essentially 2^failures
				ms = 78*exp;   // after 5 tries, start waiting around 5 secs and then doubles
				PRINTF("%s\n", waiting);
				SysTick_DelayTicks(ms);

			}
			else if(failures > 10) {
				PRINTF("%s\n", lockout);
				while(1){}
			}

		}
		 unlock();
		// wait for lock command
		while (1) {
			SCANF("%s", PinRxBuffer);
			if (PinRxBuffer[0] == '0'){
				break;
			}
		}
		lock();
	}

    return 0 ;
}
