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

#include "lisp.h"
#include "lisp-exceptions.h"
#include "lisp_globals.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

#ifdef DARWIN
#include <sys/mman.h>
#include <mach/mach.h>
#ifndef SA_NODEFER
#define SA_NODEFER 0
#endif
#include <sysexits.h>

/* a distinguished UUO at a distinguished address */
extern void pseudo_sigreturn(ExceptionInformation *);
#endif

#ifndef _FPU_RESERVED
#define _FPU_RESERVED 0xffffff00
#endif

#include "threads.h"

// #define MSR_FE0_MASK (((unsigned)0x80000000)>>20)
// #define MSR_FE1_MASK (((unsigned)0x80000000)>>23)
// #define MSR_FE0_FE1_MASK (MSR_FE0_MASK|MSR_FE1_MASK)
extern void enable_fp_exceptions(void);
extern void disable_fp_exceptions(void);

/*
  Handle exceptions.

*/

extern LispObj lisp_nil;

extern natural lisp_heap_gc_threshold;
extern Boolean grow_dynamic_area(natural);

int page_size = 0x4000;  // 16KB page size on Darwin. Future Linux ARM64 port??
int log2_page_size = 14; // 1 << 14 = 16KB

/*
  If the PC is pointing to an allocation trap, the previous instruction
  must have decremented allocptr.  Return the non-zero amount by which
  allocptr was decremented.
*/
signed_natural allocptr_displacement(ExceptionInformation *xp)
{
    assert(0);
}

/*
  A cons cell's been successfully allocated, but the allocptr's
  still tagged (as fulltag_cons, of course.)  Emulate any instructions
  that might follow the allocation (stores to the car or cdr, an
  assignment to the "result" gpr) that take place while the allocptr's
  tag is non-zero, advancing over each such instruction.  When we're
  done, the cons cell will be allocated and initialized, the result
  register will point to it, the allocptr will be untagged, and
  the PC will point past the instruction that clears the allocptr's
  tag.
*/
void finish_allocating_cons(ExceptionInformation *xp)
{
    pc program_counter = xpPC(xp);
    opcode instr;
    LispObj cur_allocptr = xpGPR(xp, allocptr);
    cons *c = (cons *)ptr_from_lispobj(untag(cur_allocptr));
    int target_reg;

    while (1)
    {
        instr = *program_counter++;

        if (instr == UNTAG_ALLOCPTR_INSTRUCTION)
        {
            xpGPR(xp, allocptr) = untag(cur_allocptr);
            xpPC(xp) = program_counter;
            return;
        }

        switch (instr & STORE_CXR_ALLOCPTR_MASK)
        {
        case STORE_CAR_ALLOCPTR_INSTRUCTION:
            c->car = xpGPR(xp, RT_field(instr));
            break;
        case STORE_CDR_ALLOCPTR_INSTRUCTION:
            c->cdr = xpGPR(xp, RT_field(instr));
            break;
        default:
            /* Assume that this is an assignment: {rt/ra} <- allocptr.
               There are several equivalent instruction forms
               that might have that effect; just assign to target here.
            */
            if (major_opcode_p(instr, major_opcode_X31))
            {
                target_reg = RA_field(instr);
            }
            else
            {
                target_reg = RT_field(instr);
            }
            xpGPR(xp, target_reg) = cur_allocptr;
            break;
        }
    }
}

/*
  We were interrupted in the process of allocating a uvector; we
  survived the allocation trap, and allocptr is tagged as fulltag_misc.
  Emulate any instructions which store a header into the uvector,
  assign the value of allocptr to some other register, and clear
  allocptr's tag.  Don't expect/allow any other instructions in
  this environment.
*/
void finish_allocating_uvector(ExceptionInformation *xp)
{
    assert(0);
}

Boolean
allocate_object(ExceptionInformation *xp,
                natural bytes_needed,
                signed_natural disp_from_allocptr,
                TCR *tcr)
{
    area *a = active_dynamic_area;

    /* Maybe do an EGC */
    if (a->older && lisp_global(OLDEST_EPHEMERAL))
    {
        if (((a->active) - (a->low)) >= a->threshold)
        {
            gc_from_xp(xp, 0L);
        }
    }

    /* Life is pretty simple if we can simply grab a segment
       without extending the heap.
    */
    if (new_heap_segment(xp, bytes_needed, false, tcr, NULL))
    {
        xpGPR(xp, allocptr) += disp_from_allocptr;
#ifdef DEBUG
        fprintf(dbgout, "New heap segment for #x%x, no GC: #x%x/#x%x, vsp = #x%x\n",
                tcr, xpGPR(xp, allocbase), tcr->last_allocptr, xpGPR(xp, vsp));
#endif
        return true;
    }

    /* It doesn't make sense to try a full GC if the object
       we're trying to allocate is larger than everything
       allocated so far.
    */
    if ((lisp_global(HEAP_END) - lisp_global(HEAP_START)) > bytes_needed)
    {
        untenure_from_area(tenured_area); /* force a full GC */
        gc_from_xp(xp, 0L);
    }

    /* Try again, growing the heap if necessary */
    if (new_heap_segment(xp, bytes_needed, true, tcr, NULL))
    {
        xpGPR(xp, allocptr) += disp_from_allocptr;
#ifdef DEBUG
        fprintf(dbgout, "New heap segment for #x%x after GC: #x%x/#x%x\n",
                tcr, xpGPR(xp, allocbase), tcr->last_allocptr);
#endif
        return true;
    }

    return false;
}

#ifndef XNOMEM
#define XNOMEM 10
#endif

void update_bytes_allocated(TCR *tcr, void *cur_allocptr)
{
    BytePtr
        last = (BytePtr)tcr->last_allocptr,
        current = (BytePtr)cur_allocptr;
    if (last && (cur_allocptr != ((void *)VOID_ALLOCPTR)))
    {
        tcr->bytes_allocated += last - current;
    }
    tcr->last_allocptr = 0;
}

void lisp_allocation_failure(ExceptionInformation *xp, TCR *tcr, natural bytes_needed)
{
    /* Couldn't allocate the object.  If it's smaller than some arbitrary
       size (say 128K bytes), signal a "chronically out-of-memory" condition;
       else signal a "allocation request failed" condition.
    */
    xpGPR(xp, allocptr) = xpGPR(xp, allocbase) = VOID_ALLOCPTR;
    handle_error(xp, bytes_needed < (128 << 10) ? XNOMEM : error_alloc_failed, 0, 0, xpPC(xp));
}

/*
  Allocate a large list, where "large" means "large enough to
  possibly trigger the EGC several times if this was done
  by individually allocating each CONS."  The number of
  ocnses in question is in arg_z; on successful return,
  the list will be in arg_z
*/

Boolean
allocate_list(ExceptionInformation *xp, TCR *tcr)
{
    natural
        nconses = (unbox_fixnum(xpGPR(xp, arg_z))),
        bytes_needed = (nconses << dnode_shift);
    LispObj
        prev = lisp_nil,
        current,
        initial = xpGPR(xp, arg_y);

    if (nconses == 0)
    {
        /* Silly case */
        xpGPR(xp, arg_z) = lisp_nil;
        xpGPR(xp, allocptr) = lisp_nil;
        return true;
    }
    update_bytes_allocated(tcr, (void *)(void *)tcr->save_allocptr);
    if (allocate_object(xp, bytes_needed, (-bytes_needed) + fulltag_cons, tcr))
    {
        for (current = xpGPR(xp, allocptr);
             nconses;
             prev = current, current += dnode_size, nconses--)
        {
            deref(current, 0) = prev;
            deref(current, 1) = initial;
        }
        xpGPR(xp, arg_z) = prev;
        xpGPR(xp, arg_y) = xpGPR(xp, allocptr);
        xpGPR(xp, allocptr) -= fulltag_cons;
    }
    else
    {
        lisp_allocation_failure(xp, tcr, bytes_needed);
    }
    return true;
}

