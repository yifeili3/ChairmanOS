#include "syscall.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"
#include "x86_desc.h"
#include "types.h"

#define EXE_ID 0x464c457f


// void init_pid() {
// 	int i;
// 	for (i = 0; i < 2; i++) {
// 		pidlist[i].pid = i;
// 		pidlist[i].proc = 0;
// 		pidlist[i].flag = 0;
// 	}
// }

int32_t sys_execute(const uint8_t * command){
	uint8_t program[128];
	uint8_t arg[128];
	uint32_t cmd_i = 0
	uint32_t idx = 0;
	/*
	for (idx = 0; idx < 128; idx++) {
		program[idx] = '/0';
		arg[idx] = '/0';
	}
	*/
	/* Check if command is valid */ 
	if (command[0] =='\0'|| command[0] ==' ' || command[0] =='\n') {
		puts("Invalid command!\n");
		return -1;
	}
	/* Parse to get exe name */ 
	while (command[cmd_i] != '\0' && command[cmd_i] != ' '&& command[cmd_i] != '\n') {		
		program[idx] = command[cmd_i];
		idx++;
		cmd_i++;
	}
	program[idx]='\0';
	idx = 0;
	/* Parse to get exe args */
	while(command[cmd_i] != '\0' && command[cmd_i] != '\n'){
		arg[idx] = command[cmd_i];
		idx++;
		cmd_i++;
	}
	arg[idx]='\0';

	/* Check if file exists */
	dentry_t temp;
	if (read_entry_by_name(program,&temp)==-1){
		puts("File does not exist.\n");
		return -1;
	}
	/* Check if file is executable */
	uint8_t Magic[4];
	if (read_data(temp.inode_idx, 0, Magic, 4) == -1) {
		puts("Failed to read file.\n");
		return -1;
	}
	if ((*((uint32_t*)Magic)) != EXE_ID) {
		puts("Not an executable.\n");
		return -1;
	}



	uint32_t phys_addr;
	if (pidlist[0].flag == 1) {
		if (pidlist[1].flag == 1) {
			puts("Process list full.\n");
			return -1;
		}
		else{
		//process 1 unused
			pidlist[1].pid=1;
			pidlist[1].flag=1;
			current_pid=1;
			phys_addr = 0x00C00000;
		}
	}
	else{
		//process 0 unused
		pidlist[0].pid=0;
		pidlist[0].flag=1;
		current_pid=0;
		phys_addr = 0x00800000;
	}

	/* Need to add page swap for multiple programs */
	if(allocatePage(2, 1, phys_addr) == -1) {
		puts("Page allocated, need to swap.\n");
		return -1;
	}
	uint8_t entry[4];
	
	read_data(temp.inode_idx, 24, entry, 4);
	read_data(temp.inode_idx, 0, (uint8_t*)0x08048000, 0x00400000 -0x00048000);

	uint32_t eip_entry;
	eip_entry = *((uint32_t*)entry);


	pcb_t * cur_process=(pcb_t*)(0x00800000-0x2000*current_pid);
	/*works for now*/
	if(current_pid==0){
		pidlist[0].proc=cur_process;
		cur_process->parent=cur_process;
	}
	else{
		pidlist[1].proc=cur_process;
		cur_process->parent=0x00800000;
	}

	/*needs to modify later*/
	int i;
	for(i=0;i<8;i++){
		cur_process->file_array[i].fop_ptr=NULL;
		cur_process->file_array[i].inode=0;
		cur_process->file_array[i].file_pos=0;
		cur_process->file_array[i].flag=0;
	}
		
		cur_process->file_array[0].fop_ptr={NULL,(int32_t)terminal_read,NULL,NULL};
		cur_process->file_array[0].inode=0;
		cur_process->file_array[0].file_pos=0;
		cur_process->file_array[0].flag=1;
	

		cur_process->file_array[1].fop_ptr={NULL,NULL,(int32_t)terminal_write,NULL};
		cur_process->file_array[1].inode=0;
		cur_process->file_array[1].file_pos=0;
		cur_process->file_array[1].flag=1;


		cur_process->pid=current_pid;
		


		asm volatile ("leal halt_ret, %%eax\n\t");
		asm volatile ("movl %%eax, %0\n\t" 
      			 :"r"(cur_process->ret_addr) 
       		 	 :); 
		asm volatile (
				"movl %%ebp, %0\n\t"
				:
			    :"r"(cur_process->ebp)
			    :);

		asm volatile (
			    "movl %%esp, %0\n\t"
				:
				:"r"(cur_process->esp)
				:);


		cur_process->esp0=tss.esp0;
		cur_process->ss0=tss.ss0;

		//content switch
		tss.ss0=KERNEL_DS;
		tss.esp0=0x00800000-0x2000*current_pid-4; //one block above pcb_t

		asm volatile(
			"movw %0, %%ax  \n\
			 movw %%ax, %%ds      \n\
			 movw %%ax, %%ex 		\n\
			 movw %%ax, %%fs      \n\
			 movw %%ax, %%gs      \n\
			 pushl %0		\n\
			 pushl %1		\n\
			 movl %EFLAGS, %eax \n\
			 orl $0x0200,%eax   \n\
			 pushl %eax         \n\
			 pushl %2      \n\
			 pushl %3    \n\
			 iret"
			 :
			 :"i"(USER_DS),"i"(0x840000-4),"i"(USER_CS),"r"(eip_entry)
			 :"memory","cc","eax"
			);
		//not sure about user esp
		asm volatile("iret");
		asm volatile("halt_ret:");

		//return ; 
}
