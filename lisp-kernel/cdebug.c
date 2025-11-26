
#include <stdio.h>
#include <stdarg.h>
#include "lisp.h"

void debug_header_print(const char *header_name)
{
#ifdef DEBUG_MEMORY
    fprintf(dbgout, "\n%s:\n", header_name);
#endif
}

void debug_section_print(const char *section_name)
{
#ifdef DEBUG_MEMORY
    fprintf(dbgout, "  %s:\n", section_name);
#endif
}

void debug_memory_printf(const char *label, const char *format, ...)
{
#ifdef DEBUG_MEMORY
    va_list ap;
    char buffer[50];
    char empty[1] = "";

    va_start(ap, format);

    /* uses the first variable argument as the value to print */
    vsprintf(buffer, format, ap);

    va_end(ap);

    fprintf(dbgout, "    %-20s: %20s\n", label, buffer);
#endif
}