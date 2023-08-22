#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <deque>
#include <list>
#include <iomanip>
#include <assert.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <functional>
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <signal.h>
#include <sched.h>
#include <pthread.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>

#include "bridge.h"
#include "trace.h"
#include "init.h"
#include "common.h"
#include "device_config.h"
#include "JLinkARMDLL.h"
#include "SYS.h"
#include "target_info.h"
#include "context.hpp"

using namespace std;

extern INIT_PARAS _Paras;

extern std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;

struct PacketHandler {
	void (*callback)(const target_info_t *target_info, const Packet_Header *, void *);
	size_t packet_size;
};

static volatile int rtt_decode_result = TRACE_RES_NORMAL;

static volatile bool rtt_thread_exit = false;

static char RTT_BUF[RTT_BUF_SIZE];

static uint32_t trace_rx_bytes = 0;

static uint16_t trace_rx_chksum = 0xFFFF;

static pthread_t trace_tid = (pthread_t)-1;

static map<uint32_t, unique_ptr<Context>> threadContexts;

static deque<unique_ptr<Context>> savedContexts;

static unique_ptr<Context> currentContext;

static uint32_t contextKey;

static atomic_bool tracing(false);

static void testcase_init(const target_info_t *target_info, int exec_index, bool persist_mode) 
{	
	if (!persist_mode || exec_index == 0) {
		// Clear the context of previous test case
		currentContext = nullptr;
		savedContexts.clear();
		threadContexts.clear();

		// if (!strcmp(sanitizer, "usan")) {
		// 	// Initialize the context
		// 	Context::Initialize();
		// }
		// FIXME
		Context::Initialize(target_info);

		// Create the main thread context
		currentContext = make_unique<Context>();
		contextKey = 0;
	}

	// Clear memory error flag
	rtt_decode_result = TRACE_RES_NORMAL;
}

static inline void update_checksum(const uint8_t *buf, size_t length)
{
	static const uint16_t crc16_tab[256] = {
		0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
		0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
		0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
		0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
		0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
		0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
		0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
		0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
		0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
		0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
		0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
		0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
		0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
		0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
		0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
		0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
		0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
		0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
		0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
		0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
		0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
		0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
		0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
		0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
		0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
		0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
		0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
		0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
		0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
		0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
		0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
		0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
	};
	size_t i;

	for (i = 0; i < length; i++) {
		trace_rx_chksum = crc16_tab[(trace_rx_chksum ^ buf[i]) & 0xFF] ^ (trace_rx_chksum >> 8);
	}
}

static void *thread_trace(void *arg)
{
	int r = 0;
	cpu_set_t cpuset;

   	CPU_ZERO(&cpuset);
   	CPU_SET(2, &cpuset);
	sched_setaffinity(0, sizeof(cpuset), &cpuset);

	while (!rtt_thread_exit) {
		if (tracing) {
			r = JLINK_RTTERMINAL_Read(0, RTT_BUF + trace_rx_bytes, RTT_SLOT_SIZE);
			if (unlikely(r < 0)) {
				spdlog::critical("Failed to retrieve trace data");
				continue;
			}
			update_checksum((const uint8_t *)RTT_BUF + trace_rx_bytes, r);
			trace_rx_bytes += r;
			usleep(2000);
		}
	}

	return nullptr;
}

#if 0
static void thread_cleanup_cb(void)
{
	if (trace_tid != (pthread_t)-1) {
		rtt_thread_exit = true;
		pthread_join(trace_tid, nullptr);
	}

	// currentContext = nullptr;
	savedContexts.clear();
	threadContexts.clear();
	currentContext = nullptr;
}
#endif

static uint32_t RTT_flush(const target_info_t *target_info)
{
	U32 trace_total_bytes = 0;
	int remaining = 0;
	int r = 0;
	int retry_count = RTT_MAX_RETRY_COUNT;

	JLINKARM_ReadMemU32(target_info->p_trace_total_rx_bytes, 1, &trace_total_bytes, nullptr);
	
	remaining = trace_total_bytes - trace_rx_bytes;

	if (unlikely(remaining < 0)) {
		spdlog::error("RTT underflow");
		goto rtt_flush_ret;
	}

	if (unlikely(trace_total_bytes > RTT_BUF_SIZE)) {
		spdlog::error("RTT buffer overflow");
		goto rtt_flush_ret;
	}

	while (remaining > 0 && retry_count > 0) {
		r = JLINK_RTTERMINAL_Read(0, RTT_BUF + trace_rx_bytes, remaining);
		if (unlikely(r < 0)) {
			spdlog::error("Failed to retrive trace data");
			goto rtt_flush_ret;
		}

		if (r > 0) {
			update_checksum((const uint8_t *)RTT_BUF + trace_rx_bytes, r);
			remaining -= r;
			trace_rx_bytes += r;
			retry_count = RTT_MAX_RETRY_COUNT;
		} else {
			retry_count--;
		}

		usleep(2000);	
	}

rtt_flush_ret:
	return remaining;
}

