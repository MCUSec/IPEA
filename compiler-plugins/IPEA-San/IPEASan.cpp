#include "llvm/Pass.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CommandLine.h"

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <vector>
#include <functional>
#include <regex>

#include "IPEASan.h"


#define DEBUG_TYPE "IPEA-San"

using namespace llvm;

STATISTIC(NumInstrumentedCheck, "Number of instrumented reads and writes");
STATISTIC(NumInstrumentedProp, "Number of instrumented propagation");
STATISTIC(NumInstrumentedArgProp, "Number of argument propagation");
STATISTIC(NumInstrumentedFuncProp, "Number of instrumented function propagation");
STATISTIC(NumInstrumentedFuncRetProp, "Number of instrumented function returns (w/. propagation)");
STATISTIC(NumInstrumentedFuncEntry, "Number of instrumented function entries");
STATISTIC(NumInstrumentedFuncRet, "Number of instrumented function returns");
STATISTIC(NumInstrumentedISR, "Number of instrumented ISRs");

static cl::opt<std::string> kUsanFunctionRules("function-rule", cl::desc("specify functions rules"));

namespace{
    std::string log_module_name;
    std::string log_global;
    std::string log_func;
    bool module_inited = false;
    std::string module_extend_ctor_name;
    short IdCounter = 0; // increase
    short global_IdCounter = -32768; // increase
    short func_IdCounter = 0; // increase
    bool has_stack_op = false;
    // std::map<StringRef, Value*> m_globals_metadata;
    std::vector<std::string> m_globals_metadata_name;
    std::vector<Value*> m_globals_metadata_id;
    std::map<Value*, int> m_present_in_original;
    std::map<Value*, Value*> m_prop_chain;
    std::map<Value*, Value*> m_pointer_metadata;
    std::map<PHINode*, PHINode*> m_phi_base;
    std::map<PHINode*, PHINode*> m_phi_bound;
    std::map<Value*, Value*> m_ptr2int_metadata;
    std::map<const AllocaInst*, bool> ProcessedAllocas;
    StringMap<bool> m_alert_funcs;
    StringMap<bool> m_func_def_special;
    StringMap<bool> m_func_wrappers_available;
    StringMap<bool> m_func_transformed;
    StringMap<bool> closed_func;
    Type* VoidType;
    Type* Int8Type;
    Type* Int16Type;
    Type* Int32Type;
    Type* VoidPtrType;
    // const DataLayout *DL;
    ConstantPointerNull* m_void_null_ptr;
    Value* m_infinite_bound_ptr;
    Value* m_null_value; // 0 for NULL as func arg
    Value* m_infinite_value; // 1 for infinite as func pointer as func arg and address of integer as arg
    Instruction* main_insert_point = NULL;
    FunctionCallee cb_fuzz_init;
    FunctionCallee cb_fuzz_finish;

    FunctionCallee cb_irq_entry;
    FunctionCallee segger_rtt_exit_irq;
    FunctionCallee cb_bb_entry;
    FunctionCallee cb_func_entry;
    FunctionCallee cb_func_entry_stack;
    FunctionCallee cb_func_entry_prop;
    FunctionCallee cb_func_entry_stack_prop;
    FunctionCallee cb_prop;

    FunctionCallee cb_func_exit;
    FunctionCallee cb_func_exit_prop;
    FunctionCallee cb_arg_prop;
    FunctionCallee cb_ret_prop;
    FunctionCallee cb_malloc;
    FunctionCallee cb_free;
    FunctionCallee cb_realloc;
    FunctionCallee cb_load_store_check;

    Instruction* ctor_global_insert_point = NULL;
#if ENABLE_LOOP_OPTIMIZATION
    SmallVector<Loop *, 16> LoopNotInstrumentTrans;
    SmallVector<Loop *, 16> LoopNotInstrumentMem;
#endif

    // const char *AnnotationString = "interruptHandler";
    // std::set<Function*> annotFuncs;
    // std::set<Function*> mallocFuncs;
    // std::set<Function*> freeFuncs;
    // std::set<Function*> noSanitize;

    std::map<std::string, std::set<std::string>> funcRules;

    std::set<std::string> syscalls{
        // task.h
        "xTaskCreateStatic",
        "xTaskCreateRestrictedStatic",
        "vTaskAllocateMPURegions",
        "vTaskDelete",
        "xTaskDelayUntil",
        "xTaskAbortDelay",
        "uxTaskPriorityGet",
        "uxTaskPriorityGetFromISR",
        "eTaskGetState",
        "vTaskGetInfo",
        "vTaskPrioritySet",
        "vTaskSuspend",
        "vTaskResume",
        "xTaskResumeFromISR",
        "pcTaskGetName",
        "xTaskGetHandle",
        "uxTaskGetStackHighWaterMark",
        "uxTaskGetStackHighWaterMark2",
        "vTaskSetApplicationTaskTag",
        "xTaskGetApplicationTaskTag",
        "xTaskGetApplicationTaskTagFromISR",
        "vTaskSetThreadLocalStoragePointer",
        "pvTaskGetThreadLocalStoragePointer",
        "vApplicationStackOverflowHook",
        "vApplicationGetIdleTaskMemory",
        "xTaskCallApplicationTaskHook",
        "xTaskGetIdleTaskHandle",
        "uxTaskGetSystemState",
        "vTaskList",
        "vTaskGetRunTimeStats",
        "ulTaskGetIdleRunTimeCounter",
        "xTaskGenericNotify",
        "xTaskGenericNotifyFromISR",
        "xTaskGenericNotifyWait",
        "vTaskGenericNotifyGiveFromISR",
        "ulTaskGenericNotifyTake",
        "xTaskGenericNotifyStateClear",
        "ulTaskGenericNotifyValueClear",
        "vTaskSetTimeOutState",
        "xTaskCheckForTimeOut",
        "vTaskPlaceOnEventList",
        "vTaskPlaceOnUnorderedEventList",
        "vTaskPlaceOnEventListRestricted",
        "xTaskRemoveFromEventList",
        "vTaskRemoveFromUnorderedEventList",
        "xTaskGetCurrentTaskHandle",
        "xTaskPriorityInherit",
        "xTaskPriorityDisinherit",
        "vTaskPriorityDisinheritAfterTimeout",
        "uxTaskGetTaskNumber",
        "vTaskSetTaskNumber",
        "pvTaskIncrementMutexHeldCount",
        "vTaskInternalSetTimeOutState",
        // timers.h
        "xTimerCreate",
        "xTimerCreateStatic",
        "pvTimerGetTimerID",
        "vTimerSetTimerID",
        "xTimerIsTimerActive",
        "xTimerGetTimerDaemonTaskHandle",
        "xTimerGenericCommand",
        "xTimerPendFunctionCallFromISR",
        "xTimerPendFunctionCall",
        "pcTimerGetName",
        "vTimerSetReloadMode",
        "uxTimerGetReloadMode",
        "xTimerGetPeriod",
        "xTimerGetExpiryTime",
        "vTimerSetTimerNumber",
        "uxTimerGetTimerNumber",
        "vApplicationGetTimerTaskMemory",
        // stream_buffer.h
        "xStreamBufferSend",
        "xStreamBufferSendFromISR",
        "xStreamBufferReceive",
        "xStreamBufferReceiveFromISR",
        "vStreamBufferDelete",
        "xStreamBufferIsFull",
        "xStreamBufferIsEmpty",
        "xStreamBufferReset",
        "xStreamBufferSpacesAvailable",
        "xStreamBufferBytesAvailable",
        "xStreamBufferSetTriggerLevel",
        "xStreamBufferSendCompletedFromISR",
        "xStreamBufferReceiveCompletedFromISR",
        "xStreamBufferGenericCreate",
        "xStreamBufferGenericCreateStatic",
        "xStreamBufferNextMessageLengthBytes",
        "vStreamBufferSetStreamBufferNumber",
        "uxStreamBufferGetStreamBufferNumber",
        "ucStreamBufferGetStreamBufferType",
        // semphr.h
        // queue.h
        "xQueueGenericSend",
        "xQueuePeek",
        "xQueuePeekFromISR",
        "xQueueReceive",
        "uxQueueMessagesWaiting",
        "uxQueueSpacesAvailable",
        "vQueueDelete",
        "xQueueGenericSendFromISR",
        "xQueueReceiveFromISR",
        "xQueueIsQueueEmptyFromISR",
        "xQueueIsQueueFullFromISR",
        "uxQueueMessagesWaitingFromISR",
        "xQueueCRSendFromISR",
        "xQueueCRReceiveFromISR"
        "xQueueCRSend",
        "xQueueCRReceive",
        "xQueueCreateMutex",
        "xQueueCreateMutexStatic",
        "xQueueCreateCountingSemaphore",
        "xQueueCreateCountingSemaphoreStatic",
        "xQueueSemaphoreTake",
        "xQueueGetMutexHolder",
        "xQueueGetMutexHolderFromISR",
        "xQueueTakeMutexRecursive",
        "xQueueGiveMutexRecursive",
        "xQueueGenericCreate",
        "xQueueGenericCreate"
        "xQueueGenericCreateStatic",
        "xQueueCreateSet",
        "xQueueAddToSet",
        "xQueueRemoveFromSet",
        "xQueueSelectFromSet",
        "xQueueSelectFromSetFromISR",
        "vQueueWaitForMessageRestricted",
        "xQueueGenericReset",
        "vQueueSetQueueNumber",
        "uxQueueGetQueueNumber",
        "ucQueueGetQueueType",
        // portable.h
        "pxPortInitialiseStack",
        "vPortDefineHeapRegions",
        "vPortGetHeapStats",
        "vPortStoreTaskMPUSettings",
        // list.h
        "vListInitialise",
        "vListInitialiseItem",
        "vListInsert",
        "vListInsertEnd",
        "uxListRemove",
        // event_groups.h
        "xEventGroupCreate",
        "xEventGroupCreateStatic",
        "xEventGroupWaitBits",
        "xEventGroupClearBits",
        "xEventGroupClearBitsFromISR",
        "xEventGroupSetBits",
        "xEventGroupSetBitsFromISR",
        "xEventGroupSync",
        "xEventGroupGetBitsFromISR",
        "vEventGroupDelete",
        "vEventGroupSetBitsCallback"
        "vEventGroupClearBitsCallback",
        "uxEventGroupGetNumber",
        "vEventGroupSetNumber",
    };

    bool ignoreAccess(Value *Ptr);

    class InterestingMemoryOperand {
    public:
        Use *PtrUse;
        bool IsWrite;
        Type *OpType;
        uint64_t TypeSize;
        MaybeAlign Alignment;
        // The mask Value, if we're looking at a masked load/store.
        Value *MaybeMask;

        InterestingMemoryOperand(Instruction *I, unsigned OperandNo, bool IsWrite,
                                class Type *OpType, MaybeAlign Alignment,
                                Value *MaybeMask = nullptr)
            : IsWrite(IsWrite), OpType(OpType), Alignment(Alignment),
                MaybeMask(MaybeMask) {
            const DataLayout &DL = I->getModule()->getDataLayout();
            TypeSize = DL.getTypeStoreSizeInBits(OpType);
            PtrUse = &I->getOperandUse(OperandNo);
        }

        Instruction *getInsn() { return cast<Instruction>(PtrUse->getUser()); }

        Value *getPtr() { return PtrUse->get(); }
    };

    struct IPEASan : public FunctionPass{
        static char ID;
        IPEASan() : FunctionPass(ID) {}

        std::map<Value *, Value *> mCachedId, mCachedAddressId;
        const DataLayout *DL;
        
        TargetLibraryInfo *TLI;
        int32_t mTempId = -1;
        bool hasPtrArg = false;

        int ModuleFilted(Module& module){
            StringMap<bool> white_list;
            StringMap<bool> black_list;
            std::string module_name(module.getName());
            std::string sub_module_name = module_name.substr(module_name.rfind("/") + 1);
            #if DEBUG_MODE
                errs() << "sub module name: " << sub_module_name << "\n";
            #endif
            // Only one list is enabled
            #if WHITE_LIST_OR_BLACK_LIST
                // White List
                white_list["hello_world.c"] = true;
                white_list["stm32l4xx_hal_uart_ex.c"] = true;
                if(white_list.count(sub_module_name) > 0){
                    return 0;
                }else{
                    return 1;
                }
            #else
                // Black List
                #if TARGET_CONFIG==1
                    // NXP
                    black_list["startup_MK64F12.S"] = true;
                    black_list["fsl_sbrk.c"] = true;
                #elif TARGET_CONFIG==2
                    // STM
                    black_list["system_stm32l4xx.c"] = true;
                    black_list["startup_stm32l475xx.s"] = true;
                    black_list["syscalls.c"] = true;
                #elif TARGET_CONFIG==3
                    // nRF52840
                    black_list["system_nrf52840.c"] = true;
                #elif TARGET_CONFIG==4
                    //Atmel samE54
                    black_list["startup_same54.c"] = true;
                #elif TARGET_CONFIG==5
                    // nRF52840 zephyr
                    black_list["system_nrf52840.c"] = true;
                    black_list["cbprintf.c"] = true;
                    black_list["cbprintf_packaged.c"] = true;
                    black_list["printk.c"] = true;
                    black_list["isr_tables.c"] = true;
                    black_list["dev_handles.c"] = true;
                    black_list["prep_c.c"] = true;
                    black_list["xip.c"] = true;
                    black_list["irq_init.c"] = true;
                    if(sub_module_name.find(".S") != std::string::npos){ // filter out .S files
                        return 1;
                    }
                    if(module_name.find("libc") != std::string::npos){ // filter out libc
                        return 1;
                    }
                    if(module_name.find("logging") != std::string::npos){ // filter out logging
                        return 1;
                    }
                #elif TARGET_CONFIG==6
                    black_list["system_stm32wbxx.c"] = true;
                    black_list["startup_stm32wb5mmghx.s "] = true;
                    black_list["syscalls.c"] = true;
                    black_list["dbg_trace.c"] = true;
                #elif TARGET_CONFIG == 7
                    // NXP
                    black_list["startup_MK66F18.S"] = true;
                    black_list["fsl_sbrk.c"] = true;
                #endif
                if(black_list.count(sub_module_name) > 0){
                    return 1;
                }else{
                    return 0;
                }
            #endif
        }

        // int isPeripheralAddress(uint64_t value){
        //     #if DEBUG_MODE
        //         errs() << "call isPeripheralAddress\n";
        //     #endif
        //     if((FLASH_REGION_BEGIN <= value && value <= FLASH_REGION_END) || (RAM_REGION_BEGIN <= value && value <= RAM_REGION_END)){
        //         #if DEBUG_MODE
        //             errs() << value << " is firmware address\n";
        //         #endif
        //         return 0;
        //     }else{
        //         #if DEBUG_MODE
        //             errs() << value << " is peripheral address\n";
        //         #endif
        //         return 1;
        //     }
        // }

