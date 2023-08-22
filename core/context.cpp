#include "context.hpp"

#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>

typedef uint32_t shadow_t;

static std::map<uint32_t, FunctionInfo> FuncInfo;
static std::vector<VariableInfo> GlobalVariables;
static uint32_t TagSeed;
// static uint32_t ShadowSRAM[TARGET_SRAM_SIZE];
// static uint32_t ShadowFlash[TARGET_FLASH_SIZE];

static shadow_t *ShadowSRAM;
static shadow_t *ShadowFlash;

static std::map<uint32_t, shadow_t> ShadowPeripherals;

extern std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;

const target_info_t *Context::TargetInfo = nullptr;

static inline bool isSRAMAddress(const target_info_t *target_info, uint32_t target_address)
{
    return target_address >= target_info->sram_base && 
            target_address < target_info->sram_base + target_info->sram_size;
}

static inline bool isFlashAddress(const target_info_t *target_info, uint32_t target_address)
{
    return target_address >= target_info->flash_base && 
            target_address < target_info->flash_base + target_info->flash_size;
}

// static inline bool isPeripheralAddress(const target_info_t *target_info, uint32_t target_address)
// {
//     return target_address >= 0x40000000 && target_address < 0x60000000
//             || target_address >= 0xE0000000 && target_address <= 0xFFFFFFFF;

// }

static inline shadow_t *getShadowAddress(const target_info_t *target_info, uint32_t target_address)
{
    if (isSRAMAddress(target_info, target_address))
        return ShadowSRAM + (target_address - target_info->sram_base);

    if (isFlashAddress(target_info, target_address))
        return ShadowFlash + (target_address - target_info->flash_base);

    // if (isPeripheralAddress(target_info, target_address) && ShadowPeripherals.find(target_address) != ShadowPeripherals.end()) {
    //     return &ShadowPeripherals[target_address];
    // }

    return nullptr;
}

Subroutine::Subroutine(Context *parent, const FunctionInfo &func_info, uint32_t stack_base, uint32_t stack_top) : 
                mParent(parent), mFuncInfo(func_info), mStackBase(stack_base), mStackTop(stack_top), mNeedClear(true)
{
    spdlog::logger logger(__func__, file_sink);
    logger.set_level(spdlog::level::debug);

    for (auto &localVar : mFuncInfo.locVars) {
        auto address = localVar.offset > 0 ? mStackTop + localVar.offset : mStackBase + localVar.offset;
        const uint32_t tag = (TagSeed << TAG_NUM_POS) | TAG_STACK;
        shadow_t *shadowAddress = getShadowAddress(Context::TargetInfo, address);
        logger.debug("Assigned tag 0x{:x} for local variable '{}' @ 0x{:x}", tag, localVar.name, address);
        std::fill(shadowAddress, shadowAddress + localVar.length, tag);
        TagSeed++;
    }

    passArguments();
}

Subroutine::Subroutine(Context *parent, const FunctionInfo &func_info):
                mParent(parent), mFuncInfo(func_info), mStackBase(0), mStackTop(0), mNeedClear(false)
{
    passArguments();
}

Subroutine::~Subroutine()
{
    if (mNeedClear) {
        for (auto &localVar : mFuncInfo.locVars) {
            auto address = localVar.offset > 0 ? mStackTop + localVar.offset : mStackBase + localVar.offset;
            shadow_t *shadowAddress = getShadowAddress(Context::TargetInfo, address);
            const uint32_t tag = *shadowAddress;
            std::fill(shadowAddress, shadowAddress + localVar.length, 0);
        }
    }
}

void Subroutine::passArguments(void) {
    for (auto argId = -1; !mParent->mTagQueue.empty(); argId--) {
        if (mParent->mTagQueue.empty()) {
            throw FunctionError("No argument passed", mFuncInfo.address);
        }
        mLocalTags.emplace(argId, mParent->mTagQueue.front());
        mParent->mTagQueue.pop_front();
    }
}

std::map<uint32_t, uint32_t> Context::MemoryTags;
std::map<uint32_t, size_t> Context::HeapObjects;
size_t Context::HeapUsage = 0;
size_t Context::MaxHeapUsage = 0;
int Context::ThreadCount = 0;

