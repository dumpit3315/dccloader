##############################################################################################
# Start of default section
#

# Tools
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc 
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary
OBJDUMP = $(TRGT)objdump
PYTHON = python

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# Define project name here
PROJECT = dumpnow

# Auto LD script stuff
LOADER_LOAD_START = 0x00000000
LOADER_LOAD_SIZE_KB = 512

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# CPU type
MCU = arm7tdmi

# Target platform
PLATFORM = default

# Storage devices for the target platform
LOADER_DEVICES = default

# Define optimisation level here
OPT = -O2

# Dependencies
DEVICES = flash/mmap/mmap.c
CONTROLLERS = 
ADD_DEPS = 

# Optional dependencies
ifeq ($(LZO), 1)
ADD_DEPS += minilzo/minilzo.c
DDEFS += -DHAVE_MINILZO=1
endif

ifeq ($(LZ4), 1)
ADD_DEPS += lz4/lz4_fs.c
DDEFS += -DHAVE_LZ4=1
endif

# Storage devices
ifeq ($(CFI), 1)
DEVICES += flash/cfi/cfi.c
endif

ifdef NAND_CONTROLLER
DEVICES += flash/nand/nand.c
CONTROLLERS += flash/nand/controller/$(NAND_CONTROLLER).c
endif

ifdef ONENAND_CONTROLLER
DEVICES += flash/onenand/onenand.c
CONTROLLERS += flash/onenand/controller/$(ONENAND_CONTROLLER).c
endif

ifdef SUPERAND_CONTROLLER
DEVICES += flash/superand/superand.c
CONTROLLERS += flash/superand/controller/$(SUPERAND_CONTROLLER).c
endif

# Loader configuration
ifeq ($(MCU), xscale)
DDEFS += -DCPU_XSCALE
DADEFS += -DCPU_XSCALE
endif

ifeq ($(ICACHE), 1)
DADEFS += -DUSE_ICACHE=1
endif

ifeq ($(BP_LOADER), 1)
DADEFS += -DUSE_BREAKPOINTS=1
DDEFS += -DUSE_BREAKPOINTS=1
endif

ifdef BUFFER_SIZE
DDEFS += -DDCC_BUFFER_SIZE=${BUFFER_SIZE}
else
DDEFS += -DDCC_BUFFER_SIZE=0x4000
endif

ifeq ($(NO_COMPRESS), 1)
DDEFS += -DDISABLE_COMPRESS=1
endif

ifeq ($(PIC), 1)
DADEFS += -DUSE_PIC
ADD_DEPS += dcc/pic.c
endif

# List C source files here
SRC = main.c dcc/memory.c dcc/dn_dcc_proto.c dcc/bitutils.c dcc/lwprintf.c plat/$(PLATFORM).c devices/$(LOADER_DEVICES).c $(DEVICES) $(CONTROLLERS) $(ADD_DEPS)

# List ASM source files here
ASRC = crt.s

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################################

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
DEFS    = $(DDEFS) $(UDEFS) -DCDEFS="\"FLAGS=$(DDEFS) $(UDEFS) CPU=$(MCU) PLATFORM=$(PLATFORM) LOADER_DEVICES=$(LOADER_DEVICES) DEVICES=$(DEVICES) CONTROLLERS=$(CONTROLLERS)\""
ADEFS   = $(DADEFS) $(UADEFS) -DADEFS="\"FLAGS=$(DADEFS) $(UADEFS) CPU=$(MCU)\""
OBJS    = $(ASRC:%.s=.out/%.o) $(SRC:%.c=.out/%.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)
CC_PIC 	= 
AS_PIC 	= 

ifeq ($(BIG_ENDIAN), 1)
MCFLAGS += -mbig-endian -mbe32
endif

ifeq ($(PIC), 1)
CC_PIC += -fPIC -mpic-register=r9 -mpic-data-is-text-relative -msingle-pic-base -fPIE
AS_PIC += -fPIC -mpic-register=r9 -mpic-data-is-text-relative -msingle-pic-base -fPIE -pie
endif

