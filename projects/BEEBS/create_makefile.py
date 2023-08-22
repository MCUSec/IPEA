import argparse
import subprocess
import os

_EXCLUDED_DIR = {
    "sglib-arraysort",
    "trio",
    "trio-snprintf",
    "trio-sscanf",
    "ctl",
    "matmult"
}

_EXTRA_FLAGS = {
    "sglib-arrayheapsort": "-DHEAP_SORT",
    "sglib-arrayquicksort": "-DQUICK_SORT",
    "trio-snprintf": "-DTRIO_EXTENSION=0 -DTRIO_DEPRECATED=0 -DTRIO_MICROSOFT=0 -DTRIO_ERRORS=0 -DTRIO_FEATURE_FLOAT=0 -DTRIO_SNPRINTF -DTRIO_SNPRINTF_ONLY",
    "matmult-int": "-DMATMULT_INT",
    "trio-sscanf": "-DTRIO_SSCANF -DTRIO_EXTENSION=0 -DTRIO_DEPRECATED=0 -DTRIO_MICROSOFT=0 -DTRIO_ERRORS=0 -DTRIO_FEATURE_FLOAT=0 -DTRIO_FEATURE_FILE=0 -DTRIO_FEATURE_STDIO=0 -DTRIO_FEATURE_FD=0 -DTRIO_FEATURE_DYNAMICSTRING=0 -DTRIO_FEATURE_CLOSURE=0 -DTRIO_FEATURE_STRERR=0 -DTRIO_FEATURE_LOCALE=0 -DTRIO_EMBED_NAN=1 -DTRIO_EMBED_STRING=1",
    "ctl-stack": "-DCTL_STACK",
    "matmult-float": "-DMATMULT_FLOAT",
    "ctl-vector": "-DCTL_VECTOR",
}

def generate_makefile(name, board="frdmk64f", sanitizer="none"):
    content = []
    content.append(f"TARGET={name}")
    content.append("")
    content.append(f"BOARD_DIR = ../../boards/{board}/")
    content.append(f"SUPPORT_DIR = ../../support/")
    content.append("")
    content.append("SOURCES = $(wildcard *.c)")
    content.append("OBJECTS = $(SOURCES:.c=.o)")
    content.append("EXT = .elf")
    content.append("")
    content.append("")
    if name in _EXTRA_FLAGS:
        content.append(f"CFLAGS += {_EXTRA_FLAGS[name]}")
    if sanitizer == "ipea":
        content.append(f"include $(BOARD_DIR)board_usan.mk")
    elif sanitizer == "asan":
        content.append(f"include $(BOARD_DIR)board_asan.mk")
    else:
        content.append(f"include $(BOARD_DIR)board.mk")
    content.append(f"include $(SUPPORT_DIR)support.mk")
    content.append("")
    content.append(".PHONY: all clean")
    content.append("")
    content.append("all: $(TARGET)")
    content.append("")
    content.append("$(TARGET): $(OBJECTS) $(C_BOARD_OBJECTS) $(AS_BOARD_OBJECTS) $(C_SUPPORT_OBJECTS)")
    content.append("\t$(LD) $(OBJECTS) $(C_BOARD_OBJECTS) $(AS_BOARD_OBJECTS) $(C_SUPPORT_OBJECTS) $(LDFLAGS) -o $@$(EXT)")
    content.append("")
    content.append("$(OBJECTS): %.o: %.c")
    content.append("\t$(CC) $(CFLAGS) $(SAN) $(INCLUDES) -c $< -o $@")
    content.append("")
    content.append("clean:")
    content.append("\trm -f *.o $(TARGET)")
    return '\n'.join(content)
    

def main(args):
    with open("Makefile", "w") as fout:
        fout.write(".PHONY: all clean\n")
        fout.write("all:\n")
        g = os.walk("./src")
        for path, dir_list, _ in g:
            dir_list.sort()
            for dir in dir_list:
                if dir in _EXCLUDED_DIR:
                    continue
                filename = os.path.join(path, f"{dir}/Makefile")
                with open(filename, "w") as f:
                    f.write(generate_makefile(dir, board=args.board, sanitizer=args.sanitizer))
                fout.write(f"\tmake -C src/{dir} all\n")
        fout.write("\nclean:\n")
        fout.write("\tfind ./ -name \"*.elf\" -exec rm -f {} \;\n")
        fout.write("\tfind ./ -name \"*.o\" -exec rm -f {} \;\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="BEEBS")
    parser.add_argument("-b", "--board", type=str, required=True, help="Target board")
    parser.add_argument("-s", "--sanitizer", type=str, default='none', help="Sanitizer")
    args = parser.parse_args()
    main(args)
