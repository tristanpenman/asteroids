TARGET := asteroids
BUILD_DIR := build-dreamcast
ELF := $(BUILD_DIR)/$(TARGET).elf
BIN := $(BUILD_DIR)/$(TARGET).bin

GAME_SRCS := \
	src/game/collision.c \
	src/game/data.c \
	src/game/draw.c \
	src/game/entities.c \
	src/game/game.c \
	src/game/gameover.c \
	src/game/highscores.c \
	src/game/initials.c \
	src/game/leaderboard.c \
	src/game/level.c \
	src/game/sandbox.c \
	src/game/text.c \
	src/game/titlescreen.c \
	src/game/transition.c \
	src/game/util.c \
	src/game/vec.c

PLATFORM_SRCS := \
	src/platform/dreamcast/canvas.c \
	src/platform/dreamcast/debug.c \
	src/platform/dreamcast/input.c \
	src/platform/dreamcast/loop.c \
	src/platform/dreamcast/main.c \
	src/platform/dreamcast/mixer.c \
	src/platform/dreamcast/storage.c \
	src/platform/dreamcast/timing.c \
	src/platform/dreamcast/video.c

SRCS := $(GAME_SRCS) $(PLATFORM_SRCS)
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS += -Isrc/game -Isrc/platform -DASTEROIDS_PLATFORM_DREAMCAST=1
CFLAGS += -std=c99 -Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections -Wl,-Map,$(BUILD_DIR)/$(TARGET).map
LIBS := -lm

ifeq ($(BUILD),release)
CFLAGS += -O2 -DNDEBUG -DDEBUG=0
else
CFLAGS += -Og -g3 -DDEBUG=1
endif

all: $(ELF)

bin: $(BIN)

include $(KOS_BASE)/Makefile.rules

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	kos-cc $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(ELF): $(OBJS)
	@mkdir -p $(dir $@)
	kos-cc $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(BIN): $(ELF)
	$(KOS_OBJCOPY) -O binary $< $@

clean:
	$(RM) -r $(BUILD_DIR)

.PHONY: all bin clean

-include $(DEPS)
