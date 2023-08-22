TARGET := usb_host
TOP_DIR := $(shell pwd)
SAMPLE_HOME := $(IPEA_HOME)/fw_samples/
SRC_PATH := $(SAMPLE_HOME)frdmk64f-sdk_v2.11.0/boards/frdmk64f/usb_examples/usb_host_msd_command/bm/

.PHONY: default ipea clean

default: ipea

ipea:
	cd $(SRC_PATH)armclang; \
	./build_debug.sh; \
	cp debug/host_msd_command_bm.elf $(TOP_DIR)/$(TARGET)

asan:
	cd $(SRC_PATH)armgcc_asan; \
	./build_debug.sh; \
	cp debug/host_msd_command_bm.elf $(TOP_DIR)/$(TARGET)

clean:
	cd $(SRC_PATH)armclang; ./clean.sh
	rm -f $(TARGET) *.bin *.hex
