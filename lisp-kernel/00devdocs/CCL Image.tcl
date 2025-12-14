#
# Hex Fiend Template for CCL Image File
# 
# Place in:
# '/Users/<username>/Library/Application Support/com.ridiculousfish.HexFiend/Templates'
#

set section_names(0) ""
set section_sizes(0) 0

proc SectionHeader {index} {
    global section_names
    global section_sizes

    set code_pos [pos]
    set code [expr [uint64] >> 3]
    
    switch $code {
        0 {set name "AREA_VOID"}
        1 {set name "AREA_CSTACK"}
        2 {set name "AREA_VSTACK"}
        3 {set name "AREA_TSTACK"}
        4 {set name "AREA_READONLY"}
        5 {set name "AREA_WATCHED"}
        6 {set name "AREA_STATIC_CONS"}
        7 {set name "AREA_MANAGED_STATIC"}
        8 {set name "AREA_STATIC"}
        9 {set name "AREA_DYNAMIC"}
        default {set name "Unknown"}
    }

    set section_names($index) $name

    section -collapsed "$name Header" {
        entry "Code" $code 8 $code_pos
        uint64 -hex "Area"
        set mem_size [uint64]
        set section_sizes($index) $mem_size
        entry "Memory Size" $mem_size 8 [expr $code_pos + 16]
        uint64 "Static dnodes"
    }
}

proc Section {name size page_size} {
    
    set page_mask [expr $page_size - 1]
    set round_size [expr {$size + $page_mask} & ~$page_mask]
    section "$name Data" {
        if {$size > 0} {
            bytes $round_size "Data"
        }
    }
}

proc Global {name} {
    uint64 -hex $name
}
proc Symbol {name} {
    section "$name" {
        uint64 -hex "header"
        uint64 -hex "pname"
        uint64 -hex "vcell"
        uint64 -hex "fcell"
        uint64 -hex "package predicate"
        uint64 -hex "flags"
        uint64 -hex "plist"
        uint64 -hex "binding index"
    }
}

proc StaticSection {name size page_size} {
    
    set page_mask [expr $page_size - 1]
    set round_size [expr {$size + $page_mask} & ~$page_mask]
    section "$name Data" {
        bytes 0xE58 "Data"

        section -collapsed "Globals" {
            Global "MANAGED_STATIC_REFIDX"
            Global "EPHEMERAL_REFIDX"
            Global "MANAGED_STATIC_DNODES"
            Global "MANAGED_STATIC_REFBITS"
            Global "WEAKVLL"
            Global "INITIAL_TCR"
            Global "IMAGE_NAME"
            Global "WEAK_GC_METHOD"
            Global "UNWIND_RESUME"
            Global "BATCH_FLAG"
            Global "HOST_PLATFORM"
            Global "ARGV"
            Global "REF_BASE"
            Global "TENURED_AREA"
            Global "OLDEST_EPHEMERAL"
            Global "LISP_EXIT_HOOK"
            Global "STATIC_CONS_AREA"
            Global "DOUBLE_FLOAT_ONE"
            Global "SHORT_FLOAT_ZERO"
            Global "OBJC_2_END_CATCH"
            Global "FREE_STATIC_CONSES"
            Global "IN_GC"
            Global "LEXPR_RETURN1V"
            Global "LEXPR_RETURN"
            Global "ALL_AREAS"
            Global "KERNEL_PATH"
            Global "OBJC_2_BEGIN_CATCH"
            Global "STACK_SIZE"
            Global "STATICALLY_LINKED"
            Global "HEAP_END"
            Global "HEAP_START"
            Global "GCABLE_POINTERS"
            Global "GC_NUM"
            Global "FWDNUM"
            Global "FLOAT_ABI"
            Global "OLDSPACE_DNODE_COUNT"
            Global "REFBITS"
            Global "GC_INHIBIT_COUNT"
            Global "INTFLAG"
            Global "DEFAULT_ALLOCATION_QUANTUM"
            Global "STATIC_CONSES"
            Global "EXCEPTION_LOCK"
            Global "TCR_AREA_LOCK"
            Global "TCR_KEY"
            Global "RET1VALN"
            Global "SUBPRIMS_BASE"
            Global "SAVER13"
            Global "SAVETOC"
            Global "OBJC_2_PERSONALITY"
            Global "KERNEL_IMPORTS"
            Global "INTERRUPT_SIGNAL"
            Global "TCR_COUNT"
            Global "GET_TCR"
        }

        bytes 0x20 "nil"

        section "Nil-Relative Symbols" {
            Symbol "t"
            Symbol "nil"
            Symbol "%err-disp"
            Symbol "cmain"
            Symbol "eval"
            Symbol "apply-evaluated-function"
            Symbol "error"
            Symbol "%defun"
            Symbol "%defvar"
            Symbol "%defconstant"
            Symbol "%macro"
            Symbol "%kernel-restart"
            Symbol "*package*"
            Symbol "*total-bytes-freed*"
            Symbol ":allow-other-keys"
            Symbol "%toplevel-catch%"
            Symbol "%toplevel-function%"
            Symbol "%pascal-functions%"
            Symbol "restore-lisp-pointers"
            Symbol "*total-gc-microseconds*"
            Symbol "%builtin-functions%"
            Symbol "%unbound-function%"
            Symbol "%init-misc%"
            Symbol "%macro-code%"
            Symbol "%closure-code%"
            Symbol "%new-gcable-ptr"
            Symbol "*gc-event-status-bits*"
            Symbol "*post-gc-hook*"
            Symbol "%handlers%"
            Symbol "%all-packages%"
            Symbol "*keyword-package*"
            Symbol "%os-init-function%"
            Symbol "%foreign-thread-control"
        }
        if {$size > 0} {
            bytes [expr $round_size - (0x1020 + (33 * 64))] "Data"
        }
    }
}

