FLASH_MODE = qio
FLASH_SPEED = 80
FLASH_SIZE = 4096
#FLASH_SIZE = 1024
STANDALONE = 1
FLASH_APP1 = 1
FLASH_APP2 = 0
APP1_ADDR = 0x002000
#APP1_ADDR = 0x002000
APP2_ADDR = 0x102000
#APP2_ADDR = 0x082000
CA_ADDR = 0x3FA000
#CA_ADDR = 0xFA000
SDK_ADDR = 0x3FC000
#SDK_ADDR = 0xFC000
ESP_SDK_IDATA_BIN = /opt/esp-open-sdk/sdk/bin/esp_init_data_default.bin
ESP_BAUD = 921600
ESP_PORT = /dev/ttyUSB0

#######################################################

# Directories
TARGETDIR = bin
SOURCEDIR = src
OBJECTDIR = bin/obj
INCLUDEDIR = include

STRUCT := $(shell find $(SOURCEDIR) -type d)

SOURCEDIRSTRUCT := $(filter-out %/include, $(STRUCT))
INCLUDEDIRSTRUCT := $(filter %/include, $(STRUCT))
OBJECTDIRSTRUCT := $(subst $(SOURCEDIR), $(OBJECTDIR), $(SOURCEDIRSTRUCT))

# Build type
BUILD_TYPE ?= release

# Version
$(shell if ! test -f $(TARGETDIR)/.version; then echo 0 > $(TARGETDIR)/.version; fi)

INC_VERSION ?= n
BUILD_VERSION = $(shell cat $(TARGETDIR)/.version)