OSStatus
handle_alloc_trap(ExceptionInformation *xp, TCR *tcr)
{
    assert(0);
}

natural gc_deferred = 0, full_gc_deferred = 0;

signed_natural
flash_freeze(TCR *tcr, signed_natural param)
{
    return 0;
}

OSStatus
handle_gc_trap(ExceptionInformation *xp, TCR *tcr)
{
    LispObj
        selector = xpGPR(xp, imm0),
        arg = xpGPR(xp, imm1);
    area *a = active_dynamic_area;
    Boolean egc_was_enabled = (a->older != NULL);
    natural gc_previously_deferred = gc_deferred;

    switch (selector)
    {
    case GC_TRAP_FUNCTION_EGC_CONTROL:
        egc_control(arg != 0, a->active);
        xpGPR(xp, arg_z) = lisp_nil + (egc_was_enabled ? t_offset : 0);
        break;

    case GC_TRAP_FUNCTION_CONFIGURE_EGC:
        a->threshold = unbox_fixnum(xpGPR(xp, arg_x));
        g1_area->threshold = unbox_fixnum(xpGPR(xp, arg_y));
        g2_area->threshold = unbox_fixnum(xpGPR(xp, arg_z));
        xpGPR(xp, arg_z) = lisp_nil + t_offset;
        break;

    case GC_TRAP_FUNCTION_SET_LISP_HEAP_THRESHOLD:
        if (((signed_natural)arg) > 0)
        {
            lisp_heap_gc_threshold =
                align_to_power_of_2((arg - 1) +
                                        (heap_segment_size - 1),
                                    log2_heap_segment_size);
        }
        /* fall through */
    case GC_TRAP_FUNCTION_GET_LISP_HEAP_THRESHOLD:
        xpGPR(xp, imm0) = lisp_heap_gc_threshold;
        break;

    case GC_TRAP_FUNCTION_USE_LISP_HEAP_THRESHOLD:
        /*  Try to put the current threshold in effect.  This may
            need to disable/reenable the EGC. */
        untenure_from_area(tenured_area);
        resize_dynamic_heap(a->active, lisp_heap_gc_threshold);
        if (egc_was_enabled)
        {
            if ((a->high - a->active) >= a->threshold)
            {
                tenure_to_area(tenured_area);
            }
        }
        xpGPR(xp, imm0) = lisp_heap_gc_threshold;
        break;

    case GC_TRAP_FUNCTION_ENSURE_STATIC_CONSES:
        ensure_static_conses(xp, tcr, 32768);
        break;

    case GC_TRAP_FUNCTION_FLASH_FREEZE:
        untenure_from_area(tenured_area);
        gc_like_from_xp(xp, flash_freeze, 0);
        a->active = (BytePtr)align_to_power_of_2(a->active, log2_page_size);
        tenured_area->static_dnodes = area_dnode(a->active, a->low);
        if (egc_was_enabled)
        {
            tenure_to_area(tenured_area);
        }
        xpGPR(xp, imm0) = tenured_area->static_dnodes << dnode_shift;
        break;

    default:
        update_bytes_allocated(tcr, (void *)ptr_from_lispobj(xpGPR(xp, allocptr)));

        if (selector == GC_TRAP_FUNCTION_IMMEDIATE_GC)
        {
            if (!full_gc_deferred)
            {
                gc_from_xp(xp, 0L);
                break;
            }
            /* Tried to do a full GC when gc was disabled.  That failed,
               so try full GC now */
            selector = GC_TRAP_FUNCTION_GC;
        }

        if (egc_was_enabled)
        {
            egc_control(false, (BytePtr)a->active);
        }
        gc_from_xp(xp, 0L);
        if (gc_deferred > gc_previously_deferred)
        {
            full_gc_deferred = 1;
        }
        else
        {
            full_gc_deferred = 0;
        }
        if (selector > GC_TRAP_FUNCTION_GC)
        {
            if (selector & GC_TRAP_FUNCTION_IMPURIFY)
            {
                impurify_from_xp(xp, 0L);
                /*        nrs_GC_EVENT_STATUS_BITS.vcell |= gc_integrity_check_bit; */
                lisp_global(OLDSPACE_DNODE_COUNT) = 0;
                gc_from_xp(xp, 0L);
            }
            if (selector & GC_TRAP_FUNCTION_PURIFY)
            {
                purify_from_xp(xp, 0L);
                lisp_global(OLDSPACE_DNODE_COUNT) = 0;
                gc_from_xp(xp, 0L);
            }
            if (selector & GC_TRAP_FUNCTION_SAVE_APPLICATION)
            {
                OSErr err;
                extern OSErr save_application(unsigned, Boolean);
                TCR *tcr = TCR_FROM_TSD(xpGPR(xp, rcontext));
                area *vsarea = tcr->vs_area;

                nrs_TOPLFUNC.vcell = *((LispObj *)(vsarea->high) - 1);
                err = save_application(arg, egc_was_enabled);
                if (err == noErr)
                {
                    _exit(0);
                }
                fatal_oserr(": save_application", err);
            }
            switch (selector)
            {

            case GC_TRAP_FUNCTION_FREEZE:
                a->active = (BytePtr)align_to_power_of_2(a->active, log2_page_size);
                tenured_area->static_dnodes = area_dnode(a->active, a->low);
                xpGPR(xp, imm0) = tenured_area->static_dnodes << dnode_shift;
                break;
            default:
                break;
            }
        }

        if (egc_was_enabled)
        {
            egc_control(true, NULL);
        }
        break;
    }

    adjust_exception_pc(xp, 4);
    return 0;
}

void signal_stack_soft_overflow(ExceptionInformation *xp, unsigned reg)
{
    /* The cstack just overflowed.  Force the current thread's
       control stack to do so until all stacks are well under their overflow
       limits.
    */

#if 0
  lisp_global(CS_OVERFLOW_LIMIT) = CS_OVERFLOW_FORCE_LIMIT; /* force unsigned traps to fail */
#endif
    handle_error(xp, error_stack_overflow, reg, 0, xpPC(xp));
}

/*
  Lower (move toward 0) the "end" of the soft protected area associated
  with a by a page, if we can.
*/

void adjust_soft_protection_limit(area *a)
{
    char *proposed_new_soft_limit = a->softlimit - 4096;
    protected_area_ptr p = a->softprot;

    if (proposed_new_soft_limit >= (p->start + 16384))
    {
        p->end = proposed_new_soft_limit;
        p->protsize = p->end - p->start;
        a->softlimit = proposed_new_soft_limit;
    }
    protect_area(p);
}

void restore_soft_stack_limit(unsigned stkreg)
{
    assert(0);
}

/* Maybe this'll work someday.  We may have to do something to
   make the thread look like it's not handling an exception */
void reset_lisp_process(ExceptionInformation *xp)
{
    TCR *tcr = TCR_FROM_TSD(xpGPR(xp, rcontext));
    catch_frame *last_catch = (catch_frame *)ptr_from_lispobj(untag(tcr->catch_top));

    tcr->save_allocptr = (void *)ptr_from_lispobj(xpGPR(xp, allocptr));
    tcr->save_allocbase = (void *)ptr_from_lispobj(xpGPR(xp, allocbase));

    tcr->save_vsp = (LispObj *)ptr_from_lispobj(((lisp_frame *)ptr_from_lispobj(last_catch->csp))->savevsp);
    tcr->save_tsp = (LispObj *)ptr_from_lispobj((LispObj)ptr_to_lispobj(last_catch)) - (2 * node_size); /* account for TSP header */

    start_lisp(tcr, 1);
}

