FLASH_MODE = dout
FLASH_SPEED = 80
#FLASH_SIZE = 4096
ESP_BAUD = 115200
#ESP_PORT = /dev/ttyS6
FLASH_SIZE = 1024
#ESP_BAUD = 115200
ESP_PORT = /dev/ttyS7

#######################################################

# Directories
TARGETDIR = bin
SOURCEDIR = src
OBJECTDIR = bin/obj

# Compillers & Linker
CC = xtensa-lx106-elf-gcc
CXX = xtensa-lx106-elf-g++
LD = xtensa-lx106-elf-gcc

# Include directories
INCLUDEDIRS = $(SOURCEDIR)/ $(SOURCEDIR)/include

# Libraries
LIBS =

# Compillers & Linker flags
CFLAGS = $(addprefix -I,$(INCLUDEDIRS)) -Os -std=c99 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -fno-builtin-printf
CXXFLAGS = $(CCFLAGS) -fno-rtti -fno-exceptions
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -Wl,--start-group $(addprefix -l,$(LIBS)) -Wl,--end-group

# Linker scripts
LDSCRIPT_BOOT = ld/eagle.app.v6.boot.ld
LDSCRIPT_LOADER = ld/eagle.app.v6.loader.ld

# Target
TARGET_BOOT = $(TARGETDIR)/boot
TARGET_LOADER = $(TARGETDIR)/loader

# Sources & objects
CSOURCES_BOOT = $(wildcard $(SOURCEDIR)/boot/*.c)
COBJECTS_BOOT = $(patsubst $(SOURCEDIR)/boot/%.c, $(OBJECTDIR)/boot/%.o, $(CSOURCES_BOOT))
CXXSOURCES_BOOT = $(wildcard $(SOURCEDIR)/boot/*.cpp)
CXXOBJECTS_BOOT = $(patsubst $(SOURCEDIR)/boot/%.cpp, $(OBJECTDIR)/boot/%.o, $(CXXSOURCES_BOOT))
SOURCES_BOOT = $(CSOURCES_BOOT) $(CXXSOURCES_BOOT)
OBJECTS_BOOT = $(COBJECTS_BOOT) $(CXXOBJECTS_BOOT)

CSOURCES_LOADER = $(wildcard $(SOURCEDIR)/loader/*.c)
COBJECTS_LOADER = $(patsubst $(SOURCEDIR)/loader/%.c, $(OBJECTDIR)/loader/%.o, $(CSOURCES_LOADER))
CXXSOURCES_LOADER = $(wildcard $(SOURCEDIR)/loader/*.cpp)
CXXOBJECTS_LOADER = $(patsubst $(SOURCEDIR)/loader/%.cpp, $(OBJECTDIR)/loader/%.o, $(CXXSOURCES_LOADER))
SOURCES_LOADER = $(CSOURCES_LOADER) $(CXXSOURCES_LOADER)
OBJECTS_LOADER = $(COBJECTS_LOADER) $(CXXOBJECTS_LOADER)


all: clean-bin $(SOURCEDIR)/include/loader.h $(TARGET_BOOT).bin

$(TARGET_BOOT).bin: $(TARGET_BOOT).elf
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_BOOT) -d $^
	@echo ---------------------------------------------------------------------------
	@esptool2 -bin -$(FLASH_SIZE) -$(FLASH_SPEED) -$(FLASH_MODE) -boot0 $^ $@ .text .rodata

$(SOURCEDIR)/include/loader.h: $(TARGET_LOADER).elf
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_LOADER) -d $^
	@echo ---------------------------------------------------------------------------
	@esptool2 -header $^ $@ .text

$(TARGET_LOADER).elf: $(OBJECTS_LOADER)
	@$(LD) -o $@ $^ -T $(LDSCRIPT_LOADER) $(LDFLAGS)

$(TARGET_BOOT).elf: $(OBJECTS_BOOT)
	@$(LD) -o $@ $^ -T $(LDSCRIPT_BOOT) $(LDFLAGS)

$(OBJECTDIR)/boot/%.o: $(SOURCEDIR)/boot/%.c
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJECTDIR)/boot/%.o: $(SOURCEDIR)/boot/%.cpp
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

$(OBJECTDIR)/loader/%.o: $(SOURCEDIR)/loader/%.c
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJECTDIR)/loader/%.o: $(SOURCEDIR)/loader/%.cpp
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

flash: clean-bin $(SOURCEDIR)/include/loader.h $(TARGET_BOOT).bin
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m 0x00000 $(TARGET_BOOT).bin

flash-conf-512k:
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m 0x01000 bootcfg_512.bin

flash-conf-1m:
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m 0x01000 bootcfg_1024.bin

clean-bin:
	@rm -f $(TARGETDIR)/*.bin
	@rm -f $(TARGETDIR)/*.elf
	@rm -f $(SOURCEDIR)/include/loader.h

clean: clean-bin
	@rm -f $(OBJECTDIR)/boot/*
	@rm -f $(OBJECTDIR)/loader/*
