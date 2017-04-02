#include "x86_desc.h"
#include "IDT.h"
#include "lib.h"
#include "trap.h"
#include "keyboard.h"
#include "rtc.h"
#include "interrupt_wrapper.h"
#include "trap_wrapper.h"
#include "syscall_handler.h"
#include "mouse.h"

/*
 * Trap and Interrupt gate layout (from Intel IA-32 reference):
 * r -> reserved in implementation of x86_desc.h
 * Interrupt: [12:8] = [0D110] Trap: [12:8] = [0D111]
 *  31          16 15  14 13 12  11  10   9   8  7   0
 * |--------------|---|-----|---|---|---|---|---|-----|
 * |              |   |  D  | r |   | r | r | r |  r  |
 * | Offset 31:16 | P |  P  | 0 | D | 1 | 2 | 3 |  4  |
 * |              |   |  L  |   |   |   |   |   |     |
 * |--------------------------------------------------|
 * | Segment      |                                   |
 * | Selector     |           Offset 15:0             |
 * |              |                                   |
 * |--------------------------------------------------|
 */
//dpl of interrupt and exception are set to 0
//dpl of system call is set to 3
#define NUM_EXCEPTION 0x20

/* 
 * init_idt
 *   DESCRIPTION: Initializes the idt table. set up enties for exception, interrupt and system call
 * 	 			  for entry vector refer to IDT.h file
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 */
void init_idt(){
	//set up entries for exception
	SET_IDT_ENTRY(idt[0],(uint32_t)divide_error_handler);//0
	SET_IDT_ENTRY(idt[1],(uint32_t)debug_handler);//1
	SET_IDT_ENTRY(idt[2],(uint32_t)nmi_intr_handler);//2
	SET_IDT_ENTRY(idt[3],(uint32_t)int3_handler);//3
	SET_IDT_ENTRY(idt[4],(uint32_t)overflow_handler);//4
	SET_IDT_ENTRY(idt[5],(uint32_t)bound_range_handler);//5
	SET_IDT_ENTRY(idt[6],(uint32_t)invalid_opcode_handler);//6
	SET_IDT_ENTRY(idt[7],(uint32_t)device_not_aval_handler);//7
	SET_IDT_ENTRY(idt[8],(uint32_t)double_fault_handler);//8
	SET_IDT_ENTRY(idt[9],(uint32_t)coprocessor_seg_overrun_handler);//9
	SET_IDT_ENTRY(idt[10],(uint32_t)invalid_tss_handler);//10
	SET_IDT_ENTRY(idt[11],(uint32_t)segment_not_present_handler);//11
	SET_IDT_ENTRY(idt[12],(uint32_t)stack_fault_handler);//12
	SET_IDT_ENTRY(idt[13],(uint32_t)general_protection_handler);//13
	SET_IDT_ENTRY(idt[14],(uint32_t)page_fault_handler);//14
	// vector 0x0f
	SET_IDT_ENTRY(idt[16],(uint32_t)floating_point_error_handler);//16
	SET_IDT_ENTRY(idt[17],(uint32_t)alignment_check_handler);//17
	SET_IDT_ENTRY(idt[18],(uint32_t)machine_check_handler);//18
	SET_IDT_ENTRY(idt[19],(uint32_t)simd_floating_point_handler);//19	
	int i;
	// set up value for exception
	// 20 (0x14) to 31 (0x1f) is reserved by Intel
	for(i = 0; i < NUM_EXCEPTION; i++) { 
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0x00;
		idt[i].reserved3 = 1;
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;
		idt[i].present = 1;
	}

	//set entries for KEYBOARD, RTC interrupt
	SET_IDT_ENTRY(idt[KEYBOARD_INTR],(uint32_t)kbd_handler);
	idt[KEYBOARD_INTR].seg_selector = KERNEL_CS;
	idt[KEYBOARD_INTR].reserved4 = 0x00;
	idt[KEYBOARD_INTR].reserved3 = 0;
	idt[KEYBOARD_INTR].reserved2 = 1;
	idt[KEYBOARD_INTR].reserved1 = 1;
	idt[KEYBOARD_INTR].size = 1;
	idt[KEYBOARD_INTR].reserved0 = 0;
	idt[KEYBOARD_INTR].dpl = 0;
	idt[KEYBOARD_INTR].present = 1;

	SET_IDT_ENTRY(idt[RTC_INTR],(uint32_t)rtc_handler);
	idt[RTC_INTR].seg_selector = KERNEL_CS;
	idt[RTC_INTR].reserved4=0x00;
	idt[RTC_INTR].reserved3=0;
	idt[RTC_INTR].reserved2=1;
	idt[RTC_INTR].reserved1=1;
	idt[RTC_INTR].size=1;
	idt[RTC_INTR].reserved0=0;
	idt[RTC_INTR].dpl=0;
	idt[RTC_INTR].present=1;

	//set entries for mouse interrupt 
	SET_IDT_ENTRY(idt[MOUSE_INTR],(uint32_t)mouse_handler);
	idt[MOUSE_INTR].seg_selector = KERNEL_CS;
	idt[MOUSE_INTR].reserved4 = 0x00;
	idt[MOUSE_INTR].reserved3 = 0;
	idt[MOUSE_INTR].reserved2 = 1;
	idt[MOUSE_INTR].reserved1 = 1;
	idt[MOUSE_INTR].size = 1;
	idt[MOUSE_INTR].reserved0 = 0;
	idt[MOUSE_INTR].dpl = 0;
	idt[MOUSE_INTR].present = 1;	
	
	SET_IDT_ENTRY(idt[PIT_INTR],(uint32_t)pit_handler);
	idt[PIT_INTR].seg_selector = KERNEL_CS;
	idt[PIT_INTR].reserved4 = 0x00;
	idt[PIT_INTR].reserved3 = 0;
	idt[PIT_INTR].reserved2 = 1;
	idt[PIT_INTR].reserved1 = 1;
	idt[PIT_INTR].size = 1;
	idt[PIT_INTR].reserved0 = 0;
	idt[PIT_INTR].dpl = 0;
	idt[PIT_INTR].present = 1;

	//set entry for system call
	SET_IDT_ENTRY(idt[SYSTEM_CALL],(uint32_t)syscall_handler); //system call
	idt[SYSTEM_CALL].seg_selector = KERNEL_CS;
	idt[SYSTEM_CALL].reserved4=0x00;
	idt[SYSTEM_CALL].reserved3=0;
	idt[SYSTEM_CALL].reserved2=1;
	idt[SYSTEM_CALL].reserved1=1;
	idt[SYSTEM_CALL].size=1;
	idt[SYSTEM_CALL].reserved0=0;
	idt[SYSTEM_CALL].dpl=3;
	idt[SYSTEM_CALL].present=1;
	
	lidt(idt_desc_ptr);
}

