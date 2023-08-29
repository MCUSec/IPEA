/**
 * @file bridge.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Interfaces of IPEA core
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include "TYPES.h"
#include "JLinkARMDLL.h"
#include "SYS.h"
#include "init.h"
#include "target_info.h"

#define IPEA_MAX_RETRY   3

/* Trace analysis results */
#define TRACE_RES_NORMAL          0  ///< No runtime error detected while analyzing the trace
#define TRACE_RES_MEMERR          1  ///< Memory error detected while analyzing the trace
#define TRACE_RES_FAULT           2  ///< Other fault detected while analyzing the trace 

/* Trace decoding results */
#define TRACE_OK                  0  ///< No error occurred while decoding tracing packets
#define TRACE_ERR                -1  ///< Error occurred while decoding tracing packets

/* Target status */
#define TARGET_NORMAL             0  ///< Target terminated normally
#define TARGET_CRASH              1  ///< Target crashed
#define TARGET_TIMEOUT            2  ///< Target timed out
#define TARGET_AGAIN              3  ///< Recoverable error occurred, run the target again
#define TARGET_ERROR              4  ///< Unrecoverable error occurred, failed to run the target


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the global context
 * 
 */
void Context_GlobalInit(void);

/**
 * @brief Initialize the shadow memory for IPEA-San
 * 
 * @param target_info Target information
 */
void Context_ShadowInit(const target_info_t *target_info);

/**
 * @brief De-initialize the shadow memory for IPEA-San
 * 
 * @param target_info Target information
 */
void Context_ShadowDeinit(const target_info_t *target_info);

/**
 * @brief Initialize SEGGER Real Time Terminal (RTT)
 * 
 * @param target_info Target information
 */
void RTT_Init(const target_info_t *target_info);

/**
 * @brief De-initialize SEGGER Real Time Terminal (RTT)
 * 
 * @param target_info 
 */
void RTT_Deinit(const target_info_t *target_info);

/**
 * @brief Start the tracing thread
 * 
 * @param target_info Target information
 * @param exec_index Index of execution
 * @param persist_mode Indicates whether the target is running in persistent mode
 */
void Start_RTT(const target_info_t *target_info, int exec_index, bool persist_mode);

/**
 * @brief Stop the tracing thread
 * 
 */
void Stop_RTT();

/**
 * @brief Dump the call stack of target to a text file, typically used when crash detected
 * 
 * @param outfile Path of text file
 */
void RTT_DumpCallstack(const char *outfile);

/**
 * @brief Parse and decode the trace packets (transmitted through RTT)
 * 
 * @param exec_index Index of the execution
 * @param bitmap Buffer for saving the analysis data
 * @param result Buffer for saving the tracing result
 * @return int Result of parsing trace packets (zero indicates success, or error happend otherwise)
 */
int RTT_Decode(const target_info_t *, int exec_index, U8 *bitmap, int *result);

/**
 * @brief Initialize the debugger
 * 
 * @param target_info Target information
 * @param config_file Path of configuration file
 * @param skip_download Indicates whether skip downloading the firmware
 */
void IPEA_DebuggerInit(target_info_t *target_info, const char *config_file, bool skip_download);

/**
 * @brief Shutdown the debugger
 * 
 * @param target_info Target information
 */
void IPEA_DebuggerShutdown(const target_info_t *target_info);

/**
 * @brief Run the target
 * 
 * @param exec_index Index of current execution
 * @param testcase Input for the target execution
 * @param size Size of input
 * @param timeout Timeout of execution
 * @param exec_ms Buffer for saving the execution time
 * @param bitmap Buffer for saving the analysis data
 * @param persist_mode Enabling persistent mode
 * @param profiling Enabling stack and heap profiling mode
 * @param verbose Enabling verbose mode
 * @return int Execution result. (zero indicates that target terminated normally, or crashed otherwise) 
 */
int IPEA_RunTarget(const target_info_t *, 
                    int exec_index, 
                    U8 *testcase, 
                    U32 size, 
                    U32 timeout, 
                    U32 *exec_ms, 
                    U8 *bitmap, 
                    bool persist_mode, 
                    bool profiling,
                    bool verbose);

#ifdef __cplusplus
}
#endif


#endif