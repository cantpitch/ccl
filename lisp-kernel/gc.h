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

#ifndef __GC_H__
#define __GC_H__ 1

#include "lisp.h"
#include "bits.h"
#include "lisp-exceptions.h"
#include "memprotect.h"



#ifdef PPC
#define is_node_fulltag(f)  ((1<<(f))&((1<<fulltag_cons)|(1<<fulltag_misc)))
#ifdef PPC64
#define PPC64_CODE_VECTOR_PREFIX (('C'<< 24) | ('O' << 16) | ('D' << 8) | 'E')
#else
/*
  A code-vector's header can't look like a valid instruction or UUO:
  the low 8 bits must be subtag_code_vector, and the top 6 bits
  must be 0.  That means that the maximum length of a code vector
  is 18 bits worth of elements (~1MB.)
*/

#define code_header_mask ((0x3f<<26) | subtag_code_vector)
#endif
#endif

#ifdef X86
#ifdef X8664
#define is_node_fulltag(f)  ((1<<(f))&((1<<fulltag_cons)    | \
				       (1<<fulltag_tra_0)   | \
				       (1<<fulltag_tra_1)   | \
				       (1<<fulltag_misc)    | \
				       (1<<fulltag_symbol)  | \
				       (1<<fulltag_function)))
#else
#define is_node_fulltag(f)  ((1<<(f))&((1<<fulltag_cons) | \
				       (1<<fulltag_misc) | \
				       (1<<fulltag_tra)))
#endif
#endif

#ifdef ARM
#define is_node_fulltag(f)  ((1<<(f))&((1<<fulltag_cons)|(1<<fulltag_misc)))
#endif

#ifdef ARM64
// NOTE: Copied from PPC, not tested
#define is_node_fulltag(f)  ((1<<(f))&((1<<fulltag_cons)|(1<<fulltag_misc)))
#define ARM64_CODE_VECTOR_PREFIX (('C'<< 24) | ('O' << 16) | ('D' << 8) | 'E')

#endif

extern LispObj GCarealow, GCareadynamiclow;
extern natural GCndnodes_in_area, GCndynamic_dnodes_in_area;
extern bitvector GCmarkbits, GCdynamic_markbits,managed_static_refbits,global_refidx,dynamic_refidx,managed_static_refidx;
extern LispObj *global_reloctab, *GCrelocptr;
extern LispObj GCfirstunmarked;

extern natural lisp_heap_gc_threshold;
extern natural lisp_heap_notify_threshold;
void mark_root(LispObj);
void mark_pc_root(LispObj);
void mark_locative_root(LispObj);
void rmark(LispObj);
LispObj *skip_over_ivector(LispObj, LispObj);
void mark_simple_area_range(LispObj *,LispObj *);
LispObj calculate_relocation();
LispObj locative_forwarding_address(LispObj);
LispObj node_forwarding_address(LispObj);
void forward_range(LispObj *, LispObj *);
void forward_tcr_xframes(TCR *);
void note_memoized_references(ExceptionInformation *,LogicalAddress, LogicalAddress, BytePtr *, BytePtr *);
void gc(TCR *, signed_natural);
int change_hons_area_size(TCR *, signed_natural);
void delete_protected_area(protected_area_ptr);
Boolean egc_control(Boolean, BytePtr);
Boolean new_heap_segment(ExceptionInformation *, natural, Boolean , TCR *, Boolean *);
void platform_new_heap_segment(ExceptionInformation *, TCR*, BytePtr, BytePtr);
/* an type representing 1/4 of a natural word */
#if WORD_SIZE == 64
typedef unsigned short qnode;
#else
typedef unsigned char qnode;
#endif


#ifdef fulltag_symbol
#define is_symbol_fulltag(x) (fulltag_of(x) == fulltag_symbol)
#else
#define is_symbol_fulltag(x) (fulltag_of(x) == fulltag_misc)
#endif

#define area_dnode(w,low) ((natural)(((ptr_to_lispobj(w)) - ptr_to_lispobj(low))>>dnode_shift))
#define gc_area_dnode(w)  area_dnode(w,GCarealow)
#define gc_dynamic_area_dnode(w) area_dnode(w,GCareadynamiclow)

#if defined(PPC64) || defined(X8632) || defined(ARM64)
#define forward_marker subtag_forward_marker
#else
#ifdef ARM
#define forward_marker (0xe7fffff0|uuo_format_unary)
#else
#define forward_marker fulltag_nil
#endif
#endif

#ifdef PPC64
#define VOID_ALLOCPTR ((LispObj)(0x8000000000000000-dnode_size))
#else
#define VOID_ALLOCPTR ((LispObj)(-dnode_size))
#endif

#ifndef WINDOWS
#include <sys/resource.h>
typedef struct rusage paging_info;
#else
typedef natural paging_info;
#endif

#undef __argv
#include <stdio.h>

void sample_paging_info(paging_info *);
void report_paging_info_delta(FILE*, paging_info *, paging_info *);