Context::Context(): mIsIrqCtx(false), mCtxId(Context::ThreadCount++)
{
    // mCtxId = Context::ThreadCount++;
}

Context::~Context()
{
    mCallStack.clear();
    mCurrentSubroutine = nullptr;
}

const FunctionInfo &Context::GetFunctionInfo(uint32_t id)
{
    if (FuncInfo.find(id) == FuncInfo.end()) {
        // Throw an exception if function ID does not exist
        throw FunctionError("No function entry found", id);
    }

    return FuncInfo[id];
}

uint32_t Context::GetMemoryTag(uint32_t address)
{
    auto shadowAddress = getShadowAddress(Context::TargetInfo, address);
    
    if (shadowAddress)
        return *shadowAddress;
    
    if (ShadowPeripherals.find(address) != ShadowPeripherals.end())
        return ShadowPeripherals[address];

    return 0;
}

uint32_t Context::AddHeapObject(uint32_t address, size_t size)
{
    if (!address)
        return 0;

    shadow_t *shadowAddress = getShadowAddress(Context::TargetInfo, address);
    const uint32_t tag = (TagSeed << TAG_NUM_POS) | TAG_HEAP;

    if (shadowAddress) {
        std::fill(shadowAddress, shadowAddress + size, tag);
        Context::HeapObjects[address] = size;
        Context::HeapUsage += size;
        if (Context::HeapUsage > Context::MaxHeapUsage) {
            Context::MaxHeapUsage = Context::HeapUsage;
        }
    }

    TagSeed++;

    return tag;
}

void Context::RemoveHeapObject(uint32_t address)
{
    shadow_t *shadowAddress = getShadowAddress(Context::TargetInfo, address);
    
    if (Context::HeapObjects.find(address) != Context::HeapObjects.end()) {    
        const size_t objSize = Context::HeapObjects[address];
        std::fill(shadowAddress, shadowAddress + objSize, TAG_HEAP_FREED);
        Context::HeapObjects.erase(address);
        Context::HeapUsage -= objSize;
    } else {
        shadow_t tag = *shadowAddress;
        const char *desc = nullptr;

        if ((tag & TAG_MASK) == TAG_HEAP_FREED)
            desc = "Double-free";
        else
            desc = "Non-heap-object-free";

        throw MemoryErrorInfo(desc, address, TAG_HEAP, tag);
    }
}

size_t Context::GetMaxHeapUsage(void)
{
    return Context::MaxHeapUsage;
}

size_t Context::GetHeapUsage(void)
{
    return Context::HeapUsage;
}

void Context::AccessCheck(uint32_t address, size_t length, uint32_t tag)
{
    if (tag == 1 || (!isSRAMAddress(Context::TargetInfo, address) && !isFlashAddress(Context::TargetInfo, address)))
        return;

    for (size_t offset = 0; offset < length; offset++) {
        const uint32_t realTag = Context::GetMemoryTag(address + offset);
        if (tag != realTag) {
            const char *desc = nullptr;
            if ((realTag & TAG_MASK) == TAG_HEAP_FREED) {
                desc = "Use-after-free";
            } else {
                switch (tag & TAG_MASK) {
                case TAG_STACK:
                    desc = "Stack buffer overflow";
                    break;
                case TAG_HEAP:
                    desc = "Heap buffer overflow";
                    break;
                case TAG_GLOBAL:
                    desc = "Global buffer overflow";
                    break;
                case TAG_PERIPHERAL:
                    desc = "Peripheral buffer overflow";
                    break;
                default:
                    desc = "Invalid memory tag";
                    break;
                }
            }
            throw MemoryErrorInfo(desc, address + offset, tag, realTag);
        }
    }
}

void Context::Initialize(const target_info_t *target_info)
{
    spdlog::logger logger("init", file_sink);

    std::fill(ShadowSRAM, ShadowSRAM + target_info->sram_size, 0);
    std::fill(ShadowFlash, ShadowFlash + target_info->flash_size, 0);
    TagSeed = 0xabcd;
    Context::MemoryTags.clear();

    // Assign tag for each global variable
    for (auto &gv : GlobalVariables) {
        auto shadowAddress = getShadowAddress(Context::TargetInfo, (uint32_t)gv.offset);
        if (shadowAddress) {
            const uint32_t tag = (TagSeed << TAG_NUM_POS) | TAG_GLOBAL;
            logger.info("Assigned tag for global variable '{}': 0x{:x},  address: 0x{:x}, length: {}", gv.name, TagSeed, gv.offset, gv.length);
            std::fill(shadowAddress, shadowAddress + gv.length, tag);
            TagSeed++;
        }
    }
}

