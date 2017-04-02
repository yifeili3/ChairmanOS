#include "syscall.h"

#define SUCCESS 1
#define EXE_ID 0x464c457f
#define OFFSET 0x00048000
#define USER_ESP 0x083ffffb
#define NOT_IN_USE 0
#define IN_USE 1
#define MAX_FD 7
#define MIN_FD 0
#define RTC_FILETYPE 0
#define DIR_FILETYPE 1
#define FILE_FILETYPE 2
#define VIRTUAL_MEM 0x20
#define ADDR_START 24
#define ADDR_SIZE 	4
#define INPUT_SIZE 128
#define PROG_IMAGE 0x08000000
#define VIDEO_VIRT_ADDR 0x084b8000
#define FISH_VIRT0 0x084ba000
#define FISH_VIRT1 0x084bc000
#define FISH_VIRT2 0x084be000
#define seven 7
#define two 2
#define num8 8
#define PAGE_SIZE_FOURMB 1

// the file struct for system call jump table
syscall_jump_table rtc_table = {open_rtc, read_rtc, write_rtc, close_rtc};
syscall_jump_table file_table = {file_open, file_read, file_write, file_close};
syscall_jump_table dir_table = {dir_open, dir_read, dir_write, dir_close};
syscall_jump_table stdin_table = {NULL, terminal_read, NULL, NULL};
syscall_jump_table stdout_table = {NULL, NULL, terminal_write, NULL};
void init_pid() {
	int i;
	for (i = 0; i < 6; i++) {
		pidlist[i].pid = i;
		pidlist[i].proc = 0;
		pidlist[i].flag = 0;
	}

	scheduled_terminal = 0;
}
/* int32_t sys_execute(const uint8_t* command)
 * INPUT： uint8_t command : command to be typed in shell
 * OUTPUT: 0 for successfully run, -1 for failure
 * FUNCTION: execute program which user typed in.
 */
