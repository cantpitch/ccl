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
	include(m4macros.m4)        
        ifdef(`LOWMEM_BIAS',`
`LOWMEM_BIAS' = LOWMEM_BIAS
',`
`LOWMEM_BIAS' = 0
')
        undefine(`LOWMEM_BIAS')
        /* DWARF2 exception fsm */
        DW_CFA_advance_loc = 0x40   
        DW_CFA_offset = 0x80
        DW_CFA_restore = 0xc0
        DW_CFA_nop = 0x00
        DW_CFA_set_loc = 0x01
        DW_CFA_advance_loc1 = 0x02
        DW_CFA_advance_loc2 = 0x03
        DW_CFA_advance_loc4 = 0x04
        DW_CFA_offset_extended = 0x05
        DW_CFA_restore_extended = 0x06
        DW_CFA_undefined = 0x07
        DW_CFA_same_value = 0x08
        DW_CFA_register = 0x09
        DW_CFA_remember_state = 0x0a
        DW_CFA_restore_state = 0x0b
        DW_CFA_def_cfa = 0x0c
        DW_CFA_def_cfa_register = 0x0d
        DW_CFA_def_cfa_offset = 0x0e
        /* DWARF 3.  */
        DW_CFA_def_cfa_expression = 0x0f
        DW_CFA_expression = 0x10
        DW_CFA_offset_extended_sf = 0x11
        DW_CFA_def_cfa_sf = 0x12
        DW_CFA_def_cfa_offset_sf = 0x13
        DW_CFA_val_offset = 0x14
        DW_CFA_val_offset_sf = 0x15
        DW_CFA_val_expression = 0x16
        /* SGI/MIPS specific.  */
        DW_CFA_MIPS_advance_loc8 = 0x1d
        /* GNU extensions.  */
        DW_CFA_GNU_window_save = 0x2d
        DW_CFA_GNU_args_size = 0x2e
        DW_CFA_GNU_negative_offset_extended = 0x2f

        ifdef(`PPC',`
         include(ppc-constants.s)
         include(ppc-macros.s)
	 include(ppc-uuo.s)
        ')
	ifdef(`X86',`
         include(x86-constants.s)
         include(x86-macros.s)
	 include(x86-uuo.s)
	')
        ifdef(`ARM',`
         include(arm-constants32.s)
         include(arm-macros.s)
         include(arm-macros32.s)
         include(arm-uuo32.s)
        ')
        ifdef(`ARM64',`
         include(arm-constants64.s)
         include(arm-macros.s)
         include(arm-macros64.s)
         include(arm-uuo64.s)
        ')