        void addToGlobalConstructors(Module& M, Function *Fn) {
            #if DEBUG_MODE
                errs() << "call addToGlobalConstructors()\n";
            #endif
            const char *Name = "llvm.global_ctors";
            GlobalVariable *GV = M.getGlobalVariable(Name);
            if(GV){
                #if DEBUG_MODE
                errs() << "GV: ";
                GV->print(errs());
                errs() << "\n";
                #endif
            }else{
                #if DEBUG_MODE
                errs() << "No GV\n";
                #endif
            }
            std::vector<Constant *> V;
            if (GV) {
                Constant *Array = GV->getInitializer();
                for (Value *X : Array->operand_values())
                    V.push_back(cast<Constant>(X));
                GV->eraseFromParent();
            }
            StructType *ST = StructType::get(Int32Type, Fn->getType(), VoidPtrType);
            V.push_back(ConstantStruct::get(ST, ConstantInt::get(Int32Type, 65535), Fn, m_void_null_ptr));
            ArrayType *Ty = ArrayType::get(ST, V.size());
            GV = new GlobalVariable(M, Ty, true, GlobalValue::AppendingLinkage, ConstantArray::get(Ty, V), Name, nullptr, GlobalVariable::NotThreadLocal);
            #if DEBUG_MODE
                errs() << "New GV: ";
                GV->print(errs());
                errs() << "\n";
            #endif
        }

        Instruction* reBuildGlobalCtorInsertPoint(Module& M){
            #if DEBUG_MODE
                errs() << "call getGlobalCtorInsertPoint()\n";
            #endif
            std::string ctor_func_name(M.getSourceFileName());
            std::replace(ctor_func_name.begin(), ctor_func_name.end(), '/' ,'_');
            std::replace(ctor_func_name.begin(), ctor_func_name.end(), '.', '_');
            module_extend_ctor_name = ctor_func_name;
            #if DEBUG_MODE
                errs() << "ctor_func_name: " << ctor_func_name << "\n";
                errs() << "module_extend_ctor_name: " << module_extend_ctor_name << "\n";
            #endif
            Function* extend_ctor = NULL; 
            BasicBlock* extend_ctor_bb = NULL;
            extend_ctor = M.getFunction(ctor_func_name);
            if(!extend_ctor){
                #if DEBUG_MODE
                    errs() << "extend_ctor not exist\n";
                #endif
                GlobalValue::LinkageTypes Linkage = Function::WeakODRLinkage;
                FunctionType* Ty = FunctionType::get(VoidType, {}, false);
                extend_ctor = Function::Create(Ty, Linkage, ctor_func_name, M);
                extend_ctor_bb = BasicBlock::Create(M.getContext(), "extend_ctor_entry", extend_ctor);
                IRBuilder<> extend_ctor_builder(extend_ctor_bb);
                extend_ctor_builder.SetInsertPoint(extend_ctor_bb);
                extend_ctor_builder.CreateCall(cb_fuzz_init, {});
                ctor_global_insert_point = dyn_cast<Instruction>(extend_ctor_builder.CreateRetVoid());
                #if DEBUG_MODE
                    errs() << "extend_ctor: \n";
                    extend_ctor->print(errs());
                    errs() << "\nglobal insert point: ";
                    ctor_global_insert_point->print(errs());
                    errs() << "\n";
                #endif
                addToGlobalConstructors(M, extend_ctor);
            }else{
                #if DEBUG_MODE
                    errs() << "extend_ctor already exist and global insert point: \n";
                    ctor_global_insert_point->print(errs());
                    errs() << "\n";
                #endif
            }
            return ctor_global_insert_point;
        }
        bool isIgnoredFunc(const std::string &str){
            if (str.find("llvm.") == 0) {
                #if DEBUG_MODE
                    errs() << "isIgnoredFunc: llvm. func\n";
                #endif
                return true;
            }
            return false;
        }
        bool isSelfSpecialFunc(const std::string &str){
            #if DEBUG_MODE
                errs() << "Call isSelfSpecialFunc\n";
            #endif
            if(m_func_def_special.getNumItems() == 0){
                // m_func_wrappers_available["malloc"] = true;
                // m_func_wrappers_available["free"] = true;
                // m_func_wrappers_available["realloc"] = true;
                m_func_wrappers_available["calloc"] = true;
                m_func_def_special["FuzzStart"] = true;
                m_func_def_special["FuzzFinish"] = true;
                m_func_def_special["__softboundcets_global_init"] = true;
                m_func_def_special["__softboundcets_init"] = true;
                // arm-none-eabi/include/stdio.h
                m_func_def_special["printf"] = true;
                m_func_def_special["scanf"] = true;
                m_func_def_special["getc"] = true;
                m_func_def_special["gets"] = true;
                m_func_def_special["puts"] = true;
                m_func_def_special["sprintf"] = true;
                // arm-none-eabi/include/string.h
                m_func_def_special["memchr"] = true;
                m_func_def_special["memcmp"] = true;
                m_func_def_special["strcat"] = true;
                m_func_def_special["strchr"] = true;
                m_func_def_special["strcmp"] = true;
                m_func_def_special["strcoll"] = true;
                m_func_def_special["strcpy"] = true;
                m_func_def_special["strcspn"] = true;
                m_func_def_special["strerror"] = true;
                m_func_def_special["strlen"] = true;
                m_func_def_special["strncat"] = true;
                m_func_def_special["strncmp"] = true;
                m_func_def_special["strncpy"] = true;
                m_func_def_special["strpbrk"] = true;
                m_func_def_special["strrchr"] = true;
                m_func_def_special["strspn"] = true;
                m_func_def_special["strstr"] = true;
                #if TARGET_CONFIG==1
                // nxp sdk
                m_func_def_special["StrFormatPrintf"] = true;
                m_func_def_special["StrFormatScanf"] = true;
                m_func_def_special["DbgConsole_PrintfFormattedData"] = true;
                m_func_def_special["DbgConsole_ScanfFormattedData"] = true;
                m_func_def_special["DbgConsole_Printf"] = true;
                m_func_def_special["DbgConsole_Putchar"] = true;
                m_func_def_special["DbgConsole_Scanf"] = true;
                m_func_def_special["DbgConsole_Getchar"] = true;
                m_func_def_special["DbgConsole_PrintfPaddingCharacter"] = true;
                m_func_def_special["DbgConsole_BlockingPrintf"] = true;
                m_func_def_special["SystemCoreClockUpdate"] = true;
                m_func_def_special["SystemInit"] = true;
                m_func_def_special["SystemInitHook"] = true;
                #elif TARGET_CONFIG==2
                // stm sdk
                m_func_def_special["SPI_WIFI_DelayUs"] = true;
                m_func_def_special["SPI_WIFI_Delay"] = true;
                #elif TARGET_CONFIG==4
                m_func_def_special["mschf_is_enabled"] = true;
                #elif TARGET_CONFIG==5
                // nRF52840 zephyr
                m_func_def_special["cbprintf"] = true;
                m_func_def_special["str_out"] = true;
                m_func_def_special["fprintfcb"] = true;
                m_func_def_special["vfprintfcb"] = true;
                m_func_def_special["printfcb"] = true;
                m_func_def_special["vprintfcb"] = true;
                m_func_def_special["snprintfcb"] = true;
                m_func_def_special["vsnprintfcb"] = true;
                m_func_def_special["cbprintf_via_va_list"] = true;
                m_func_def_special["arch_printk_char_out"] = true;
                m_func_def_special["__printk_hook_install"] = true;
                m_func_def_special["__printk_get_hook"] = true;
                m_func_def_special["buf_flush"] = true;
                m_func_def_special["buf_char_out"] = true;
                m_func_def_special["char_out"] = true;
                m_func_def_special["vprintk"] = true;
                m_func_def_special["printk"] = true;
                m_func_def_special["z_impl_k_str_out"] = true;
                m_func_def_special["z_vrfy_k_str_out"] = true;
                m_func_def_special["str_out"] = true;
                m_func_def_special["snprintk"] = true;
                m_func_def_special["vsnprintk"] = true;
                m_func_def_special["cbvprintf"] = true;
                m_func_def_special["z_fdtable_call_ioctl"] = true;
                m_func_def_special["z_log_printk"] = true;
                m_func_def_special["log_generic"] = true;
                m_func_def_special["log_string_sync"] = true;
                m_func_def_special["log_from_user"] = true;
                m_func_def_special["z_bss_zero"] = true;
                m_func_def_special["log_0"] = true;
                m_func_def_special["log_1"] = true;
                m_func_def_special["log_2"] = true;
                m_func_def_special["log_3"] = true;
                #elif TARGET_CONFIG==6
                m_func_def_special["DbgTraceBuffer"] = true;
                m_func_def_special["__write"] = true;
                m_func_def_special["DbgTraceGetFileName"] = true;
                m_func_def_special["DbgTrace_TxCpltCallback"] = true;
                m_func_def_special["DbgTraceInit"] = true;
                m_func_def_special["_write"] = true;
                m_func_def_special["DbgTraceWrite"] = true;
                m_func_def_special["fputc"] = true;
                m_func_def_special["SysTick_Handler"] = true;
                m_func_def_special["HAL_IncTick"] = true;
                #endif
                // arm-none-eabi/include/alloca.h
                m_alert_funcs["alloca"] = true;
                // arm-none-eabi/include/argz.h
                m_alert_funcs["argz_create"] = true;
                m_alert_funcs["argz_create_sep"] = true;
                m_alert_funcs["argz_count"] = true;
                m_alert_funcs["argz_extract"] = true;
                m_alert_funcs["argz_stringify"] = true;
                m_alert_funcs["argz_add"] = true;
                m_alert_funcs["argz_add_sep"] = true;
                m_alert_funcs["argz_append"] = true;
                m_alert_funcs["argz_delete"] = true;
                m_alert_funcs["argz_insert"] = true;
                m_alert_funcs["argz_next"] = true;
                m_alert_funcs["argz_replace"] = true;
                // arm-none-eabi/include/devctl.h
                m_alert_funcs["posix_devctl"] = true;
                // arm-none-eabi/include/dirent.h
                m_alert_funcs["alphasort"] = true;
                m_alert_funcs["dirfd"] = true;
                m_alert_funcs["fdclosedir"] = true;
                m_alert_funcs["opendir"] = true;
                m_alert_funcs["fdopendir"] = true;
                m_alert_funcs["readdir"] = true;
                m_alert_funcs["readdir_r"] = true;
                m_alert_funcs["rewinddir"] = true;
                m_alert_funcs["scandir"] = true;
                m_alert_funcs["_seekdir"] = true;
                m_alert_funcs["seekdir"] = true;
                m_alert_funcs["telldir"] = true;
                m_alert_funcs["closedir"] = true;
                m_alert_funcs["scandirat"] = true;
                m_alert_funcs["versionsort"] = true;
                // arm-none-eabi/include/envlock.h
                m_alert_funcs["__env_lock"] = true;
                m_alert_funcs["__env_unlock"] = true;
                // arm-none-eabi/include/envz.h
                m_alert_funcs["envz_entry"] = true;
                m_alert_funcs["envz_get"] = true;
                m_alert_funcs["envz_add"] = true;
                m_alert_funcs["envz_merge"] = true;
                m_alert_funcs["envz_remove"] = true;
                m_alert_funcs["envz_strip"] = true;
                // arm-none-eabi/include/fenv.h
                m_alert_funcs["fegetexceptflag"] = true;
                m_alert_funcs["fesetexceptflag"] = true;
                m_alert_funcs["fegetenv"] = true;
                m_alert_funcs["feholdexcept"] = true;
                m_alert_funcs["fesetenv"] = true;
                m_alert_funcs["feupdateenv"] = true;
                // arm-none-eabi/include/fnmatch.h
                m_alert_funcs["fnmatch"] = true;
                // arm-none-eabi/include/getopt.h
                m_alert_funcs["getopt"] = true;
                m_alert_funcs["getopt_long"] = true;
                m_alert_funcs["getopt_long_only"] = true;
                m_alert_funcs["__getopt_r"] = true;
                m_alert_funcs["__getopt_long_r"] = true;
                m_alert_funcs["__getopt_long_only_r"] = true;
                // arm-none-eabi/include/glob.h
                m_alert_funcs["glob"] = true;
                m_alert_funcs["globfree"] = true;
                // arm-none-eabi/include/grp.h
                m_alert_funcs["getgrgid"] = true;
                m_alert_funcs["getgrnam"] = true;
                m_alert_funcs["getgrnam_r"] = true;
                m_alert_funcs["getgrgid_r"] = true;
                m_alert_funcs["getgrent"] = true;
                m_alert_funcs["initgroups"] = true;
                // arm-none-eabi/include/iconv.h
                m_alert_funcs["iconv_open"] = true;
                m_alert_funcs["iconv"] = true;
                m_alert_funcs["iconv_close"] = true;
                m_alert_funcs["_iconv_open_r"] = true;
                m_alert_funcs["_iconv_r"] = true;
                m_alert_funcs["_iconv_close_r"] = true;
                // arm-none-eabi/include/inttypes.h
                m_alert_funcs["strtoimax"] = true;
                m_alert_funcs["_strtoimax_r"] = true;
                m_alert_funcs["strtoumax"] = true;
                m_alert_funcs["_strtoumax_r"] = true;
                m_alert_funcs["wcstoimax"] = true;
                m_alert_funcs["_wcstoimax_r"] = true;
                m_alert_funcs["wcstoumax"] = true;
                m_alert_funcs["_wcstoumax_r"] = true;
                m_alert_funcs["strtoimax_l"] = true;
                m_alert_funcs["strtoumax_l"] = true;
                m_alert_funcs["wcstoimax_l"] = true;
                m_alert_funcs["wcstoumax_l"] = true;
                // arm-none-eabi/include/langinfo.h
                m_alert_funcs["nl_langinfo"] = true;
                m_alert_funcs["nl_langinfo_l"] = true;
                // arm-none-eabi/include/libgen.h
                m_alert_funcs["basename"] = true;
                m_alert_funcs["dirname"] = true;
                // arm-none-eabi/include/locale.h
                m_alert_funcs["_setlocale_r"] = true;
                m_alert_funcs["_localeconv_r"] = true;
                m_alert_funcs["_newlocale_r"] = true;
                m_alert_funcs["_freelocale_r"] = true;
                m_alert_funcs["_duplocale_r"] = true;
                m_alert_funcs["_uselocale_r"] = true;
                m_alert_funcs["setlocale"] = true;
                m_alert_funcs["localeconv"] = true;
                m_alert_funcs["newlocale"] = true;
                // arm-none-eabi/include/malloc.h
                m_alert_funcs["memalign"] = true;
                m_alert_funcs["malloc_usable_size"] = true;
                m_alert_funcs["valloc"] = true;
                m_alert_funcs["pvalloc"] = true;
                m_alert_funcs["__malloc_lock"] = true;
                m_alert_funcs["__malloc_unlock"] = true;
                m_alert_funcs["mstats"] = true;
                m_alert_funcs["cfree"] = true;
                // arm-none-eabi/include/math.h
                m_alert_funcs["frexp"] = true;
                m_alert_funcs["modf"] = true;
                m_alert_funcs["nan"] = true;
                m_alert_funcs["remquo"] = true;
                m_alert_funcs["frexpf"] = true;
                m_alert_funcs["modff"] = true;
                m_alert_funcs["remquof"] = true;
                m_alert_funcs["nanf"] = true;
                m_alert_funcs["frexpl"] = true;
                m_alert_funcs["modfl"] = true;
                m_alert_funcs["nanl"] = true;
                m_alert_funcs["remquol"] = true;
                m_alert_funcs["gamma_r"] = true;
                m_alert_funcs["lgamma_r"] = true;
                m_alert_funcs["gammaf_r"] = true;
                m_alert_funcs["lgammaf_r"] = true;
                m_alert_funcs["sincos"] = true;
                m_alert_funcs["sincosf"] = true;
                m_alert_funcs["sincosl"] = true;
                m_alert_funcs["__signgam"] = true;
                // arm-none-eabi/include/ndbm.h
                m_alert_funcs["dbm_clearerr"] = true;
                m_alert_funcs["dbm_close"] = true;
                m_alert_funcs["dbm_delete"] = true;
                m_alert_funcs["dbm_error"] = true;
                m_alert_funcs["dbm_fetch"] = true;
                m_alert_funcs["dbm_firstkey"] = true;
                m_alert_funcs["dbm_nextkey"] = true;
                m_alert_funcs["dbm_open"] = true;
                m_alert_funcs["dbm_store"] = true;
                m_alert_funcs["dbm_dirfno"] = true;
                // arm-none-eabi/include/pthread.h
                // arm-none-eabi/include/pwd.h
                // arm-none-eabi/include/reent.h
                // arm-none-eabi/include/regex.h
                // arm-none-eabi/include/sched.h
                // arm-none-eabi/include/search.h
                // arm-none-eabi/include/signal.h
                // arm-none-eabi/include/spawn.h
                // arm-none-eabi/include/stdatomic.h
                // arm-none-eabi/include/stdio_ext.h
                // arm-none-eabi/include/stdio.h
                // arm-none-eabi/include/stdlib.h
                m_alert_funcs["atoi"] = true;
                m_alert_funcs["atol"] = true;
                m_alert_funcs["atoll"] = true;
                // arm-none-eabi/include/string.h
                // arm-none-eabi/include/strings.h
                // arm-none-eabi/include/threads.h
                // arm-none-eabi/include/time.h
                m_alert_funcs["time"] = true;
                // arm-none-eabi/include/wchar.h
                // arm-none-eabi/include/wctype.h
                // arm-none-eabi/include/wordexp.h
            }
            if(m_alert_funcs.count(str) > 0){
                return true;
            }
            if(m_func_def_special.count(str) > 0){
                #if DEBUG_MODE
                    errs() << "in m_func_def_special\n";
                #endif
                return true;
            }
            if (str.find("isoc99") != std::string::npos){
                #if DEBUG_MODE
                    errs() << "isoc99 func\n";
                #endif
                return true;
            }
            if (str.find("llvm.") == 0) {
                #if DEBUG_MODE
                    errs() << "llvm. func\n";
                #endif
                return true;
            }
            return false;
        }
        std::string transformFunctionName(const std::string &str) {
            return "softboundcets_" + str; 
        }
        int renameFunctionName(Function* func, Module& module, bool external){
            #if DEBUG_MODE
                errs() << "call renameFunctionName()\n";
            #endif
            Type* ret_type = func->getReturnType();
            const FunctionType* fty = func->getFunctionType();
            std::vector<Type*> params;
            SmallVector<AttributeSet, 8> param_attrs_vec;
            int arg_index = 1;
            if(!m_func_wrappers_available.count(func->getName().str())){
                #if DEBUG_MODE
                    errs() << "no wrapper\n";
                #endif
                return 0;
            }
            for(Function::arg_iterator i = func->arg_begin(), e = func->arg_end(); i != e; ++i, arg_index++) {
                params.push_back(i->getType());
            }
            FunctionType* nfty = FunctionType::get(ret_type, params, fty->isVarArg());
            Function* new_func = Function::Create(nfty, func->getLinkage(), transformFunctionName(func->getName().str()));
            new_func->copyAttributesFrom(func);
            // new_func->setAttributes(AttributeSet::get(func->getContext(), param_attrs_vec)); // 这个没什么作用啊
            func->getParent()->getFunctionList().insert(func->getIterator(), new_func);
            if(!external) {
                SmallVector<Value*, 16> call_args;      
                new_func->getBasicBlockList().splice(new_func->begin(), func->getBasicBlockList());      
                Function::arg_iterator arg_i2 = new_func->arg_begin();      
                for(Function::arg_iterator arg_i = func->arg_begin(), arg_e = func->arg_end(); arg_i != arg_e; ++arg_i) {  
                    arg_i->replaceAllUsesWith(&*arg_i2);
                    arg_i2->takeName(&*arg_i);        
                    ++arg_i2;
                    arg_index++;
                }
            }
            func->replaceAllUsesWith(new_func);                            
            func->eraseFromParent();
            return 1;
        }
        void renameFunctions(Module& module){
            #if DEBUG_MODE
                errs() << "call renameFunctions\n";
            #endif
            #if TARGET_CONFIG==5
                std::string module_name(module.getName());
                if(module_name.find("libc/minimal") != std::string::npos){
                    #if DEBUG_MODE
                        errs() << "minimal libc\n";
                    #endif
                    return;
                }else{
                    #if DEBUG_MODE
                        errs() << "not minimal libc\n";
                    #endif
                }
            #endif
            bool change = false;
            do{
                change = false;
                #if DEBUG_MODE
                    errs() << "iterate functions\n";
                #endif
                for(Module::iterator ff_begin = module.begin(), ff_end = module.end(); ff_begin != ff_end; ++ff_begin){
                    Function* func_ptr = dyn_cast<Function>(ff_begin);
                    #if DEBUG_MODE
                        errs() << "try " << func_ptr->getName().str() << "\n";
                    #endif
                    if(m_func_transformed.count(func_ptr->getName().str()) || isSelfSpecialFunc(func_ptr->getName().str())){
                        #if DEBUG_MODE
                            errs() << "transformed or isSelfSpecialFunc\n";
                        #endif
                        continue;
                    }
                    m_func_transformed[func_ptr->getName().str()] = true;
                    m_func_transformed[transformFunctionName(func_ptr->getName().str())] = true;
                    bool is_external = func_ptr->isDeclaration();
                    if(renameFunctionName(func_ptr, module, is_external) == 1){
                        change = true;
                        break;
                    }
                }
            }while(change);
        }
        void getPHIBaseBound(PHINode* phi_node, PHINode* & phi_node_base, PHINode* & phi_node_bound){
            if(m_phi_base.count(phi_node) > 0){
                phi_node_base = m_phi_base[phi_node];
                phi_node_bound = m_phi_bound[phi_node];  
            }else{
                assert(!"PHINode metadata not presented");
            }
        }
        
