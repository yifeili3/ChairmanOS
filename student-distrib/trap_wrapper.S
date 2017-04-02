#define ASM 1
#include "trap_wrapper.h"
.text
.global divide_error_handler,debug_handler,nmi_intr_handler,int3_handler,overflow_handler,	\
bound_range_handler,invalid_opcode_handler,device_not_aval_handler,double_fault_handler,	\
coprocessor_seg_overrun_handler,invalid_tss_handler,segment_not_present_handler,stack_fault_handler,	\
general_protection_handler,page_fault_handler,floating_point_error_handler,	\
alignment_check_handler,machine_check_handler,simd_floating_point_handler

# Function Description: assembly wraper for exception handler. see @trap.c for details of do_exception
# 						save all register and flags and restore afterwards.
# INPUT: none
# OUTPUT: none
#
divide_error_handler:
	cli
	pushal   
	pushfl
	call do_divide_error
	popfl
	popal
	sti

	iret

debug_handler:
	cli
	pushal   
	pushfl
	call do_debug
	popfl
	popal
	sti

	iret
nmi_intr_handler:
	cli
	pushal   
	pushfl
	call do_nmi_intr
	popfl
	popal
	sti

	iret
int3_handler:
	cli
	pushal   
	pushfl
	call do_int3
	popfl
	popal
	sti

	iret
overflow_handler:
	cli
	pushal   
	pushfl
	call do_overflow
	popfl
	popal
	sti

	iret

bound_range_handler:
	cli
	pushal   
	pushfl
	call do_bound_range
	popfl
	popal
	sti

	iret
invalid_opcode_handler:
	cli
	pushal   
	pushfl
	call do_invalid_opcode
	popfl
	popal
	sti

	iret
device_not_aval_handler:
	cli
	pushal   
	pushfl
	call do_device_not_aval
	popfl
	popal
	sti

	iret
double_fault_handler:
	cli
	pushal   
	pushfl
	call do_double_fault
	popfl
	popal
	sti

	iret

coprocessor_seg_overrun_handler:
	cli
	pushal   
	pushfl
	call do_coprocessor_seg_overrun
	popfl
	popal
	sti

	iret
invalid_tss_handler:
	cli
	pushal   
	pushfl
	call do_invalid_tss
	popfl
	popal
	sti

	iret
segment_not_present_handler:
	cli
	pushal   
	pushfl
	call do_segment_not_present
	popfl
	popal
	sti

	iret
stack_fault_handler:
	cli
	pushal   
	pushfl
	call do_stack_fault
	popfl
	popal
	sti

	iret

general_protection_handler:
	cli
	pushal   
	pushfl
	call do_general_protection
	popfl
	popal
	sti

	iret
page_fault_handler:
	cli
	pushal   
	pushfl
	call do_page_fault
	popfl
	popal
	sti

	iret
floating_point_error_handler:
	cli
	pushal   
	pushfl
	call do_floating_point_error
	popfl
	popal
	sti

	iret

alignment_check_handler:
	cli
	pushal   
	pushfl
	call do_alignment_check
	popfl
	popal
	sti

	iret
machine_check_handler:
	cli
	pushal   
	pushfl
	call do_machine_check
	popfl
	popal
	sti

	iret
simd_floating_point_handler:
	cli
	pushal   
	pushfl
	call do_simd_floating_point
	popfl
	popal
	sti

	iret
