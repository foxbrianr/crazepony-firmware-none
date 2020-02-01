###############################################################################
# "THE BEER-WARE LICENSE" (Revision 42):
# <msmith@FreeBSD.ORG> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return
###############################################################################
#
# Makefile for building the cleanflight firmware.
#
# Invoke this with 'make help' to see the list of supported targets.
#

###############################################################################
# Things that the user might override on the commandline
#

# The target to build, see VALID_TARGETS below
TARGET ?= CRAZYPONYMINI

# Compile-time options
OPTIONS ?=

# Debugger optons, must be empty or GDB
DEBUG ?=GBD

# Serial port/Device for flashing
SERIAL_DEVICE	?= $(firstword $(wildcard /dev/ttyUSB*) no-port-found)

# Flash size (KB).  Some low-end chips actually have more flash than advertised, use this to override.
FLASH_SIZE ?= 64

###############################################################################
# Things that need to be maintained as the source changes
#

FORKNAME = crazepony_firmware

VALID_TARGETS	 = CRAZEPONYMINI 

REVISION = $(shell git log -1 --format="%h")

# Working directories
ROOT		 := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
SRC_DIR		 = $(ROOT)
OBJECT_DIR	 = $(ROOT)/obj/main
BIN_DIR		 = $(ROOT)/obj
INCLUDE_DIRS = $(SRC_DIR)
LINKER_DIR	 = $(ROOT)

# Search path for sources
VPATH		:= $(SRC_DIR):$(SRC_DIR)/Startup

CSOURCES    := $(shell find $(SRC_DIR) -name '*.c')

LD_SCRIPT	 = $(ROOT)/stm32_flash_f103_64k.ld

ARCH_FLAGS	 = -mthumb -mcpu=cortex-m3

TARGET_FLAGS = -D$(TARGET) -pedantic

DEVICE_FLAGS = -DSTM32F10X_MD -DSTM32F10X



ifneq ($(FLASH_SIZE),)
DEVICE_FLAGS := $(DEVICE_FLAGS) -DFLASH_SIZE=$(FLASH_SIZE)
endif

TARGET_DIR = $(ROOT)