        // Done
        bool checkIfFunctionOfInterest(Function* func){
            // if(func->isDeclaration()){
            //     #if DEBUG_MODE
            //         errs() << "This is a declaration\n";
            //     #endif
            //     return false;
            // }else if(func->getName().str() == module_extend_ctor_name){
            //     #if DEBUG_MODE
            //         errs() << "Instrumented global extend ctor\n";
            //     #endif
            //     return false;
            // }else if(isSelfSpecialFunc(func->getName().str())){
            //     #if DEBUG_MODE
            //         errs() << "This is a softbound defined func\n";
            //     #endif
            //     return false;
            // }else if (noSanitize.find(func) != noSanitize.end()) {
            //     return false;
            // }
            // return true;
            return !(func->isDeclaration() 
                        || func->getName().str() == module_extend_ctor_name 
                        || isSelfSpecialFunc(func->getName().str()) 
                        || func->hasFnAttribute("no_instrument")
                        || func->hasFnAttribute("malloc")
                        || func->hasFnAttribute("free")
                        || func->hasFnAttribute("syscall"));
        }

        void instrumentCheck(Value *pointer, Instruction *where)
        {
            assert(isa<LoadInst>(where) || isa<StoreInst>(where));

            IRBuilder<> IRB(where);
            // Value *addr;
            
            // if (mCachedAddressId.find(pointer) != mCachedAddressId.end()) {
            //     addr = mCachedAddressId[pointer];
            // } else {
            //     addr = IRB.CreatePointerCast(pointer, Int32Type);
            // }

            auto value = isa<StoreInst>(where) ? cast<StoreInst>(where)->getValueOperand() : where;
            auto addr = emitPointerAddress(pointer);
            auto size = ConstantInt::get(Int32Type, DL->getTypeAllocSize(value->getType()));
            auto ptr_id = getPointerIDObject(pointer);

            assert(ptr_id);

            NumInstrumentedCheck++;

            IRB.CreateCall(cb_load_store_check, {ptr_id, addr, size});
        }


        void addLoadStoreChecks(Instruction* load_store){
            #if DEBUG_MODE
                errs() << "call addLoadStoreChecks()\n";
                load_store->print(errs());
                errs() << "\n";
            #endif

            auto ptr = load_store->getOperand(isa<LoadInst>(load_store) ? 0 : 1);
            instrumentCheck(ptr, load_store);    
        }

