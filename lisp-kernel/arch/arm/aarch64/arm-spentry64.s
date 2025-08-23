
/*
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
	.p2align 3
        
_spentry(bind)
	__(ldr imm1,[arg_y,#symbol.binding_index])
	__(ldr imm0,[rcontext,#tcr.tlb_limit])
	__(cmp imm0,imm1)
        __(bhi 1f)
	__(uuo_tlb_too_small(c_al,imm1))
1:              
	__(cmp imm1,#0)
	__(ldr imm2,[rcontext,#tcr.tlb_pointer])
	__(ldr imm0,[rcontext,#tcr.db_link])
	__(ldr temp1,[imm2,imm1])
	__(beq 9f)
	__(vpush1(temp1))
	__(vpush1(imm1))
	__(vpush1(imm0))
	__(str arg_z,[imm2,imm1])
	__(str vsp,[rcontext,#tcr.db_link])
	__(bx lr)
9:
	__(mov arg_z,arg_y)
	__(mov arg_y,#XSYMNOBIND)
	__(set_nargs(2))
	__(b _SPksignalerr)
        
        _endfile
