/*
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

define(`imm0',`x0')  
define(`imm1',`x1')
define(`imm2',`x2')
define(`imm3',`x3')
define(`imm4',`x4')
define(`imm5',`x5') 
define(`imm6',`x6')
define(`imm7',`x7')
define(`nargs',`x8')

define(`temp2',`x10')
define(`temp1',`x11')
define(`temp0',`x12')
define(`arg_x',`x13')                
define(`arg_y',`x14')                
define(`arg_z',`x15')                
define(`save0',`x16')
define(`save1',`x17')
/* macos uses x18 (Platform Register) https://stackoverflow.com/questions/71152539/consequence-of-violating-macoss-arm64-calling-convention */
define(`save2',`x18')
define(`save3',`x19')
define(`save4',`x20')
define(`save5',`x21')
define(`save6',`x22')
define(`save7',`x23')       
define(`loc_pc',`x24')
define(`vsp',`x25')
define(`allocptr',`x26')
define(`allocbase',`x27')                
define(`rcontext',`x28')
define(`fp',`x29') /* frame pointer */
define(`lr',`x30') /* link register */

/* Non-volatile registers (callee-saved) for ARM64 are x19-x28, x30 (lr) */

define(`fname',`temp3')
define(`nfn',`temp2')


// /* Lisp code keeps 0.0 in fp_zero */
// define(`fp_zero',`f31')   /* a non-volatile reg as far as FFI is concerned. */
// define(`fp_s32conv',`f30')   /* for s32->fp conversion */

/* registers, as used in destrucuring-bind/macro-bind */

define(`whole_reg',`temp1')
define(`arg_reg',`temp3')
define(`keyvect_reg',`temp2')
define(`mask_req_start',`24')
define(`mask_req_width',`8')
define(`mask_opt_start',`16')
define(`mask_opt_width',`8')
define(`mask_key_start',`8')
define(`mask_key_width',`8')
define(`mask_initopt',`7')
define(`mask_keyp',`6') /*  note that keyp can be true even when 0 keys. */
define(`mask_aok',`5')
define(`mask_restp',`4')

ifdef(`DARWIN',`
    define(`STACK_ALIGN',16)
    define(`STACK_ALIGN_MASK',15)
',`
    define(`STACK_ALIGN',8)
    define(`STACK_ALIGN_MASK',7)
')

/* Indices in %builtin-functions% */
_builtin_plus = 0	/* +-2 */
_builtin_minus = 1	/* --2 */
_builtin_times = 2	/* *-2 */
_builtin_div = 3	/* /-2 */
_builtin_eq = 4		/* =-2 */
_builtin_ne = 5		/* /-2 */
_builtin_gt = 6		/* >-2 */
_builtin_ge = 7		/* >=-2 */
_builtin_lt = 8		/* <-2 */
_builtin_le = 9		/* <=-2 */
_builtin_eql = 10	/* eql */
_builtin_length = 11	/* length */
_builtin_seqtype = 12	/* sequence-type */
_builtin_assq = 13	/* assq */
_builtin_memq = 14	/* memq */
_builtin_logbitp = 15	/* logbitp */
_builtin_logior = 16	/* logior-2 */
_builtin_logand = 17	/* logand-2 */
_builtin_ash = 18	/* ash */
_builtin_negate = 19	/* %negate */
_builtin_logxor = 20	/* logxor-2 */
_builtin_aref1 = 21	/* %aref1 */
_builtin_aset1 = 22	/* %aset1 */

//     /* FPSCR status bits */
// fpscr_FX = 0
// fpscr_FEX = 1
// fpscr_VX = 2
// fpscr_OX = 3
// fpscr_UX = 4
// fpscr_ZX = 5
// fpscr_XX = 6
//     /* FPSCR control bits */
// fpscr_VE = 24
// fpscr_OE = 25
// fpscr_UE = 26
// fpscr_ZE = 27
// fpscr_XE = 28

/* This should be (a) an (UNSIGNED-BYTE 16) and (b) one less than */
/* TSTACK_SOFTPROT (defined in "area.h") */
        
tstack_alloc_limit = 0xffff
        
define(`TCR_STATE_FOREIGN',1)
define(`TCR_STATE_LISP',0)
define(`TCR_STATE_EXCEPTION_WAIT',2)
define(`TCR_STATE_EXCEPTION_RETURN',4)


nbits_in_word = 64
nbits_in_byte = 8
ntagbits = 4
nlisptagbits = 3
nfixnumtagbits = 3
nlowtagbits = 2        
num_subtag_bits = 8
fixnumshift = 3
fixnum_shift = 3
fulltagmask = 15
tagmask = 7
fixnummask = 7
subtag_mask = 0xff 
ncharcodebits = 8
charcode_shift = 8
word_shift = 3
node_size = 8
dnode_size = 16
dnode_align_bits = 4
dnode_shift = dnode_align_bits        
bitmap_shift = 6

        
fixnumone = (1<<fixnumshift)
fixnum_one = fixnumone
fixnum1 = fixnumone


lowtagmask = ((1<<nlowtagbits)-1)
lowtag_mask = lowtagmask

lowtag_primary = 0
lowtag_imm = 1
lowtag_immheader = 2
lowtag_nodeheader = 3

tag_fixnum = 0

fulltag_even_fixnum = 0
fulltag_imm_0 = 1
fulltag_immheader_0 = 2
fulltag_nodeheader_0 = 3
fulltag_cons = 4
fulltag_imm_1 = 5
fulltag_immheader_1 = 6
fulltag_nodeheader_1 = 7
fulltag_odd_fixnum = 8
fulltag_imm_2 = 9
fulltag_immheader_2 = 10
fulltag_nodeheader_2 = 11
fulltag_misc = 12
fulltag_imm_3 = 13
fulltag_immheader_3 = 14
fulltag_nodeheader_3 = 15

define(`define_subtag',`
subtag_$1 = ($2 | ($3 << ntagbits))
')
            
cl_array_subtag_mask = 0x80
define(`define_cl_array_subtag',`
define_subtag($1,(cl_array_subtag_mask|$2),$3)
')

define_cl_array_subtag(arrayH,fulltag_nodeheader_1,0)
define_cl_array_subtag(vectorH,fulltag_nodeheader_2,0)
define_cl_array_subtag(simple_vector,fulltag_nodeheader_3,0)
min_vector_subtag = subtag_vectorH
min_array_subtag = subtag_arrayH
        
    
ivector_class_64_bit = fulltag_immheader_3
ivector_class_32_bit = fulltag_immheader_2
ivector_class_other_bit = fulltag_immheader_1
ivector_class_8_bit = fulltag_immheader_0

define_cl_array_subtag(s64_vector,ivector_class_64_bit,1)
define_cl_array_subtag(u64_vector,ivector_class_64_bit,2)
define_cl_array_subtag(fixnum_vector,ivector_class_64_bit,3)        
define_cl_array_subtag(double_float_vector,ivector_class_64_bit,4)
define_cl_array_subtag(complex_single_float_vector,ivector_class_64_bit,5)
define_cl_array_subtag(s32_vector,ivector_class_32_bit,1)
define_cl_array_subtag(u32_vector,ivector_class_32_bit,2)
define_cl_array_subtag(single_float_vector,ivector_class_32_bit,3)
define_cl_array_subtag(simple_base_string,ivector_class_32_bit,5)
define_cl_array_subtag(s16_vector,ivector_class_other_bit,1)
define_cl_array_subtag(u16_vector,ivector_class_other_bit,2)
define_cl_array_subtag(complex_double_float_vector,ivector_class_other_bit,3)
define_cl_array_subtag(bit_vector,ivector_class_other_bit,7)
define_cl_array_subtag(s8_vector,ivector_class_8_bit,1)
define_cl_array_subtag(u8_vector,ivector_class_8_bit,2)
min_cl_ivector_subtag = subtag_s8_vector        
/* There's some room for expansion in non-array ivector space. */
define_subtag(macptr,ivector_class_64_bit,1)
define_subtag(dead_macptr,ivector_class_64_bit,2)
define_subtag(code_vector,ivector_class_32_bit,0)
define_subtag(xcode_vector,ivector_class_32_bit,1)
define_subtag(bignum,ivector_class_32_bit,2)
define_subtag(double_float,ivector_class_32_bit,3)
define_subtag(complex_single_float,ivector_class_32_bit,4)
define_subtag(complex_double_float,ivector_class_32_bit,5)


/* Size doesn't matter for non-CL-array gvectors; I can't think of a good */
/* reason to classify them in any particular way.  Let's put funcallable */
/* things in the first slice by themselves, though it's not clear that */
/* that helps FUNCALL much. */
        
gvector_funcallable = fulltag_nodeheader_0
    
define_subtag(function,gvector_funcallable,0)
define_subtag(symbol,gvector_funcallable,1)
define_subtag(catch_frame,fulltag_nodeheader_1,0)
define_subtag(basic_stream,fulltag_nodeheader_1,1)
define_subtag(lock,fulltag_nodeheader_1,2)
define_subtag(hash_vector,fulltag_nodeheader_1,3)
define_subtag(pool,fulltag_nodeheader_1,4)
define_subtag(weak,fulltag_nodeheader_1,5)
define_subtag(package,fulltag_nodeheader_1,6)
        
define_subtag(slot_vector,fulltag_nodeheader_2,0)
define_subtag(instance,fulltag_nodeheader_2,1)
define_subtag(struct,fulltag_nodeheader_2,2)
define_subtag(istruct,fulltag_nodeheader_2,3)
define_subtag(value_cell,fulltag_nodeheader_2,4)
define_subtag(xfunction,fulltag_nodeheader_2,5)
    
define_subtag(ratio,fulltag_nodeheader_3,0)
define_subtag(complex,fulltag_nodeheader_3,1)
            
t_value = (0x3000+fulltag_misc)	
misc_bias = fulltag_misc
cons_bias = fulltag_cons
define(`t_offset',-symbol.size)
    
misc_header_offset = -fulltag_misc
misc_data_offset = misc_header_offset+node_size /* first word of data */
misc_subtag_offset = misc_data_offset-1       /* low byte of header */
misc_dfloat_offset = misc_data_offset		/* double-floats are doubleword-aligned */

define_subtag(single_float,fulltag_imm_0,0)

define_subtag(go_tag,fulltag_imm_1,0)
define_subtag(block_tag,fulltag_imm_1,1)

define_subtag(character,fulltag_imm_1,0)
                    
define_subtag(unbound,fulltag_imm_3,0)
unbound_marker = subtag_unbound
undefined = unbound_marker
define_subtag(slot_unbound,fulltag_imm_3,1)
slot_unbound_marker = subtag_slot_unbound
define_subtag(illegal,fulltag_imm_3,2)
illegal_marker = subtag_illegal
define_subtag(no_thread_local_binding,fulltag_imm_3,3)
no_thread_local_binding_marker = subtag_no_thread_local_binding        

    
max_64_bit_constant_index = ((0x7fff + misc_dfloat_offset)>>3)
max_32_bit_constant_index = ((0x7fff + misc_data_offset)>>2)
max_16_bit_constant_index = ((0x7fff + misc_data_offset)>>1)
max_8_bit_constant_index = (0x7fff + misc_data_offset)
max_1_bit_constant_index = ((0x7fff + misc_data_offset)<<5)


    
/* The objects themselves look something like this: */
    
/* Order of CAR and CDR doesn't seem to matter much - there aren't */
/* too many tricks to be played with predecrement/preincrement addressing. */
/* Keep them in the confusing MCL 3.0 order, to avoid confusion. */
    _struct(cons,-cons_bias)
        _node(cdr)
        _node(car)
    _ends
    
    _structf(ratio)
        _node(numer)
        _node(denom)
    _endstructf
    
    _structf(double_float)
        _word(value)
        _word(val_low)
    _endstructf

    _structf(complex_single_float)
        _word(realpart)
        _word(imagpart)
    _endstructf

    _structf(complex_double_float)
        _field(pad,8)
        _field(realpart,8)
        _field(imagpart,8)
    _endstructf
    
    _structf(macptr)
        _node(address)
        _node(domain)
        _node(type)
    _endstructf
    
/* Functions are of (conceptually) unlimited size. */
    _struct(_function,-misc_bias)
        _node(header)
        _node(codevector)
    _ends

    _struct(tsp_frame,0)
        _node(backlink)
        _node(type)
        _struct_label(fixed_overhead)
        _struct_label(data_offset)
    _ends

    _structf(symbol)
        _node(pname)
        _node(vcell)
        _node(fcell)
        _node(package_predicate)
        _node(flags)
        _node(plist)
        _node(binding_index)
    _endstructf

    _structf(catch_frame)
        _node(catch_tag) /* #<unbound> -> unwind-protect, else catch */
        _node(link)         /* backpointer to previous catch frame */
        _node(mvflag)		/* 0 if single-valued catch, fixnum 1 otherwise */
        _node(csp)		/* pointer to lisp_frame on csp */
        _node(db_link)		/* head of special-binding chain */
        _field(regs,8*node_size)	/* save7-save0 */
        _node(xframe)		/* exception frame chain */
        _node(nfp)             
    _endstructf

    _structf(vectorH)
        _node(logsize)
        _node(physsize)
        _node(data_vector)
        _node(displacement)
        _node(flags)
    _endstructf	
    
    _structf(arrayH)
        _node(rank)
        _node(physsize)
        _node(data_vector)
        _node(displacement)
        _node(flags)
        _struct_label(dim0)
    _endstructf
    
    // ARM64 PCS doesn't use thsi calling convention
    // _struct(c_frame,0)	/* PowerOpen ABI C stack frame */
    //     _node(backlink)
    //     _node(crsave)
    //     _node(savelr)
    //     _field(unused, 16)
    //     _node(savetoc)
    //     _struct_label(params)
    //         _node(param0)
    //         _node(param1)
    //         _node(param2)
    //         _node(param3)
    //         _node(param4)
    //         _node(param5)
    //         _node(param6)
    //         _node(param7)
    //     _struct_label(minsiz)
    // _ends

    // _struct(eabi_c_frame,0)
    //     _word(backlink) 
    //     _word(savelr)
    //     _word(param0)
    //     _word(param1)
    //     _word(param2)
    //     _word(param3)
    //     _word(param4)
    //     _word(param5)
    //     _word(param6)
    //     _word(param7)
    //     _struct_label(minsiz)
    // _ends

    // /* For entry to variable-argument-list functions */
    // /* (e.g., via callback) */
    // _struct(varargs_eabi_c_frame,0)
    //     _word(backlink)
    //     _word(savelr)
    //     _struct_label(va_list)
    //     _word(flags)   /* gpr count byte, fpr count byte, padding */
    //     _word(overflow_arg_area)
    //     _word(reg_save_area)
    //     _field(padding,4)
    //     _struct_label(regsave)
    //     _field(gp_save,8*4)
    //     _field(fp_save,8*8)
    //     _word(old_backlink)
    //     _word(old_savelr)
    //     _struct_label(incoming_stack_args)
    // _ends
            
    _struct(lisp_frame,0)
        _node(backlink)
        _node(savefn)
        _node(savelr)
        _node(savevsp)
    _ends

    _struct(vector,-fulltag_misc)
        _node(header)
        _struct_label(data)
    _ends

    _struct(binding,0)
        _node(link)
        _node(sym)
        _node(val)
    _ends


/* Nilreg-relative globals.  Talking the assembler into doing something reasonable here */
/* is surprisingly hard. */


nrs_origin = (0x3000+(LOWMEM_BIAS))
nrs_symbol_fulltag = fulltag_misc        
define(`nilsym',`nil')        
lisp_globals_limit = (0x3000+(LOWMEM_BIAS))
        
        include(lisp_globals.s)
        
    

define(`def_header',`
$1 = ($2<<num_subtag_bits)|$3')

def_header(double_float_header,2,subtag_double_float)
def_header(two_digit_bignum_header,2,subtag_bignum)
def_header(three_digit_bignum_header,3,subtag_bignum)
def_header(four_digit_bignum_header,4,subtag_bignum)
def_header(five_digit_bignum_header,5,subtag_bignum)        
def_header(symbol_header,symbol.element_count,subtag_symbol)
def_header(value_cell_header,1,subtag_value_cell	)
def_header(macptr_header,macptr.element_count,subtag_macptr)
def_header(vectorH_header,vectorH.element_count,subtag_vectorH)
def_header(complex_single_float_header,2,subtag_complex_single_float)
def_header(complex_double_float_header,6,subtag_complex_double_float)
    
include(errors.s)

/* Symbol bits that we care about */
sym_vbit_bound = (0+fixnum_shift)
sym_vbit_bound_mask = (1<<sym_vbit_bound)
sym_vbit_const = (1+fixnum_shift)
sym_vbit_const_mask = (1<<sym_vbit_const)

    _struct(area,0)
        _node(pred) 
        _node(succ) 
        _node(low) 
        _node(high) 
        _node(active) 
        _node(softlimit) 
        _node(hardlimit) 
        _node(code) 
        _node(markbits) 
        _node(ndwords) 
        _node(older) 
        _node(younger) 
        _node(h) 
        _node(sofprot) 
        _node(hardprot) 
        _node(owner) 
        _node(refbits) 
        _node(nextref) 
    _ends


/* This is only referenced by c->lisp code that needs to save/restore C NVRs in a TSP frame. */
    _struct(c_reg_save,0)
        _node(tsp_link)	/* backpointer */
        _node(tsp_mark)	/* frame type */
        _node(save_fpscr)	/* for Cs FPSCR */
        _field(save_gprs,19*node_size) /* r13-r31 */
        _dword(save_fp_zero)	/* for fp_zero */
        _dword(save_fps32conv)
        _field(save_fprs,13*8)
    _ends


TCR_BIAS = 0
    
/*  Thread context record. */

    _struct(tcr,-TCR_BIAS)
        _node(prev)		/* in doubly-linked list */
        _node(next)		/* in doubly-linked list */
        _node(single_float_convert) /* xxxf0 */
        _word(lisp_fpscr)	/* lisp thread's fpscr (in low word) */
        _word(lisp_fpscr_low)
        _node(db_link)		/* special binding chain head */
        _node(catch_top)	/* top catch frame */
        _node(save_vsp)	/* VSP when in foreign code */
        _node(save_tsp)	/* TSP when in foreign code */
        _node(cs_area)		/* cstack area pointer */
        _node(vs_area)		/* vstack area pointer */
        _node(ts_area)		/* tstack area pointer */
        _node(cs_limit)	/* cstack overflow limit */
        _word(bytes_consed_high)
        _word(bytes_consed_low)
        _node(log2_allocation_quantum)
        _node(interrupt_pending)
        _node(xframe)		/* per-thread exception frame list */
        _node(errno_loc)	/* per-thread  errno location */
        _node(ffi_exception)	/* fpscr exception bits from ff-call */
        _node(osid)		/* OS thread id */
        _node(valence)		/* odd when in foreign code */
        _node(foreign_exception_status)
        _node(native_thread_info)
        _node(native_thread_id)
        _node(last_allocptr)
        _node(save_allocptr)
        _node(save_allocbase)
        _node(reset_completion)
        _node(activate)
        _node(suspend_count)
        _node(suspend_context)
        _node(pending_exception_context)
        _node(suspend) /* semaphore for suspension notify */
        _node(resume)  /* sempahore for resumption notify */
        _word(flags_pad)
        _word(flags)      
        _node(gc_context)
        _node(termination_semaphore)
        _node(unwinding)
        _node(tlb_limit)
        _node(tlb_pointer)     /* Consider using tcr+N as tlb_pointer */
        _node(shutdown_count)
        _node(safe_ref_address)
        _node(nfp)
    _ends

TCR_FLAG_BIT_FOREIGN = fixnum_shift
TCR_FLAG_BIT_AWAITING_PRESET = (fixnum_shift+1)
TCR_FLAG_BIT_ALT_SUSPEND = (fixnumshift+2)
TCR_FLAG_BIT_PROPAGATE_EXCEPTION = (fixnumshift+3)
TCR_FLAG_BIT_SUSPEND_ACK_PENDING = (fixnumshift+4)
TCR_FLAG_BIT_PENDING_EXCEPTION = (fixnumshift+5)
TCR_FLAG_BIT_FOREIGN_EXCEPTION = (fixnumshift+6)
TCR_FLAG_BIT_PENDING_SUSPEND = (fixnumshift+7)        


nil_value = (0x3000+symbol.size+fulltag_misc+(LOWMEM_BIAS))
            
define(`RESERVATION_DISCHARGE',(0x2008+(LOWMEM_BIAS)))


INTERRUPT_LEVEL_BINDING_INDEX = fixnumone