        void getInterestingMemoryOperands(
                Instruction *I, SmallVectorImpl<InterestingMemoryOperand> &Interesting) {
            // Skip memory accesses inserted by another instrumentation.
            if (I->hasMetadata("nosanitize"))
                return;

            if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
                if (ignoreAccess(LI->getPointerOperand()))
                    return;
                #if DEBUG_MODE
                    errs() << "Interesting operands: ";
                    LI->getPointerOperand()->print(errs());
                    errs() << "\n";
                #endif
                Interesting.emplace_back(I, LI->getPointerOperandIndex(), false,
                                        LI->getType(), LI->getAlign());
            } else if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
                if (ignoreAccess(SI->getPointerOperand()))
                    return;
                #if DEBUG_MODE
                    errs() << "Interesting operands: ";
                    SI->getPointerOperand()->print(errs());
                    errs() << "\n";
                #endif
                Interesting.emplace_back(I, SI->getPointerOperandIndex(), true,
                                        SI->getValueOperand()->getType(), SI->getAlign());
            } else if (AtomicRMWInst *RMW = dyn_cast<AtomicRMWInst>(I)) {
                if (ignoreAccess(RMW->getPointerOperand()))
                    return;
                Interesting.emplace_back(I, RMW->getPointerOperandIndex(), true,
                                        RMW->getValOperand()->getType(), None);
            } else if (AtomicCmpXchgInst *XCHG = dyn_cast<AtomicCmpXchgInst>(I)) {
                if (ignoreAccess(XCHG->getPointerOperand()))
                    return;
                Interesting.emplace_back(I, XCHG->getPointerOperandIndex(), true,
                                        XCHG->getCompareOperand()->getType(), None);
            } else if (auto CI = dyn_cast<CallInst>(I)) {
                auto *F = CI->getCalledFunction();
                if (F && (F->getName().startswith("llvm.masked.load.") ||
                        F->getName().startswith("llvm.masked.store."))) {
                bool IsWrite = F->getName().startswith("llvm.masked.store.");
                // Masked store has an initial operand for the value.
                unsigned OpOffset = IsWrite ? 1 : 0;
                auto BasePtr = CI->getOperand(OpOffset);
                if (ignoreAccess(BasePtr))
                    return;
                auto Ty = cast<PointerType>(BasePtr->getType())->getElementType();
                MaybeAlign Alignment = Align(1);
                // Otherwise no alignment guarantees. We probably got Undef.
                if (auto *Op = dyn_cast<ConstantInt>(CI->getOperand(1 + OpOffset)))
                    Alignment = Op->getMaybeAlignValue();
                    Value *Mask = CI->getOperand(2 + OpOffset);
                    Interesting.emplace_back(I, OpOffset, IsWrite, Ty, Alignment, Mask);
                } else {
                    for (unsigned ArgNo = 0; ArgNo < CI->getNumArgOperands(); ArgNo++) {
                        if (!CI->isByValArgument(ArgNo) ||
                            ignoreAccess(CI->getArgOperand(ArgNo)))
                        continue;
                        Type *Ty = CI->getParamByValType(ArgNo);
                        Interesting.emplace_back(I, ArgNo, false, Ty, Align(1));
                    }
                }
            }
        }

        bool GlobalIsLinkerInitialized(GlobalVariable *G) {
            // If a global variable does not have dynamic initialization we don't
            // have to instrument it.  However, if a global does not have initializer
            // at all, we assume it has dynamic initializer (in other TU).
            //
            // FIXME: Metadata should be attched directly to the global directly instead
            // of being added to llvm.asan.globals.
            return G->hasInitializer();
        }

        // isSafeAccess returns true if Addr is always inbounds with respect to its
        // base object. For example, it is a field access or an array access with
        // constant inbounds index.
        bool isSafeAccess(ObjectSizeOffsetVisitor &ObjSizeVis,
                                            Value *Addr, uint64_t TypeSize) const {
            #if DEBUG_MODE
                errs() << "calling isSafeAccess: ";
                Addr->print(errs());
                errs() << "\n";
            #endif
            SizeOffsetType SizeOffset = ObjSizeVis.compute(Addr);
            if (!ObjSizeVis.bothKnown(SizeOffset)) return false;
            uint64_t Size = SizeOffset.first.getZExtValue();
            int64_t Offset = SizeOffset.second.getSExtValue();
            // Three checks are required to ensure safety:
            // . Offset >= 0  (since the offset is given from the base ptr)
            // . Size >= Offset  (unsigned)
            // . Size - Offset >= NeededSize  (unsigned)
            #if DEBUG_MODE
                errs() << "Offset: " << Offset << ", Size: " << Size << "\n";
            #endif
            return Offset >= 0 && Size >= uint64_t(Offset) &&
                    Size - uint64_t(Offset) >= TypeSize / 8;
        }


        void addDereferenceChecks(Function* func, std::vector<Instruction *> &insts, std::deque<std::function<void()>> &task_queue){
            #if DEBUG_MODE
                errs() << "Call addDereferenceChecks()\n";
            #endif
            if(func->isVarArg()){
                #if DEBUG_MODE
                    errs() << "Ignored: function '" << func->getName() << "' takes a variable number of arguments\n";
                #endif
                return;
            }
            
            SmallVector<InterestingMemoryOperand, 16> OperandsToInstrument;
            SmallVector<MemIntrinsic *, 16> IntrinToInstrument;

            for (auto I : insts) {
                if (!isa<LoadInst>(I) && !isa<StoreInst>(I) && !isa<MemIntrinsic>(I)) {
                    continue;
                }

                SmallVector<InterestingMemoryOperand, 1> InterestingOperands;
                getInterestingMemoryOperands(I, InterestingOperands);
                
                if (!InterestingOperands.empty()) {
                    for (auto &Operand : InterestingOperands) {
                        OperandsToInstrument.push_back(Operand);
                    }
                } else if (MemIntrinsic *MI = dyn_cast<MemIntrinsic>(I)) {
                    IntrinToInstrument.push_back(MI);
                    #if DEBUG_MODE
                        errs() << "Memory intrinsic: ";
                        MI->print(errs());
                        errs() << "\n";
                    #endif
                } else {
                    // To Do Nothing
                }

            }

            ObjectSizeOpts ObjSizeOpts;
            ObjSizeOpts.RoundToAlign = true;
            ObjectSizeOffsetVisitor ObjSizeVis(*DL, TLI, func->getContext(), ObjSizeOpts);

            for (auto &Operand : OperandsToInstrument) {
                Value *Addr = Operand.getPtr();
                Value *UO = getUnderlyingObject(Addr);
                #if DEBUG_MODE
                    errs() << "handlilng pointer operand: ";
                    Addr->print(errs());
                    errs() << "\n";
                #endif
                if (isa<GlobalVariable>(UO)) {
                    auto *G = dyn_cast<GlobalVariable>(UO);
                    auto name = G->getName();
                    if ((GlobalIsLinkerInitialized(G) && isSafeAccess(ObjSizeVis, Addr, Operand.TypeSize))) {
                        #if DEBUG_MODE
                            errs() << "ignore initialized variable: ";
                            G->print(errs());
                            errs() << "\n";
                        #endif 
                        continue;
                    }
                } else if (isa<AllocaInst>(UO) 
                        && isSafeAccess(ObjSizeVis, Addr, Operand.TypeSize)) {
                    #if DEBUG_MODE
                        auto *AI = dyn_cast<AllocaInst>(UO);
                        errs() << "ingore alloca: ";
                        AI->print(errs());
                        errs() << "\n";
                    #endif
                    continue;
                }

                #if DEBUG_MODE
                    errs() << "adding load/store check: ";
                    Operand.getInsn()->print(errs());
                    errs() << "\n";
                #endif

                task_queue.push_back(std::bind(&IPEASan::addLoadStoreChecks, this, Operand.getInsn()));

                // addLoadStoreChecks(Operand.getInsn());
            }
        }
        void extendLifeTime(Function* func_ptr){
            #if DEBUG_MODE
                errs() << "Call extendLifeTime\n";
            #endif
            for(Function::iterator bb_begin = func_ptr->begin(), bb_end = func_ptr->end(); bb_begin != bb_end; ++bb_begin) {
                for(BasicBlock::iterator i_begin = bb_begin->begin(), i_end = bb_begin->end(); i_begin != i_end; ++i_begin){
                    CallInst* call_inst = dyn_cast<CallInst>(i_begin);
                    if(call_inst && 
                        call_inst->getCalledFunction() && 
                        call_inst->getCalledFunction()->getName().contains(StringRef("llvm.lifetime"))){
                        #if DEBUG_MODE
                            errs() << "lifetime erase:";
                            call_inst->print(errs());
                            errs() << "\n";
                        #endif
                        i_begin = call_inst->eraseFromParent();
                    }
                }
            }
        }
        
        void addMemcopyMemsetCheck(CallInst* call_inst, Function* called_func){
            #if DEBUG_MODE
                errs() << "Call addMemcopyMemsetCheck()\n";
            #endif
            IRBuilder<> builder(call_inst);
            builder.SetInsertPoint(call_inst);
            if(called_func->getName().equals(StringRef("memcpy")) || 
               called_func->getName().equals(StringRef("memmove")) ||
               called_func->getName().equals(StringRef("wmemcpy")) ||
               called_func->getName().equals(StringRef("wmemmove"))){
                Value* size = call_inst->getArgOperand(2);
                Value* src = call_inst->getArgOperand(1);
                 #if DEBUG_MODE
                    errs() << "memcpy or memmove\n";
                    errs() << "size: ";
                    size->print(errs());
                    errs() << "\nsrc: ";
                    src->print(errs());
                    errs() << "\n";
                #endif
                if (auto src_id = getPointerIDObject(src)) {
                    auto src_addr = builder.CreatePtrToInt(src, Int32Type, "src.addr");
                    // XXX: To be revisied
                    NumInstrumentedCheck++;
                    builder.CreateCall(cb_load_store_check, {src_id, src_addr, size});
                }
                Value* tar = call_inst->getArgOperand(0);
                #if DEBUG_MODE
                    errs() << "tar: ";
                    tar->print(errs());
                    errs() << "\n";
                #endif
                if (auto tar_id = getPointerIDObject(tar)) {
                    auto dst_addr = builder.CreatePtrToInt(tar, Int32Type, "dst.addr");
                    // XXX:
                    NumInstrumentedCheck++;
                    builder.CreateCall(cb_load_store_check, {tar_id, dst_addr, size});
                }
                return;
            }
            if(called_func->getName().equals(StringRef("memset")) ||
                called_func->getName().equals(StringRef("wmemset"))){
                Value* size = call_inst->getArgOperand(2);
                Value* tar = call_inst->getArgOperand(0);
                #if DEBUG_MODE
                    errs() << "handle memset\n";
                    errs() << "size: ";
                    size->print(errs());
                    errs() << "\ntar: ";
                    tar->print(errs());
                    errs() << "\n";
                #endif
                if (auto tar_id = getPointerIDObject(tar)) {
                    if(!size->getType()->isIntegerTy(32)){
                        size = builder.CreateIntCast(size, Int32Type, true);
                    }
                    auto tar_addr = builder.CreatePtrToInt(tar, Int32Type, "dst.addr");
                    // XXX:
                    NumInstrumentedCheck++;
                    builder.CreateCall(cb_load_store_check, {tar_id, tar_addr, size});
                }
                return;
            }
            #if DEBUG_MODE
                errs() << "Shouldn't reach here: Not valid special mem func?\n";
            #endif
        }

        void addStringCheck(CallInst *call_inst)
        {
            IRBuilder<> IRB(call_inst);
            FunctionCallee F;
            Value *newCI;
            auto M = call_inst->getModule();
            auto numOperands = call_inst->getNumArgOperands();
            auto Int8PtrType = Type::getInt8PtrTy(M->getContext());
            auto wCharPtrType = Type::getInt32PtrTy(M->getContext());

            assert(numOperands == 2 || numOperands == 3);

            auto wrapperName = "__usan_" + call_inst->getCalledFunction()->getName();
            auto destId = getPointerIDObject(call_inst->getArgOperand(0));
            auto srcId = getPointerIDObject(call_inst->getArgOperand(1));

            assert(destId != nullptr && srcId != nullptr);

            if (numOperands == 2) {
                // strcpy or strcat
                if (wrapperName.str() == "__usan_strcpy" || wrapperName.str() == "__usan_strcat") {
                    F = M->getOrInsertFunction(wrapperName.str(), 
                                FunctionType::get(Int8PtrType, {Int8PtrType, Int8PtrType, Int32Type, Int32Type}, false));
                } else {
                    assert(wrapperName.str() == "__usan_wcscpy" || wrapperName.str() == "__usan_wcscat");
                    F = M->getOrInsertFunction(wrapperName.str(), 
                                FunctionType::get(wCharPtrType, {wCharPtrType, wCharPtrType, Int32Type, Int32Type}, false));
                }
                newCI = IRB.CreateCall(F, {call_inst->getArgOperand(0), call_inst->getArgOperand(1), destId, srcId}); 
            } else {
                // strncpy or strncat
                if (wrapperName.str() == "__usan_strncpy" || wrapperName.str() == "__usan_strncat") {
                    F = M->getOrInsertFunction(wrapperName.str(), 
                                FunctionType::get(Int8PtrType, {Int8PtrType, Int8PtrType, Int32Type, Int32Type, Int32Type}, false));
                } else {
                    assert(wrapperName.str() == "__usan_wcsncpy" || wrapperName.str() == "__usan_wcsncat");
                    F = M->getOrInsertFunction(wrapperName.str(), 
                                FunctionType::get(wCharPtrType, {wCharPtrType, wCharPtrType, Int32Type, Int32Type, Int32Type}, false));
                }
                newCI = IRB.CreateCall(F, {call_inst->getArgOperand(0), call_inst->getArgOperand(1), call_inst->getArgOperand(2), destId, srcId});
            }

            call_inst->replaceAllUsesWith(newCI);
            call_inst->eraseFromParent();
        }


        void handleStrlen(CallInst *call_inst)
        {
            IRBuilder<> IRB(call_inst);
            IRB.SetInsertPoint(call_inst->getNextNonDebugInstruction());

            auto ptrId = getPointerIDObject(call_inst->getArgOperand(0));
            assert(ptrId != nullptr);
            auto addr = IRB.CreatePointerCast(call_inst->getArgOperand(0), Int32Type);
            NumInstrumentedCheck++;
            IRB.CreateCall(cb_load_store_check, {ptrId, addr, call_inst});
        }

        void handleSprintf(CallInst *call_inst)
        {
            IRBuilder<> IRB(call_inst);
            IRB.SetInsertPoint(call_inst->getNextNonDebugInstruction());

            auto srcId = getPointerIDObject(call_inst->getArgOperand(0));
            assert(srcId != nullptr);

            auto addr = IRB.CreatePointerCast(call_inst->getArgOperand(0), Int32Type);
            NumInstrumentedCheck++;
            IRB.CreateCall(cb_load_store_check, {srcId, addr, call_inst});
        }


        void handleMalloc(CallInst* call_inst){
            #if DEBUG_MODE
                errs() << "call handleMalloc\n";
            #endif
            Value* size = call_inst->getArgOperand(0);
            // ConstantInt* ci = dyn_cast<ConstantInt>(size);
            IRBuilder<> builder(call_inst->getNextNonDebugInstruction());
            builder.SetInsertPoint(call_inst->getNextNonDebugInstruction());

            auto malloc_addr = builder.CreatePtrToInt(call_inst, Int32Type, "malloc.addr");
            auto id = getPointerIDObject(call_inst);
            assert(id);
            builder.CreateCall(cb_malloc, {id, malloc_addr, size});
        }

        void handleFree(CallInst* call_inst){
            #if DEBUG_MODE
                errs() << "call handleFree\n";
            #endif
            Value* addr = call_inst->getArgOperand(0);
            IRBuilder<> builder(call_inst->getNextNonDebugInstruction());
            builder.SetInsertPoint(call_inst->getNextNonDebugInstruction());
            auto free_addr = builder.CreatePtrToInt(addr, Int32Type, "free.addr");
            builder.CreateCall(cb_free, {free_addr});
        }

        void handleRealloc(CallInst* call_inst){
            #if DEBUG_MODE
                errs() << "call handleRealloc\n";
            #endif
            IRBuilder<> builder(call_inst->getNextNonDebugInstruction());
            builder.SetInsertPoint(call_inst->getNextNonDebugInstruction());
            
            auto old_addr = builder.CreatePtrToInt(call_inst->getArgOperand(0), Int32Type);
            auto new_addr = builder.CreatePtrToInt(call_inst, Int32Type);
            auto id = getPointerIDObject(call_inst);
            assert(id);
            
            builder.CreateCall(cb_realloc, {id, new_addr, old_addr, call_inst->getArgOperand(1)});
        }

        void handleCalloc(CallInst* call_inst){
            #if DEBUG_MODE
                errs() << "call handleCalloc\n";
            #endif
            IRBuilder<> IRB(call_inst->getNextNonDebugInstruction());
            auto size = IRB.CreateMul(call_inst->getArgOperand(0), call_inst->getArgOperand(1), "calloc.size");
            auto malloc_addr = IRB.CreatePtrToInt(call_inst, Int32Type, "calloc.addr");
            auto id = getPointerIDObject(call_inst);
            assert(id);
            IRB.CreateCall(cb_malloc, {id, malloc_addr, size});
        }

        void handleCreateNewThread(CallInst *call_inst) {
            IRBuilder<> IRB(call_inst);
            Function *F = call_inst->getCalledFunction();
            Module *M = call_inst->getModule();

            Value *forthParam = call_inst->getArgOperand(3);  // pvParameters
            if (!forthParam || !forthParam->getType()->isPointerTy()) {
                return;
            }
            auto taskParamId = getPointerIDObject(forthParam);
            assert(taskParamId);

            if (isa<ConstantInt>(taskParamId) && cast<ConstantInt>(taskParamId)->isZeroValue())
                return;

            auto FC = M->getOrInsertFunction("__usan_trace_new_thread", FunctionType::get(VoidType, Int32Type, false));
            IRB.CreateCall(FC, taskParamId);
        }

        // Done
        void handleCall(CallInst* call_inst){
            #if DEBUG_MODE
                errs() << "call handleCall()\n";
            #endif
            if(call_inst->isInlineAsm()){
                #if DEBUG_MODE
                    errs() << "ignore: is inline asm\n";
                #endif
                return;
            }
            Function* func = call_inst->getCalledFunction();
            if(!func){
                #if DEBUG_MODE
                    errs() << "null func: ";
                    call_inst->getCalledOperand()->print(errs());
                    errs() << "\n";
                #endif
                func = dyn_cast<Function>(call_inst->getCalledOperand()->stripPointerCasts());
                #if DEBUG_MODE
                    if(func){
                        errs() << "func: ";
                        func->print(errs());
                        errs() << "\n";
                        if(func->getName() != ""){
                            errs() << "func_name:" << func->getName().str() << "\n";
                        }
                    }
                #endif
            }else{
                #if DEBUG_MODE
                    errs()<< "not a null func\n";
                    errs() << "func_name:" << func->getName().str() << "\n";
                #endif
            }

            if (func && (func->hasFnAttribute("no_instrument") || syscalls.find(func->getName().str()) != syscalls.end())) {
                return;
            }

            if(func && (func->getName().equals("malloc") || func->hasFnAttribute("malloc"))){
                handleMalloc(call_inst);
            }else if(func && (func->getName().equals("free") || func->hasFnAttribute("free"))){
                handleFree(call_inst);
            }else if(func && func->getName().equals("realloc")){
                handleRealloc(call_inst);
            }else if(func && func->getName().equals("calloc")){
                handleCalloc(call_inst);
            }else if(func && (func->getName().equals(StringRef("memcpy")) || 
                        func->getName().equals(StringRef("memmove")) || 
                        func->getName().equals(StringRef("memset")) ||
                        func->getName().equals(StringRef("wmemcpy")) ||
                        func->getName().equals(StringRef("wmemmove")) ||
                        func->getName().equals(StringRef("wmemset")))){
                addMemcopyMemsetCheck(call_inst, func);
                return;
            }else if(func && (func->getName().equals(StringRef("strtok")))){
                // addStrtokProp(call_inst, func);
                return;
            }else if(func && (
                func->getName().equals(StringRef("strcat")) || 
                func->getName().equals(StringRef("strcpy")) ||
                func->getName().equals(StringRef("strncat")) ||
                func->getName().equals(StringRef("strncpy")) ||
                func->getName().equals(StringRef("wcscat")) ||
                func->getName().equals(StringRef("wcsncat")) ||
                func->getName().equals(StringRef("wcscpy")) ||
                func->getName().equals(StringRef("wcsncpy")))) {
                addStringCheck(call_inst);
                return;
            } else if (func && (func->getName().equals(StringRef("strlen")) || 
                func->getName().equals(StringRef("wcslen")))) {
                handleStrlen(call_inst);
                return;
            } else if (func && (
                func->getName().equals(StringRef("sprintf")) || 
                func->getName().equals(StringRef("snprintf")) ||
                func->getName().equals(StringRef("vsprintf")) ||
                func->getName().equals(StringRef("vsnprintf")) ||
                func->getName().equals(StringRef("swprintf")) ||
                func->getName().equals(StringRef("vswprintf")))) {
                handleSprintf(call_inst);
                return;
            }else if(func && func->getName().equals(StringRef("__usan_"))){
                #if DEBUG_MODE
                    errs() << "ignored __usan_\n";
                #endif
                return;
            }else if(func && isIgnoredFunc(func->getName().str())){
                #if DEBUG_MODE
                    errs() << "Ingore: Ignored func\n";
                #endif
                return;
            }else if(func && isSelfSpecialFunc(func->getName().str())){
                #if DEBUG_MODE
                    errs() << "SoftBound defined functions\n";
                #endif
                if(!isa<PointerType>(call_inst->getType())){
                    #if DEBUG_MODE
                        errs() << "call_inst type is not a PointerType\n";
                    #endif
                }else{
                    #if DEBUG_MODE
                        errs() << "call_inst type is a PointerType\n";
                    #endif
                    // associateMetadata(call_inst, m_void_null_ptr);
                }
                return;
            } else if (func && func->getName().equals("xTaskCreate")) {
                handleCreateNewThread(call_inst);
            }else if(func && closed_func.count(func->getName().str())){
                return;
            }else{
                #if DEBUG_MODE
                    errs() << "need real handle\n";
                #endif
                Instruction* insert_before_call = call_inst;
                IRBuilder<> builder(insert_before_call);
                builder.SetInsertPoint(insert_before_call);
                // before call
                // iterateCallSiteIntroduceShadowStackStores
                SmallVector<Value *, 2> args;
                args.push_back(ConstantInt::get(Int8Type, 0));

                for(auto arg_beg = call_inst->arg_begin(), arg_end = call_inst->arg_end(); arg_beg != arg_end; arg_beg++){
                    Value* arg_value = dyn_cast<Value>(arg_beg);
                    #if DEBUG_MODE
                        errs() << "handle arg {";
                        arg_value->print(errs());
                        errs() << "}\n";
                    #endif
                    
                    if(arg_value->getType()->isPointerTy() && !arg_value->getType()->getPointerElementType()->isFunctionTy()){
                        #if DEBUG_MODE
                            errs() << "arg is PointerType\n";
                        #endif
                        if (auto arg_id = getPointerIDObject(arg_value)) {
                            args.push_back(arg_id);
                        }
                    }
                }

                if (args.size() > 1) {
                    args[0] = ConstantInt::get(Int8Type, args.size() - 1);
                    NumInstrumentedArgProp++;
                    builder.CreateCall(cb_arg_prop, args);                    
                }
                
                // after call
                Instruction* insert_after_call = call_inst->getNextNonDebugInstruction();
                builder.SetInsertPoint(insert_after_call);
                
                if (isa<PointerType>(call_inst->getType())) {
                    #if DEBUG_MODE
                        errs() << "ret arg is a pointer\n";
                    #endif
                    NumInstrumentedFuncProp++;
                    builder.CreateCall(cb_ret_prop, {getPointerIDObject(call_inst)});
                }
            }
        }

        void handleAlloca(AllocaInst *AI)
        {
            if (AI->isStaticAlloca() && !AI->isArrayAllocation()) {
                return;
            }

            IRBuilder<> IRB(AI);
            IRB.SetInsertPoint(AI->getNextNonDebugInstruction());
            auto M = AI->getModule();
            auto F = M->getOrInsertFunction("__usan_trace_alloca", FunctionType::get(VoidType, {Int32Type, Int32Type, Int32Type}, false));
            auto id = getPointerIDObject(AI);
            auto alloca_addr = IRB.CreatePtrToInt(AI, Int32Type, "alloca.addr");

            if (AI->isStaticAlloca()) {
                IRB.CreateCall(F, {id, alloca_addr, ConstantInt::get(Int32Type, getAllocaSizeInBytes(*AI))});
            } else {
                auto Ty = AI->getAllocatedType();
                uint64_t SizeInBytes = AI->getModule()->getDataLayout().getTypeAllocSize(Ty);
                auto AllocaSize = IRB.CreateMul(AI->getArraySize(), ConstantInt::get(Int32Type, SizeInBytes));
                IRB.CreateCall(F, {id, alloca_addr, AllocaSize});
            }
        }

        void handleReturnInst(ReturnInst* ret_inst){
            Value* ret_value = ret_inst->getReturnValue();
            IRBuilder<> IRB(ret_inst);

            IRB.SetInsertPoint(ret_inst);

            if (ret_inst->getFunction()->getName().equals(StringRef("main"))) {
                IRB.CreateCall(cb_fuzz_finish);
                return;
            }

            if(ret_value && isa<PointerType>(ret_value->getType())){
                 #if DEBUG_MODE
                    errs() << "call handleReturnInst\n";
                    if(ret_value){
                        errs() << "ret value: ";
                        ret_value->print(errs());
                        errs() << "\n";
                    }
                #endif
                auto ret_id = getPointerIDObject(ret_value);
                assert(ret_id);
                NumInstrumentedFuncRetProp++;
                IRB.CreateCall(cb_func_exit_prop, {ret_id});
            } else {
                NumInstrumentedFuncRet++;
                IRB.CreateCall(cb_func_exit);
            }
        }

        Value *emitPointerID(Value *V, bool PointerAddressAsId = false)
        {
            if (mCachedId.find(V) != mCachedId.end()) {
                return mCachedId[V];
            }

            Value *retValue = nullptr;

            if (auto AI = dyn_cast<AllocaInst>(V)) {
                IRBuilder<> IRB(AI->getNextNonDebugInstruction());
                if (AI->isStaticAlloca() && !AI->isArrayAllocation()) {
                    auto BOPC = IRB.CreateBitOrPointerCast(AI, Int32Type, AI->getName() + ".id");
                    retValue = IRB.CreateOr(BOPC, 1ull << 30);
                } else {
                    // FIXME: deal with alloca() function
                    // typically, alloca() is array allocation (except "alloca(sizeof(char))") or non-static.
                    retValue = ConstantInt::get(Int32Type, mTempId--);
                }
            } else if (auto EV = dyn_cast<ExtractValueInst>(V)) {
                // FIXME: XXX
                retValue = ConstantInt::get(Int32Type, 1);
            } else if (auto GV = dyn_cast<GlobalVariable>(V)) {
                auto name = GV->getName();
                // if (name == "TestCaseIdx" || name == "TestCaseLen" || name == "DeviceTestCaseBuffer") {
                //     #if DEBUG_MODE
                //         errs() << "Ignore pointer: ";
                //         GV->print(errs());
                //         errs() << "\n";
                //     #endif
                //     return nullptr;
                // }
                #if DEBUG_MODE
                    errs() << "Emitting id for global pointer: ";
                    GV->print(errs());
                    errs() << "\n";
                #endif
                // FIXEME
                if (GV->getType()->isPointerTy() && GV->hasInitializer() && isa<ConstantExpr>(GV->getInitializer())) {
                    retValue = emitPointerID(GV->getInitializer());
                } else {
                    retValue = ConstantExpr::getOr(ConstantExpr::getPointerCast(GV, Int32Type), ConstantInt::get(Int32Type, 1 << 30));
                }
            } else if (isa<ConstantExpr>(V) || isa<ConstantData>(V)) {
                retValue = ConstantInt::get(Int32Type, (uint64_t)!cast<Constant>(V)->isZeroValue());
            } else if (isa<CallInst>(V) || isa<Argument>(V) || isa<PHINode>(V) || isa<SelectInst>(V)) {
                retValue = ConstantInt::get(Int32Type, mTempId--);
            } else if (isa<Function>(V)) {
                retValue = ConstantInt::get(Int32Type, 1);
            } else {
                // #if DEBUG_MODE
                    errs() << "Unable to emit an ID for: ";
                    V->print(errs());
                    errs() << "\n";
                // #endif
            }

            mCachedId.insert(std::pair<Value *, Value *>(V, retValue));

            return retValue;
        }

        Value *emitPointerAddress(Value *V)
        {
            if (mCachedAddressId.find(V) != mCachedAddressId.end()) {
                return mCachedAddressId[V];
            }

            Value *retValue;

            if (isa<Instruction>(V)) {
                if (auto PHI = dyn_cast<PHINode>(V)) {
                    auto I = PHI->getParent()->getFirstInsertionPt();
                    IRBuilder<> IRB(&*I);
                    retValue = IRB.CreateBitOrPointerCast(V, Int32Type);
                } else {
                    IRBuilder<> IRB(cast<Instruction>(V)->getNextNonDebugInstruction());
                    retValue = IRB.CreateBitOrPointerCast(V, Int32Type);
                }
            } else if (auto C = dyn_cast<Constant>(V)) {
                retValue = ConstantExpr::getPointerCast(C, Int32Type);
            } else if (auto A = dyn_cast<Argument>(V)) {
                auto I = A->getParent()->getEntryBlock().getFirstInsertionPt();
                IRBuilder<> IRB(&*I);
                retValue = IRB.CreateBitOrPointerCast(V, Int32Type);
            } else {
                errs() << "I don't know how to handle it\n";
                V->print(errs());
                errs() << "\n";
                abort();
            }

            mCachedAddressId[V] = retValue;

            return retValue;
        }

        Value *getPointerIDObject(Value *ptr)
        {
            auto V = ptr;

            if (isa<AllocaInst>(V) 
                    || isa<CallInst>(V) || isa<Argument>(V) 
                    || isa<GlobalValue>(V) || isa<ConstantData>(V)
                    || isa<PHINode>(V) || isa<SelectInst>(V)
                    || isa<ExtractValueInst>(V)) {
                return emitPointerID(V);
            } else if (isa<LoadInst>(V)) {
                return emitPointerAddress(cast<LoadInst>(V)->getPointerOperand());
            } else if (auto GEP = dyn_cast<GEPOperator>(V)) {
                V = GEP->getPointerOperand();
            } else if (isa<BinaryOperator>(V)) {
                // XXX: Is this correct??
                V = cast<Operator>(V)->getOperand(0);
            } else if (isa<IntToPtrInst>(V) || isa<PtrToIntInst>(V) || isa<BitCastInst>(V) || isa<ZExtInst>(V)) {
                V = cast<Instruction>(V)->getOperand(0);
            } else if (isa<ConstantExpr>(V)) {
                V = cast<ConstantExpr>(V)->getOperand(0);
            } else {
                // #if DEBUG_MODE
                    errs() << "I don't know how to handle it: ";
                    V->print(errs());
                    errs() << "\n";
                    V->getType()->print(errs());
                    errs() << "\n";
                // #endif
                abort();
            }

            return getPointerIDObject(V);
        }

        void instrumentPropagation(Value *dest, Value *src, Instruction *where)
        {
            if (auto src_id = getPointerIDObject(src)) {
                #if DEBUG_MODE
                    errs() << "source id: ";
                    src_id->print(errs());
                    errs() << "\n";
                #endif
                NumInstrumentedProp++;
                IRBuilder<> IRB(where);
                IRB.SetInsertPoint(where);
                auto dst_id = IRB.CreatePointerCast(dest, Int32Type, dest->getName() + ".id");
                IRB.CreateCall(cb_prop, {dst_id, src_id});                
            }
        }

        bool isPointerType(Value *operand) {
            return isa<PointerType>(operand->getType()) || isa<PtrToIntInst>(operand) || isa<PtrToIntOperator>(operand);
        }

        // Done
        void handleStore(StoreInst* store_inst){
            Value* operand = store_inst->getOperand(0);
            Value* pointer_dest = store_inst->getOperand(1);
            
            if(isa<VectorType>(operand->getType())){
                assert(!"operand is VectorType");
            }

            if(!isPointerType(operand)) {
                // ignore non-pointer type
                return;
            }

            Instruction* insert_at = store_inst->getNextNonDebugInstruction();

            #if DEBUG_MODE
                errs() << "call handleStore\n";
                errs() << "Store value:";
                operand->print(errs());
                errs() << "\nStore ptr:";
                pointer_dest->print(errs());
                errs() << "\n";
            #endif

            instrumentPropagation(pointer_dest, operand, insert_at);
        }

        void handleSelect(SelectInst *SI){
            if (!isa<PointerType>(SI->getType())){
                #if DEBUG_MODE
                    errs() << "is not PointerType\n";
                #endif
                return;
            }

            #if DEBUG_MODE
                errs() << "call handleSelect\n";
            #endif

            IRBuilder<> IRB(SI->getNextNonDebugInstruction());
            Value *srcId = nullptr;

            auto falseValueId = getPointerIDObject(SI->getFalseValue());
            auto trueValueId = getPointerIDObject(SI->getTrueValue());
            auto dstId = getPointerIDObject(SI);

            assert(falseValueId || trueValueId);

            if (falseValueId && trueValueId)
                srcId = IRB.CreateSelect(SI->getCondition(), trueValueId, falseValueId, SI->getName() + ".src.id");
            else if (falseValueId)
                srcId = falseValueId;
            else
                srcId = trueValueId;

            NumInstrumentedProp++;
            IRB.CreateCall(cb_prop, {dstId, srcId});
        }

        void handlePHI(PHINode *PHI) {
            if (!PHI->getType()->isPointerTy()) {
                return;
            }

            #if DEBUG_MODE
                errs() << "handle PHI node: ";
                PHI->print(errs());
                errs() << "\n";
            #endif

            BasicBlock *BB = PHI->getParent();
            IRBuilder<> IRB(PHI);
            SmallVector<std::pair<Value *, BasicBlock *>, 2> terminatedPairs;

            for (auto i = 0; i < PHI->getNumIncomingValues(); i++) {
                auto incomingId = getPointerIDObject(PHI->getIncomingValue(i));
                assert(incomingId);
                terminatedPairs.emplace_back(std::pair<Value *, BasicBlock *>(incomingId, PHI->getIncomingBlock(i)));
            }

            auto srcId = IRB.CreatePHI(Int32Type, terminatedPairs.size(), PHI->getName() + ".src.id");
            auto dstId = getPointerIDObject(PHI);

            for (auto p : terminatedPairs) {
                srcId->addIncoming(p.first, p.second);
            }

            NumInstrumentedProp++;
            IRB.SetInsertPoint(BB->getFirstNonPHIOrDbg());
            IRB.CreateCall(cb_prop, {dstId, srcId});
        }

        void handlePHI1(PHINode* phi_node){
            #if DEBUG_MODE
                errs() << "call handlePHI1()\n";
            #endif
            if(!(isa<PointerType>(phi_node->getType()) && phi_node->getNumUses() > 0)){
                #if DEBUG_MODE
                    errs() << "Ignored: not a PointerType or no uses\n";
                #endif
                return;
            }
            Instruction* insert_at = dyn_cast<Instruction>(phi_node);
            do{
                insert_at = insert_at->getNextNode();
                #if DEBUG_MODE
                    errs() << "insert_at: ";
                    insert_at->print(errs());
                    errs() << "\n";
                #endif
            }while(isa<PHINode>(insert_at)); // PHI need to be continuous
            unsigned num_incoming_values = phi_node->getNumIncomingValues();
            #if DEBUG_MODE
                errs() << "num incoming value: " << num_incoming_values << "\n";
            #endif
            PHINode* prop_phi_node = PHINode::Create(Int16Type, num_incoming_values, "phi.prop", insert_at);
            // PHINode* base_phi_node = PHINode::Create(Int32Type, num_incoming_values, "phi.base", insert_at);
            // PHINode* bound_phi_node = PHINode::Create(Int32Type, num_incoming_values, "phi.bound", insert_at);
            for (unsigned m = 0; m < num_incoming_values; m++){
                #if DEBUG_MODE
                    errs() << "incoming number: " << m << "\n";
                #endif
                Value* incoming_value = phi_node->getIncomingValue(m);
                #if DEBUG_MODE
                    errs() << "incomiing_value: ";
                    if(incoming_value){
                        incoming_value->print(errs());
                        errs() << "\nincoming_value type: ";
                        incoming_value->getType()->print(errs());
                    }
                    errs() << "\n";
                #endif
                BasicBlock* bb_incoming = phi_node->getIncomingBlock(m);
                Function* PHI_func = phi_node->getFunction();
                Instruction* PHI_func_entry = dyn_cast<Instruction>(PHI_func->begin()->begin());
                // Value* id = getMetadataId(incoming_value);
                // prop_phi_node->addIncoming(id, bb_incoming);
                // incoming_value = getChainOrigin(incoming_value);
                // Constant* given_constant = dyn_cast<Constant>(incoming_value);
                // if(given_constant){
                //     #if DEBUG_MODE
                //         errs() << "incoming value is Constant\n";
                //     #endif
                //     Value* base = NULL;
                //     Value* bound = NULL;
                //     getConstantExprBaseBound(given_constant, base, bound);
                //     if(!isa<ConstantInt>(base)){
                //         #if DEBUG_MODE
                //             errs() << "base PtrToIntInst\n";
                //         #endif
                //         base = new PtrToIntInst(base, Int32Type, "base", PHI_func_entry);   
                //     }
                //     if(!isa<ConstantInt>(bound)){
                //         #if DEBUG_MODE
                //             errs() << "bound PtrToIntInst\n";
                //         #endif
                //         bound = new PtrToIntInst(bound, Int32Type, "bound", PHI_func_entry);
                //     }
                //     base_phi_node->addIncoming(base, bb_incoming);
                //     bound_phi_node->addIncoming(bound, bb_incoming);
                // }else if(isa<UndefValue>(incoming_value)){
                //     #if DEBUG_MODE
                //         errs() << "incoming value is undef\n";
                //     #endif
                //     assert(!"incoming value is undef");
                // }else if(checkMetadataPresent(incoming_value)){
                //     #if DEBUG_MODE
                //         errs() << "incoming value metatdata present\n";
                //     #endif
                //     Value* base = NULL;
                //     Value* bound = NULL;
                //     base = getMetadata(incoming_value);
                //     if(!isa<ConstantInt>(base)){
                //         base = new PtrToIntInst(base, Int32Type, "base", PHI_func_entry);
                //     }
                //     bound = ConstantInt::get(Int32Type, 0);
                //     base_phi_node->addIncoming(base, bb_incoming);
                //     bound_phi_node->addIncoming(bound, bb_incoming);
                // }else{
                //     Value* origin_inst = incoming_value;
                //     #if DEBUG_MODE
                //         errs() << "incoming_value may be integer, try to add alloca rtt\n";
                //     #endif
                //     AllocaInst* alloca_inst = dyn_cast<AllocaInst>(origin_inst);
                //     if(alloca_inst){
                //         preAllocaAssign(alloca_inst, true);
                //         handleAlloca(alloca_inst, true);
                //         Value* base = NULL;
                //         Value* bound = NULL;
                //         base = getMetadata(incoming_value);
                //         if(!isa<ConstantInt>(base)){
                //             base = new PtrToIntInst(base, Int32Type, "base", PHI_func_entry);
                //         }
                //         bound = ConstantInt::get(Int32Type, 0);
                //         base_phi_node->addIncoming(base, bb_incoming);
                //         bound_phi_node->addIncoming(bound, bb_incoming);
                //     }else{
                //         assert(!"Unable reached phi situation");
                //     }
                // }
            }
            IRBuilder<> builder(insert_at);
            builder.SetInsertPoint(insert_at);
            // builder.CreateCall(cb_prop, {getMetadataId(phi_node), prop_phi_node});
            #if DEBUG_MODE
                errs() << "id: ";
                // getMetadataId(phi_node)->print(errs());
                errs() << "\nprop: ";
                prop_phi_node->print(errs());
                errs() << "\n";
            #endif
            // builder.CreateCall(segger_rtt_write_propagate_const, {getMetadata(phi_node), base_phi_node, bound_phi_node});
            // #if DEBUG_MODE
            //     errs() << "Id: ";
            //     getMetadata(phi_node)->print(errs());
            //     errs() << "\n";
            //     errs() << "base: ";
            //     base_phi_node->print(errs());
            //     errs() << "\n" << "bound: ";
            //     bound_phi_node->print(errs());
            //     errs() << "\n";
            // #endif
        }
        
        uint64_t getAllocaSizeInBytes(const AllocaInst &AI) const {
            uint64_t ArraySize = 1;
            if (AI.isArrayAllocation()) {
            const ConstantInt *CI = dyn_cast<ConstantInt>(AI.getArraySize());
            assert(CI && "non-constant array size");
            ArraySize = CI->getZExtValue();
            }
            Type *Ty = AI.getAllocatedType();
            uint64_t SizeInBytes =
                AI.getModule()->getDataLayout().getTypeAllocSize(Ty);
            return SizeInBytes * ArraySize;
        }
        bool isInterestingAlloca(const AllocaInst &AI){
            auto PreviouslySeenAllocaInfo = ProcessedAllocas.find(&AI);
            if (PreviouslySeenAllocaInfo != ProcessedAllocas.end()){
                #if DEBUG_MODE
                    errs() << "processed alloca, interesting is ";
                    if(PreviouslySeenAllocaInfo->second)
                        errs() << "true\n";
                    else
                        errs() << "false\n";
                #endif
                return PreviouslySeenAllocaInfo->second;
            }
            #if DEBUG_MODE
                errs() << "Call isInterestingAlloca()\n";
                if(AI.getAllocatedType()->isSized()){
                    errs() << "AI.getAllocatedType()->isSized() is true\n";
                }else{
                    errs() << "AI.getAllocatedType()->isSized() is false\n";
                }
                if(!AI.isStaticAlloca()){
                    errs() << "!AI.isStaticAlloca() is true\n";
                }else{
                    errs() << "!AI.isStaticAlloca() is false\n";
                }
                if(getAllocaSizeInBytes(AI) > 0){
                    errs() << "getAllocaSizeInBytes(AI) > 0 is true\n";
                }else{
                    errs() << "getAllocaSizeInBytes(AI) > 0 is true\n";
                }
                if(!isAllocaPromotable(&AI)){
                    errs() << "!isAllocaPromotable(&AI) is true\n";
                }else{
                    errs() << "!isAllocaPromotable(&AI) is false\n";
                }
                if(!AI.isUsedWithInAlloca()){
                    errs() << "!AI.isUsedWithInAlloca() is true\n";
                }else{
                    errs() << "!AI.isUsedWithInAlloca() is false\n";
                }
                if(!AI.isSwiftError()){
                    errs() << "!AI.isSwiftError() is ture\n";
                }else{
                    errs() << "!AI.isSwiftError() is false\n";
                }
            #endif
            bool IsInteresting =
                (AI.getAllocatedType()->isSized() &&
                // alloca() may be called with 0 size, ignore it.
                ((!AI.isStaticAlloca()) || getAllocaSizeInBytes(AI) > 0) &&
                // We are only interested in allocas not promotable to registers.
                // Promotable allocas are common under -O0.
                (!isAllocaPromotable(&AI)) &&
                // inalloca allocas are not treated as static, and we don't want
                // dynamic alloca instrumentation for them as well.
                !AI.isUsedWithInAlloca() &&
                // swifterror allocas are register promoted by ISel
                !AI.isSwiftError());
            #if DEBUG_MODE
                if(IsInteresting){
                    errs() << "IsInteresting is true\n";
                }else{
                    errs() << "IsInteresting is false\n";
                }
            #endif
            ProcessedAllocas[&AI] = IsInteresting;
            return IsInteresting;
        }

        bool ignoreAccess(Value *Ptr){
            #if DEBUG_MODE
                errs() << "Call ignoreAccess()\n";
            #endif
            // Do not instrument acesses from different address spaces; we cannot deal
            // with them.
            Type *PtrTy = cast<PointerType>(Ptr->getType()->getScalarType());
            if (PtrTy->getPointerAddressSpace() != 0){
                #if DEBUG_MODE
                    errs() << "getPointerAddressSpace not zero, return true\n";
                #endif
                return true;
            }

            // Ignore swifterror addresses.
            // swifterror memory addresses are mem2reg promoted by instruction
            // selection. As such they cannot have regular uses like an instrumentation
            // function and it makes no sense to track them as memory.
            if (Ptr->isSwiftError()){
                #if DEBUG_MODE
                    errs() << "isSwiftError(), return true\n";
                #endif
                return true;
            }

            // Treat memory accesses to promotable allocas as non-interesting since they
            // will not cause memory violations. This greatly speeds up the instrumented
            // executable at -O0.
            if (auto AI = dyn_cast_or_null<AllocaInst>(Ptr)){
                #if DEBUG_MODE
                    errs() << "AI exists\n";
                #endif
                if (!isInterestingAlloca(*AI)){
                    #if DEBUG_MODE
                        errs() << "isInterestingAlloca return false and ignore access\n";
                    #endif
                    return true;
                }
            }

            return false;
        }
        
        void gatherBaseBoundPass1(Function* func_ptr, std::vector<Instruction *> &insts, std::deque<std::function<void()>> &task_queue){
            #if DEBUG_MODE
                errs() << "Call gatherBaseBoundPass1\n" << "handle function parameters\n";
            #endif
            if (!func_ptr->getName().equals(StringRef("main"))) {
                // don't process "char *argv[]" for main function
                // function parameters
                for(Function::arg_iterator arg_begin = func_ptr->arg_begin(), arg_end = func_ptr->arg_end(); arg_begin != arg_end; arg_begin++){
                    if(!arg_begin->getType()->isPointerTy() || arg_begin->getType()->getPointerElementType()->isFunctionTy()){
                        continue;
                    }
                    #if DEBUG_MODE
                        arg_begin->print(errs());
                        errs() << "\n";
                    #endif
                    getPointerIDObject(arg_begin);
                }
            }

            // function body
            #if DEBUG_MODE
                errs() << "handle function body\n";
            #endif

            for (auto I : insts) {
                #if DEBUG_MODE
                    errs() << "handling instruction:";
                    I->print(errs());
                    errs() << "\n";
                #endif

                switch(I->getOpcode()){
                case Instruction::Alloca:
                    task_queue.push_back(std::bind(&IPEASan::handleAlloca,  this, dyn_cast<AllocaInst>(I)));
                    break;

                case Instruction::PHI:
                    task_queue.push_back(std::bind(&IPEASan::handlePHI, this, dyn_cast<PHINode>(I)));
                    break;
                
                case Instruction::Call:
                    task_queue.push_back(std::bind(&IPEASan::handleCall, this, dyn_cast<CallInst>(I)));
                    break;
                
                case Instruction::Select:
                    task_queue.push_back(std::bind(&IPEASan::handleSelect, this, dyn_cast<SelectInst>(I)));
                    break;
                
                case Instruction::Store: // this is located in the pass2
                    task_queue.push_back(std::bind(&IPEASan::handleStore, this, dyn_cast<StoreInst>(I)));
                    break;
                
                case Instruction::Ret:
                    task_queue.push_back(std::bind(&IPEASan::handleReturnInst, this, dyn_cast<ReturnInst>(I)));
                    break;
                
                default:
                    break;
                }
            }
        }
        
    #if ENABLE_LOOP_OPTIMIZATION
        bool isAcceptableToNotInstrumentMem(Instruction* inst){
            #if DEBUG_MODE
                errs() << "Call isAcceptableToNotInstrumentMem()\n";
            #endif
            switch(inst->getOpcode()){
                case Instruction::Alloca:
                {
                    AllocaInst* alloca_inst = dyn_cast<AllocaInst>(inst);
                    if(isInterestingAlloca(*alloca_inst)){
                        return false;
                    }else{
                        return true;
                    }
                }
                case Instruction::PHI:
                case Instruction::Select:
                case Instruction::ExtractValue:
                {
                    if( !isa<PointerType>(inst->getType()) && !isa<ArrayType>(inst->getType()) && !isa<StructType>(inst->getType()) ){
                        #if DEBUG_MODE
                            errs() << "Ignored: not a PointerType\n";
                        #endif
                        return true;
                    }else{
                        return false;
                    }
                }
                case Instruction::IntToPtr:
                case Instruction::Ret:
                {
                    return false;
                }
                case Instruction::ExtractElement:
                {
                    assert(!"first time meet extract");
                }
                case Instruction::Call:
                {
                    if(cast<CallBase>(inst)->isInlineAsm()){
                        #if DEBUG_MODE
                            errs() << "Ignored: isInlineAsm\n";
                        #endif
                        return true;
                    }
                    CallInst* call_inst = dyn_cast<CallInst>(inst);
                    Function* func = call_inst->getCalledFunction();
                    if(func && func->getName().contains(StringRef("llvm.dbg"))){
                        #if DEBUG_MODE
                            errs() << "Ignored: llvm debug func\n";
                        #endif
                        return true;
                    }
                    return false;
                }
                case Instruction::Load:
                {
                    return cast<LoadInst>(inst)->isVolatile();
                }
                case Instruction::Store:
                {
                    return cast<StoreInst>(inst)->isVolatile();
                }
                default:
                {
                    #if DEBUG_MODE
                        errs() << "default true\n";
                    #endif
                    return true;
                }
            }
        }
        bool isLoopDeadMem(Loop *L, ScalarEvolution &SE, SmallVectorImpl<BasicBlock *> &ExitingBlocks, BasicBlock *ExitBlock, bool &Changed, BasicBlock *Preheader){
            #if DEBUG_MODE
                errs() << "call isLoopDeadMem\n";
            #endif
            bool all_outgoing_values_same = true;
            bool all_entries_in_variant = true;
            for (PHINode &P : ExitBlock->phis()){
                Value *incoming = P.getIncomingValueForBlock(ExitingBlocks[0]);
                #if DEBUG_MODE
                    errs() << "incoming: ";
                    incoming->print(errs());
                    errs() << "\n";
                #endif
                all_outgoing_values_same = all_of(makeArrayRef(ExitingBlocks).slice(1), [&](BasicBlock *BB) {
                    return incoming == P.getIncomingValueForBlock(BB);
                });
                if(!all_outgoing_values_same){
                    #if DEBUG_MODE
                        errs() << "all_outgoing_values_same is false\n";
                    #endif
                    break;
                }
                Instruction* I = dyn_cast<Instruction>(incoming);
                if(I){
                    if(!L->makeLoopInvariant(I, Changed, Preheader->getTerminator())){
                        #if DEBUG_MODE
                            errs() << "make loop in variant fails\n";
                        #endif
                        all_entries_in_variant = false;
                    }else{
                        assert(!"make loop in variant succeed");
                    }
                }else{
                    assert(!"not an instruction");
                }
            }
            if(Changed){
                #if DEBUG_MODE
                    errs() << "changed is true\n";
                #endif
                SE.forgetLoopDispositions(L);
            }
            if(!all_entries_in_variant || !all_outgoing_values_same){
                #if DEBUG_MODE
                    errs() << "variant or value failes\n";
                #endif
                return false;
            }
            for (BasicBlock *BB : L->getBlocks())
            {
                // errs() << "basic block name: "<< BB->getName() <<"\n";
                #if DEBUG_MODE
                    errs() << "basic block:[\n";
                    BB->print(errs());
                    errs() << "]\n";
                #endif
                for (auto &Inst : *BB)
                {
                    Instruction *i = dyn_cast<Instruction>(&Inst);
                    #if DEBUG_MODE
                        errs() << "instruction: ";
                        i->print(errs());
                        errs() << "\n";
                    #endif
                    if (i && !isAcceptableToNotInstrumentMem(i))
                    {
                        #if DEBUG_MODE
                            errs() << "not acceptable to NotInstrumentMem\n";
                        #endif
                        return false;
                    }
                }
            }
            #if DEBUG_MODE
                errs() << "return true\n";
            #endif
            return true;
        }
        bool isLoopDeadTrans(Loop *L, ScalarEvolution &SE, SmallVectorImpl<BasicBlock *> &ExitingBlocks, BasicBlock *ExitBlock, bool &Changed, BasicBlock *Preheader){
            #if DEBUG_MODE
                errs() << "call isLoopDeadTrans\n";
            #endif
            bool all_outgoing_values_same = true;
            bool all_entries_in_variant = true;
            for (PHINode &P : ExitBlock->phis()){
                Value *incoming = P.getIncomingValueForBlock(ExitingBlocks[0]);
                #if DEBUG_MODE
                    errs() << "incoming: ";
                    incoming->print(errs());
                    errs() << "\n";
                #endif
                all_outgoing_values_same = all_of(makeArrayRef(ExitingBlocks).slice(1), [&](BasicBlock *BB) {
                    return incoming == P.getIncomingValueForBlock(BB);
                });
                if(!all_outgoing_values_same){
                    #if DEBUG_MODE
                        errs() << "all_outgoing_values_same is false\n";
                    #endif
                    break;
                }
                Instruction* I = dyn_cast<Instruction>(incoming);
                if(I){
                    if(!L->makeLoopInvariant(I, Changed, Preheader->getTerminator())){
                        #if DEBUG_MODE
                            errs() << "make loop in variant fails\n";
                        #endif
                        all_entries_in_variant = false;
                        break;
                    }else{
                        assert(!"make loop in variant succeed");
                    }
                }else{
                    assert(!"not an instruction");
                } 
            }
            if(Changed){
                #if DEBUG_MODE
                    errs() << "changed is true\n";
                #endif
                SE.forgetLoopDispositions(L);
            }
            if(!all_entries_in_variant || !all_outgoing_values_same){
                #if DEBUG_MODE
                    errs() << "variant or value failes\n";
                #endif
                return false;
            }
            for (auto *BB : L->getBlocks()){
                for(auto &Inst: *BB){
                    Instruction *i = dyn_cast<Instruction>(&Inst);
                    #if DEBUG_MODE
                        errs() << "inst: ";
                        i->print(errs());
                        errs() << "\n";
                    #endif
                    CallInst* call_inst = dyn_cast<CallInst>(i);
                    if(call_inst){
                        Function* func = call_inst->getCalledFunction();
                        if (func) {
                            auto name = func->getName();
                            if(name.contains(StringRef("__usan_"))){
                                #if DEBUG_MODE
                                    errs() << "mem instrumented\n";
                                #endif
                                return false;
                            }
                        }
                    }
                }
            }
            #if DEBUG_MODE
                errs() << "return true\n";
            #endif
            return true;
        }
        void getAllLoops(SmallVectorImpl<Loop *> &LoopVector, Loop *L){
            #if DEBUG_MODE
                errs() << "Call getAllLoops\n";
            #endif
            LoopVector.push_back(L);
            std::vector<Loop *> subLoops = L->getSubLoops();
            Loop::iterator j, f;
            for (j = subLoops.begin(), f = subLoops.end(); j != f; ++j)
                getAllLoops(LoopVector, *j);
        }
        void handleLoopTrans(Function* func){
            #if DEBUG_MODE
                errs() << "call handleLoopTrans\n";
            #endif
            SmallVector<Loop *, 16> LoopVector;
            DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
            ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            for (Loop *L : LI){
                #if DEBUG_MODE
                    errs() << "loop detected\n";
                #endif
                getAllLoops(LoopVector, L);
            }
            #if DEBUG_MODE
                errs() << "Total " << LoopVector.size() << " loops\n";
            #endif 
            for(unsigned int N = 0; N < LoopVector.size(); N++){
                Loop *L = LoopVector[N];
                bool changed = false;
                BasicBlock *Preheader = L->getLoopPreheader();
                BasicBlock *ExitBlock = L->getUniqueExitBlock();
                SmallVector<BasicBlock *, 4> ExitingBlocks;
                SmallVector<BasicBlock *, 4> ExitBlocks;
                L->getExitingBlocks(ExitingBlocks);
                L->getExitBlocks(ExitBlocks);
                if(!Preheader){
                    #if DEBUG_MODE
                        errs() << "no preheader\n"; // non-reachable
                    #endif
                    continue;
                }else{
                    #if DEBUG_MODE
                        errs() << "Loop In Function: " << Preheader->getParent()->getName() << "\n";
                        errs() << "Loop Preheader:[\n";
                        Preheader->print(errs());
                        errs() << "\n]\n";
                        for(unsigned int i = 0; i < ExitingBlocks.size(); i++){
                            errs() << "Loop exiting " << i << " :[\n";
                            ExitingBlocks[i]->print(errs());
                            errs() << "\n]\n";
                        }
                        for(unsigned int i = 0; i < ExitBlocks.size(); i++){
                            errs() << "Loop exit " << i << " :[\n";
                            ExitBlocks[i]->print(errs());
                            errs() << "\n]\n";
                        }
                    #endif                    
                }
                if(!L->hasDedicatedExits()){
                    #if DEBUG_MODE
                        errs() << "has no dedicated exits\n";
                    #endif
                    continue;
                    // assert(!"no DedicatedExits\n");
                }
                if(!ExitBlock){
                    #if DEBUG_MODE
                        errs() << "require unique exit block\n";
                    #endif
                    continue;
                    // assert(!"multiple exit blocks");
                }
                if(isLoopDeadTrans(L, SE, ExitingBlocks, ExitBlock, changed, Preheader)){
                    #if DEBUG_MODE
                        errs() << "loop should not be instrumented for trans\n";
                    #endif
                    LoopNotInstrumentTrans.push_back(L);
                }else{
                    #if DEBUG_MODE
                        errs() << "loop should be instrumented for trans\n";
                    #endif
                } 
            }
        }
        void handleLoopMem(){
            #if DEBUG_MODE
                errs() << "call handleLoopMem\n";
            #endif
            SmallVector<Loop *, 16> LoopVector;
            DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
            ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            for(Loop *L : LI){
                #if DEBUG_MODE
                    errs() << "loop detected\n";
                #endif
                getAllLoops(LoopVector, L);
            }
            #if DEBUG_MODE
                errs() << "Total " << LoopVector.size() << " loops\n";
            #endif
            for(unsigned int N = 0; N < LoopVector.size(); N++){
                Loop *L = LoopVector[N];
                bool changed = false;
                BasicBlock *Preheader = L->getLoopPreheader();
                BasicBlock *ExitBlock = L->getUniqueExitBlock();
                SmallVector<BasicBlock *, 4> ExitingBlocks;
                SmallVector<BasicBlock *, 4> ExitBlocks;
                L->getExitingBlocks(ExitingBlocks);
                L->getExitBlocks(ExitBlocks);
                if(!Preheader){
                    #if DEBUG_MODE
                        errs() << "no preheader\n"; // non-reachable
                    #endif
                    continue;
                }else{
                    #if DEBUG_MODE
                        errs() << "Loop In Function: " << Preheader->getParent()->getName() << "\n";
                        errs() << "Loop Preheader:[\n";
                        Preheader->print(errs());
                        errs() << "\n]\n";
                        for(unsigned int i = 0; i < ExitingBlocks.size(); i++){
                            errs() << "Loop exiting " << i << " :[\n";
                            ExitingBlocks[i]->print(errs());
                            errs() << "\n]\n";
                        }
                        for(unsigned int i = 0; i < ExitBlocks.size(); i++){
                            errs() << "Loop exit " << i << " :[\n";
                            ExitBlocks[i]->print(errs());
                            errs() << "\n]\n";
                        }
                    #endif
                }
                if(!L->hasDedicatedExits()){
                    #if DEBUG_MODE
                        errs() << "has no dedicated exits\n";
                    #endif
                    continue;
                    // assert(!"no DedicatedExits\n");
                }
                if(!ExitBlock){
                    #if DEBUG_MODE
                        errs() << "require unique exit block\n";
                    #endif
                    continue;
                    // assert(!"multiple exit blocks");
                }
                if(isLoopDeadMem(L, SE, ExitingBlocks, ExitBlock, changed, Preheader)){
                    #if DEBUG_MODE
                        errs() << "loop should not be instrumented for memory access\n";
                    #endif
                    LoopNotInstrumentMem.push_back(L);
                }else{
                    #if DEBUG_MODE
                        errs() << "loop should be instrumented for memory access\n";
                    #endif
                }
            }
        }
    #endif

        void preProcessing(Function* func_ptr){
            #if DEBUG_MODE
                errs() << "call preProcessing\n";
            #endif

             for(Function::arg_iterator i = func_ptr->arg_begin(), e = func_ptr->arg_end(); i != e; ++i) {
                if (isa<PointerType>(i->getType())) {
                    hasPtrArg = true;
                    break;
                }
            }

            for (auto &BB : *func_ptr) {
                for (auto &I : BB) {
                    if (auto AI = dyn_cast<AllocaInst>(&I)) {
                        if (isInterestingAlloca(*AI)) {
                            has_stack_op = true;
                            return;
                        }
                    }
                }
            }
        }

        
    #if ENABLE_LOOP_OPTIMIZATION
        bool BBinLoopNotTrans(BasicBlock* bb){
            #if DEBUG_MODE
                errs() << "Call BBinLoopNotTrans\n";
                errs() << "matching bb[\n";
                bb->print(errs());
                errs() << "]\n";
            #endif
            for(int i = 0; i < LoopNotInstrumentTrans.size(); i++){
                Loop* L = LoopNotInstrumentTrans[i];
                for(BasicBlock* bb_in_loop : L->getBlocks()){
                    #if DEBUG_MODE
                        errs() << "checking bb not trans[\n";
                        bb_in_loop->print(errs());
                        errs() << "]\n";
                    #endif
                    if(bb == bb_in_loop){
                        #if DEBUG_MODE
                            errs() << "bb in loop not trans\n";
                        #endif
                        return true;
                    }
                }
            }
            #if DEBUG_MODE
                errs() << "bb not in loop not trans\n";
            #endif
            return false;
        }
        bool BBinLoopNotMem(BasicBlock* bb){
            #if DEBUG_MODE
                errs() << "Call BBinLoopNotMem\n";
                errs() << "matching bb[\n";
                bb->print(errs());
                errs() << "]\n";
            #endif
            for(int i = 0; i < LoopNotInstrumentMem.size(); i++){
                Loop* L = LoopNotInstrumentMem[i];
                for(BasicBlock* bb_in_loop : L->getBlocks()){
                    #if DEBUG_MODE
                        errs() << "checking bb not mem[\n";
                        bb_in_loop->print(errs());
                        errs() << "]\n";
                    #endif
                    if(bb == bb_in_loop){
                        #if DEBUG_MODE
                            errs() << "bb in loop not mem\n";
                        #endif
                        return true;
                    }
                }
            }
            #if DEBUG_MODE
                errs() << "bb not in loop not mem\n";
            #endif
            return false;
        }
    #endif
        void insertRandom(Function* func_ptr){
            #if DEBUG_MODE
                errs() << "Call insertRandom()\n";
            #endif
            unsigned int inst_ratio = 100;
            // for(Module::iterator ff_begin = module.begin(), ff_end = module.end(); ff_begin != ff_end; ff_begin++){
                // Function* func_ptr = dyn_cast<Function>(ff_begin);
                #if DEBUG_MODE
                    errs() << "[insertRandom] handling " << func_ptr->getName() << "\n";
                #endif
                if(!checkIfFunctionOfInterest(func_ptr) || isInterruptHandler(func_ptr)){
                    #if DEBUG_MODE
                        errs() << "Do not insert random to function:" << func_ptr->getName().str() << "\n";
                    #endif
                    // continue;
                    return;
                }
                for(Function::iterator bb_begin = func_ptr->begin(), bb_end = func_ptr->end(); bb_begin != bb_end; bb_begin++){
                    BasicBlock* bb_ptr = dyn_cast<BasicBlock>(bb_begin);
                    #if ENABLE_LOOP_OPTIMIZATION
                        if(BBinLoopNotTrans(bb_ptr)){
                            #if DEBUG_MODE
                                errs() << "this bb should not instrument random\n";
                            #endif
                            continue;
                        }
                    #endif
                    #if DEBUG_MODE
                        errs() << "this bb should instrument random\n";
                    #endif
                    BasicBlock::iterator IP = bb_ptr->getFirstInsertionPt();
                    IRBuilder<> builder(&(*IP));
                    if ( AFL_R(100) >= inst_ratio) 
                        continue;
                    unsigned int cur_loc = AFL_R(MAP_SIZE);
                    builder.CreateCall(cb_bb_entry, {ConstantInt::get(Int16Type, cur_loc)});
                }
            // }
        }

        void instrumentInterruptHandler(Function* func){
            #if DEBUG_MODE
                errs() << "call instrumentInterruptHandler\n";
            #endif
            BasicBlock::iterator insert_point = func->getEntryBlock().getFirstInsertionPt();
            IRBuilder<> builder(&(*insert_point));
            NumInstrumentedISR++;
            builder.CreateCall(cb_irq_entry);
        }

        bool isInterruptHandler(Function* func){
            #if 0
            #if DEBUG_MODE
                errs() << "call isInterruptHandler()\n";
            #endif
            bool found = annotFuncs.find(func) != annotFuncs.end();
            #if DEBUG_MODE
                if(found){
                    errs() << "is interrupt handler\n";
                }else{
                    errs() << "not interrupt handler\n";
                }
            #endif
            return found;
            #endif

            return func->hasFnAttribute("interruptHandler");
        }

        void annotateFunctions(Module &M) {
            auto global_annos = M.getNamedGlobal("llvm.global.annotations");
            if (global_annos) {
                auto a = cast<ConstantArray>(global_annos->getOperand(0));
                for (int i = 0; i < a->getNumOperands(); i++) {
                    auto e = cast<ConstantStruct>(a->getOperand(i));
                    if (auto fn = dyn_cast<Function>(e->getOperand(0)->getOperand(0))) {
                        auto anno = cast<ConstantDataArray>(cast<GlobalVariable>(e->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
                        errs() << fn->getName() << ": " << anno << "\n";
                        fn->addFnAttr(anno);
                    }
                }
            }
        }
#if 0
        void getInterruptHandler(Module& module){
            for (Module::global_iterator global_begin = module.global_begin(), global_end = module.global_end(); global_begin!= global_end; ++global_begin){
                if(global_begin->getName() == "llvm.global.annotations"){
                    ConstantArray *CA = dyn_cast<ConstantArray>(global_begin->getOperand(0));
                    for(auto operator_begin = CA->op_begin(); operator_begin != CA->op_end(); ++operator_begin){
                        ConstantStruct *CS = dyn_cast<ConstantStruct>(operator_begin->get());
                        Function *FUNC = dyn_cast<Function>(CS->getOperand(0)->getOperand(0));
                        GlobalVariable *AnnotationGL = dyn_cast<GlobalVariable>(CS->getOperand(1)->getOperand(0));
                        StringRef annotation = dyn_cast<ConstantDataArray>(AnnotationGL->getInitializer())->getAsCString();
                        if(annotation.compare(AnnotationString)==0){
                            annotFuncs.insert(FUNC);
                            #if DEBUG_MODE
                                errs() << "Found interrupt handler: " << FUNC->getName()<<"\n";
                            #endif
                        } else if (annotation.compare("no_instrument") == 0) {
                            errs() << FUNC->getName() << ": no instrument\n";
                            noSanitize.insert(FUNC);
                        } else if (annotation.compare("malloc") == 0) {
                            errs() << FUNC->getName() << ": malloc\n";
                            mallocFuncs.insert(FUNC);
                        } else if (annotation.compare("free") == 0) {
                            errs() << FUNC->getName() << ": free\n";
                            freeFuncs.insert(FUNC);
                        } else {
                            // Nothing
                        }
                    }
                }
            }
        }
#endif
        void transformMain(Module& module){
            #if DEBUG_MODE
                errs() << "call transformMain()\n";
            #endif
            // Function* main_func = module.getFunction("main");
            // if (!main_func){
            //     #if DEBUG_MODE
            //         errs() << "no main func\n";
            //     #endif
            //     return;
            // }else{
            //     #if DEBUG_MODE
            //         errs() << "rename main to softboundcets_pseudo_main\n";
            //     #endif
            //     main_func->setName("softboundcets_pseudo_main");
            // }
        }
        void init(Module& module){
            #if DEBUG_MODE
                errs() << "Call init()\n";
                errs() << "init type\n";
                errs() << "IdCounter initial value: " << IdCounter << "\n";
            #endif
            log_module_name = "";
            log_global = "";
            module_extend_ctor_name = "";
            // init type
            VoidType = Type::getVoidTy(module.getContext());
            Int8Type = Type::getInt8Ty(module.getContext());
            Int16Type = Type::getInt16Ty(module.getContext());
            Int32Type = Type::getInt32Ty(module.getContext());
            VoidPtrType = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
            DL = &module.getDataLayout();
            PointerType* vptrty = dyn_cast<PointerType>(VoidPtrType);
            m_void_null_ptr = ConstantPointerNull::get(vptrty);
            // init global variable
            #if DEBUG_MODE
                errs() << "init global variables\n";
            #endif
            size_t inf_bound;
            ConstantInt* infinite_bound;
            infinite_bound = ConstantInt::get(Type::getInt32Ty(module.getContext()), inf_bound, false);
            m_infinite_bound_ptr = ConstantExpr::getIntToPtr(infinite_bound, VoidPtrType);
            m_null_value = ConstantInt::get(Int16Type, 0, false);
            m_infinite_value = ConstantInt::get(Int16Type, 1, false);
            // init function
            #if DEBUG_MODE
                errs() << "insert segger functions\n";
            #endif
            cb_fuzz_init = module.getOrInsertFunction("FuzzInit", FunctionType::get(VoidType, false));
            cb_fuzz_finish = module.getOrInsertFunction("FuzzFinish", FunctionType::get(VoidType, false));
           
            cb_irq_entry = module.getOrInsertFunction("__usan_trace_irq_entry", FunctionType::get(VoidType, false));
            segger_rtt_exit_irq = module.getOrInsertFunction("__usan_trace_irq_exit", FunctionType::get(VoidType, {Int16Type}, false));

            cb_bb_entry = module.getOrInsertFunction("__usan_trace_basicblock", FunctionType::get(VoidType, {Int16Type}, false));
            
            cb_func_entry = module.getOrInsertFunction("__usan_trace_func_entry", FunctionType::get(VoidType, {Int32Type}, false));
            cb_func_entry_stack = module.getOrInsertFunction("__usan_trace_func_entry_stack", FunctionType::get(VoidType, {Int32Type}, false));

            cb_func_entry_prop = module.getOrInsertFunction("__usan_trace_func_entry_prop", FunctionType::get(VoidType, {Int32Type}, false));
            cb_func_entry_stack_prop = module.getOrInsertFunction("__usan_trace_func_entry_stack_prop", FunctionType::get(VoidType, {Int32Type}, false));

            cb_prop = module.getOrInsertFunction("__usan_trace_prop", FunctionType::get(VoidType, {Int32Type, Int32Type}, false));
           
            cb_func_exit = module.getOrInsertFunction("__usan_trace_func_exit", FunctionType::get(VoidType, false));
            cb_func_exit_prop = module.getOrInsertFunction("__usan_trace_func_exit_prop", FunctionType::get(VoidType, {Int32Type}, false));

            /* SEGGER_RTT_Write_ArgProp(uint8_t args, ...) */
            cb_arg_prop = module.getOrInsertFunction("__usan_trace_callsite", FunctionType::get(VoidType, {Int8Type}, true));
            cb_ret_prop = module.getOrInsertFunction("__usan_trace_return_prop", FunctionType::get(VoidType,{Int32Type},false));

            cb_malloc = module.getOrInsertFunction("__usan_trace_malloc", FunctionType::get(VoidType, {Int32Type, Int32Type, Int32Type}, false));
            cb_free = module.getOrInsertFunction("__usan_trace_free", FunctionType::get(VoidType, {Int32Type}, false));
            cb_realloc = module.getOrInsertFunction("__usan_trace_realloc", FunctionType::get(VoidType, {Int32Type, Int32Type, Int32Type, Int32Type}, false));
            cb_load_store_check = module.getOrInsertFunction("__usan_trace_load_store", FunctionType::get(VoidType, {Int32Type, Int32Type, Int32Type}, false));
            #if TARGET_CONFIG==5
                closed_func["log_backend_activate"] = true;
                closed_func["log_backend_dropped"] = true;
                closed_func["log_backend_enable"] = true;
                closed_func["log_backend_id_set"] = true;
                closed_func["log_backend_is_active"] = true;
                closed_func["log_backend_panic"] = true;
                closed_func["log_backend_put"] = true;
                closed_func["log_backend_std_dropped"] = true;
                closed_func["log_backend_std_panic"] = true;
                closed_func["log_backend_std_put"] = true;
                closed_func["log_backend_uart_init"] = true;
                closed_func["log_core_init"] = true;
                closed_func["log_free"] = true;
                closed_func["log_init"] = true;
                closed_func["log_is_strdup"] = true;
                closed_func["log_list_head_get"] = true;
                closed_func["log_list_head_peek"] = true;
                closed_func["log_list_init"] = true;
                closed_func["log_msg_arg_get"] = true;
                closed_func["log_msg_get"] = true;
                closed_func["log_msg_hexdump_data_get"] = true;
                closed_func["log_msg_hexdump_data_op"] = true;
                closed_func["log_msg_is_std"] = true;
                closed_func["log_msg_level_get"] = true;
                closed_func["log_msg_nargs_get"] = true;
                closed_func["log_msg_pool_init"] = true;
                closed_func["log_msg_put"] = true;
                closed_func["log_msg_str_get"] = true;
                closed_func["log_msg_dropped_process"] = true;
                closed_func["log_output_flush"] = true;
                closed_func["log_output_msg_process"] = true;
                closed_func["log_output_timestamp_freq_set"] = true;
                closed_func["log_panic"] = true;
                closed_func["log_process"] = true;
                closed_func["log_process_thread_func"] = true;
                closed_func["log_process_thread_timer_expiry_fn"] = true;
                closed_func["log_source_name_get"] = true;
            #endif
            // prepare output file for static analysis
            #if 0
            log_module_name = module.getName().str();
            replace(log_module_name.begin(), log_module_name.end(), '/' ,'_');
            log_module_name += ".txt";
            log_module_name = STATIC_PASS_LOG_PATH + log_module_name;
            #if DEBUG_MODE
                errs() << "pass log path: " << log_module_name << "\n";
            #endif
            // prepare input and output file for global static data
            log_global = STATIC_PASS_LOG_PATH;
            log_global += "log_global.txt";
            #endif
        }
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            // // getLoopAnalysisUsage(AU);
            AU.addPreserved<MemorySSAWrapperPass>();

            // By definition, all loop passes need the LoopInfo analysis and the
            // Dominator tree it depends on. Because they all participate in the loop
            // pass manager, they must also preserve these.
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.addPreserved<DominatorTreeWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
            AU.addPreserved<LoopInfoWrapperPass>();

            AU.addRequired<AAResultsWrapperPass>();
            AU.addPreserved<AAResultsWrapperPass>();
            AU.addPreserved<BasicAAWrapperPass>();
            AU.addPreserved<GlobalsAAWrapperPass>();
            AU.addPreserved<SCEVAAWrapperPass>();

            AU.addRequired<ScalarEvolutionWrapperPass>();
            AU.addPreserved<ScalarEvolutionWrapperPass>();
            AU.addRequired<TargetLibraryInfoWrapperPass>();
        }

        bool doInitialization(Module &M) override {
            if (ModuleFilted(M))
                return false;

            init(M);
            annotateFunctions(M);

            std::fstream fin;
            fin.open(kUsanFunctionRules, std::ios::in);
            
            if (fin.is_open()) {
                std::string fn_name, fn_attr;
                while (!fin.eof()) {
                    fin >> fn_name >> fn_attr;
                    if (funcRules.find(fn_name) == funcRules.end()) {
                        std::set<std::string> rule_set{fn_attr};
                        funcRules.insert(std::pair<std::string, std::set<std::string>>(fn_name, rule_set));
                    } else {
                        funcRules[fn_name].insert(fn_attr);
                    }
                }

                for (auto &F : M) {
                    if (F.isDeclaration() && funcRules.find(F.getName().str()) != funcRules.end()) {
                        for (auto &a : funcRules[F.getName().str()])
                            F.addFnAttr(a);
                    }
                }
            }
            
            fin.close();

            return true;
        }
        
        bool doFinalization(Module &M) override{
            #if DEBUG_MODE
                errs() << "Call doFinalization\n";
            #endif
            if(ModuleFilted(M)){
                #if DEBUG_MODE
                    errs() << "Do not handle this module [" << M.getName() << "]\n";
                #endif
                return false;
            }
            // renameFunctions(M);
            #if DEBUG_MODE
                errs() << "==================Module After Pass begin====================\n";
                M.print(errs(), nullptr);
                errs() << "==================Module After Pass end====================\n";
            #endif
            return true;
        }
        bool runOnFunction(Function &F){
            #if DEBUG_MODE
                errs() << "Call runOnFunction\n";
            #endif
            has_stack_op = false;
            hasPtrArg = false;
            Module &module = *F.getParent();
            
            TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
            mCachedId.clear();
            mCachedAddressId.clear();
            mTempId = -1;
            
            #if DEBUG_MODE
                errs() << "Function [" << F.getName() << "] belongs to Module " << module.getName() << "\n";
            #endif
            // if(ModuleFilted(module)){
            //     #if DEBUG_MODE
            //         errs() << "Do not handle this module [" << module.getName() << "]\n";
            //     #endif
            //     return false;
            // }

            // if(!module_inited){
            //     module_inited = true;
            //     #if DEBUG_MODE
            //         errs() << "Handle this module\n";
            //         errs() << "==================Module begin====================\n";
            //         module.print(errs(), nullptr);
            //         errs() << "==================Module end====================\n";
            //     #endif
            //     init(module);
            //     // getInterruptHandler(module);
            //     annotateFunctions(module);
            //     // reBuildGlobalCtorInsertPoint(module);
            // }
            
            Function* func_ptr = &F;
            if(!checkIfFunctionOfInterest(func_ptr)) {
                #if DEBUG_MODE
                    errs() << "not interesting function\n";
                #endif
                return false;
            }

            std::vector<Instruction *> insts;
            std::deque<std::function<void()>> task_queue;

            #if RANDOM_ONLY_DEBUG==0
                #if DEBUG_MODE
                    errs() << "Handling this function:\n";
                    func_ptr->print(errs());
                    errs() << "\n";
                #endif
                ProcessedAllocas.clear();
                extendLifeTime(func_ptr);
                #if ENABLE_LOOP_OPTIMIZATION
                    LoopNotInstrumentMem.clear();
                    handleLoopMem();
                #endif
             
                for (auto &BB : F) {
                    #if ENABLE_LOOP_OPTIMIZATION
                        if(BBinLoopNotMem(&BB)){
                            #if DEBUG_MODE
                                errs() << "this bb should not be instrumented for mem\n";
                            #endif
                            continue;
                        }
                    #endif
                    for (auto &I : BB) {
                        insts.push_back(&I);  
                    }
                }

                preProcessing(func_ptr);
                gatherBaseBoundPass1(func_ptr, insts, task_queue);
                addDereferenceChecks(func_ptr, insts, task_queue);

                while (!task_queue.empty()) {
                    auto f = task_queue.front();
                    f();
                    task_queue.pop_front();
                }

                #if 0
                if (F.getName().equals(StringRef("main"))) {
                    Instruction* first_inst = dyn_cast<Instruction>(func_ptr->begin()->begin());
                    IRBuilder<> builder(first_inst);
                    auto M = F.getParent();
                    auto cb_fuzz_start = M->getOrInsertFunction("FuzzStart", VoidType);
                    builder.CreateCall(cb_fuzz_start);
                }
                #endif

            #endif
            #if ENABLE_LOOP_OPTIMIZATION
                LoopNotInstrumentTrans.clear();
                handleLoopTrans(func_ptr);
            #endif
            #if ENABLE_AFL
                insertRandom(func_ptr);
            #endif
            // #if RANDOM_ONLY_DEBUG==0
            //     if(isInterruptHandler(func_ptr)){
            //         instrumentInterruptHandler(func_ptr);
            //     }
            // #endif
            #if DEBUG_MODE
                errs() << "----------------------------------------------\n";
            #endif
            #if RANDOM_ONLY_DEBUG==0
                // Instruction* first_inst = dyn_cast<Instruction>(func_ptr->begin()->begin());
                auto I = func_ptr->begin()->getFirstInsertionPt();
                IRBuilder<> builder(&*I);
                // builder.SetInsertPoint(first_inst);
                auto func_id = ConstantExpr::getPtrToInt(func_ptr, Int32Type);

                NumInstrumentedFuncEntry++;
                
                if(has_stack_op){ // SP
                    // builder.CreateCall(cb_func_entry_stack, {func_id});
                    if (!hasPtrArg)
                        builder.CreateCall(cb_func_entry_stack, {func_id});
                    else
                        builder.CreateCall(cb_func_entry_stack_prop, {func_id});
                } else {
                    //  builder.CreateCall(cb_func_entry, {func_id});
                    if (!hasPtrArg)
                        builder.CreateCall(cb_func_entry, {func_id});
                    else
                        builder.CreateCall(cb_func_entry_prop, {func_id});
                }
                
                if(isInterruptHandler(func_ptr)){
                    instrumentInterruptHandler(func_ptr);
                }
                func_IdCounter++;
            #endif
            return true;
            
        }
    };
}

char IPEASan::ID = 0;
static RegisterPass<IPEASan> X("mcu", "MCU Pass");
static RegisterStandardPasses Y(
    // PassManagerBuilder::EP_OptimizerLast,
    PassManagerBuilder::EP_ModuleOptimizerEarly,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM) { PM.add(new IPEASan()); });

static RegisterStandardPasses Z(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM) { PM.add(new IPEASan()); });
