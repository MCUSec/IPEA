#!/bin/bash

CURRENT_DIR=`pwd`

echo '=== Building IPEA framework ==='
mkdir -p build && cd build
cmake .. && make
cd ${CURRENT_DIR}

echo '=== Building IPEA compiler runtime ==='
mkdir -p compiler-rt/build && cd compiler-rt/build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/armgcc.cmake .. && make
cd ${CURRENT_DIR}

echo '=== Building MCU AddressSanitizer ==='
cd projects/MCU_ASAN
make
cd ${CURRENT_DIR}
