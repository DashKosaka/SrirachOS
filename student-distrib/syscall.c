#include "syscall.h"
#include "filesys.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "i8259.h"
#include "rtc.h"
#include "scheduler.h"
#include "lib.h"

#define MAX_COMMAND_LENGTH 		128
#define MAX_PROCESSES		 	  6
#define UNIT_8_MB 		   0x800000
#define UNIT_128_MB       0x8000000
#define UNIT_4_MB 		   0x400000
#define UNIT_4_KB 			 0x1000
#define UNIT_8_KB 		     0x8000
#define PCB_MASK		 0xFFFF8000
#define STDIN_IDX				  1
#define STDOUT_IDX				  0
#define EXC_NUM_1              0x7f
#define EXC_NUM_2              0x45
#define EXC_NUM_3              0x4c
#define EXC_NUM_4              0x46
#define PROG_IMG_OFF	 0x08048000
#define KERNEL_DS            0x0018
#define ENTRY_POINT				 24
#define MAX_FD					  7

#define BUFSIZE					128

#define MAX_FILES				8
#define FILES 					2

#define VIDEO_MEMORY_ADDRESS   0xB8000
#define COPY_SIZE 4096
// global vairiable to keep track of number of running processes
// make array (edited this to make -1 mean not in use)
static int8_t process_num[MAX_PROCESSES] = {-1,-1,-1,-1,-1,-1};
							   //0, 1, 2, x, x, x
static f_ops_table file_table;
static f_ops_table dir_table;
static f_ops_table rtc_table;
static f_ops_table terminal_table;

static int8_t arg_buf[BUFSIZE];

// Stores the current terminal that is being displayed
#define MAX_TERMINALS 			3
#define BUFSIZE 				128
#define KEYBOARD_IRQ_NUM	1
static uint8_t active_terminal = 0;	// Warning this is going to be "0" indexed
//static int8_t saved_bufs[MAX_TERMINALS][BUFSIZE];
//static int buf_idx[MAX_TERMINALS] = {0, 0, 0}; 
static int main_process = 0;

/*  init_terminals()
 *  
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: This initializes the three terminal structs to
 * 				 initial values.
 */
void init_terminals() {
	int i;
	for (i = 0; i < 3; i++) {
		terminals[i].terminal_idx = i;
		terminals[i].x_cursor = 0;
		terminals[i].y_cursor = 0;
		terminals[i].save_buf[0] = '\0';
		terminals[i].save_idx = 0;
		terminals[i].curr_task = -1;
		terminals[i].video_addr = VIDEO_MEMORY_ADDRESS + UNIT_4_KB * (i + 1);
		terminals[i].color = i+1;
	}
}

/*  switch_terminal
 *  
 *  INPUT: terminal - number of the terminal we want to swtich to
 *  OUTPUT: updated visual and terminal focus
 *  RETURN: none
 *  DESCRIPTION: lets the user switch which terminal he is currently working on
 */
void switch_terminal(int terminal) {
	// Switching to the same terminal shouldn't do anything
	if(active_terminal == terminal) {return;}
	
	cli();
	
	// Save the old video memory
	memcpy((int*)terminals[active_terminal].video_addr, (int*)VIDEO_MEMORY_ADDRESS, COPY_SIZE);
	
	// Update color
	cycle_text_color(terminals[terminal].color);
	
	// Change the current terminal number
	active_terminal = terminal;

	// Switch to the "most childish" process of the terminal
	int next_process = terminals[terminal].curr_task;

	// Start an initial shell for the process
	if(next_process == -1) {
		// save the terminal screen to page given to terminal
		memcpy((int*)VIDEO_MEMORY_ADDRESS, (int*) terminals[active_terminal].video_addr, COPY_SIZE);
				
		// get pcb of the main_process
		pcb_t* pcb = return_pcb_parent(main_process);
		
		// Save ebp/esp
		do {                                    																						\
			asm volatile ("                   			  				  					 								  		  \n\
					movl %%ebp, %0            			  				 					 								  		  \n\
					movl %%esp, %1            			  				  					 								  		  \n\
					"                          			    																			\
					: "=r"(pcb->pit_ebp), "=r"(pcb->pit_esp)				   															\
					:           																										\
					: "ebp", "esp"			                          																	\
			);                                  																						\
		} while (0);
		sti();
		// execute shell
		system_execute((uint8_t*)"shell");
	}
	
	// Restore cursor information
	x_set_screen(terminals[active_terminal].x_cursor);
	y_set_screen(terminals[active_terminal].y_cursor);
	//update_cursor();
	
	// Copy terminal screen
	memcpy((int*)VIDEO_MEMORY_ADDRESS, (int*)terminals[active_terminal].video_addr, COPY_SIZE);
	
	sti();
}

