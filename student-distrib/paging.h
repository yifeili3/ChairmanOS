/* paging.h by S.W
 * Header file for paging
 */
#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

// Paging Initialization function
void pagingInit();

// Paging enable function
void enablePaging(unsigned int pageDirAddr);

// Flush function to reset TLB
void flush();

// Allocate page and clear page
int32_t allocatePage(uint32_t page_idx, uint32_t type, uint32_t phys_addr);

/* Clear a present page to 0 for page swap or clear */
int32_t clearPage(uint32_t page_idx);

/* remap video memeory for video map */
void remap_video_memory(uint8_t new_terminal);

/* remap page table entry for video map */
void vidmem_alloc(uint32_t idx, uint32_t phys_addr);
#endif
