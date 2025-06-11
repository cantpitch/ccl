#include "arm64-constants.h"
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

#ifdef DARWIN
#include <sysexits.h>
#include <dlfcn.h>
#include <mach/mach.h>
#include <mach/machine/vm_types.h>
#endif

void fatal_mach_error(char *format, ...);

#define MACH_CHECK_ERROR(context, x)                              \
    if (x != KERN_SUCCESS)                                        \
    {                                                             \
        fatal_mach_error("Mach error while %s : %d", context, x); \
    }

void enable_fp_exceptions()
{
}

void disable_fp_exceptions()
{
}

void associate_tcr_with_exception_port(mach_port_t port, TCR *tcr)
{
    kern_return_t kret;

    kret = mach_port_set_context(mach_task_self(),
                                 port, (mach_vm_address_t)tcr);
    MACH_CHECK_ERROR("associating TCR with exception port", kret);
}