/*  find_terminal_process
 *  
 *  INPUT: none
 *  OUTPUT: none 
 *  RETURN: idx to process num array
 *			-1 if there is none
 *  DESCRIPTION: finds the main process of a specific terminal
 */
int8_t find_terminal_process(int8_t terminal) {
	int i;

	// go backwards through the process array
	for(i=MAX_PROCESSES-1;i>=0;i--) {
				//my_puts(save_buf);
		if(process_num[i] == terminal) {
			return i;
		}
	}

	return -1;
}


int8_t find_parent_process(int8_t idx, int8_t terminal) {
	// This really shouldn't happen
	if(idx < MAX_TERMINALS) {return -1;}

	int i;

	// go backwards from the given index
	for(i=idx-1;i>=0;i--) {
		if(process_num[i] == terminal) {
			return i;
		}
	}

	// again...really shouldn't happen
	return -1;
}

/*  setup_exec
 *  
 *  INPUT: none
 *  OUTPUT: updated pointers to function ops table
 *  RETURN: none
 *  DESCRIPTION: sets up the system calls before exec can be called
 */
void setup_exec() {

	/*
	// fill the f_ops table
	pcb->fdt[fd].f_ops.read = (int32_t (*)(int32_t, void*, int32_t)) dir_read;
	pcb->fdt[fd].f_ops.write =(int32_t (*)(int32_t, const void*, int32_t))dir_write;
	pcb->fdt[fd].f_ops.open = dir_open;
	pcb->fdt[fd].f_ops.close = dir_close;
	*/
	// setup file pointers
	file_table.read = file_read;
	file_table.write = file_write;
	file_table.open = file_open;
	file_table.close = file_close;

	// setup directory pointers
	dir_table.read = dir_read;
	dir_table.write = dir_write;
	dir_table.open = dir_open;
	dir_table.close = dir_close;

	// setup rtc pointers
	rtc_table.read = rtc_read;
	rtc_table.write = rtc_write;
	rtc_table.open = rtc_open;
	rtc_table.close = rtc_close;

	// setup terminal pointers
	terminal_table.read = terminal_read;
	terminal_table.write = terminal_write;
	terminal_table.open = terminal_open;
	terminal_table.close = terminal_close;

	return;	
}


/*  pcb_t* return_pcb_parent(int8_t num) 
 *  
 *  INPUT: process number
 *  OUTPUT: 
 *  RETURN: returns the process address stack top
 *  DESCRIPTION: helper function to return process address top of stack
 */
pcb_t* return_pcb_parent(int8_t num) {
	 return (pcb_t*)(UNIT_8_MB - (1 + num) * UNIT_8_KB);
}

/*  pcb_t* return_pcb_addr(int8_t num) 
 *  
 *  INPUT: process number
 *  OUTPUT: none
 *  RETURN: returns the process address
 *  DESCRIPTION: helper function to return process address
 */
pcb_t* return_pcb_addr(int8_t num) {
	// return (pcb_t*)(UNIT_8_MB - (1 + num) * UNIT_8_KB);
	int32_t addr;
	do {                                    			\
		asm volatile ("                   			  \n\
				movl %%esp, %0            			  \n\
				"                          			    \
				: "=r"(addr)                        	\
				:           							\
				: "ebp", "esp"                          \
		);                                  			\
	} while (0);
	return (pcb_t*)(addr & PCB_MASK);
}

/*  pcb_t* create_pcb(int8_t process_idx)
 *  
 *  INPUT: process number
 *  OUTPUT: none
 *  RETURN: returns pcb pointer
 *  DESCRIPTION: creates pcb based on process number
 */
