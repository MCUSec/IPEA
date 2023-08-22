#!/usr/bin/env python

import argparse
import json
import re
import subprocess
import sys
import os
import time

from elftools.elf.elffile import ELFFile

false_negatives = 0
false_positives = 0
report = dict()

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
    return output_json


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
    # s = subprocess.Popen(cmd, shell=True)
    
    return s.wait()


def build(testsuite_path, board, use_asan, run_bad=True):
    tokens = ["python", f"create_per_cwe_files.py"]
    tokens.append(f"--target={board}")
    if use_asan:
        tokens.append("--use-asan")
    
    if run_bad:
        tokens.append("--omit-good")
    else:
        tokens.append("--omit-bad")

    cmd = ' '.join(tokens)
    s = subprocess.Popen(cmd, shell=True)
    if s.wait() != 0:
        print("failed to create per cwe makefiles")
        sys.exit(1)

    print("Building %s testcases. This process would take a long time..." % ("BAD" if run_bad else "GOOD"))

    s = subprocess.Popen(f"make -C {testsuite_path} clean", stdout=subprocess.DEVNULL, shell=True)
    if s.wait() != 0:
        print("Failed to clean testsuites")
        sys.exit(1)

    #n_threads = max(int(os.cpu_count() / 2), 1)

    s = subprocess.Popen(f"make -C {testsuite_path} individuals", stdout=subprocess.DEVNULL, shell=True)
    if s.wait() != 0:
        print("failed to build testsuites")
        sys.exit(1)


def run_all(testsuite_path, probe_conf_path, use_asan=False, run_bad=True, max_run=0):
    global false_negatives, false_positives, report

    targets = list()
    walk_generator = os.walk(testsuite_path)
    for root_path, _, files in walk_generator:
        if len(files) < 1:
            continue
        for file in files:
            _, suffix_name = os.path.splitext(file)
            if suffix_name == '.out':
                matches = re.findall(r'CWE(\d+)', root_path)
                assert matches, "No CWE number found or confusing"
                cwe = matches[0]
                if cwe not in report:
                    report[cwe] = {
                        "total_bad": 0,
                        "total_good": 0,
                        "false_negatives": 0,
                        "false_positives": 0,
                        "fault": 0
                    }
                targets.append({"cwe": cwe, "path": os.path.join(root_path, file)})
    
    total = len(targets) if max_run == 0 else max_run
    done = 1
    sanitizer = "asan" if use_asan else "usan"
    for target in targets:
        print(f"[{done}/{total}] Testing {target['path']}...")
        done += 1
        retry = 3
        exitcode = 0
        while retry > 0:
            exitcode = run(target['path'], sanitizer, probe_conf_path)
            if exitcode in (0, 1):
                print("Done")
                break
            retry -= 1
            if retry == 0 and exitcode not in (0, 1):
                print(f"Error (exitcode={exitcode})")
                prefix = "bad" if run_bad else "good"
                with open(f"{prefix}_errors.txt", "a") as f:
                    f.write(f"{target['path']}, exitcode={exitcode}\n")
                break
            time.sleep(1)
        if run_bad:
            report[target["cwe"]]["total_bad"] += 1
            if exitcode == 0:
                with open("false_negatives.txt", "a") as f:
                    f.write(f"{target['path']}\n")
                report[target["cwe"]]["false_negatives"] += 1
                false_negatives += 1
        else:
            report[target["cwe"]]["total_good"] += 1
            if exitcode == 1:
                with open("false_positives.txt", "a") as f:
                    f.write(f"{target['path']}\n")
                report[target["cwe"]]["false_positives"] += 1
                false_positives += 1

        if done > max_run:
            break
    

def main(args):
    if args.run == "all":
        if args.skip_build:
            print("warning: '--skip-build option is ignored in 'run all' mode")
        build(args.testsuite_path, args.device, args.use_asan, run_bad=True)
        run_all(args.testsuite_path, args.probe_conf , args.use_asan, run_bad=True, max_run=args.max_run)
        print("Testing BAD cases is finished")
        time.sleep(1)
        build(args.testsuite_path, args.device, args.use_asan, run_bad=False)
        run_all(args.testsuite_path, args.probe_conf, args.use_asan, run_bad=False, max_run=args.max_run)
        print("Testing GOOD cases is finished")
        time.sleep(1)
    elif args.run == "bad":
        if not args.skip_build:
            build(args.testsuite_path, args.device, args.use_asan, run_bad=True)
        run_all(args.testsuite_path, args.probe_conf , args.use_asan, run_bad=True, max_run=args.max_run)
        print("Testing BAD cases is finished")
        time.sleep(1)
    elif args.run == "good":
        if not args.skip_build:
            build(args.testsuite_path, args.device, args.use_asan, run_bad=False)
        run_all(args.testsuite_path, args.probe_conf, args.use_asan, run_bad=False, max_run=args.max_run)
        print("Testing GOOD cases is finished")
        time.sleep(1)
    else:
        print(f"Invalid parameter for 'run' option: {args.run}. Expected: run | good | all")
        sys.exit(-1)

    postfix = "ipea" if not args.use_asan else "asan"
    with open(f"report_{postfix}.json", "w") as f:
        json.dump(report, f)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Juliet C/C++ Testsuite")
    parser.add_argument("-p", "--testsuite-path", type=str, required=True, help="Path of juliet testsuite")
    parser.add_argument("-c", "--probe-conf", type=str, required=True, help="J-Link configuration file")
    parser.add_argument("-d", "--device", type=str, default="mk64f12", help="Target device")
    parser.add_argument("--use-asan", action="store_true", help="Use google address sanitizer")
    parser.add_argument("--run", type=str, default="all", help="Run which case? (bad | good | all), default is all")
    parser.add_argument("--skip-build", action="store_true", default=False, help="Skip build")
    parser.add_argument("--max-run", type=int, default=0, help="Maximum number of testcases to be tested. Default is all")
    args = parser.parse_args()
    main(args)

