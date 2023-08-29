/**
 * @file init.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Interfaces and definitions for J-Link module
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __INIT_H__
#define __INIT_H__

#include "JLinkARMDLL.h"
#include <stdio.h>
#include <stdbool.h>
#include "target_info.h"

#define MAX_LOGPATH_LENGTH  128

typedef struct {
    U32 HostIF;                 // Host interface used to connect to J-Link. 1 = USB, 2 = IP
    U32 TargetIF;               // See JLINKARM_Const.h "Interfaces" for valid values
    U32 SerialNo;               // Serial number of J-Link we want to connect to via USB
    U32 Speed;                  // Target interface speed in kHz
    char sHost[64];          // Points to the IPAddr / nickname of the J-Link we want to connect to.
    char sSettingsFile[64];  // Points to J-Link settings file to store connection settings
    char sDevice[64];        // Target device J-Link is connected to
    U32 FlashBase;         // Download address
    U32 FlashSize;             // Flash size
    U32 SRAMBase;               // SRAM base
    U32 SRAMSize;               // SRAM size
	// const char *sBinary;
    // const char *sLogPath;
    // const char *sSanitizer;
    // bool EnableTrace;
    // bool PersistMode;
    // bool FuzzMode;
    // bool Verbose;
    // U32 RunTimeout;
    // U32 FlashAddressess;
    // U32 RTTCBAddress;
    // U32 EntryPoint;
    // U32 TraceCRCAddress;
	// U32 testcase_buf_addr;
	// U32 testcase_size_addr;
	// // U32 rtt_control_block_addr;
    // U32 rtt_status_addr;
    // U32 rtt_lock_addr;
	// U32 rtt_total_size_addr;
    // U32 rtt_ena_flag_addr;
    // U32 rnd_ena_flag_addr;
} INIT_PARAS;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the J-Link probe
 * 
 * @param config_file Path of the configuration file
 */
void InitJLink(INIT_PARAS *, target_info_t *, const char *config_file);

/**
 * @brief Initialize a debug session
 * 
 */
void InitDebugSession(INIT_PARAS *);

/**
 * @brief Close a debug session
 * 
 * @param target_info Target information
 */
void CloseDebugSession(const target_info_t *target_info);

/**
 * @brief Reset a debug information
 * 
 */
void ResetDebugSession(INIT_PARAS *);

/**
 * @brief Initialize the firmware
 * 
 * @param target_info Target information
 * @param skip_download Indicates whether skip firmware downloading
 */
void InitFirmware(const target_info_t *target_info, bool skip_download);

#ifdef __cplusplus
}
#endif

#endif // __INIT_H__
