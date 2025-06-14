// PPC64: r0 = rzero (zero register), ARM64: rzero
define(`rzero',`x0')  
// PPC64: r1 = sp (stack pointer), ARM64: temp2 can't be at x18, so stick it here for now
define(`temp2',`x1')
// PPC64: r2 = current thread's TCR
define(`tcr',`x2')
// PPC64: r3-r8 imm0-imm5
define(`imm0',`x3')
define(`imm1',`x4')
define(`imm2',`x5') 
define(`imm3',`x6')
define(`imm4',`x7')
define(`imm5',`x8')

// PPC64: r9, r10 (symbolic names allocptr and allocbase) are used to do per-thread memory allocation
define(`allocptr',`x9')
define(`allocbase',`x10')
// PPC64: r11 (symbolic name nargs) contains the number of function arguments on entry and the number of return values 
//        in multiple-value returning constructs. 
define(`nargs',`x11')
// PPC64: r12 (symbolic name tsp) holds the top of the current thread's temp stack.
define(`tsp',`x12')
// PPC64: r13 - not used
define(`',`x13')    
// PPC64: r14 (symbolic name loc-pc) is used to copy "pc-locative" values between main memory and special-purpose 
//        PPC registers (LR and CTR) used in function-call and return instructions.            
define(`loc_pc',`x14')            

// PPC64: r15-r31 are always treated as node registers, ARM64: x18, x29, & x30 are reserved

// PPC64: r15 (symbolic name vsp) addresses the top of the current thread's value stack.    
define(`vsp',`x15')  

// PPC64: r16 (symbolic name fn) 
define(`fn',`x16')
// PPC64: r17 (symbolic name temp3) 
define(`temp3',`x17')

// PPC64: r18 (symbolic name temp2), ARM64: x18 reserved by Apple on Darwin
define(`resd',`x18')

// PPC64: r19-20 (symbolic name temp1-0) 
define(`temp1',`x19')
define(`temp0',`x20')

// PPC64: r21-23 (symbolic names arg_x, arg_y, arg_z) 
define(`arg_x',`x21')
define(`arg_y',`x22')
define(`arg_z',`x23')
        
// PPC64: r24-31 (symbolic names save7-0) 
define(`save7',`x24')
define(`save6',`x25')
define(`save5',`x26')
define(`save4',`x27')                
define(`save3',`x28')

// ARM64: x29 = frame pointer, used for stack unwinding
define(`fp',`x29') 
// ARM64: x30 = link register, used for return address
define(`lr',`x30')


charcode_shift = 8
dnode_align_bits = 4
dnode_shift = dnode_align_bits 
fixnumshift = 3
fixnum_shift = fixnumshift
fulltagmask = 15
nbits_in_word = 64
ntagbits = 4
num_subtag_bits = 8