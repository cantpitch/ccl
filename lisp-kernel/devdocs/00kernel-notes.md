LISP-KERNEL NOTES
=================

This is a collection notes I put together while researching how the CCL kernel works. 
    - Hans Van Slooten, 2025

# Assembly Macros

## M4 Notes
* `` `' `` quoting with backtick and single quote prevent M4 from replacing within the quoted string for one loop of processing. CCL uses this feature generously to ensure that there isn't odd behavior in macro expansion.

* `ifelse` If statements are a bit hard to read. Here is a primer:
  * ``ifelse(`something')`` - Idiom for creating block comments in M4
  * ``ifelse(`foo',`bar',`true')`` - Outputs nothing because `foo != foo`
  * ``ifelse(`foo',`foo',`true')`` - Outputs `true` because `foo == foo`
  * ``ifelse(`foo',`bar',`true',`false')`` - Outputs `false` because `foo != bar`
  * ``ifelse(`foo',`foo',`true',`false')`` - Outputs `true` because `foo != bar`
  * If 6+ arguments, acts (kinda) like a `switch` statement:\
  ``ifelse(`foo',`foo',`third',`foo',`bar',`sixth',`seventh')`` -> `third`\
  ``ifelse(`foo',`bar',`third',`foo',`foo',`sixth',`seventh')`` -> `sixth`\
  ``ifelse(`foo',`bar',`third',`foo',`bar',`sixth',`seventh')`` -> `seventh`


## Structs and Memory Layout

There are a number of macros defined in `m4macros.m4`. The `_struct*` macros are used extensively in `lisp_globals.s` and `*-constants*.s` to define the layout of objects in memory for that platform. For instance, in `lisp_globals.s` the beginning of the NRS is `t` and `nil` which is defined as:
```lisp_globals.s
    nrs_symbol_extra = symbol.size-nrs_symbol_fulltag

    _struct(nrs,nrs_origin)

    _struct_pad(nrs_symbol_fulltag)
    _struct_label(tsym)
    _struct_pad(nrs_symbol_extra)    /* t */

    _struct_pad(nrs_symbol_fulltag)
    _struct_label(nilsym)
    _struct_pad(nrs_symbol_extra)    /* nil */
    ...
```
Note a couple things here:
* The NRS are treated as a single struct.
* That struct is located in memory at `nrs_origin` (which needs to be defined in `*-constants*.s` _before_ including `lisp_globals.s`)
* These aren't structs in the _C language sense_, but simply equates in assembly to point to locations in memory.
* The section above will be expanded by M4 into:
```lisp_globals.s
    nrs_symbol_extra = symbol.size-nrs_symbol_fulltag

    .set _nrs_org,nrs_origin
    .set _nrs_base,_nrs_org
  
    .set _nrs_org,_nrs_org + nrs_symbol_fulltag
    .set nrs.tsym, _nrs_org
    .set _nrs_org,_nrs_org + nrs_symbol_extra
    /* t */

    .set _nrs_org,_nrs_org + nrs_symbol_fulltag
    .set nrs.nil, _nrs_org
    .set _nrs_org,_nrs_org + nrs_symbol_extra
    /* nil */

    ...
```
`_nrs_org` is set to `nrs_origin` and then is progressively incremented by the amount specified by `_struct_pad` (in order to make room for the object). `symbol.size` is from the `symbol` struct defined in `*-constants*.s` for the platform, in this case PPC64:
```ppc-constants64.s
    _structf(symbol)
        _node(pname)
        _node(vcell)
        _node(fcell)
        _node(package_predicate)
        _node(flags)
        _node(plist)
        _node(binding_index)
    _endstructf
```
which expands to:
```ppc-constants64.s
	.set _symbol_org,-misc_bias
	.set _symbol_base,_symbol_org
 
	.set symbol.header, _symbol_org
	.set _symbol_org,_symbol_org + node_size

	.set symbol.pname, _symbol_org 
	.set _symbol_org,_symbol_org + node_size

	.set symbol.vcell, _symbol_org
	.set _symbol_org,_symbol_org + node_size

	.set symbol.fcell, _symbol_org 
	.set _symbol_org,_symbol_org + node_size

	.set symbol.package_predicate, _symbol_org 
	.set _symbol_org,_symbol_org + node_size

	.set symbol.flags, _symbol_org 
	.set _symbol_org,_symbol_org + node_size

	.set symbol.plist, _symbol_org 
	.set _symbol_org,_symbol_org + node_size

	.set symbol.binding_index, _symbol_org 
	.set _symbol_org,_symbol_org + node_size

	.set symbol.element_count,((_symbol_org-node_size)-_symbol_base)/node_size	
	.set symbol.size, _symbol_org-_symbol_base
```
The important thing to note for Lisp Globals is that `symbol.size` for `nrs_symbol_extra` is defined here. So `nrs_symbol_size` is the `symbol.size`:
```
    .set symbol.size, _symbol_org - _symbol_base
=>  .set symbol.size, (-misc_bias + (8 * node_size)) - _symbol_org
=>  .set symbol.size, (-fulltag_misc + (8 * 8)) - (-misc_bias)
=>  .set symbol.size, (-12L + (8 * 8)) - (-12L)
=>  .set symbol.size, -12L + 64 + 12L
=>  .set symbol.size, 64
```
The `-misc_bias` is there to mask off the tag bits on the symbol and ultimately cancels out in this case to give a symbol size of 64 bytes on PPC64. So,
```
    nrs_symbol_extra = symbol.size - nrs_symbol_fulltag
=>  nrs_symbol_extra = 64 - fulltag_misc
=>  nrs_symbol_extra = 64 - 12
=>  nrs_symbol_extra = 52
```
Going back to the original file:
```lisp_globals.s
    nrs_symbol_extra = 52

    .set _nrs_org,0x3000
    .set _nrs_base,0x3000
  
    .set _nrs_org,0x3000 + 12
    .set nrs.tsym,0x300C
    .set _nrs_org,0x300C + 52
    /* t */

    .set _nrs_org,0x3040 + 12
    .set nrs.nil,0x304C
    .set _nrs_org,0x304C + 52
    /* nil */

    .set _nrs_org,0x3080 + 12
```
From this we can see that the NRS pointer to a symbol _includes_ the tag (e.g. 12/0xC). How is this structure used in code? Look in `ppc-macros.s` at:
```ppc-macros.s
define(`ref_nrs_value',`
	__(ldr($1,((nrs.$2)+(symbol.vcell))(0)))
')
```
This macro loads the NRS symbol `$2` into register `$1`. The calculation for the memory location for `nrs.nil` looks like:
```ppc-macros.s
    ld r14, ((nrs.nil) + (symbol.vcell))(0)
=>  ld r14, ((0x304C) + (-misc_bias + node_size + node_size + node_size))(0)
=>  ld r14, ((0x304C) + (-fulltag_misc + 8 + 8 + 8))(0)
=>  ld r14, ((0x304C) + (-12 + 8 + 8 + 8))(0)
=>  ld r14, ((0x304C) + (-12 + 24))(0)
=>  ld r14, (0x3040 + 24)(0) ; fulltag_misc masked off by bias of struct in memory
=>  ld r14, (0x3058)(0)      ; nrs.nil.vcell
```
Now we see that the `-misc_bias` for the origin definition of the symbol structure was designed to mask off the tag bits of the Lisp Global pointer. Elegant, but also can be a bit confusing when seen in the code.

As a side note, it should be noted that symbols have a `vcell` and `fcell` value. This is how symbols can be bound to both a value _and_ a function. There's also `plist` to store the property list for the symbol.

The rest of `m4macros.s` is boilerplate generation for functions based on the platform as well as some tweaks for the M4 macro processor working with the assembler.

## NRS - "NIL-Relative Symbol" & Lisp Globals
* Location in memory defined in `lisp_globals.h` (along with Lisp Globals)
* It's an array of `lispsymbol *` pointers starting with NIL (hence the name).
* Lisp Globals is an array of `LispObject *` pointers, often starting at the same location as NRS objects but they are referenced in negative numbers from this origin. So in `lisp_globals.h` you'll see something like:\
`#define IN_GC (-32)             /* non-zero when lisp addresses may be invalid */`\
which means `IN_GC` is defined as `globals origin - 32 nodes`.
* Architecture specifics:
  * PPC32:\
  `lisp_global` at `nil_value - fulltag_nil` == `0x3015 - 5` == `0x3010` (Masking off the tag of `nil_value` to get the actual pointer)\
  `nrs_symbol` at `nil_value + (8 - fulltag_nil) + 8` == `0x3015 + (8 - 5) + 8` == `0x3020`
  * PPC64: `lisp_global` and `nrs_symbol` both start at `0x3000`
  * x86-32: `lisp_global` at `0x13000`, `nrs_symbol` at `0x13008`
  * x86-64: `lisp_global` at `0x13000`, `nrs_symbol` at `0x13020`
  * ARM32:\
  `lisp_global` at `nil_value - fulltag_nil - dnode_size` == `(0x4000000 + 1) - 1 - 8` == `0x3FFFFF8` (mask off tag then back up one dnode)\
  `nrs_symbol` at `nil_value - fulltag_nil - dnode_size` == `(0x4000000 + 1) - 1 + 8` == `0x4000008` (mask off tag then move up one dnode)
  * ARM64: _Working on now_


# Register Allocation

PPC and ARM64 have pretty different register allocations for their ABIs, but since the code uses `define`s for register names, this can be worked around pretty easily. For reference, here's the PPC64 ABI:

## PPC64 ABI

### Integer Registers
| PPC64 | USAGE | SAVE | RESV'D | ARM64 | USAGE | SAVE | RESV'D |
|---|---|:---:|:---:|---|---|:---:|:---:|
| `r0` | Used in prolog/epilog | N | N | `x0` | 1st parameter/return value | N | N |
| `r1` | Stack pointer | Y | Y |  `x1-x7` | 2nd-8th parameters | N | N |
| `r2` | TOC pointer | Y | Y |`x8` | Indirect result register | N | N |
| `r3` | 1st parameter/return value | N | N | `x9-15` | Scratch registers | N | N |
| `r4-r10` | 2nd-8th parameters | N | N | `x16-17` | Intraprocedure call or scratch | N | N |
| `r11` | Environment pointer | N | N |  `x18` | Platform Register | N/A | Y |
| `r12` | Used by global linkage | N | N | `x19-x28` | Callee scratch registers | Y | N |
| `r13` | System thread ID  | N/A | N | `x29` | Frame pointer | N | Y |
| `r14-r31` | Global integer registers | Y | N | `x30` | Link Register | N/A | N |
| | | | | `sp` | Stack Pointer | N/A | Y |

### Floating Point Registers
| PPC64 | USAGE | CALLEE SAVE | ARM64 | USAGE | CALLEE SAVE |
|---|---|---|---|---|---|
| `f0` | Scratch register | N | `v0-v7` | 1st-8th floating point parameter | N |
| `f1-f13` | 1st-13th floating point parameter | N | `v8-v15` | Non-volatile FP registers | Y (`d8-d15` only) |
| `f14-f31` | Global FP registers | Y | `v16-v31` | Volatile FP registers | N |

### Special Registers
| REGISTER | USAGE | CALLEE SAVE |
|---|---|---|
|   |   |   |
|   |   |   |
|   |   |   |


### Vector Registers
| REGISTER | USAGE | CALLEE SAVE |
|---|---|---|
|   |   |   |
|   |   |   |
|   |   |   |

# `stack_alloc_marker`
* PPC64 doesn't use a `stack_alloc_marker` because it has the PowerOpen ABI stack frame.
* ARM32 and the aborted ARM64 use a `stack_alloc_marker` for displaying and walking the stack.

## ARM32 
In `arm-constants.h` it's defined as `#define stack_alloc_marker SUBTAG(fulltag_imm,1)` which is `3 | (1 << 3)` => `0xB`.

## Previous ARM64
In `arm64-constants.s.prev` it was going to be defined as:
```arm64-constants.s.prev
tag_stack_alloc = (tag_imm | 6) ; This was supposed to be `imm_tag_mask`
stack_alloc_marker = (tag_stack_alloc << tag_shift)   
```
which (after corrections) would be:
```
tag_stack_alloc = (0x10 | 6)
stack_alloc_marker = (0x10 | 6) << tag_shift ; This was going to use ARM64's high tag ability, so the shift is immaterial.
```
The tag was going to be `0x16`.

The question becomes, what should it be on ARM64 now?

For now, in `arm64-constants.h` it will be defined (along with `lisp_frame_marker` another potential necessity on ARM64) as:
```
#define subtag_stack_alloc SUBTAG(fulltag_imm_3,4)
#define stack_alloc_marker subtag_stack_alloc
#define subtag_lisp_frame SUBTAG(fulltag_imm_3,5)
#define lisp_frame_marker subtag_lisp_frame
```
and in `arm64-constants.s` as:
```
define_subtag(stack_alloc,fulltag_imm_3,4)
stack_alloc_marker = subtag_stack_alloc
define_subtag(lisp_frame,fulltag_imm_3,5)
lisp_frame_marker = subtag_lisp_frame
```

# Garbage Collector

## Mark Phase
* Each doublenode in the heap has a bit in the markbits vector:\
`(<addr of dnode> - <heap start addr>)/16`


# Glossary
* **acode** - An intermediate representation of Lisp code produced by the compiler front-end. Short for “alphatized code,” meaning “has undergone alpha reduction,” which in turn means “all lambda-bound variables have been consistently renamed.”
* **command stack (cstack)** -
* **gvector** - A uvector whose elements are nodes.
* **immediate** - A value in memory or a register that is a raw, untagged value.
* **ivector** - A uvector whose elements are immediates.
* **LAP** - An old acronym for Lisp Assembly Program. A notation for writing assembly language in Lisp-like syntax. Not surprisingly, the exact LAP syntax varies, depending on processor architecture.
* **nfn** - new function
* **node** & **dnode**
  * A value in memory or a register that is a tagged lisp datum. Size of the standard integer on that platform (i.e. 4 bytes on 32-bit platforms, 8 bytes on 64-bit platforms). `LispObject` in code.
  * A "dnode" is a "double node". Heap memory is allocated in dnodes so these are often seen in GC code.
* **nrs** - nil-relative symbol
* **nvr** - A non-volatile register (i.e. *callee-saved*, as opposed to caller-saved)
* **punt, puntable** - If a variable is bound to a simple expression and is never setq’d, then all references to the variable can be replaced by references to the simple expression in question. This process is called “punting.”
* **tagged return address (tra)** - On x86, the call instruction unconditionally pushes a return address onto the stack. The compiler aligns the call instruction (via insertion of nop instructions) such that the pushed return address will have a special tag that the GC can recognize.
* **TCR** - Thread context record. All of the state of a thread.
* **TSP** - Temp-consing stack pointer
* **uuo** - An “unimplemented user operation.” Sometimes called a trap (after the PowerPC instructions). An illegal instruction used as a way for lisp code to request service from the lisp kernel (such as allocate more memory, initiate a GC, and so forth).
* **uvector** - A memory-allocated object with a header word that describes the object’s type and the number of elements it contains. (The name comes from Spice Lisp.)
* **value stack (vstack)** - A stack that contains tagged lisp objects (nodes) such as function arguments, local variables, and other stack-allocated lisp objects. The contents of the value stack between its bottom and top are always unambiguously nodes.
* **vinsn** - Virtual Instruction. A pre-assembled fragment of code used as a code generation template by the compiler backend. **Vinsns** are written in a LAP-like notation, but a key difference is that **vinsns** are expanded/assembled as much as possible at **vinsn** definition time. When a **vinsn** is emitted at compile time, the operands in the **vinsn** template are then filled in.
