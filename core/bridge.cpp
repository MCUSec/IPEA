#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include <fstream>
#include <iostream>

#include "init.h"
#include "bridge.h"
#include "device_config.h"
#include "common.h"
#include "target_info.h"

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types
#include "spdlog/sinks/basic_file_sink.h"

#define WAIT_BEFORE_RTT_INIT        10000
#define WAIT_BEFORE_MAIN            10000
#define WAIT_FUZZ_STOP              200000

#define TARGET_EVENT_RTT_INIT     RTT_INIT_FLAG
#define TARGET_EVENT_START        FUZZ_START_FLAG
#define TARGET_EVENT_FINISH       FUZZ_STOP_FLAG
#define TARGET_EVENT_ABORT        FUZZ_ABORT_FLAG
#define TARGET_EVENT_FAULT        FUZZ_FAULT_FLAG

#define IPEA_STATE_OK  0
#define IPEA_STATE_UNEXPECTED  1
#define IPEA_STATE_TIMEOUT 2
#define IPEA_STATE_FATAL 3

enum TargetStatus {
    INIT = 0,
    START,
    RUN,
    TERMINATED,
    TIMEOUT,
    ERROR
};

std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink = nullptr;

void IPEA_DebuggerInit(target_info_t *target_info, const char *config_file, bool skip_download)
{	
	INIT_PARAS _Paras;
    InitJLink(&_Paras, target_info, config_file);
    InitDebugSession(&_Paras);
	InitFirmware(target_info, skip_download);	// FIXME
	if (TARGET_IN_TRACE_MODE(target_info))
		RTT_Init(target_info);
}

void IPEA_DebuggerShutdown(const target_info_t *target_info)
{
	RTT_Deinit(target_info);
	CloseDebugSession(target_info);
}

static inline bool _WriteInput(const target_info_t *target_info, U8 *testcase, U32 size)
{
	if (!TARGET_IN_FUZZ_MODE(target_info) || !testcase || !size)
		return false;

	for (int i = 0; i < 3; i++) {
		if (size == JLINKARM_WriteMem(target_info->p_fuzz_inpbuf, size, testcase)) {
			JLINKARM_WriteU32(target_info->p_fuzz_input_len, size);
			return true;
		}
		usleep(10000);
	}

	return false;
}


static int _WaitEvent(const U8 expected, const U32 timeout, U8 *happend = nullptr)
{
	U32 regPC;
	U16 fetchedInst;
	U8 happendEvent;
	int r;

	r = JLINKARM_WaitForHalt(timeout);
	if (r == 1) {
		regPC = JLINKARM_ReadReg((ARM_REG)JLINKARM_CM3_REG_R15);
		JLINKARM_ReadMemU16(regPC, 1, &fetchedInst, nullptr);

		happendEvent = fetchedInst & 0xFF;

		if (happend)
			*happend = happendEvent;

		if (happendEvent != expected)
			return IPEA_STATE_UNEXPECTED;

		return IPEA_STATE_OK;
	} else if (r == 0) {
		return IPEA_STATE_TIMEOUT;
	} else {
		spdlog::error("JLINKARM_WaitForHalt() error: {}", r);
		return IPEA_STATE_FATAL;
	}
}

static inline void _ResetTarget(const target_info_t *target_info)
{
	JLINKARM_Reset();
	JLINKARM_WriteReg((ARM_REG)JLINKARM_CM3_REG_R15, target_info->p_entry);
}

static void _ProcessTimeout(const target_info_t *target_info)
{
	uint8_t rtt_locked = 1;

	if (!JLINKARM_IsHalted())
		JLINKARM_Halt();

	JLINKARM_WriteU8(target_info->p_trace_enabled, 0);

	do {
		if (JLINKARM_IsHalted())
			JLINKARM_Go();
		usleep(2000);
		if (!JLINKARM_IsHalted())
			JLINKARM_Halt();
		JLINKARM_ReadMemU8(target_info->p_trace_locked, 1, &rtt_locked, nullptr);
	} while (rtt_locked);
}

static int _TraceDecode(const target_info_t *target_info, int exec_index, U8 *bitmap)
{
	int res = TRACE_RES_NORMAL;

	if (RTT_Decode(target_info, exec_index, bitmap, &res))
		return TRACE_ERR;

	return res;
}

/**
 * @brief 
 * 
 * @param target_info 
 * @param exec_index 
 * @param testcase 
 * @param size 
 * @param timeout 
 * @param exec_ms 
 * @param bitmap 
 * @param persist_mode 
 * @param profiling 
 * @param verbose 
 * @return int 
 */
