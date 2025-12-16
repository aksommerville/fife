all:
.SILENT:
.SECONDARY:
.SUFFIXES:
PRECMD=echo "  $@" ; mkdir -p $(@D) ;

# Things we can do without scanning the project.
ifneq (,$(strip $(filter clean,$(MAKECMDGOALS))))

clean:;rm -rf mid out

# The proper build...
else

SRCFILES:=$(shell find src -type f)

include local/config.mk

CFILES:=$(filter %.c,$(SRCFILES))
CFILES_OPT:=$(filter $(addprefix src/opt/,$(addsuffix /%,$(OPT_ENABLE))),$(CFILES))
CFILES_ALL:=$(filter-out src/opt/%,$(CFILES))
CFILES:=$(CFILES_ALL) $(CFILES_OPT)
OFILES:=$(patsubst src/%.c,mid/%.o,$(CFILES))
-include $(OFILES:.o=.d)
mid/%.o:src/%.c;$(PRECMD) $(CC) -o$@ $<

EXE_DEMO:=out/demo$(EXESFX)
all:$(EXE_DEMO)
$(EXE_DEMO):$(OFILES);$(PRECMD) $(LD) -o$@ $(OFILES) $(LDPOST)
run:$(EXE_DEMO);$(EXE_DEMO)

LIB_STATIC:=out/libfife.a
all:$(LIB_STATIC)
LIB_STATIC_OFILES:=$(filter-out mid/demo/%,$(OFILES))
$(LIB_STATIC):$(LIB_STATIC_OFILES);$(PRECMD) $(AR) rc $@ $(LIB_STATIC_OFILES)

endif