void RTT_DumpCallstack(const char *outfile)
{
	ofstream fout(outfile);

	if (!fout.is_open()) {
		spdlog::warn("Failed to open file {} to dump callstack", outfile);
		return;
	}

	if (currentContext)
		currentContext->dumpCallStack(fout);

	for (auto i = savedContexts.rbegin(); i != savedContexts.rend(); i++) {
		fout << std::endl;
		(*i)->dumpCallStack(fout);
	}

	fout.close();
}

void RTT_Init(const target_info_t *target_info) 
{
	JLINK_RTTERMINAL_START rtt_start_param;
	JLINK_RTTERMINAL_BUFDESC rtt_desc;
	int direction = 0;
	int channel_num = 0;
	int r = 0;

	rtt_start_param.ConfigBlockAddress = target_info->p_RTTCB;
	JLINK_RTTERMINAL_Control(JLINKARM_RTTERMINAL_CMD_START, &rtt_start_param);
	direction = JLINKARM_RTTERMINAL_BUFFER_DIR_UP;
	
	do{
		channel_num = JLINK_RTTERMINAL_Control(JLINKARM_RTTERMINAL_CMD_GETNUMBUF, &direction);
	}while(channel_num == -2);
	
	if(channel_num < 0) {
		spdlog::critical("RTT control block not found");
		exit(-1);
	}

	spdlog::debug("RTT channel number: {}", channel_num);

	rtt_desc.BufferIndex = 0;
	rtt_desc.Direction = JLINKARM_RTTERMINAL_BUFFER_DIR_UP;
	r = JLINK_RTTERMINAL_Control(JLINKARM_RTTERMINAL_CMD_GETDESC, &rtt_desc);
	if(r < 0) {
		spdlog::critical("RTT descriptor not found");
		exit(-1);
	}

	r = JLINK_RTTERMINAL_Read(0, RTT_BUF, RTT_SLOT_SIZE);
	spdlog::debug("check read out rtt {}", r);

	unsigned int res = 0;
	JLINKARM_ReadMemU32(target_info->p_trace_total_rx_bytes, 1, &res, NULL);
	spdlog::debug("check total size {}", res);
}

void RTT_Deinit(const target_info_t *target_info)
{
	Stop_RTT();

	if (trace_tid != (pthread_t)-1) {
		rtt_thread_exit = true;
		pthread_join(trace_tid, nullptr);
	}

	currentContext = nullptr;
	savedContexts.clear();
	threadContexts.clear();
}

void Start_RTT(const target_info_t *target_info, int exec_index, bool persist_mode)
{
	testcase_init(target_info, exec_index, persist_mode);
	
	if (unlikely(trace_tid == (pthread_t)-1)) {
		int r = pthread_create(&trace_tid, nullptr, thread_trace, nullptr);
		if (r) {
			spdlog::critical("Failed to create the tracing thread: {}", strerror(r));
			exit(-1);
		}
		// atexit(thread_cleanup_cb);
	}

	trace_rx_bytes = 0;
	trace_rx_chksum = 0xFFFF;
	tracing = true;
}


void Stop_RTT(void)
{
	tracing = false;
}


static void handleIrqEnter(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Irq_Packet *>(phdr);
	const uint8_t irq_num = packet->irq_num;

	if (irq_num) {
		logger.debug("IRQ number: {}", irq_num);
		savedContexts.push_back(move(currentContext));
		currentContext = make_unique<Context>(irq_num);
	} else {
		logger.critical("Not an interrupt context");
		rtt_decode_result = TRACE_RES_FAULT;
	}
}

static void handleNewThreadProp(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const OuterProp_Packet *>(phdr);
	// auto newContext = make_unique<Context>();
	// uint32_t tag = currentContext->getPointerTag(packet->arg_id);
	// int threadId = newContext->getContextId();

	// newContext->transfer(tag);
	// threadContexts[threadId] = move(newContext);

	// logger.debug("Registered a new thread. (id={})", threadId);
	currentContext->createThread(packet->ptr_id);
}