#define GC_TRAP_FUNCTION_IMMEDIATE_GC (-1)
#define GC_TRAP_FUNCTION_GC 0
#define GC_TRAP_FUNCTION_PURIFY 1
#define GC_TRAP_FUNCTION_IMPURIFY 2
#define GC_TRAP_FUNCTION_FLASH_FREEZE 4
#define GC_TRAP_FUNCTION_SAVE_APPLICATION 8

#define GC_TRAP_FUNCTION_GET_LISP_HEAP_THRESHOLD 16
#define GC_TRAP_FUNCTION_SET_LISP_HEAP_THRESHOLD 17
#define GC_TRAP_FUNCTION_USE_LISP_HEAP_THRESHOLD 18
#define GC_TRAP_FUNCTION_ENSURE_STATIC_CONSES 19
#define GC_TRAP_FUNCTION_GET_GC_NOTIFICATION_THRESHOLD 20
#define GC_TRAP_FUNCTION_SET_GC_NOTIFICATION_THRESHOLD 21
#define GC_TRAP_FUNCTION_ALLOCATION_CONTROL 22
#define GC_TRAP_FUNCTION_EGC_CONTROL 32
#define GC_TRAP_FUNCTION_CONFIGURE_EGC 64
#define GC_TRAP_FUNCTION_FREEZE 129
#define GC_TRAP_FUNCTION_THAW 130

extern Boolean GCDebug, GCverbose, just_purified_p;
extern bitvector GCmarkbits, GCdynamic_markbits;
extern LispObj GCarealow, GCareadynamiclow;
extern natural GCndnodes_in_area, GCndynamic_dnodes_in_area;
extern LispObj GCweakvll, GCdwsweakvll;
extern LispObj GCephemeral_low;
extern natural GCn_ephemeral_dnodes;
extern natural GCstack_limit;

#if WORD_SIZE == 64
extern unsigned short *_one_bits;
#else
extern const unsigned char _one_bits[256];
#endif

#define one_bits(x) _one_bits[x]

natural static_dnodes_for_area(area *a);
void reapweakv(LispObj weakv);
void reaphashv(LispObj hashv);
Boolean mark_weak_hash_vector(hash_table_vector_header *hashp, natural elements);
Boolean mark_weak_alist(LispObj weak_alist, int weak_type);
void mark_tcr_tlb(TCR *);
void mark_tcr_xframes(TCR *);
void freeGCptrs(void);
void reap_gcable_ptrs(void);
unsigned short logcount16(unsigned short);
void gc_init(void);
LispObj node_forwarding_address(LispObj);
Boolean update_noderef(LispObj *);
void update_locref(LispObj *);
void forward_gcable_ptrs(void);
void forward_memoized_area(area *, natural, bitvector, bitvector);

void forward_tcr_tlb(TCR *);
void reclaim_static_dnodes(void);
Boolean youngest_non_null_area_p(area *);
void gc(TCR *, signed_natural);

/* backend-interface */

typedef void (*weak_mark_fun) (LispObj);
extern weak_mark_fun mark_weak_htabv, dws_mark_weak_htabv;

typedef void (*weak_process_fun)(void);
extern weak_process_fun markhtabvs;


#define hash_table_vector_header_count (sizeof(hash_table_vector_header)/sizeof(LispObj))

void mark_root(LispObj);
void rmark(LispObj);
#ifdef X8632
void mark_xp(ExceptionInformation *, natural);
#else
void mark_xp(ExceptionInformation *);
#endif
LispObj dnode_forwarding_address(natural, int);
LispObj locative_forwarding_address(LispObj);
void check_refmap_consistency(LispObj *, LispObj *, bitvector, bitvector);
void check_all_areas(TCR *);
void mark_tstack_area(area *);
void mark_vstack_area(area *);
void mark_cstack_area(area *);
void mark_simple_area_range(LispObj *, LispObj *);
void mark_memoized_area(area *, natural, bitvector);
LispObj calculate_relocation(void);
void forward_range(LispObj *, LispObj *);
void forward_tstack_area(area *);
void forward_vstack_area(area *);
void forward_cstack_area(area *);
LispObj compact_dynamic_heap(void);
signed_natural purify(TCR *, signed_natural);
signed_natural impurify(TCR *, signed_natural);
signed_natural gc_like_from_xp(ExceptionInformation *, signed_natural(*fun)(TCR *, signed_natural), signed_natural);
Boolean mark_ephemeral_root(LispObj);


typedef enum {
  xmacptr_flag_none = 0,        /* Maybe already disposed by Lisp */
  xmacptr_flag_recursive_lock,  /* recursive-lock */
  xmacptr_flag_ptr,             /* malloc/free */
  xmacptr_flag_rwlock,          /* read/write lock */
  xmacptr_flag_semaphore,        /* semaphore */
  xmacptr_flag_user_first = 8,  /* first user-defined dispose fn */
  xmacptr_flag_user_last = 16   /* exclusive upper bound */
} xmacptr_flag;


typedef void (*xmacptr_dispose_fn)(void *);

extern xmacptr_dispose_fn xmacptr_dispose_functions[];

extern bitvector global_mark_ref_bits, dynamic_mark_ref_bits, relocatable_mark_ref_bits;

extern Boolean
did_gc_notification_since_last_full_gc;

extern BytePtr heap_dirty_limit;

#endif                          /* __GC_H__ */
