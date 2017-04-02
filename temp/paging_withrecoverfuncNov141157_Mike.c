// intel manual, in the entries of page table(0-4MB is video) and page directories(4MB kernel code )
// CR3, CR0 regs, and more
// set the first 8 MBs, and set the rest to not present with lowest bit set to 1


// how and where to debug?
// test by dereferencing a NULL pointer
#include "paging.h"
#include "lib.h"

#define fourKB 4096
#define oneKB 1024
#define videoPhysMem 0xB8000
#define videoTableEntry 0xB8

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
		unsigned int pageTablePhysAddr: 20; //
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
		unsigned int pageTablePhysAddr: 20;
	} __attribute__((packed));
} PTE_t;

PDE_t pageDir[oneKB] __attribute__((aligned(fourKB)));
PTE_t zeroToFourMBTable[oneKB] __attribute__((aligned(fourKB)));

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
	    zeroToFourMBTable[i].value = (i * 0x00001000) | 0x03; //1000 is 4k
	}
	zeroToFourMBTable[0].value = 0;
	pageDir[0].value = ((unsigned int)zeroToFourMBTable) | 0x03;
	pageDir[1].value = 0x00400083; // kernel code at 4MB
	// after padding with 0, the final number must match 4MB

	// what to put in video entry?
	// find the actual video mem addr, 
	// put the phys mem value into the entry in the page table
	zeroToFourMBTable[videoTableEntry].value = videoPhysMem | 0x03;

	enablePaging((unsigned int)pageDir);
}

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

void flush() {
	asm volatile (
		"movl %%cr3, %%ebx			\n\
		 movl %%ebx, %%cr3 			\n\
		 "
		 : :
		 : "ebx"
	);
}

/* @type: 0 for 4KB, 1 for 4MB */
uint32_t allocatePage(uint32_t page_idx, uint32_t type, uint32_t phys_addr) {
	if (pageDir[page_idx].Present == 1) {
		puts("Page already existed.\n");
		return -1;
	}
	else {
		pageDir[page_idx].Present = 1;
		pageDir[page_idx].ReadWrite = 1;
		pageDir[page_idx].UserAccess = 1;
		pageDir[page_idx].pageTablePhysAddr = (phys_addr >> 22) << 10;
		pageDir[page_idx].SPageSize = type;
		flush();
		return 0;
	}
	/* This should never be reached. */
	puts("Failed to allocated.\n");
	return -1;
}

uint32_t clearPage(uint32_t page_idx) {
	pageDir[page_idx].value = 0;
	flush();
	return 0;
}

void recoverPaging(){
	pageDir[0x20].pageTablePhysAddr = (0x0800000 >> 22) << 10;
}