.PHONY: clean format

HOST_OS = $(shell uname -s)
TOOL_LDFLAGS :=

ifndef IN_CONTAINER
ifneq ("$(wildcard /etc/image-version)","")
    IN_CONTAINER = 1
else
    IN_CONTAINER = 0
endif
endif

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
ifeq ($(HOST_OS), Darwin)
# MacOS
TOOLPREFIX = aarch64-elf-
TOOL_LDFLAGS += -Wl,-pie,--no-dynamic-linker
else
# Linux/Windows
ifneq ($(shell uname -m), aarch64)
TOOLPREFIX = aarch64-linux-gnu-
endif
endif
endif


GCC = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
PYTHON ?= python3

MAKEDIR = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
IMGNAME = weenix-ubuntu24
CONTNAME = weenix-container

QEMU ?= qemu-system-aarch64

CFLAGS = -Wall -Wextra -Werror -g -std=gnu2x
# Tell compiler to avoid automatically linking the C standard library and Linux/MacOS startup files
CFLAGS += -nostdlib -nostartfiles -ffreestanding
# Prevent compiler from using floating point and SIMD registers in kernel code
CFLAGS += -mgeneral-regs-only

# Modify the number of debug prints
DEBUG_LVL ?= 0
ifeq ($(DEBUG_LVL),0)
    CFLAGS += -DDEBUG_LEVEL=0
else ifeq ($(DEBUG_LVL),1)
    CFLAGS += -DDEBUG_LEVEL=1
else ifeq ($(DEBUG_LVL),2)
    CFLAGS += -DDEBUG_LEVEL=2
else
    $(error Invalid DEBUG_LEVEL value: $(DEBUG_LEVEL). Must be 0, 1 or 2)
endif


ASMFLAGS = -g


K = kernel
U = user
ULIB = user/lib

# Find all the source files for user code
USRCS  := $(wildcard $(U)/*.c)
UELFS  := $(USRCS:.c=.elf)
UOBJS  := $(patsubst $(U)/%.elf,$(K)/%_kproc.o,$(UELFS))

ULIBSRCS := $(wildcard $(ULIB)/*.c)
ULIBOBJS := $(ULIBSRCS:.c=.o)
UHDRS    := $(wildcard $(U)/*.h) $(wildcard $(ULIB)/*.h)

# Find all the source files for kernel code
KSRCS  := $(wildcard $(K)/*.c $(K)/drivers/*.c)
KASM   := $(wildcard $(K)/*.S)
KOBJS  := $(KASM:%.S=%.o) $(KSRCS:%.c=%.o)

DEPSDIR = .deps
DEPS := $(shell find $(DEPSDIR) -name '*.d' 2>/dev/null)

all: kernel8.img

clean:
	rm -rf kernel8.img
	rm -rf $(K)/*.elf
	find $(K) -name *.o -delete
	rm -rf $(DEPSDIR)
	rm -f $(UOBJS)
	rm -f $(U)/u_klib.h

format:
	find . -name *.[c,h] | xargs clang-format -i

ifneq ($(DEPS),)
include $(DEPS)
endif

###################
# kernel files
###################

$(K)/%.o: $(K)/%.c
	mkdir -p $(@D) $(DEPSDIR)/$(K)/$(*D)
	$(GCC) $(CFLAGS) -MMD -MF $(DEPSDIR)/$(K)/$*.d -Ikernel -c $< -o $@

$(K)/%.o: $(K)/%.S
	mkdir -p $(DEPSDIR)/$(K)/$(*D)
	$(GCC) $(ASMFLAGS) -MMD -MF $(DEPSDIR)/$(K)/$*.d -c $< -o $@

kernel8.img: $(KOBJS) $(UOBJS)
	$(LD) -T kernel/linker.ld -o $(K)/kernel8.elf $(KOBJS) $(UOBJS)
	$(OBJCOPY) $(K)/kernel8.elf -O binary kernel8.img

###################
# user programs
###################

# Generate trampoline header file
$(U)/u_klib.h:
	$(PYTHON) $(U)/gen_klib_header.py > $@

# Compile user library files into .o files
$(ULIB)/%.o: $(ULIB)/%.c $(UHDRS) $(U)/u_klib.h $(BUILDSTAMP)
	mkdir -p $(DEPSDIR)/$(ULIB)
	$(GCC) $(CFLAGS) -I$(U) -I$(ULIB) -MMD -MF $(DEPSDIR)/$(ULIB)/$*.d -c $< -o $@

# Compile user programs into .elf files (ELF executables)
# -e main: set entry point to 'main'
# -I$(U) -I$(ULIB): include user/ and user/lib directories for headers
# -static: disable dynamic linking
# -static-pie: create a position-independent executable
# -Wl passes options to the linker, specifically:
#  -nmagic: avoid page alignment of segments in ELF executables (ARM64 requires 64kB alignment, which wastes a lot of space)
#  -T $(U)/procs.ld: use custom linker script to define memory layout
$(U)/%.elf: $(U)/%.c $(ULIBOBJS) $(U)/u_klib.h
	$(GCC) $(CFLAGS) -e main -I$(U) -I$(ULIB) -static -static-pie $(TOOL_LDFLAGS) -Wl,-nmagic,-T,$(U)/procs.ld -o $@ $^

# Turn ELF executable into an object file, so that we can link it into the kernel executable.
$(K)/%_kproc.o: $(U)/%.elf
	$(OBJCOPY) -I binary $< -O elf64-littleaarch64 -B aarch64 $@ --rename-section .data=.elf_executables,alloc,load,readonly,data,contents


###################
# QEMU
###################

include qemu.mk


