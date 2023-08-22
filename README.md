# Facilitating Non-Intrusive In-Vivo Firmware Testing with Stateless Instrumentation

## Introduction
Although numerous dynamic testing techniques have been developed, they can hardly be directly applied to firmware of deeply embedded (e.g., microcontroller-based) devices due to the tremendously different runtime environment and restricted resources on these devices. This work tackles these challenges by leveraging the unique position of microcontroller devices during firmware development. That is, firmware de- velopers have to rely on a powerful engineering workstation that connects to the target device to program and debug code. Therefore, we develop a decoupled firmware testing framework named IPEA, which shifts the overhead of resource-intensive analysis tasks from the microcontroller to the workstation. Only lightweight “needle probes” are left in the firmware to collect internal execution information without processing it. We also instantiated this framework with a sanitizer based on pointer capability (IPEA-San) and a feedback-guided fuzzer (IPEA-Fuzz). By comparing IPEA-San with a port of AddressSanitizer for microcontrollers, we show that IPEA-San reduces the memory overhead by 73.11% in real-world firmware with better detection accuracy. Running IPEA-Fuzz with IPEA-San, we found five new bugs in real IoT libraries and peripheral driver code.

For **NDSS'24 AE**, please refer to this [documentation](docs/AE.md).

## Directories
- ``AFL``: Source code of IPEA-Fuzz (based on AFL-2.5b)
- ``core``: Source code of IPEA-Core and IPEA-San
- ``compiler-plugins``: LLVM pass of IPEA-San instrumentation
- ``compiler-rt``: IPEA runtime library (for target MCU)
- ``fw_samples``: Source code of sample projects used in the evaluation
- ``include``: Header files
- ``projects``: Ported projects
    - ``MCU_ASAN``: Ported ASan for MCU
    - ``MCU_Juliet_Testsuite``: Ported Juliet Test Suite for 
    MCU
    - ``BEEBS``: BEEBS benchmark
- ``scripts``: Scripts that facilitate environment setup, unit test, fuzzing, experiments, etc.
- ``unitttest``: Unit test program for IPEA framework

## Environment

#### Hardware

- Debugger
    - SEGGER J-Link (Edu/Edu Mini/Onboard) or J-Trace

- Development boards (used in our experiments)
    - NXP FRDM-K64F 
    - STM32 Nucleo-F446RE
    - STM32H7B3I-EVAL

#### Software and libraries

- Ubuntu 22.04 LTS x86_64 (recommended)
- J-Link Software and Documentation pack
- J-Link Runtime Library
- Arm GNU Toolchain
- LLVM 13

## Getting Started

#### Install dependencies

1. Install J-Link Software and Documentation pack.
    - Download from https://www.segger.com/downloads/jlink/JLink_Linux_V758e_x86_64.deb and install:
        ```bash
        $ sudo dpkg -i JLink_Linux_V758e_x86_64.deb
        ```

2. Install J-Link Runtime Library
    - Download from https://www.segger.com/downloads/jlink/JLink_Linux_V758e_x86_64.tgz
    - Extract and copy ``libjlinkarm.so.7.58.5`` to ``/usr/lib`` directory:
        ````bash
        $ sudo cp /path/to/JLink_Linux_V758e_x86_64/libjlinkarm.so.7.58.5 /usr/lib
        $ sudo ldconfig
        $ sudo ln -s /usr/lib/jlinkarm.so.7 /usr/lib/jlinkarm.so
        ````

4. Install Arm GNU toolchain

    - Download the latest version of Arm GNU toolchain
