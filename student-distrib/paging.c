// intel manual, in the entries of page table(0-4MB is video) and page directories(4MB kernel code )
// CR3, CR0 regs, and more
// set the first 8 MBs, and set the rest to not present with lowest bit set to 1


// how and where to debug?
// test by dereferencing a NULL pointer
#include "paging.h"
#include "lib.h"
#include "magic.h"
#include "kmalloc.h"

#define FOURMB		0x00400000
#define ONEMB		0x00100000
#define FOURKB		0x00001000
#define fourKB 4096
#define oneKB 1024
#define videoPhysMem 0xB8000
#define videoTableEntry 0xB8
#define r_w_p 	0x3 
#define u_r_w_p 0x7 
#define KERNEL_ENTRY 0x00400083
#define PDMASK_R 	22
#define PDMASK_L 	10
#define VIDEO_PAGE_DIR 33
#define SHIFT 12
#define twentyBits 20
#define NUM_PROCESS 6
#define TYPE4MB 1
#define STARTKMALLOC 200

void allocate_kmalloc_mem();

typedef union PDE_t{
	unsigned int value;
	struct {
		unsigned int Present:			1; //
		unsigned int ReadWrite:			1; //
		unsigned int UserAccess:		1; //
		unsigned int WriteThroughCache:	1;
		unsigned int DisableCache:		1;
		unsigned int Accessed:			1;
		unsigned int Zero:				1;
		unsigned int SPageSize:			1; //
		unsigned int GIgnored:			1;
		unsigned int avail: 			3;
		unsigned int pageTablePhysAddr: twentyBits; //
	} __attribute__((packed));
} PDE_t;

typedef union PTE_t{
	unsigned int value;
	struct {
		unsigned int Present:			1; //
		unsigned int ReadWrite:			1; //
		unsigned int UserAccess:		1; //
		unsigned int WriteThroughCache:	1;
		unsigned int CacheDisabled:		1;
		unsigned int Accessed:			1;		
		unsigned int Dirty:				1;
		unsigned int Zero:				1;
		unsigned int Global:			1;
		unsigned int avail: 			3;
		unsigned int pageTablePhysAddr: twentyBits;
	} __attribute__((packed));
} PTE_t;

PDE_t pageDir[oneKB] __attribute__((aligned(fourKB)));
PTE_t zeroToFourMBTable[oneKB] __attribute__((aligned(fourKB)));
PTE_t videoMem_4kb_Page[oneKB] __attribute__((aligned(fourKB)));

/*
 * pagingInit
 *	DESCRIPTION: Initialize paging by setting up page directory according to Intel manual
 *	INPUTS: none
 * 	OUTPUTS: none
 *	RETURN VALUES: none
 */
void pagingInit(){
	//set each entry to not present
	int i;
	for(i = 0; i < oneKB; i++)
	{
	    // kernel only, read/write, non-present
	    pageDir[i].value = 0;
	}

	for(i = 0; i < oneKB; i++)
	{
	    // lowest 12 bits are 0 for addr, while they are actually atts
	    // supervisor level, read/write, present
	    zeroToFourMBTable[i].value = (i * fourKB) | r_w_p; //1000 is 4k
	}
	zeroToFourMBTable[0].value = 0;
	pageDir[0].value = ((unsigned int)zeroToFourMBTable) | r_w_p;
	pageDir[1].value = KERNEL_ENTRY; // kernel code at 4MB
	// after padding with 0, the final number must match 4MB

	// what to put in video entry?
	// find the actual video mem addr, 
	// put the phys mem value into the entry in the page table
	zeroToFourMBTable[videoTableEntry].value = videoPhysMem | u_r_w_p;
	zeroToFourMBTable[videoTableEntry + 2].value = VIDEO_MEM0 | u_r_w_p;
	zeroToFourMBTable[videoTableEntry + 4].value = VIDEO_MEM1 | u_r_w_p;
	zeroToFourMBTable[videoTableEntry + 6].value = VIDEO_MEM2 | u_r_w_p;

	// allocate vid memory for user space in PDE
	pageDir[VIDEO_PAGE_DIR].pageTablePhysAddr = (uint32_t)videoMem_4kb_Page>>SHIFT;	
	pageDir[VIDEO_PAGE_DIR].Present = 1;						
	pageDir[VIDEO_PAGE_DIR].ReadWrite = 1;
	pageDir[VIDEO_PAGE_DIR].UserAccess = 1;			
	pageDir[VIDEO_PAGE_DIR].SPageSize = 0;

	// set up vid memory for user space in PTE
	videoMem_4kb_Page[VIDEO_IDX0].pageTablePhysAddr = 0xB8;
	videoMem_4kb_Page[VIDEO_IDX0].Present = 1;				 		
	videoMem_4kb_Page[VIDEO_IDX0].ReadWrite = 1;
	videoMem_4kb_Page[VIDEO_IDX0].UserAccess = 1;
	videoMem_4kb_Page[VIDEO_IDX1].pageTablePhysAddr = VIDEO_IDX1;
	videoMem_4kb_Page[VIDEO_IDX1].Present = 1;				 		
	videoMem_4kb_Page[VIDEO_IDX1].ReadWrite = 1;
	videoMem_4kb_Page[VIDEO_IDX1].UserAccess = 1;
	videoMem_4kb_Page[VIDEO_IDX2].pageTablePhysAddr = VIDEO_IDX2;
	videoMem_4kb_Page[VIDEO_IDX2].Present = 1;				 		
	videoMem_4kb_Page[VIDEO_IDX2].ReadWrite = 1;
	videoMem_4kb_Page[VIDEO_IDX2].UserAccess = 1;

	allocate_kmalloc_mem();

	enablePaging((unsigned int)pageDir);
}