int IPEA_RunTarget(const target_info_t *target_info, 
					int exec_index, 
					U8 *testcase, 
					U32 size, 
					U32 timeout, 
					U32 *exec_ms, 
					U8 *bitmap, 
					bool persist_mode,
					bool profiling, 
					bool verbose)
{
	char filename[64];
	snprintf(filename, sizeof(filename) - 1, "tracelog_%d.txt", exec_index);
	file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	spdlog::debug("Running testcase {}", exec_index);

	int target_status = TARGET_NORMAL;
	U32 initTime, finiTime;
	U8 happend_event;
	int res, trace_res;
	bool halt = false;

	auto status = TargetStatus::INIT;
	auto getTimestamp = []() {
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	};

	if (size > target_info->p_fuzz_inpbuf_size){
		size = target_info->p_fuzz_inpbuf_size;
	}

	// Reset target
	if (!persist_mode || exec_index == 0) {
		_ResetTarget(target_info);
	} else {
		if (TARGET_IN_TRACE_MODE(target_info)) {
			Start_RTT(target_info, exec_index, true);
		}
		JLINKARM_WriteU32(target_info->p_trace_total_rx_bytes, 0);
		JLINKARM_WriteU16(target_info->p_trace_rx_chksum, 0xFFFF);
		status = TargetStatus::START;
	}

	// Main loop of the state machine

	if (exec_ms)
		*exec_ms = 0; // initialize execution time
	
	while (!halt) {
		if (status != TargetStatus::TERMINATED && status != TargetStatus::TIMEOUT) {
			JLINKARM_Step();
			JLINKARM_Go();
		}
		
		switch (status) {
		case TargetStatus::INIT:
			res = _WaitEvent(TARGET_EVENT_RTT_INIT, WAIT_BEFORE_RTT_INIT, &happend_event);
			if (res == IPEA_STATE_OK) {
				if (TARGET_IN_TRACE_MODE(target_info)) { // FIXME 1
					if (!profiling) {
						Start_RTT(target_info, exec_index, false);
						logger.info("RTT initialized");		
					} else {
						JLINKARM_WriteU8(target_info->p_trace_enabled, 0);
					}
				}

				if (TARGET_IN_FUZZ_MODE(target_info)) { // FIXME 2
					status = TargetStatus::START;
				} else {
					initTime = getTimestamp();
					status = TargetStatus::RUN;
				}
			} else {
				logger.error("Unexpected event occurred or waiting for RTT initialization timed out: 0x{:x}", happend_event);
				return TARGET_ERROR;
			}
			break;

		case TargetStatus::START:
			res = _WaitEvent(TARGET_EVENT_START, WAIT_BEFORE_MAIN, &happend_event);
			if (res == IPEA_STATE_OK) {
				logger.info("Reach fuzz start point");
				if (_WriteInput(target_info, testcase, size)) {
					logger.info("Written testcase: {} bytes", size);
					status = TargetStatus::RUN;
					initTime = getTimestamp();
				} else {
					spdlog::error("Failed to write testcase");
					return TARGET_ERROR;
				}				
			} else {
				logger.error("Unexpected event occurred or waiting for fuzz start timed out: 0x{:x}", happend_event);
				return TARGET_ERROR;
			}
			break;

		case TargetStatus::RUN:
			logger.info("Target is running");
			res = _WaitEvent(TARGET_EVENT_FINISH, timeout, &happend_event);
			finiTime = getTimestamp();

			if (TARGET_IN_TRACE_MODE(target_info))
				Stop_RTT();

			if (exec_ms)
				*exec_ms = finiTime - initTime;

			switch(res) {
			case IPEA_STATE_OK:
			case IPEA_STATE_UNEXPECTED:
				status = TargetStatus::TERMINATED;
				break;

			case IPEA_STATE_TIMEOUT:
				_ProcessTimeout(target_info);
				status = TargetStatus::TIMEOUT;
				break;

			case IPEA_STATE_FATAL:
				spdlog::error("Debug dongle error");
				return TARGET_ERROR;

			default:
				// Never reach here
				spdlog::error("Unexpected result - {}", res);
				abort();
			}
			break;

		case TargetStatus::TERMINATED:
			logger.info("Terminated. Execution time: {} ms", finiTime - initTime);
			target_status = happend_event == TARGET_EVENT_FINISH ? TARGET_NORMAL : TARGET_CRASH;
			halt = true;
			break;

		case TargetStatus::TIMEOUT:
			logger.info("Timeout");
			target_status = TARGET_TIMEOUT;
			halt = true;
			break;

		default:
			// Never reach here
			spdlog::error("Unexpected status - {}", status);
			abort();
		}
	}

	if (TARGET_IN_TRACE_MODE(target_info)) {
		trace_res = _TraceDecode(target_info, exec_index, bitmap);
		if (trace_res == TRACE_ERR || trace_res == TRACE_RES_FAULT) {
			remove(filename);
			return TARGET_AGAIN;
		}

		if (trace_res != TRACE_RES_NORMAL)
			target_status = TARGET_CRASH;
		
		logger.info("Trace analysis result: {}", trace_res);
	}

	if (profiling) {
		if (target_info->p_max_stack_usage) {
			unsigned int max_stack_usage;
			JLINKARM_ReadMemU32(target_info->p_max_stack_usage, 1, &max_stack_usage, nullptr);
			logger.info("max stack usage: {}", max_stack_usage);
		}

		if (target_info->p_max_heap_usage) {
			unsigned int max_heap_usage;
			JLINKARM_ReadMemU32(target_info->p_max_heap_usage, 1, &max_heap_usage, nullptr);
			logger.info("max heap usage: {}", max_heap_usage);
		}
	}
	
	if (!verbose && (target_status == TARGET_NORMAL || target_status == TARGET_TIMEOUT))
		remove(filename);

	return target_status;
}
