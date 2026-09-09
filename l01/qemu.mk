# This file defines makefile targets for running QEMU

QEMU_OPTS  = -M raspi3b -smp 4
QEMU_OPTS += -bios kernel8.img
# Add a jump instruction at address 0x0 to enter the bootup code at 0x80000
QEMU_OPTS += -device loader,addr=0x0,data=0x14020000,data-len=4,cpu-num=0

# Default options
GDBPORT ?= 1234
VNCPORT ?= 5700
WSCPORT ?= 5838
CONSOLE ?= serial
WSADDR  ?= auto

QEMU_OPTS_VERBOSE = -d int
QEMU_OPTS_GDB = -S -gdb tcp::$(GDBPORT) $(QEMU_OPTS_VERBOSE)

QEMU_CONSOLE_STDIO = -serial mon:stdio
QEMU_CONSOLE_WS    = -serial websocket:$(WSADDR):$(WSCPORT),server=on,wait=on
QEMU_OPTS_SERIAL  := $(QEMU_CONSOLE_STDIO)
QEMU_OPTS_FB      :=

ifeq ($(HOST_OS),Darwin)
  QEMU_OPTS_FB += -display cocoa,zoom-to-fit=on
endif

ifeq ($(CONSOLE),ws)
  QEMU_OPTS_SERIAL := $(QEMU_CONSOLE_WS)
endif

ifeq ($(WSADDR),auto)
	ifeq ($(IN_CONTAINER),1)
		WSADDR := 0.0.0.0
	else
		WSADDR := localhost
	endif
endif

# Default QEMU target:  disable display mode and run directly in terminal
qemu: kernel8.img
	$(QEMU) $(QEMU_OPTS) $(QEMU_OPTS_SERIAL) -nographic

qemu-verbose: kernel8.img
	$(QEMU) $(QEMU_OPTS) $(QEMU_OPTS_SERIAL) -nographic $(QEMU_OPTS_VERBOSE)

# Run with the framebuffer display enabled.
# This opens a display window showing the framebuffer
# Serial output still goes to stdio
	qemu-fb: kernel8.img
	$(QEMU) $(QEMU_OPTS) $(QEMU_OPTS_FB) $(QEMU_OPTS_SERIAL)

	qemu-ws: kernel8.img
ifeq ($(CONSOLE),ws)
	@echo "***************************************************************"
	@echo "** Starting websocket connections; launch web view to start VM"
	@echo "** To exit, press Ctrl+C in this terminal"
	@echo "***************************************************************"
endif
	$(QEMU) $(QEMU_OPTS) -vnc :99,websocket=$(WSADDR):$(VNCPORT) -audiodev none,id=id $(QEMU_OPTS_SERIAL)

# Run QEMU with GDB
# Adds GDB options to each target before running
qemu-gdb: QEMU_OPTS += $(QEMU_OPTS_GDB)
qemu-gdb: qemu

qemu-fb-gdb: QEMU_OPTS += $(QEMU_OPTS_GDB)
qemu-fb-gdb: qemu-fb

qemu-ws-gdb: QEMU_OPTS += $(QEMU_OPTS_GDB)
qemu-ws-gdb: qemu-ws

# Stop all running qemu instances (to kill websocket version, or in case of issues)
stop kill:
	-killall -u $$(whoami) $(QEMU)
	@sleep 0.2; if ps -U $$(whoami) | grep $(QEMU) >/dev/null; then killall -9 -u $$(whoami) $(QEMU)
