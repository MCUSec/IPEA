/**
 * @file device_config.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Definitions for target device
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

// J-Trace
#define JLINK_SPEED                 (4 * 1024)
#define JLINKARM_HOSTIF             JLINKARM_HOSTIF_USB // or JLINKARM_HOSTIF_IP
// Fuzz
// #define NON_PERSISTENT_MODE         0
// #define NORMAL			            0
// #define TIMEOUT			            1
// #define CRASH			            2
#define RTT_INIT_FLAG               0x99
#define FUZZ_START_FLAG             0x10
#define FUZZ_STOP_FLAG              0x20
#define FUZZ_ABORT_FLAG             0x47
#define FUZZ_FAULT_FLAG             0x50
// RTT
#define RTT_BUF_SIZE                (1000 * 1024 * 1024)
#define RTT_SLOT_SIZE	            (1024 * 1024)
#define RTT_MAX_RETRY_COUNT         (10)

#endif // __DEVICE_CONFIG_H__
