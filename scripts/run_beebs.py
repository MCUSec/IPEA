#!/usr/bin/env python

import argparse
import json
import re
import stat
import subprocess
import sys
import os
import time

from elftools.elf.elffile import ELFFile
from elftools.dwarf.descriptions import describe_DWARF_expr
from elftools.dwarf.locationlists import LocationEntry, LocationExpr, LocationParser

_USAN_SYMBOLS = dict(
    RTT_CB_ADDR = "MCU_SANITIZER_SEGGER_RTT",
    RTT_COUNTER_ADDR = "rtt_total_bytes",
    RTT_TX_CRC = "rtt_tx_chksum",
    RTT_EN_ADDR = "__usan_trace_enabled",
    RTT_LOCK_ADDR = "__usan_rtt_lock"
)

_DEVICES = dict(
    mk64f12 = "MK64FN1M0xxx12",
    mk66f18 = "MK66FN2M0xxx18",
    stm32f446re = "STM32F446RE", # 777659861
    stm32h7 = "STM32H7B3VI" # 932000275
)

def decode_signed_leb128(raw_leb):
    value = 0
    for b in reversed(raw_leb):
        value = (value << 7) + (b & 0x7F)
    if raw_leb[-1] & 0x40:
        # negative -> sign extend
        value |= - (1 << (7 * len(raw_leb)))
    return value

def get_array_count(cu, base, arrdie):
    assert arrdie.tag == 'DW_TAG_array_type'
    count = 1
    refaddr = base + arrdie.size
    while True:
        subrange = cu.get_DIE_from_refaddr(refaddr)
        if not subrange.tag:
            break
        if 'DW_AT_count' in subrange.attributes:
            count = count * subrange.attributes['DW_AT_count'].value
        elif 'DW_AT_upper_bound' in subrange.attributes:
            count = count * (subrange.attributes['DW_AT_upper_bound'].value + 1)
        else:
            assert False
        refaddr += subrange.size
    return count


def get_type_size(cu, die):
    refaddr = die.attributes['DW_AT_type'].value + cu.cu_offset
    count = 1
    size = 4
    while True:
        dietype = cu.get_DIE_from_refaddr(refaddr)
        if dietype.tag == 'DW_TAG_array_type':
            count = get_array_count(cu, refaddr, dietype)
        if dietype.tag == 'DW_TAG_pointer_type':
            size = 4
            break
        if 'DW_AT_byte_size' in dietype.attributes:
            size = dietype.attributes["DW_AT_byte_size"].value
            break
        refaddr = dietype.attributes['DW_AT_type'].value + cu.cu_offset

    return size * count
    

