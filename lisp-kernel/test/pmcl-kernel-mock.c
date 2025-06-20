

#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include "../lisp.h"
#include "../lisp_globals.h"
#include "../gc.h"
#include "../area.h"
#include "../lisp-exceptions.h"


LispObj lisp_nil = (LispObj) 0;
bitvector global_mark_ref_bits = NULL, dynamic_mark_ref_bits = NULL, relocatable_mark_ref_bits = NULL, global_refidx = NULL, dynamic_refidx = NULL,managed_static_refidx = NULL;
LispObj *global_reloctab = (LispObj*) 0, *GCrelocptr = (LispObj*) 0;
void *tcr_area_lock = NULL;


LispObj text_start = 0;


/* The highest heap address that's (probably) been written to. */
BytePtr heap_dirty_limit = NULL;



natural
reserved_area_size = MAXIMUM_MAPPABLE_MEMORY;

BytePtr reserved_region_end = NULL;

area 
    *nilreg_area=NULL,
    *tenured_area=NULL, 
    *g2_area=NULL, 
    *g1_area=NULL,
    *managed_static_area=NULL,
    *static_cons_area=NULL,
    *readonly_area=NULL;

area *all_areas=NULL;
int cache_block_size=32;



#if WORD_SIZE == 64
#define DEFAULT_LISP_HEAP_GC_THRESHOLD (32<<20)
#define G2_AREA_THRESHOLD (8<<20)
#define G1_AREA_THRESHOLD (4<<20)
#define G0_AREA_THRESHOLD (2<<20)
#else
#define DEFAULT_LISP_HEAP_GC_THRESHOLD (16<<20)
#define G2_AREA_THRESHOLD (4<<20)
#define G1_AREA_THRESHOLD (2<<20)
#define G0_AREA_THRESHOLD (1<<20)
#endif

#define MIN_DYNAMIC_SIZE (DEFAULT_LISP_HEAP_GC_THRESHOLD *2)

#if (WORD_SIZE == 32)
#define DEFAULT_INITIAL_STACK_SIZE (1<<20)
#else
#define DEFAULT_INITIAL_STACK_SIZE (2<<20)
#endif

natural
lisp_heap_gc_threshold = DEFAULT_LISP_HEAP_GC_THRESHOLD;

natural
lisp_heap_notify_threshold = 0;

natural 
initial_stack_size = DEFAULT_INITIAL_STACK_SIZE;

natural
thread_stack_size = 0;

Boolean bogus_fp_exceptions = false;

#ifndef WINDOWS
natural user_signal_semaphores[NSIG];
sigset_t user_signals_reserved;
#endif

LispObj image_base=0;
BytePtr pure_space_start, pure_space_active, pure_space_limit;
BytePtr static_space_start, static_space_active, static_space_limit;

pid_t main_thread_pid = (pid_t)0;

BytePtr reloctab_limit = NULL, markbits_limit = NULL;
BytePtr low_relocatable_address = NULL, high_relocatable_address = NULL,
  low_markable_address = NULL, high_markable_address = NULL;

void Fatal(StringPtr param0, StringPtr param1)
{
    assert(0);
}

area * allocate_dynamic_area(natural initsize)
{
    assert(0);
}

area * allocate_tstack_holding_area_lock(natural usable)
{
    assert(0);
}

area * allocate_vstack_holding_area_lock(natural usable)
{
    assert(0);
}

size_t ensure_stack_limit(size_t stack_size)
{
    assert(0);
    return 0;
}

void ensure_static_conses(ExceptionInformation *xp, TCR *tcr, natural nconses)
{
    assert(0);
}

area *extend_readonly_area(natural more)
{
    assert(0);
    return NULL;
}

void xMakeDataExecutable(BytePtr start, natural nbytes)
{
    assert(0);
}

void terminate_lisp()
{
    assert(0);
}

Boolean shrink_dynamic_area(natural delta)
{
    assert(0);
    return false;
}

area *set_nil(LispObj r)
{
    assert(0);
    return NULL;
}

void sample_paging_info(paging_info *stats)
{
    assert(0);
}

void report_paging_info_delta(FILE *out, paging_info *start, paging_info *stop)
{
    assert(0);
}

area *register_cstack_holding_area_lock(BytePtr bottom, natural size)
{
    assert(0);
    return NULL;
}

void raise_limit()
{
    assert(0);
}

void map_initial_reloctab(BytePtr low, BytePtr high)  
{
    assert(0);
}

void map_initial_markbits(BytePtr low, BytePtr high)
{
    assert(0);
}

void make_dynamic_heap_executable(void *p, void *q)
{
    assert(0);
}

void lower_heap_start(BytePtr new_low, area *a)
{
    assert(0);
}

Boolean grow_dynamic_area(natural delta)
{
    assert(0);
    return false;
}

void fatal_oserr(StringPtr param, OSErr err)
{
    assert(0);
}

void *allocate_from_reserved_area(natural size)
{
    assert(0);
    return NULL;
}


#ifndef DARWIN
#ifdef WINDOWS
extern void *windows_open_shared_library(char *);

void *xGetSharedLibrary(char *path, int mode)
{
    assert(0);
}
#else
void *xGetSharedLibrary(char *path, int mode)
{
    assert(0);
}
#endif
#else
void *xGetSharedLibrary(char *path, int *resultType)
{
    assert(0);
}
#endif

int fd_setsize_bytes()
{
    assert(0);
    return 0;
}

void do_fd_set(int fd, fd_set *fdsetp)
{
    assert(0);
}

void do_fd_clr(int fd, fd_set *fdsetp)
{
    assert(0);
}

int do_fd_is_set(int fd, fd_set *fdsetp)
{
    assert(0);
    return 0;
}

void do_fd_zero(fd_set *fdsetp)
{   
    assert(0);
}

void * xFindSymbol(void* handle, char *name)
{
    assert(0);
    return NULL;
}

int wait_for_signal(int signo, int seconds, int milliseconds)
{
    assert(0);
    return 0;
}

void usage_exit(char *herald, int exit_status, char* other_args)
{
    assert(0);
}

typedef int (*jvm_initfunc)(void*,void*,void*);

int jvm_init(jvm_initfunc f,void*arg0,void*arg1,void*arg2)
{
    assert(0);
    return 0;
}

void * get_r_debug()
{
    assert(0);
    return NULL;
}