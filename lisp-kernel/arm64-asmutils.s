/***** BEGIN IMPORT FROM ppc-asmutils.s *****/

/*
 * Copyright 1994-2009 Clozure Associates
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

	

	include(lisp.s)

	_beginfile
/*  Zero R4 cache lines, starting at address in R3.  Each line is assumed to be */
/* R5 bytes wide. */
/* PPC CODE
_exportfn(C(zero_cache_lines))
	__(cmpri(cr0,r4,0))
	__(mtctr r4)
	__(beqlr)
1:
	__(DCBZL(0,r3))
	__(add r3,r3,r5)
	__(bdnz 1b)
	__(blr)
_endfn

/*  Flush R4 cache lines, starting at address in R3.  Each line is */
/* assumed to be R5 bytes wide. */
/* PPC CODE
_exportfn(C(flush_cache_lines))
	__(cmpri(cr0,r4,0))
	__(mtctr r4)
        __(mr r6,r3)
	__(beqlr)
1:
	__(dcbst 0,r3)
        __(add r3,r3,r5)
        __(bdnz 1b)
	__(sync)                /* wait until dcbst's get to memory */
/* PPC CODE
        __(mr r3,r6)
        __(mtctr r4)
2:      
	__(icbi 0,r3)
	__(add r3,r3,r5)
	__(bdnz 2b)
        __(sync)
	__(isync)
	__(blr)
_endfn

_exportfn(C(touch_page))
        __(str(r3,0(r3)))
        __(li r4,0)
        __(str(r4,0(r3)))
        __(li r3,1) /* can't assume that low 32 bits of r3 are non-zero */
/* PPC CODE
        .globl C(touch_page_end)
C(touch_page_end):
        __(blr)
_endfn
*/

/* ARM64 */
_exportfn(C(current_stack_pointer))
    __(mov x0, sp)
    __(ret)
_endfn

/* PPC CODE                                
_exportfn(C(current_stack_pointer))
	__(mr r3,sp)
	__(blr)
_endfn
	
_exportfn(C(count_leading_zeros))
        __ifdef(`PPC64')
        __(cntlzd r3,r3)
        __else
	__(cntlzw r3,r3)
        __endif
	__(blr)
_endfn

_exportfn(C(noop))
	__(blr)
_endfn

_exportfn(C(set_fpscr))
	__(stru(sp,-32(sp)))
	__(stw r3,12(sp))
	__(lfd f0,8(sp))
	__(mtfsf 0xff,f0)
	__(la sp,32(sp))
	__(blr)
_endfn


_exportfn(C(get_fpscr))
	__(stru(sp,-32(sp)))
        __(mffs f0)
        __(stfd f0,8(sp))
        __(lwz r3,12(sp))
	__(la sp,32(sp))
	__(blr)
_endfn
                

/* The Linux kernel is constantly enabling and disabling the FPU and enabling */
/* FPU exceptions.  We can't touch the FPU without turning off the FPSCR`FEX' */
/* bit and we can't turn off the FPSCR`FEX' bit without touching the FPU. */
/* Force a distinguished exception, and let the handler for that exception */
/* zero the fpscr in its exception context. */
/* PPC CODE
_exportfn(C(zero_fpscr))
	__(uuo_zero_fpscr())
	__(blr)
_endfn
*/

