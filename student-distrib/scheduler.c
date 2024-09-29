#include "scheduler.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"
#include "x86_desc.h"
#include "paging.h"

#define TASK_RUNNABLE	1
#define TASK_SLEEP		0
#define UNIT_8_MB 		   0x800000
#define UNIT_128_MB       0x8000000
#define UNIT_4_MB 		   0x400000
#define KERNEL_DS            0x0018
#define UNIT_8_KB 		     0x8000
#define irq_0					  0

// global variable used to keep track of the current processes pcb
static uint32_t curr_process;

/*  switch_task()
 *
 *  INPUT: num -- the process number 
 *	OUTPUT: none
 *	RETURN: -1 if there are no other tasks to run
 *			0 if the curr_process was updated
 *	DESCRIPTION: This function calls a helper to
 *				 check if there is a task to switch
 *				 to, and if so, it will update curr_process.
 */
uint32_t switch_task(int num) {
	int next;
	next = check_tasks(num);
	if (next == -1) return -1;
	curr_process = (UNIT_8_MB - (1 + next) * UNIT_8_KB);
	return 0;	
}

/*  scheduler_handler()
 *
 *  INPUT: none 
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: This function, if necessary, will perform
 * 				 a context switch to another process. The
 *				 esp and ebp of the process are saved and the 
 *				 next processes ebp and esp are restored. The 
 *				 memory of the program is remapped to the relevant
 *				 process. The memory used by vidmap is also updated
 *				 to the relevant page based on the active terminal.
 * 				 The tss is set to the next process.
 */
void scheduler_handler() {
	send_eoi(irq_0);
	
	// helper to get the main_process 	
	int num = get_main_process();
	// current process to be saved
	uint32_t save_process = (UNIT_8_MB - (1 + num) * UNIT_8_KB);
	
	// get next process
	if (switch_task(num) == -1) return;
	
	// sets the main_process value
	set_main_process(((pcb_t*)curr_process)->pid);
	
	// save esp/ebp
	do {                                    														\
		asm volatile ("                   			  				  					 		  \n\
				movl %%ebp, %0            			  				 					 		  \n\
				movl %%esp, %1            			  				  					 		  \n\
				"                          			    											\
				: "=r"(((pcb_t*)save_process)->pit_ebp), "=r"(((pcb_t*)save_process)->pit_esp)   	\
				:           																		\
				: "ebp", "esp"			                          									\
		);                                  														\
	} while (0);
	
	//switch process paging
	map_page((uint32_t*)UNIT_8_MB + ((pcb_t*)curr_process)->pid * UNIT_4_MB, (uint32_t*)UNIT_128_MB);
	uint32_t vid_mem = get_vid_mem();
	map_vid((char*)vid_mem);
	
	//set tss
	tss.ss0 = KERNEL_DS;
	tss.esp0 = UNIT_8_MB - UNIT_8_KB * ((pcb_t*)curr_process)->pid - 4;	
		
	//restore next processes esp/ebp
	do {                                    			   						     											\
		asm volatile ("                   			     					       									  		  \n\
				movl %0, %%ebp            			    					       									  		  \n\
				movl %1, %%esp            			    					       									  		  \n\
				"                          			       							 											\
				: 								    	   							 											\
				: "r"(((pcb_t*)curr_process)->pit_ebp), "r"(((pcb_t*)curr_process)->pit_esp)  									\
				: "ebp", "esp"			                             					         								\
		);                                  			   							 											\
	} while (0);
	
	return;
}

void scheduler_init() {
	//process_queue = NULL;
	curr_process = NULL;
}