uint32_t Context::addStackObject(uint32_t address, size_t size)
{
    const uint32_t tag = (TagSeed << TAG_NUM_POS) | TAG_STACK;
    auto shadowAddress = getShadowAddress(Context::TargetInfo, address);
    
    if (!shadowAddress) {
        throw MemoryErrorInfo("Invalid memory address", address, 0, 0);
    }

    std::fill(shadowAddress, shadowAddress + size, tag);
    TagSeed++;

    return tag;
}

void Context::pushCallStack(const FunctionInfo &func_info, uint32_t stack_base, uint32_t stack_top)
{
    if (mCurrentSubroutine) {
        mCallStack.emplace_back(std::move(mCurrentSubroutine));
    }

    mCurrentSubroutine = std::make_unique<Subroutine>(this, func_info, stack_base, stack_top);
}

void Context::pushCallStack(const FunctionInfo &func_info)
{
    if (mCurrentSubroutine) {
        mCallStack.emplace_back(std::move(mCurrentSubroutine));
    }

    mCurrentSubroutine = std::make_unique<Subroutine>(this, func_info);
}

void Context::popCallStack(void)
{
    if (!mCallStack.empty()) {
        mCurrentSubroutine = std::move(mCallStack.back());
        mCallStack.pop_back();
    } else {
        mCurrentSubroutine = nullptr;
    }
}

void Context::popCallStack(uint32_t pointer_id) 
{    
    uint32_t tag = getPointerTag(pointer_id);
    mTagQueue.push_back(tag);

    popCallStack();
}

void Context::dumpCallStack(std::ostream &stream)
{
    int dumpIdx = 0;

    if (isInterrupt())
        stream << "In interrupt context: IRQ" << mCtxId << std::endl;
    else
        stream << "In thread T" << mCtxId << std::endl; 

    if (!mCurrentSubroutine) {
        stream << "Inactive context!" << std::endl;
        return;
    }

    stream << dumpIdx++ << ": " << mCurrentSubroutine->GetFunctionInfo().name 
            << ", BB: 0x" << std::hex << mCurrentSubroutine->getCurrentBBNum() << std::dec << std::endl;

    for (auto i = mCallStack.rbegin(); i != mCallStack.rend(); i++) {
        stream << dumpIdx++ << ": " << (*i)->GetFunctionInfo().name 
                << ", BB: 0x" << std::hex << (*i)->getCurrentBBNum() << std::dec << std::endl;
    }
}

uint32_t Context::setPointerTag(uint32_t pointer_id) {
    if (!mTagQueue.empty()) {
        uint32_t tag = mTagQueue.front();
        setPointerTag(pointer_id, tag);
        mTagQueue.pop_front();
        return tag;
    }

    throw;
}

void Context::setPointerTag(uint32_t pointer_id, uint32_t tag) {
    if ((int32_t)pointer_id < 0) {
        mCurrentSubroutine->mLocalTags[pointer_id] = tag;
    } else {
        Context::MemoryTags[pointer_id] = tag;
    }
}

uint32_t Context::getPointerTag(uint32_t pointer_id) {
    if (pointer_id == 1)
        return 1;

    // FIXME
    if ((int32_t)pointer_id > 0 && !(pointer_id & (1 << 30)) && Context::MemoryTags.find(pointer_id) == Context::MemoryTags.end()) {
        return 1;
    }
        
    if ((int32_t)pointer_id > 0 && (pointer_id & (1 << 30))) {
        const shadow_t *shadowAddress = getShadowAddress(Context::TargetInfo, pointer_id & ~(1 << 30));
        return shadowAddress ? *shadowAddress : 1;
    }

    return (int32_t)pointer_id < 0 ? mCurrentSubroutine->mLocalTags[pointer_id] : Context::MemoryTags[pointer_id];
}

