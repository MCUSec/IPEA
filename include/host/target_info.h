/**
 * @file target_info.h
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Interfaces and definitions for target information
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef TARGET_INFO_H
#define TARGET_INFO_H

#include <stdbool.h>
#include <stdint.h>

struct target_info {
    char hexfile[128];
    // download address
    uint32_t p_load;
    // entry point
    uint32_t p_entry;
    // target memory
    uint32_t flash_base;
    uint32_t flash_size;
    uint32_t sram_base;
    uint32_t sram_size;
    // address of trace symbols
    uint32_t p_RTTCB;
    uint32_t p_trace_total_rx_bytes;
    uint32_t p_trace_rx_chksum;
    uint32_t p_trace_enabled;
    uint32_t p_trace_locked;
    // address of fuzz symbols
    uint32_t p_fuzz_inpbuf;
    uint32_t p_fuzz_inpbuf_size;
    uint32_t p_fuzz_input_len;
    // address of profiling symbols
    uint32_t p_max_stack_usage;
    uint32_t p_max_heap_usage;
};

typedef struct target_info target_info_t;

#define TARGET_IN_TRACE_MODE(ti) ( \
    (ti)->p_RTTCB && \
    (ti)->p_trace_total_rx_bytes && \
    (ti)->p_trace_rx_chksum && \
    (ti)->p_trace_enabled && \
    (ti)->p_trace_locked \
)

#define TARGET_IN_FUZZ_MODE(ti) ( \
    (ti)->p_fuzz_inpbuf && \
    (ti)->p_fuzz_input_len \
)

#ifdef __cplusplus
extern "C" {
#endif

bool parse_target(const char *filename, target_info_t *target_info);

#ifdef __cplusplus
}
#endif

#endif // TARGET_INFO_H
