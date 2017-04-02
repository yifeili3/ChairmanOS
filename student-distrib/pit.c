#include "pit.h"

#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"

#define CHANNEL0 0x40         // Channel 0 data port (read/write)
#define CHANNEL1 0x41         // Channel 1 data port (read/write)
#define CHANNEL2 0x42         // Channel 2 data port (read/write)
#define REG      0x43         // Mode/Command register (write only, a read is ignored)
#define MAGIC    1193182 
#define PIT_IRQ  0
#define BYTE     0XFF
#define SHIFT    8

/* void init_pit(uint32_t channelnum, uint32_t freq)
 * INPUT: NONE
 * OUTPUT: NONE
 * FUNCITON: initialize pit
 */
void init_pit(uint32_t channelnum, uint32_t freq){
	// enable pit irq
	enable_irq(PIT_IRQ);

	//switch the channel number for more options
	switch(channelnum)
	{
		case 0:{
			// set initial freq and initialize the pit
			int32_t data = MAGIC / freq;
			outb(0, REG);
			outb(data & BYTE, CHANNEL0);
			outb((data >> SHIFT) & BYTE, CHANNEL0);
			return;
		}

		default:
			return;
	}
	return;
}

/* void do_pit_handler()
 * INPUT: NONE
 * OUTPUT: NONE
 * FUNCITON: main pic handler
 */
void do_pit_handler(){
	//send eoi here and call the schedule function													
	send_eoi(PIT_IRQ);
	schedule();
	return;
}
