TARGET=Juliet1.3

# Bins
MAKE=make

# Support files
SUPPORT_PATH=testcasesupport/
INCLUDES=$(SUPPORT_PATH)
SUPPORT_SRCS=$(addprefix $(SUPPORT_PATH),main_linux.cpp io.c std_thread.c)
SUPPORT_OBJS=$(addsuffix .o,$(SUPPORT_SRCS))

# Partial files
MAKE_FILES=$(wildcard testcases/*/s*/Makefile) $(wildcard testcases/*/Makefile)
PARTIALS=$(patsubst %Makefile,%partial,$(MAKE_FILES))
INDIVIDUALS=$(patsubst %Makefile,%individuals,$(MAKE_FILES))

.PHONY: default individuals clean

default: individuals

$(PARTIALS): 
	$(MAKE) -C $(dir $@) $(notdir $@).o

individuals: $(INDIVIDUALS)

$(INDIVIDUALS):
	$(MAKE) -C $(dir $@) $(notdir $@)

clean:
	find . -name *.o -exec rm -f {} \;
	find . -name *.out -exec rm -f {} \;
	find . -name *.bin -exec rm -f {} \;
