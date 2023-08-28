# Artifact Evaluation

We claim "**Avaliable**" and "**Functional**" badges in NDSS'24 AE.

This manual includes instructions of how to evaluate the following IPEA components:

- `ipea-unittest`: a unit testing program of **IPEA-San**. It is responsible for verifying the capability of capturing memory-related errors in the target firmware. This program will download the instrumented firmware to the target MCU and run it. An error will be returned if any memory error detected.

- `ipea-fuzz`: the main program of **IPEA-Fuzz**, an AFL-based, coverage-guided fuzzer. Like fuzzing x86_64 applications on a PC, it generates test cases and feeds to the target MCU and receives code coverage feedbacks via the debug dongle (i.e., a J-Link Probe). 


## Getting Started

IPEA framework requires special hardware (e.g., J-Link Probe and evaluation board). Therefore, we provide SSH access to our server in which all software and hardware dependencies are in place for the evaluation.

1. SSH into the remote server (for Linux OS)

    ```bash
    $ ssh -X ndss24@24.199.78.229         # password: ndss24
    $ ssh -X -p 2222 ndss24_ae@localhost
    ```

    **NOTE**: X11 forwarding must be enabled. Please don't omit `-X` parameter in the command lines. For Windows and Mac users, please refer to this [instruction](x11.md).

2. Build IPEA framework simply by:

    ```bash
    $ cd NDSS24_AE/IPEA
    $ ./build.sh  # Everything will be built!
    ```

3. Build the `Toy` firmware. The source code will be instrumented by the IPEA-San compiler plugin.

    ```bash
    $ cd fw_samples/Toy
    $ make
    ```

## IPEA-San Evaluation

`Toy` program accepts arbitrary strings. The input starting with the specific character will trigger different memory bugs:

- '`a`': Stack buffer overflow
- '`e`': Heap buffer overflow
- '`i`': Global buffer overflow
- '`o`': Use after free
- '`u`': Double free
- '`x`': Null pointer dereference
- '`y`': Peripheral-based buffer overflow
- '`z`': Sub-object buffer overflow

For example, run `Toy` with an input string 'axxxx' (this input will trigger stack buffer overflow):

```bash
$ echo 'axxxx' | run_unittest.py -b toy -t 1000
```

**NOTE**: When running `ipea-unittest` (and `ipea-fuzz`), a pop-up window would appear to indicate the progress of firmware downloading.
If using a J-Link Edu/Edu mini/OB, you will be asked to agree the agreement of usage. 

Arguments:

- `-b`: the path of firmware
- `-t`: timeout in milliseconds

The output would look like:

```bash
global variable - name: of_global, addr: 0x1fff02b8, size: 16
global variable - name: DeviceTestCaseBuffer, addr: 0x1fff0098, size: 512
global variable - name: TestCaseLen, addr: 0x1fff0298, size: 4
global variable - name: s_uartHandle, addr: 0x1fff02c8, size: 24
global variable - name: s_uartIsr, addr: 0x1fff02e0, size: 4
taskset 0x1 ipea-unittest toy -c jlink.conf -t 1000
test case buffer length: 512
Target crashed
```

The detailed execution log can be found from `tracelog_0.txt`:

```bash
$ cat tracelog_0.txt

[2023-07-15 02:41:04.555] [init] [info] Assigned tag for global variable 'of_global': 0xabcd,  address: 0x1fff02b8, length: 16
[2023-07-15 02:41:04.555] [init] [info] Assigned tag for global variable 'DeviceTestCaseBuffer': 0xabce,  address: 0x1fff0098, length: 512
[2023-07-15 02:41:04.555] [init] [info] Assigned tag for global variable 'TestCaseLen': 0xabcf,  address: 0x1fff0298, length: 4
[2023-07-15 02:41:04.555] [init] [info] Assigned tag for global variable 's_uartHandle': 0xabd0,  address: 0x1fff02c8, length: 24
[2023-07-15 02:41:04.555] [init] [info] Assigned tag for global variable 's_uartIsr': 0xabd1,  address: 0x1fff02e0, length: 4
[2023-07-15 02:41:04.555] [init] [info] Assigned tag for global variable 'heap_end': 0xabd2,  address: 0x1fff02e4, length: 4
[2023-07-15 02:41:04.555] [IPEA_RunTarget] [info] RTT initialized
[2023-07-15 02:41:04.557] [IPEA_RunTarget] [info] Reach fuzz start point
[2023-07-15 02:41:04.558] [IPEA_RunTarget] [info] Written testcase: 5 bytes
[2023-07-15 02:41:04.559] [IPEA_RunTarget] [info] Target is running
[2023-07-15 02:41:04.560] [IPEA_RunTarget] [info] Terminated. Execution time: 2 ms
[2023-07-15 02:41:04.562] [RTT_Decode] [debug] Total trace size: 69 bytes
[2023-07-15 02:41:04.562] [Subroutine] [debug] Assigned tag 0x55e99 for local variable 'of_stack' @ 0x2002ffe8
[2023-07-15 02:41:04.562] [Subroutine] [debug] Assigned tag 0x55ea1 for local variable 'of_heap' @ 0x2002ffe4
[2023-07-15 02:41:04.562] [Subroutine] [debug] Assigned tag 0x55ea9 for local variable 'flag' @ 0x2002ffe3
[2023-07-15 02:41:04.562] [handleFuncEntryStack] [debug] Enter function main, id = 0x6e9, stack_base = 0x2002fff8, stack_top = 0x2002ff88
[2023-07-15 02:41:04.562] [handleProp] [debug] Pointer propagation: from 0x0 to 0x2002ffe4 (tag = 0x0)
[2023-07-15 02:41:04.562] [handleRandom] [debug] Basic block random number: -16234 ( 0xc096 )
[2023-07-15 02:41:04.562] [handleMalloc] [debug] Allocated a heap object (size = 16) @ 0x20000008
[2023-07-15 02:41:04.562] [handleProp] [debug] Pointer propagation: from 0xffffffff to 0x2002ffe4 (tag = 0x55eb2)
[2023-07-15 02:41:04.562] [handleRandom] [debug] Basic block random number: 6249 ( 0x1869 )
[2023-07-15 02:41:04.562] [handleRandom] [debug] Basic block random number: 26920 ( 0x6928 )
[2023-07-15 02:41:04.562] [handleCheck] [info] Checking pointer dereference: pointer_id = 0x6002ffe8, address = 0x2002ffe8, length = 17
[2023-07-15 02:41:04.562] [handleCheck] [info] Stack buffer overflow detected @ 0x2002fff8, expected tag: 0x55e99, real tag: 0x0
[2023-07-15 02:41:04.562] [IPEA_RunTarget] [info] Trace analysis result: 1
```


