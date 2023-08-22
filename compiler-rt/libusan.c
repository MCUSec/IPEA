#include <stdbool.h>
#include "libusan_conf.h"
#include "RTT/SEGGER_RTT.h"
#include "trace.h"
#include "fuzz.h"

bool __usan_trace_enabled = false;

bool __usan_bb_trace_enabled = false;

bool __usan_rtt_lock = false;

static void __func_entry(uint32_t func_id
#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
    , uint32_t sp
#endif
);

static void __func_entry_stack(uint32_t func_id, uint32_t stack_base, uint32_t stack_top);

static inline void __usan_check(uint32_t tar_id, uint32_t addr, size_t size);

#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
uint32_t __max_stack_usage;
#endif

static inline void __usan_send_packet(uint8_t opcode, struct Packet_Header *phdr, size_t pkt_size)
{
    if (unlikely(opcode >= __MAX_OPCODE)) {
        __asm volatile("bkpt #0x47");
        while (1) {}
    }

    if (likely(__usan_trace_enabled)) {   
        phdr->opcode = opcode;
        SEGGER_RTT_LOCK();
        __usan_rtt_lock = true;
        SEGGER_RTT_WriteNoLock(0, phdr, pkt_size);
        __usan_rtt_lock = false;
        SEGGER_RTT_UNLOCK();
    }
}

