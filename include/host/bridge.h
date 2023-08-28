#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include "TYPES.h"
#include "JLinkARMDLL.h"
#include "SYS.h"
#include "init.h"
#include "target_info.h"

#define AFL_MAX_RETRY   3

/* Trace analysis results */
#define TRACE_RES_NORMAL          0     /* No memory error */
#define TRACE_RES_MEMERR          1     /* Memory error */
#define TRACE_RES_FAULT           2

/* Trace decode results */
#define TRACE_OK       0
#define TRACE_ERR      -1

/* Target status */
#define TARGET_NORMAL  0
#define TARGET_CRASH   1
#define TARGET_TIMEOUT 2
#define TARGET_AGAIN   3
#define TARGET_ERROR   4


#define AFL_EV_RTT_INIT     RTT_INIT_FLAG
#define AFL_EV_START        FUZZ_START_FLAG
#define AFL_EV_FINISH       FUZZ_STOP_FLAG
#define AFL_EV_ABORT        FUZZ_ABORT_FLAG
#define AFL_EV_FAULT        FUZZ_FAULT_FLAG

#define AFL_DFA_ERR_OK  0
#define AFL_DFA_ERR_UNEXPECTED  1
#define AFL_DFA_ERR_TIMEOUT 2
#define AFL_DFA_ERR_FATAL 3

enum TargetStatus {
    INIT = 0,
    START,
    RUN,
    TERMINATED,
    TIMEOUT,
    ERROR
};

#ifdef __cplusplus
extern "C" {
#endif

void IPEA_DebuggerInit(target_info_t *, const char *config_file, bool skip_download);

void IPEA_DebuggerShutdown(const target_info_t *target_info);

void Context_GlobalInit(void);

void Context_ShadowInit(const target_info_t *);

void Context_ShadowDeinit(const target_info_t *);

void RTT_Init(const target_info_t *);

void RTT_Deinit(const target_info_t *target_info);

void Start_RTT(const target_info_t *, int exec_index, bool persist_mode);

void Stop_RTT();

void RTT_DumpCallstack(const char *outfile);

int RTT_Decode(const target_info_t *, int exec_index, U8 *bitmap, int *result);

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