pcb_t* create_pcb(int8_t process_idx) {
	// use helper to get addr of top of kernel stack
	pcb_t* pcb = return_pcb_parent(process_idx);

	pcb->pid = process_idx;
	// initialize fdt entries
	int i;
	// max fd number is 7
	for (i = 0; i < 8; i++) {
		// 0 is not present
		pcb->fdt[i].flags = 0;
		// -1 is not being used
		pcb->fdt[i].inode_num = -1;
		pcb->fdt[i].file_pos = 0;
	}
	//stdin
	pcb->fdt[STDIN_IDX].f_ops = &terminal_table;

	// 1 is present
	pcb->fdt[STDIN_IDX].flags = 1;

	//stdout
	pcb->fdt[STDOUT_IDX].f_ops = &terminal_table;

	// 1 is present
	pcb->fdt[STDOUT_IDX].flags = 1;

	// need to check array or check by terminal basis
	int parent_pid;
	if (process_idx < MAX_TERMINALS) parent_pid = process_idx;
	else parent_pid = find_parent_process(process_idx, active_terminal);
	
	pcb->parent_pcb = (int32_t)return_pcb_parent(parent_pid);
	pcb->status_retval = 0;
	
	// pcb->task.running = 0;
	// pcb->task.next = NULL;
	// pcb->task.prev = NULL;
	
	terminals[active_terminal].curr_task = process_idx;
	
	// add_task(pcb);

	return pcb;
}

/*  int32_t system_execute(const uint8_t* command)
 *  
 *  INPUT: command (what to execute)
 *  OUTPUT: none
 *  RETURN: returns 0 on success, 0 otherwise
 *  DESCRIPTION: executes commands passed into the function 
 */
int32_t system_execute(const uint8_t* command) {
	// parse args
	uint8_t parsed_command[MAX_COMMAND_LENGTH];
	int i = 0;
	int j = 0;

	// strip leading spaces
	while (command[i] == ' ') {
		i++;
	}
	
	// parse command
	while (command[i] != '\0') {
		// to strip trailing spaces
		if (command[i] == ' ') break;
		parsed_command[j] = command[i];
		i++;
		j++;
	}
	parsed_command[j] = '\0';
	
	// parse arg, clear spaces and then get argument
	arg_buf[0] = '\0';
	j = 0;
	while(command[i] == ' ') {i++;}
	while(command[i] != '\0') {
		arg_buf[j] = command[i];
		i++; j++;
	}
	arg_buf[j] = '\0';

	// check file validity
	dentry_t test;
	if ( -1 == read_dentry_by_name(parsed_command, &test))
		return -1;
	// check if executable
	// entry point is 32 bits
	uint8_t buf[4];
	read_data(test.inode_num, 0, buf, 4);
	if (buf[0] != EXC_NUM_1 || buf[1] != EXC_NUM_2 || buf[2] != EXC_NUM_3 || buf[3] != EXC_NUM_4)
		return -1;
	// can only allow 6 processes at a time
	// check if array is full/ assign vairable baseed off of array idx
	int8_t process_idx = 0;
	while (process_idx < 8) {
		if (process_idx >= 6) {
			my_puts("Max processes.\n", 15);
			return 0;
		}
		if (process_num[(int)process_idx] == -1) {
			// Reserved processes for specific terminals
			if(process_idx < MAX_TERMINALS && process_idx == active_terminal) {
				// Mark the process num as owned by the current terminal
				process_num[(int)process_idx] = active_terminal;
				break;
			} 
			// Free processes open to any terminal
			else if(MAX_TERMINALS <= process_idx && process_idx < MAX_PROCESSES) {				
				// Mark the process num as owned by the current terminal
				process_num[(int)process_idx] = active_terminal;
				break;
			}
		}
		process_idx++;
	}
	
	// if (process_idx == -1) {
		// printf("Max processes.\n");
		// return 0;
	// }
	
	// setup paging
	// get entry point
	uint8_t save_entry_point[4];
	read_data(test.inode_num, ENTRY_POINT, save_entry_point, 4);
	//stored little endian so need to flip
	// shift each byte by its corresponding position
	uint32_t entry_point = 0;
	entry_point |= (uint32_t)save_entry_point[0];
	entry_point |= (uint32_t)save_entry_point[1]<<8; // shift left 8 bits
	entry_point |= (uint32_t)save_entry_point[2]<<16; // shift left 16 bits
	entry_point |= (uint32_t)save_entry_point[3]<<24; // shift left 24 bits
	
	// map virtual address: 128MB to physical address: 8MB + process offset
	map_page((uint32_t*)UNIT_8_MB + process_idx * UNIT_4_MB, (uint32_t*)UNIT_128_MB);
	
	// load file into memory
	// 0 is the offset for the file
	read_data(test.inode_num, 0, (uint8_t*)PROG_IMG_OFF, UNIT_8_MB/*need to read whole file*/);
	
	cli(); 
	
	// create pcb
	pcb_t* pcb = create_pcb(process_idx);

	// switches the main current proccess to be the just made process
	main_process = process_idx;
	
	// save esp and ebp in pcb
	do {                                    			\
		asm volatile ("                   			  \n\
				movl %%ebp, %0            			  \n\
				movl %%esp, %1            			  \n\
				"                          			    \
				: "=r"(pcb->ebp), "=r"(pcb->esp)    	\
				:           							\
				: "ebp", "esp"                          \
		);                                  			\
	} while (0);
	
	// prepare for context switch
	tss.ss0 = KERNEL_DS;
	tss.esp0 = UNIT_8_MB - UNIT_8_KB * process_idx - 4;	
	
	// push iret context to stack
	// 0x002B is user stack segment (USER_DS in x86_desc.h)
	// 0x083ffffc is the location of user stack (0x08400000 - 4B)
	// 0x0023 is the user code segment (USER_CS in x86_desc.h)
	do {                                    \
		asm volatile ("                   \n\
				cli                       \n\
				mov $0x2B, %%ax           \n\
				mov %%ax, %%ds            \n\
				pushl $0x002B             \n\
				pushl $0x083ffffc         \n\
				pushfl                    \n\
				popl %%eax                \n\
				orl $0x200, %%eax         \n\
				pushl %%eax               \n\
				pushl $0x0023             \n\
				pushl %0                  \n\
				iret                      \n\
				ret_from_halt:            \n\
				leave                     \n\
				ret                       \n\
				"                           \
				: /* no outputs */          \
				: "r"(entry_point)          \
				: "eax"                     \
		);                                  \
	} while (0);
	
	// return should not be executed
	return pcb->status_retval;
}

