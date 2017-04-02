#define ASM 1
#include "interrupt_wrapper.h"
.text

.global kbd_handler, rtc_handler, pit_handler, mouse_handler
# Function Description: assembly wraper for keyboard interrupt handler. 
# 						save all register and flags and restore afterwards.
# INPUT: none
# OUTPUT: none
# SIDE EFFECT:
#

kbd_handler:
 	cli
	pushal   
	pushfl
	call handle_keyboard_interrupt
	popfl
	popal
 	sti
	iret

mouse_handler:
 	cli
	pushal   
	pushfl
	call mouse_interrupt_handler
	popfl
	popal
 	sti
	iret

rtc_handler:
	cli
	pushal
	pushfl
	call do_rtc_handler
	popfl
	popal
	sti
	iret

pit_handler:
	cli
	pushal
	pushfl
	call do_pit_handler
	popfl
	popal
	sti
	iret
