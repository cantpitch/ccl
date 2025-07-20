#
# Copyright 2025 Clozure Associates
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

MACOSX_SDK := $(shell xcrun --sdk macosx --show-sdk-path)
OSVERSION = 13.0
VPATH = ../ ../test
RM = /bin/rm
AS = clang
M4 = m4
CC = clang
ASFLAGS = -arch arm64 -g -mmacosx-version-min=$(OSVERSION)
M4FLAGS = -DDARWIN -DARM64
CDEFINES = -isysroot $(MACOSX_SDK) -DARM64 -DDARWIN -D_REENTRANT -D_XOPEN_SOURCE -DDISABLE_EGC -DGC_INTEGRITY_CHECKING -Wp,-dM
CDEBUG = -g -O0
COPT = #-O2
# word size issues are a little more relevant on a 64-bit platform
# than elsewhere, but most gcc format warnings are still nonsense.
WFORMAT = -Wno-format
PLATFORM_H = platform-darwinarm64.h
LISP_KERNEL = ../../darmcl64

SRCDIRS = 


.s.o:
	$(M4) $(M4FLAGS) -I../ $< | $(AS) -x assembler -c $(ASFLAGS) -o $@ -
.c.o:
	$(CC) -include ../$(PLATFORM_H) -c $< $(CDEFINES) $(CDEBUG) $(COPT) \
	$(WFORMAT) -mmacosx-version-min=$(OSVERSION) -o $@ 

ARM64_ASM_OBJ = arm-asmutils64.o 
ARM64_ASM_INC = arm-constants64.s arm-macros64.s arm-uuo64.s
ARM64_C_OBJ = arm-gc64.o arm-exceptions64.o 
ARM64_C_INC = arm-constants64.h arm-exceptions64.h $(PLATFORM_H)

SPOBJ = pad.o arm-spentry64.o arm-spjump64.o arm-subprims64.o
COBJ  = gc-common.o bits.o image.o thread_manager.o lisp-debug.o memory.o unix-calls.o mach-o-image.o \
	$(ARM64_C_OBJ) 
ASMOBJ = imports.o $(ARM64_ASM_OBJ) 

DEBUGOBJ = lispdcmd.o plprint.o plsym.o plbt.o arm64_print.o
KERNELOBJ= pmcl-kernel.o $(COBJ) $(ASMOBJ)
TESTOBJ = testrunner.o memory_tests.o pmcl-kernel-mock.o unity.o $(COBJ) $(ASMOBJ)

SPINC =	lisp.s m4macros.m4 errors.s lisp_globals.s $(ARM64_ASM_INC) 
CHEADERS = area.h bits.h lisp-errors.h gc.h lisp.h lisp-exceptions.h lisp_globals.h macros.h memprotect.h image.h \
	threads.h constants.h os-linux.h $(PLATFORM_H) $(ARM64_C_INC)

all:	$(LISP_KERNEL)

# -pagezero_size 0x8000  
${LISP_KERNEL}:	$(SPOBJ) $(KERNELOBJ) $(DEBUGOBJ) Makefile
	$(LD)  -arch arm64 -sectalign __TEXT __text 0x1000  \
	-L$(MACOSX_SDK)/usr/lib -lsystem -o $@ $(SPOBJ) $(KERNELOBJ) $(DEBUGOBJ)


$(SPOBJ): $(SPINC)
$(ASMOBJ): $(SPINC)
$(COBJ): $(CHEADERS)
$(DEBUGOBJ): $(CHEADERS) ../lispdcmd.h

cclean:
	$(RM) -f $(KERNELOBJ) $(DEBUGOBJ) ${LISP_KERNEL}

clean:	cclean
	$(RM) -f $(SPOBJ)

dump:
	otool -tvv ${LISP_KERNEL}

strip:	${LISP_KERNEL}
	strip -g ${LISP_KERNEL}

testrunner: $(SPOBJ) $(TESTOBJ) $(DEBUGOBJ) Makefile
	${CC} -mmacosx-version-min=$(OSVERSION) -arch arm64 -lSystem \
	-L$(MACOSX_SDK)/usr/lib -o $@ $(SPOBJ) $(TESTOBJ) $(DEBUGOBJ)

run_tests: testrunner
	./testrunner
