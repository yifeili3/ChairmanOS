.global syscall_handler
# Function Description: assembly wraper for systemcall handler. 
# 						save all register and flags and restore afterwards.
# INPUT: none
# OUTPUT: none
# SIDE EFFECT:
syscall_handler:
	# save all register
	pushl %eax
	#SAVE_ALL
	push %es	
	push %ds 	
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	# validity check:  valid syscall # 1-10
	cmpl $1, %eax
	jl invalid
	cmpl $12, %eax
	jg invalid
nobadsys:
	# invoke specific service routine
	addl $-1, %eax
	sti 
	call *sys_call_table(, %eax, 4)
	# stores return value in stack location
	movl %eax, 24(%esp)
	
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop %ds
	pop %es
	addl $4, %esp
	iret
invalid:	
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es	
	addl $4, %esp
	xorl %eax, %eax
	movl $-1, %eax
	iret

sys_call_table:
	.long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_kmalloc, sys_kfree #, sys_set_handler, sys_sigreturn
	# only 8 syscall now, last two for extra credit
