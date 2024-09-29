#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "paging.h"


/* Function operations table pointer structure. */
typedef struct fops_table_desc {
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*open)(const uint8_t* filename);
	int32_t (*close)(int32_t fd);
} f_ops_table;

/* File descriptor table structure. */
typedef struct fdt_desc {
	// file operations table
	f_ops_table* f_ops;
	// inode
	// inode_lets us find dentry
	int32_t inode_num;
	// file position
	int32_t file_pos;
	// flags
	int32_t flags;
} fdt_t;

/* Process control block structure. */
/*
 * fdt -- The file descriptor table. This allows 8 files - 0th index
 * 		  mapped to stdin, 1st index mapped to stdout.
 * pid -- The process id. This is the index number into process_num, 
 *		  the array that keeps track of running processes.
 * ebp -- The saved ebp of kernel stack before context switch. Used to 
 *		  go back to execute properly after a halt.
 * esp -- The saved esp of kernel stack before context switch. Used to 
 *		  go back to execute properly after a halt.
 * parent_pcb -- The pointer to the parent pcb. This allows a child 
 *				 process to access information about its parent process.
 * status_retval -- The return value of a process. Updated after a halt.
 * pit_ebp -- The saved ebp of process used in scheduling. Needs to be 
 * 			  saved and restored before switching to a different process
 *			  and returning to the process.
 * pit_esp -- The saved esp of process used in scheduling. Needs to be 
 * 			  saved and restored before switching to a different process
 *			  and returning to the process.
 */
typedef struct pcb_desc {
	// File descriptor array
	fdt_t fdt[8];
	// current process num
	uint8_t pid;
	uint32_t ebp;
	uint32_t esp;
	uint32_t parent_pcb;
	uint32_t status_retval;
	
	uint32_t pit_ebp;
	uint32_t pit_esp;
} pcb_t;

/* Initialize the terminal structs */
void init_terminals();

/* Returns the address of the video memory */
char* video_return();

/* Return the x pos of the screen */
int x_return(int t);

/* Return the y pos of the screen */
int y_return(int t);

/* Set the position */
void set_terminal_pos(uint8_t x, uint8_t y, int t);

/* Gets the video memory of the terminal */
uint32_t get_vid_mem();

/* Function to setup stuff before anything happens */
void setup_exec();

/* Helper to return the address of the pcb. */
pcb_t* return_pcb_addr(int8_t num);

/* returns the parent of some pcb */
pcb_t* return_pcb_parent(int8_t num);

/* Helper function to initialize an fdt entry. */
int32_t fill_open_fd(pcb_t* pcb);

/* Helper function to get the process number. */
int8_t get_proccess_num();

/* Gets the current process number */
int get_main_process();

/* Check what the taskts*/
int check_tasks(int num);

/* Return the number for the current active terminal */
int return_active_terminal();

/* Check if the main process is the same as the active terminal */
int check_read_process();

/* Sets the main process */
void set_main_process(uint8_t pid);

/* Helper function to create and initialize a pcb. */
pcb_t* create_pcb(int8_t process_num);

/* Function to change the displayed (main) terminal */
void switch_terminal(int terminal);

/* Function to change to the main process of a specific terminal */
int8_t find_terminal_process(int8_t terminal);

/* Finds ther parent process of a specific index and its relative terminal number */
int8_t find_parent_process(int8_t idx, int8_t terminal);

/* Execute system call. */
int32_t system_execute(const uint8_t* command);

/* Read system call. */
int32_t system_read(int32_t fd, void* buf, int32_t nbytes);

/* Write system call. */
int32_t system_write(int32_t fd, const void* buf, int32_t nbytes);

/* Open system call. */
int32_t system_open(const uint8_t* filename);

/* Close system call. */
int32_t system_close(int32_t fd);

/* Halt system call. */
int32_t system_halt(uint8_t status);

/* Getargs system call */
int32_t	system_getargs(uint8_t* buf, int32_t nbytes);

/* Vidmap system call */
int32_t system_vidmap(uint8_t** screen_start);

#endif
