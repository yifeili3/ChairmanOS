#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "types.h"
#include "filesys.h"
#include "paging.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"
#include "kmalloc.h"

#define EIGHTMB 	0x00800000
#define FOURMB		0x00400000
#define EIGHTKB 	0x00002000
#define FOURKB		0x00001000
#define FDSIZE		8
#define STATICFD	1
#define PROGRAMIMG  0x08048000
#define MAXPROC		6
//#define asmlinkage CPP_ASMLINKAGE __attribute__((regparm(0)))

/* the system call function signiture for open read write close function
  used for jump table */
typedef int32_t (*open_syscall)(const uint8_t*);
typedef int32_t (*read_syscall)(int32_t, void*, int32_t);
typedef int32_t (*write_syscall)(int32_t, const void*, int32_t);
typedef int32_t (*close_syscall)(int32_t);

/* system call jump table data structure*/
typedef struct {
	open_syscall open;
	read_syscall read;
	write_syscall write;
	close_syscall close;
} syscall_jump_table;

/* Each element in file array */
typedef struct __attribute__((packed)) {
	syscall_jump_table* file_op_ptr;
	uint32_t inode;
	uint32_t file_pos;
	uint32_t flag;
} file_element_t;

/* Process control block structure 
	1. file_array[FDSIZE]: file elements associated with this pcb
	2. parent_pid: store the parent of current process. This is used when handling scheduling 
	   and exit from current process
	3. pid: id of current process
	4: esp0: starting address of the kernel stack of current process
	5: eip: instruction pointer for jumping to user program
	6: ss0: store KERNEL_DS segment
	7: esp: most important info about the current process. store kernel stack pointer of this process int pcb struct
	8: ebp: most important info about the current process. store kernel stack base pointer of this process int pcb struct
	9: arg: used in get_arg
	10:ret_addr: return address used in sys_halt
*/
typedef struct pcb_t {
	file_element_t file_array[FDSIZE];
	uint8_t parent_pid;
	uint8_t pid;
	uint32_t esp0;
	uint32_t eip;
	uint8_t ss0;
	uint32_t esp;
	uint32_t ebp;
	uint8_t * arg;
	uint32_t ret_addr;
}pcb_t;

/* Process Identifier structure */
typedef struct __attribute__((packed)) {
 	uint8_t pid;
 	pcb_t* proc;
 	uint8_t flag;
} PID_t;

/* Structures declared to use for all system calls */
PID_t pidlist[MAXPROC];

/* the current pid running */
uint32_t current_pid;

/* the next scheduled terminal */
uint8_t scheduled_terminal;

/* Functions related to system calls */
void init_pid();
int32_t sys_halt(uint8_t status);
int32_t sys_execute(const uint8_t * command);
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void * buf, int32_t nbytes);
int32_t sys_open(const uint8_t * filename);
int32_t sys_close(int32_t fd);
int32_t sys_getargs(uint8_t * buf, int32_t nbytes);
int32_t sys_vidmap(uint8_t ** screen_start);
void * sys_kmalloc(unsigned int size);
unsigned int sys_free(void * physaddr);

/* get current pid running */
uint32_t get_current_pid();

/* do the sheduling between the three terminal */
void schedule();

/* context switch between the two terminals */
void context_switch(uint8_t previous_terminal, uint8_t scheduled_terminal);

/*below are extra credits*/
/*
int32_t set_handler(int32_t signum, void * handler_address);
int32_t sigreturn(void);
*/

#endif
