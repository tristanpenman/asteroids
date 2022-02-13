include $(ROOT)/usr/include/make/PRdefs

N64KITDIR = c:/nintendo/n64kit
NUSYSINCDIR = $(N64KITDIR)/nusys/include
NUSYSLIBDIR = $(N64KITDIR)/nusys/lib

LIB = $(ROOT)/usr/lib
LPR = $(LIB)/PR
INC = $(ROOT)/usr/include

LCDEFS = -DDEBUG -DN_AUDIO -DNU_DEBUG -DF3DEX_GBI_2 -DN64
LCINCS = -I. -I$(NUSYSINCDIR) -I$(ROOT)/usr/include/PR
LCOPTS = -G 0 -O2
LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(NUSYSLIBDIR) -ln_gmus_d -lnualstl_n_d -ln_gaudio_sc -lnusys_d -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc

OPTIMIZER = -g

BUILD_DIR = build
APP = $(BUILD_DIR)/asteroids.out
TARGETS = $(BUILD_DIR)/asteroids.n64

CODEFILES = $(wildcard src/*.c)

CODEOBJECTS = $(foreach file,$(CODEFILES),$(BUILD_DIR)/$(file:.c=.o)) \
	$(NUSYSLIBDIR)/nusys.o

DATAFILES =

DATAOBJECTS = \
	$(foreach file,$(DATAFILES),$(BUILD_DIR)/$(file:.c=.o))

CODESEGMENT = $(BUILD_DIR)/codesegment.o

OBJECTS = $(CODESEGMENT) $(DATAOBJECTS)

default: $(TARGETS)

include $(COMMONRULES)

# Compile C code
$(BUILD_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(CODESEGMENT): $(CODEOBJECTS) Makefile
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(TARGETS): $(OBJECTS) spec
	$(MAKEROM) spec -I$(NUSYSINCDIR) -r $(TARGETS) -e $(APP)
	makemask $(TARGETS)
