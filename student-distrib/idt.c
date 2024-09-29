

#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"


#include "keyboard_asm_wrapper.h"
#include "syscall_wrapper.h"

#include "keyboard.h"
#include "rtc.h"
#include "rtc_wrapper.h"
#include "pit.h"
#include "pit_wrapper.h"

#include "syscall.h"

#define IDT_END			256
#define REG_INT			0x30
#define EXCP_RET		255

#define INTEL_START		0X00
#define INTEL_END		0x1F
#define MASTER_START	0x20
#define MASTER_END		0x27
#define SLAVE_START		0x28
#define SLAVE_END		0x2F

#define PIT_IDX			0x20

#define SYS_CALL		0x80

#define KEYB_IDX		33
#define RTC_IDX         0x28
								 			

/*	DIVIDE_BY_ZERO()
 *	DIVIDE ERROR EXCEPTION
 *	INPUT:none
 *	OUTPUT:print the exception
 *	RETURN: none
 *	DESCRIPTION:handles the exception named here
 */
void DIVIDE_BY_ZERO(){
	printf("DIVIDE BY ZERO EXCEPTION\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	DB()
 *	DEBUG EXCEPTION
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void DB(){
	printf("DEBUG EXCEPTION\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	NMI()
 *	NMI
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void NMI(){
	printf("NMI EXCEPTION\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	BP()
 *	Breakpoint exception
 *	INPUT: none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void BP(){
	printf("BREAKPOINT_KGDB\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	OF()
 *	Overflow exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION:handles the exception named here
 */
void OF(){
	printf("NMI OVERFLOW_EXCEPTION\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	BR()
 *	BOUND Range Exceeded Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void BR(){
	printf("BOUND Range Exceeded Exception\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	UD()
 *	Invalid Opcode Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void UD(){
	printf("Invalid Opcode Exception\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	
 *	Device Not Available Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void NM(){
	printf("Device Not Available Exception\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	DF()
 *	Double Fault Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void DF(){
	printf("Double Fault Exception \n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	CSO()
 *	Coprocessor Segment Overrun
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void CSO(){
	printf("Coprocessor Segment Overrun\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	TS()
 *	Invalid TSS Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void TS(){
	printf("Invalid TSS Exception\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	NP()
 *	Segment Not Present
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here 
 */
void NP(){
	printf("Segment Not Present \n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	SS()
 *	Stack Fault Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void SS(){
	printf("Stack Fault Exception\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	GP()
 *	General Protection Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void GP(){
	printf("General Protection Exception\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	PF()
 *	Page-Fault Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION:handles the exception named here
 */
void PF(){
	printf("Page-Fault Exception\n");
	system_halt(EXCP_RET);
	while(1){};
}
/*	MF()
 *	x87 FPU Floating-Point Error
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION:handles the exception named here
 */
void MF(){
	printf("FPU Floating-Point Error\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	AC()
 *	Alignment Check Exception 
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void AC(){
	printf("Alignment Check Exception\n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	MC()
 *	Machine-Check Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void MC(){
	printf("Machine-Check Exception \n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	XF()
 *	SIMD Floating-Point Exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void XF(){
	printf("SIMD Floating-Point Exception \n" );
	system_halt(EXCP_RET);
	while(1){};
}
/*	
 *	default exception
 *	INPUT:none
 *	OUTPUT: print the exception
 *	RETURN: none
 *	DESCRIPTION: handles the exception named here
 */
void DEFAULT(){
	cli();
	printf("default \n" );
	sti();
	while(1){};
}

/*	init_idt
 *	
 *	INPUT: none 
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: initializes the idt with values for needed interrupts 
 */
void init_idt() {

	int i; /* counter */

	for (i = 0; i < NUM_VEC; i++) {

		idt[i].seg_selector=KERNEL_CS; 
		idt[i].dpl=0; 					/*specifies level of protection */
		idt[i].present=0; 				/*set all to non present first */
		idt[i].size = 1;				/*size 1*/

		idt[i].reserved0=0;				/*specify as interrupt gates */
		idt[i].reserved1=1;
		idt[i].reserved2=1;
		idt[i].reserved3=0;
		idt[i].reserved4=0;

		/*If statement to make all used interrupts and exceptions that will be used*/
		/*The <20 means there are 19 interrupts that will be present */
		if (i < 20 || i == KEYB_IDX || i == SYS_CALL || i == RTC_IDX || i == PIT_IDX) {
				idt[i].present = 1;
		}
	}

	idt[SYS_CALL].dpl = 3; /*system call protection is 3*/
	

	/*Set each idt entry to their approriate exception to handle */
	/*Index numbers according to https://courses.engr.illinois.edu/ece391/sp2018/secure/references/IA32-ref-manual-vol-3.pdf */

	SET_IDT_ENTRY(idt[0],DIVIDE_BY_ZERO);
	SET_IDT_ENTRY(idt[1],DB);
	SET_IDT_ENTRY(idt[2],NMI);
	SET_IDT_ENTRY(idt[3],BP);
	SET_IDT_ENTRY(idt[4],OF);
	SET_IDT_ENTRY(idt[5],BR);
	SET_IDT_ENTRY(idt[6],UD);
	SET_IDT_ENTRY(idt[7],NM);
	SET_IDT_ENTRY(idt[8],DF);
	SET_IDT_ENTRY(idt[9],CSO);
	SET_IDT_ENTRY(idt[10],TS);
	SET_IDT_ENTRY(idt[11],NP);
	SET_IDT_ENTRY(idt[12],SS);
	SET_IDT_ENTRY(idt[13],GP);
	SET_IDT_ENTRY(idt[14],PF);
	SET_IDT_ENTRY(idt[15],DEFAULT);
	SET_IDT_ENTRY(idt[16],MF);
	SET_IDT_ENTRY(idt[17],AC);
	SET_IDT_ENTRY(idt[18],MC);
	SET_IDT_ENTRY(idt[19],XF);
	SET_IDT_ENTRY(idt[SYS_CALL], syscall_wrapper);
	SET_IDT_ENTRY(idt[RTC_IDX],rtc_wrapper);
	SET_IDT_ENTRY(idt[KEYB_IDX], keyboard_wrapper);
	SET_IDT_ENTRY(idt[PIT_IDX], pit_wrapper);
}

