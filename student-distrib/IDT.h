#ifndef IDT_H
#define IDT_H

//define entry vector for exception
#define DIVIDE_ERROR 0
#define DEBUG 1
#define NMI_INTR 2
#define INT3 3
#define OVERFLOW 4
#define BOUND_RANGE 5
#define INVALID_OPCODE 6
#define DEVICE_NOT_AVAL 7
#define DOUBLE_FAULT 8
#define COPROCESSOR_SEG_OVERRUN 9
#define INVALID_TSS 10
#define SEGMENT_NOT_PRESENT 11
#define STACK_FAULT 12
#define GENERAL_PROTECTION 13
#define PAGE_FAULT 14
#define FLOATING_POINT_ERROR 16
#define ALIGNMENT_CHECK 17
#define MACHINE_CHECK 18
#define SIMD_FLOATING_POINT 19

#define PIT_INTR 0x20
#define MOUSE_INTR 0x2C
#define KEYBOARD_INTR 0x21
#define RTC_INTR 0x28

#define SYSTEM_CALL 0x80
//initiaize interrupt descriptor table
void init_idt();
#endif


