#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>

#include "target_info.h"

#define TARGET_SYMBOL_NAME(name) __target_p_##name

#define DEF_TARGET_SYMBOL_NAME(name, str) \
    static const char *TARGET_SYMBOL_NAME(name) = str;

DEF_TARGET_SYMBOL_NAME(RTTCB, "MCU_SANITIZER_SEGGER_RTT")
DEF_TARGET_SYMBOL_NAME(trace_rx_bytes, "rtt_total_bytes")
DEF_TARGET_SYMBOL_NAME(trace_rx_chksum, "rtt_tx_chksum")
DEF_TARGET_SYMBOL_NAME(trace_enabled, "__usan_trace_enabled")
DEF_TARGET_SYMBOL_NAME(trace_locked, "__usan_rtt_lock")
DEF_TARGET_SYMBOL_NAME(fuzz_inpbuf, "DeviceTestCaseBuffer")
DEF_TARGET_SYMBOL_NAME(fuzz_input_len, "TestCaseLen")
DEF_TARGET_SYMBOL_NAME(max_stack_usage, "__max_stack_usage")
DEF_TARGET_SYMBOL_NAME(max_heap_usage, "__max_heap_usage")

static bool elf2hex(const char *elf_path, target_info_t *target_info)
{
    char *objcopy_path = getenv("IPEA_OBJCOPY_PATH");
    pid_t pid;
    int r;

    if (!objcopy_path) {
        fprintf(stderr, "No `IPEA_OBJCOPY_PATH' environment variable defined\n");
        return false;
    }

    if (access(objcopy_path, F_OK)) {
        fprintf(stderr, "objcopy command not found\n");
        return false;
    }

    snprintf(target_info->hexfile, sizeof(target_info->hexfile) - 1, "%s.hex", elf_path);

    if ((pid = fork()) < 0) {
        fprintf(stderr, "failed to execute objcopy\n");
        return false;
    }
    
    if (pid == 0) {
        char *args[] = {
            strrchr(objcopy_path, '/') + 1,
            "-O",
            "ihex",
            (char *)elf_path,
            target_info->hexfile,
            NULL
        };

        if (execvp(objcopy_path, args) == -1) {
            fprintf(stderr, "failed to execute objcopy\n");
            exit(1);
        }
    }

    wait(&r);

    if (r != 0) {
        fprintf(stderr, "failed to generate hex file\n");
        return false;
    }

    return true;
}

bool parse_target(const char *filename, target_info_t *target_info)
{
    Elf *elf;
    Elf_Scn *scn = NULL;
    GElf_Ehdr ehdr;
    GElf_Shdr shdr;
    Elf_Data *data;
    int fd, i, count;
    bool r = true;

    elf_version(EV_CURRENT);

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        r = false;
        goto err_exit_1;
    }

    elf = elf_begin(fd, ELF_C_READ, NULL);
    if (!elf || elf_kind(elf) != ELF_K_ELF) {
        r = false;
        goto err_exit_2;
    }

    gelf_getehdr(elf, &ehdr);

    memset(target_info, 0, sizeof(*target_info));

    target_info->p_entry = (uint32_t)ehdr.e_entry;
    
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB) {
            break;
        }
    }

    data = elf_getdata(scn, NULL);
    count = shdr.sh_size / shdr.sh_entsize;

    for (i = 0; i < count; i++) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        const char *sym_name = elf_strptr(elf, shdr.sh_link, sym.st_name);
        const uint32_t sym_value = (uint32_t)sym.st_value;

        if (!strcmp(sym_name, TARGET_SYMBOL_NAME(RTTCB))) {
            target_info->p_RTTCB = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(trace_rx_bytes))) {
            target_info->p_trace_total_rx_bytes = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(trace_rx_chksum))) {
            target_info->p_trace_rx_chksum = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(trace_enabled))) {
            target_info->p_trace_enabled = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(trace_locked))) {
            target_info->p_trace_locked = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(fuzz_inpbuf))) {
            target_info->p_fuzz_inpbuf = sym_value;
            target_info->p_fuzz_inpbuf_size = (uint32_t)sym.st_size;
            printf("test case buffer length: %u\n", target_info->p_fuzz_inpbuf_size);
        }
        else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(fuzz_input_len))) {
            target_info->p_fuzz_input_len = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(max_stack_usage))) {
            target_info->p_max_stack_usage = sym_value;
        } else if (!strcmp(sym_name, TARGET_SYMBOL_NAME(max_heap_usage))) {
            target_info->p_max_heap_usage = sym_value;
        }
    }
    
    elf_end(elf);
    r = elf2hex(filename, target_info);

err_exit_2:
    if (fd != -1) 
        close(fd);

err_exit_1:
    return r;
}

