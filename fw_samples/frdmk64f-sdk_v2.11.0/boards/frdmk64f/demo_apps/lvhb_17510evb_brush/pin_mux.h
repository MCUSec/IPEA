/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*! @name PORTA2 (number 36), J1[12]/J9[6]/TRACE_SWO
  @{ */
#define BOARD_LVHB_IN1_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_LVHB_IN1_PIN 2U     /*!<@brief PORTA pin index: 2 */
                                  /* @} */

/*! @name PORTC2 (number 72), J1[14]
  @{ */
#define BOARD_LVHB_IN2_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_LVHB_IN2_PIN 2U     /*!<@brief PORTC pin index: 2 */
                                  /* @} */

/*! @name PORTC17 (number 91), J1[4]
  @{ */
#define BOARD_LVHB_EN_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_LVHB_EN_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_LVHB_EN_PIN 17U    /*!<@brief PORTC pin index: 17 */
                                 /* @} */

/*! @name PORTE25 (number 32), J2[18]/U8[6]/I2C0_SDA
  @{ */
#define BOARD_LVHB_GIN_GPIO GPIOE /*!<@brief GPIO device name: GPIOE */
#define BOARD_LVHB_GIN_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_LVHB_GIN_PIN 25U    /*!<@brief PORTE pin index: 25 */
                                  /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

#define SOPT5_UART0TXSRC_UART_TX 0x00u /*!<@brief UART 0 transmit data source select: UART0_TX pin */

/*! @name PORTB17 (number 63), U10[1]/UART0_TX
  @{ */
#define BOARD_DEBUG_UART_TX_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_DEBUG_UART_TX_PIN 17U    /*!<@brief PORTB pin index: 17 */
                                       /* @} */

/*! @name PORTB16 (number 62), U7[4]/UART0_RX
  @{ */
#define BOARD_DEBUG_UART_RX_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_DEBUG_UART_RX_PIN 16U    /*!<@brief PORTB pin index: 16 */
                                       /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitDEBUG_UARTPins(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