package from the official [website](https://developer.arm.com/downloads/-/gnu-rm) and unpack it.

    - Add the unpacked toolchain path to the following enviroment variables:
        ```bash
        export ARMGCC_DIR=/path/to/arm-toolchain
        export PATH=${ARMGCC_DIR}/bin:$PATH
        ```
   

5. Install `spdlog`
    ```bashrc
    $ git clone https://github.com/gabime/spdlog.git
    $ mkdir -p spdlog/build
    $ cd spdlog/build && cmake ..
    $ make && sudo make install
    ```

6. Install LLVM-13 and other dependencies

    ```bash
    $ sudo apt install cmake python3-pip clang-13 llvm-13-dev libjsoncpp-dev libconfig-dev libelf-dev
    $ pip install pyelftools cmsis-svd 
    ```

#### Build IPEA framework

1. Clone source code of IPEA framework to your work directory and add the path to `IPEA_HOME` environment variable:
    ```bash
    export IPEA_HOME=/path/to/IPEA  # IPEA work directory
    ```
2. Build IPEA framework simply by:
    ```bash
    $ cd ${IPEA_HOME}
    $ ./build.sh
    ```

3. Lastly, add the following paths to `PATH` environment variable:
    ```bash
    export PATH=${IPEA_HOME}/build/AFL:${IPEA_HOME}/build/unittest:${IPEA_HOME}/scripts:$PATH
    ```

<!-- #### Run the target with IPEA-San

Once the firmware has been compiled with IPEA-San, the firmware can be executed with ``ipea-unittest`` program. It is responsible for:
- Downloading the firmware to the target MCU
- Boostrapping the MCU
- Receiving and analyzing the runtime trace of the firmware

Command line usage:

```Bash
$ run_unittest.py -b /path/to/firmware.elf -c /path/to/jlink.conf [-t <timeout>]
```

Arguments:

- `-b`: specify the path of firmware (ELF formant)
- `-c`: specify the configuration file path of J-Link and target MCU
- `-t`: specify the timeout in milliseconds (default is 1000 ms) -->


## Usage

#### Taming the firmware source code

This step needs the full knowledge of the firmware behavior. Here, we illustrate the generic procedure of taming the target source code.

1. Define the following global variables in the source code
    ```C
    /* used for receiving testcase */
    unsigned char DeviceTestCaseBuffer[2000] __attribute__((section(".noinit"))); 
    unsigned int TestCaseLen __attribute__((section(".noinit")));
    ```

2. Insert the following code in the fuzz start point
    ```C
    #include "fuzz.h"
    ...
    FuzzStart();
    ```
3. Insert the following code in the fuzz stop point
    ```C
    #include "fuzz.h"
    ...
    FuzzFinish();
    ```
4. Insert the following code in ``HardFault_Handler``
    ```C
    #include "fuzz.h"
    ...
    FuzzAbort();
    ```
5. Find the place in which receiving the input and add the following code. IPEA-Fuzz will inject test cases to `DeviceTestCaseBuffer`.
    ```C
    extern unsigned char DeviceTestCaseBuffer[];
    extern unsigned int TestCaseLen;
    ...
    // replace input_buf with the name of which used in the source code
    memcpy(input_buf, DeviceTestCaseBuffer, TestCaseLen);
    ```
6. Annotate functions that no need to be sanitized, such as system initialization (e.g., ``SystemInit()``) that invoked by the reset handler with ``__attribute__((annotate("no_instrument")))``.
7. Annotate interrupt handlers with `__attribute__((annotate("interruptHandler")))`.

#### Compile the firmware with IPEA-San

- Add following flags to ``Makefile`` and compile the firmware:
    ```lang-makefile
    CC = clang
    LD = arm-none-eabi-gcc
    ...
    CFLAGS += --target=arm-none-eabi
    CFLAGS += --sysroot=$(ARMGCC_DIR)/arm-none-eabi
    CFLAGS += -flegacy-pass-manager -Xclang -load -Xclang $(IPEA_HOME)/build/compiler-plugins/uSan/usan.so
    ...
    LDFLAGS += -L$(IPEA_HOME)/compiler-rt/build -lipea-rt
    ...
    ```

#### Hardware wiring

- For NXP FRDM-K64F, connect J-Link or J-Trace to Pin J9 with 10-pin cable.
- For STM32 Nucleo-F446RE, only onboard J-Link is avaliable since no debug port is present. To make ST-LINK compatible with J-Link onboard, please refer to this [link](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/).
- For STM32H7B3I-EVAL, connect J-Link or J-Trace to Pin CN15 with 20-pin cable.

#### Start fuzzing

IPEA-Fuzz needs to collect target information from the ELF-format file and pass them to the fuzzer. Please use ``run_afl.py`` to run `ipea-fuzz`. 

```bash
$ run_afl.py -b /path/to/firmware.elf -c /path/to/jlink.conf -i /path/to/input -t <timeout>
```

Arguments:

- `-b`: specify the ELF-format firmware
- `-c`: specify the configuration file path of J-Link and target MCU
- `-i`: specify the path of inputs
- `-t`: specify the timeout in milliseconds

The fuzzing result will be saved in `output` directory.

#### Test a usecase

Please use `ipea-unittest` to run the firmware with a specific input. Command line usage:
```bash
$ cat output/crashes/<use_case_name> | run_unittest.py -b /path/to/firmware.elf -c /path/to/jlink.conf -t <timeout>
```

Arguments:

- `-b`: specify the path of firmware (ELF formant)
- `-c`: specify the configuration file path of J-Link and target MCU
- `-t`: specify the timeout in milliseconds (default is 1000 ms)

The runtime log would be saved as `tracelog_0.txt`. If a crash dected, the call stack information would be saved as `callstack.txt`.