static void handleNewThread(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	if (phdr->opcode == NewThreadFail_OP) {
		currentContext->createThreadNotify(false);
		logger.error("Failed to create a thread");
		return;
	}

	auto packet = reinterpret_cast<const NewThread_Packet *>(phdr);
	auto newContext = currentContext->createThreadNotify(true);
	assert(newContext != nullptr);
	assert(threadContexts.find(packet->thread_id) == threadContexts.end());

	logger.debug("Registered a new thread. (id={})", newContext->getContextId());

	threadContexts[packet->thread_id] = std::move(newContext);
}

static void handleSwitchContext(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const ThreadSwitch_Packet *>(phdr);

	if (contextKey == packet->thread_id)
		return;

	if (threadContexts.find(packet->thread_id) == threadContexts.end()) {
		spdlog::critical("No thread registered! (id={})", packet->thread_id);
		rtt_decode_result = TRACE_RES_FAULT;
		return;
	}

	threadContexts[contextKey] = move(currentContext);
	currentContext = move(threadContexts[packet->thread_id]);
	threadContexts.erase(packet->thread_id);
	contextKey = packet->thread_id;

	logger.debug("Switched to thread {}", currentContext->getContextId());
}

static void handleIrqExit(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Irq_Packet *>(phdr);
	const int16_t irq_num = (int16_t)packet->irq_num;

	logger.debug("IRQ number: {}", irq_num);

	if (!currentContext->isInterrupt()) {
		logger.critical("Not in an interrupt context");
		rtt_decode_result = TRACE_RES_FAULT;
		return;
	}

	if (currentContext->getContextId() != irq_num) {
		logger.critical("Interrupt number doesn't match, expected {}, but received {}", currentContext->getContextId(), irq_num);
		rtt_decode_result = TRACE_RES_FAULT;
		return;
	}

	if (!savedContexts.empty()) {
		currentContext = move(savedContexts.back());
		savedContexts.pop_back();
	} else {
		currentContext = nullptr;
		logger.critical("No saved thread context. Probably caused by the corrupted trace. Retry this case.");
		rtt_decode_result = TRACE_RES_FAULT;
	}
}

static void handleRandom(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Random_Packet *>(phdr);
	U8 *bitmap = (U8 *)user_data;

	logger.debug("Basic block random number: {} ( 0x{:x} )", (int16_t)packet->random, packet->random);

	currentContext->recordBitmap(bitmap, packet->random);
}

static void handleFuncEntry(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const FuncEntry_Packet *>(phdr);

	try {
		const FunctionInfo &funcInfo = Context::GetFunctionInfo(packet->func_id);
		currentContext->pushCallStack(funcInfo);
		logger.debug("Enter function {}, id = 0x{:x}", funcInfo.name, packet->func_id);
	} catch (FunctionError &e) {
		logger.critical("Failed to enter function (id = 0x{:x}). {}", e.funcId, e.desc);
		rtt_decode_result = TRACE_RES_FAULT;
	}
}


static void handleFuncEntryStack(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const FuncEntryStack_Packet *>(phdr);

	try {
		const FunctionInfo &funcInfo = Context::GetFunctionInfo(packet->func_id);
		currentContext->pushCallStack(funcInfo, packet->stack_base, packet->stack_top);
		logger.debug("Enter function {}, id = 0x{:x}, stack_base = 0x{:x}, stack_top = 0x{:x}", 
						funcInfo.name, packet->func_id, packet->stack_base, packet->stack_top);
	} catch (FunctionError &e) {
		logger.critical("Failed to enter function (id = 0x{:x}). {}", e.funcId, e.desc);
		rtt_decode_result = TRACE_RES_FAULT;
	}
}

static void handleFuncArg(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const OuterProp_Packet *>(phdr);
	uint32_t tag = currentContext->getPointerTag(packet->ptr_id);

	logger.debug("Transfer argument tag 0x{:x} (pointer_id = 0x{:x})", tag, packet->ptr_id);

	currentContext->transfer(tag);
}

static void handleFuncRetParam(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const OuterProp_Packet *>(phdr);
	uint32_t tag = currentContext->setPointerTag(packet->ptr_id);

	logger.debug("Assigned return tag 0x{:x} to pointer 0x{:x}", tag, packet->ptr_id);
}

