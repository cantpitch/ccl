#include <assert.h>
#include <fenv.h>
#include "arm-constants64.h"
#include "arm-exceptions64.h"

#ifdef DARWIN
#include <mach/mach.h>
#endif

int page_size = 0x8000;
int log2_page_size = 13; // log2(0x8000) = 13


void enable_fp_exceptions()
{
    return;
}

void disable_fp_exceptions()
{
    return;
}

void associate_tcr_with_exception_port(mach_port_t port, TCR *tcr)
{
    kern_return_t kret;
    
    kret = mach_port_set_context(mach_task_self(), port, (mach_vm_address_t)tcr);
    MACH_CHECK_ERROR("associating TCR with exception port", kret); 
}

void adjust_exception_pc(ExceptionInformation *xp, int delta)
{
    assert(0); 
}

ExceptionInformation *create_thread_context_frame(mach_port_t thread,
                                                  natural *new_stack_top,
                                                  siginfo_t **info_ptr,
                                                  TCR *tcr,
                                                  native_thread_state_t *ts)
{
    assert(0); 
    return NULL;
}

/*
  The tcr is the "name" of the corresponding thread's exception port.
  Destroying the port should remove it from all port sets of which it's
  a member (notably, the exception port set.)
*/
void darwin_exception_cleanup(TCR *tcr)
{
    assert(0); 
}

void darwin_exception_init(TCR *tcr)
{
    assert(0); 
}

void install_signal_handler(int signo, void *handler, unsigned flags)
{
    assert(0); 
}

Boolean lisp_frame_p(lisp_frame *spPtr)
{
    assert(0); 
    return false; // Placeholder return value.
}

void platform_new_heap_segment(ExceptionInformation *xp, TCR *tcr, BytePtr low, BytePtr high)
{
    assert(0); 
}

/* Maybe this'll work someday.  We may have to do something to
   make the thread look like it's not handling an exception */
void reset_lisp_process(ExceptionInformation *xp)
{
    assert(0); 
}

void restore_soft_stack_limit(unsigned stkreg)
{
    assert(0); 
}

kern_return_t tcr_establish_lisp_exception_port(TCR *tcr)
{
    assert(0); 
    return KERN_FAILURE; // Placeholder return value.
}

LispObj *tcr_frame_ptr(TCR *tcr)
{
    assert(0); 
    return NULL; // Placeholder return value.
}

void thread_signal_setup()
{
    assert(0); 
}