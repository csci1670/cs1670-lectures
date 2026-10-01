# This file defines makefile targets for running QEMU

QEMU_OPTS  = -M raspi3b -smp 4
QEMU_OPTS += -bios kernel8.img
# Add a branch at address 0 to enter the bootloader (0x14020000 is b 0x80000).
# QEMU needs this because it doesn't run the armstub bootloader, but instead
# bakes its functionality into the machine simulation.
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

# Create a reminder message to display when starting GDB
# Some hosts report arm64 rather than aarch64;  both can use plain gdb.
GDB_CMD   = $(if $(filter aarch64 arm64,$(HOST_ARCH)),gdb,gdb-multiarch)
GDB_WHERE = $(if $(filter 1,$(IN_CONTAINER)), in your container,)
GDB_BANNER = printf '%s\n' \
	"** ***** Starting QEMU in GDB mode *****" \
	"** To continue, open a new terminal$(GDB_WHERE) and then" \
	"** run '$(GDB_CMD)' to get started!"


# Default QEMU target:  disable display mode and run directly in terminal
qemu: kernel8.img procs.gdb
	@$(if $(GDB_MODE),$(GDB_BANNER))
	$(QEMU) $(QEMU_OPTS) $(QEMU_OPTS_SERIAL) -nographic

qemu-verbose: kernel8.img procs.gdb
	$(QEMU) $(QEMU_OPTS) $(QEMU_OPTS_SERIAL) -nographic $(QEMU_OPTS_VERBOSE)

# Run with the framebuffer display enabled.
# This opens a display window showing the framebuffer
# Serial output still goes to stdio
qemu-fb: kernel8.img procs.gdb
	@$(if $(GDB_MODE),$(GDB_BANNER))
	$(QEMU) $(QEMU_OPTS) $(QEMU_OPTS_FB) $(QEMU_OPTS_SERIAL)

qemu-ws: kernel8.img procs.gdb
ifeq ($(CONSOLE),ws)
	@echo "***************************************************************"
	@echo "** Starting websocket connections; launch web view to start VM"
	@echo "** To exit, press Ctrl+C in this terminal"
	@echo "***************************************************************"
endif
	@$(if $(GDB_MODE),$(GDB_BANNER))
	$(QEMU) $(QEMU_OPTS) -vnc :99,websocket=$(WSADDR):$(VNCPORT) -audiodev none,id=id $(QEMU_OPTS_SERIAL)

# Run QEMU with GDB
qemu-gdb: QEMU_OPTS += $(QEMU_OPTS_GDB)
qemu-gdb: GDB_MODE := 1
qemu-gdb: qemu

qemu-fb-gdb: QEMU_OPTS += $(QEMU_OPTS_GDB)
qemu-fb-gdb: GDB_MODE := 1
qemu-fb-gdb: qemu-fb

qemu-ws-gdb: QEMU_OPTS += $(QEMU_OPTS_GDB)
qemu-ws-gdb: GDB_MODE := 1
qemu-ws-gdb: qemu-ws

# Stop all running qemu instances (to kill websocket version, or in case of issues)
stop kill:
	-killall -u $$(whoami) $(QEMU)
	@sleep 0.2; if ps -U $$(whoami) | grep $(QEMU) >/dev/null; then killall -9 -u $$(whoami) $(QEMU); fi
