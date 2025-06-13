#include <assert.h>
#include "arm64-constants.h"
#include "arm64-exceptions.h"

#ifdef DARWIN
#include <mach/mach.h>
#endif

int page_size = 0x8000;
int log2_page_size = 13; // log2(0x8000) = 13

void enable_fp_exceptions()
{
    assert(0); // This function is not implemented for this architecture.
}

void disable_fp_exceptions()
{
    assert(0); // This function is not implemented for this architecture.
}

void associate_tcr_with_exception_port(mach_port_t port, TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
}

void adjust_exception_pc(ExceptionInformation *xp, int delta)
{
    assert(0); // This function is not implemented for this architecture.
}

ExceptionInformation *create_thread_context_frame(mach_port_t thread,
                                                  natural *new_stack_top,
                                                  siginfo_t **info_ptr,
                                                  TCR *tcr,
                                                  native_thread_state_t *ts)
{
    assert(0); // This function is not implemented for this architecture.
    return NULL;
}

/*
  The tcr is the "name" of the corresponding thread's exception port.
  Destroying the port should remove it from all port sets of which it's
  a member (notably, the exception port set.)
*/
void darwin_exception_cleanup(TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
}

void darwin_exception_init(TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
}

void install_signal_handler(int signo, void *handler, unsigned flags)
{
    assert(0); // This function is not implemented for this architecture.
}

Boolean lisp_frame_p(lisp_frame *spPtr)
{
    assert(0); // This function is not implemented for this architecture.
    return false; // Placeholder return value.
}

void platform_new_heap_segment(ExceptionInformation *xp, TCR *tcr, BytePtr low, BytePtr high)
{
    assert(0); // This function is not implemented for this architecture.
}

/* Maybe this'll work someday.  We may have to do something to
   make the thread look like it's not handling an exception */
void reset_lisp_process(ExceptionInformation *xp)
{
    assert(0); // This function is not implemented for this architecture.
}

void restore_soft_stack_limit(unsigned stkreg)
{
    assert(0); // This function is not implemented for this architecture.
}

kern_return_t tcr_establish_lisp_exception_port(TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
    return KERN_FAILURE; // Placeholder return value.
}

LispObj *tcr_frame_ptr(TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
    return NULL; // Placeholder return value.
}

void thread_signal_setup()
{
    assert(0); // This function is not implemented for this architecture.
}