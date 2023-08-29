/**
 * @file init.c
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Source code of J-Link module
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#include "init.h"
#include "device_config.h"
#include "common.h"
#include "JLinkARMDLL.h"
#include "bridge.h"

static void _cbLogOutHandler(const char* sLog) {
  (void) sLog;
}

static void _cbErrorOutHandler(const char* sError) {
  _ErrorOut(sError);
}

void InitJLink(INIT_PARAS *_Paras, target_info_t *target_info, const char *config_file)
{
    config_t cfg;
    config_setting_t *setting;
    const char *strval;
    int intval;

    config_init(&cfg);
    memset(_Paras, 0, sizeof(INIT_PARAS));

    if (!config_file || !config_file[0])
        config_file = "jlink.conf";

    if (!config_read_file(&cfg, config_file)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), 
                        config_error_line(&cfg),
                        config_error_text(&cfg));
    }

    if (config_lookup_string(&cfg, "target_if", &strval)) {
        if (!strcmp(strval, "SWD") || !strcmp(strval, "swd")) {
            _Paras->TargetIF = JLINKARM_TIF_SWD;
        } else if (!strcmp(strval, "JTAG") || !strcmp(strval, "jtag")) {
            _Paras->TargetIF = JLINKARM_TIF_JTAG;
        } else {
            fprintf(stderr, "WARNING: unknown target interface -' %s'. Use default setting: 'SWD'\n", strval);
            _Paras->TargetIF = JLINKARM_TIF_SWD;
        }
    }

    if (config_lookup_string(&cfg, "host_if", &strval)) {
        if (!strcmp(strval, "USB") || !strcmp(strval, "usb")) {
            _Paras->HostIF = JLINKARM_HOSTIF_USB;
        } else if (!strcmp(strval, "IP") || !strcmp(strval, "ip")) {
            _Paras->HostIF = JLINKARM_HOSTIF_IP;
            if (config_lookup_string(&cfg, "jlink_addr", &strval)) {
                strncpy(_Paras->sHost, strval, sizeof(_Paras->sHost) - 1);
            } else {
                fprintf(stderr, "WARNING: use default J-Link IP address: 10.0.0.1");
                strcpy(_Paras->sHost, "10.0.0.1");
            }
        } else {
            fprintf(stderr, "WARNING: unknown host interface - '%s'. Use default setting: 'USB'\n", strval);
            _Paras->HostIF = JLINKARM_HOSTIF_USB;
        }
    }

    if (config_lookup_int(&cfg, "speed", &intval)) {
        _Paras->Speed = intval;
    } else {
        _Paras->Speed = 4000;
    }

    if (config_lookup_int(&cfg, "serial", &intval)) {
        _Paras->SerialNo = intval;
    } else {
        fprintf(stderr, "WARNING: use default J-Link S/N: 123456\n");
        _Paras->SerialNo = 123456;
    }

    if (config_lookup_string(&cfg, "device", &strval)) {
        strncpy(_Paras->sDevice, strval, sizeof(_Paras->sDevice) - 1);
    } else {
        fprintf(stderr, "ERROR: no target device specified\n");
        exit(1);
    }

    if (config_lookup_int(&cfg, "flash_base", &intval)) {
        target_info->flash_base = intval;
    } else {
        target_info->flash_base = 0;
    }

    if (config_lookup_int(&cfg, "flash_size", &intval)) {
        target_info->flash_size = intval;
    } else {
        fprintf(stderr, "ERROR: flash size is not specified\n");
        exit(1);
    }

    if (config_lookup_int(&cfg, "sram_base", &intval)) {
        target_info->sram_base = intval;
    } else {
        target_info->sram_base = 0x20000000;
    }

    if (config_lookup_int(&cfg, "sram_size", &intval)) {
        target_info->sram_size = intval;
    } else {
        fprintf(stderr, "ERROR: SRAM size is not specified\n");
        exit(1);
    }

    if (config_lookup_string(&cfg, "project_file", &strval)) {
        strncpy(_Paras->sSettingsFile, strval, sizeof(_Paras->sSettingsFile) - 1);
    } else {
        strcpy(_Paras->sSettingsFile, "Settings.jlink");
    }

    config_destroy(&cfg);
}

void InitDebugSession(INIT_PARAS *_Paras){
    const char* sError = NULL;
    int r = 0;
    U8 acIn[0x100] = {0x0};
    U8 acOut[0x100] = {0x0};
    if(_Paras->HostIF == JLINKARM_HOSTIF_USB){
        r = JLINKARM_EMU_SelectByUSBSN(_Paras->SerialNo);
		if(r < 0)
			_ErrorOut("Serial Number not exist");
    }else{
        _ErrorOut("IP interface is not supported yet");
    }
    sError = JLINKARM_OpenEx(_cbLogOutHandler, _cbErrorOutHandler);
    if(sError){
        _ErrorOut(sError);
    }
    strcpy(acIn, "ProjectFile = ");
    strcat(acIn, _Paras->sSettingsFile);
    JLINKARM_ExecCommand(acIn, acOut, sizeof(acOut));
    if (acOut[0]) {
        _ErrorOut(acOut);
    }
    memset(acIn, 0x0, 0x100);
    strcpy(acIn, "device = ");
    strcat(acIn, _Paras->sDevice);
    JLINKARM_ExecCommand(acIn, &acOut[0], sizeof(acOut));
    if (acOut[0]) {
        _ErrorOut(acOut);
    }
    JLINKARM_TIF_Select(_Paras->TargetIF);
    JLINKARM_SetSpeed(_Paras->Speed);
    r = JLINKARM_Connect();
	if(r < 0)
		_ErrorOut("Fail to connect JLink");
    // JLINK_DownloadFile(_Paras->sBinary, _Paras->StartAddress);
}

void CloseDebugSession(const target_info_t *target_info)
{
    Context_ShadowDeinit(target_info);
    JLINK_Reset();
    JLINKARM_Close();
}

void ResetDebugSession(INIT_PARAS *_Paras)
{
    JLINKARM_Close();
    InitDebugSession(_Paras);
}

void InitFirmware(const target_info_t *target_info, bool skip_download) {
    Context_ShadowInit(target_info);
    JLINKARM_Reset();
    if (!skip_download) {
        JLINK_DownloadFile(target_info->hexfile, target_info->flash_base);
	JLINKARM_Reset();
	JLINKARM_Go();
    }
}
