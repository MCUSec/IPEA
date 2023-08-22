#!/usr/bin/env python

import argparse
import json
import re
import subprocess
import sys
from elftools.elf.elffile import ELFFile
# from cmsis_svd.parser import SVDParser


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
    return output_json


def parse(elf_path, sanitizer="usan"):
    with open(elf_path, "rb") as fin:
        elf = ELFFile(fin)
        if sanitizer == "usan":
            if elf.has_dwarf_info():
                dwarfinfo = elf.get_dwarf_info()
                output = parse_dwarf(dwarfinfo)
                with open("/tmp/mcu_sanitizer_dwarf.json", "w") as fout:
                    json.dump(output, fout)
            else:
                print('file has no DWARF info')
                sys.exit(-1)


def parse_svd(svd_path):
    parser = SVDParser.for_xml_file(svd_path)
    if not parser:
        return None

    output = list()

    for peripheral in parser.get_device().peripherals:
        p_dict = peripheral.to_dict()        
        output.append({
            "name": peripheral.name,
            "base": peripheral.base_address,
            "size": int(p_dict["address_block"]["size"]),
            "regs": [{"offset": reg["address_offset"], "size": reg["size"]} for reg in p_dict["registers"]]
        })
    
    return output


def run(target_path, *, timeout=0, input_path="", probe_conf="jlink.conf", svd_path="", skip_dl=False):
    if not target_path:
        print("failed to run target")
        sys.exit(-1)

    parse(target_path)

    arguments = [target_path, f"-c {probe_conf}"]

    if timeout > 0:
        arguments.append(f"-t {timeout}")

    if len(input_path) > 0:
        arguments.append(f"-i {input_path}")

    # if len(svd_path) > 0:
    #     peripheral_info = parse_svd(svd_path)
    #     if peripheral_info:
    #         with open("peripherals.json", "w") as fout:
    #             json.dump(peripheral_info, fout)
    #             arguments.append("-S peripherals.json")
    #     else:
    #         print(f"WARNING: failed to parse peripherals of the target MCU form {svd_path}")
    # else:
    #     print("WARNING: No SVD file specified")
        
    if skip_dl:
        arguments.append("-N")

    cmd_args = " ".join(arguments)

    cmd = f"taskset 0x1 ipea-unittest {cmd_args}"
    print(cmd)

    s = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    
    inpbuf = b''
    
    if not sys.stdin.isatty():
        try:
            with open(0, "rb") as f:
                while True:
                    buf = f.read()
                    if not buf:
                        break
                    inpbuf += buf
        except:
            pass
    
    s.stdin.write(inpbuf)
    s.stdin.close()
    sys.exit(s.wait())


def main(args):
    run(args.target_binary, timeout=args.timeout, input_path=args.input, probe_conf=args.probe_conf, skip_dl=args.skip_download)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="IPEA unittest")
    parser.add_argument("-b", "--target-binary", type=str, required=True, help="Target to be tested")
    parser.add_argument('-s', "--svd-path", type=str, default="", help="SVD file for the target MCU")
    parser.add_argument("-c", "--probe-conf", type=str, default="jlink.conf", help="J-Link configuration file")
    parser.add_argument("-i", "--input", type=str, default="", help="Input path")
    parser.add_argument("-t", "--timeout", type=int, default=1000, help="Timeout")
    parser.add_argument("-N", "--skip-download", action="store_true", help="Skip downloading the firmware to target device")
    args = parser.parse_args()
    main(args)