## IPEA-Fuzz Evaluation

By fuzz-testing `Toy` firmware, all the memory errors listed aboved should be captured (i.e., eight unique crashes). To start the fuzzing, run `ipea-fuzz` by:

```bash
$ run_afl.py -b toy -i ./fuzz_input -t 1000
```

Arguments:

- `-b`: the path of firmware
- `-i`: the path of initial seeds
- `-t`: timeout in milliseconds

Then, an AFL UI will appear on the terminal, press `Ctrl`+`C` to exit. The fuzzing results will be saved to `output` directory, please test a usecase with `ipea-unittest` by:

```bash
$ cat output/crashes/<use_case_name> | run_unittest.py -b toy -t 1000
```
The execution log can be found from `tracelog_0.txt`. If a crash detected, the call stack information will be saved to `callstack.txt`.

## Correctness Evaluation

Juliet C/C++ Testsuite is used for evaluating the correctness of sanitizers. 

- Enter the directory of Juliet project
    ```bash
    $ cd ${IPEA_HOME}/projects/MCU_Juliet_Testsuite
    ```
- Evaluate the correctness of IPEA-San
  
  **NOTE**: This experiment may take a long time. `--max-run=10` makes the script run only 10 programs.

    ```bash
    $ run_julite.py -p . -c mk64f.conf --max-run=10   # Test the correctness of IPEA-San
    ```

- Evaluate the correctness of ASan

  **NOTE**: This experiment may take a long time. `--max-run=10` makes the script run only 10 programs. 

    ```bash
    $ run_juliet.py -p . -c mk64f.conf --max-run=10 --uss-asan    # Test the correctness of ASan
    ```
The result (i.e., number of FPs and FNs in each CWE) will be saved as `report_ipea.json` and `report_asan.json` for IPEA-San and ASan respectively.

## Performance Evaluation

[BEEBS](https://github.com/mageec/beebs) is a set of benchmarks for measuring the performance of embedded systems.

- Enter the directory of BEEBS project
  ```bash
  $ cd ${IPEA_HOME}/projects/BEEBS
  ```

- Run BEEBS without sanitizers (baseline):
  
  ```bash
  $ run_beebs.py -p . -c mk64f.conf
  ```
  
- Run BEEBS with IPEA-San:

  ```bash
  $ run_beebs.py -p . -c mk64f.conf -s ipea
  ```
- Run BEEBS with ASan:
  
  ```bash
  $ run_beebs.py -p . -c mk64f.conf -s asan  
  ```

The result (the time-consumption of each program) will be saved as `report_none.json`, `report_ipea.json` and `report_asan.json` for baseline, IPEA-San and ASan respectively. Performance overhead of each sanitizer can be obtained by comparing the corresponding time-consumption with baseline.

## Fuzz-testing FRDM-K64F USB-Host Driver

This experiment needs to take over 24 hours.

```bash
$ cd ${IPEA_HOME}/fw_samples/USB-Host
$ make ipea
$ run_afl.py -b usb_host -i ./fuzz_input -t 3000
```
A use-after-free bug would be found. The fuzzing result can be found from `output` directory, please test a crash usecase with `ipea-unittest` tool as described in [IPEA-Fuzz Evaluation](#IPEA-Fuzz-Evaluation).