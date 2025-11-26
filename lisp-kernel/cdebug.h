#ifndef __CDEBUG_H__
#define __CDEBUG_H__

void debug_header_print(const char *header_name);
void debug_section_print(const char *section_name);
void debug_memory_printf(const char *label, const char *format, ...);


#endif /* __CDEBUG_H__ */