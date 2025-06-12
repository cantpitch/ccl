#include <assert.h>
#include <stdlib.h>

#include "arm64-constants.h"
#include "area.h"
#include "bits.h"
#include "lisptypes.h"
#include "platform-darwinarm64.h"



LispObj dnode_forwarding_address(natural dnode, int tag_n)
{
    assert(0); // This function is not implemented for this architecture.
    return 0;
}

void check_all_areas(TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
}

void check_refmap_consistency(LispObj *start, LispObj *end, bitvector refbits, bitvector refidx)
{
    assert(0); // This function is not implemented for this architecture.
}

/*
  Compact the dynamic heap (from GCfirstunmarked through its end.)
  Return the doublenode address of the new freeptr.
  */
LispObj compact_dynamic_heap()
{
    assert(0); // This function is not implemented for this architecture.
    return 0;
}

/* A "pagelet" contains 32 doublewords.  The relocation table contains
   a word for each pagelet which defines the lowest address to which
   dnodes on that pagelet will be relocated.

   The relocation address of a given pagelet is the sum of the relocation
   address for the preceding pagelet and the number of bytes occupied by
   marked objects on the preceding pagelet.
*/

LispObj calculate_relocation()
{
    assert(0); // This function is not implemented for this architecture.
    return 0;
}

void forward_cstack_area(area *a)
{
    assert(0); // This function is not implemented for this architecture.
}

void forward_range(LispObj *range_start, LispObj *range_end)
{
    assert(0); // This function is not implemented for this architecture.
}

void forward_tcr_xframes(TCR *tcr)
{
    assert(0); // This function is not implemented for this architecture.
}

void forward_tstack_area(area *a)
{
    assert(0); // This function is not implemented for this architecture.
}

void forward_vstack_area(area *a)
{
    assert(0); // This function is not implemented for this architecture.
}

LispObj locative_forwarding_address(LispObj obj)
{
    assert(0); // This function is not implemented for this architecture.
    return 0;
}

void mark_cstack_area(area *a)
{
    assert(0); // This function is not implemented for this architecture.
}

Boolean mark_ephemeral_root(LispObj n)
{
    assert(0); // This function is not implemented for this architecture.
    return false;
}

void mark_root(LispObj n)
{
    assert(0); // This function is not implemented for this architecture.
}

void mark_simple_area_range(LispObj *start, LispObj *end)
{
    assert(0); // This function is not implemented for this architecture.
}

void mark_tstack_area(area *a)
{
    assert(0); // This function is not implemented for this architecture.
}

/*
  It's really important that headers never wind up in tagged registers.
  Those registers would (possibly) get pushed on the vstack and confuse
  the hell out of this routine.

  vstacks are just treated as a "simple area range", possibly with
  an extra word at the top (where the area's active pointer points.)
  */

void mark_vstack_area(area *a)
{
    assert(0); // This function is not implemented for this architecture.
}

/* Mark the lisp objects in an exception frame */
void mark_xp(ExceptionInformation *xp)
{
    assert(0); // This function is not implemented for this architecture.
}

/*
  This wants to be in assembler even more than "mark_root" does.
  For now, it does link-inversion: hard as that is to express in C,
  reliable stack-overflow detection may be even harder ...
*/
void rmark(LispObj n)
{
    assert(0); // This function is not implemented for this architecture.
}

LispObj *skip_over_ivector(natural start, LispObj header)
{
    assert(0); // This function is not implemented for this architecture.
    return 0; // Placeholder return value.
}