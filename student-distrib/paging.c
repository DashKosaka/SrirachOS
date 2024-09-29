
#include "paging.h"
#include "types.h"

#define DIR_SIZE	1024
#define TABLE_SIZE	1024
#define ALLIGN_SIZE	4096
#define PDE_VAL 0x00000002
#define UNIT_4_KB 0x1000
#define UNIT_4_MB 0x400000
#define VIDEO_MEM_ADDR 0xB8000
#define RWPRESENT	0x03
#define PRESENT_4_MB	0x083

#define PAGE_TAB_IDX 0
#define KERNEL_IDX	1
#define VID_MEM_IDX	184
#define ENABLE_RW 2
#define SET_PRESENT 1
#define U_R_P		0x07

#define PAGE_MAP 	0x87

#define NEW_VID_IDX 34
#define NEW_VIRTUAL	UNIT_4_MB * NEW_VID_IDX

#define NEW_PHYSICAL VIDEO_MEM_ADDR

/* Create array to hold pointers to page_tables*/
static uint32_t page_dir[DIR_SIZE] __attribute__ ((aligned (ALLIGN_SIZE)));

/* Create array to hold pointers to pages */
static uint32_t page_table[TABLE_SIZE] __attribute__((aligned (ALLIGN_SIZE)));

/*	init_page_directory
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: Creates the page directory that will hold pointers to page tables.
 */
void init_page_directory() {
	int i;
	for (i = 0; i < DIR_SIZE; i++) {
		// Initialize each entry
		page_dir[i] = PDE_VAL; // other entries present set to 0.
	}
}

/*	init_page_table
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: Creates the page table that will hold pointers to pages.
 */
void init_page_table() {
	int i;
	for (i = 0; i < TABLE_SIZE; i++) {
		// Initialize each entry at a 4kB paging size except for the kernel which has page at 4MB
		page_table[i] = (i*UNIT_4_KB)|ENABLE_RW;
	}
}

/*	init_page_map
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: Initialize starting page directory and fill page table.
 */
void init_page_map() {
	// Map the page table to index 0
	page_dir[PAGE_TAB_IDX] = ((uint32_t)page_table) | RWPRESENT;

	// Map the kernel to index 1
	page_dir[KERNEL_IDX] = UNIT_4_MB | PRESENT_4_MB;

	// Map the page_table to index 184
	page_table[VID_MEM_IDX] |= SET_PRESENT;
	
	page_table[VID_MEM_IDX + 1] |= SET_PRESENT;
	page_table[VID_MEM_IDX + 2] |= SET_PRESENT;
	page_table[VID_MEM_IDX + 3] |= SET_PRESENT;
}

/*	setup_paging
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: Enables paging for the OS
 */
void setup_paging() {
	
	// Initialize the page direcotry
	init_page_directory();

	// Initialize the page table
	init_page_table();

	// Put the starting values into page_dir and page_table
	init_page_map();

	// Put the page_dir address into cr3
	page_access();
	
	// Enable 4 Mb pages
	page_enable_size_extend();
	
	// Set bit high for paging
	page_enable();



}

/*	map_page
 *	
 *	INPUT: physaddr - the physical address to be mapped
		   virtualaddr - the virtual address to be mapped
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: maps a virtual address to a physical address through a 
 *   			 4MB pade in the page direcotry then fluches the tlb
 */
 // code referenced from osdev
void map_page(void * physaddr, void * virtualaddr) {
    unsigned long pdindex = (unsigned long)virtualaddr / UNIT_4_MB;
	page_dir[pdindex] = (unsigned long)physaddr | PAGE_MAP;
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
	page_access();
}

/*	map_vid
 *	
 *	INPUT: physaddr - the physical address to be mapped
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: maps a virtual address to a physical address through a 
 *   			 4kB page in the page table then flushes the tlb
 */
void map_vid(void * physaddr /*, void * virtualaddr */) {
	
	page_table[0] = 0x00;
	page_table[0] |= (uint32_t)physaddr; 
	page_table[0] |= U_R_P;
	
	page_access();
}

/* get_vidmap_page
 * 
 * INPUT: terminal - the specific terminal in charge of the video
 * OUTPUT: vidmap's page
 * RETURN: page mapped to the physical video memory
 * DESCRIPTION: when the screen start needs to be remapped, the address can be
 *				obtained with this function
 *
 */
uint32_t get_vidmap_page(int terminal) {

	// Map a page for video
	page_dir[NEW_VID_IDX] = 0x00;
	page_dir[NEW_VID_IDX] |= (uint32_t) &page_table[0];
	page_dir[NEW_VID_IDX] |= U_R_P;

	page_table[0] = 0x00;
	page_table[0] |= VIDEO_MEM_ADDR; 
	page_table[0] |= U_R_P;

	page_access();

	return NEW_VIRTUAL;
}

void flush_tlb_helper() {
	page_access();
}
