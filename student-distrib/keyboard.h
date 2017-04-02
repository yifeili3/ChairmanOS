#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#define INPUT_SIZE 128

#include "lib.h"
#include "keyboard.h"
#include "types.h"
#include "paging.h"
#include "syscall.h"

// the main function to handle keyboard interrupt
void handle_keyboard_interrupt();

/* reads data from the keyboard */ 
int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes);

/* writes data to terminal */ 
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);

/* opens a file */
int32_t terminal_open(const uint8_t* filename);

/* closes a file */
int32_t terminal_close(int32_t fd);

/* initializes the terminal */
void terminal_init();

// the extern function defined in syscall.h
extern int32_t sys_halt(uint8_t status);
extern int32_t sys_execute(const uint8_t * command);

/* initializes the terminal */
int8_t get_current_flag();

/* initializes the terminal */
void set_current_flag(int8_t flag);

/* initializes the terminal */
uint8_t get_disp_terminal();

/* initializes the terminal */
void modify_terminal_curr_pid(int8_t pid);

/* initializes the terminal */
void recover_terminal_curr_pid(int8_t current_pid, int new_terminal_pid);

/* the terminal struct */
typedef struct terminal {
	// the top pid of the shell
	int32_t  shell_pid;

	// its own input count for the input buffer
	int32_t  input_count;

	// the local keyboard buffer
	uint8_t  local_keyboard_buffer[INPUT_SIZE];

	// flag for initialize
	uint8_t init;

	// its esp and ebp vefore context switch
	uint32_t esp;
	uint32_t ebp;
} terminal_t;

/* the terminals array */
terminal_t terminals[3];

#endif
