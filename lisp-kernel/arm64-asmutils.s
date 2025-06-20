

    include(lisp.s)

    _beginfile

/* Force data from x0, size x1 into the icache */        
_exportfn(C(flush_cache_lines))
    __(nop)
    __(ret)
_endfn

/* this should be inlined so the SP is the actual stack pointer, not the one for this function call */
_exportfn(C(current_stack_pointer))
	__(mov x0, sp)
	__(ret)
_endfn

_exportfn(C(noop))
	__(ret)
_endfn

_exportfn(C(touch_page))
    __(mov x0, #0) // return false
    __(ret)
_endfn

/* Logior the value in *r0 with the value in r1 (presumably a bitmask with exactly 1 */
/* bit set.)  Return non-zero if any of the bits in that bitmask were already set. */
        
_exportfn(C(atomic_ior))
    // spinlock
1:  __(ldxr x2, [x0])
    __(orr x3, x2, x1)
    __(stxr w4, x3, [x0])
    __(cbnz w4, 1b)          // if store failed, retry
    __(and x0, x2, x1)      // return the bits that were already set
    __(ret)
_endfn

_exportfn(C(atomic_and))
    __(nop)
    __(ret)
_endfn

/* Atomically store new_value(r1) in *r0 ;  return previous contents */
/* of *r0. */

_exportfn(C(atomic_swap))
    __(nop)
    __(ret)
_endfn 

_exportfn(C(darwin_sigreturn))
	__(nop)         // should be a syscall
	__(ret)			/* shouldn't return */
_endfn

_exportfn(C(get_vector_registers))
    __(nop)
    __(ret)
_endfn

_exportfn(C(put_vector_registers))
    __(nop)
    __(ret)
_endfn       

_exportfn(C(restore_fp_context))
    __(nop)
    __(ret)
_endfn    

_exportfn(C(save_fp_context))
    __(nop)
    __(ret)
_endfn

_exportfn(C(store_conditional))
    __(nop)
    __(ret)
_endfn

    _endfile
