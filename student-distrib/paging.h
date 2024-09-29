#ifndef _PAGING_H
#define _PAGING_H
#include "types.h"

/* Referenced code from OSdev {https://wiki.osdev.org/Paging}, modified to fit our OS */

/* Enable paging */
#define page_enable()                   \
do {                                    \
    asm volatile ("					  \n\
    		mov %%cr0, %%eax          \n\
    		or	$0x80000001, %%eax	  \n\
    		mov	%%eax, %%cr0          \n\
    		"						    \
            : /* no outputs */          \
            : /* no inputs */           \
            : "eax", "memory", "cc"     \
    );                                  \
} while (0)


/* Enable page size extension to support 4Mb paging for the kernel page */
#define page_enable_size_extend()       \
do {                                    \
    asm volatile ("                   \n\
            mov %%cr4, %%eax          \n\
            or  $0x10, %%eax          \n\
            mov %%eax, %%cr4          \n\
            "                           \
            : /* no outputs */          \
            : /* no inputs */           \
            : "eax", "memory", "cc"     \
    );                                  \
} while (0)

/*Gets the address of the page_dir*/
#define page_access()                   \
do {                                    \
    asm volatile ("                   \n\
            mov %0, %%eax             \n\
            mov %%eax, %%cr3          \n\
            "                           \
            : /* no outputs */          \
            : "r"((page_dir))           \
            : "eax", "memory", "cc"     \
    );                                  \
} while (0)

/* Loads the page directory into cr3 */
void load_page_directory(); 

/* Initializes the page directory */
void init_page_directory();

/* Initializes the page table*/
void init_page_table();

/* Map the important indices */
void init_page_map();

/* Enable paging for the OS */
void setup_paging();

/* Maps a virtual address to a physical address */
void map_page(void * physaddr, void * virtualaddr);

void map_vid(void * physaddr /*, void * virtualaddr */);

/* Gets the page for the vidmap system call */
uint32_t get_vidmap_page(int terminal);

/* Helper function to flush tlb */
void flush_tlb_helper();

#endif	