def parse_dwarf(dwarfinfo):
    output_json = {
        'global_var': [],
        'func': {}
    }
    for CU in dwarfinfo.iter_CUs():
        for DIE in CU.iter_DIEs():
            if 'DW_AT_inline' in DIE.attributes:
                continue
            if DIE.tag == 'DW_TAG_subprogram':
                func_addr = DIE.attributes['DW_AT_low_pc'].value
                if func_addr == 0:
                    continue
                func_name = DIE.attributes['DW_AT_name'].value.decode('utf-8')
                if re.match(r'__usan_(.*)|SEGGER_RTT_(.*)|_WriteBlocking', func_name):
                    continue
                output_json['func'][func_addr | 1] = {'name': func_name, 'local_var': []}

    for CU in dwarfinfo.iter_CUs():
        for DIE in CU.iter_DIEs():
            if DIE.tag == 'DW_TAG_variable':
                parent = DIE.get_parent()
                while parent.tag not in ('DW_TAG_compile_unit', 'DW_TAG_subprogram'):
                    parent = parent.get_parent()
                
                if 'DW_AT_name' not in DIE.attributes or 'DW_AT_location' not in DIE.attributes:
                    continue
                var_name = DIE.attributes['DW_AT_name'].value.decode('utf-8')
                if re.match(r'__usan_(.*)|_acUpBuffer|rtt_total_bytes', var_name):
                    continue
                if parent.tag == 'DW_TAG_compile_unit':
                    # process global variable
                    assert 'DW_AT_type' in DIE.attributes
                    raw_addr = DIE.attributes['DW_AT_location'].value[1:]
                    addr = raw_addr[0] | (raw_addr[1] << 8) | (raw_addr[2] << 16) | (raw_addr[3] << 24)
                    if addr == 0:
                        continue
                    size = get_type_size(CU, DIE)                
                    print("global variable - name: %s, addr: %s, size: %d" % (var_name, hex(addr), size))
                    output_json['global_var'].append({
                        'var_name': var_name,
                        'addr': addr,
                        'byte_size': size  
                    })
                else:
                    func_addr = parent.attributes['DW_AT_low_pc'].value | 1
                    if func_addr not in output_json['func']:
                        continue
                    # process local variable
                    loc = DIE.attributes['DW_AT_location'].value
                    # if isinstance(loc, int):
                    #     continue
                    if loc[0] in (0x91, 0x7d):
                        # DW_OP_fbreg / DW_OP_breg13 / DW_OP_addr
                        offset = decode_signed_leb128(loc[1:])
                        size = get_type_size(CU, DIE)
                        output_json['func'][func_addr]['local_var'].append({
                            'var_name': var_name,
                            'offset': offset,
                            'byte_size': size
                        })
                    elif loc[0] == 0x03:
                        # handle static local variable as global variable
                        addr = int.from_bytes(loc[1:], byteorder='little')
                        if addr != 0:
                            output_json['global_var'].append({
                                'var_name': var_name,
                                'addr': addr,
                                'byte_size': get_type_size(CU, DIE)
                            })
                    
                    # print("function: %s@%s" % (parent.attributes['DW_AT_name'].value.decode('utf-8'), hex(parent.attributes['DW_AT_low_pc'].value)))
                    # print("local variable - name: %s, offset: %d, size: %d" % (var_name, offset, size))
    return output_json


# def parse(elf_path, sanitizer="usan"):
#     with open(elf_path, "rb") as fin:
#         elf = ELFFile(fin)
#         if sanitizer == "usan":
#             if elf.has_dwarf_info():
#                 dwarfinfo = elf.get_dwarf_info()
#                 output = parse_dwarf(dwarfinfo)
#                 with open("/tmp/mcu_sanitizer_dwarf.json", "w") as fout:
#                     json.dump(output, fout)
#             else:
#                 print('file has no DWARF info')
#                 sys.exit(-1)

#         symtab = elf.get_section_by_name('.symtab')
#         if not symtab:
#             print('No symbol table found')
#             sys.exit(-1)

#         target_info = dict()

#         if sanitizer == "usan":
#             for sym_name in _USAN_SYMBOLS:
#                 symbol = symtab.get_symbol_by_name(_USAN_SYMBOLS[sym_name])
#                 if symbol:
#                     target_info[sym_name] = symbol[0]['st_value']
#                     print(f"{_USAN_SYMBOLS[sym_name]} @ {hex(symbol[0]['st_value'])}")
#                 else:
#                     print(f"No symbol '{_USAN_SYMBOLS[sym_name]}' found")
#                     sys.exit(1)

#         s = subprocess.Popen(f"arm-none-eabi-objcopy -O binary {elf_path} {elf_path}.bin", shell=True)
#         s.wait()

#         target_info['bin_path'] = f"{elf_path}.bin"
        
#         return target_info
    

def parse(elf_path):
    with open(elf_path, "rb") as fin:
        elf = ELFFile(fin)
        if elf.has_dwarf_info():
            dwarfinfo = elf.get_dwarf_info()
            output = parse_dwarf(dwarfinfo)
            with open("/tmp/mcu_sanitizer_dwarf.json", "w") as fout:
                json.dump(output, fout)
        else:
            print('file has no DWARF info')
            sys.exit(-1)


