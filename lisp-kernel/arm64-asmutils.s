

    include(lisp.s)

    _beginfile


/* Logior the value in *r0 with the value in r1 (presumably a bitmask with exactly 1 */
/* bit set.)  Return non-zero if any of the bits in that bitmask were already set. */
        
_exportfn(C(atomic_ior))
    __(nop)
    __(ret)
_endfn

/* Atomically store new_value(r1) in *r0 ;  return previous contents */
/* of *r0. */

_exportfn(C(atomic_swap))
    __(nop)
    __(ret)
_endfn

_exportfn(C(current_stack_pointer))
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