/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define keyboard_irq    1
#define rtc_irq         8
#define MASK_ALL_IRQ    0xff
#define MAX_IRQ         8
#define slave_irq		2

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask= 0xff; /* IRQs 0-7  */
uint8_t slave_mask= 0xff;  /* IRQs 8-15 */

/*  i8259_init()
 *
 *  INPUT: none 
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: This function initialize the 8259 PIC.
 */
void i8259_init(void) {
	
	outb(master_mask, MASTER_8259_PORT + 1);		/* mask all of master_8259 */
	outb(slave_mask, SLAVE_8259_PORT + 1); 			/* mask all of slave_8259 */
	
	outb(ICW1, MASTER_8259_PORT); 				/* ICW1: select master_8259 init */
	outb(ICW2_MASTER + 0, MASTER_8259_PORT + 1);	/* ICW2: master_8259 IR0-7 mapped to 0x20-0x27 */
	outb(ICW3_MASTER, MASTER_8259_PORT + 1);		/* master has slave on IR2 */
	outb(ICW4, MASTER_8259_PORT + 1);				/* normal EOI */	
	
	outb(ICW1, SLAVE_8259_PORT); 					/* ICW1: select slave_8259 init */
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);	/* ICW2: slave_8259 IR0-7 mapped to 0x28-0x2f */
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);		/* slave on masters IR2 */
	outb(ICW4, SLAVE_8259_PORT + 1);				/* normal EOI */
	
	outb(master_mask, MASTER_8259_PORT + 1);		/* mask all of master_8259 */
	outb(slave_mask, SLAVE_8259_PORT + 1); 			/* mask all of slave_8259 */
	
	enable_irq(slave_irq);
}

/*  enable_irq()
 *
 *  INPUT: irq_num specifying which irq line to enable 
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: This function enables (unmask) the specified IRQ.
 */
void enable_irq(uint32_t irq_num) {
	uint16_t port;
	uint8_t irq_mask;
	if (irq_num < MAX_IRQ) {
		port = MASTER_8259_PORT + 1;				/* IRQ line < 8 is on master */
		//irq_mask = master_mask;						/* get current master IRQ mask */
	}
	else {
		port = SLAVE_8259_PORT + 1;					/* IRQ line > 7 is on slave */
		irq_num -= MAX_IRQ;								/* adjust irq_num to map to correct IR line */
		//irq_mask = slave_mask;						/* get current slave IRQ mask */
	}
	irq_mask = inb(port) & ~(1 << irq_num);						/* set corresponding IRQ line to 1 */
	outb(irq_mask, port);
}

/*  disable_irq()
 *
 *  INPUT: irq_num specifying which irq line to disable 
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: This function disables (mask) the specified IRQ.
 */
void disable_irq(uint32_t irq_num) {
	uint16_t port;
	uint8_t irq_mask;
	if (irq_num < MAX_IRQ) {
		port = MASTER_8259_PORT + 1;				/* IRQ line < 8 is on master */
		//irq_mask = master_mask;						/* get current master IRQ mask */
	}
	else {
		port = SLAVE_8259_PORT + 1;					/* IRQ line > 7 is on slave */
		irq_num -= MAX_IRQ;								/* adjust irq_num to map to correct IR line */
		//irq_mask = slave_mask;						/* get current slave IRQ mask */
	}
	irq_mask = inb(port) | (1 << irq_num);					/* set corresponding IRQ line to 0 */
	outb(irq_mask, port);
}

/*  disable_irq()
 *
 *  INPUT: irq_num specifying which irq line was being serviced 
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: This function sends end-of-interrupt signal for the specified IRQ.
 */
void send_eoi(uint32_t irq_num) {
	if (irq_num >= MAX_IRQ) {
		irq_num -= MAX_IRQ;
		outb((EOI|irq_num), SLAVE_8259_PORT);/* IRQ >= 8 is on slave */
		outb((EOI | slave_irq), MASTER_8259_PORT);	/*need to let master port know that the interrupt finish*/
	}
	else {
		outb((EOI|irq_num), MASTER_8259_PORT);
	}
}