/*  int32_t system_read(int32_t fd, void* buf, int32_t nbytes)
 *  
 *  INPUT: file descriptor, buffer, number of bytes
 *  OUTPUT: none
 *  RETURN: returns the file operation read
 *  DESCRIPTION: a read function for system calls
 */
int32_t system_read(int32_t fd, void* buf, int32_t nbytes) {
	if (buf == NULL || fd < 0 || fd > MAX_FD) return -1;
	// based off pid in pcb
	pcb_t* pcb = return_pcb_parent(main_process);
	set_filesys_pcb(pcb);
	return pcb->fdt[fd].f_ops->read(fd, buf, nbytes);
}

/*  int32_t system_write(int32_t fd, void* buf, int32_t nbytes)
 *  
 *  INPUT: file descriptor, buffer, number of bytes
 *  OUTPUT: none
 *  RETURN: returns the file operation write
 *  DESCRIPTION: a write function for system calls
 */
int32_t system_write(int32_t fd, const void* buf, int32_t nbytes) {
	if (buf == NULL || fd < 0 || fd > MAX_FD) return -1;
	pcb_t* pcb = return_pcb_parent(main_process);
	set_filesys_pcb(pcb);
	return pcb->fdt[fd].f_ops->write(fd, buf, nbytes);
}

/*  int32_t system_open(const uint8_t* filename)
 *  
 *  INPUT: file name
 *  OUTPUT: none
 *  RETURN: returns the file descriptor
 *  DESCRIPTION: an open function for system calls
 */
