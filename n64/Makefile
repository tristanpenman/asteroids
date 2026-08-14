TARGET := asteroids
BUILD_DIR := build-n64
ROM := $(BUILD_DIR)/$(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
LD_SCRIPT := asteroids.ld
LD_SCRIPT_SOURCE := n64/asteroids.ld
BOOT := /usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ := $(BUILD_DIR)/boot.6102.o

DEFINES := _FINALROM=1 F3DEX_GBI_2=1 N64
SRC_DIRS := src/platform/n64 n64/asm
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
O_FILES := /usr/lib/n64/nusys/nusys_rom.o \
	$(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
	$(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
	$(BOOT_OBJ)
DEP_FILES := $(O_FILES:.o=.d)

CC := mips-n64-gcc
CPP := cpp
LD := mips-n64-ld
OBJCOPY := mips-n64-objcopy

INCLUDE_DIRS := . /usr/include/n64 /usr/include/n64/PR \
	/usr/include/n64/nustd /usr/include/n64/nusys n64/include \
	src/game src/platform $(BUILD_DIR)
C_DEFINES := $(foreach definition,$(DEFINES),-D$(definition))
INCLUDES := $(foreach directory,$(INCLUDE_DIRS),-I$(directory))
CFLAGS := -G 0 -g -mabi=32 -ffreestanding -mfix4300 $(INCLUDES) $(C_DEFINES)
CPPFLAGS := -P -Wno-trigraphs $(INCLUDES) $(C_DEFINES)

default: $(ROM)

clean:
	$(RM) -r $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -MMD -MF $(@:.o=.d) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(foreach directory,$(INCLUDE_DIRS),-Wa,-I$(directory)) \
		-x assembler-with-cpp -MMD -MF $(@:.o=.d) -o $@ $<

$(BUILD_DIR)/$(LD_SCRIPT): $(LD_SCRIPT_SOURCE)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ \
		-MF $@.d -o $@ $<

$(BOOT_OBJ): $(BOOT)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

$(ELF): $(O_FILES) $(BUILD_DIR)/$(LD_SCRIPT)
	$(LD) -L $(BUILD_DIR) -T $(BUILD_DIR)/$(LD_SCRIPT) \
		-Map $(BUILD_DIR)/$(TARGET).map --no-check-sections -o $@ $(O_FILES) \
		-L/usr/lib/n64/nusys -lnusys -L/usr/lib/n64 -lultra_rom -lnustd \
		-L$(N64_LIBGCCDIR) -lgcc

$(ROM): $(ELF)
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $< $@ -O binary
	makemask $@

.PHONY: clean default
.SECONDARY:
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)
