#ifndef _TRAP_WRAPPER_H
#define _TRAP_WRAPPER_H

#ifndef ASM
// assembly wrapper for exception handler, see @trap.h @trap.c for details
extern void divide_error_handler();
extern void debug_handler();
extern void nmi_intr_handler();
extern void int3_handler();
extern void overflow_handler();
extern void bound_range_handler();
extern void invalid_opcode_handler();
extern void device_not_aval_handler();
extern void double_fault_handler();
extern void coprocessor_seg_overrun_handler();
extern void invalid_tss_handler();
extern void segment_not_present_handler();
extern void stack_fault_handler();
extern void general_protection_handler();
extern void page_fault_handler();
extern void floating_point_error_handler();
extern void alignment_check_handler();
extern void machine_check_handler();
extern void simd_floating_point_handler();
#endif
#endif