void platform_new_heap_segment(ExceptionInformation *xp, TCR *tcr, BytePtr low, BytePtr high)
{
    tcr->last_allocptr = (void *)high;
    xpGPR(xp, allocptr) = (LispObj)high;
    xpGPR(xp, allocbase) = (LispObj)low;
}

void update_area_active(area **aptr, BytePtr value)
{
    area *a = *aptr;
    for (; a; a = a->older)
    {
        if ((a->low <= value) && (a->high >= value))
            break;
    };
    if (a == NULL)
        Bug(NULL, "Can't find active area");
    a->active = value;
    *aptr = a;

    for (a = a->younger; a; a = a->younger)
    {
        a->active = a->high;
    }
}

LispObj *
tcr_frame_ptr(TCR *tcr)
{
    assert(0);
}

void normalize_tcr(ExceptionInformation *xp, TCR *tcr, Boolean is_other_tcr)
{
    assert(0);
}

TCR *gc_tcr = NULL;

/* Suspend and "normalize" other tcrs, then call a gc-like function
   in that context.  Resume the other tcrs, then return what the
   function returned */

signed_natural
gc_like_from_xp(ExceptionInformation *xp,
                signed_natural (*fun)(TCR *, signed_natural),
                signed_natural param)
{
    TCR *tcr = TCR_FROM_TSD(xpGPR(xp, rcontext)), *other_tcr;
    int result;
    signed_natural inhibit;

    suspend_other_threads(true);
    inhibit = (signed_natural)(lisp_global(GC_INHIBIT_COUNT));
    if (inhibit != 0)
    {
        if (inhibit > 0)
        {
            lisp_global(GC_INHIBIT_COUNT) = (LispObj)(-inhibit);
        }
        resume_other_threads(true);
        gc_deferred++;
        return 0;
    }
    gc_deferred = 0;

    gc_tcr = tcr;

    xpGPR(xp, allocptr) = VOID_ALLOCPTR;
    xpGPR(xp, allocbase) = VOID_ALLOCPTR;

    normalize_tcr(xp, tcr, false);

    for (other_tcr = tcr->next; other_tcr != tcr; other_tcr = other_tcr->next)
    {
        if (other_tcr->pending_exception_context)
        {
            other_tcr->gc_context = other_tcr->pending_exception_context;
        }
        else if (other_tcr->valence == TCR_STATE_LISP)
        {
            other_tcr->gc_context = other_tcr->suspend_context;
        }
        else
        {
            /* no pending exception, didn't suspend in lisp state:
           must have executed a synchronous ff-call.
            */
            other_tcr->gc_context = NULL;
        }
        normalize_tcr(other_tcr->gc_context, other_tcr, true);
    }

    result = fun(tcr, param);

    other_tcr = tcr;
    do
    {
        other_tcr->gc_context = NULL;
        other_tcr = other_tcr->next;
    } while (other_tcr != tcr);

    gc_tcr = NULL;

    resume_other_threads(true);

    return result;
}

/* Returns #bytes freed by invoking GC */

signed_natural
gc_from_tcr(TCR *tcr, signed_natural param)
{
    area *a;
    BytePtr oldfree, newfree;
    BytePtr oldend, newend;

#ifdef DEBUG
    fprintf(dbgout, "Start GC  in 0x%lx\n", tcr);
#endif
    a = active_dynamic_area;
    oldend = a->high;
    oldfree = a->active;
    gc(tcr, param);
    newfree = a->active;
    newend = a->high;
#if 0
  fprintf(dbgout, "End GC  in 0x%lx\n", tcr);
#endif
    return ((oldfree - newfree) + (newend - oldend));
}

signed_natural
gc_from_xp(ExceptionInformation *xp, signed_natural param)
{
    signed_natural status = gc_like_from_xp(xp, gc_from_tcr, param);

    freeGCptrs();
    return status;
}

signed_natural
purify_from_xp(ExceptionInformation *xp, signed_natural param)
{
    return gc_like_from_xp(xp, purify, param);
}

signed_natural
impurify_from_xp(ExceptionInformation *xp, signed_natural param)
{
    return gc_like_from_xp(xp, impurify, param);
}

protection_handler
    *protection_handlers[] = {
        do_spurious_wp_fault,
        do_soft_stack_overflow,
        do_soft_stack_overflow,
        do_soft_stack_overflow,
        do_hard_stack_overflow,
        do_hard_stack_overflow,
        do_hard_stack_overflow};

Boolean
is_write_fault(ExceptionInformation *xp, siginfo_t *info)
{
    assert(0);
}

OSStatus
handle_protection_violation(ExceptionInformation *xp, siginfo_t *info, TCR *tcr, int old_valence)
{
    assert(0);
}

OSStatus
do_hard_stack_overflow(ExceptionInformation *xp, protected_area_ptr area, BytePtr addr)
{
#ifdef SUPPORT_PRAGMA_UNUSED
#pragma unused(area, addr)
#endif
    reset_lisp_process(xp);
    return -1;
}

extern area *
allocate_vstack(natural useable); /* This is in "pmcl-kernel.c" */

extern area *
allocate_tstack(natural useable); /* This is in "pmcl-kernel.c" */

#ifdef EXTEND_VSTACK
Boolean
catch_frame_p(lisp_frame *spPtr)
{
    catch_frame *catch = (catch_frame *)untag(lisp_global(CATCH_TOP));

    for (; catch; catch = (catch_frame *)untag(catch->link))
    {
        if (spPtr == ((lisp_frame *)catch->csp))
        {
            return true;
        }
    }
    return false;
}
#endif

Boolean
unwind_protect_cleanup_frame_p(lisp_frame *spPtr)
{
    if ((spPtr->savevsp == (LispObj)NULL) || /* The frame to where the unwind-protect will return */
        (((spPtr->backlink)->savevsp) == (LispObj)NULL))
    { /* The frame that returns to the kernel  from the cleanup form */
        return true;
    }
    else
    {
        return false;
    }
}

Boolean
lexpr_entry_frame_p(lisp_frame *spPtr)
{
    LispObj savelr = spPtr->savelr;
    LispObj lexpr_return = (LispObj)lisp_global(LEXPR_RETURN);
    LispObj lexpr_return1v = (LispObj)lisp_global(LEXPR_RETURN1V);
    LispObj ret1valn = (LispObj)lisp_global(RET1VALN);

    return (savelr == lexpr_return1v) ||
           (savelr == lexpr_return) ||
           ((savelr == ret1valn) &&
            (((spPtr->backlink)->savelr) == lexpr_return));
}

Boolean
lisp_frame_p(lisp_frame *spPtr)
{
    LispObj savefn;
    /* We can't just look at the size of the stack frame under the EABI
       calling sequence, but that's the first thing to check. */
    if (((lisp_frame *)spPtr->backlink) != (spPtr + 1))
    {
        return false;
    }
    savefn = spPtr->savefn;
    return (savefn == 0) || (fulltag_of(savefn) == fulltag_misc);
}

int ffcall_overflow_count = 0;