static void handleFuncExit(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	const FunctionInfo &funcInfo = currentContext->GetCurrentFunctionInfo();

	if (phdr->opcode == FuncExitProp_OP) {
		auto packet = reinterpret_cast<const OuterProp_Packet *>(phdr);
		currentContext->popCallStack(packet->ptr_id);
	} else {
		currentContext->popCallStack();
	}
	
	logger.debug("Exit function {}, id = 0x{:x}", funcInfo.name, funcInfo.address);

	if (!currentContext->active()) {
		if (currentContext->isInterrupt())
			logger.debug("Exit IRQ. irq_num = {}", currentContext->getContextId());
		else
			logger.debug("Exit context {}", currentContext->getContextId());

		if (!savedContexts.empty()) {
			currentContext = move(savedContexts.back());
			savedContexts.pop_back();
		} else {
			logger.critical("No saved context");
			rtt_decode_result = TRACE_RES_FAULT;
			currentContext = nullptr;
		}
	}
}

static void handleAlloca(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Alloca_Packet *>(phdr);
	
	try {
		uint32_t tag = currentContext->addStackObject(packet->addr, packet->size);
		currentContext->setPointerTag(packet->id, tag);
		logger.info("Allocate a stack object at 0x{:x}, size: {}, tag: 0x{:x}", packet->addr, packet->size, tag);
	} catch (...) {
		rtt_decode_result = TRACE_RES_MEMERR;
	}
}

static void handleMalloc(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Malloc_Packet *>(phdr);

	if (!packet->addr) {
		logger.info("Failed to allocate heap object. size = {}", packet->size);
		currentContext->setPointerTag(packet->id, 0);
		return;
	}

	uint32_t tag = Context::AddHeapObject(packet->addr, packet->size);

	logger.debug("Allocated a heap object (size = {}) @ 0x{:x}", packet->size, packet->addr);
	currentContext->setPointerTag(packet->id, tag);
}

static void handleFree(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Free_Packet *>(phdr);

	try {
		Context::RemoveHeapObject(packet->addr);
		logger.debug("Released the heap object @ 0x{:x}", packet->addr);
	} catch (MemoryErrorInfo e) {
		logger.info("{} detected when attempt to release object @ 0x{:x}", e.desc, e.memAddress);
		rtt_decode_result = TRACE_RES_MEMERR;
	}
}

static void handleRealloc(const target_info_t *target_info, const Packet_Header *phdr, void *user_data)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Realloc_Packet *>(phdr);

	if (packet->old_addr) {
		// clear tags at the old address
		try {
			Context::RemoveHeapObject(packet->old_addr);
		} catch (MemoryErrorInfo e) {
			logger.info("{} detected when attempt to re-allocate the object @ 0x{:x}", e.desc, e.memAddress);
			rtt_decode_result = TRACE_RES_MEMERR;
			return;
		}
	}

	if (packet->new_addr) {
		uint32_t tag = Context::AddHeapObject(packet->new_addr, packet->size);
		logger.debug("Reallocated a heap object (size = {}) @ 0x{:x}", packet->size, packet->new_addr);
		currentContext->setPointerTag(packet->id, tag);
	} else {
		logger.info("Failed to realloc heap object at 0x{:x} (size = {})", packet->new_addr, packet->size);
	}
}

static void handleProp(const target_info_t *target_info, const Packet_Header *phdr, void *user_data) 
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Prop_Packet *>(phdr);
	const uint32_t destId = packet->dst_id;
	const uint32_t srcId = packet->src_id;
	uint32_t tag = currentContext->getPointerTag(srcId);

	logger.debug("Pointer propagation: from 0x{:x} to 0x{:x} (tag = 0x{:x})", srcId, destId, tag);
	currentContext->setPointerTag(destId, tag);
}

static void handleCheck(const target_info_t *target_info, const Packet_Header *phdr, void *user_data) 
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	auto packet = reinterpret_cast<const Load_Store_Check_Packet *>(phdr);
	const uint32_t pointerId = packet->tar_id;
	const uint32_t accessAddr = packet->addr;
	const uint32_t accessSize = packet->size;

	logger.info("Checking pointer dereference: pointer_id = 0x{:x}, address = 0x{:x}, length = {}", pointerId, accessAddr, accessSize);
	try {
		currentContext->pointerCheck(pointerId, accessAddr, accessSize);
	} catch (MemoryErrorInfo e) {
		logger.info("{} detected @ 0x{:x}, expected tag: 0x{:x}, real tag: 0x{:x}", e.desc, e.memAddress, e.expTag, e.realTag);
		rtt_decode_result = TRACE_RES_MEMERR;
	}
}