int32_t sys_execute(const uint8_t * command){
	uint8_t program[INPUT_SIZE];
	uint8_t arg[INPUT_SIZE];
	uint32_t cmd_i = 0;
	uint32_t idx = 0;
	/* fetch current esp and ebp and will store them in pcb */
	uint32_t esp;
	uint32_t ebp;
	asm volatile ("movl %%ebp, %0\n\t":"=r"(ebp):);

	asm volatile (
			    "movl %%esp, %0\n\t"
				:"=r"(esp)
				:);
	/* Check if command is valid */ 

	if (command[0] =='\0'|| command[0] ==' ') {
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
	while(command[cmd_i] == ' ')
		cmd_i++;

	while(command[cmd_i] != '\0' && command[cmd_i] != '\n'){
		arg[idx] = command[cmd_i];
		idx++;
		cmd_i++;
	}
	arg[idx]='\0';

	// /printf("%s %s\n", program, arg);

	/* Check if file exists */
	dentry_t temp;
	if (read_dentry_by_name(program,&temp)==-1){
		//puts("File does not exist.\n");
		return -1;
	}
	/* Check if file is executable */
	uint8_t Magic[ADDR_SIZE];
	if (read_data(temp.inode_idx, 0, Magic, ADDR_SIZE) == -1) {
		puts("Failed to read file.\n");
		return -1;
	}
	if ((*((uint32_t*)Magic)) != EXE_ID) {
		puts("Not an executable.\n");
		return -1;
	}
	/* determine physical address for current_pid */
	uint32_t phys_addr;
	uint8_t i = 0;
	int8_t temp_parent_pid;
	if(get_current_flag() == 1){
		set_current_flag(0);
		i = scheduled_terminal;
		//i = get_disp_terminal();
		pidlist[i].flag = 1;
		current_pid = i;
		phys_addr = EIGHTMB + i * FOURMB;
		temp_parent_pid = scheduled_terminal;
		//temp_parent_pid = get_disp_terminal();
	}
	else{
		i = 3;
		while (1) {
			if (i >= 6) {
				puts("Process list full.\n");
				return 0;
			}
			if (pidlist[i].flag == 1)
				i++;
			else
			{
				pidlist[i].flag = 1;
				current_pid = i;
				phys_addr = EIGHTMB + i * FOURMB;
				break;
			}	
		}
		temp_parent_pid = terminals[get_disp_terminal()].shell_pid;
		//printf("%d\n", terminals[get_disp_terminal()].shell_pid);
		//printf("%d\n", i);
		modify_terminal_curr_pid(i);
	}

	/* Need to add page swap for multiple programs */
	if(allocatePage(VIRTUAL_MEM, PAGE_SIZE_FOURMB, phys_addr) == -1) {
		//puts("Page allocated, need to swap.\n");
		clearPage(VIRTUAL_MEM);
		allocatePage(VIRTUAL_MEM, PAGE_SIZE_FOURMB, phys_addr);
	}

	uint8_t entry[ADDR_SIZE];
	/* read starting point of instruction */
	read_data(temp.inode_idx, ADDR_START, entry, ADDR_SIZE);
	/* load program img in to 128MB*/
	read_data(temp.inode_idx, 0, (uint8_t*)PROGRAMIMG, FOURMB-OFFSET);

	uint32_t eip_entry;
	eip_entry = *((uint32_t*)entry);
	/* allocate pcb_t pointer for current process*/
	pcb_t * cur_process = (pcb_t*)(EIGHTMB-EIGHTKB*(current_pid+1));
	
	// get args
	cur_process->arg = arg;

	/*set up PID_t struct*/
	pidlist[i].proc = cur_process;
	if(temp_parent_pid == 0){
		cur_process->parent_pid = 0;
	}
	else if(temp_parent_pid == 1){
		cur_process->parent_pid = 1;
	}
	else if(temp_parent_pid == 2){
		cur_process->parent_pid = 2;
	}
	else{
		cur_process->parent_pid = temp_parent_pid;
	}

	/* set up file array in pcb_t */
	for(i=0;i<num8;i++){
		cur_process->file_array[i].file_op_ptr=NULL;
		cur_process->file_array[i].inode=0;
		cur_process->file_array[i].file_pos=0;
		cur_process->file_array[i].flag=NOT_IN_USE;
	}
	/* stdin */
	cur_process->file_array[0].file_op_ptr = &stdin_table;
	cur_process->file_array[0].inode=0;
	cur_process->file_array[0].file_pos=0;
	cur_process->file_array[0].flag=IN_USE;

	/* stdout */
	cur_process->file_array[1].file_op_ptr = &stdout_table;
	cur_process->file_array[1].inode=0;
	cur_process->file_array[1].file_pos=0;
	cur_process->file_array[1].flag=IN_USE;

	/* store context info */
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

	/* content switch */
	tss.ss0 = KERNEL_DS;
	tss.esp0 = EIGHTMB-EIGHTKB*(current_pid)-4; 
    /* push necessary information for iret */
    asm volatile ("movw  %0, %%ax\n\t": :"g"(USER_DS)); 
    asm volatile ("movw %%ax, %%ds\n\t": :);   
    asm volatile ("pushl %0\n\t": :"g"(USER_DS)); 
    asm volatile ("pushl %0\n\t": :"g"(USER_ESP)); 
    asm volatile ("pushfl\n\t": :); 

    asm volatile ("pushl %0\n\t": :"g"(USER_CS)); 
    asm volatile ("pushl %0\n\t": :"g"(eip_entry));   
                                       
    asm volatile ("iret\n\t"::);
    /* where sys_halt return to */ 
    asm volatile ("halt_ret:"); 
    asm volatile ("sti\n\t": :);     

	return 0; 
}

/* int32_t sys_open(const uint8_t* filename)
 * INPUT： the name of the file to open
 * OUTPUT: the file descripter associated with this file
 * FUNCTION: open the given file by assigning the file descriptor in the processor
 */
int32_t sys_open(const uint8_t* filename)
{
	// get the current process
	pcb_t * cur_process = (pcb_t*)(EIGHTMB-EIGHTKB*(current_pid+1));

	//find an empty fd
	file_element_t* file_array = cur_process->file_array;

	int32_t fd = 0;
	while(file_array[fd].flag == 1) {
		if (fd++ == seven)	return -1;
	}

	// read the input file
	dentry_t dentry;
	if(read_dentry_by_name(filename, &dentry) == -1)
		return -1;

	file_element_t* curr_file = &(file_array[fd]);

	//case RTC:
	if(dentry.file_type == RTC_FILETYPE){
		if(open_rtc(filename) == - 1)
			return -1;
		curr_file->file_op_ptr = &rtc_table;
		curr_file->inode = 0;
		curr_file->file_pos = 0;
		curr_file->flag = IN_USE;
	}

	//case directory:
	else if(dentry.file_type == DIR_FILETYPE){
		if(dir_open(filename) == -1)
			return -1;
		curr_file->file_op_ptr = &dir_table;
		curr_file->inode = 0;
		curr_file->file_pos = 0;
		curr_file->flag = IN_USE;
	}
	
	//case regular file:
	else if(dentry.file_type == FILE_FILETYPE){
		if(file_open(filename) == -1)
			return -1;	
		curr_file->file_op_ptr = &file_table;
		curr_file->inode = dentry.inode_idx;
		curr_file->file_pos = 0;
		curr_file->flag = IN_USE;
	}
	return fd;
}

/* int32_t sys_close(int32_t fd)
 * INPUT： tthe file descriptor to close
 * OUTPUT: SUCCESS
 * FUNCTION: open the given file by assigning the file descriptor in the processor
 */
int32_t sys_close(int32_t fd)
{
	// get the current process
	pcb_t * cur_process = (pcb_t*)(EIGHTMB-EIGHTKB*(current_pid+1));

	// sanity check for invalid fd
	if((fd < MIN_FD + two) || fd > MAX_FD)
		return -1;

	file_element_t* curr_file = &(cur_process->file_array[fd]);
	if(curr_file->flag == NOT_IN_USE)
		return -1;

	// clear the variable
	curr_file->flag = NOT_IN_USE;
	curr_file->inode = 0;
	curr_file->file_pos = 0;
	return SUCCESS;
}

/* int32_t sys_read(int32_t fd, void* buf, int32_t count)
 * INPUT： the fd to identify the file to manipulate, the input parameter for read function, and bytes to read
 * OUTPUT: SUCCESS
 * FUNCTION: do the read operation of file of the current process specified by the input fd 
 */
int32_t sys_read(int32_t fd, void* buf, int32_t count)
{
	// get current process
	pcb_t * cur_process = (pcb_t*)(EIGHTMB-EIGHTKB*(current_pid+1));

	// sanity check for invalid fd and not read operation
	if(fd < MIN_FD || fd > MAX_FD || buf == NULL || count < 0)
		return -1;

	file_element_t* curr_file = &(cur_process->file_array[fd]);
	if(curr_file->flag == NOT_IN_USE || curr_file->file_op_ptr->read == NULL)
		return -1;

	// call the specified read operation
	return curr_file->file_op_ptr->read(fd, buf, count);
}

/* int32_t sys_write(int32_t fd, const void* buf, int32_t count)
 * INPUT： the fd to identify the file to manipulate, the input parameter for write function, and bytes to read
 * OUTPUT: SUCCESS
 * FUNCTION: do the write operation of file of the current process specified by the input fd 
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t count)
{
	// get current process
	pcb_t * cur_process = (pcb_t*)(EIGHTMB-EIGHTKB*(current_pid+1));

	// sanity check for invalid fd and not read operation
	if(fd < MIN_FD || fd > MAX_FD || buf == NULL || count < 0)
		return -1;

	file_element_t* curr_file = &(cur_process->file_array[fd]);
	if(curr_file->flag == NOT_IN_USE || curr_file->file_op_ptr->write == NULL)
		return -1;

	// call the specified write operation
	return curr_file->file_op_ptr->write(fd, buf, count);
	//return (&(((pcb_t*)(EIGHTMB-EIGHTKB*(current_pid+1)))->file_array[fd]))->file_op_ptr->write(fd, buf, count);
}

/* int32_t sys_halt(uint8_t status)
 * INPUT： the status of current running process
 * OUTPUT: 0 for success, -1 for failure
 * FUNCTION: halt the current process
 */
int32_t sys_halt(uint8_t status) {
	//get pcb of current running process
	pcb_t* cur_proc = (pcb_t*)(EIGHTMB - EIGHTKB * (current_pid + 1));
	int32_t fd = MIN_FD + two;
	while(fd <= MAX_FD) {
		sys_close(fd++);
	}
	//calculate physical address based on pid
	uint32_t phys_addr = EIGHTMB + cur_proc->parent_pid * FOURMB;
	//restore tss ss0 & esp0 using info in pcb_t
	tss.ss0 = cur_proc->ss0;
	tss.esp0 = cur_proc->esp0;
	//decrement pid and reallocate page for parent process
	pidlist[current_pid].flag = 0;
	recover_terminal_curr_pid(current_pid, cur_proc->parent_pid);

	if (current_pid != 0 && current_pid != 1 && current_pid != 2) {	
		current_pid = cur_proc->parent_pid;
		//printf("%d\n", current_pid); 
	}
	else{
		set_current_flag(1);
		sys_execute((uint8_t*)"shell");
	} 
 	if(allocatePage(VIRTUAL_MEM, 1, phys_addr) == -1) {
		//puts("Page allocated, need to swap.\n");
		clearPage(VIRTUAL_MEM);
		allocatePage(VIRTUAL_MEM, 1, phys_addr);
	}
	//restore esp and ebp using info in pcb_t
	asm volatile(
		"movl %0, %%ebp \n\t"
		"movl %1, %%esp \n\t"
		:
		:"r"(cur_proc->ebp), "r"(cur_proc->esp)
		);
	//jump to halt return label
	asm volatile(
		"jmp halt_ret \n\t"
		: :);
	//should never reach here
	return 0;
}

/* int32_t sys_getargs(uint8_t * buf, int32_t nbytes)
 * DESCRIPTION: get the argument for process to be run
 * INPUT： 	buf - store args in this buffer
 * 			nbytes - 
 * OUTPUT: 0 for success, -1 for failure
 * FUNCTION: get argument typed in shell program
 */
int32_t sys_getargs(uint8_t * buf, int32_t nbytes) {
	pcb_t* cur_proc = (pcb_t*)(EIGHTMB - EIGHTKB * (current_pid + 1));
	
	//printf("get args %s\n", cur_proc->arg);

	if((char)(cur_proc->arg[0]) == '\0')
		return -1;

	strncpy((char *)buf, (char *)(cur_proc->arg), nbytes);

	return 0;
}

/* int32_t sys_vidmap(uint8_t ** screen_start)
 * DESCRIPTION: maps the text-mode video memory into user space at a pre-set virtual address
 * INPUT： 	screen_start: the memory to be mapped
 * OUTPUT: 0 for success, -1 for failure
 * FUNCTION: maps the text-mode video memory into user space at a pre-set virtual address
 */
int32_t sys_vidmap(uint8_t ** screen_start) {
	if((uint32_t)screen_start < PROG_IMAGE || (uint32_t)screen_start >= (PROG_IMAGE + FOURMB)){
		//puts("Invalid argument for vidmap!\n");
		return -1;
	}
	switch(scheduled_terminal) {
		case 0:	*screen_start = (uint8_t *)FISH_VIRT0; return 0;
		case 1: *screen_start = (uint8_t *)FISH_VIRT1; return 0;
		case 2: *screen_start = (uint8_t *)FISH_VIRT2; return 0;
		default:	return -1;
	}
}

/* void schedule()
 * DESCRIPTION: determine the next scheduled terminal 
 * INPUT： 	NONE
 * OUTPUT: NONE
 * FUNCTION: determine the next scheduled terminal in pit
 */
void schedule(){
	// do the round-robin rotation
	// and cahnge the scheduled terminal flag
	uint8_t temp = scheduled_terminal;
	scheduled_terminal = (scheduled_terminal + 1)%3;

	// call the context switch
	context_switch(temp, scheduled_terminal);
}

/* void context_switch(uint8_t previous_terminal, uint8_t scheduled_terminal)
 * DESCRIPTION: do the context switch between the current terminal and new terminal
 * INPUT： index for previous scheduled terminal and index for next scheduled terminal
 * OUTPUT: NONE
 * FUNCTION: context switch between the old terminal and new terminal
 */
void context_switch(uint8_t previous_terminal, uint8_t scheduled_terminal){
  // retrieve the previous esp and ebp for old scheduled terminal
  uint32_t cur_esp;
  uint32_t cur_ebp;
  asm volatile (
                "movl %%ebp, %0\n\t"
                :"=g"(terminals[previous_terminal].ebp)
                :
                :"memory");
  
  asm volatile (
                "movl %%esp, %0\n\t"
                :"=g"(terminals[previous_terminal].esp)
                :
                :"memory");

  /* Initialize Unseen Terminal */
  if(terminals[scheduled_terminal].shell_pid == -1){
  	// tells execute there is a terminal running
    set_current_flag(1);

    // set the shell pid
    terminals[scheduled_terminal].shell_pid = scheduled_terminal;
    terminals[scheduled_terminal].init = 1;
    sys_execute((uint8_t*)"shell");
    return;
  }
  /* Switch back to initialized terminal */
  else{ 
  	//retrive the process for the new scheduled terminal, get its physical address, and allocate page
    current_pid = terminals[scheduled_terminal].shell_pid;
    uint32_t phys_addr = EIGHTMB + current_pid * FOURMB;
    allocatePage(0x20, 1, phys_addr);

    /* restore context of Switched-out terminal */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHTMB-EIGHTKB*(current_pid)-4;
    cur_esp = terminals[scheduled_terminal].esp;
    cur_ebp = terminals[scheduled_terminal].ebp;
    
    //put esp and ebp of new process into register 
    asm volatile (
                "movl %0, %%ebp \n\t"
                :
                :"g"(terminals[scheduled_terminal].ebp)
                :"memory"
                );
  
    asm volatile (
                "movl %0, %%esp\n\t"
                :
                :"g"(terminals[scheduled_terminal].esp)
                :"memory"
                );
  }
}
void * sys_kmalloc(unsigned int size)
{
	return kmalloc(size);
}

unsigned int sys_kfree(void * physaddr)
{
	kfree(physaddr);
	return 1;
}
