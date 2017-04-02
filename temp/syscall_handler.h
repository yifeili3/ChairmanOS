#ifndef _SYSCALL_WRAPPER_H
#define _SYSCALL_WRAPPER_H

#include "x86_desc.h"

#ifndef ASM

#define SAVE_ALL	\
		"cld"		\
		"push %es"	\
		"push %ds" 	\
		"pushl %eax"\
		"pushl %ebp"\
		"pushl %edi"\
		"pushl %esi"\
		"pushl %edx"\
		"pushl %ecx"\
		"pushl %ebx"
		/*	
		"movl USER_DS, %edx"\
		"movl %edx, %ds"\
		"movl %edx, %es"
		*/

#define RESTORE_ALL 	\
		"popl %ebx"		\
		"popl %ecx"		\
		"popl %edx"		\
		"popl %esi"		\
		"popl %edi" 	\
		"popl %ebp"		\
		"popl %eax"		\
		"popl %ds"		\
		"popl %es"		\
		"std"	


// assembly wrapper for systemcall_handler
extern void syscall_handler();

#endif
#endif