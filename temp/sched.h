#ifndef _SCHED_H
#define _SCHED_H

#include "types.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"

struct pid_struct {
	uint32_t pid;
	uint32_t thread_info;
	uint32_t in_use;
};

struct task_struct {
	unsigned int pid;
	volatile long state;
	unsigned long esp0;
	unsigned long eip;
	unsigned long args;
	unsigned long ret_val;
	file_operations* fops;
	struct task_struct *parent;	
};



#endif