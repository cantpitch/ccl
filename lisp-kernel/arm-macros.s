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

define(`make_header',`(($1<<num_subtag_bits)|($2&subtag_mask))')

define(`test_fixnum',`
    __(tst $1,#fixnummask)
')

define(`test_two_fixnums',`
    __(orr $3,$1,$2)
    __(test_fixnum($3))
')

define(`extract_fulltag',`
    __(and $1,$2,#fulltagmask)
')

define(`extract_lisptag',`
    __(and $1,$2,#tagmask)
')

define(`extract_lisptag_',`
    __(ands $1,$1,#tagmask)
')

define(`extract_subtag',`
    __(ldrb $1,[$2,#misc_subtag_offset])
')

define(`extract_lowbyte',`
    __(and $1,$2,#((1<<num_subtag_bits)-1))
')

define(`extract_header',`
    __(ldr $1,[$2,#misc_header_offset])
')

define(`box_fixnum',`
    __(mov $1,$2, lsl #fixnumshift)
')

define(`unbox_fixnum',`	
    __(mov $1,$2, asr #fixnumshift)
')

define(`unbox_character',`
    __(mov $1,$2, lsr #charcode_shift)
')

define(`set_nargs',`
    __(mov nargs,#($1)<<fixnumshift)
')

define(`getvheader',`
    __(ldr $1,[$2,#vector.header])
')

/* "Length" is fixnum element count */
define(`header_length',`
    __(bic $1,$2,#subtag_mask)
    __(mov $1,$1,lsr #num_subtag_bits-fixnumshift)
')

define(`vector_length',`
    __(getvheader($3,$2))
    __(header_length($1,$3))
')

define(`ref_global',`
    __(mov ifelse($3,`',$1,$3),#nil_value)
    __(ldr $1,[ifelse($3,`',$1,$3),#lisp_globals.$2])
')

define(`ref_nrs_value',`
    __(mov $1,#nil_value)
    __(ldr $1,[$1,#((nrs.$2)+(symbol.vcell))])
')

define(`ref_nrs_function',`
    __(mov $1,#nil_value)
    __(ldr $1,[$1,#((nrs.$2)+(symbol.fcell))])
')

define(`_car',`
    __(ldr $1,[$2,#cons.car])
')

define(`_cdr',`
    __(ldr $1,[$2,#cons.cdr])
')

define(`_rplaca',`
    __(str $2,[$1,#cons.car])
')

define(`_rplacd',`
    __(str $2,[$1,#cons.cdr])
')

ifdef(`ARM64',`
    include(arm-constants64.s)
',`
    include(arm-constants32.s)
')