/**
 * @file context.hpp
 * @author Jiameng Shi (jiameng@uga.edu)
 * @brief Interfaces of IPEA-San
 * @version 0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */
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

/**
 * @brief Abstract a subroutine instance
 * 
 */
class Subroutine {
public:
    Subroutine(Context *, const FunctionInfo &, uint32_t, uint32_t);
    Subroutine(Context *, const FunctionInfo &);
    ~Subroutine();

public:
    /**
     * @brief Get the function ID of this subroutine
     * 
     * @return Function ID
     */
    uint32_t getId(void) const {
        return mFuncInfo.address;
    }

    /**
     * @brief Get the stack base
     * 
     * @return Stack base
     */
    uint32_t getStackBase(void) const {
        return mStackBase;
    }

    /**
     * @brief Get the stack top
     * 
     * @return Stack top
     */
    uint32_t getStackTop(void) const {
        return mStackTop;
    }

    /**
     * @brief Get the function information of this subroutine
     * 
     * @return Function information
     */
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

/**
 * @brief Abstract a context
 * 
 */
class Context {
public:
    Context();
    Context(int irqNumber): mCtxId(irqNumber), mIsIrqCtx(true) {}
    ~Context();

public:
    /**
     * @brief Get the Context ID
     * 
     * @return Context ID
     */
    int getContextId(void) const {
        return mCtxId;
    }

    /**
     * @brief 
     * 
     * @return true - interrupt context 
     * @return false - thread context
     */
    bool isInterrupt(void) const {
        return mIsIrqCtx;
    }

    /**
     * @brief Transfer a memory tag when a pointer is propagating
     * 
     * @param tag Memory tag
     */
    void transfer(uint32_t tag) {
        mTagQueue.push_back(tag);
    }

    /**
     * @brief Get the current function information
     * 
     * @return Function Information
     */
    const FunctionInfo &GetCurrentFunctionInfo() const {
        return mCurrentSubroutine->GetFunctionInfo();
    }

    /**
     * @brief Get the current subroutine
     * 
     * @return Instance of current subroutine
     */
    const Subroutine *getCurrentSubroutine() const {
        return mCurrentSubroutine.get();
    }

    /**
     * @brief Record the coverage (used by IPEA-Fuzz)
     * 
     * @param bitmap Bitmap
     * @param random The random number of the basic basic
     */
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

    /**
     * @brief Add a stack object
     * 
     * @param address address of the stack object
     * @param size size of the stack object
     * @return Memory tag of the stack object
     */
    uint32_t addStackObject(uint32_t address, size_t size);

    /**
     * @brief Push a function onto the call stack (when the function is called)
     * 
     * @param func_info Function information
     * @param stack_base Base of the stack
     * @param stack_top Top of the stack
     */
    void pushCallStack(const FunctionInfo &func_info, uint32_t stack_base, uint32_t stack_top);   

    /**
     * @brief Push a function onto the call stack (when the function is called)
     * 
     * @param func_info Function information
     */
    void pushCallStack(const FunctionInfo &func_info);

    /**
     * @brief Pop a function from the call stack (when the function returns)
     * 
     */
    void popCallStack(void);

    /**
     * @brief Pop a function from the call stack while returning a pointer
     * 
     * @param pointer_id The pointer ID of the pointer to be returned
     */
    void popCallStack(uint32_t pointer_id);

    /**
     * @brief Dump the call stack
     * 
     * @param stream The output stream
     */
    void dumpCallStack(std::ostream &stream);

    /**
     * @brief Bind a memory tag to the specified pointer
     * 
     * @param pointer_id Pointer ID
     * @return Tag bound to the pointer 
     */
    uint32_t setPointerTag(uint32_t pointer_id);

    /**
     * @brief Bind a memory tag to the specified pointer
     * 
     * @param pointer_id Pointer ID
     * @param tag Memory tag
     */
    void setPointerTag(uint32_t pointer_id, uint32_t tag);

    /**
     * @brief Get the memory tag of the specified pointer
     * 
     * @param pointer_id Pointer ID
     * @return Memory tag
     */
    uint32_t getPointerTag(uint32_t pointer_id);

    /**
     * @brief Check a pointer dereference
     * 
     * @param pointer_id Pointer ID
     * @param address The address pointed by the pointer
     * @param length Length
     */
    void pointerCheck(uint32_t pointer_id, uint32_t address, size_t length);

    /**
     * @brief Track the event of thread creation
     * 
     * @param param_id Pointer ID of the thread parameter
     */
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
    /**
     * @brief Get the function information specified by the function ID
     * 
     * @param id Function ID
     * @return Function Information 
     */
    static const FunctionInfo &GetFunctionInfo(uint32_t id);

    /**
     * @brief Get the memory tag by the specified memory address
     * 
     * @param address Memory address
     * @return Memory tag
     */
    static uint32_t GetMemoryTag(uint32_t address);

    /**
     * @brief Add a heap object
     * 
     * @param address Address of the heap object
     * @param size Size of the heap object
     * @return Memory tag of the heap object
     */
    static uint32_t AddHeapObject(uint32_t address, size_t size);

    /**
     * @brief Remove a heap object
     * 
     * @param address Address of the heap object
     */
    static void RemoveHeapObject(uint32_t address);

    /**
     * @brief Check the memory access
     * 
     * @param address Address to be accessed
     * @param length Length to be accessed
     * @param tag Expected memory tag
     */
    static void AccessCheck(uint32_t address, size_t length, uint32_t tag = 0);

    /**
     * @brief Initialize the shadow memory
     * 
     */
    static void Initialize(const target_info_t *);

    // static std::unique_ptr<Context> CreateThreadContext();

    /**
     * @brief Create a thread context
     * 
     * @return A thread context 
     */
    static std::unique_ptr<Context> CreateThreadContext(uint32_t);

    /**
     * @brief Create an interrupt context
     * 
     * @return An interrupt context
     */
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