int32_t system_open(const uint8_t* filename) {
	// terminal_open(filename);	
	// return 0;
	pcb_t* pcb = return_pcb_parent(main_process);
	set_filesys_pcb(pcb);
	int32_t fd = -1;

	// get the filetype 
	int32_t filetype = get_filetype(filename);

	switch (filetype) {
		// rtc
		case 0:
			if(-1 == (fd = fill_open_fd(pcb))) {return -1;}

			// fill the f_ops table
			pcb->fdt[fd].f_ops = &rtc_table;

			break;


		// directory
		case 1:
			fd = dir_open(filename);
			
			// fill the f_ops table
			pcb->fdt[fd].f_ops = &dir_table;

			break;

		// file
		case 2:
			fd = file_open(filename);

			// fill the f_ops table
			pcb->fdt[fd].f_ops = &file_table;	

			break;

		default:
			return -1;
			break;
	}

	return fd;
}

/*  int32_t system_close(int32_t fd)
 *  
 *  INPUT: file descriptor - index into the process fdt table
 *  OUTPUT: none
 *  RETURN: returns the file operation close
 *  DESCRIPTION: a close function for system calls
 */
int32_t system_close(int32_t fd) {
	if (fd < 0 || fd > MAX_FD) return -1;
	pcb_t* pcb = return_pcb_parent(main_process);
	set_filesys_pcb(pcb);
	return pcb->fdt[fd].f_ops->close(fd);
}
/* int32_t system_halt(uint8_t status) 
 *  
 *  INPUT: status - error code
 *  OUTPUT: none
 *  RETURN: none
 *  DESCRIPTION: halts a process
 */
int32_t system_halt(uint8_t status) {
	cli();
	// restore parent pcb
	pcb_t* curr_pcb = return_pcb_parent(main_process);
	// remove_task(curr_pcb);
	pcb_t* parent_pcb = (pcb_t*)(curr_pcb->parent_pcb);
	// store retval - not used
	parent_pcb->status_retval = status;
	
	int i;
	for (i = 0; i < 3; i++) {
		if (terminals[i].curr_task == curr_pcb->pid)
			terminals[i].curr_task = parent_pcb->pid;
	}
	// check if process num is 0
	// decrement process num
	// set as available
	process_num[curr_pcb->pid] = -1;

	// change this check
	if (curr_pcb->pid < MAX_TERMINALS) {
		system_execute((uint8_t*)"shell");
	}
		
	// remap program
	main_process = parent_pcb->pid;
	map_page((uint32_t*)UNIT_8_MB + parent_pcb->pid * UNIT_4_MB, (uint32_t*)UNIT_128_MB);
	
	// restore ebp and esp and tss.esp0
	// tss.esp0 = curr_pcb->esp;
	tss.esp0 = UNIT_8_MB - UNIT_8_KB * parent_pcb->pid - 4;	
	do {                                    			   						    \
		asm volatile ("                   			     					      \n\
				movl %0, %%ebp            			    					      \n\
				movl %1, %%esp            			    					      \n\
				movl %2, %%eax            			    					      \n\
				jmp ret_from_halt            			 						  \n\
				"                          			       							\
				: 								    	   							\
				: "r"(curr_pcb->ebp), "r"(curr_pcb->esp), "r"((uint32_t)status)     \
				: "ebp", "esp", "eax"                             					\
		);                                  			   							\
	} while (0);
	
	return 0;
}

/* int32_t getargs(uint8_t* buf, int32_t nbytes)
 *  
 *  INPUT: buffer, number of bytes
 *  OUTPUT: the arguments read
 *  RETURN: return -1 if buf is null or 0 if successful
 *  DESCRIPTION: gets the arguments when a command is typed
 */
int32_t system_getargs(uint8_t* buf, int32_t nbytes){

	if(buf == NULL || nbytes == 0)
		return -1;
	
	if (strlen((const int8_t*)arg_buf) == 0) return -1;

	// Copy the arg
	strcpy((int8_t*)buf, (const int8_t*)arg_buf);

	return 0;

}

/* vidmap
 *  
 *  INPUT: screen_start - the double pointer to the location of the screen
 *  OUTPUT: none
 *  RETURN: return -1 if buf is null or 0 if successful 
 *  DESCRIPTION: gets the arguments when a command is typed
 */