/* Find a frame that is neither a catch frame nor one of the
   lexpr_entry frames We don't check for non-lisp frames here because
   we'll always stop before we get there due to a dummy lisp frame
   pushed by .SPcallback that masks out the foreign frames.  The one
   exception is that there is a non-lisp frame without a valid VSP
   while in the process of ppc-ff-call. We recognize that because its
   savelr is NIL.  If the saved VSP itself is 0 or the savevsp in the
   next frame is 0, then we're executing an unwind-protect cleanup
   form, and the top stack frame belongs to its (no longer extant)
   catch frame.  */

#ifdef EXTEND_VSTACK
lisp_frame *
find_non_catch_frame_from_xp(ExceptionInformation *xp)
{
    lisp_frame *spPtr = (lisp_frame *)xpGPR(xp, sp);
    if ((((natural)spPtr) + sizeof(lisp_frame)) != ((natural)(spPtr->backlink)))
    {
        ffcall_overflow_count++; /* This is mostly so I can breakpoint here */
    }
    for (; !lisp_frame_p(spPtr) || /* In the process of ppc-ff-call */
           unwind_protect_cleanup_frame_p(spPtr) ||
           catch_frame_p(spPtr) ||
           lexpr_entry_frame_p(spPtr);)
    {
        spPtr = spPtr->backlink;
    };
    return spPtr;
}
#endif

#ifdef EXTEND_VSTACK
Boolean
db_link_chain_in_area_p(area *a)
{
    LispObj *db = (LispObj *)lisp_global(DB_LINK),
            *high = (LispObj *)a->high,
            *low = (LispObj *)a->low;
    for (; db; db = (LispObj *)*db)
    {
        if ((db >= low) && (db < high))
            return true;
    };
    return false;
}
#endif

/* Note: CURRENT_VS (CURRENT_TS) is always either the area containing
  the current value of VSP (TSP) or an older area.  */

OSStatus
do_vsp_overflow(ExceptionInformation *xp, BytePtr addr)
{
    TCR *tcr = TCR_FROM_TSD(xpGPR(xp, rcontext));
    area *a = tcr->vs_area;
    protected_area_ptr vsp_soft = a->softprot;
    unprotect_area(vsp_soft);
    signal_stack_soft_overflow(xp, vsp);
    return 0;
}

OSStatus
do_tsp_overflow(ExceptionInformation *xp, BytePtr addr)
{
    assert(0);
}

OSStatus
do_soft_stack_overflow(ExceptionInformation *xp, protected_area_ptr prot_area, BytePtr addr)
{
    /* Trying to write into a guard page on the vstack or tstack.
       Allocate a new stack segment, emulate stwu and stwux for the TSP, and
       signal an error_stack_overflow condition.
        */
    lisp_protection_kind which = prot_area->why;
    Boolean on_TSP = (which == kTSPsoftguard);

    if (on_TSP)
    {
        return do_tsp_overflow(xp, addr);
    }
    else
    {
        return do_vsp_overflow(xp, addr);
    }
}

OSStatus
do_spurious_wp_fault(ExceptionInformation *xp, protected_area_ptr area, BytePtr addr)
{
#ifdef SUPPORT_PRAGMA_UNUSED
#pragma unused(xp, area, addr)
#endif
    return -1;
}

/*
  We have a couple of choices here.  We can simply unprotect the page
  and let the store happen on return, or we can try to emulate writes
  that we know will involve an intergenerational reference.  Both are
  correct as far as EGC constraints go, but the latter approach is
  probably more efficient.  (This only matters in the case where the
  GC runs after this exception handler returns but before the write
  actually happens.  If we didn't emulate node stores here, the EGC
  would scan the newly-writen page, find nothing interesting, and
  run to completion.  This thread will try the write again afer it
  resumes, the page'll be re-protected, and we'll have taken this
  fault twice.  The whole scenario shouldn't happen very often, but
  (having already taken a fault and committed to an mprotect syscall)
  we might as well emulate stores involving intergenerational references,
  since they're pretty easy to identify.

  Note that cases involving two or more threads writing to the same
  page (before either of them can run this handler) is benign: one
  invocation of the handler will just unprotect an unprotected page in
  that case.

  If there are GCs (or any other suspensions of the thread between
  the time that the write fault was detected and the time that the
  exception lock is obtained) none of this stuff happens.
*/

/*
  Return true (and emulate the instruction) iff:
  a) the fault was caused by an "stw rs,d(ra)" or "stwx rs,ra.rb"
     instruction.
  b) RS is a node register (>= fn)
  c) RS is tagged as a cons or vector
  d) RS is in some ephemeral generation.
  This is slightly conservative, since RS may be no younger than the
  EA being written to.
*/
Boolean
is_ephemeral_node_store(ExceptionInformation *xp, BytePtr ea)
{
    assert(0);
}

OSStatus
handle_sigfpe(ExceptionInformation *xp, TCR *tcr)
{
    assert(0);
}

int
    altivec_present = 1;

/* This only tries to implement the "optional" fsqrt and fsqrts
   instructions, which were generally implemented on IBM hardware
   but generally not available on Motorola/Freescale systems.
*/
OSStatus
handle_unimplemented_instruction(ExceptionInformation *xp,
                                 opcode instruction,
                                 TCR *tcr)
{
    assert(0);
}

OSStatus
PMCL_exception_handler(int xnum,
                       ExceptionInformation *xp,
                       TCR *tcr,
                       siginfo_t *info,
                       int old_valence)
{
    OSStatus status = -1;
    pc program_counter;
    opcode instruction = 0;

    program_counter = xpPC(xp);

    if ((xnum == SIGILL) | (xnum == SIGTRAP))
    {
        instruction = *program_counter;
    }

    if (instruction == ALLOC_TRAP_INSTRUCTION)
    {
        status = handle_alloc_trap(xp, tcr);
    }
    else if ((xnum == SIGSEGV) ||
             (xnum == SIGBUS))
    {
        status = handle_protection_violation(xp, info, tcr, old_valence);
    }
    else if (xnum == SIGFPE)
    {
        status = handle_sigfpe(xp, tcr);
    }
    else if ((xnum == SIGILL) || (xnum == SIGTRAP))
    {
        if (instruction == GC_TRAP_INSTRUCTION)
        {
            status = handle_gc_trap(xp, tcr);
        }
        else if (IS_UUO(instruction))
        {
            status = handle_uuo(xp, instruction, program_counter);
        }
        else if (is_conditional_trap(instruction))
        {
            status = handle_trap(xp, instruction, program_counter, info);
        }
        else
        {
            status = handle_unimplemented_instruction(xp, instruction, tcr);
        }
    }
    else if (xnum == SIGNAL_FOR_PROCESS_INTERRUPT)
    {
        tcr->interrupt_pending = 0;
        callback_for_trap(nrs_CMAIN.vcell, xp, 0, TRI_instruction(TO_GT, nargs, 0), 0, 0);
        status = 0;
    }

    return status;
}

void adjust_exception_pc(ExceptionInformation *xp, int delta)
{
    xpPC(xp) += (delta >> 2);
}

/*
  This wants to scan backwards until "where" points to an instruction
   whose major opcode is either 63 (double-float) or 59 (single-float)
*/

OSStatus
handle_fpux_binop(ExceptionInformation *xp, pc where)
{
    assert(0);
}

OSStatus
handle_uuo(ExceptionInformation *xp, opcode the_uuo, pc where)
{
    assert(0);
}

natural
register_codevector_contains_pc(natural lisp_function, pc where)
{
    natural code_vector, size;

    if ((fulltag_of(lisp_function) == fulltag_misc) &&
        (header_subtag(header_of(lisp_function)) == subtag_function))
    {
        code_vector = deref(lisp_function, 1);
        size = header_element_count(header_of(code_vector)) << 2;
        if ((untag(code_vector) < (natural)where) &&
            ((natural)where < (code_vector + size)))
            return (code_vector);
    }

    return (0);
}

