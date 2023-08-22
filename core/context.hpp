#ifndef CONTEXT_H
#define CONTEXT_H

#include <cstdint>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <ostream>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types
#include "spdlog/sinks/basic_file_sink.h"
#include "target_info.h"

#define TAG_NUM_POS      3
#define TAG_MASK         7

#define TAG_INVALID      0   // 0b000
#define TAG_STACK        1   // 0b001
#define TAG_HEAP         2   // 0b010
#define TAG_HEAP_FREED   3   // 0b011
#define TAG_GLOBAL       4   // 0b100
#define TAG_PERIPHERAL   5   // 0b101

class Context;

struct VariableInfo {
    std::string name;
    int32_t offset;
    size_t length;

    VariableInfo(std::string name, int32_t offset, size_t length): 
                    name(name), offset(offset), length(length) {}
};

struct FunctionInfo {
	std::string name;
	uint32_t address;
	std::vector<VariableInfo> locVars;

    FunctionInfo(std::string name, uint32_t address):
                    name(name), address(address) {}
    FunctionInfo() = default;
};

struct Error {
    std::string desc;
    Error(const char *str): desc(str) {}
};

struct MemoryErrorInfo: public Error {
    const uint32_t memAddress;
    const uint32_t expTag;
    const uint32_t realTag;

    MemoryErrorInfo(const char *desc, uint32_t address, uint32_t exp_tag, uint32_t real_tag):
                        Error(desc), memAddress(address), expTag(exp_tag), realTag(real_tag) {}
};

struct FunctionError: public Error {
    const int funcId;
    
    FunctionError(const char *desc, int funcId): Error(desc), funcId(funcId) {}
};

struct ContextError: public Error {
    const int ctxId;

    ContextError(const char *desc, int ctxId): Error(desc), ctxId(ctxId) {}
};

class Subroutine {
public:
    Subroutine(Context *, const FunctionInfo &, uint32_t, uint32_t);
    Subroutine(Context *, const FunctionInfo &);
    ~Subroutine();

public:
    uint32_t getId(void) const {
        return mFuncInfo.address;
    }

    uint32_t getStackBase(void) const {
        return mStackBase;
    }

    uint32_t getStackTop(void) const {
        return mStackTop;
    }

    const FunctionInfo &GetFunctionInfo() const {
        return mFuncInfo;
    }

    uint16_t getCurrentBBNum() const {
        return mCurrentBB;
    }

private:
    void passArguments(void);

private:
    Context *mParent;
    const FunctionInfo &mFuncInfo;
    const uint32_t mStackBase;
    const uint32_t mStackTop;
    const bool mNeedClear;
    std::map<uint32_t, uint32_t> mLocalTags;
    uint16_t mCurrentBB = 0;
    friend class Context;
};

class Context {
public:
    Context();
    Context(int irqNumber): mCtxId(irqNumber), mIsIrqCtx(true) {}
    ~Context();

public:
    int getContextId(void) const {
        return mCtxId;
    }

    bool isInterrupt(void) const {
        return mIsIrqCtx;
    }

    void transfer(uint32_t tag) {
        mTagQueue.push_back(tag);
    }

    const FunctionInfo &GetCurrentFunctionInfo() const {
        return mCurrentSubroutine->GetFunctionInfo();
    }

    const Subroutine *getCurrentSubroutine() const {
        return mCurrentSubroutine.get();
    }

    void recordBitmap(unsigned char *bitmap, uint16_t random) { 
        mAFLCurLoc = random;
        
        if (bitmap) {
            uint32_t index = (mAFLCurLoc ^ mAFLPrevLoc) % (1 << 16);
            bitmap[index]++;
        }

        mAFLPrevLoc = mAFLCurLoc >> 1;
        mCurrentSubroutine->mCurrentBB = random;
    }

    bool active() const {
        return mCurrentSubroutine != nullptr;
    }

    uint32_t addStackObject(uint32_t address, size_t size);

    void pushCallStack(const FunctionInfo &, uint32_t, uint32_t);   

    void pushCallStack(const FunctionInfo &);

    void popCallStack(void);

    void popCallStack(uint32_t pointer_id);

    void dumpCallStack(std::ostream &stream);

    uint32_t setPointerTag(uint32_t pointer_id);

    void setPointerTag(uint32_t pointer_id, uint32_t tag);

    uint32_t getPointerTag(uint32_t pointer_id);

    void pointerCheck(uint32_t pointer_id, uint32_t address, size_t length);

    void createThread(uint32_t param_id);

    std::unique_ptr<Context> createThreadNotify(bool success) {
        std::unique_ptr<Context> context = nullptr;

        if (success) {
            context = newThreadCallback ? newThreadCallback() : Context::CreateThreadContext(0);
        }

        newThreadCallback = nullptr;
        return context;
    }

public:
    static const FunctionInfo &GetFunctionInfo(uint32_t id);

    static uint32_t GetMemoryTag(uint32_t address);

    static uint32_t AddHeapObject(uint32_t address, size_t size);

    static void RemoveHeapObject(uint32_t address);

    static void AccessCheck(uint32_t address, size_t length, uint32_t tag = 0);

    static void Initialize(const target_info_t *);

    // static std::unique_ptr<Context> CreateThreadContext();

    static std::unique_ptr<Context> CreateThreadContext(uint32_t);

    static std::unique_ptr<Context> CreateInterruptContext(int);

    static size_t GetMaxHeapUsage(void);

    static size_t GetHeapUsage(void);

public:
    static const target_info_t *TargetInfo;

private:
    static std::map<uint32_t, uint32_t> MemoryTags;
    static std::map<uint32_t, size_t> HeapObjects;
    static size_t HeapUsage, MaxHeapUsage;
    static int ThreadCount;

private:
    // std::unique<Context> createThreadCallback(uint32_t thread_id, uint32_t param_tag);
    std::function<std::unique_ptr<Context>()> newThreadCallback = nullptr; 
    const int mCtxId;
    const bool mIsIrqCtx;
    uint32_t mAFLPrevLoc = 0x0;
	uint32_t mAFLCurLoc = 0x0;
    std::vector<std::unique_ptr<Subroutine>> mCallStack;
    std::deque<uint32_t> mTagQueue;
    std::unique_ptr<Subroutine> mCurrentSubroutine = nullptr;
    friend class Subroutine;
};

#endif // CONTEXT_H