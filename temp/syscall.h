#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "types.h"
#include "filesys.h"
#include "paging.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"


typedef struct __attribute__((packed)) {
	int32_t* fop_ptr;
	uint32_t inode;
	uint32_t file_pos;
	uint32_t flags;
} file_element_t;

typedef struct __attribute__((packed)) {
	file_element_t file_array[8];
	pcb_t* parent;
	uint8_t pid;
	uint32_t esp0;
	uint32_t eip;
	uint8_t ss0;
	uint32_t esp;
	uint32_t ebp;
	uint8_t * arg;
	uint32_t ret_addr;
} pcb_t;

typedef struct __attribute__((packed)) {
 	uint8_t pid;
 	pcb_t* proc;
 	uint8_t flag;
} PID_t;

PID_t pidlist[10];
uint32_t current_pid;


void init_pid();
asmlinkage int32_t sys_halt(uint8_t status);
asmlinkage int32_t sys_execute(const uint8_t * command);
asmlinkage int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
asmlinkage int32_t sys_write(int32_t fd, const void * buf, int32_t nbytes);
asmlinkage int32_t sys_open(const uint8_t * filename);
asmlinkage int32_t sys_close(int32_t fd);
asmlinkage int32_t sys_getargs(uint8_t * buf, int32_t nbytes);
asmlinkage int32_t sys_vidmap(uint8_t ** screen_start);

/*below are extra credits*/
/*
int32_t set_handler(int32_t signum, void * handler_address);
int32_t sigreturn(void);
*/

/*help function below*/

#endif