/* Callback to lisp to handle a trap. Need to translate the
   PC (where) into one of two forms of pairs:

   1. If PC is in fn or nfn's code vector, use the register number
      of fn or nfn and the index into that function's code vector.
   2. Otherwise use 0 and the pc itself
*/
void callback_for_trap(LispObj callback_macptr, ExceptionInformation *xp, pc where,
                       natural arg1, natural arg2, natural arg3)
{
    assert(0);
}

area *
allocate_no_stack(natural size)
{
#ifdef SUPPORT_PRAGMA_UNUSED
#pragma unused(size)
#endif

    return (area *)NULL;
}

/* callback to (symbol-value cmain) if it is a macptr,
   otherwise report cause and function name to console.
   Returns noErr if exception handled OK */
OSStatus
handle_trap(ExceptionInformation *xp, opcode the_trap, pc where, siginfo_t *info)
{
    assert(0);
}

/* Look at up to TRAP_LOOKUP_TRIES instrs before trap instr for a pattern.
   Stop if subtag_code_vector is encountered. */
unsigned
scan_for_instr(unsigned target, unsigned mask, pc where)
{
    int i = TRAP_LOOKUP_TRIES;

    while (i--)
    {
        unsigned instr = *(--where);
        if (codevec_hdr_p(instr))
        {
            return 0;
        }
        else if (match_instr(instr, mask, target))
        {
            return instr;
        }
    }
    return 0;
}

void non_fatal_error(char *msg)
{
    fprintf(dbgout, "Non-fatal error: %s.\n", msg);
    fflush(dbgout);
}

/* The main opcode.  */

int is_conditional_trap(opcode instr)
{
    unsigned to = TO_field(instr);
    int is_tr = X_opcode_p(instr, major_opcode_X31, minor_opcode_TR);

#ifndef MACOS
    if ((instr == LISP_BREAK_INSTRUCTION) ||
        (instr == QUIET_LISP_BREAK_INSTRUCTION))
    {
        return 1;
    }
#endif
    if (is_tr || major_opcode_p(instr, major_opcode_TRI))
    {
        /* A "tw/td" or "twi/tdi" instruction.  To be unconditional, the
           EQ bit must be set in the TO mask and either the register
           operands (if "tw") are the same or either both of the signed or
           both of the unsigned inequality bits must be set. */
        if (!(to & TO_EQ))
        {
            return 1; /* Won't trap on EQ: conditional */
        }
        if (is_tr && (RA_field(instr) == RB_field(instr)))
        {
            return 0; /* Will trap on EQ, same regs: unconditional */
        }
        if (((to & (TO_LO | TO_HI)) == (TO_LO | TO_HI)) ||
            ((to & (TO_LT | TO_GT)) == (TO_LT | TO_GT)))
        {
            return 0; /* Will trap on EQ and either (LT|GT) or (LO|HI) : unconditional */
        }
        return 1; /* must be conditional */
    }
    return 0; /* Not "tw/td" or "twi/tdi".  Let
                             debugger have it */
}

OSStatus
handle_error(ExceptionInformation *xp, unsigned errnum, unsigned rb, unsigned continuable, pc where)
{
    LispObj errdisp = nrs_ERRDISP.vcell;

    if ((fulltag_of(errdisp) == fulltag_misc) &&
        (header_subtag(header_of(errdisp)) == subtag_macptr))
    {
        /* errdisp is a macptr, we can call back to lisp */
        callback_for_trap(errdisp, xp, where, errnum, rb, continuable);
        return (0);
    }

    return (-1);
}

/*
   Current thread has all signals masked.  Before unmasking them,
   make it appear that the current thread has been suspended.
   (This is to handle the case where another thread is trying
   to GC before this thread is able to sieze the exception lock.)
*/
int prepare_to_wait_for_exception_lock(TCR *tcr, ExceptionInformation *context)
{
    int old_valence = tcr->valence;

    tcr->pending_exception_context = context;
    tcr->valence = TCR_STATE_EXCEPTION_WAIT;

    ALLOW_EXCEPTIONS(context);
    return old_valence;
}

void wait_for_exception_lock_in_handler(TCR *tcr,
                                        ExceptionInformation *context,
                                        xframe_list *xf)
{

    LOCK(lisp_global(EXCEPTION_LOCK), tcr);
#ifdef DEBUG
    fprintf(dbgout, "0x%x has exception lock\n", tcr);
#endif
    xf->curr = context;
    xf->prev = tcr->xframe;
    tcr->xframe = xf;
    tcr->pending_exception_context = NULL;
    tcr->valence = TCR_STATE_FOREIGN;
}

void unlock_exception_lock_in_handler(TCR *tcr)
{
    tcr->pending_exception_context = tcr->xframe->curr;
    tcr->xframe = tcr->xframe->prev;
    tcr->valence = TCR_STATE_EXCEPTION_RETURN;
#ifdef DEBUG
    fprintf(dbgout, "0x%x releasing exception lock\n", tcr);
#endif
    UNLOCK(lisp_global(EXCEPTION_LOCK), tcr);
}

/*
   If an interrupt is pending on exception exit, try to ensure
   that the thread sees it as soon as it's able to run.
*/
void raise_pending_interrupt(TCR *tcr)
{
    if (TCR_INTERRUPT_LEVEL(tcr) > 0)
    {
        pthread_kill((pthread_t)ptr_from_lispobj(tcr->osid), SIGNAL_FOR_PROCESS_INTERRUPT);
    }
}

void exit_signal_handler(TCR *tcr, int old_valence)
{
    sigset_t mask;
    sigfillset(&mask);

    pthread_sigmask(SIG_SETMASK, &mask, NULL);
    tcr->valence = old_valence;
    tcr->pending_exception_context = NULL;
}

void signal_handler(int signum, siginfo_t *info, ExceptionInformation *context)
{
    TCR *tcr;
    int old_valence;
    xframe_list xframe_link;

    tcr = (TCR *)get_interrupt_tcr(false);

    /* The signal handler's entered with all signals (notably the
       thread_suspend signal) blocked.  Don't allow any other signals
       (notably the thread_suspend signal) to preempt us until we've
       set the TCR's xframe slot to include the current exception
       context.
    */

    old_valence = prepare_to_wait_for_exception_lock(tcr, context);

    if (tcr->flags & (1 << TCR_FLAG_BIT_PENDING_SUSPEND))
    {
        CLR_TCR_FLAG(tcr, TCR_FLAG_BIT_PENDING_SUSPEND);
        pthread_kill(pthread_self(), thread_suspend_signal);
    }

    wait_for_exception_lock_in_handler(tcr, context, &xframe_link);
    if ((noErr != PMCL_exception_handler(signum, context, tcr, info, old_valence)))
    {
        char msg[512];
        snprintf(msg, sizeof(msg), "Unhandled exception %d at 0x%lx, context->regs at #x%lx", signum, xpPC(context), (natural)xpGPRvector(context));
        if (lisp_Debugger(context, info, signum, false, msg))
        {
            SET_TCR_FLAG(tcr, TCR_FLAG_BIT_PROPAGATE_EXCEPTION);
        }
    }

    unlock_exception_lock_in_handler(tcr);

    /* This thread now looks like a thread that was suspended while
       executing lisp code.  If some other thread gets the exception
       lock and GCs, the context (this thread's suspend_context) will
       be updated.  (That's only of concern if it happens before we
       can return to the kernel/to the Mach exception handler).
    */
    exit_signal_handler(tcr, old_valence);
    raise_pending_interrupt(tcr);
}

