#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "types.h"

#define CHANNEL0 0x40         // Channel 0 data port (read/write)
#define CHANNEL1 0x41         // Channel 1 data port (read/write)
#define CHANNEL2 0x42         // Channel 2 data port (read/write)
#define REG      0x43         // Mode/Command register (write only, a read is ignored)
#define MAGIC    1193182 
#define PIT_IRQ  0
#define BYTE     0XFF
#define SHIFT    8

#define NUM_TICKS 3
uint32_t pit_num;

int32_t init_pit(int32_t channel_num, int32_t freq){
	enable_irq(PIT_IRQ);
	switch(channel_num):{
		case 0:
			int32_t data = MAGIC / freq;
			outb(0, REG);
			outb(data & BYTE, CHANNEL0);
			outb((data >> SHIFT) & BYTE, CHANNEL0);
		case 2:
			return 0;

		default:
			return 0;
	}
}


int32_t pit_handler(){
	cli();
	pit_num++;							//increase PIT number whenever we get a PIT interrupt->this will help with clock frequency
	//schedule();							//display the status bar with the time displayed
	if(pit_num%(100) == 0)				//This decides when the time on the clock will increase by 1 sec
	{	
		//status = embed_time(status);		//put this time on the status bar
	}	
	sti();													
	send_eoi(PIT_IRQ);										
	return;
}