ASFLAGS = $(MCFLAGS) $(CC_PIC) -g -gdwarf-2 -Wa,-amhls=$(<:.s=.lst) $(ADEFS) -c
#CPFLAGS = $(MCFLAGS) $(CC_PIC) -I . $(OPT) -g -gdwarf-2 -mthumb-interwork -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS) -c
CPFLAGS = $(MCFLAGS) $(CC_PIC) -I . $(OPT) -g -gdwarf-2 -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS) -c
LDFLAGS = $(MCFLAGS) $(AS_PIC) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=build/$(PROJECT).map,--cref,--no-warn-mismatch $(LIBDIR)


# Generate dependency information
#CPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

ifeq ($(PLATFORM), default)
$(warning Building without platform specific routines, specify PLATFORM to change that)
endif

ifndef LDSCRIPT
LDSCRIPT = linker/linker.ld
all: $(LDSCRIPT) $(OBJS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin $(PROJECT).lst
else
all: $(OBJS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin $(PROJECT).lst
endif

%.ld : %.ld.tl
	$(info Generating linker script with offset: $(LOADER_LOAD_START); size: $(LOADER_LOAD_SIZE_KB)k)
	$(PYTHON) "utils/substitute.py" $< $@ $(LOADER_LOAD_START) $(LOADER_LOAD_SIZE_KB)

.out/%.o : %.c
	@mkdir -p $(@D)
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

.out/%.o : %.s
	@mkdir -p $(@D)
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o build/$@

%hex: %elf
	$(HEX) build/$< build/$@

%bin: %elf
	$(BIN) build/$< build/$@

%.lst: %.elf
	$(OBJDUMP) -h -S build/$< > build/$@

clean:
	-rm -f $(OBJS)
	-rm -f build/$(PROJECT).elf
	-rm -f build/$(PROJECT).map
	-rm -f build/$(PROJECT).hex
	-rm -f build/$(PROJECT).bin
	-rm -f build/$(PROJECT).lst
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

help:
ifeq ($(OS),Windows_NT)
	@echo > NUL
else
	@echo > /dev/null
endif
	$(info Dumpnow DCC Loader)
	$(info Optional libraries:)
	$(info $(NULL)  LZO=1 = Enable LZO Compression)
	$(info $(NULL)  LZ4=1 = Enable LZ4 Compression)
	$(info Target configuration:)
	$(info $(NULL)  PLATFORM=(name) Select chipset platform)
	$(info $(NULL)  MCU=(MCU) = Select CPU architecture)
	$(info $(NULL)  ICACHE=1 = Use instruction cache (ARM9 and later))
	$(info $(NULL)  BP_LOADER=1 = If the chipset have broken DCC Support, compiling as Breakpoint-based loader might help)
	$(info $(NULL)  BUFFER_SIZE=(Buffer Size) = DCC Buffer Size (Default: 0x4000))
	$(info $(NULL)  PROJECT=(name) = Output name)
	$(info $(NULL)  LDSCRIPT=(ld) = Linker script)
	$(info $(NULL)  NO_COMPRESS=1 = Disable RLE compression, used if using with RIFF says failed to unpack received data.)
	$(info $(NULL)  BIG_ENDIAN=1 = Big endian format)
	$(info $(NULL)  PIC=1 = Use PIC code)
	$(info $(NULL)  LOADER_LOAD_START=(LOAD_OFFSET_IN_HEX) = With LDSCRIPT unset, set loader start offset)
	$(info $(NULL)  LOADER_LOAD_SIZE_KB=(LOAD_SIZE_IN_KB) = With LDSCRIPT unset, set RAM size for the loader)
	$(info Flash devices:)
	$(info $(NULL)  CFI=1 = Enable CFI interface)
	$(info $(NULL)  NAND_CONTROLLER=(name) = Enable NAND controller)
	$(info $(NULL)  ONENAND_CONTROLLER=(name) = Enable OneNAND controller)
	$(info $(NULL)  SUPERAND_CONTROLLER=(name) = Enable SuperAND controller)
	$(info $(NULL)  LOADER_DEVICES=(name) = Select which memory combination to use)

#
# Include the dependency files, should be the last of the makefile
#
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