/*
  If it looks like we're in the middle of an atomic operation, make
  it seem as if that operation is either complete or hasn't started
  yet.

  The cases handled include:

  a) storing into a newly-allocated lisp frame on the stack.
  b) marking a newly-allocated TSP frame as containing "raw" data.
  c) consing: the GC has its own ideas about how this should be
     handled, but other callers would be best advised to back
     up or move forward, according to whether we're in the middle
     of allocating a cons cell or allocating a uvector.
  d) a STMW to the vsp
  e) EGC write-barrier subprims.
*/

extern opcode
    egc_write_barrier_start,
    egc_write_barrier_end,
    egc_store_node_conditional,
    egc_store_node_conditional_test,
    egc_set_hash_key, egc_set_hash_key_did_store,
    egc_gvset, egc_gvset_did_store,
    egc_rplaca, egc_rplaca_did_store,
    egc_rplacd, egc_rplacd_did_store,
    egc_set_hash_key_conditional,
    egc_set_hash_key_conditional_test;

extern opcode ffcall_return_window, ffcall_return_window_end;

void pc_luser_xp(ExceptionInformation *xp, TCR *tcr, signed_natural *alloc_disp)
{
    assert(0);
}

void interrupt_handler(int signum, siginfo_t *info, ExceptionInformation *context)
{
    TCR *tcr = get_interrupt_tcr(false);
    if (tcr)
    {
        if (TCR_INTERRUPT_LEVEL(tcr) < 0)
        {
            tcr->interrupt_pending = 1 << fixnumshift;
        }
        else
        {
            LispObj cmain = nrs_CMAIN.vcell;

            if ((fulltag_of(cmain) == fulltag_misc) &&
                (header_subtag(header_of(cmain)) == subtag_macptr))
            {
                /*
                   This thread can (allegedly) take an interrupt now.
                   It's tricky to do that if we're executing
                   foreign code (especially Linuxthreads code, much
                   of which isn't reentrant.)
                       If we're unwinding the stack, we also want to defer
                       the interrupt.
                */
                if ((tcr->valence != TCR_STATE_LISP) ||
                    (tcr->unwinding != 0))
                {
                    TCR_INTERRUPT_LEVEL(tcr) = (1 << fixnumshift);
                }
                else
                {
                    xframe_list xframe_link;
                    int old_valence;
                    signed_natural disp = 0;

                    pc_luser_xp(context, tcr, &disp);
                    old_valence = prepare_to_wait_for_exception_lock(tcr, context);
                    wait_for_exception_lock_in_handler(tcr, context, &xframe_link);
#ifdef DEBUG
                    fprintf(dbgout, "[0x%x acquired exception lock for interrupt]\n", tcr);
#endif
                    PMCL_exception_handler(signum, context, tcr, info, old_valence);
                    if (disp)
                    {
                        xpGPR(context, allocptr) -= disp;
                    }
                    unlock_exception_lock_in_handler(tcr);
#ifdef DEBUG
                    fprintf(dbgout, "[0x%x released exception lock for interrupt]\n", tcr);
#endif
                    exit_signal_handler(tcr, old_valence);
                }
            }
        }
    }
#ifdef DARWIN
    DarwinSigReturn(context);
#endif
}

void install_signal_handler(int signo, void *handler, unsigned flags)
{
    struct sigaction sa;
    int err;

    sa.sa_sigaction = (void *)handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (flags & RESTART_SYSCALLS)
        sa.sa_flags |= SA_RESTART;
    if (flags & RESERVE_FOR_LISP)
    {
        extern sigset_t user_signals_reserved;
        sigaddset(&user_signals_reserved, signo);
    }

    err = sigaction(signo, &sa, NULL);
    if (err)
    {
        perror("sigaction");
        exit(1);
    }
}

void install_pmcl_exception_handlers()
{

    extern int no_sigtrap;
    install_signal_handler(SIGILL, (void *)signal_handler, RESERVE_FOR_LISP);
    if (no_sigtrap != 1)
    {
        install_signal_handler(SIGTRAP, (void *)signal_handler, RESERVE_FOR_LISP);
    }
    install_signal_handler(SIGBUS, (void *)signal_handler, RESERVE_FOR_LISP);
    install_signal_handler(SIGSEGV, (void *)signal_handler, RESERVE_FOR_LISP);
    install_signal_handler(SIGFPE, (void *)signal_handler, RESERVE_FOR_LISP);

    install_signal_handler(SIGNAL_FOR_PROCESS_INTERRUPT,
                           (void *)interrupt_handler, RESERVE_FOR_LISP);
    signal(SIGPIPE, SIG_IGN);
}

void thread_kill_handler(int signum, siginfo_t info, ExceptionInformation *xp)
{
    TCR *tcr = get_tcr(false);
    area *a;
    sigset_t mask;

    sigemptyset(&mask);

    if (tcr)
    {
        tcr->valence = TCR_STATE_FOREIGN;
        a = tcr->vs_area;
        if (a)
        {
            a->active = a->high;
        }
        a = tcr->ts_area;
        if (a)
        {
            a->active = a->high;
        }
        a = tcr->cs_area;
        if (a)
        {
            a->active = a->high;
        }
    }

    pthread_sigmask(SIG_SETMASK, &mask, NULL);
    pthread_exit(NULL);
}

void thread_signal_setup()
{
    thread_suspend_signal = SIG_SUSPEND_THREAD;
    thread_kill_signal = SIG_KILL_THREAD;

    install_signal_handler(thread_suspend_signal, (void *)suspend_resume_handler,
                           RESERVE_FOR_LISP | RESTART_SYSCALLS);
    install_signal_handler(thread_kill_signal, (void *)thread_kill_handler,
                           RESERVE_FOR_LISP);
}

void unprotect_all_areas()
{
    protected_area_ptr p;

    for (p = AllProtectedAreas, AllProtectedAreas = NULL; p; p = p->next)
    {
        unprotect_area(p);
    }
}

/*
  A binding subprim has just done "twlle limit_regno,idx_regno" and
  the trap's been taken.  Extend the tcr's tlb so that the index will
  be in bounds and the new limit will be on a page boundary, filling
  in the new page(s) with 'no_thread_local_binding_marker'.  Update
  the tcr fields and the registers in the xp and return true if this
  all works, false otherwise.

  Note that the tlb was allocated via malloc, so realloc can do some
  of the hard work.
*/
Boolean
extend_tcr_tlb(TCR *tcr,
               ExceptionInformation *xp,
               unsigned limit_regno,
               unsigned idx_regno)
{
    unsigned
        index = (unsigned)(xpGPR(xp, idx_regno)),
        old_limit = tcr->tlb_limit,
        new_limit = align_to_power_of_2(index + 1, 12),
        new_bytes = new_limit - old_limit;
    LispObj
        *old_tlb = tcr->tlb_pointer,
        *new_tlb = realloc(old_tlb, new_limit),
        *work;

    if (new_tlb == NULL)
    {
        return false;
    }

    work = (LispObj *)((BytePtr)new_tlb + old_limit);

    while (new_bytes)
    {
        *work++ = no_thread_local_binding_marker;
        new_bytes -= sizeof(LispObj);
    }
    tcr->tlb_pointer = new_tlb;
    tcr->tlb_limit = new_limit;
    xpGPR(xp, limit_regno) = new_limit;
    return true;
}

void exception_init()
{
    install_pmcl_exception_handlers();
}

/* FROM x86-exceptions.c */
#ifdef DARWIN

// The next three functions are for MIG generated mach exception handling

