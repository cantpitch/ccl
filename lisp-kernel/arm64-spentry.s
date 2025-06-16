
/*
 * Copyright 2012 Clozure Associates
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


	include(lisp.s)
	_beginfile
	.p2align 2


local_label(start):
define(`_spentry',`ifdef(`__func_name',`_endfn',`')
	_startfn(_SP$1)
L__SP$1:                        
	.line  __line__
')


define(`_endsubp',`
	_endfn(_SP$1)
# __line__
')


	

define(`jump_builtin',`
        __(ref_nrs_value(fname,builtin_functions))
	__(set_nargs($2))
	__(ldr fname,[fname,#$1<<3])
	__(jump_fname())
')


_spentry(fix_nfn_entrypoint)
        __(nop)
        __(ret)
        
/* Construct a lisp integer out of the 32-bit signed value in imm0 */
/* arg_z should be of type (SIGNED-BYTE 32); return unboxed result in imm0 */
_spentry(gets32)
        __(nop)
        __(ret)

/*  */
/* arg_z should be of type (UNSIGNED-BYTE 32); return unboxed result in imm0 */
/*  */
_spentry(getu32)
        __(nop)
        __(ret)

_spentry(sdiv32)
        __(nop)
        __(ret)

_spentry(udiv32)
        __(nop)
        __(ret)

_spentry(udiv64by32)
        __(nop)
        __(ret)

_spentry(builtin_plus)
		__(nop)
        __(ret)
        
_spentry(builtin_minus)
        __(nop)
        __(ret)

_spentry(builtin_times)
        __(nop)
        __(ret)

_spentry(builtin_div)
        __(nop)
        __(ret)

_spentry(builtin_eq)
        __(nop)
        __(ret)
                     
_spentry(builtin_ne)
        __(nop)
        __(ret)


_spentry(builtin_gt)
        __(nop)
        __(ret)


_spentry(builtin_ge)
        __(nop)
        __(ret)

_spentry(builtin_lt)
        __(nop)
        __(ret)

_spentry(builtin_le)
        __(nop)
        __(ret)

_spentry(builtin_eql)
        __(nop)
        __(ret)
        
_spentry(builtin_length)
        __(nop)
        __(ret)     

_spentry(builtin_seqtype)
        __(nop)
        __(ret)

/* This is usually inlined these days */
_spentry(builtin_assq)
        __(nop)
        __(ret)
 
_spentry(builtin_memq)
        __(nop)
        __(ret)

_spentry(builtin_logbitp)
        __(nop)
        __(ret)

_spentry(builtin_logior)
        __(nop)
        __(ret)

_spentry(builtin_logand)
        __(nop)
        __(ret)

_spentry(builtin_ash)
        __(nop)
        __(ret)
                  	
_spentry(builtin_negate)
        __(nop)
        __(ret)

_spentry(builtin_logxor)
        __(nop)
        __(ret)

_spentry(builtin_aref1)
        __(nop)
        __(ret)

_spentry(builtin_aset1)
        __(nop)
        __(ret)

	/*  Call nfn if it's either a symbol or function */
_spentry(funcall)
        __(nop)
        __(ret)

/* Subprims for catch, throw, unwind_protect.  */


_spentry(mkcatch1v)
        __(nop)
        __(ret)

_spentry(mkcatchmv)
        __(nop)
        __(ret)

_spentry(mkunwind)
        __(nop)
        __(ret)

_spentry(bind)
        __(nop)
        __(ret)

_spentry(conslist)
        __(nop)
        __(ret)

_spentry(conslist_star)
        __(nop)
        __(ret)

_spentry(makes64)
        __(nop)
        __(ret)

_spentry(makeu64)
        __(nop)
        __(ret)

_spentry(fix_overflow)
        __(nop)
        __(ret)

_spentry(makeu128)
        __(nop)
        __(ret)

_spentry(makes128)
        __(nop)
        __(ret)

_spentry(mvpass)
        __(nop)
        __(ret)

_exportfn(C(ret1valn))
        __(nop)
        __(ret)

_spentry(values)
        __(nop)
        __(ret)

_spentry(nvalret)
	.globl C(nvalret)
C(nvalret): 
        __(nop)
        __(ret)
                       
 _spentry(throw)
         __(nop)
        __(ret)