TARGET_SRC = $(notdir $(wildcard $(TARGET_DIR)/*.c))


INCLUDE_DIRS := \
		    $(INCLUDE_DIRS) \
		    $(TARGET_DIR) \
		    StartUp \
		    HardWareDriver/H \
		    User_Src \
		    User_Src/H \
			Lib \
			Lib/inc \
			Control \
			Control/H
		    

VPATH		:= $(VPATH):$(TARGET_DIR)

TARGET_SRC := \
	StartUp/core_cm3.c              \
	StartUp/system_stm32f10x.c      \
	StartUp/startup_stm32f10x_md.s  \
	User_Src/main.c                 \
	User_Src/Altitude.c             \
	User_Src/AppInterface.c         \
	User_Src/CommApp.c              \
	User_Src/CommPc.c               \
	User_Src/ConfigTable.c          \
	User_Src/FailSafe.c             \
	User_Src/filter.c               \
	User_Src/IMU.c                  \
	User_Src/IMUSO3.c               \
	User_Src/stm32f10x_it.c         \
	User_Src/SysConfig.c            \
	User_Src/Sys_Fun.c              \
	Control/C/Control.c           \
	Control/C/ReceiveData.c       \
	HardWareDriver/C/Battery.c    \
	HardWareDriver/C/BT.c         \
	HardWareDriver/C/delay.c      \
	HardWareDriver/C/DMP.c        \
	HardWareDriver/C/drv_ms5611.c \
	HardWareDriver/C/HMC5883L.c   \
	HardWareDriver/C/IIC.c        \
	HardWareDriver/C/Led.c        \
	HardWareDriver/C/Moto.c       \
	HardWareDriver/C/MPU6050.c    \
	HardWareDriver/C/MS5611.c     \
	HardWareDriver/C/ms5611_I2C.c \
	HardWareDriver/C/NRF24L01.c   \
	HardWareDriver/C/SPI.c        \
	HardWareDriver/C/stmflash.c   \
	HardWareDriver/C/Tim.c        \
	HardWareDriver/C/UART1.c      \
	Lib/src/misc.c              \
	Lib/src/stm32f10x_gpio.c    \
	Lib/src/stm32f10x_i2c.c     \
	Lib/src/stm32f10x_pwr.c     \
	Lib/src/stm32f10x_rcc.c     \
	Lib/src/stm32f10x_spi.c     \
	Lib/src/stm32f10x_tim.c     \
	Lib/src/stm32f10x_usart.c   \

#	StartUp/startup_stm32f10x_md.s  \
#	Lib/src/stm32f10x_can.c     \
#	Lib/src/stm32f10x_cec.c     \
#	Lib/src/stm32f10x_crc.c     \

# Search path and source files
VPATH		:= $(VPATH):Startup/C
VPATH		:= $(VPATH):Lib/src
VPATH		:= $(VPATH):HardwareDriver/C
VPATH		:= $(VPATH):Control/C
VPATH		:= $(VPATH):User_Src/C

###############################################################################
# Things that might need changing to use different tools
#

# Tool names

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY		 = arm-none-eabi-objcopy
SIZE		 = arm-none-eabi-size

#
# Tool options.
#

ifeq ($(DEBUG),GDB)
OPTIMIZE	 = -O0
LTO_FLAGS	 = $(OPTIMIZE)
else
OPTIMIZE	 = -Os
LTO_FLAGS	 = -flto -fuse-linker-plugin $(OPTIMIZE)
endif

DEBUG_FLAGS	 = -ggdb3 -DDEBUG

CFLAGS  = $(ARCH_FLAGS) \
		   $(LTO_FLAGS) \
		   $(addprefix -D,$(OPTIONS)) \
		   $(addprefix -I,$(INCLUDE_DIRS)) \
		   $(DEBUG_FLAGS) \
		   -std=gnu99 \
		   -Wall -Wextra -Wunsafe-loop-optimizations  \
		   -ffunction-sections \
		   -fdata-sections \
		   $(DEVICE_FLAGS) \
		   -DUSE_STDPERIPH_DRIVER \
		   $(TARGET_FLAGS) \
		   -D'__FORKNAME__="$(FORKNAME)"' \
		   -D'__TARGET__="$(TARGET)"' \
		   -D'__REVISION__="$(REVISION)"' \
		   -save-temps=obj \
		   -MMD -MP

ASFLAGS		 = $(ARCH_FLAGS) \
		   -x assembler-with-cpp \
		   $(addprefix -I,$(INCLUDE_DIRS)) \
		  -MMD -MP

LDFLAGS		 = -lm \
		   -nostartfiles \
		   --specs=nano.specs \
		   -lc \
		   -lnosys \
		   $(ARCH_FLAGS) \
		   $(LTO_FLAGS) \
		   $(DEBUG_FLAGS) \
		   -static \
		   -Wl,-gc-sections,-Map,$(TARGET_MAP) \
		   -Wl,-L$(LINKER_DIR) \
           -Wl,--cref \
		   -T$(LD_SCRIPT)

###############################################################################
# No user-serviceable parts below
###############################################################################

CPPCHECK = cppcheck $(CSOURCES) --enable=all --platform=unix64 \
		   --std=c99 --inline-suppr --quiet --force \
		   $(addprefix -I,$(INCLUDE_DIRS)) \
		   -I/usr/include -I/usr/include/linux


TARGET_BIN	 = $(BIN_DIR)/$(FORKNAME)_$(TARGET).bin
TARGET_HEX	 = $(BIN_DIR)/$(FORKNAME)_$(TARGET).hex
TARGET_ELF	 = $(OBJECT_DIR)/$(FORKNAME)_$(TARGET).elf
TARGET_OBJS	 = $(addsuffix .o,$(addprefix $(OBJECT_DIR)/$(TARGET)/,$(basename $(TARGET_SRC))))
TARGET_DEPS	 = $(addsuffix .d,$(addprefix $(OBJECT_DIR)/$(TARGET)/,$(basename $(TARGET_SRC))))
TARGET_MAP	 = $(OBJECT_DIR)/$(FORKNAME)_$(TARGET).map


## Default make goal:
## hex         : Make filetype hex only
.DEFAULT_GOAL := hex

## Optional make goals:
## all         : Make all filetypes, binary and hex
all: hex bin

## bin         : Make binary filetype
## binary      : Make binary filtype
## hex         : Make hex filetype
bin:    $(TARGET_BIN)
binary: $(TARGET_BIN)
hex:    $(TARGET_HEX)

## rule to reinvoke make with TARGET= parameter
# rules that should be handled in toplevel Makefile, not dependent on TARGET
GLOBAL_GOALS	= all_targets cppcheck test

.PHONY: $(VALID_TARGETS)
$(VALID_TARGETS):
	$(MAKE) TARGET=$@ $(filter-out $(VALID_TARGETS) $(GLOBAL_GOALS), $(MAKECMDGOALS))

## rule to build all targets
.PHONY: all_targets
all_targets : $(VALID_TARGETS)


.PHONY: info
info: 
	@echo target=$(TARGET)
	@echo bin=$(TARGET_BIN)
	@echo src=$(basename $($(TARGET)_SRC))
	@echo obj=$(TARGET_OBJS)

## clean       : clean up all temporary / machine-generated files
clean:
	rm -f $(TARGET_BIN) $(TARGET_HEX) $(TARGET_ELF) $(TARGET_OBJS) $(TARGET_MAP)
	rm -rf $(OBJECT_DIR)/$(TARGET)
	cd src/test && $(MAKE) clean || true


## flash       : flash firmware (.bin) onto flight controller
#flash: flash_$(TARGET)

flash:
	./stm32loader.py -p /dev/ttyUSB0 -ewv $(TARGET_BIN)
	
	

## help        : print this help message and exit
help: Makefile
	@echo ""
	@echo "Makefile for the $(FORKNAME) firmware"
	@echo ""
	@echo "Usage:"
	@echo "        make [goal] [TARGET=<target>] [OPTIONS=\"<options>\"]"
	@echo ""
	@echo "Valid TARGET values are: $(VALID_TARGETS)"
	@echo ""
	@sed -n 's/^## //p' $<


# rebuild everything when makefile changes
$(TARGET_OBJS) : Makefile

# List of buildable ELF files and their object dependencies.
# It would be nice to compute these lists, but that seems to be just beyond make.

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex --set-start 0x8000000 $< $@

$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@

$(TARGET_ELF):  $(TARGET_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	$(SIZE) $(TARGET_ELF)

# Compile
$(OBJECT_DIR)/$(TARGET)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	$(CC) -c -o $@ $(CFLAGS) $<

# Assemble
$(OBJECT_DIR)/$(TARGET)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	$(CC) -c -o $@ $(ASFLAGS) $<

$(OBJECT_DIR)/$(TARGET)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	$(CC) -c -o $@ $(ASFLAGS) $<



# include auto-generated dependencies
-include $(TARGET_DEPS)
	