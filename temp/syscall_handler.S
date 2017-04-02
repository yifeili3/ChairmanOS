#define ASM 1
#include "syscall_handler.h"
.text

.global syscall_handler
# Function Description: assembly wraper for systemcall handler. 
# 						save all register and flags and restore afterwards.
# INPUT: none
# OUTPUT: none
# SIDE EFFECT:
systemcall_handler:
	# save all register
	pushl %eax
	SAVE_ALL
	# validity check:  valid syscall # 1-10
	cmpl $1, %eax
	jl invalid
	cmpl $10, %eax
	jg invalid
nobadsys:
	# invoke specific service routine
	call *sys_call_table(0, %eax, 4)
	# stores return value in stack location
	movl %eax, 24(%esp)
	RESTORE_ALL
	addl $4, %esp
	iret
invalid:	
	RESTORE_ALL
	addl $4, %esp
	xorl %eax, %eax
	movl $-1, %eax
	iret

sys_call_table:
	.long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmaps #, sys_set_handler, sys_sigreturn
	# only 8 syscall now, last two for extra credit
