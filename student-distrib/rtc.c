#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "syscall.h"
#include "magic.h"

#define PORT0			0x70
#define PORT1			0x71
#define REG_A			0x0A
#define REG_B			0x0B
#define REG_C			0x0C
#define MAGIC			32768
#define BIT6_REGB		0x40
#define BOTTOM_FOUR_BITS 0xF0
#define SUCCESS 		0
#define FAILURE 		-1
#define MASK_LAST_FOUR	0x000F
#define INIT_RATE 		2
#define LARGEST_FREQ 	1024

// local variable
uint32_t curr_rate = INIT_RATE;				// the current rate to set
volatile int curr_interrupt[3] = {-1, -1, -1};	// flag for read interrupt

// local function
static int calculate_rate(uint32_t freq);
int check_param(uint32_t freq);

/* int init_rtc()
 * INPUT: NONE
 * OUTPUT: 0 (ALWAYS SUCCESS)
 * FUNCITON: initialize rtc
 */
int32_t
init_rtc()
{
	outb(REG_B, PORT0);									// set index to register B, disable NMI
	char prev= inb(PORT1);								// get initial value of register B
	outb(REG_B, PORT0);									// reset index to B
	outb(prev | BIT6_REGB, PORT1);  					// write value to REG_B

	outb(REG_A, PORT0);									// set index to register A, disable NMI
	prev= inb(PORT1);									// get initial value of register A
	outb(REG_A, PORT0);									// reset index to A
	uint8_t rate = calculate_rate(curr_rate) & MASK_LAST_FOUR;  // calculate the rate
	outb(REG_A, PORT0);
	outb(prev | rate, PORT1);  							//write the rate to A.
	curr_interrupt[0] = 0;
	curr_interrupt[1] = 0;
	curr_interrupt[2] = 0;
	// enable rtc interrupt
	enable_irq(RTC_IRQ);
	return SUCCESS;
}

/* int calculate_rate(uint32_t freq)
 * INPUT: the frequency for the rtc
 * OUTPUT: the rate for the rtc
 * FUNCITON: calculate the rate for the rtc
 */
int calculate_rate(uint32_t freq)
{
	// calculate rate
	int shift = 0;
	while(freq != MAGIC){
		shift++;
		freq = freq << 1;
	}

	// return rate + 1 for correct result
	return shift + 1;
}

/* void do_rtc_handler()
 * INPUT: NONE
 * OUTPUT: NONE
 * FUNCITON: main rtc handler
 */
void do_rtc_handler(){
	//printf("RTC interrupt received!\n");
	//test_interrupts();

	// flush REG_C
	outb(REG_C, PORT0);	
	inb(PORT1);
	// send eoi to rtc
	send_eoi(RTC_IRQ);

	//clear flag
	curr_interrupt[0] = 0;
	curr_interrupt[1] = 0;
	curr_interrupt[2] = 0;
}

/* int32_t open()
 * INPUT: NONE
 * OUTPUT: SUCCESS
 * FUNCITON: open the rtc handler
 */
int32_t open_rtc(const uint8_t* filename){
	return init_rtc();
}

/* int32_t read_rtc(void* buf, int32_t count)
 * INPUT: the buffer to read, 
 * OUTPUT: SUCCESS
 * FUNCITON: block rtc until the next interrupt arrives
 */
int32_t read_rtc(int32_t fd, void* buf, int32_t count){
	// wait until rtc interrupt reach a certain number
	disable_irq(KEYBOARD_IRQ);

	curr_interrupt[scheduled_terminal] = 1;
	while(1 == curr_interrupt[scheduled_terminal]){};
	
	//printf("return from RTC read!\n");
	enable_irq(KEYBOARD_IRQ);
	return SUCCESS;
}

/* int32_t write_rtc(void* buf, int32_t count)
 * INPUT: the frequency to set
 * OUTPUT: SUCCESS
 * FUNCITON: change rtc frequency
 */
int32_t write_rtc(int32_t fd, const void* buf, int32_t count){
	uint32_t freq = *(uint32_t*) buf;
	//printf("current freq is: %d\n", freq);
	if((freq > LARGEST_FREQ) || (freq <= 0)){
		printf("Invalid Parameter!\n");
		return FAILURE;
	}
		
	if(-1 == check_param(freq)){
		printf("Invalid Parameter!\n");
		return FAILURE;
	}

	cli();
	outb(REG_A, PORT0);									// set index to register A, disable NMI
	char prev= inb(PORT1) & BOTTOM_FOUR_BITS;			// get initial value of register A
	uint8_t rate = calculate_rate(freq) & MASK_LAST_FOUR;   // calculate the rate
	outb(REG_A, PORT0);
	outb(prev | rate, PORT1);  							//write the rate to A.
	sti();
	return SUCCESS;
}

/* int check_param(uint32_t freq)
 * INPUT: the frequency to be set to rtc handler
 * OUTPUT: 1 if the input is valid, 0 if not
 * FUNCITON: sanity check for write rtc
 */
int32_t check_param(uint32_t freq){
	int32_t i = 1;

	// check whether input is a power of 2
	while(i <= freq){
		if(freq == i)
			// if true, return 1
			return 1;

		i = i << 1;
	}

	// if not, return -1
	return -1;
}

/* int32_t close_rtc(int32_t fd)
 * INPUT: the file to be close
 * OUTPUT: Success
 * FUNCITON: CLOSE RTC
 */
int32_t close_rtc(int32_t fd){
	return SUCCESS;
}
