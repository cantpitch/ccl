/*
 * Copyright 1994-2025 Clozure Associates
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

/* dnode_align(dest,src,delta) */
define(`dnode_align',`
    __(add $1,$2,#$3+(dnode_size-1))
    __(bic $1,$1,#((1<<dnode_align_bits)-1))
')

define(`extract_typecode',`
    new_macro_labels()
    __(extract_fulltag($1,$2))
    __(cmp $1,#fulltag_misc)
    __(extract_lisptag($1,$1))
    __(b.ne macro_label(not_misc))
    __(extract_subtag($1,$2))
macro_label(not_misc):
')

/* "real" stack operations on ARM64 require that the stack pointer be 16-byte aligned.
   https://community.arm.com/arm-community-blogs/b/architectures-and-processors-blog/posts/using-the-stack-in-aarch64-implementing-push-and-pop
 */


// define(`push',`
//     __(stru($1,-node_size($2)))
//     ')
    
//     /* Generally not a great idea. */
// define(`pop',`
//     __(ldr($1,0($2)))
//     __(la $2,node_size($2))
//     ')
    
define(`vpush',`
    __(str $1,[vsp,#-node_size]!)
')
    
define(`vpop',`
    __(ldr $1,[vsp],#node_size)
')

define(`vref64',`
    __(ldr $1,[$2,#misc_data_offset+(($3)<<3)])
')

define(`vrefr',`
    __(vref64($1,$2,$3))
')

// //     /* Size is unboxed element count */
// // define(`header_size',`
// //     __(srri($1,$2,num_subtag_bits))
// //     ')

// define(`extract_unsigned_byte_bits',`
// ifdef(`PPC64',`
//         __(rldicr $1,$2,64-fixnumshift,63-$3)
// ',`                
//         __(rlwinm $1,$2,0,32-fixnumshift,31-($3+fixnumshift))
// ')        
// ')

// define(`extract_unsigned_byte_bits_',`
// ifdef(`PPC64',`
//         __(rldicr. $1,$2,64-fixnumshift,63-$3)
// ',`                
//         __(rlwinm. $1,$2,0,32-fixnumshift,31-($3+fixnumshift))
// ')        
// ')

//     /* vpop argregs - nargs is known to be non-zero */
// define(`vpop_argregs_nz',`
//     new_macro_labels()
//     __(cmplri(cr1,nargs,node_size*2))
//     __(vpop(arg_z))
//     __(blt cr1,macro_label(l0))
//     __(vpop(arg_y))
//     __(bne cr1,macro_label(l0))
//     __(vpop(arg_x))
// macro_label(l0):')

                
//     /* vpush argregs */
// define(`vpush_argregs',`
//     new_macro_labels()
//     __(cmplri(cr0,nargs,0))
//     __(cmplri(cr1,nargs,node_size*2))
//     __(beq cr0,macro_label(done))
//     __(blt cr1,macro_label(z))
//     __(beq cr1,macro_label(yz))
//     __(vpush(arg_x))
// macro_label(yz):
//     __(vpush(arg_y))
// macro_label(z):
//     __(vpush(arg_z))
// macro_label(done):
// ')

// define(`create_lisp_frame',`
//     __(stru(sp,-lisp_frame.size(sp)))
// ')

                
// define(`build_lisp_frame',`
//     create_lisp_frame()
//     __(str(ifelse($1,`',fn,$1),lisp_frame.savefn(sp)))
//     __(str(ifelse($2,`',loc_pc,$2),lisp_frame.savelr(sp)))
//     __(str(ifelse($3,`',vsp,$3),lisp_frame.savevsp(sp)))
// ')

            
// define(`discard_lisp_frame',`
//     __(la sp,lisp_frame.size(sp))
//     ')


define(`vpush_saveregs',`
    __(vpush(save7))
    __(vpush(save6))
    __(vpush(save5))
    __(vpush(save4))
    __(vpush(save3))
    __(vpush(save2))
    __(vpush(save1))
    __(vpush(save0))
    ')
    
define(`restore_saveregs',`
    __(ldr(save0,node_size*0($1)))
    __(ldr(save1,node_size*1($1)))
    __(ldr(save2,node_size*2($1)))
    __(ldr(save3,node_size*3($1)))
    __(ldr(save4,node_size*4($1)))
    __(ldr(save5,node_size*5($1)))
    __(ldr(save6,node_size*6($1)))
    __(ldr(save7,node_size*7($1)))
')

define(`vpop_saveregs',`
    __(restore_saveregs(vsp))
    __(la vsp,node_size*8(vsp))
')

define(`trap_unless_lisptag_equal',`
    __(extract_lisptag($3,$1))
    __(trnei($3,$2))
')

ifdef(`PPC64',`
define(`trap_unless_list',`
    new_macro_labels()
    __(cmpdi ifelse($3,$3,cr0),$1,nil_value)
    __(extract_fulltag($2,$1))
    __(beq ifelse($3,$3,cr0),macro_label(is_list))
    __(tdnei $2,fulltag_cons)
macro_label(is_list):	

')',`	
define(`trap_unless_list',`
    __(trap_unless_lisptag_equal($1,tag_list,$2))
')
')

define(`trap_unless_fulltag_equal',`
    __(extract_fulltag($3,$1))
    __(trnei($3,$2))
')
    
define(`trap_unless_typecode_equal',`
        __(extract_typecode($3,$1))
        __(trnei($3,$2))
')
        
/* "jump" to the code-vector of the function in nfn. */
define(`jump_nfn',`
    __(ldr(temp0,_function.codevector(nfn)))
    __(mtctr temp0)
    __(bctr)
')

/* "call the code-vector of the function in nfn. */
define(`call_nfn',`
    __(ldr(temp0,_function.codevector(nfn)))
    __(mtctr temp0)
    __(bctrl)
')
    

/* "jump" to the function in fnames function cell. */
define(`jump_fname',`
    __(ldr(nfn,symbol.fcell(fname)))
    __(jump_nfn())
')

/* call the function in fnames function cell. */
define(`call_fname',`
    __(ldr(nfn,symbol.fcell(fname)))
    __(call_nfn())
')

define(`do_funcall',`
    new_macro_labels()
    __(extract_fulltag(imm0,temp0))
    __(cmpri(imm0,fulltag_misc))
    __(mr nfn,temp0)
    __(bne- macro_label(bad))
    __(extract_subtag(imm0,temp0))
    __(cmpri(imm0,subtag_function))
    __(cmpri(cr1,imm0,subtag_symbol))
        __(bne cr0,macro_label(_sym))
        __(jump_nfn())
macro_label(_sym):             
    __(mr fname,temp0)
    __(bne cr1,macro_label(bad))
    __(jump_fname())
macro_label(bad):
    __(uuo_interr(error_cant_call,temp0))
')	

define(`mkcatch',`
    __(mflr loc_pc)
    __(ldr(imm0,tcr.catch_top(rcontext)))
    __(lwz imm1,0(loc_pc)) /* a forward branch to the catch/unwind cleanup */
    __(rlwinm imm1,imm1,0,6,29)	/* extract LI */
    __(add loc_pc,loc_pc,imm1)
    __(build_lisp_frame(fn,loc_pc,vsp))
    __(sub loc_pc,loc_pc,imm1)
    __(la loc_pc,4(loc_pc))	/* skip over the forward branch */
    __(mtlr loc_pc)
    __(lwi(imm4,(catch_frame.element_count<<num_subtag_bits)|subtag_catch_frame))
    __(ldr(imm3,tcr.xframe(rcontext)))
    __(ldr(imm1,tcr.db_link(rcontext)))
    __(TSP_Alloc_Fixed_Unboxed(catch_frame.size))
    __(la nargs,tsp_frame.data_offset+fulltag_misc(tsp))
        __(str(imm4,catch_frame.header(nargs)))
        __(ldr(imm4,tcr.nfp(rcontext)))
    __(str(arg_z,catch_frame.catch_tag(nargs)))
    __(str(imm0,catch_frame.link(nargs)))
    __(str(imm2,catch_frame.mvflag(nargs)))
    __(str(sp,catch_frame.csp(nargs)))
    __(str(imm1,catch_frame.db_link(nargs)))
        __(str(first_nvr,catch_frame.regs+0*node_size(nargs)))
        __(str(second_nvr,catch_frame.regs+1*node_size(nargs)))
        __(str(third_nvr,catch_frame.regs+2*node_size(nargs)))
        __(str(fourth_nvr,catch_frame.regs+3*node_size(nargs)))
        __(str(fifth_nvr,catch_frame.regs+4*node_size(nargs)))
        __(str(sixth_nvr,catch_frame.regs+5*node_size(nargs)))
        __(str(seventh_nvr,catch_frame.regs+6*node_size(nargs)))
        __(str(eighth_nvr,catch_frame.regs+7*node_size(nargs)))
    __(str(imm3,catch_frame.xframe(nargs)))
         __(str(imm4,catch_frame.nfp(nargs)))
    __(Set_TSP_Frame_Boxed())
    __(str(nargs,tcr.catch_top(rcontext)))
        __(li nargs,0)

')	

define(`restore_catch_nvrs',`
        __(ldr(first_nvr,catch_frame.regs+(node_size*0)($1)))
        __(ldr(second_nvr,catch_frame.regs+(node_size*1)($1)))
        __(ldr(third_nvr,catch_frame.regs+(node_size*2)($1)))
        __(ldr(fourth_nvr,catch_frame.regs+(node_size*3)($1)))
        __(ldr(fifth_nvr,catch_frame.regs+(node_size*4)($1)))
        __(ldr(sixth_nvr,catch_frame.regs+(node_size*5)($1)))
        __(ldr(seventh_nvr,catch_frame.regs+(node_size*6)($1)))
        __(ldr(eighth_nvr,catch_frame.regs+(node_size*7)($1)))
')               

define(`DCBZL',`
    __(.long (31<<26)+(1<<21)+($1<<16)+($2<<11)+(1014<<1))
')
    
define(`check_stack_alignment',`
    new_macro_labels()
    __(andi. $1,sp,STACK_ALIGN_MASK)
    __(beq+ macro_label(stack_ok))
    __(.long 0)
macro_label(stack_ok):
')

define(`stack_align',`((($1)+STACK_ALIGN_MASK)&~STACK_ALIGN_MASK)')

define(`clear_alloc_tag',`
    __(clrrri(allocptr,allocptr,ntagbits))
')

/* If the GC interrupts the current thread (after the trap), it needs */
/*   to ensure that the cons cell that's been "reserved" stays reserved */
/*   (e.g. the tagged allocptr has to be treated as a node.)  If that */
/*   reserved cons cell gets tenured, the car and cdr are of a generation */
/*   that's at least as old (so memoization isn't an issue.) */

/*   More generally, if the GC interrupts a thread when allocptr is */
/*   tagged as a cons: */

/*    a) if the trap hasn't been taken (yet), the GC should force the */
/*       thread to resume in such a way that the trap will be taken ; */
/*       the segment allocator should worry about allocating the object. */

/*    b) If the trap has been taken, allocptr is treated as a node as */
/*       described above.  Allocbase is made to point to the base of the */
/*       cons cell, so that the thread's next allocation attempt will */
/*       invoke the segment allocator. */
    
define(`Cons',`
    __(la allocptr,(-cons.size+fulltag_cons)(allocptr))
        __(alloc_trap())
    __(str($3,cons.cdr(allocptr)))
    __(str($2,cons.car(allocptr)))
    __(mr $1,allocptr)
    __(clear_alloc_tag())
')


/* This is probably only used once or twice in the entire kernel, but */
/* I wanted a place to describe the constraints on the mechanism. */

/* Those constaints are (not surprisingly) similar to those which apply */
/* to cons cells, except for the fact that the header (and any length */
/* field that might describe large arrays) has to have been stored in */
/* the object if the trap has succeeded on entry to the GC.  It follows */
/* that storing the register containing the header must immediately */
/* follow the allocation trap (and an auxiliary length register must */
/* be stored immediately after the header.)  Successfully falling */
/* through the trap must emulate any header initialization: it would */
/* be a bad idea to have allocptr pointing to a zero header ... */



/* Parameters: */

/* $1 = dest reg */
/* $2 = header.  (For now, assume that this always encodes length ; */
/* that may change with "large vector" support.) */
/* $3 = register containing size in bytes.  (We're going to subtract */
/* fulltag_misc from this; do it in the macro body, rather than force the
/* (1 ?) caller to do it. */


define(`Misc_Alloc',`
    __(la $3,-fulltag_misc($3))
    __(sub allocptr,allocptr,$3)
        __(alloc_trap())
    __(str($2,misc_header_offset(allocptr)))
    __(mr $1,allocptr)
    __(clear_alloc_tag())
')

/*  Parameters $1, $2 as above; $3 = physical size constant. */
define(`Misc_Alloc_Fixed',`
    __(la allocptr,(-$3)+fulltag_misc(allocptr))
        __(alloc_trap())
    __(str($2,misc_header_offset(allocptr)))
    __(mr $1,allocptr)
    __(clear_alloc_tag())
')


/*  Zero $3 bytes worth of doublewords, starting at offset $2 relative */
/* to the base register $1. */


ifdef(`DARWIN',`
    .macro zero_doublewords
    .if $2
    stfd fp_zero,$1($0)
    zero_doublewords $0,$1+8,$2-8
    .endif
    .endmacro
')

ifdef(`LINUX',`
    .macro zero_doublewords base,disp,nbytes
    .if \nbytes
    stfd fp_zero,\disp(\base)
    zero_doublewords \base,\disp+8,\nbytes-8
    .endif
    .endm
')	

define(`Set_TSP_Frame_Unboxed',`
    __(str(tsp,tsp_frame.type(tsp)))
')

define(`Set_TSP_Frame_Boxed',`
    __(str(rzero,tsp_frame.type(tsp)))
')
        
/* A newly allocated TSP frame is always "raw" (has non-zero type, indicating */
/* that it doesn't contain tagged data. */

define(`TSP_Alloc_Fixed_Unboxed',`
    __(stru(tsp,-($1+tsp_frame.data_offset)(tsp)))
    __(Set_TSP_Frame_Unboxed())
')

define(`TSP_Alloc_Fixed_Unboxed_Zeroed',`
    __(TSP_Alloc_Fixed_Unboxed($1))
    __(zero_doublewords tsp,tsp_frame.fixed_overhead,$1)
')

define(`TSP_Alloc_Fixed_Boxed',`
    __(TSP_Alloc_Fixed_Unboxed_Zeroed($1))
    __(Set_TSP_Frame_Boxed())
')


        
    

/* This assumes that the backpointer points  to the first byte beyond */
/* each frame.  If we allow segmented tstacks, that constraint might */
/* complicate  their implementation. */
/* We don't need to know the size of the frame (positive or negative, */
/* with or without header).  $1 and $2 are temp registers, $3 is an */
/* optional CR field. */


/* Handle the general case, where the frame might be empty */
define(`Zero_TSP_Frame',`
    __(new_macro_labels())
    __(la $1,tsp_frame.size-8(tsp))
    __(ldr($2,tsp_frame.backlink(tsp)))
    __(la $2,-8($2))
    __(b macro_label(zero_tsp_test))
macro_label(zero_tsp_loop):
    __(stfdu fp_zero,8($1))
macro_label(zero_tsp_test):	
    __(cmpr(ifelse($3,`',`cr0',$3),$1,$2))
    __(bne ifelse($3,`',`cr0',$3),macro_label(zero_tsp_loop))
')

/* Save some branching when we know that the frame can't be empty.*/
define(`Zero_TSP_Frame_nz',`
    new_macro_labels()
    __(la $1,tsp_frame.size-8(tsp))
    __(ldr($2,tsp_frame.backlink(tsp)))
    __(la $2,-8($2))
macro_label(zero_tsp_loop):
    __(stfdu fp_zero,8($1))
    __(cmpr(ifelse($3,`',`cr0',$3),$1,$2))
    __(bne ifelse($3,`',`cr0',$3),macro_label(zero_tsp_loop))
')
    
/* $1 = 8-byte-aligned size, positive.  $2 (optiional) set */
/* to negated size. */
define(`TSP_Alloc_Var_Unboxed',`
    __(neg ifelse($2,`',$1,$2),$1)
    __(strux(tsp,tsp,ifelse($2,`',$1,$2)))
    __(Set_TSP_Frame_Unboxed())
')

define(`TSP_Alloc_Var_Boxed',`
    __(TSP_Alloc_Var_Unboxed($1))
    __(Zero_TSP_Frame($1,$2))
    __(Set_TSP_Frame_Boxed())
')		


define(`TSP_Alloc_Var_Boxed_nz',`
    __(TSP_Alloc_Var_Unboxed($1))
    __(Zero_TSP_Frame_nz($1,$2))
    __(Set_TSP_Frame_Boxed())
')		

define(`check_pending_interrupt',`
    new_macro_labels()
        __(ldr(nargs,tcr.tlb_pointer(rcontext)))
    __(ldr(nargs,INTERRUPT_LEVEL_BINDING_INDEX(nargs)))
    __(cmpri(ifelse($1,`',`cr0',$1),nargs,0))
    __(blt ifelse($1,`',`cr0',$1),macro_label(done))
    __(bgt ifelse($1,`',`cr0',$1),macro_label(trap))
    __(ldr(nargs,tcr.interrupt_pending(rcontext)))
macro_label(trap):
    __(trgti(nargs,0))
macro_label(done):
')

/* $1 = ndigits.  Assumes 4-byte digits */        
define(`aligned_bignum_size',`((~(dnode_size-1)&(node_size+(dnode_size-1)+(4*$1))))')

define(`suspend_now',`
    __(uuo_interr(error_propagate_suspend,rzero))
')

define(`ivector_typecode_p',`
    new_macro_labels()
        ifdef(`PPC64',`
         __(clrldi $3,$2,64-nlowtagbits)
         __(cmpdi $3,lowtag_immheader)
         __(mr $1,$2)
         __(beq macro_label(done))
         __(mr $1,rzero)
        ',`
         __(clrlwi $3,$2,32-ntagbits)
         __(cmpwi $3,fulltag_immheader)
         __(mr $1,$2)
         __(beq macro_label(done))
         __(mr $1,rzero)
        ')
macro_label(done):
        ')