_spentry(nthrowvalues)
        __(nop)
        __(ret)

_spentry(nthrow1value)
        __(nop)
        __(ret)

_spentry(bind_self)
        __(nop)
        __(ret)

_spentry(bind_nil)
        __(nop)
        __(ret)

_spentry(bind_self_boundp_check)
        __(nop)
        __(ret)

        .globl C(egc_write_barrier_start)
_spentry(rplaca)
C(egc_write_barrier_start):     
        __(nop)
        __(ret)

        .globl C(egc_rplacd)
_spentry(rplacd)
C(egc_rplacd):
        __(nop)
        __(ret)

	    .globl C(egc_gvset)
_spentry(gvset)
C(egc_gvset):
        __(nop)
        __(ret)
        
        .globl C(egc_set_hash_key)        
_spentry(set_hash_key)
C(egc_set_hash_key):
        __(nop)
        __(ret)

        .globl C(egc_store_node_conditional)
        .globl C(egc_write_barrier_end)
_spentry(store_node_conditional)
C(egc_store_node_conditional):

        .globl C(egc_store_node_conditional_test)
C(egc_store_node_conditional_test): 
       
        .globl C(egc_set_hash_key_conditional_test)
C(egc_set_hash_key_conditional_test): 
        __(nop)
        __(ret)

_spentry(set_hash_key_conditional)
        .globl C(egc_set_hash_key_conditional)
C(egc_set_hash_key_conditional):
         
C(egc_write_barrier_end):
        __(nop)
        __(ret)

_spentry(stkconslist)
        
C(stkconslist_star):           
        __(nop)
        __(ret)

_spentry(stkconslist_star)
       
_spentry(mkstackv)
        __(nop)
        __(ret)

	
_spentry(setqsym)
        __(nop)
        __(ret)

_spentry(progvsave)
        __(nop)
        __(ret)

_spentry(stack_misc_alloc)
        __(nop)
        __(ret)

_spentry(gvector)
        __(nop)
        __(ret)

_spentry(fitvals)
        __(nop)
        __(ret)

_spentry(nthvalue)
        __(nop)
        __(ret)



_spentry(default_optional_args)
        __(nop)
        __(ret)

_spentry(opt_supplied_p)
        __(nop)
        __(ret)

_spentry(heap_rest_arg)
        __(nop)
        __(ret)

_spentry(req_heap_rest_arg)
        __(nop)
        __(ret)

_spentry(heap_cons_rest_arg)
        __(nop)
        __(ret)

_spentry(check_fpu_exception)
        __(nop)
        __(ret)

_spentry(discard_stack_object)
        __(nop)
        __(ret)

_spentry(ksignalerr)
        __(nop)
        __(ret)

_spentry(stack_rest_arg)
        __(nop)
        __(ret)

_spentry(req_stack_rest_arg)
        __(nop)
        __(ret)

_spentry(stack_cons_rest_arg)
        __(nop)
        __(ret)
	
_spentry(call_closure)        
        __(nop)
        __(ret)

_spentry(spreadargz)
        __(nop)
        __(ret)

_spentry(tfuncallgen)
        __(nop)
        __(ret)

_spentry(tfuncallslide)
        __(nop)
        __(ret)

_spentry(jmpsym)
        __(nop)
        __(ret)

_spentry(tcallsymgen)
        __(nop)
        __(ret)


_spentry(tcallsymslide)
        __(nop)
        __(ret)

_spentry(tcallnfngen)
        __(nop)
        __(ret)

_spentry(tcallnfnslide)
        __(nop)
        __(ret)

_spentry(misc_ref)
        __(nop)
        __(ret)

_spentry(subtag_misc_ref)
        __(nop)
        __(ret)

_spentry(makestackblock)
        __(nop)
        __(ret)

_spentry(makestackblock0)
        __(nop)
        __(ret)

_spentry(makestacklist)
        __(nop)
        __(ret)

_spentry(stkgvector)
        __(nop)
        __(ret)

_spentry(misc_alloc)
        __(nop)
        __(ret)

_spentry(atomic_incf_node)
        __(nop)
        __(ret)
        
_spentry(unused1)

_spentry(unused2)

