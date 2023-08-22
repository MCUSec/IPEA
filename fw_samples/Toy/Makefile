TARGET := toy
TOP_DIR := $(shell pwd)
SAMPLE_HOME := $(IPEA_HOME)/fw_samples/
SRC_PATH := $(SAMPLE_HOME)frdmk64f-sdk_v2.11.0/boards/frdmk64f/demo_apps/hello_world/

.PHONY: default ipea clean

default: ipea

ipea:
	cd $(SRC_PATH)armclang; \
	./build_debug.sh; \
	cp debug/hello_world.elf $(TOP_DIR)/$(TARGET)

asan:
	cd $(SRC_PATH)armgcc_asan; \
	./build_debug.sh; \
	cp debug/hello_world.elf $(TOP_DIR)/$(TARGET)

clean:
	cd $(SRC_PATH)armclang; ./clean.sh
	rm -f $(TARGET) *.bin *.hex
