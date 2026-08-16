TARGET := asteroids
BUILD_DIR := build-n64
ROM := $(BUILD_DIR)/$(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
LD_SCRIPT := asteroids.ld
LD_SCRIPT_SOURCE := n64/asteroids.ld
BOOT := /usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ := $(BUILD_DIR)/boot.6102.o
GFX_UCODE_FILES := /usr/lib/n64/PR/rspboot.o \
	/usr/lib/n64/PR/gspF3DEX2.fifo.o \
	/usr/lib/n64/PR/gspL3DEX2.fifo.o \
	/usr/lib/n64/PR/gspF3DEX2.Rej.fifo.o \
	/usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o \
	/usr/lib/n64/PR/gspF3DLX2.Rej.fifo.o \
	/usr/lib/n64/PR/gspS2DEX2.fifo.o

DEFINES := _FINALROM=1 F3DEX_GBI_2=1 N64 ASTEROIDS_PLATFORM_N64=1
SRC_DIRS := src/platform/n64 src/platform/n64/unfloader n64/asm
GAME_C_FILES := \
	src/game/collision.c \
	src/game/data.c \
	src/game/draw.c \
	src/game/entities.c \
	src/game/game.c \
	src/game/gameover.c \
	src/game/highscores.c \
	src/game/initials.c \
	src/game/level.c \
	src/game/sandbox.c \
	src/game/text.c \
	src/game/titlescreen.c \
	src/game/transition.c \
	src/game/util.c \
	src/game/vec.c

# Hide tool commands by default; use VERBOSE=1 to show them.
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
V := @
endif

# Colorize build status messages by default; use COLOR=0 to disable colors.
COLOR ?= 1

C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(GAME_C_FILES)
S_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
O_FILES := /usr/lib/n64/nusys/nusys_rom.o \
	$(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
	$(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
	$(GFX_UCODE_FILES) \
	$(BOOT_OBJ)
DEP_FILES := $(O_FILES:.o=.d)

CC := mips-n64-gcc
CPP := cpp
LD := mips-n64-ld
OBJCOPY := mips-n64-objcopy

INCLUDE_DIRS := . /usr/include/n64 /usr/include/n64/PR \
	/usr/include/n64/nustd /usr/include/n64/nusys n64/include \
	src/game src/platform/n64/unfloader src/platform $(BUILD_DIR)
C_DEFINES := $(foreach definition,$(DEFINES),-D$(definition))
INCLUDES := $(foreach directory,$(INCLUDE_DIRS),-I$(directory))
CFLAGS := -G 0 -g -mabi=32 -ffreestanding -mfix4300 $(INCLUDES) $(C_DEFINES)
CPPFLAGS := -P -Wno-trigraphs $(INCLUDES) $(C_DEFINES)

PRINT := printf

ifeq ($(COLOR),1)
NO_COL := \033[0m
GREEN := \033[0;32m
BLUE := \033[0;34m
YELLOW := \033[0;33m
endif

define print
	@$(PRINT) "$(GREEN)$(1) $(YELLOW)$(2)$(GREEN) -> $(BLUE)$(3)$(NO_COL)\n"
endef

default: $(ROM)

clean:
	$(RM) -r $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) -MMD -MF $(@:.o=.d) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(call print,Assembling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(foreach directory,$(INCLUDE_DIRS),-Wa,-I$(directory)) \
		-x assembler-with-cpp -MMD -MF $(@:.o=.d) -o $@ $<

$(BUILD_DIR)/$(LD_SCRIPT): $(LD_SCRIPT_SOURCE)
	@mkdir -p $(dir $@)
	$(call print,Preprocessing linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ \
		-MF $@.d -o $@ $<

$(BOOT_OBJ): $(BOOT)
	@mkdir -p $(dir $@)
	$(V)$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

$(ELF): $(O_FILES) $(BUILD_DIR)/$(LD_SCRIPT)
	@$(PRINT) "$(GREEN)Linking ELF file: $(BLUE)$@$(NO_COL)\n"
	$(V)$(LD) -L $(BUILD_DIR) -T $(BUILD_DIR)/$(LD_SCRIPT) \
		-Map $(BUILD_DIR)/$(TARGET).map --no-check-sections -o $@ $(O_FILES) \
		-L/usr/lib/n64/nusys -lnusys -L/usr/lib/n64 -lultra_rom -lnustd \
		-L$(N64_LIBGCCDIR) -lgcc

$(ROM): $(ELF)
	$(call print,Building ROM:,$<,$@)
	$(V)$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $< $@ -O binary
	$(V)makemask $@

.PHONY: clean default
.SECONDARY:
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)
