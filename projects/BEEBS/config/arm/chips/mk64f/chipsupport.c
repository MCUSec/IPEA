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

#include <string.h>

// This file is needed to copy the initialised data from flash to RAM

extern void __StackTop(void);

extern unsigned char __data_start__;
extern unsigned char __data_end__;
extern unsigned char __etext;

extern unsigned char __bss_start__;
extern unsigned char __bss_end__;

extern void __libc_init_array();
extern int main();

void software_init_hook();
static void nmi_handler(void);
static void hard_fault_handler(void);
static void default_int_handler(void);


__attribute__ ((section(".isr_vector")))
void (* const __isr_vector[])(void) = {
    __StackTop,
    software_init_hook,                     // The reset handler
    nmi_handler,                            // The NMI handler
    hard_fault_handler,                     // The hard fault handler
    0,                                      //Reserved
    0,                                      //Reserved
    0,                                      //Reserved
    0,                                      //Reserved
    0,                                      //Reserved
    0,                                      //Reserved
    0,                                      //Reserved
    default_int_handler,                    //SVC_Handler
    0,                                      //Reserved
    0,                                      //Reserved
    default_int_handler,                    //PendSV_Handler
    default_int_handler,                    //SysTick_Handler
    default_int_handler,   // DMA0_IRQHandler                                 /* DMA Channel 0 Transfer Complete*/
    default_int_handler,   // DMA1_IRQHandler                                 /* DMA Channel 1 Transfer Complete*/
    default_int_handler,   // DMA2_IRQHandler                                 /* DMA Channel 2 Transfer Complete*/
    default_int_handler,   // DMA3_IRQHandler                                 /* DMA Channel 3 Transfer Complete*/
    default_int_handler,   // DMA4_IRQHandler                                 /* DMA Channel 4 Transfer Complete*/
    default_int_handler,   // DMA5_IRQHandler                                 /* DMA Channel 5 Transfer Complete*/
    default_int_handler,   // DMA6_IRQHandler                                 /* DMA Channel 6 Transfer Complete*/
    default_int_handler,   // DMA7_IRQHandler                                 /* DMA Channel 7 Transfer Complete*/
    default_int_handler,   // DMA8_IRQHandler                                 /* DMA Channel 8 Transfer Complete*/
    default_int_handler,   // DMA9_IRQHandler                                 /* DMA Channel 9 Transfer Complete*/
    default_int_handler,   // DMA10_IRQHandler                                /* DMA Channel 10 Transfer Complete*/
    default_int_handler,   // DMA11_IRQHandler                                /* DMA Channel 11 Transfer Complete*/
    default_int_handler,   // DMA12_IRQHandler                                /* DMA Channel 12 Transfer Complete*/
    default_int_handler,   // DMA13_IRQHandler                                /* DMA Channel 13 Transfer Complete*/
    default_int_handler,   // DMA14_IRQHandler                                /* DMA Channel 14 Transfer Complete*/
    default_int_handler,   // DMA15_IRQHandler                                /* DMA Channel 15 Transfer Complete*/
    default_int_handler,   // DMA_Error_IRQHandler                            /* DMA Error Interrupt*/
    default_int_handler,   // MCM_IRQHandler                                  /* Normal Interrupt*/
    default_int_handler,   // FTFE_IRQHandler                                 /* FTFE Command complete interrupt*/
    default_int_handler,   // Read_Collision_IRQHandler                       /* Read Collision Interrupt*/
    default_int_handler,   // LVD_LVW_IRQHandler                              /* Low Voltage Detect, Low Voltage Warning*/
    default_int_handler,   // LLWU_IRQHandler                                 /* Low Leakage Wakeup Unit*/
    default_int_handler,   // WDOG_EWM_IRQHandler                             /* WDOG Interrupt*/
    default_int_handler,   // RNG_IRQHandler                                  /* RNG Interrupt*/
    default_int_handler,   // I2C0_IRQHandler                                 /* I2C0 interrupt*/
    default_int_handler,   // I2C1_IRQHandler                                 /* I2C1 interrupt*/
    default_int_handler,   // SPI0_IRQHandler                                 /* SPI0 Interrupt*/
    default_int_handler,   // SPI1_IRQHandler                                 /* SPI1 Interrupt*/
    default_int_handler,   // I2S0_Tx_IRQHandler                              /* I2S0 transmit interrupt*/
    default_int_handler,   // I2S0_Rx_IRQHandler                              /* I2S0 receive interrupt*/
    default_int_handler,   // UART0_LON_IRQHandler                            /* UART0 LON interrupt*/
    default_int_handler,   // UART0_RX_TX_IRQHandler                          /* UART0 Receive/Transmit interrupt*/
    default_int_handler,   // UART0_ERR_IRQHandler                            /* UART0 Error interrupt*/
    default_int_handler,   // UART1_RX_TX_IRQHandler                          /* UART1 Receive/Transmit interrupt*/
    default_int_handler,   // UART1_ERR_IRQHandler                            /* UART1 Error interrupt*/
    default_int_handler,   // UART2_RX_TX_IRQHandler                          /* UART2 Receive/Transmit interrupt*/
    default_int_handler,   // UART2_ERR_IRQHandler                            /* UART2 Error interrupt*/
    default_int_handler,   // UART3_RX_TX_IRQHandler                          /* UART3 Receive/Transmit interrupt*/
    default_int_handler,   // UART3_ERR_IRQHandler                            /* UART3 Error interrupt*/
    default_int_handler,   // ADC0_IRQHandler                                 /* ADC0 interrupt*/
    default_int_handler,   // CMP0_IRQHandler                                 /* CMP0 interrupt*/
    default_int_handler,   // CMP1_IRQHandler                                 /* CMP1 interrupt*/
    default_int_handler,   // FTM0_IRQHandler                                 /* FTM0 fault, overflow and channels interrupt*/
    default_int_handler,   // FTM1_IRQHandler                                 /* FTM1 fault, overflow and channels interrupt*/
    default_int_handler,   // FTM2_IRQHandler                                 /* FTM2 fault, overflow and channels interrupt*/
    default_int_handler,   // CMT_IRQHandler                                  /* CMT interrupt*/
    default_int_handler,   // RTC_IRQHandler                                  /* RTC interrupt*/
    default_int_handler,   // RTC_Seconds_IRQHandler                          /* RTC seconds interrupt*/
    default_int_handler,   // PIT0_IRQHandler                                 /* PIT timer channel 0 interrupt*/
    default_int_handler,   // PIT1_IRQHandler                                 /* PIT timer channel 1 interrupt*/
    default_int_handler,   // PIT2_IRQHandler                                 /* PIT timer channel 2 interrupt*/
    default_int_handler,   // PIT3_IRQHandler                                 /* PIT timer channel 3 interrupt*/
    default_int_handler,   // PDB0_IRQHandler                                 /* PDB0 Interrupt*/
    default_int_handler,   // USB0_IRQHandler                                 /* USB0 interrupt*/
    default_int_handler,   // USBDCD_IRQHandler                               /* USBDCD Interrupt*/
    default_int_handler,   // Reserved71_IRQHandler                           /* Reserved interrupt 71*/
    default_int_handler,   // DAC0_IRQHandler                                 /* DAC0 interrupt*/
    default_int_handler,   // MCG_IRQHandler                                  /* MCG Interrupt*/
    default_int_handler,   // LPTMR0_IRQHandler                               /* LPTimer interrupt*/
    default_int_handler,   // PORTA_IRQHandler                                /* Port A interrupt*/
    default_int_handler,   // PORTB_IRQHandler                                /* Port B interrupt*/
    default_int_handler,   // PORTC_IRQHandler                                /* Port C interrupt*/
    default_int_handler,   // PORTD_IRQHandler                                /* Port D interrupt*/
    default_int_handler,   // PORTE_IRQHandler                                /* Port E interrupt*/
    default_int_handler,   // SWI_IRQHandler                                  /* Software interrupt*/
    default_int_handler,   // SPI2_IRQHandler                                 /* SPI2 Interrupt*/
    default_int_handler,   // UART4_RX_TX_IRQHandler                          /* UART4 Receive/Transmit interrupt*/
    default_int_handler,   // UART4_ERR_IRQHandler                            /* UART4 Error interrupt*/
    default_int_handler,   // UART5_RX_TX_IRQHandler                          /* UART5 Receive/Transmit interrupt*/
    default_int_handler,   // UART5_ERR_IRQHandler                            /* UART5 Error interrupt*/
    default_int_handler,   // CMP2_IRQHandler                                 /* CMP2 interrupt*/
    default_int_handler,   // FTM3_IRQHandler                                 /* FTM3 fault, overflow and channels interrupt*/
    default_int_handler,   // DAC1_IRQHandler                                 /* DAC1 interrupt*/
    default_int_handler,   // ADC1_IRQHandler                                 /* ADC1 interrupt*/
    default_int_handler,   // I2C2_IRQHandler                                 /* I2C2 interrupt*/
    default_int_handler,   // CAN0_ORed_Message_buffer_IRQHandler             /* CAN0 OR'd message buffers interrupt*/
    default_int_handler,   // CAN0_Bus_Off_IRQHandler                         /* CAN0 bus off interrupt*/
    default_int_handler,   // CAN0_Error_IRQHandler                           /* CAN0 error interrupt*/
    default_int_handler,   // CAN0_Tx_Warning_IRQHandler                      /* CAN0 Tx warning interrupt*/
    default_int_handler,   // CAN0_Rx_Warning_IRQHandler                      /* CAN0 Rx warning interrupt*/
    default_int_handler,   // CAN0_Wake_Up_IRQHandler                         /* CAN0 wake up interrupt*/
    default_int_handler,   // SDHC_IRQHandler                                 /* SDHC interrupt*/
    default_int_handler,   // ENET_1588_Timer_IRQHandler                      /* Ethernet MAC IEEE 1588 Timer Interrupt*/
    default_int_handler,   // ENET_Transmit_IRQHandler                        /* Ethernet MAC Transmit Interrupt*/
    default_int_handler,   // ENET_Receive_IRQHandler                         /* Ethernet MAC Receive Interrupt*/
    default_int_handler,   // ENET_Error_IRQHandler                           /* Ethernet MAC Error and miscelaneous Interrupt*/
    default_int_handler,                                      /* 102*/
    default_int_handler,                                      /* 103*/
    default_int_handler,                                      /* 104*/
    default_int_handler,                                      /* 105*/
    default_int_handler,                                      /* 106*/
    default_int_handler,                                      /* 107*/
    default_int_handler,                                      /* 108*/
    default_int_handler,                                      /* 109*/
    default_int_handler,                                      /* 110*/
    default_int_handler,                                      /* 111*/
    default_int_handler,                                      /* 112*/
    default_int_handler,                                      /* 113*/
    default_int_handler,                                      /* 114*/
    default_int_handler,                                      /* 115*/
    default_int_handler,                                      /* 116*/
    default_int_handler,                                      /* 117*/
    default_int_handler,                                      /* 118*/
    default_int_handler,                                      /* 119*/
    default_int_handler,                                      /* 120*/
    default_int_handler,                                      /* 121*/
    default_int_handler,                                      /* 122*/
    default_int_handler,                                      /* 123*/
    default_int_handler,                                      /* 124*/
    default_int_handler,                                      /* 125*/
    default_int_handler,                                      /* 126*/
    default_int_handler,                                      /* 127*/
    default_int_handler,                                      /* 128*/
    default_int_handler,                                      /* 129*/
    default_int_handler,                                      /* 130*/
    default_int_handler,                                      /* 131*/
    default_int_handler,                                      /* 132*/
    default_int_handler,                                      /* 133*/
    default_int_handler,                                      /* 134*/
    default_int_handler,                                      /* 135*/
    default_int_handler,                                      /* 136*/
    default_int_handler,                                      /* 137*/
    default_int_handler,                                      /* 138*/
    default_int_handler,                                      /* 139*/
    default_int_handler,                                      /* 140*/
    default_int_handler,                                      /* 141*/
    default_int_handler,                                      /* 142*/
    default_int_handler,                                      /* 143*/
    default_int_handler,                                      /* 144*/
    default_int_handler,                                      /* 145*/
    default_int_handler,                                      /* 146*/
    default_int_handler,                                      /* 147*/
    default_int_handler,                                      /* 148*/
    default_int_handler,                                      /* 149*/
    default_int_handler,                                      /* 150*/
    default_int_handler,                                      /* 151*/
    default_int_handler,                                      /* 152*/
    default_int_handler,                                      /* 153*/
    default_int_handler,                                      /* 154*/
    default_int_handler,                                      /* 155*/
    default_int_handler,                                      /* 156*/
    default_int_handler,                                      /* 157*/
    default_int_handler,                                      /* 158*/
    default_int_handler,                                      /* 159*/
    default_int_handler,                                      /* 160*/
    default_int_handler,                                      /* 161*/
    default_int_handler,                                      /* 162*/
    default_int_handler,                                      /* 163*/
    default_int_handler,                                      /* 164*/
    default_int_handler,                                      /* 165*/
    default_int_handler,                                      /* 166*/
    default_int_handler,                                      /* 167*/
    default_int_handler,                                      /* 168*/
    default_int_handler,                                      /* 169*/
    default_int_handler,                                      /* 170*/
    default_int_handler,                                      /* 171*/
    default_int_handler,                                      /* 172*/
    default_int_handler,                                      /* 173*/
    default_int_handler,                                      /* 174*/
    default_int_handler,                                      /* 175*/
    default_int_handler,                                      /* 176*/
    default_int_handler,                                      /* 177*/
    default_int_handler,                                      /* 178*/
    default_int_handler,                                      /* 179*/
    default_int_handler,                                      /* 180*/
    default_int_handler,                                      /* 181*/
    default_int_handler,                                      /* 182*/
    default_int_handler,                                      /* 183*/
    default_int_handler,                                      /* 184*/
    default_int_handler,                                      /* 185*/
    default_int_handler,                                      /* 186*/
    default_int_handler,                                      /* 187*/
    default_int_handler,                                      /* 188*/
    default_int_handler,                                      /* 189*/
    default_int_handler,                                      /* 190*/
    default_int_handler,                                      /* 191*/
    default_int_handler,                                      /* 192*/
    default_int_handler,                                      /* 193*/
    default_int_handler,                                      /* 194*/
    default_int_handler,                                      /* 195*/
    default_int_handler,                                      /* 196*/
    default_int_handler,                                      /* 197*/
    default_int_handler,                                      /* 198*/
    default_int_handler,                                      /* 199*/
    default_int_handler,                                      /* 200*/
    default_int_handler,                                      /* 201*/
    default_int_handler,                                      /* 202*/
    default_int_handler,                                      /* 203*/
    default_int_handler,                                      /* 204*/
    default_int_handler,                                      /* 205*/
    default_int_handler,                                      /* 206*/
    default_int_handler,                                      /* 207*/
    default_int_handler,                                      /* 208*/
    default_int_handler,                                      /* 209*/
    default_int_handler,                                      /* 210*/
    default_int_handler,                                      /* 211*/
    default_int_handler,                                      /* 212*/
    default_int_handler,                                      /* 213*/
    default_int_handler,                                      /* 214*/
    default_int_handler,                                      /* 215*/
    default_int_handler,                                      /* 216*/
    default_int_handler,                                      /* 217*/
    default_int_handler,                                      /* 218*/
    default_int_handler,                                      /* 219*/
    default_int_handler,                                      /* 220*/
    default_int_handler,                                      /* 221*/
    default_int_handler,                                      /* 222*/
    default_int_handler,                                      /* 223*/
    default_int_handler,                                      /* 224*/
    default_int_handler,                                      /* 225*/
    default_int_handler,                                      /* 226*/
    default_int_handler,                                      /* 227*/
    default_int_handler,                                      /* 228*/
    default_int_handler,                                      /* 229*/
    default_int_handler,                                      /* 230*/
    default_int_handler,                                      /* 231*/
    default_int_handler,                                      /* 232*/
    default_int_handler,                                      /* 233*/
    default_int_handler,                                      /* 234*/
    default_int_handler,                                      /* 235*/
    default_int_handler,                                      /* 236*/
    default_int_handler,                                      /* 237*/
    default_int_handler,                                      /* 238*/
    default_int_handler,                                      /* 239*/
    default_int_handler,                                      /* 240*/
    default_int_handler,                                      /* 241*/
    default_int_handler,                                      /* 242*/
    default_int_handler,                                      /* 243*/
    default_int_handler,                                      /* 244*/
    default_int_handler,                                      /* 245*/
    default_int_handler,                                      /* 246*/
    default_int_handler,                                      /* 247*/
    default_int_handler,                                      /* 248*/
    default_int_handler,                                      /* 249*/
    default_int_handler,                                      /* 250*/
    default_int_handler,                                      /* 251*/
    default_int_handler,                                      /* 252*/
    default_int_handler,                                      /* 253*/
    default_int_handler,                                      /* 254*/
    (void (*)(void))0xFFFFFFFF                                /*  Reserved for user TRIM value*/
};


