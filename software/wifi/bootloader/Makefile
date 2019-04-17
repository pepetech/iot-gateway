FLASH_MODE = dio
FLASH_SPEED = 80
FLASH_SIZE = 4096
#FLASH_SIZE = 1024
ESP_BAUD = 921600
ESP_PORT = /dev/ttyUSB0

#######################################################

# Directories
TARGETDIR = bin
SOURCEDIR = src
OBJECTDIR = bin/obj

INCLUDEDIRS = $(SOURCEDIR)/ $(SOURCEDIR)/include

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
CFLAGS = $(addprefix -I,$(INCLUDEDIRS)) -Os -std=gnu99 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -fno-builtin-printf
CXXFLAGS = $(addprefix -I,$(INCLUDEDIRS)) -fno-rtti -fno-exceptions -Os -std=gnu99 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -fno-builtin-printf
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,--gc-sections
LDLIBS =

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


all: clean-bin make-dir $(SOURCEDIR)/include/loader.h $(TARGET_BOOT).bin

$(TARGET_BOOT).bin: $(TARGET_BOOT).elf
	@echo Creating BIN file \'$(TARGET_BOOT).bin\'...
	@esptool2 -bin -$(FLASH_SIZE) -$(FLASH_SPEED) -$(FLASH_MODE) -boot0 $^ $@ .text .rodata
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_BOOT) -d $^

$(SOURCEDIR)/include/loader.h: $(TARGET_LOADER).elf
	@echo Creating loader header file \'$@\'...
	@esptool2 -header $^ $@ .text
	@echo ---------------------------------------------------------------------------
	@espmem -l $(LDSCRIPT_LOADER) -d $^
	@echo ---------------------------------------------------------------------------

$(TARGET_LOADER).elf: $(OBJECTS_LOADER)
	@echo ---------------------------------------------------------------------------
	@echo Creating ELF file \'$@\'...
	@$(LD) -o $@ $^ -T $(LDSCRIPT_LOADER) $(LDFLAGS)

$(TARGET_BOOT).elf: $(OBJECTS_BOOT)
	@echo ---------------------------------------------------------------------------
	@echo Creating ELF file \'$@\'...
	@$(LD) -o $@ $^ -T $(LDSCRIPT_BOOT) $(LDFLAGS)

$(OBJECTDIR)/boot/%.o: $(SOURCEDIR)/boot/%.c
	@echo Compilling C file \'$<\' \> \'$@\'...
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJECTDIR)/boot/%.o: $(SOURCEDIR)/boot/%.cpp
	@echo Compilling C++ file \'$<\' \> \'$@\'...
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

$(OBJECTDIR)/loader/%.o: $(SOURCEDIR)/loader/%.c
	@echo Compilling C file \'$<\' \> \'$@\'...
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJECTDIR)/loader/%.o: $(SOURCEDIR)/loader/%.cpp
	@echo Compilling C++ file \'$<\' \> \'$@\'...
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

flash: clean-bin $(SOURCEDIR)/include/loader.h $(TARGET_BOOT).bin
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m 0x00000 $(TARGET_BOOT).bin

flash-conf-512k:
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m 0x01000 bootcfg_512.bin

flash-conf-1m:
	@esptool.py -b $(ESP_BAUD) -p $(ESP_PORT) -a hard_reset write_flash -fm $(FLASH_MODE) -ff $(FLASH_SPEED)m 0x01000 bootcfg_1024.bin

make-dir:
	@mkdir -p $(OBJECTDIR)/boot
	@mkdir -p $(OBJECTDIR)/loader
	@echo ---------------------------------------------------------------------------

clean-bin:
	@rm -f $(TARGETDIR)/*.bin
	@rm -f $(TARGETDIR)/*.elf
	@rm -f $(SOURCEDIR)/include/loader.h

clean: clean-bin
	@rm -f $(OBJECTDIR)/boot/*
	@rm -f $(OBJECTDIR)/loader/*
