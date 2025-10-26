    include(lisp.s)

    _beginfile

// Atomically OR the value at *x0 with x1 (bitmask), return nonzero if any bits were already set.
_exportfn(C(atomic_ior))
    __(dmb ish)              // sync
1:  __(ldxr x2, [x0])        // load exclusive x2 = *x0
    __(orr x3, x2, x1)       // x3 = x2 | x1
    __(stxr w4, x3, [x0])    // try to store x3 at *x0, w4 = success?
    __(cbnz w4, 1b)          // if failed, retry
    __(dmb ish)              // isync
    __(and x0, x2, x1)       // x0 = x2 & x1 (return nonzero if any bits were already set)
    __(ret)
_endfn

// Atomically AND the value at *x0 with x1 (bitmask), return the new value
_exportfn(C(atomic_and))
    __(dmb ish)              // sync
1:  __(ldxr x2, [x0])        // load exclusive x2 = *x0
    __(and x3, x2, x1)       // x3 = x2 & x1
    __(stxr w4, x3, [x0])    // try to store x3 at *x0, w4 = success?
    __(cbnz w4, 1b)          // if failed, retry
    __(dmb ish)              // isync
    __(mov x0, x3)           // return new value in x0
    __(ret)
_endfn

// Atomically store new_value (x1) in *x0; return previous contents of *x0
_exportfn(C(atomic_swap))
    __(dmb ish)              // sync
1:  __(ldxr x2, [x0])        // load exclusive x2 = *x0
    __(stxr w3, x1, [x0])    // try to store x1 at *x0, w3 = success?
    __(cbnz w3, 1b)          // if failed, retry
    __(dmb ish)              // isync
    __(mov x0, x2)           // return previous value in x0
    __(ret)
_endfn

_exportfn(C(current_stack_pointer))
    __(mov x0, sp)
    __(ret)
_endfn
	

// Save all 32 FP registers (d0-d31) to the buffer at x0 (must be 256 bytes, 16-byte aligned)
_exportfn(C(save_fp_context))
    __(cbz x0, 1f)                  // if x0 == 0, return
    __(stp d0, d1, [x0, #16*0])     // store d0, d1
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


// Restore all 32 FP registers (d0-d31) from the buffer at x0 (must be 256 bytes, 16-byte aligned)
_exportfn(C(restore_fp_context))
    __(cbz x0, 1f)                  // if x0 == 0, return
    __(ldp d0, d1, [x0, #16*0])     // load d0, d1
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


/* Atomically store new value (r5) in *r3, if old value == expected. */
/* Return actual old value. */


// Atomically store new value (x2) in *x0, if old value == x1; return actual old value
_exportfn(C(store_conditional))
    __(dmb ish)                  // memory barrier
1:  __(ldxr x3, [x0])           // load exclusive x3 = *x0
    __(cmp x3, x1)              // compare with expected value (x1)
    __(b.ne 2f)                 // if not equal, branch to fail
    __(stxr w4, x2, [x0])       // try to store x2 at *x0, w4 = success?
    __(cbnz w4, 1b)             // if failed, retry
    __(dmb ish)                 // memory barrier
    __(mov x0, x3)              // return actual old value
    __(ret)
2:  __(mov x0, x3)              // return actual old value (no store)
    __(ret)
_endfn


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



        __ifdef(`DARWIN')
_exportfn(C(darwin_sigreturn))
        .globl C(sigreturn)
        mov x16, #0xB8           // syscall number for sigreturn on arm64 macOS
        svc #0                   // make syscall
        ret
_endfn
        __endif

// Store all 32 vector (NEON) registers v0-v31 to the buffer at x0 (must be 512 bytes, 16-byte aligned)
_exportfn(C(put_vector_registers))
    __(cbz x0, 1f)                // if x0 == 0, return
    __(st1 {v0.16b-v3.16b}, [x0], #64)   // store v0-v3, increment x0 by 64
    __(st1 {v4.16b-v7.16b}, [x0], #64)
    __(st1 {v8.16b-v11.16b}, [x0], #64)
    __(st1 {v12.16b-v15.16b}, [x0], #64)
    __(st1 {v16.16b-v19.16b}, [x0], #64)
    __(st1 {v20.16b-v23.16b}, [x0], #64)
    __(st1 {v24.16b-v27.16b}, [x0], #64)
    __(st1 {v28.16b-v31.16b}, [x0], #64)
1:  __(ret)
_endfn


// Load all 32 vector (NEON) registers v0-v31 from the buffer at x0 (must be 512 bytes, 16-byte aligned)
_exportfn(C(get_vector_registers))
    __(cbz x0, 1f)                // if x0 == 0, return
    __(ld1 {v0.16b-v3.16b}, [x0], #64)   // load v0-v3, increment x0 by 64
    __(ld1 {v4.16b-v7.16b}, [x0], #64)
    __(ld1 {v8.16b-v11.16b}, [x0], #64)
    __(ld1 {v12.16b-v15.16b}, [x0], #64)
    __(ld1 {v16.16b-v19.16b}, [x0], #64)
    __(ld1 {v20.16b-v23.16b}, [x0], #64)
    __(ld1 {v24.16b-v27.16b}, [x0], #64)
    __(ld1 {v28.16b-v31.16b}, [x0], #64)
1:  __(ret)
_endfn

    _endfile