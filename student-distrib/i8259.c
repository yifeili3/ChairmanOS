/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define  MASTER_SLAVE_PORT  2
#define  MAX_MASTER_PORT  7
#define  MASKMASTER 0xFB
#define  MASKSLAVE 0xFF
#define  hex01 0x01
#define  OFFSET 8
// global variable for the master and slave mask
uint8_t master; /* IRQs 0-7 */
uint8_t slave; /* IRQs 8-15 */

/* void i8259_init() 
   input: none
   output: none
   function: initialize the 8259 PIC
*/
void
i8259_init(void)
{
	  master = MASKMASTER;
    slave = MASKSLAVE;

    // disable all the interrupts
    outb(MASKSLAVE, MASTER_8259_PORT + 1);
    outb(MASKSLAVE, SLAVE_8259_PORT + 1);

	  // send initialize bytes to master and slave ports
	  outb(ICW1, MASTER_8259_PORT);
  	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
  	outb(ICW3_MASTER, MASTER_8259_PORT + 1);
  	outb(ICW4, MASTER_8259_PORT + 1);

  	outb(ICW1, SLAVE_8259_PORT);
  	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
  	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
  	outb(ICW4, SLAVE_8259_PORT + 1);
    
    // disable all the interrupts
    outb(master, MASTER_8259_PORT + 1);
    outb(slave, SLAVE_8259_PORT + 1);
}

/* void enable_irq(uint32_t irq_num) 
   input: the interupt to enable
   output: none
   function: enable the specified IRQ
*/
void
enable_irq(uint32_t irq_num)
{
	  uint8_t mask = 0;
    cli();
	  if (irq_num <= MAX_MASTER_PORT) {
		    // generate new master mask
        mask = inb(MASTER_8259_PORT + 1) & (~(hex01 << irq_num));

		    // send new masks to PIC 
	      outb(mask, MASTER_8259_PORT + 1);
    }
    else{
		    // generate new slave mask
        mask = inb(MASTER_8259_PORT + 1) & (~(hex01 << (irq_num - OFFSET)));

		    // send new masks to PIC 
	      outb(mask, SLAVE_8259_PORT + 1);
    }
    sti();
}

/* void disable_irq(uint32_t irq_num) 
   input: the interupt to disable
   output: none
   function: disable the specified IRQ
*/
void
disable_irq(uint32_t irq_num)
{
	  uint8_t mask = 0;
    cli();
	  if (irq_num <= MAX_MASTER_PORT) {
		    // generate new master mask
		    mask = inb(MASTER_8259_PORT + 1) | (hex01 << irq_num);
    	  outb(mask, MASTER_8259_PORT + 1); 
    }
    else{
    		// generate new slave mask
        mask = inb(MASTER_8259_PORT + 1) | (hex01 << (irq_num - OFFSET));
		
    		// send new masks to PIC 
    	  outb(mask, SLAVE_8259_PORT + 1);
    }
    sti();
}

/* void send_eoi(uint32_t irq_num) 
   input: the interupt to send the eoi
   output: none
   function: send eoi for the specified IRQ 
*/
void
send_eoi(uint32_t irq_num)
{
	   uint8_t mask = 0;
	   if (irq_num <= MAX_MASTER_PORT) {
		    // generate and send the signal to master
		    mask = (EOI | irq_num);
        outb(mask, MASTER_8259_PORT);
    }
    else{
		    // generate and send the signal to slave
		    mask = (EOI | (irq_num - OFFSET));
        outb(mask, SLAVE_8259_PORT);

        // send eoi for the master port
        mask = (EOI | MASTER_SLAVE_PORT);
        outb(mask, MASTER_8259_PORT);
    }
}

