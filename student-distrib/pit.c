#include "pit.h"
#include "scheduler.h"

#define MAX_FREQ  1193180
#define CHOSEN_HZ 30
#define BIT_MASK 0xFF
#define COMMAND_BYTE 0x36
#define PORT_1 0x40
#define PORT_2 0x43
#define SHIFTER 8


/*	pit_init()
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: Initializes the PIT
 *  reference at http://www.osdever.net/bkerndev/Docs/pit.htm
 */
 

void pit_init(){

    int divisor = MAX_FREQ / CHOSEN_HZ;       /* Calculate our divisor */
    outb(COMMAND_BYTE, PORT_2);             /* Set our command byte 0x36 */
    outb(divisor & BIT_MASK, PORT_1);   /* Set low byte of divisor */
    outb(divisor >> SHIFTER, PORT_1);     /* Set high byte of divisor */
}
/*	pit_init()
 *	
 *	INPUT: none
 *	OUTPUT: none
 *	RETURN: none
 *	DESCRIPTION: PIT handler calls the schedule handler
 */

void pit_handler(){

	scheduler_handler();

}
