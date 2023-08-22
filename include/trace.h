#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>

enum OPCODE {
	Thread_Switch_OP,
	NewThreadProp_OP,
	NewThreadSuccess_OP,
	NewThreadFail_OP,
	Irq_Enter_OP,
	Irq_Exit_OP,
	Random_OP,
	FuncEntry_OP,
	FuncEntryStack_OP,
	Prop_OP,
	ArgProp_OP,
	RetProp_OP,
	FuncExit_OP,
	FuncExitProp_OP,
	Alloca_OP,
	Malloc_OP,
	Free_OP,
	Realloc_OP,
	Check_OP,
	__MAX_OPCODE
};

struct Packet_Header {
	uint8_t opcode;
}__attribute__((packed));

struct ThreadSwitch_Packet{
	struct Packet_Header phdr;
	uint32_t thread_id;
}__attribute__((packed));

struct NewThreadProp_Packet {
	struct Packet_Header phdr;
	uint32_t arg_id;
}__attribute__((packed));

struct NewThread_Packet {
	struct Packet_Header phdr;
	uint32_t thread_id;
}__attribute__((packed));

struct Irq_Packet{
	struct Packet_Header phdr;
	uint8_t irq_num;
}__attribute__((packed));

struct Random_Packet{
	struct Packet_Header phdr;
	uint16_t random;
}__attribute__((packed));

struct FuncEntry_Packet{
	struct Packet_Header phdr;
	uint32_t func_id;
}__attribute__((packed));

struct FuncEntryStack_Packet{
	struct Packet_Header phdr;
	uint32_t func_id;
	uint32_t stack_base;
	uint32_t stack_top;
}__attribute__((packed));

struct Prop_Packet{
	struct Packet_Header phdr;
	uint32_t dst_id;
	uint32_t src_id;
}__attribute__((packed));

struct OuterProp_Packet {
	struct Packet_Header phdr;
	uint32_t ptr_id;
}__attribute__((packed));

struct FuncExit_Packet{
	struct Packet_Header phdr;
}__attribute__((packed));

struct FuncExitProp_Packet {
	struct Packet_Header phdr;
	uint32_t ret_id;
}__attribute__((packed));

struct ArgProp_Packet{
	struct Packet_Header phdr;
	uint32_t arg_id;
}__attribute__((packed));

struct RetProp_Packet{
	struct Packet_Header phdr;
	uint32_t ret_param;
}__attribute__((packed));

struct Alloca_Packet {
	struct Packet_Header phdr;
	uint32_t id;
	uint32_t addr;
	uint32_t size;
}__attribute__((packed));

struct Malloc_Packet{
	struct Packet_Header phdr;
	uint32_t id;
	uint32_t addr;
	uint32_t size;
}__attribute__((packed));

struct Free_Packet{
	struct Packet_Header phdr;
	uint32_t addr;
}__attribute__((packed));

struct Realloc_Packet{
	struct Packet_Header phdr;
	uint32_t id;
	uint32_t new_addr;
	uint32_t old_addr;
	uint32_t size;
}__attribute__((packed));

struct Load_Store_Check_Packet{
	struct Packet_Header phdr;
	uint32_t tar_id;
	uint32_t addr;
	uint32_t size;
}__attribute__((packed));

struct Memcpy2Ptr_Packet{
	struct Packet_Header phdr;
	uint32_t tar_addr;
	uint32_t src_value;
	uint32_t size;
}__attribute__((packed));

#endif