__attribute__((used, section(".FlashConfig")))
unsigned long flash_config[] = {
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFEUL,
};


__attribute__((annotate("no_sanitize")))
void software_init_hook()
{
#if 0
    __asm volatile(
        "    .extern __isr_vector      \n"
        "    .extern __etext           \n"
        "    .extern __data_start__    \n"
        "    .extern __data_end__      \n"
        "    .extern __bss_start__     \n"
        "    .extern __bss_end__       \n"
        "    .extern _start            \n"
        "    cpsid i                   \n"
        "    ldr r0, =0xe000ed08       \n"
        "    ldr r1, =__isr_vector     \n"
        "    str r1, [r0]              \n"
        "    ldr r2, [r1]              \n"
        "    msr msp, r2               \n"
        "    ldr r1, =__etext          \n"
        "    ldr r2, =__data_start__   \n"
        "    ldr r3, =__data_end__     \n"
        ".LC0:                         \n"
        "    cmp r2, r3                \n"
        "    ittt lt                   \n"
        "    ldrlt r0, [r1], #4        \n"
        "    strlt r0, [r2], #4        \n"
        "    blt .LC0                  \n"
        "    ldr r1, =__bss_start__    \n"
        "    ldr r2, =__bss_end__      \n"
        "    movs r0, #0               \n"
        ".LC1:                         \n"
        "    cmp r1, r2                \n"
        "    itt lt                    \n"
        "    strlt r0, [r1], #4        \n"
        "    blt .LC1                  \n"
        "    cpsie i                   \n"
        "    ldr r0, =_start           \n"
        "    blx r0                    \n"
        "    b .                       \n"
        "    .align 2                  \n"
        "    .pool                     \n"
    );
#endif
    __asm volatile("cpsid i");

    *((volatile unsigned long *)0xE000ED08) = (unsigned long)__isr_vector;

    __asm volatile("msr msp, %0" : : "r"(__isr_vector[0]));
    
    memcpy(&__data_start__, &__etext, (unsigned)&__data_end__ - (unsigned)&__data_start__);
    memset(&__bss_start__, 0, (unsigned)&__bss_end__ - (unsigned)&__bss_start__);
    
    __asm volatile("cpsie i");

    __libc_init_array();
    main();

    while (1) {}
}

__attribute__((annotate("no_sanitize")))
static void nmi_handler(void){
    while(1){ }
}

__attribute__((annotate("no_sanitize")))
static void hard_fault_handler(void){
    __asm volatile("bkpt #0x50");
    while(1){ }
}

__attribute__((annotate("no_sanitize")))
static void default_int_handler(void){
    while(1){ }
}