/*
 * allocate_kmalloc_mem
 *	DESCRIPTION: reserve pages for kmalloc to use
 *				 reserved page : 200MB - 240MB
 *	INPUTS: none
 *	OUTPUS: none
 *	RETURN VALUES: none
 *	SIDE EFFECTS: reserves 40MB for kmalloc
 */
void allocate_kmalloc_mem(){
	int i=0;
	// allocate 10 4MB pages
	for(i=0; i<10; i++){
		pageDir[50 + i].Present = 1;
		pageDir[50 + i].ReadWrite = 1;
		pageDir[50 + i].UserAccess = 1;
		pageDir[50 + i].pageTablePhysAddr = ((STARTKMALLOC*ONEMB + i*FOURMB) >> PDMASK_R) << PDMASK_L;
		pageDir[50 + i].SPageSize = TYPE4MB;	
	}

}


/*
 * enablePaging
 *	DESCRIPTION: Assembly code to enable paging by setting registers
 *	INPUTS: @pageDirAddr, base address of page directory
 *	OUTPUS: none
 *	RETURN VALUES: none
 *	SIDE EFFECTS: flush TLB
 */
void enablePaging(unsigned int pageDirAddr){
		// set CR3 = page_dir addr, set 32 bit of CR0 paging bit
		// set PSE bit (4th bit of CR0)
	asm volatile(" 								\n\
				movl %0, %%cr3		\n\
	     		movl %%cr4, %%ecx		\n\
				orl $0x00000010, %%ecx	\n\
				movl %%ecx, %%cr4		\n\
	     		movl %%cr0, %%ecx		\n\
				orl $0x80000000, %%ecx	\n\
				movl %%ecx, %%cr0		\n\
		"
		:
		:"r"(pageDirAddr)
		:"ecx");	/* %ecx is clobbered register */
}

/*
 * flush
 *	DESCRIPTION: explicit function for TLB flushing
 *	INPUTS: none
 * 	OUTPUTS: none
 *	RETURN VALUES: none
 */
void flush() {
	asm volatile (
		"movl %%cr3, %%ebx			\n\
		 movl %%ebx, %%cr3 			\n\
		 "
		 : :
		 : "ebx"
	);
}

/*
 * allocatePage
 *	DESCRIPTION: Allocate a non-present page to specified type and addr
 *	INPUTS: @page_idx, the idx of page to be allocated; 
 *			@type, size of page 0->4KB, 1->4MB
 *			@phys_addr, the physical address of page
 *	OUTPUTS: none
 *	RETURN VALUES: -1 if page already existed, 0 if success
 * 	SIDE EFFECTS: flush TLB
 */
int32_t allocatePage(uint32_t page_idx, uint32_t type, uint32_t phys_addr) {
	pageDir[page_idx].Present = 1;
	pageDir[page_idx].ReadWrite = 1;
	pageDir[page_idx].UserAccess = 1;
	pageDir[page_idx].pageTablePhysAddr = (phys_addr >> PDMASK_R) << PDMASK_L;
	pageDir[page_idx].SPageSize = type;
	flush();
	return 0;
}

/*
 *  int32_t clearPage(uint32_t page_idx)
 *	DESCRIPTION: Clear a present page to 0 for page swap or clear
 *	INPUTS: @page_idx, the idx of page to be cleared
 *	OUTPUTS: none
 *	RETURN VALUES: 0
 */
int32_t clearPage(uint32_t page_idx) {
	pageDir[page_idx].value = 0;
	flush();
	return 0;
}

/*
 *  void remap_video_memory(uint8_t new_terminal)
 *	DESCRIPTION: remap video memeory for video map
 *	INPUTS: the new terminal to be displayed
 *	OUTPUTS: NONE
 *	RETURN VALUES: NONE
 */
void remap_video_memory(uint8_t new_terminal){
	zeroToFourMBTable[videoTableEntry + new_terminal * 2].value = (0xB8000 + 2 * FOURKB * new_terminal) | u_r_w_p;
	flush();
}

/*
 *  void vidmem_alloc(uint32_t idx, uint32_t phys_addr)
 *	DESCRIPTION: remap page table entry for video map
 *	INPUTS: page table index and its corresponding physical address
 *	OUTPUTS: NONE
 *	RETURN VALUES: NONE
 */
void vidmem_alloc(uint32_t idx, uint32_t phys_addr) {
	videoMem_4kb_Page[idx].value = phys_addr | u_r_w_p;
	flush();
}
