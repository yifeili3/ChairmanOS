#ifndef TRAP_H
#define TRAP_H

/*exception handler for IDT table */
extern void do_divide_error();
extern void do_debug();
extern void do_nmi_intr();
extern void do_int3();
extern void do_overflow();
extern void do_bound_range();
extern void do_invalid_opcode();
extern void do_device_not_aval();
extern void do_double_fault();
extern void do_coprocessor_seg_overrun();
extern void do_invalid_tss();
extern void do_segment_not_present();
extern void do_stack_fault();
extern void do_general_protection();
extern void do_page_fault();
extern void do_floating_point_error();
extern void do_alignment_check();
extern void do_machine_check();
extern void do_simd_floating_point();

#endif
