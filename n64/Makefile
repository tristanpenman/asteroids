include $(ROOT)/usr/include/make/PRdefs

N64KITDIR = c:\nintendo\n64kit
NUSYSINCDIR = $(N64KITDIR)/nusys/include
NUSYSLIBDIR = $(N64KITDIR)/nusys/lib

LIB = $(ROOT)/usr/lib
LPR = $(LIB)/PR
INC = $(ROOT)/usr/include

LCDEFS = -DN_AUDIO -DNU_DEBUG -DF3DEX_GBI_2 -DN64
LCINCS = -I. -I$(NUSYSINCDIR) -I$(ROOT)/usr/include/PR
LCOPTS = -G 0 -O2
LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(NUSYSLIBDIR) -ln_gmus_d -lnualstl_n_d -ln_gaudio_sc -lnusys_d -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc

OPTIMIZER = -g

APP = asteroids.out

TARGETS = asteroids.n64

CODEFILES = $(wildcard *.c)

CODEOBJECTS = $(CODEFILES:.c=.o) $(NUSYSLIBDIR)/nusys.o

DATAFILES =

DATAOBJECTS = $(DATAFILES:.c=.o)

CODESEGMENT = codesegment.o

OBJECTS = $(CODESEGMENT) $(DATAOBJECTS)

default: $(TARGETS)

include $(COMMONRULES)

$(CODESEGMENT): $(CODEOBJECTS) Makefile
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(TARGETS): $(OBJECTS)
	$(MAKEROM) spec -I$(NUSYSINCDIR) -r $(TARGETS) -e $(APP)
	makemask $(TARGETS)
