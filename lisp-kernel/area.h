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

#ifndef __AREA_H__
#define __AREA_H__ 1


#include "bits.h"
#include "memprotect.h"



typedef enum {
  AREA_VOID = 0,		/* Not really an area at all */
  AREA_CSTACK = 1<<fixnumshift, /* A control stack */
  AREA_VSTACK = 2<<fixnumshift, /* A value stack.  The GC sees it as being doubleword-aligned */
  AREA_TSTACK = 3<<fixnumshift, /* A temp stack.  It -is- doubleword-aligned */
  AREA_READONLY = 4<<fixnumshift, /* A (cfm) read-only section. */
  AREA_WATCHED = 5<<fixnumshift, /* A static area containing a single object. */
  AREA_STATIC_CONS = 6<<fixnumshift, /* static, conses only */
  AREA_MANAGED_STATIC = 7<<fixnumshift, /* A resizable static area */
  AREA_STATIC = 8<<fixnumshift, /* A  static section: contains
                                 roots, but not GCed */
  AREA_DYNAMIC = 9<<fixnumshift /* A heap. Only one such area is "the heap."*/
} area_code;

typedef struct area {
  struct area* pred;            /* linked list predecessor */
  struct area* succ;            /* linked list successor */
  char* low;                    /* arithmetic lower limit on addresses
                                   (inclusive) */
  char* high;                   /* arithmetic upper limit on addresses
                                   (exclusive) */
  char* active;                 /* low bound (stack) or high bound
                                   (heap) */
  char* softlimit;		/* only makes sense for dynamic heaps
                                   & stacks */
  char* hardlimit;		/* only makes sense for dynamic heaps
                                   & stacks */
  natural code;
  natural*  markbits;           /* markbits for active area */
  natural ndnodes;		/* "active" size of dynamic area or
                                   stack */
  struct area* older;		/* if ephemeral, the next older ephemeral area
				 or the dynamic area */
  struct area* younger;         /* if ephemeral, the next "younger"
                                  ephemeral area if there is one.  If
                                  dynamic, the oldest ephemeral
                                  area. */
  char*  h;			/* The pointer allocated to contain
				 this area, or NULL if the operating
				 system allocated it for us. */
  protected_area* softprot;     /* "soft" protected_area */
  protected_area* hardprot;     /* "hard" protected_area */
  TCR * owner;                  /* TCR that the area belongs to, if a stack */
  natural*  refbits;            /* intergenerational references.  May
                                               or may not be the same
                                               as markbits */
  natural threshold;            /* egc threshold (boxed "fullword
                                   count") or 0 */
  LispObj gccount;              /* boxed generation GC count. */
  natural static_dnodes;        /* for hash consing, maybe other things. */
  natural *static_used;         /* bitvector */
  natural *refidx;              /* compressed refbits */
} area;


/*
  Areas are kept in a doubly-linked list.
  The list header is just a distinguished element of
  that list; by convention, the "active" dynamic
  area is described by that header's successor, and areas
  that may have entries in their "markbits" vector (heaps)
  precede (in the area_list->succ sense) those  that don't (stacks).
  The list header's "area" pointer is an "AREA_VOID" area; the header
  (once allocated during kernel initialization) never
  moves or changes.  Lisp code can get its hands on
  the list header via a nilreg global, and carefully,
  atomically, traverse it to do ROOM, etc.
*/


area *new_area(BytePtr, BytePtr, area_code);
void add_area(area *, TCR *);
void add_area_holding_area_lock(area *);
void condemn_area(area *, TCR *);
void condemn_area_holding_area_lock(area *);
area *area_containing(BytePtr);
area *stack_area_containing(BytePtr);
area *heap_area_containing(BytePtr);
void tenure_to_area(area *);
void untenure_from_area(area *);

Boolean grow_dynamic_area(natural);
Boolean shrink_dynamic_area(natural);

/* serialize add_area/remove_area, and also the tcr queue */
extern void *tcr_area_lock;

#define reserved_area ((area *)(all_areas))
#define active_dynamic_area ((area *)(reserved_area->succ))

/* The useable size of a tsp or vsp stack segment.
  */
#define MIN_CSTACK_SIZE (128<<10)  /* 128 KiB */
#define CSTACK_HARDPROT (100<<10)  /* 100 KiB */
#define CSTACK_SOFTPROT (100<<10)  /* 100 KiB */
#define MIN_VSTACK_SIZE (64<<10)   /* 64 KiB */
#define VSTACK_HARDPROT (4<<10)    /* 4 KiB */

#ifdef PPC
#define VSTACK_SOFTPROT (64<<10)   /* 64 KiB */
#else
#define VSTACK_SOFTPROT CSTACK_SOFTPROT /* 100 KiB */
#endif

#define MIN_TSTACK_SIZE (256<<10)  /* 256 KiB */
#define TSTACK_HARDPROT ((64<<10)+(4<<10)) /* 68 KiB */
#define TSTACK_SOFTPROT ((64<<10)+(4<<10)) /* 68 KiB */

#ifdef PPC
#define CS_OVERFLOW_FORCE_LIMIT ((natural)(-(sizeof(lisp_frame))))
#endif

#ifdef X86
#define CS_OVERFLOW_FORCE_LIMIT ((natural)(-16))
#endif

#ifdef ARM
#define CS_OVERFLOW_FORCE_LIMIT ((natural)(-(sizeof(lisp_frame))))
#endif

// NOTE: from PPC64
#ifdef ARM64
#define CS_OVERFLOW_FORCE_LIMIT ((natural)(-(sizeof(lisp_frame))))
#endif



#if (WORD_SIZE==64)
#define PURESPACE_RESERVE (128LL<<30LL) /* 128 GiB */
#define PURESPACE_SIZE (1LL<<30LL)      /* 1 GiB */
#else
#ifdef ARM
#define PURESPACE_RESERVE (64<<20)      /* 64 MiB */
#define PURESPACE_SIZE (32<<20)         /* 32 MiB */
#else
#define PURESPACE_RESERVE (128<<20)     /* 128 MiB */
#define PURESPACE_SIZE (64<<20)         /* 64 MiB */
#endif
#endif

#if defined(ARM64) && defined(DARWIN)
/* MacOS on ARM64 defines the page size as 16 KiB, so we want to use that 
   when mmapping. */
#define STATIC_RESERVE (16<<10)        /* 16 KiB */
#else
#define STATIC_RESERVE (8<<10)         /* 8 KiB */ 
#endif

#define MANAGED_STATIC_SIZE ((natural) ((PURESPACE_RESERVE-PURESPACE_SIZE)/2))

// ASLR disallows static addresses
#ifndef DARWIN_ON_ARM64
#define SPJUMP_TARGET_ADDRESS (STATIC_BASE_ADDRESS+0x3000)
#else
#define SPJUMP_TARGET_ADDRESS (static_space_start+0x3000)
#endif

extern LispObj image_base;
extern BytePtr pure_space_start, pure_space_active, pure_space_limit;
extern BytePtr static_space_start, static_space_active, static_space_limit;
extern area *find_readonly_area(void);
extern BytePtr low_relocatable_address, high_relocatable_address,
  low_markable_address, high_markable_address, reserved_region_end;

#endif /* __AREA_H__ */