define(`mvcall_older_value_set',`node_size')
define(`mvcall_younger_value_set',`node_size+4')
        

_spentry(recover_values)
        __(nop)
        __(ret)

_spentry(integer_sign)
        __(nop)
        __(ret)

_spentry(subtag_misc_set)
         __(nop)
        __(ret)

_spentry(misc_set)
         __(nop)
        __(ret)

_spentry(spread_lexprz)
        __(nop)
        __(ret)

_spentry(reset)
        __(nop)
        __(ret)

_spentry(mvslide)
        __(nop)
        __(ret)

_spentry(save_values)
        __(nop)
        __(ret)

_spentry(add_values)
        __(nop)
        __(ret)

_spentry(misc_alloc_init)
        __(nop)
        __(ret)

_spentry(stack_misc_alloc_init)
        __(nop)
        __(ret)

_spentry(popj)
        __(nop)
        __(ret)

_spentry(getu64)
        __(nop)
        __(ret)

_spentry(gets64)
        __(nop)
        __(ret)

_spentry(specref)
        __(nop)
        __(ret)

_spentry(specrefcheck)
        __(nop)
        __(ret)

_spentry(specset)
        __(nop)
        __(ret)

_spentry(mvpasssym)
        __(nop)
        __(ret)


_spentry(unbind)
        __(nop)
        __(ret)

_spentry(unbind_n)
        __(nop)
        __(ret)

_spentry(unbind_to)
        __(nop)
        __(ret)

_spentry(progvrestore)
        __(nop)
        __(ret)

_spentry(bind_interrupt_level_0)
        __(nop)
        __(ret)

_spentry(bind_interrupt_level_m1)
        __(nop)
        __(ret)

_spentry(bind_interrupt_level)
        __(nop)
        __(ret)

_spentry(unbind_interrupt_level)
        __(nop)
        __(ret)

_spentry(aref2)
        __(nop)
        __(ret)

_spentry(aref3)
        __(nop)
        __(ret)

_spentry(aset2)
        __(nop)
        __(ret)
       
_spentry(aset3)
        __(nop)
        __(ret)


define(`keyword_flags',`arg_y')
define(`key_value_count',`arg_z')

define(`keyword_flag_allow_other_keys',`(fixnumone<<0)')
define(`keyword_flag_seen_allow_other_keys',`(fixnumone<<1)')
define(`keyword_flag_rest',`(fixnumone<<2)')
define(`keyword_flag_unknown_keyword_seen',`(fixnumone<<3)')
define(`keyword_flag_current_aok',`(fixnumone<<4)')

_spentry(keyword_bind)
        __(nop)
        __(ret)

_spentry(eabi_ff_callhf)
        __(nop)
        __(ret)
_spentry(eabi_ff_call)
        __(nop)
        __(ret)

_spentry(eabi_callback)
        __(nop)
        __(ret)

_startfn(C(misc_ref_common))
        __(nop)
        __(ret)

_endfn
        
_startfn(C(misc_set_common))
        __(nop)
        __(ret)
              
_startfn(C(_throw_found))
        __(nop)
        __(ret)
_endfn(C(_throw_found))        

_startfn(C(nthrow1v))
        __(nop)
        __(ret)
_endfn        

_startfn(C(nthrownv))
        __(nop)
        __(ret)
_endfn                

               
_startfn(stack_misc_alloc_init_no_room)
        __(nop)
        __(ret)
_endfn        

_startfn(toplevel_loop)
        __(nop)
        __(ret)
_endfn

	.globl _SPreset
_exportfn(C(start_lisp))
        __(nop)
        __(ret)
_endfn
        

_exportfn(C(init_lisp))
        __(nop)
        __(ret)
_endfn

                                
        .data
        .align 4
        .globl C(sptab)
        .align 4
        .globl C(sptab_end)
        .align 4
C(sptab): 
C(sptab_end):                   
        .quad _SPfix_nfn_entrypoint /* must be first */
        .quad _SPbuiltin_plus
        .quad _SPbuiltin_minus
        .quad _SPbuiltin_times
        .quad _SPbuiltin_div
        .quad _SPbuiltin_eq
        .quad _SPbuiltin_ne
        .quad _SPbuiltin_gt
        .quad _SPbuiltin_ge
        .quad _SPbuiltin_lt
        .quad _SPbuiltin_le
        .quad _SPbuiltin_eql
        .quad _SPbuiltin_length
        .quad _SPbuiltin_seqtype
        .quad _SPbuiltin_assq
        .quad _SPbuiltin_memq
        .quad _SPbuiltin_logbitp
        .quad _SPbuiltin_logior
        .quad _SPbuiltin_logand
        .quad _SPbuiltin_ash
        .quad _SPbuiltin_negate
        .quad _SPbuiltin_logxor
        .quad _SPbuiltin_aref1
        .quad _SPbuiltin_aset1
        .quad _SPfuncall
        .quad _SPmkcatch1v
        .quad _SPmkcatchmv
        .quad _SPmkunwind
        .quad _SPbind
        .quad _SPconslist
        .quad _SPconslist_star
        .quad _SPmakes64
        .quad _SPmakeu64
        .quad _SPfix_overflow
        .quad _SPmakeu128
        .quad _SPmakes128
        .quad _SPmvpass
        .quad _SPvalues
        .quad _SPnvalret
        .quad _SPthrow
        .quad _SPnthrowvalues
        .quad _SPnthrow1value
        .quad _SPbind_self
        .quad _SPbind_nil
        .quad _SPbind_self_boundp_check
        .quad _SPrplaca
        .quad _SPrplacd
        .quad _SPgvset
        .quad _SPset_hash_key
        .quad _SPstore_node_conditional
        .quad _SPset_hash_key_conditional
        .quad _SPstkconslist
        .quad _SPstkconslist_star
        .quad _SPmkstackv
        .quad _SPsetqsym
        .quad _SPprogvsave
        .quad _SPstack_misc_alloc
        .quad _SPgvector
        .quad _SPfitvals
        .quad _SPnthvalue
        .quad _SPdefault_optional_args
        .quad _SPopt_supplied_p
        .quad _SPheap_rest_arg
        .quad _SPreq_heap_rest_arg
        .quad _SPheap_cons_rest_arg
        .quad _SPcheck_fpu_exception
        .quad _SPdiscard_stack_object
        .quad _SPksignalerr
        .quad _SPstack_rest_arg
        .quad _SPreq_stack_rest_arg
        .quad _SPstack_cons_rest_arg
        .quad _SPcall_closure        
        .quad _SPspreadargz
        .quad _SPtfuncallgen
        .quad _SPtfuncallslide
        .quad _SPjmpsym
        .quad _SPtcallsymgen
        .quad _SPtcallsymslide
        .quad _SPtcallnfngen
        .quad _SPtcallnfnslide
        .quad _SPmisc_ref
        .quad _SPsubtag_misc_ref
        .quad _SPmakestackblock
        .quad _SPmakestackblock0
        .quad _SPmakestacklist
        .quad _SPstkgvector
        .quad _SPmisc_alloc
        .quad _SPatomic_incf_node
        .quad _SPunused1
        .quad _SPunused2
        .quad _SPrecover_values
        .quad _SPinteger_sign
        .quad _SPsubtag_misc_set
        .quad _SPmisc_set
        .quad _SPspread_lexprz
        .quad _SPreset
        .quad _SPmvslide
        .quad _SPsave_values
        .quad _SPadd_values
        .quad _SPmisc_alloc_init
        .quad _SPstack_misc_alloc_init
        .quad _SPpopj
        .quad _SPudiv64by32
        .quad _SPgetu64
        .quad _SPgets64
        .quad _SPspecref
        .quad _SPspecrefcheck
        .quad _SPspecset
        .quad _SPgets32
        .quad _SPgetu32
        .quad _SPmvpasssym
        .quad _SPunbind
        .quad _SPunbind_n
        .quad _SPunbind_to
        .quad _SPprogvrestore
        .quad _SPbind_interrupt_level_0
        .quad _SPbind_interrupt_level_m1
        .quad _SPbind_interrupt_level
        .quad _SPunbind_interrupt_level
        .quad _SParef2
        .quad _SParef3
        .quad _SPaset2
        .quad _SPaset3
        .quad _SPkeyword_bind
        .quad _SPudiv32
        .quad _SPsdiv32
        .quad _SPeabi_ff_call
        .quad _SPeabi_callback
        .quad _SPeabi_ff_callhf
local_label(end):       
        	_endfile
