#ifndef _INTERRUPT_WRAPPER_H
#define _INTERRUPT_WRAPPER_H

#ifndef ASM
/* extern defined assembly wrapper for keyboard_handler, rtc_handler, and pit_handler */
extern void kbd_handler();
extern void rtc_handler();
extern void pit_handler();
extern void mouse_handler();

#endif
#endif