// May need global?
int32_t system_vidmap(uint8_t** screen_start){
	// *screen_start = 
	//Things for this function
	//Refer to pic sent in group chat

	//Currently the virtually memory and the physical memory are almost 1 to 1 
	//user has no access to the space less than 4mb
	//allocate a page for fish anywhere between the 8mb  and 128mb or somewhere 
	//start will hold the address where the new page of video memory will be held
	//virtual video memory needs to be pointed to the physical video memory(some new mapping in the page table has to be done i believe)	

	// Error checks here
	// if(something that will break the OS) {return -1;}
	if (!screen_start || (int32_t)screen_start == UNIT_4_MB)
		return -1;

	*screen_start = (uint8_t*) get_vidmap_page(active_terminal);

	return 0;

}



/* int32_t fill_open_fd(pcb_t* pcb)
 *  
 *  INPUT: pcb
 *  OUTPUT: none
 *  RETURN: fd descriptor on success, -1 otherwise
 *  DESCRIPTION: fills an open file descriptor
 */
int32_t fill_open_fd(pcb_t* pcb) {
	int fd;

	for(fd=FILES;fd<MAX_FILES;fd++) {
		if(pcb->fdt[fd].flags == 0) {
			pcb->fdt[fd].flags = 1;
			return fd;
		}
	}

	return -1;
}



int get_main_process() {
	return main_process;
}

int check_tasks(int num) {
	int i;
	num = process_num[num];
	for (i = 1; i < 4; i++) {
		if (terminals[(num+i)%3].curr_task != main_process
						&& terminals[(num+i)%3].curr_task != -1) {
				//if (terminals[(num+i)%3].curr_task > 2 || process_num[terminals[(num+i)%3].curr_task] == active_terminal)
				return terminals[(num+i)%3].curr_task;
		}
		num++;
	}
	return -1;
}

/*	void set_main_process(uint8_t pid)
 *	
 *	INPUT: pid
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: Sets the main process to pid
 */

void set_main_process(uint8_t pid) {
	main_process = pid;
}

/*	char* video_return()
 *	
 *	INPUT: pid
 *	OUTPUT: none
 *	RETURN: video address
 *	DESCRIPTION: return the address where we should write depending on the active process and active terminal
 */

char* video_return() {
	if (process_num[main_process] == active_terminal)
		return (char *)VIDEO_MEMORY_ADDRESS;
	return (char *) (VIDEO_MEMORY_ADDRESS + UNIT_4_KB * (1 + process_num[main_process]));
}

/*	int x_return(int t)
 *	
 *	INPUT: terminal
 *	OUTPUT: none
 *	RETURN: x coordinate of cursor
 *	DESCRIPTION: returns the x coordinate of cursor
 */


int x_return(int t) {
	if (t) 	return (int)(terminals[active_terminal].x_cursor);
	return (int)(terminals[(int)process_num[main_process]].x_cursor);
}

/*	int y_return(int t)
 *	
 *	INPUT: terminal
 *	OUTPUT: none
 *	RETURN: y coordinate of cursor
 *	DESCRIPTION: returns the y coordinate of cursor
 */

int y_return(int t) {
	if (t) 	return terminals[(int)active_terminal].y_cursor;
	return terminals[(int)process_num[main_process]].y_cursor;}

/*	void set_terminal_pos(uint8_t x, uint8_t y, int t)
 *	
 *	INPUT: coordinate x, coordinate y, terminal
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: set the x coordinate of cursor and y coordinate of the terminal
 */	

void set_terminal_pos(uint8_t x, uint8_t y, int t) {
	if (t) {
		terminals[(int)active_terminal].x_cursor = x;
		terminals[(int)active_terminal].y_cursor = y;
		return;
	}
	terminals[(int)process_num[main_process]].x_cursor = x;
	terminals[(int)process_num[main_process]].y_cursor = y;
}

/*	uint32_t get_vid_mem()
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: get the vid memory depending on active terminal and active process
 */	

uint32_t get_vid_mem() {
	if (process_num[(int)main_process] == active_terminal) return VIDEO_MEMORY_ADDRESS;
	else return terminals[(int)process_num[main_process]].video_addr;
}

/*	uint32_t return_active_terminal()
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: returns active terminal
 */	


int return_active_terminal() {
	return (int)active_terminal;
}


/*	int check_read_process()
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: returns -1 if main process is not in active terminal, 0 otherwise
 */	


int check_read_process() {
	if (process_num[main_process] != active_terminal) return -1;
	return 0;
}


