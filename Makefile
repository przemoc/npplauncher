BIN := notepad

notepad_SRCS := notepad.c
notepad_RCS  := notepad.rc

notepad_OBJS := $(notepad_SRCS:.c=.o)
notepad_RSRS := $(notepad_RCS:.rc=.res)

all: $(BIN)

notepad: $(notepad_OBJS) $(notepad_RSRS)

###

V ?= 0
HIDE_0 := @
HIDE_1 :=
HIDE := $(HIDE_$(V))

comma := ,

DEFCC  := gcc
ifeq ($(origin CC),default)
CC     := $(DEFCC)
endif
ifneq ($(origin CC),environment)
CC  := $(CROSS_COMPILE)$(CC)
endif
ifneq ($(origin CCLD),environment)
CCLD  := $(CC)
endif
ifneq ($(origin STRIP),environment)
STRIP := $(CROSS_COMPILE)strip
endif
ifneq ($(origin WINDRES),environment)
WINDRES := $(CROSS_COMPILE)windres
endif

MUSTHAVE_FLAGS    := \
 -D_FILE_OFFSET_BITS=64 \
 -Wall -Wextra -pedantic \
 -g \

MUSTHAVE_CFLAGS   := -std=c99
OPTIONAL_FLAGS    := -O2

ifneq ($(origin CFLAGS),environment)
CFLAGS   = $(OPTIONAL_FLAGS)
endif
override CFLAGS   += $(MUSTHAVE_FLAGS) $(MUSTHAVE_CFLAGS)

override LDFLAGS  := $(subst -Wl$(comma),,$(LDFLAGS))
ifneq ($(origin CCLDFLAGS),environment)
CCLDFLAGS  := $(addprefix -Wl$(comma),$(LDFLAGS))
endif

CC_PARAMS = $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH)

RM ?= rm -f

BIN_EXT := .exe

###

%.o: %.c
	@echo "        CC      $@"
	$(HIDE)$(CC) $(CC_PARAMS) -c -o $@ $<

%.res: %.rc
	@echo "        WINDRES $@"
	$(HIDE)$(WINDRES) -i $< --input-format=rc -o $@ -O coff

$(BIN):
	@echo "        CCLD    $@"
	$(HIDE)$(CCLD) $(LDFLAGS) $(TARGET_ARCH) -o $@ $^ \
	 -mwindows \
	 -Wl,-Bstatic $($@_SLIBS) -Wl,-Bdynamic $($@_DLIBS)

.PHONY: strip
strip: $(BIN)
	@echo "        STRIP   $^"
	$(HIDE)$(STRIP) $^$(BIN_EXT)

.PHONY: clean
clean:
	@echo "        CLEAN"
	$(HIDE)$(RM) $(BIN) $($(BIN)_OBJS) $($(BIN)_RSRS)
