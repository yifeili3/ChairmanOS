
#define EIGHT_MB 0x0800000
#define EIGHT_KB 0x2000

/* Description: terminates a process, return the specified value
 * 				to the parent process. expands 8-bit argument from
 * 				BL into 32-bit return val to parent program's 
 *				execute system call. never returns to the caller.
 *				switch process back to shell, resets paging.
 * Input:	uint8_t status
 * Output:	
 * Return:	0 - normal return, 256 if exception
 * Side effect: flushes TLB
 */
int32_t halt (uint8_t status){

	/*Question: 
		1. how does pid determine the kernel stack location in kernel
	
			A: counting upwards

		2. if the page for user is 4MB down the memory space, what do 
			we reset CR3 to? Do we create a new page? becase right 
			now we only have one Page directory, the one we use in init

			A: find the PD pointer in PCB, for separate page directory,
			if using only one, just flush TLB by replacing with same val
		
        3. how is TSS in our code defined?
     
        4. how should I get the cur_pid we use in finding the shell?
     
        5. pid == -1 case?
     
        6. how do we determine if there is an exception? status?
     
        7. do we movl $0, %%eax?
     
	*/


	// determine current process
	// need shell always 8kb below the current process  -- Q4
	pcb_t * shell_pcb = (pcb_t *)(EIGHT_MB - (current_pid+1) * EIGHT_KB);


	// reload CR3 to flush TLB
    flush();


	// restore tss.ss0, tss.esp0    -- Q3
	tss.ss0 = shell_pcb->ss0;
	tss.esp0 = shell_pcb->esp0;
    
    // do we need this?             -- Q5
/*    if(shell_pcb->parent == -1){
        execute((unsigned char)"shell");
    }*/

	// if exception return 256, otherwise 0, but the return val in eax  -- Q6
    
    
    // switch esp and ebp to paren kernel stack
	// jump to the end of execute(), after the IRET -- Q7
	// movl $0, %%eax          \n\
    asm volatile(
                 "movl %0, %%esp			\n\
                 movl %1, %%ebp			\n\
                 jmp *%2"
                 :
                 :"r"(shell_pcb->esp), "r"(shell_pcb->ebp), "r" (shell_pcb->ret_addr)
    );

    return 0;
}