/* ARM64 Save all 32 FP registers (d0-d31) to the buffer at x0 (must be 256 bytes, 16-byte aligned) */
_exportfn(C(save_fp_context))
    __(cbz x0, 1f)                  /* if x0 == 0, return */
    __(stp d0, d1, [x0, #16*0])     /* store d0, d1 */
    __(stp d2, d3, [x0, #16*1])
    __(stp d4, d5, [x0, #16*2])
    __(stp d6, d7, [x0, #16*3])
    __(stp d8, d9, [x0, #16*4])
    __(stp d10, d11, [x0, #16*5])
    __(stp d12, d13, [x0, #16*6])
    __(stp d14, d15, [x0, #16*7])
    __(stp d16, d17, [x0, #16*8])
    __(stp d18, d19, [x0, #16*9])
    __(stp d20, d21, [x0, #16*10])
    __(stp d22, d23, [x0, #16*11])
    __(stp d24, d25, [x0, #16*12])
    __(stp d26, d27, [x0, #16*13])
    __(stp d28, d29, [x0, #16*14])
    __(stp d30, d31, [x0, #16*15])
1:  __(ret)
_endfn
	
/* PPC CODE
_exportfn(C(save_fp_context))
	__(subi r4,r3,8)
	__(stfdu f0,8(r4))
	__(stfdu f1,8(r4))
	__(stfdu f2,8(r4))
	__(stfdu f3,8(r4))
	__(stfdu f4,8(r4))
	__(stfdu f5,8(r4))
	__(stfdu f6,8(r4))
	__(stfdu f7,8(r4))
	__(stfdu f8,8(r4))
	__(stfdu f9,8(r4))
	__(stfdu f10,8(r4))
	__(stfdu f11,8(r4))
	__(stfdu f12,8(r4))
	__(stfdu f13,8(r4))
	__(stfdu f14,8(r4))
	__(stfdu f15,8(r4))
	__(stfdu f16,8(r4))
	__(stfdu f17,8(r4))
	__(stfdu f18,8(r4))
	__(stfdu f19,8(r4))
	__(stfdu f20,8(r4))
	__(stfdu f21,8(r4))
	__(stfdu f22,8(r4))
	__(stfdu f23,8(r4))
	__(stfdu f24,8(r4))
	__(stfdu f25,8(r4))
	__(stfdu f26,8(r4))
	__(stfdu f27,8(r4))
	__(stfdu f28,8(r4))
	__(stfdu f29,8(r4))
	__(stfdu f30,8(r4))
	__(stfdu f31,8(r4))
	__(mffs f0)
	__(stfd f0,8(r4))
	__(lfd f0,0(r3))
	__(blr)
_endfn
*/

/* ARM64 Restore all 32 FP registers (d0-d31) from the buffer at x0 (must be 256 bytes, 16-byte aligned) */
_exportfn(C(restore_fp_context))
    __(cbz x0, 1f)                  /* if x0 == 0, return */
    __(ldp d0, d1, [x0, #16*0])     /* load d0, d1 */
    __(ldp d2, d3, [x0, #16*1])
    __(ldp d4, d5, [x0, #16*2])
    __(ldp d6, d7, [x0, #16*3])
    __(ldp d8, d9, [x0, #16*4])
    __(ldp d10, d11, [x0, #16*5])
    __(ldp d12, d13, [x0, #16*6])
    __(ldp d14, d15, [x0, #16*7])
    __(ldp d16, d17, [x0, #16*8])
    __(ldp d18, d19, [x0, #16*9])
    __(ldp d20, d21, [x0, #16*10])
    __(ldp d22, d23, [x0, #16*11])
    __(ldp d24, d25, [x0, #16*12])
    __(ldp d26, d27, [x0, #16*13])
    __(ldp d28, d29, [x0, #16*14])
    __(ldp d30, d31, [x0, #16*15])
1:  __(ret)
_endfn

/* PPC CODE
_exportfn(C(restore_fp_context))
	__(mr r4,r3)
	__(lfdu f1,8(r4))
	__(lfdu f2,8(r4))
	__(lfdu f3,8(r4))
	__(lfdu f4,8(r4))
	__(lfdu f5,8(r4))
	__(lfdu f6,8(r4))
	__(lfdu f7,8(r4))
	__(lfdu f8,8(r4))
	__(lfdu f9,8(r4))
	__(lfdu f10,8(r4))
	__(lfdu f11,8(r4))
	__(lfdu f12,8(r4))
	__(lfdu f13,8(r4))
	__(lfdu f14,8(r4))
	__(lfdu f15,8(r4))
	__(lfdu f16,8(r4))
	__(lfdu f17,8(r4))
	__(lfdu f18,8(r4))
	__(lfdu f19,8(r4))
	__(lfdu f20,8(r4))
	__(lfdu f21,8(r4))
	__(lfdu f22,8(r4))
	__(lfdu f23,8(r4))
	__(lfdu f24,8(r4))
	__(lfdu f25,8(r4))
	__(lfdu f26,8(r4))
	__(lfdu f27,8(r4))
	__(lfdu f28,8(r4))
	__(lfdu f29,8(r4))
	__(lfdu f30,8(r4))
	__(lfdu f31,8(r4))
	__(lfd f0,8(r4))
	__(mtfsf 0xff,f0)
	__(lfd f0,0(r3))
	__(blr)
_endfn



/* Atomically store new value (r5) in *r3, if old value == expected. */
/* Return actual old value. */

/* ARM64 Atomically store new value (x2) in *x0, if old value == x1; return actual old value */
_exportfn(C(store_conditional))
    __(dmb ish)                  /* memory barrier */
1:  __(ldxr x3, [x0])            /* load exclusive x3 = *x0 */
    __(cmp x3, x1)               /* compare with expected value (x1) */
    __(b.ne 2f)                  /* if not equal, branch to fail */
    __(stxr w4, x2, [x0])        /* try to store x2 at *x0, w4 = success? */
    __(cbnz w4, 1b)              /* if failed, retry */
    __(dmb ish)                  /* memory barrier */
    __(mov x0, x3)               /* return actual old value */
    __(ret) 
2:  __(mov x0, x3)               /* return actual old value (no store) */
    __(ret)
_endfn

/* PPC CODE
_exportfn(C(store_conditional))
        __(mr r6,r3)
1:      __(lrarx(r3,0,r6))
        __(cmpw r3,r4)
        __(bne- 2f)
        __(strcx(r5,0,r6))
        __(bne- 1b)
        __(isync)
        __(blr)
2:      __(li r0,RESERVATION_DISCHARGE)
        __(strcx(r0,0,r0))
        __(blr)
_endfn

/* Atomically store new_value(r4) in *r3 ;  return previous contents */
/* of *r3. */
/* ARM64 Atomically store new_value (x1) in *x0; return previous contents of *x0 */
_exportfn(C(atomic_swap))
    __(dmb ish)              /* sync */
1:  __(ldxr x2, [x0])        /* load exclusive x2 = *x0 */
    __(stxr w3, x1, [x0])    /* try to store x1 at *x0, w3 = success? */
    __(cbnz w3, 1b)          /* if failed, retry */
    __(dmb ish)              /* isync */
    __(mov x0, x2)           /* return previous value in x0 */
    __(ret)
_endfn

/* PPC CODE
_exportfn(C(atomic_swap))
        __(sync)
1:	__(lrarx(r5,0,r3))
	__(strcx(r4,0,r3))
	__(bne- 1b)
	__(isync)
	__(mr r3,r5)
	__(blr)
_endfn

/* Logior the value in *r3 with the value in r4 (presumably a bitmask with exactly 1 */
/* bit set.)  Return non-zero if any of the bits in that bitmask were already set. */
/* PPC CODE        
_exportfn(C(atomic_ior))
        __(sync)
1:	__(lrarx(r5,0,r3))
        __(or r6,r4,r5)
	__(strcx(r6,0,r3))
	__(bne- 1b)
	__(isync)
	__(and r3,r4,r5)
	__(blr)
_endfn


/* Logand the value in *r3 with the value in r4 (presumably a bitmask with exactly 1 */
/* bit set.)  Return the value now in *r3 (for some value of "now" */

/* ARM64 Atomically AND the value at *x0 with x1 (bitmask), return the new value */
_exportfn(C(atomic_and))
    __(dmb ish)              /* sync */
1:  __(ldxr x2, [x0])        /* load exclusive x2 = *x0 */
    __(and x3, x2, x1)       /* x3 = x2 & x1 */
    __(stxr w4, x3, [x0])    /* try to store x3 at *x0, w4 = success? */
    __(cbnz w4, 1b)          /* if failed, retry */
    __(dmb ish)              /* isync */
    __(mov x0, x3)           /* return new value in x0 */
    __(ret)
_endfn

/* PPC CODE
_exportfn(C(atomic_and))
        __(sync)
1:	__(lrarx(r5,0,r3))
        __(and r6,r4,r5)
	__(strcx(r6,0,r3))
	__(bne- 1b)
	__(isync)
	__(mr r3,r6)
	__(blr)
_endfn
*/                

/* ARM64 */
    __ifdef(`DARWIN')
_exportfn(C(enable_fp_exceptions))
    __(.long 0)
    __(ret)
_endfn
        
_exportfn(C(disable_fp_exceptions))
    __(.long 0)
    __(ret)
_endfn
    __endif

/* PPC CODE
        __ifdef(`DARWIN')
_exportfn(C(enable_fp_exceptions))
        __(.long 0)
        __(blr)
_endfn
        
_exportfn(C(disable_fp_exceptions))
        __(.long 0)
        __(blr)
_endfn
*/

    __ifdef(`DARWIN')
_exportfn(C(darwin_sigreturn))
    .globl C(sigreturn)
    mov x16, #0xB8           /* syscall number for sigreturn on arm64 macOS */
    svc #0                   /* make syscall */
    ret
_endfn
    __endif

/* PPC CODE
_exportfn(C(pseudo_sigreturn))
	__(.long 0)
	__(b C(pseudo_sigreturn))
_endfn
        __endif
*/

/* ARM64 Store all 32 vector (NEON) registers v0-v31 to the buffer at x0 (must be 512 bytes, 16-byte aligned) */
_exportfn(C(put_vector_registers))
    __(cbz x0, 1f)                /* if x0 == 0, return */
    __(st1 {v0.16b-v3.16b}, [x0], #64)   /* store v0-v3, increment x0 by 64 */
    __(st1 {v4.16b-v7.16b}, [x0], #64)
    __(st1 {v8.16b-v11.16b}, [x0], #64)
    __(st1 {v12.16b-v15.16b}, [x0], #64)
    __(st1 {v16.16b-v19.16b}, [x0], #64)
    __(st1 {v20.16b-v23.16b}, [x0], #64)
    __(st1 {v24.16b-v27.16b}, [x0], #64)
    __(st1 {v28.16b-v31.16b}, [x0], #64)
1:  __(ret)
_endfn

/* Copy all 32 Altivec registers (+ VSCR & VRSAVE) to the buffer */
/* in r3.  If the buffer's non-NULL, it's aligned and big enough, */
/* and Altivec is present. */
/* PPC CODE
_exportfn(C(put_vector_registers))
	__(cmpri(r3,0))
	__(li r4,0)
	__(beqlr)
	__(stvx v0,r3,r4)
	__(la r4,16(r4))
	__(stvx v1,r3,r4)
	__(la r4,16(r4))
	__(stvx v2,r3,r4)
	__(la r4,16(r4))
	__(stvx v3,r3,r4)
	__(la r4,16(r4))
	__(stvx v4,r3,r4)
	__(la r4,16(r4))
	__(stvx v5,r3,r4)
	__(la r4,16(r4))
	__(stvx v6,r3,r4)
	__(la r4,16(r4))
	__(stvx v7,r3,r4)
	__(la r4,16(r4))
	__(stvx v8,r3,r4)
	__(la r4,16(r4))
	__(stvx v9,r3,r4)
	__(la r4,16(r4))
	__(stvx v10,r3,r4)
	__(la r4,16(r4))
	__(stvx v11,r3,r4)
	__(la r4,16(r4))
	__(stvx v12,r3,r4)
	__(la r4,16(r4))
	__(stvx v13,r3,r4)
	__(la r4,16(r4))
	__(stvx v14,r3,r4)
	__(la r4,16(r4))
	__(stvx v15,r3,r4)
	__(la r4,16(r4))
	__(stvx v16,r3,r4)
	__(la r4,16(r4))
	__(stvx v17,r3,r4)
	__(la r4,16(r4))
	__(stvx v18,r3,r4)
	__(la r4,16(r4))
	__(stvx v19,r3,r4)
	__(la r4,16(r4))
	__(stvx v20,r3,r4)
	__(la r4,16(r4))
	__(stvx v21,r3,r4)
	__(la r4,16(r4))
	__(stvx v22,r3,r4)
	__(la r4,16(r4))
	__(stvx v23,r3,r4)
	__(la r4,16(r4))
	__(stvx v24,r3,r4)
	__(la r4,16(r4))
	__(stvx v25,r3,r4)
	__(la r4,16(r4))
	__(stvx v26,r3,r4)
	__(la r4,16(r4))
	__(stvx v27,r3,r4)
	__(la r4,16(r4))
	__(stvx v28,r3,r4)
	__(la r4,16(r4))
	__(stvx v29,r3,r4)
	__(la r4,16(r4))
	__(stvx v30,r3,r4)
	__(la r4,16(r4))
	__(stvx v31,r3,r4)
	__(la r4,16(r4))
	__(mfvscr v0)
	__(stvx v0,r3,r4)
	__(mfspr r5,256)
	__(stw r5,8(r4))
	__(blr)
_endfn
*/

/* ARM64 Load all 32 vector (NEON) registers v0-v31 from the buffer at x0 (must be 512 bytes, 16-byte aligned) */
_exportfn(C(get_vector_registers))
    __(cbz x0, 1f)                       /* if x0 == 0, return */
    __(ld1 {v0.16b-v3.16b}, [x0], #64)   /* load v0-v3, increment x0 by 64 */
    __(ld1 {v4.16b-v7.16b}, [x0], #64)
    __(ld1 {v8.16b-v11.16b}, [x0], #64)
    __(ld1 {v12.16b-v15.16b}, [x0], #64)
    __(ld1 {v16.16b-v19.16b}, [x0], #64)
    __(ld1 {v20.16b-v23.16b}, [x0], #64)
    __(ld1 {v24.16b-v27.16b}, [x0], #64)
    __(ld1 {v28.16b-v31.16b}, [x0], #64)
1:  __(ret)
_endfn

/* PPC CODE
_exportfn(C(get_vector_registers))
	__(cmpri(r3,0))
	__(li r4,32*16)
	__(beqlr)
	__(lvx v0,r3,r4)
	__(mtvscr v0)
	__(lwz r5,8(r4))
	__(mtspr 256,r5)
	__(la r4,-16(r4))
	__(lvx v31,r3,r4)
	__(la r4,-16(r4))
	__(lvx v30,r3,r4)
	__(la r4,-16(r4))
	__(lvx v29,r3,r4)
	__(la r4,-16(r4))
	__(lvx v28,r3,r4)
	__(la r4,-16(r4))
	__(lvx v27,r3,r4)
	__(la r4,-16(r4))
	__(lvx v26,r3,r4)
	__(la r4,-16(r4))
	__(lvx v25,r3,r4)
	__(la r4,-16(r4))
	__(lvx v24,r3,r4)
	__(la r4,-16(r4))
	__(lvx v23,r3,r4)
	__(la r4,-16(r4))
	__(lvx v22,r3,r4)
	__(la r4,-16(r4))
	__(lvx v21,r3,r4)
	__(la r4,-16(r4))
	__(lvx v20,r3,r4)
	__(la r4,-16(r4))
	__(lvx v19,r3,r4)
	__(la r4,-16(r4))
	__(lvx v18,r3,r4)
	__(la r4,-16(r4))
	__(lvx v17,r3,r4)
	__(la r4,-16(r4))
	__(lvx v16,r3,r4)
	__(la r4,-16(r4))
	__(lvx v15,r3,r4)
	__(la r4,-16(r4))
	__(lvx v14,r3,r4)
	__(la r4,-16(r4))
	__(lvx v13,r3,r4)
	__(la r4,-16(r4))
	__(lvx v12,r3,r4)
	__(la r4,-16(r4))
	__(lvx v11,r3,r4)
	__(la r4,-16(r4))
	__(lvx v10,r3,r4)
	__(la r4,-16(r4))
	__(lvx v9,r3,r4)
	__(la r4,-16(r4))
	__(lvx v8,r3,r4)
	__(la r4,-16(r4))
	__(lvx v7,r3,r4)
	__(la r4,-16(r4))
	__(lvx v6,r3,r4)
	__(la r4,-16(r4))
	__(lvx v5,r3,r4)
	__(la r4,-16(r4))
	__(lvx v4,r3,r4)
	__(la r4,-16(r4))
	__(lvx v3,r3,r4)
	__(la r4,-16(r4))
	__(lvx v2,r3,r4)
	__(la r4,-16(r4))
	__(lvx v1,r3,r4)
	__(la r4,-16(r4))
	__(lvx v0,r3,r4)
	__(blr)
_endfn
	_endfile
/***** END IMPORT FROM ppc-asmutils.s *****/