kern_return_t catch_mach_exception_raise(mach_port_t exception_port,
                                         mach_port_t thread,
                                         mach_port_t task,
                                         exception_type_t exception,
                                         mach_exception_data_t code,
                                         mach_msg_type_number_t code_count)
{
    abort();
    return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state(mach_port_t exception_port,
                                               exception_type_t exception,
                                               mach_exception_data_t code,
                                               mach_msg_type_number_t code_count,
                                               int *flavor,
                                               thread_state_t in_state,
                                               mach_msg_type_number_t in_state_count,
                                               thread_state_t out_state,
                                               mach_msg_type_number_t *out_state_count)
{
    //   int64_t code0 = code[0];
    //   int signum = 0;
    //   TCR *tcr = TCR_FROM_EXCEPTION_PORT(exception_port);
    //   mach_port_t thread = (mach_port_t)((natural)tcr->native_thread_id);
    //   kern_return_t kret, call_kret;

    //   native_thread_state_t
    //     *ts = (native_thread_state_t *)in_state,
    //     *out_ts = (native_thread_state_t*)out_state;
    //   mach_msg_type_number_t thread_state_count;

    //   if (tcr->flags & (1<<TCR_FLAG_BIT_PENDING_EXCEPTION)) {
    //     CLR_TCR_FLAG(tcr,TCR_FLAG_BIT_PENDING_EXCEPTION);
    //   }
    //   if ((code0 == EXC_I386_GPFLT) &&
    //       ((natural)(ts_pc(ts)) == (natural)pseudo_sigreturn)) {
    //     kret = do_pseudo_sigreturn(thread, tcr, out_ts);
    // #if 0
    //     fprintf(dbgout, "Exception return in 0x%x\n",tcr);
    // #endif
    //   } else if (tcr->flags & (1<<TCR_FLAG_BIT_PROPAGATE_EXCEPTION)) {
    //     CLR_TCR_FLAG(tcr,TCR_FLAG_BIT_PROPAGATE_EXCEPTION);
    //     kret = 17;
    //   } else {
    //     switch (exception) {
    //     case EXC_BAD_ACCESS:
    //       if (code0 == EXC_I386_GPFLT) {
    // 	signum = SIGSEGV;
    //       } else {
    // 	signum = SIGBUS;
    //       }
    //       break;

    //     case EXC_BAD_INSTRUCTION:
    //       if (code0 == EXC_I386_GPFLT) {
    // 	signum = SIGSEGV;
    //       } else {
    // 	signum = SIGILL;
    //       }
    //       break;

    //     case EXC_SOFTWARE:
    //       signum = SIGILL;
    //       break;

    //     case EXC_ARITHMETIC:
    //       signum = SIGFPE;
    //       if (code0 == EXC_I386_DIV)
    // 	code0 = FPE_INTDIV;
    //       break;

    //     default:
    //       break;
    //     }
    // #if WORD_SIZE==64
    //     if ((signum==SIGFPE) &&
    // 	(code0 != FPE_INTDIV) &&
    // 	(tcr->valence != TCR_STATE_LISP)) {
    //       mach_msg_type_number_t thread_state_count = x86_FLOAT_STATE64_COUNT;
    //       x86_float_state64_t fs;

    //       thread_get_state(thread,
    // 		       x86_FLOAT_STATE64,
    // 		       (thread_state_t)&fs,
    // 		       &thread_state_count);

    //       if (! (tcr->flags & (1<<TCR_FLAG_BIT_FOREIGN_FPE))) {
    // 	tcr->flags |= (1<<TCR_FLAG_BIT_FOREIGN_FPE);
    // 	tcr->lisp_mxcsr = (fs.__fpu_mxcsr & ~MXCSR_STATUS_MASK);
    //       }
    //       fs.__fpu_mxcsr &= ~MXCSR_STATUS_MASK;
    //       fs.__fpu_mxcsr |= MXCSR_CONTROL_MASK;
    //       thread_set_state(thread,
    // 		       x86_FLOAT_STATE64,
    // 		       (thread_state_t)&fs,
    // 		       x86_FLOAT_STATE64_COUNT);
    //       *out_state_count = NATIVE_THREAD_STATE_COUNT;
    //       *out_ts = *ts;
    //       return KERN_SUCCESS;
    //     }
    // #endif
    //     if (signum) {
    //       kret = setup_signal_frame(thread,
    // 				(void *)DARWIN_EXCEPTION_HANDLER,
    // 				signum,
    // 				code0,
    // 				tcr,
    // 				ts,
    // 				out_ts);

    //     } else {
    //       kret = 17;
    //     }
    //   }

    //   if (kret) {
    //     *out_state_count = 0;
    //     *flavor = 0;
    //   } else {
    //     *out_state_count = NATIVE_THREAD_STATE_COUNT;
    //   }
    //   return kret;
}

kern_return_t catch_mach_exception_raise_state_identity(mach_port_t exception_port,
                                                        mach_port_t thread,
                                                        mach_port_t task,
                                                        exception_type_t exception,
                                                        mach_exception_data_t code,
                                                        mach_msg_type_number_t code_count,
                                                        int *flavor,
                                                        thread_state_t old_state,
                                                        mach_msg_type_number_t old_count,
                                                        thread_state_t new_state,
                                                        mach_msg_type_number_t *new_count)
{
    abort();
    return KERN_FAILURE;
}

/*
  Mach's exception mechanism works a little better than its signal
  mechanism (and, not incidentally, it gets along with GDB a lot
  better.

  Initially, we install an exception handler to handle each native
  thread's exceptions.  This process involves creating a distinguished
  thread which listens for kernel exception messages on a set of
  0 or more thread exception ports.  As threads are created, they're
  added to that port set; a thread's exception port is destroyed
  (and therefore removed from the port set) when the thread exits.

  A few exceptions can be handled directly in the handler thread;
  others require that we resume the user thread (and that the
  exception thread resumes listening for exceptions.)  The user
  thread might eventually want to return to the original context
  (possibly modified somewhat.)

  As it turns out, the simplest way to force the faulting user
  thread to handle its own exceptions is to do pretty much what
  signal() does: the exception handlng thread sets up a sigcontext
  on the user thread's stack and forces the user thread to resume
  execution as if a signal handler had been called with that
  context as an argument.  We can use a distinguished UUO at a
  distinguished address to do something like sigreturn(); that'll
  have the effect of resuming the user thread's execution in
  the (pseudo-) signal context.

  Since:
    a) we have miles of code in C and in Lisp that knows how to
    deal with Linux sigcontexts
    b) Linux sigcontexts contain a little more useful information
    (the DAR, DSISR, etc.) than their Darwin counterparts
    c) we have to create a sigcontext ourselves when calling out
    to the user thread: we aren't really generating a signal, just
    leveraging existing signal-handling code.

  we create a Linux sigcontext struct.

  Simple ?  Hopefully from the outside it is ...

  We want the process of passing a thread's own context to it to
  appear to be atomic: in particular, we don't want the GC to suspend
  a thread that's had an exception but has not yet had its user-level
  exception handler called, and we don't want the thread's exception
  context to be modified by a GC while the Mach handler thread is
  copying it around.  On Linux (and on Jaguar), we avoid this issue
  because (a) the kernel sets up the user-level signal handler and
  (b) the signal handler blocks signals (including the signal used
  by the GC to suspend threads) until tcr->xframe is set up.

  The GC and the Mach server thread therefore contend for the lock
  "mach_exception_lock".  The Mach server thread holds the lock
  when copying exception information between the kernel and the
  user thread; the GC holds this lock during most of its execution
  (delaying exception processing until it can be done without
  GC interference.)

*/

#define C_REDZONE_LEN 320
#define C_STK_ALIGN 32

#define TRUNC_DOWN(a, b, c) (((((natural)a) - (b)) / (c)) * (c))

ExceptionInformation *create_thread_context_frame(mach_port_t thread,
                                                  natural *new_stack_top,
                                                  siginfo_t **info_ptr,
                                                  TCR *tcr,
                                                  native_thread_state_t *ts)
{
    assert(0);
}

#define TCR_FROM_EXCEPTION_PORT(p) find_tcr_from_exception_port(p)
#define TCR_TO_EXCEPTION_PORT(t) (mach_port_name_t)((natural)(((TCR *)t)->io_datum))

#define LISP_EXCEPTIONS_HANDLED_MASK \
    (EXC_MASK_SOFTWARE | EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC)

/* (logcount LISP_EXCEPTIONS_HANDLED_MASK) */
#define NUM_LISP_EXCEPTIONS_HANDLED 4

typedef struct
{
    int foreign_exception_port_count;
    exception_mask_t masks[NUM_LISP_EXCEPTIONS_HANDLED];
    mach_port_t ports[NUM_LISP_EXCEPTIONS_HANDLED];
    exception_behavior_t behaviors[NUM_LISP_EXCEPTIONS_HANDLED];
    thread_state_flavor_t flavors[NUM_LISP_EXCEPTIONS_HANDLED];
} MACH_foreign_exception_state;

/*
  Establish the lisp thread's TCR as its exception port, and determine
  whether any other ports have been established by foreign code for
  exceptions that lisp cares about.

  If this happens at all, it should happen on return from foreign
  code and on entry to lisp code via a callback.

  This is a lot of trouble (and overhead) to support Java, or other
  embeddable systems that clobber their caller's thread exception ports.

*/
kern_return_t tcr_establish_exception_port(TCR *tcr, mach_port_t thread)
{
    kern_return_t kret;
    MACH_foreign_exception_state *fxs = (MACH_foreign_exception_state *)tcr->native_thread_info;
    int i;
    unsigned n = NUM_LISP_EXCEPTIONS_HANDLED;
    mach_port_t lisp_port = TCR_TO_EXCEPTION_PORT(tcr), foreign_port;
    exception_mask_t mask = 0;

    kret = thread_swap_exception_ports(thread,
                                       LISP_EXCEPTIONS_HANDLED_MASK,
                                       lisp_port,
                                       MACH_EXCEPTION_CODES | EXCEPTION_STATE,
                                       ARM_THREAD_STATE64,
                                       fxs->masks,
                                       &n,
                                       fxs->ports,
                                       fxs->behaviors,
                                       fxs->flavors);
    if (kret == KERN_SUCCESS)
    {
        fxs->foreign_exception_port_count = n;
        for (i = 0; i < n; i++)
        {
            foreign_port = fxs->ports[i];

            if ((foreign_port != lisp_port) &&
                (foreign_port != MACH_PORT_NULL))
            {
                mask |= fxs->masks[i];
            }
        }
        tcr->foreign_exception_status = (int)mask;
    }
    return kret;
}

kern_return_t tcr_establish_lisp_exception_port(TCR *tcr)
{
    return tcr_establish_exception_port(tcr, (mach_port_t)((natural)tcr->native_thread_id));
}

void fatal_mach_error(char *format, ...);
#define MACH_CHECK_ERROR(context, x)                              \
    if (x != KERN_SUCCESS)                                        \
    {                                                             \
        fatal_mach_error("Mach error while %s : %d", context, x); \
    }

static mach_port_t mach_exception_thread = (mach_port_t)0;

/*
  The initial function for an exception-handling thread.
*/

void *exception_handler_proc(void *arg)
{
    extern boolean_t mach_exc_server();
    mach_port_t p = (mach_port_t)((natural)arg);

    mach_exception_thread = pthread_mach_thread_np(pthread_self());
    mach_msg_server(mach_exc_server, 256, p, 0);
    /* Should never return. */
    abort();
}

mach_port_t mach_exception_port_set()
{
    static mach_port_t __exception_port_set = MACH_PORT_NULL;
    kern_return_t kret;
    if (__exception_port_set == MACH_PORT_NULL)
    {

        kret = mach_port_allocate(mach_task_self(),
                                  MACH_PORT_RIGHT_PORT_SET,
                                  &__exception_port_set);
        MACH_CHECK_ERROR("allocating thread exception_ports", kret);
        create_system_thread(0,
                             NULL,
                             exception_handler_proc,
                             (void *)((natural)__exception_port_set));
    }
    return __exception_port_set;
}

/*
  This assumes that a Mach port (to be used as the thread's exception port) whose
  "name" matches the TCR's 32-bit address has already been allocated.
*/

kern_return_t
setup_mach_exception_handling(TCR *tcr)
{
    mach_port_t
        thread_exception_port = TCR_TO_EXCEPTION_PORT(tcr),
        task_self = mach_task_self();
    kern_return_t kret;

    kret = mach_port_insert_right(task_self,
                                  thread_exception_port,
                                  thread_exception_port,
                                  MACH_MSG_TYPE_MAKE_SEND);
    MACH_CHECK_ERROR("adding send right to exception_port", kret);

    kret = tcr_establish_exception_port(tcr, (mach_port_t)((natural)tcr->native_thread_id));
    if (kret == KERN_SUCCESS)
    {
        mach_port_t exception_port_set = mach_exception_port_set();

        kret = mach_port_move_member(task_self,
                                     thread_exception_port,
                                     exception_port_set);
    }
    return kret;
}

void darwin_exception_init(TCR *tcr)
{
    void tcr_monitor_exception_handling(TCR *, Boolean);
    kern_return_t kret;
    MACH_foreign_exception_state *fxs =
        calloc(1, sizeof(MACH_foreign_exception_state));

    tcr->native_thread_info = (void *)fxs;

    if ((kret = setup_mach_exception_handling(tcr)) != KERN_SUCCESS)
    {
        fprintf(dbgout, "Couldn't setup exception handler - error = %d\n", kret);
        terminate_lisp();
    }
}

void associate_tcr_with_exception_port(mach_port_t port, TCR *tcr)
{
    kern_return_t kret;

    kret = mach_port_set_context(mach_task_self(),
                                 port, (mach_vm_address_t)tcr);
    MACH_CHECK_ERROR("associating TCR with exception port", kret);
}

void disassociate_tcr_from_exception_port(mach_port_t port)
{
    kern_return_t kret;

    kret = mach_port_set_context(mach_task_self(), port, 0);
    MACH_CHECK_ERROR("disassociating TCR with exception port", kret);
}

/*
  The tcr is the "name" of the corresponding thread's exception port.
  Destroying the port should remove it from all port sets of which it's
  a member (notably, the exception port set.)
*/
void darwin_exception_cleanup(TCR *tcr)
{
    mach_port_t exception_port;
    void *fxs = tcr->native_thread_info;

    if (fxs)
    {
        tcr->native_thread_info = NULL;
        free(fxs);
    }

    exception_port = TCR_TO_EXCEPTION_PORT(tcr);
    disassociate_tcr_from_exception_port(exception_port);
    mach_port_deallocate(mach_task_self(), exception_port);
    /* Theoretically not needed, I guess... */
    mach_port_destroy(mach_task_self(), exception_port);
}

void fatal_mach_error(char *format, ...)
{
    va_list args;
    char s[512];

    va_start(args, format);
    vsnprintf(s, sizeof(s), format, args);
    va_end(args);

    Fatal("Mach error", s);
}

#endif // DARWIN