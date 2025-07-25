#
# Copyright 2010 Clozure Associates
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LIBBASE = 0x04001000
NDK = /usr/local/android-ndk-r10c
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Linux)



HOST = linux-x86_64
else
ifeq ($(UNAME_S),Darwin)
HOST = darwin-x86
else
HOST = unknown
endif
endif
ANDROIDVERSION = android-9
SYSROOT = $(NDK)/platforms/$(ANDROIDVERSION)/arch-arm
NDKLIB = $(SYSROOT)/usr/lib
NDKINC = $(SYSROOT)/usr/include
ABI = arm-linux-androideabi
NDKBIN = $(NDK)/toolchains/$(ABI)-4.9/prebuilt/$(HOST)/bin/$(ABI)-
GCCLIB = $(NDK)/toolchains/$(ABI)-4.9/prebuilt/$(HOST)/lib/gcc/$(ABI)/4.9/armv7-a
VPATH = ../
RM = /bin/rm
AS = $(NDKBIN)as
CC = $(NDKBIN)gcc --sysroot $(SYSROOT)
LD = $(NDKBIN)ld
HOSTCC = cc -m32
M4 = m4
ASFLAGS = -mfpu=vfp -k
M4FLAGS = -DLINUX -DARM -DANDROID -DLIBBASE=$(LIBBASE)
CDEFINES = -DLINUX -DARM -DANDROID -D_REENTRANT -D_GNU_SOURCE -DCCLSHARED
CDEBUG = -g
COPT = #-O2
# Once in a while, -Wformat says something useful.  The odds are against that,
# however.
WFORMAT = -Wno-format
PLATFORM_H = platform-androidarm.h

# If the linker supports a "--hash-style=" option, use traditional
# Sysv hash tables.  (If it doesn't support that option, assume
# that traditional hash tables will be used by default.)
ld_has_hash_style = $(shell $(LD) --help | grep "hash-style=")
ifeq ($(ld_has_hash_style),)
HASH_STYLE=
else
HASH_STYLE="-Wl,--hash-style=sysv"
endif



.s.o:
	$(M4) $(M4FLAGS) -I../ $< | $(AS) $(ASFLAGS) -o $@
.c.o:
	$(CC) -include ../$(PLATFORM_H) -I. -I$(NDKINC) -c $<  $(CDEFINES) $(CDEBUG) $(COPT) $(WFORMAT) -fPIC -o $@


# -nostdinc -isystem ${NDK}/usr/gcc-include -isystem $(NDK)/usr/include
SPOBJ = pad.o  arm-spentry32.o 
#arm-subprims.o
ASMOBJ = arm-asmutils32.o imports.o

COBJ  = pmcl-kernel.o gc-common.o arm-gc32.o bits.o  arm-exceptions32.o \
	image.o thread_manager.o lisp-debug.o memory.o unix-calls.o \
	android_native_app_glue.o

DEBUGOBJ = lispdcmd.o plprint.o plsym.o albt.o arm_print.o
KERNELOBJ= $(COBJ) arm-asmutils32.o  imports.o

SPINC =	lisp.s m4macros.m4 arm-constants32.s arm-macros32.s errors.s arm-uuo32.s \
	lisp_globals.s

CHEADERS = area.h bits.h arm-constants32.h lisp-errors.h gc.h lisp.h \
	lisp-exceptions.h lisp_globals.h macros.h memprotect.h image.h \
	threads.h arm-exceptions32.h $(PLATFORM_H)

# Subprims linked into the kernel ?
# Yes:

KSPOBJ = $(SPOBJ)
all:	../../libaarmcl.so ../../aarmcl

aarmcl.o: aarmcl.c
	$(CC) -g -I$(NDKINC) -c $< -fPIC -o $@


OSLIBS = -ldl -lm -lc -landroid -llog
APPLIBS = $(OSLIBS) $(NDKLIB)/crtbegin_dynamic.o $(NDKLIB)/crtend_android.o $(GCCLIB)/libgcc.a


../../libaarmcl.so:	$(KSPOBJ) $(KERNELOBJ) $(DEBUGOBJ) ./armandroid.x ./fixlib
	$(LD) --shared -fPIC -Bdynamic -dynamic-linker /system/bin/linker -nostdlib -z nocopyreloc  -o ../../libaarmcl.so  -L $(NDKLIB) pad.o arm-spentry32.o  pmcl-kernel.o gc-common.o arm-gc32.o bits.o arm-exceptions32.o image.o thread_manager.o lisp-debug.o memory.o unix-calls.o arm-asmutils32.o imports.o lispdcmd.o plprint.o plsym.o albt.o arm_print.o android_native_app_glue.o --no-as-needed --warn-shared-textrel $(OSLIBS)
#	./fixlib $(LIBBASE) >> ../../libaarmcl.so

../../aarmcl:	aarmcl.o
		$(LD) -Bdynamic -pie -e _start  -dynamic-linker /system/bin/linker -nostdlib -z nocopyreloc  -o ../../aarmcl -L $(NDKLIB) aarmcl.o $(APPLIBS)

./fixlib: ./fixlib.c
	$(HOSTCC) -g $< -o $@

$(SPOBJ): $(SPINC)
$(ASMOBJ): $(SPINC)
$(COBJ): $(CHEADERS)
$(DEBUGOBJ): $(CHEADERS) lispdcmd.h


cclean:
	$(RM) -f $(KERNELOBJ) $(DEBUGOBJ) ../../aarmcl ../../libaarmcl.so ./fixlib

clean:	cclean
	$(RM) -f $(SPOBJ)