EXPORT void __usan_trace_switch_context(uint32_t thread_id)
{
    struct ThreadSwitch_Packet packet = {
        .thread_id = thread_id,
    };
    __usan_send_packet(Thread_Switch_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_new_thread(uint32_t arg_id)
{
    struct OuterProp_Packet packet = {
        .ptr_id = arg_id
    };
    __usan_send_packet(NewThreadProp_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_new_thread_on_success(uint32_t thread_id)
{
    struct NewThread_Packet packet = {
        .thread_id = thread_id
    };
    __usan_send_packet(NewThreadSuccess_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_new_thread_on_fail(void)
{
    struct NewThread_Packet packet = {
        .thread_id = 0
    };
    __usan_send_packet(NewThreadFail_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_irq_entry(void)
{
    struct Irq_Packet packet;
    __asm volatile("mrs %0, ipsr" : "=r"(packet.irq_num) : : "memory");
    __usan_send_packet(Irq_Enter_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_irq_exit(uint16_t irq_num)
{
    struct Irq_Packet packet = {
        .irq_num = irq_num,
    };
    __usan_send_packet(Irq_Exit_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_basicblock(uint16_t random)
{
    if (__usan_bb_trace_enabled) {
        struct Random_Packet packet = {
            .random = random,
        };
        __usan_send_packet(Random_OP, (struct Packet_Header *)&packet, sizeof(packet));
    } 
}

ASM EXPORT void __usan_trace_func_entry_stack(uint32_t func_id)  /* naked */
{
    __asm volatile(
        "mov r1, r7\n"
        "mov r2, sp\n" 
        "b __func_entry_stack\n"
    );
}

ASM EXPORT void __usan_trace_func_entry(uint32_t func_id){ /* naked */
    __asm volatile(
#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
        "mov r1, sp\n"
#endif
        "b __func_entry\n"
    );
}

ASM EXPORT void __usan_trace_func_entry_stack_prop(uint32_t func_id)  /* naked */
{
    __asm volatile(
        "mov r1, r7\n"
        "mov r2, sp\n"
        "b __func_entry_stack\n"
    );
}

ASM EXPORT void __usan_trace_func_entry_prop(uint32_t func_id){ /* naked */
    __asm volatile(
#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
        "mov r1, sp\n"
#endif
        "b __func_entry\n"
    );
}

EXPORT void __usan_trace_prop(uint32_t tar, uint32_t src)
{
    if (likely(tar != src)) {
        struct Prop_Packet packet= {
            .dst_id = tar,
            .src_id = src,
        };
        __usan_send_packet(Prop_OP, (struct Packet_Header *)&packet, sizeof(packet));
    }
}

EXPORT void __usan_trace_func_exit(void)
{
    struct FuncExit_Packet packet;
    __usan_send_packet(FuncExit_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_func_exit_prop(uint32_t ret_id)
{
    struct OuterProp_Packet packet = { .ptr_id = ret_id };
    __usan_send_packet(FuncExitProp_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_callsite(uint8_t args, ...)
{
    va_list valist;
    uint8_t i;

    va_start(valist, args);

    for (i = 0; i < args; i++) {
        struct OuterProp_Packet packet = {
            .ptr_id = va_arg(valist, uint32_t),
        };
        __usan_send_packet(ArgProp_OP, (struct Packet_Header *)&packet, sizeof(packet));
    }

    va_end(valist);
}

EXPORT void __usan_trace_return_prop(uint32_t ret_param)
{
    struct OuterProp_Packet packet = {
        .ptr_id = ret_param,
    };
    __usan_send_packet(RetProp_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_alloca(uint32_t id, uint32_t addr, uint32_t size)
{
    struct Alloca_Packet packet = {
        .id = id,
        .addr = addr,
        .size = size,
    };
    __usan_send_packet(Alloca_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_malloc(uint32_t id, uint32_t addr, uint32_t size)
{
    struct Malloc_Packet packet = {
        .id = id,
        .addr = addr,
        .size = size,
    };
    __usan_send_packet(Malloc_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_free(uint32_t addr)
{
    struct Free_Packet packet = {
        .addr = addr,
    };
    __usan_send_packet(Free_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_realloc(uint32_t id, uint32_t new_addr, uint32_t old_addr, uint32_t size)
{
    struct Realloc_Packet packet = {
        .id = id,
        .new_addr = new_addr,
        .old_addr = old_addr,
        .size = size,
    };
    __usan_send_packet(Realloc_OP, (struct Packet_Header *)&packet, sizeof(packet));
}

EXPORT void __usan_trace_load_store(uint32_t ptr_id, uint32_t addr, size_t size)
{
    __usan_check(ptr_id, addr, size);
}


__attribute__((constructor))
void __usan_initialize(void)
{
    __usan_trace_enabled = true;
    __usan_rtt_lock = false;
    __asm volatile("bkpt #0x99");
}


__attribute__((used))
static void __func_entry(uint32_t func_id
#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
    , uint32_t sp
#endif
)
{
    struct FuncEntry_Packet packet = {
        .func_id = func_id,
    };
#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
    const uint32_t stack_start = *(uint32_t *)(*(uint32_t *)0xE000ED08); 
    if (stack_start - sp > __max_stack_usage)
        __max_stack_usage = stack_start - sp;
#endif
    __usan_send_packet(FuncEntry_OP, (struct Packet_Header *)&packet, sizeof(packet));
  
}

__attribute__((used))
static void __func_entry_stack(uint32_t func_id, uint32_t stack_base, uint32_t stack_top)
{
    struct FuncEntryStack_Packet packet = {
        .func_id = func_id,
        .stack_base = stack_base,
        .stack_top = stack_top
    };
#if defined(USAN_ENABLE_PROFILING) && USAN_ENABLE_PROFILING
    const uint32_t stack_start = *(uint32_t *)(*(uint32_t *)0xE000ED08); 
    if (stack_start - stack_top > __max_stack_usage)
        __max_stack_usage = stack_start - stack_top;
#endif
    __usan_send_packet(FuncEntryStack_OP, (struct Packet_Header *)&packet, sizeof(packet));
}


static inline void __usan_check(uint32_t tar_id, uint32_t addr, size_t size)
{
    if (__usan_bb_trace_enabled){
        struct Load_Store_Check_Packet packet = {
            .tar_id = tar_id,
            .addr = addr,
            .size = size,
        };
        __usan_send_packet(Check_OP, (struct Packet_Header *)&packet, sizeof(packet));
    }
}