void Context::pointerCheck(uint32_t pointer_id, uint32_t address, size_t length) {
    if (!(isSRAMAddress(Context::TargetInfo, address) || isFlashAddress(Context::TargetInfo, address)) || pointer_id == 1)
        return;

    uint32_t tag = getPointerTag(pointer_id);
    Context::AccessCheck(address, length, tag);
}

void Context::createThread(uint32_t param_id) {
    newThreadCallback = std::bind(Context::CreateThreadContext, getPointerTag(param_id));
}

// std::unique_ptr<Context> Context::CreateThreadContext() {
//     return std::make_unique<Context>();
// }

std::unique_ptr<Context> Context::CreateThreadContext(uint32_t param_tag) {
    auto context = std::make_unique<Context>();
    context->transfer(param_tag);
    return std::move(context);
}

std::unique_ptr<Context> Context::CreateInterruptContext(int irqNumber) {
    return std::make_unique<Context>(irqNumber);
}

static void Context_LoadPassLog(void)
{
    const char *file_path = "/tmp/mcu_sanitizer_dwarf.json";
    std::ifstream in(file_path);

    if (!in.is_open()) {
        spdlog::error("Failed to open {}", file_path);
        exit(1);
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(in, root)) {
        spdlog::error("Failed to parse json");
        in.close();
        exit(1);
    }

    for (auto gv_item : root["global_var"]) {
        GlobalVariables.emplace_back(gv_item["var_name"].asString(), gv_item["addr"].asInt(), gv_item["byte_size"].asUInt());
    }

    for (auto fn_addr : root["func"].getMemberNames()) {
        auto fn_item = root["func"][fn_addr];
        const uint32_t addr = (uint32_t)stoi(fn_addr);
        FuncInfo.emplace(addr, FunctionInfo(fn_item["name"].asString(), addr));
        auto &fi = FuncInfo[addr];
        for (auto lv_item : fn_item["local_var"]) {
            fi.locVars.emplace_back(lv_item["var_name"].asString(), lv_item["offset"].asInt(), lv_item["byte_size"].asUInt());
        }
    }

    in.close();
}

#if 0
static void Context_LoadPeripherals(void)
{
    const char *file_path = "peripherals.json";
    std::ifstream in(file_path);

    if (!in.is_open()) {
        spdlog::warn("Peripheral description file not found");
        return;
    }

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(in, root)) {
        spdlog::error("Failed to parse peripheral description file");
        in.close();
        return;
    }

    for (auto peripheral : root) {
        const uint32_t base = peripheral["base"].asUInt();
        for (auto reg : peripheral["regs"]) {
            const uint32_t address = base + reg["offset"].asUInt();
            const int size = reg["size"].asUInt() / 8;
            for (int i = 0; i < size; i++) {
                ShadowPeripherals[address + i] = (TagSeed << TAG_NUM_POS) | TAG_PERIPHERAL;
            }
            TagSeed++;
        }
    }

}
#endif

extern "C" {
    void Context_GlobalInit(void)
    {
        Context_LoadPassLog();
        // Context_LoadPeripherals();
    }

    void Context_ShadowInit(const target_info_t *target_info)
    {
        unsigned flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE;

        ShadowFlash = (shadow_t *)mmap(nullptr, target_info->flash_size * sizeof(shadow_t), PROT_READ | PROT_WRITE, flags, -1, 0);
        if (ShadowFlash == MAP_FAILED) {
            fprintf(stderr, "ERROR: Failed to generate shadow memory for target flash\n");
            ShadowFlash = nullptr;
            exit(1);
        }

        ShadowSRAM = (shadow_t *)mmap(nullptr, target_info->sram_size * sizeof(shadow_t), PROT_READ | PROT_WRITE, flags, -1, 0);
        if (ShadowSRAM == MAP_FAILED) {
            fprintf(stderr, "ERROR: failed to create shadow memory for target SRAM\n");
            ShadowSRAM = nullptr;
            exit(1);
        }

        Context::TargetInfo = target_info;
    }

    void Context_ShadowDeinit(const target_info_t *target_info)
    {
        if (ShadowFlash)
            munmap(ShadowFlash, target_info->flash_size * sizeof(shadow_t));

        if (ShadowSRAM)
            munmap(ShadowSRAM, target_info->sram_size * sizeof(shadow_t));
    }
}
