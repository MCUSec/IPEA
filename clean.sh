#!/bin/bash

rm -rf build
rm -rf compiler-rt/build
cd projects/MCU_ASAN && make clean
cd -
