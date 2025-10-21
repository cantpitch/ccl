#
# Copyright 1994-2001 Clozure Associates
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

# 2025-10-20 - File created from lisp-kernel/linuxppc64/Makefile

VC_REVISION := "$(shell git describe --dirty 2>/dev/null || echo unknown)"
MACOSX_SDK := $(shell xcrun --sdk macosx --show-sdk-path)

# macOS Ventura
OSVERSION = 13.0
VPATH = ../
RM = /bin/rm
# Versions of GNU as >= 2.9.1 all seem to work
# AS = gas-2.9.1
AS = clang
M4 = m4
ASFLAGS = -arch arm64 -g -mmacosx-version-min=$(OSVERSION)
M4FLAGS = -DDARWIN -DARM64
CDEFINES = -isysroot $(MACOSX_SDK) -DARM64 -DDARWIN -D_REENTRANT -D_XOPEN_SOURCE \
	-DDISABLE_EGC -DGC_INTEGRITY_CHECKING -DVC_REVISION=$(VC_REVISION) -Wp,-dM 
CDEBUG = -g
COPT = #-O2
# word size issues are a little more relevant on a 64-bit platform
# than elsewhere, but most gcc format warnings are still nonsense.
WFORMAT = -Wno-format
PLATFORM_H = platform-darwinarm64.h


.s.o:
	$(M4) $(M4FLAGS) -I../ $< | $(AS) -x assembler -c $(ASFLAGS) -o $@ -
.c.o:
	$(CC) -include ../$(PLATFORM_H) -c $< $(CDEFINES) $(CDEBUG) $(COPT) $(WFORMAT) -m64 -o $@

SPOBJ = pad.o arm64-spjump.o arm64-spentry.o arm64-subprims.o
ASMOBJ = arm64-asmutils.o imports.o

COBJ  = pmcl-kernel.o gc-common.o arm64-gc.o bits.o arm64-exceptions.o \
	image.o thread_manager.o lisp-debug.o memory.o unix-calls.o

DEBUGOBJ = lispdcmd.o plprint.o plsym.o plbt.o arm64_print.o
KERNELOBJ= $(COBJ) arm64-asmutils.o  imports.o

SPINC =	lisp.s m4macros.m4 arm64-macros.s errors.s arm64-uuo.s arm64-constants.s lisp_globals.s

CHEADERS = area.h bits.h arm64-constants.h lisp-errors.h gc.h lisp.h lisp-exceptions.h \
	lisp_globals.h macros.h memprotect.h image.h threads.h arm64-constants.h arm64-exceptions.h \
	$(PLATFORM_H) constants.h

# Subprims linked into the kernel ?
# Yes:

KSPOBJ = $(SPOBJ)
all:	../../darm64cl


# No:

# KSPOBJ=
# all:	../../ppccl64 ../../subprims.so

OSLIBS = -lsystem


../../darm64cl:	$(KSPOBJ) $(KERNELOBJ) $(DEBUGOBJ)
	$(CC) -arch arm64 $(CDEBUG)  -sectalign __TEXT __text 0x1000  \
	-L$(MACOSX_SDK)/usr/lib -o $@ $(KSPOBJ) $(KERNELOBJ) $(DEBUGOBJ) 


$(SPOBJ): $(SPINC)
$(ASMOBJ): $(SPINC)
$(COBJ): $(CHEADERS)
$(DEBUGOBJ): $(CHEADERS) lispdcmd.h


cclean:
	$(RM) -f $(KERNELOBJ) $(DEBUGOBJ) ../../darm64cl

clean:	cclean
	$(RM) -f $(SPOBJ)

strip:	../../darm64cl
	strip -g ../../darm64cl