def run(target_path, sanitizer, probe_conf="jlink.conf"):
    if not target_path:
        print("failed to run target")
        sys.exit(-1)

    if sanitizer == "usan":
        parse(target_path)

    cmd = f"taskset 0x1 ipea-unittest {target_path} -c {probe_conf} -t 1000"
    #print(cmd)

    s = subprocess.Popen(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    return s.wait()


# def run(target_info, device, serial, *, sanitizer="usan", entry_point="0x0"):
#     if not target_info:
#         print("failed to run target")
#         sys.exit(-1)

#     arguments = [device, str(serial), target_info['bin_path'], f"-l {hex(int(entry_point, base=16))}", "-t 70000"]

#     if sanitizer == "usan":
#         arguments.append(f"-T {hex(target_info['RTT_CB_ADDR'])},{hex(target_info['RTT_COUNTER_ADDR'])},{hex(target_info['RTT_TX_CRC'])},{hex(target_info['RTT_EN_ADDR'])},{hex(target_info['RTT_LOCK_ADDR'])}")
    
#     arguments.append(f"-s {sanitizer}")

#     cmd_args = " ".join(arguments)

#     cmd = f"taskset 0x1 ipea-unittest {cmd_args}"
#     print(cmd)

#     s = subprocess.Popen(cmd, shell=True)
#     sys.exit(s.wait())

def build_all(board, sanitizer='none'):
    tokens = ["python", f"create_makefile.py"]
    tokens.append(f"--board={board}")
    if sanitizer == 'ipea':
        tokens.append("--sanitizer=ipea")
    elif sanitizer == 'asan':
        tokens.append("--sanitizer=asan")

    cmd = ' '.join(tokens)
    s = subprocess.Popen(cmd, shell=True)
    if s.wait() != 0:
        print("failed to create per makefiles")
        sys.exit(1)
    subprocess.Popen("make clean", shell=True).wait()
    subprocess.Popen("make all", shell=True).wait()

def run_all(benchmark_path, probe_conf='jlink.conf', sanitizer='none', max_run=0):
    excluded = {"wikisort.elf", "mergesort.elf", "picojpeg.elf", "trio-snprintf.elf", "trio-sscanf.elf", "levenshtein.elf"}
    targets = list()
    output = list()
    walk_generator = os.walk(benchmark_path)
    for root_path, _, files in walk_generator:
        if len(files) < 1:
            continue
        for file in files:
            if file in excluded:
                continue
            _, suffix_name = os.path.splitext(file)
            if suffix_name != '.elf':
                continue
            targets.append({"benchmark": file, "path": os.path.join(root_path, file), "time": 0})
    targets.sort(key=lambda x: x["benchmark"])
    total = len(targets) if max_run == 0 else max_run
    done = 1
    for target in targets:
        print(f"[{done}/{total}] Testing {target['benchmark']}...")
        done += 1
        if sanitizer == "ipea":
            parse(target['path'])
        retry = 1
        while retry > 0:
            start = int(round(time.time() * 1000))
            exitcode = run(target['path'], probe_conf=probe_conf, sanitizer=sanitizer)
            end = int(round(time.time() * 1000))
            if exitcode in (0, 1, 2):
                output.append({"benchmark": target["benchmark"], "time": end - start})
                target['time'] = end - start
                print(f"Done. {target['time']} ms")
                break
            retry -= 1
            if retry == 0 and exitcode not in (0, 1):
                print("Error")
                with open(f"errors.txt", "a") as f:
                    f.write(f"{target['path']}, exitcode={exitcode}\n")
                break
            time.sleep(1)
        if done > total:
            break
    return output
    

def main(args):
    build_all(args.board, args.sanitizer)
    report = run_all(args.benchmark_path, args.probe_conf, args.sanitizer, args.max_run)
    with open(f"report_{args.sanitizer}.json", "w") as f:
        json.dump(report, f)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="BEEBS runner")
    parser.add_argument("-b", "--board", type=str, default="mk64f12", help="Target board")
    parser.add_argument("-p", "--benchmark-path", type=str, required=True, help="Path of BEEBS")
    parser.add_argument("-c", "--probe-conf", type=str, required=True, help="J-Link configuration file")
    parser.add_argument("-s", "--sanitizer", type=str, default="none", help="Sanitizer")
    parser.add_argument("-m", "--max-run", type=int, default=0, help="Max run")
    args = parser.parse_args()
    main(args)

