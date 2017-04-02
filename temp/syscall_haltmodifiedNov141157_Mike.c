#include "syscall.h"
#include "lib.h"
#define SUCCESS 1
#define EXE_ID 0x464c457f
#define EIGHT_MB 0x0800000
#define EIGHT_KB 0x2000

uint32_t t_op[4] = {(uint32_t) terminal_read, (uint32_t) terminal_write, (uint32_t) terminal_open, (uint32_t) terminal_close};
//uint32_t page[4]

syscall_jump_table rtc_table = {open_rtc, read_rtc, write_rtc, close_rtc};
syscall_jump_table file_table = {file_open, file_read, file_write, file_close};
syscall_jump_table dir_table = {dir_open, dir_read, dir_write, dir_close};
syscall_jump_table stdin_table = {NULL, terminal_read, NULL, NULL};
syscall_jump_table stdout_table = {NULL, NULL, terminal_write, NULL};

void init_pid() {
	int i;
	for (i = 0; i < 2; i++) {
		pidlist[i].pid = i;
		pidlist[i].proc = 0;
		pidlist[i].flag = 0;
	}
}

int32_t sys_execute(const uint8_t * command){

	uint8_t program[128];
	uint8_t arg[128];
	uint32_t cmd_i = 0;
	uint32_t idx = 0;
	/*
	for (idx = 0; idx < 128; idx++) {
		program[idx] = '/0';
		arg[idx] = '/0';
	}
	*/
	/* Check if command is valid */ 
	uint32_t esp;
	uint32_t ebp;
	asm volatile ("movl %%ebp, %0\n\t":"=r"(ebp):);

	asm volatile (
			    "movl %%esp, %0\n\t"
				:"=r"(esp)
				:);

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
	if (read_dentry_by_name(program,&temp)==-1){
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
	if(allocatePage(0x20, 1, phys_addr) == -1) {
		puts("Page allocated, need to swap.\n");
		clearPage(0x20);
		allocatePage(0x20, 1, phys_addr);
	}

	uint8_t entry[4];
	
	read_data(temp.inode_idx, 24, entry, 4);
	read_data(temp.inode_idx, 0, (uint8_t*)0x08048000, 0x00400000-0x00048000);

	uint32_t eip_entry;
	eip_entry = *((uint32_t*)entry);

	pcb_t * cur_process = (pcb_t*)(0x00800000-0x2000*(current_pid+1));
	//printf("%h\n", (uint32_t*)cur_process);
	/*works for now*/
	if(current_pid == 0){
		pidlist[0].proc = cur_process;
		cur_process->parent = cur_process;
	}
	else{
		pidlist[1].proc = cur_process;
		cur_process->parent  = pidlist[0].proc;
	}

	/*needs to modify later*/
	int i;
	for(i=0;i<8;i++){
		cur_process->file_array[i].file_op_ptr=NULL;
		cur_process->file_array[i].inode=0;
		cur_process->file_array[i].file_pos=0;
		cur_process->file_array[i].flag=0;
	}
		
		//cur_process->file_array[0].fop_ptr = (uint32_t)&t_op;
		cur_process->file_array[0].file_op_ptr = &stdin_table;
		cur_process->file_array[0].inode=0;
		cur_process->file_array[0].file_pos=0;
		cur_process->file_array[0].flag=1;
	

		//cur_process->file_array[1].fop_ptr= (uint32_t)&t_op;
		cur_process->file_array[1].file_op_ptr = &stdout_table;
		cur_process->file_array[1].inode=0;
		cur_process->file_array[1].file_pos=0;
		cur_process->file_array[1].flag=1;


		cur_process->pid=current_pid;
		cur_process->esp=esp;
		cur_process->ebp=ebp;

		asm volatile ( 
    			 "leal halt_ret, %%eax		\n 	 	\
        		 movl %%eax, %0" 
      			 :"=m"(cur_process->ret_addr) 
       		 	 : 
      			 :"eax", "memory"  
       			 ); 
		
		cur_process->esp0 = tss.esp0;
		cur_process->ss0 = tss.ss0;

		//content switch
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x00800000-0x2000*(current_pid)-4; 
		//tss.esp0 = 0x7ffe08;
		
	//asm volatile ("cli\n\t": :);  
    asm volatile ("movw  $0x2B, %%ax\n\t": :"g"(USER_DS)); 
    asm volatile ("movw %%ax, %%ds\n\t": :);   
    asm volatile ("pushl %0\n\t": :"g"(USER_DS)); 
    asm volatile ("pushl %0\n\t": :"g"(0x083ffffb)); 
    asm volatile ("pushfl\n\t": :); 
  	
    //asm volatile ("popl %%eax\n\t": :);                
    //asm volatile ("movl $0x202, %%eax\n\t": :);          
    //asm volatile ("pushl %%eax\n\t": :);              
    asm volatile ("pushl %0\n\t": :"g"(USER_CS)); 
    asm volatile ("pushl %0\n\t": :"g"(eip_entry));   
                                       
    asm volatile ("iret\n\t"::); 
    asm volatile ("halt_ret:	\n\
    				sti 		\n\
    				leave		\n\
    				ret 		\n\
    				"); 
    asm volatile ("sti\n\t": :);     


	return 0; 
}

int32_t sys_open(const uint8_t* filename)
{
	pcb_t * cur_process = (pcb_t*)(0x00800000-0x2000*(current_pid+1));

	//find an empty fd
	file_element_t* file_array = cur_process->file_array;

	if((uint32_t)(*filename) == 0)
		return 0;

	if((uint32_t)(*filename) == 1)
		return 1;

	int32_t fd = 0;
	while(file_array[fd].flag == 1)
		fd++;

	dentry_t dentry;
	if(fd == 7 || read_dentry_by_name(filename, &dentry) == -1)
		return -1;

	file_element_t* curr_file = &(file_array[fd]);

	//case RTC:
	if(dentry.file_type == 0){
		if(open_rtc(filename) == - 1)
			return -1;
		curr_file->file_op_ptr = &rtc_table;
		curr_file->inode = 0;
		curr_file->file_pos = 0;
		curr_file->flag = 1;
	}

	//case directory:
	if(dentry.file_type == 1){
		if(dir_open(filename) == -1)
			return -1;
		curr_file->file_op_ptr = &file_table;
		curr_file->inode = dentry.inode_idx;
		curr_file->file_pos = 0;
		curr_file->flag = 1;
	}
	
	//case regular file:
	if(dentry.file_type == 2){
		if(file_open(filename) == -1)
			return -1;	
		curr_file->file_op_ptr = &dir_table;
		curr_file->inode = 0;
		curr_file->file_pos = 0;
		curr_file->flag = 1;
	}
	return fd;
}

int32_t sys_close(int32_t fd)
{
	pcb_t * cur_process = (pcb_t*)(0x00800000-0x2000*(current_pid+1));

	if(fd < 2 || fd > 7)
		return -1;

	file_element_t* curr_file = &(cur_process->file_array[fd]);
	if(curr_file->flag == 0)
		return -1;

	curr_file->flag = 0;
	curr_file->inode = 0;
	curr_file->file_pos = 0;
	return SUCCESS;
}

int32_t sys_read(int32_t fd, void* buf, int32_t count)
{
	pcb_t * cur_process = (pcb_t*)(0x00800000-0x2000*(current_pid+1));

	if(fd <0 || fd > 7 || buf == NULL || count < 0)
		return -1;

	file_element_t* curr_file = &(cur_process->file_array[fd]);
	if(curr_file->flag == 0 || curr_file->file_op_ptr->read == NULL)
		return -1;
	//sti();
	return curr_file->file_op_ptr->read(fd, buf, count);
}

int32_t sys_write(int32_t fd, const void* buf, int32_t count)
{
	pcb_t * cur_process = (pcb_t*)(0x00800000-0x2000*(current_pid+1));

	if(fd <0 || fd > 7 || buf == NULL || count < 0)
		return -1;

	file_element_t* curr_file = &(cur_process->file_array[fd]);
	if(curr_file->flag == 0 || curr_file->file_op_ptr->write == NULL)
		return -1;

	return curr_file->file_op_ptr->write(fd, buf, count);
}


int32_t sys_halt(uint8_t status) {

	pcb_t * shell_pcb = (pcb_t *)(EIGHT_MB - (current_pid+1) * EIGHT_KB);
	pcb_t * cur = (pcb_t *)(EIGHT_MB - (current_pid) * EIGHT_KB);
	// do I set current process to parent process?
	// cur_process is local var
	//cur_process = shell_pcb;

	// do I worry about runqueue? -- Q9

	// reload CR3 to flush TLB
	recoverPaging();

	// restore tss.ss0, tss.esp0    -- Q3
	tss.ss0 = shell_pcb->ss0;
	tss.esp0 = shell_pcb->esp0;

	int i;
	for(i = 2; i < 8; i++)
		cur->file_array[i].flag = 0;

	current_pid--;
	pidlist[0].flag=0;

	// if exception return 256, otherwise 0, but the return val in eax  -- Q6
      
    // switch esp and ebp to paren kernel stack
	// jump to the end of execute(), after the IRET -- Q7
	// 
	// do I do ' "esp","ebp" '
    asm volatile(
                 "movl %0, %%esp		\n\
                 movl %1, %%ebp			\n\
                 movl %3, %%eax          \n\
                 jmp *%2				\n\
                 "
                 :
                 :"r"(shell_pcb->esp), "r"(shell_pcb->ebp), "r" (shell_pcb->ret_addr), "r" ((uint32_t)status) 
    );

    return 0;

/*	//restore the esp0 and ss0
	pcb_t * cur = (pcb_t *) (EIGHT_MB - EIGHT_KB*(terminal_pid[cur_index]+1));
	tss.esp0 = cur->esp0;
	tss.ss0 = cur->ss0;
	int pid = (int)cur->pid;

	//check whether it is first process
	int i;
	for(i = 2; i < 8; i++)
		cur->file_array[i].flag = 0;

	pid_status[pid] = 0;
	pid = cur->parent;
	terminal_pid[cur_index] = pid;
	//printf("halt2:%d\n", pid);
	//printf("halt cur: %x, cur_pid: %x\n", (int)cur, cur->PID);
	//printf("halt esp: %x\n", cur->esp);
	//printf("halt ebp: %x\n", cur->ebp);
	if(pid == -1){
		execute((uint8_t *) "shell");
	}
	//reset the paging
	flush();

	// printf("exit to parent, now pid: %d\n", pid);
	
	//jump to parent process
	asm volatile (
        		"movl %0, %%esp			\n\
        		movl %1, %%ebp			\n\
        		jmp *%2"
       			 :
        		 :"r"(cur->esp), "r"(cur->ebp), "r" (cur->return_add)
       			 );

	return 0;*/
}
int32_t sys_getargs(uint8_t * buf, int32_t nbytes) {return 0;}
int32_t sys_vidmap(uint8_t ** screen_start) {return 0;}