static const PacketHandler decode_handler[] = {
	[Thread_Switch_OP] = { .callback = handleSwitchContext, .packet_size = sizeof(ThreadSwitch_Packet) },
	[NewThreadProp_OP] = { .callback = handleNewThreadProp, .packet_size = sizeof(OuterProp_Packet)},
	[NewThreadSuccess_OP] = { .callback = handleNewThread, .packet_size = sizeof(NewThread_Packet) },
	[NewThreadFail_OP] = { .callback = handleNewThread, .packet_size = sizeof(NewThread_Packet) },
	[Irq_Enter_OP] = { .callback = handleIrqEnter, .packet_size = sizeof(Irq_Packet)},
	[Irq_Exit_OP] = { .callback = handleIrqExit, .packet_size = sizeof(Irq_Packet)},
	[Random_OP] = {.callback = handleRandom, .packet_size = sizeof(Random_Packet)},
	[FuncEntry_OP] = {.callback = handleFuncEntry, .packet_size = sizeof(FuncEntry_Packet)},
	[FuncEntryStack_OP] = { .callback = handleFuncEntryStack, .packet_size = sizeof(FuncEntryStack_Packet)},
	[Prop_OP] = { .callback = handleProp, .packet_size = sizeof(Prop_Packet)},
	[ArgProp_OP] = { .callback = handleFuncArg, .packet_size = sizeof(OuterProp_Packet)},
	[RetProp_OP] = { .callback = handleFuncRetParam, .packet_size = sizeof(OuterProp_Packet)},
	[FuncExit_OP] = { .callback = handleFuncExit, .packet_size = sizeof(FuncExit_Packet)},
	[FuncExitProp_OP] = { .callback = handleFuncExit, .packet_size = sizeof(OuterProp_Packet)},
	[Alloca_OP] = { .callback = handleAlloca, .packet_size = sizeof(Alloca_Packet)},
	[Malloc_OP] = { .callback = handleMalloc, .packet_size = sizeof(Malloc_Packet)},
	[Free_OP] = { .callback = handleFree, .packet_size = sizeof(Free_Packet)},
	[Realloc_OP] = { .callback = handleRealloc, .packet_size = sizeof(Realloc_Packet)},
	[Check_OP] = {.callback =  handleCheck, .packet_size = sizeof(Load_Store_Check_Packet)},
	[__MAX_OPCODE] = { nullptr, 0 }
};

static inline bool trace_integrity_check(const target_info_t *target_info)
{
	uint16_t trace_tx_chksum;
	JLINKARM_ReadMemU16(target_info->p_trace_rx_chksum, 1, &trace_tx_chksum, nullptr);
	return trace_tx_chksum == trace_rx_chksum;
}

int RTT_Decode(const target_info_t *target_info, int exec_index, U8 *bitmap, int *result)
{
	spdlog::logger logger(__func__, file_sink);
	logger.set_level(spdlog::level::debug);

	uint32_t remainBytes = RTT_flush(target_info);
	
	if (remainBytes > 0) {
		spdlog::error("{} bytes data couldn't be retrieved!", remainBytes);
		return TRACE_ERR;
	}

	if (!trace_integrity_check(target_info)) {
		spdlog::error("CRC error. Try again");
		return TRACE_ERR;
	}

	logger.debug("Total trace size: {} bytes", trace_rx_bytes);

	for (uint32_t offset = 0; offset < trace_rx_bytes && rtt_decode_result == TRACE_RES_NORMAL; ) {
		Packet_Header *phdr = (Packet_Header *)(RTT_BUF + offset);
		const int opcode = (int)phdr->opcode;

		if (unlikely(opcode >= __MAX_OPCODE || !decode_handler[opcode].callback)) {
			spdlog::error("Unexpected opcode - {}", opcode);
			return TRACE_ERR;
		}

		if (currentContext && 
			(currentContext->active() || opcode == Thread_Switch_OP 
				|| opcode == Irq_Enter_OP || opcode == FuncEntry_OP 
				|| opcode == FuncEntryStack_OP)) {
			decode_handler[opcode].callback(target_info, phdr, bitmap);
			offset += decode_handler[opcode].packet_size;
		} else {
			spdlog::error("No active context. opcode = {}", opcode);
			return TRACE_ERR;
		}
	}

	if (result)
		*result = rtt_decode_result;

	logger.info("max heap usage: {}", Context::GetMaxHeapUsage());

	return TRACE_OK;
}