ifeq ($(INC_VERSION), y)
$(shell echo $$(($(BUILD_VERSION) + 1)) > $(TARGETDIR)/.version)
$(shell rm -f $(OBJECTDIR)/*)
BUILD_VERSION = $(shell cat $(TARGETDIR)/.version)
endif

ifeq ($(BUILD_VERSION), 0)
$(shell echo $$(($(BUILD_VERSION) + 1)) > $(TARGETDIR)/.version)
$(shell rm -f $(OBJECTDIR)/*)
BUILD_VERSION = $(shell cat $(TARGETDIR)/.version)
endif

# Compillers & Linker
CC = xtensa-lx106-elf-gcc
CXX = xtensa-lx106-elf-g++
LD = xtensa-lx106-elf-gcc
AS = xtensa-lx106-elf-as
STRIP = xtensa-lx106-elf-strip
OBJCOPY = xtensa-lx106-elf-objcopy
OBJDUMP = xtensa-lx106-elf-objdump
GDB = xtensa-lx106-elf-gdb

# Compillers & Linker flags
CFLAGS = $(addprefix -I,$(INCLUDEDIRSTRUCT)) -Os -std=gnu99 -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -fno-builtin-printf -D__ets__ -DICACHE_FLASH -DBUILD_VERSION=$(BUILD_VERSION)
CXXFLAGS = $(addprefix -I,$(INCLUDEDIRSTRUCT)) -fno-rtti -fno-exceptions -Os -std=c++17 -Wpointer-arith -Wundef -Werror -Wl,-EL -nostdlib -mlongcalls -mtext-section-literals -fno-builtin-printf -D__ets__ -DICACHE_FLASH -DBUILD_VERSION=$(BUILD_VERSION)
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,--gc-sections
LDLIBS = -lmain -lnet80211 -lhal -lcrypto -lwpa -lwpa2 -llwip -lpp -lphy -lc -lmbedtls -lgcc


ifeq ($(BUILD_TYPE), debug)
CFLAGS += -g
CXXFLAGS += -g
endif

## Linker scripts
LDSCRIPT_STANDALONE = ld/eagle.app.v6.standalone.ld
LDSCRIPT_APP1 = ld/eagle.app.v6.$(FLASH_SIZE).app1.ld
LDSCRIPT_APP2 = ld/eagle.app.v6.$(FLASH_SIZE).app2.ld

# Target
TARGET = $(TARGETDIR)/v$(BUILD_VERSION)

# Sources & objects
SRCFILES := $(addsuffix /*, $(SOURCEDIRSTRUCT))
SRCFILES := $(wildcard $(SRCFILES))

CSOURCES := $(filter %.c, $(SRCFILES))
COBJECTS := $(subst $(SOURCEDIR), $(OBJECTDIR), $(CSOURCES:%.c=%.o))

CXXSOURCES := $(filter %.cpp, $(SRCFILES))
CXXOBJECTS := $(subst $(SOURCEDIR), $(OBJECTDIR), $(CXXSOURCES:%.cpp=%.o))

SOURCES = $(CSOURCES) $(CXXSOURCES)
OBJECTS = $(COBJECTS) $(CXXOBJECTS)

# Flash binaries and locations
ESPTOOL_BINS = $(SDK_ADDR)
ESPTOOL_BINS += $(ESP_SDK_IDATA_BIN)

ifeq ($(STANDALONE), 1)
	ESPTOOL_BINS += 0x00000
	ESPTOOL_BINS += $(TARGET).iram1.bin
	ESPTOOL_BINS += 0x10000
	ESPTOOL_BINS += $(TARGET).irom0.bin
else
	ifeq ($(FLASH_APP1), 1)
		ESPTOOL_BINS += $(APP1_ADDR)
		ESPTOOL_BINS += $(TARGET).app1.bin
	endif
	ifeq ($(FLASH_APP2), 1)
		ESPTOOL_BINS += $(APP2_ADDR)
		ESPTOOL_BINS += $(TARGET).app2.bin
	endif
endif


all: clean-bin make-dir version $(TARGET).lss $(TARGET).bin

$(TARGET).lss: $(TARGET).elf
ifeq ($(STANDALONE), 1)
	@echo Creating LSS file \'$@\'...
	@$(OBJDUMP) -S --disassemble $< > $@
else
	@echo Creating LSS file \'$(TARGET).app1.lss\'...
	@$(OBJDUMP) -S --disassemble $(TARGET).app1.elf > $(TARGET).app1.lss
	@echo Creating LSS file \'$(TARGET).app2.lss\'...
	@$(OBJDUMP) -S --disassemble $(TARGET).app2.elf > $(TARGET).app2.lss
endif

$(TARGET).bin: $(TARGET).elf
ifeq ($(STANDALONE), 1)
	@echo ---------------------------------------------------------------------------
	@echo Creating BIN file \'$(TARGET).iram1.bin\'...
	@esptool2 -bin -$(FLASH_SIZE) -$(FLASH_SPEED) -$(FLASH_MODE) -boot0 $^ $(TARGET).iram1.bin .text .data .rodata
	@echo Creating BIN file \'$(TARGET).irom0.bin\'...
	@esptool2 -lib $^ $(TARGET).irom0.bin
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_STANDALONE) -d $^
else
	@echo Creating BIN file \'$(TARGET).app1.bin\'...
	@esptool2 -bin -$(FLASH_SIZE) -$(FLASH_SPEED) -$(FLASH_MODE) -boot2 -iromchksum $(TARGET).app1.elf $(TARGET).app1.bin .text .data .rodata
	@echo Creating BIN file \'$(TARGET).app2.bin\'...
	@esptool2 -bin -$(FLASH_SIZE) -$(FLASH_SPEED) -$(FLASH_MODE) -boot2 -iromchksum $(TARGET).app2.elf $(TARGET).app2.bin .text .data .rodata
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_APP1) -d $(TARGET).app1.elf
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_APP2) -d $(TARGET).app2.elf
endif

$(TARGET).elf: $(OBJECTS)
	@echo ---------------------------------------------------------------------------
ifeq ($(STANDALONE), 1)
	@echo Creating ELF file \'$@\'...
	@$(LD) -o $@ $^ -T $(LDSCRIPT_STANDALONE) $(LDFLAGS) -Wl,-static -Wl,--start-group $(LDLIBS) -Wl,--end-group -Wl,-Map=$(TARGET).map
ifeq ($(BUILD_TYPE), release)
	@$(STRIP) -g $@
endif
else
	@echo Creating ELF file \'$(TARGET).app1.elf\'...
	@$(LD) -o $(TARGET).app1.elf $^ -T $(LDSCRIPT_APP1) $(LDFLAGS) -Wl,-static -Wl,--start-group $(LDLIBS) -Wl,--end-group -Wl,-Map=$(TARGET).app1.map
	@echo Creating ELF file \'$(TARGET).app2.elf\'...
	@$(LD) -o $(TARGET).app2.elf $^ -T $(LDSCRIPT_APP2) $(LDFLAGS) -Wl,-static -Wl,--start-group $(LDLIBS) -Wl,--end-group -Wl,-Map=$(TARGET).app2.map
ifeq ($(BUILD_TYPE), release)
	@$(STRIP) -g $(TARGET).app1.elf
	@$(STRIP) -g $(TARGET).app2.elf
endif
endif

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c
	@echo Compilling C file \'$<\' \> \'$@\'...
	@$(CC) $(CFLAGS) -MD -c -o $@ $<

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.cpp
	@echo Compilling C++ file \'$<\' \> \'$@\'...
	@$(CXX) $(CXXFLAGS) -MD -c -o $@ $<

flash: clean-bin make-dir version $(TARGET).lss $(TARGET).bin
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m $(ESPTOOL_BINS)

flash-ca:
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m $(CA_ADDR) esp_ca_cert.bin

inc-version:
	@echo $$(($(BUILD_VERSION) + 1)) > $(TARGETDIR)/.version

dec-version:
	@echo $$(($(BUILD_VERSION) - 1)) > $(TARGETDIR)/.version

version:
	@echo Build version: v$(BUILD_VERSION)
	@echo ---------------------------------------------------------------------------

make-dir:
	@mkdir -p $(OBJECTDIRSTRUCT)

clean-bin:
	@rm -f $(TARGETDIR)/*.lss
	@rm -f $(TARGETDIR)/*.hex
	@rm -f $(TARGETDIR)/*.bin
	@rm -f $(TARGETDIR)/*.elf

clean: clean-bin
	@rm -rf $(OBJECTDIR)/*

-include $(OBJECTS:.o=.d)