section "Footer" {
    set magic_loc [expr [len] - 16]
    goto $magic_loc

    requires $magic_loc "6E 65 70 4F 49 4C 43 4D 65 67 61 6D"

    set sig [ascii 12]
    entry "Signature" $sig 12 $magic_loc
    set delta [int32]
    entry "Delta" $delta 4 [expr $magic_loc + 12]
}

section "Header" {
    move $delta

    ascii 16 "Signature"
    unixtime32 "Timestamp"
    uint32 -hex "Canonical Image Base 32"
    uint32 -hex "Actual Image Base 32"
    set sections [uint32]
    entry "Sections" $sections 4 [expr [pos] - 4]
    uint32 "ABI Version"
    set section_data_offset [int32]
    entry "Section Data Offset High" $section_data_offset 4 [expr [pos] - 4]
    set section_data_offset_low [uint32]
    entry "Section Data Offset Low" $section_data_offset_low 4 [expr [pos] - 4]
    set section_data_offset [expr {$section_data_offset << 32} | $section_data_offset_low]
    entry "(Section Data Offset)" $section_data_offset 8 [expr [pos] - 8]
    
    set curr_pos [pos]
    set flags [uint32]
    section Flags {
        switch [expr $flags & 0x7] {
            0 {set os "VxWorks"}
            1 {set os "Linux"}
            2 {set os "Solaris"}
            3 {set os "Darwin"}
            4 {set os "FreeBSD"}
            5 {set os "Windows"}
            6 {set os "Android"}
            default {set os "Unknown" 4 $curr_pos}
        }
        entry "OS" $os 4 $curr_pos
        
        set cpuno [expr {$flags >> 3} & 0x3]
        switch [expr {$flags >> 3} & 0x3] {
            0 {set cpu "PowerPC"}
            1 {set cpu "SPARC"}
            2 {set cpu "x86"}
            3 {set cpu "ARM"}
            default {set cpu "Unknown"}
        }
        entry "CPU" $cpu 4 $curr_pos

        set word_size [expr {$flags >> 6} & 0x1]
        switch $word_size {
            0 {set ws "32-bit"}
            1 {set ws "64-bit"}
            default {set ws "Unknown"}
        }
        entry "Word Size" $ws 4 $curr_pos
    }
    if {$word_size == 1} {
        uint64 -hex "Canonical Image Base 64"
        uint64 -hex "Actual Image Base 64"
    }
}

for {set i 0} {$i < $sections} {incr i} {
    SectionHeader $i
}

move $section_data_offset

if {($cpu == "ARM") && ($ws == "64-bit") && ($os == "Darwin")} {
    set page_size 0x4000
} else {
    set page_size 0x1000
}

if [catch {
    for {set i 0} {$i < $sections} {incr i} {
        if {$section_names($i) == "AREA_STATIC"} {
            StaticSection $section_names($i) $section_sizes($i) $page_size
        } else {
            Section $section_names($i) $section_sizes($i) $page_size
        }
    }
}] {
    puts $errorInfo
}