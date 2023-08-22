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

void InitJLink(INIT_PARAS *, target_info_t *, const char *config_file);

void InitDebugSession(INIT_PARAS *);

void CloseDebugSession(const target_info_t *target_info);

void ResetDebugSession(INIT_PARAS *_Paras);

void InitFirmware(const target_info_t *target_info, bool skip_download);

void GetInitParameters(INIT_PARAS *, FILE *);

#ifdef __cplusplus
}
#endif

#endif // __INIT_H__
