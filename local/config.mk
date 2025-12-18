# local/config.mk
# Configuration specific to this build host.
# Eventually this will be gitignored, and you'll get a default on the initial clone, which you can tweak to suit. TODO

# Directories immediately under `src/opt/` to include in the build.
# These are also used as general-purpose preprocessor flags: mswin
OPT_ENABLE:=fs wm_x11

# GCC or similar toolchain.
# LDPOST must agree with OPT_ENABLE, ensuring it is up to you.
CC:=gcc -c -MMD -O3 -Isrc -Werror -Wimplicit $(foreach U,$(OPT_ENABLE),-DUSE_$U=1)
LD:=gcc -z noexecstack
LDPOST:=-lX11
AR:=ar
EXESFX:=
