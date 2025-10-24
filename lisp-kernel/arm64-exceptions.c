#ifdef DARWIN
#include <mach/mach.h>
#endif


int page_size = 0x8000;
int log2_page_size = 13; // log2(0x8000) = 13

void
fatal_mach_error(char *format, ...);

#define MACH_CHECK_ERROR(context,x) if (x != KERN_SUCCESS) {fatal_mach_error("Mach error while %s : %d", context, x);}



void enable_fp_exceptions()
{
    return;
}

void disable_fp_exceptions()
{
    return;
}