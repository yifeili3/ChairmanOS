#include "trap.h"

#include "lib.h"

/* FUNCTION DESCRIPTION: C function for handling divide error exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
void do_divide_error(void) {
	printf("Divide Error. (0x00)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling debug exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_debug(void) {
	printf("Debug. (0x01)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling nmi interrupt exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT:none
*/
 void do_nmi_intr(void) {
	printf("Non-maskable Interrupt. (0x02)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling int3 exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_int3(void) {
	printf("Breakpoint. (0x03)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling overflow exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_overflow(void) {
	printf("Overflow. (0x04)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling bound range exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_bound_range(void) {
	printf("Bounds Check. (0x05)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling invalid opcode exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_invalid_opcode(void) {
	printf("Invalid Opcode. (0x06)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling device not available exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_device_not_aval(void) {
	printf("Device not available. (0x07)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling double fault exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_double_fault(void) {
	printf("Double Fault. (0x08)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling coprocessor segment overrun exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_coprocessor_seg_overrun(void) {
	printf("Coprocessor segment overrun. (0x09)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling invalid tss exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_invalid_tss(void) {
	printf("Invalid TSS. (0x0A)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling segment not present exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_segment_not_present(void) {
	printf("Segment not present. (0x0B)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling stack fault exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_stack_fault(void) {
	printf("Stack segment fault. (0x0C)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling general protection exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_general_protection(void) {
	printf("General Protection. (0x0D)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling page fault exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_page_fault(void) {
 	uint32_t err_add;
	printf("Page Fault. (0x0E)\n");

	asm volatile(" movl %%cr2, %%eax	\n\
					movl %%eax, %0 "
					: "=r"(err_add) 
					: /* No inputs */
					: "eax");
	printf(" Page Fault at address 0x%#x \n", err_add);
	while(1);
}



/* FUNCTION DESCRIPTION: C function for floating point error overflow exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_floating_point_error(void) {
	printf("Floating-point error. (0x10)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling alignment check exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_alignment_check(void) {
	printf("Alignment check. (0x11)\n");
	while(1);
}

/* FUNCTION DESCRIPTION: C function for handling machinecheck exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_machine_check(void) {
	printf("Machine check. (0x12)\n");
	while(1);
}



/* FUNCTION DESCRIPTION: C function for handling simd floating point exception; used in IDT.
* INPUT:none
* OUTPUT: none
* SIDE EFFFECT: mask out interrupt
*/
 void do_simd_floating_point(void) {
	printf("SIMD floating point. (0x13)\n");
	while(1);
}
