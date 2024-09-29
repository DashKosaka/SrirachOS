#include "rtc.h"

#define IRQ_8 		8
#define RATE 		0x2F
#define RTC_PORT	0x70
#define CMOS_PORT   0x71
#define REG_A  		0x8A
#define REG_B		0x8B
#define PREVIOUS  	0x40
#define REG_C		0x0C
#define FREQ_8192	8192
#define FREQ_4096 	4096
#define FREQ_2048 	2048
#define FREQ_1024	1024
#define FREQ_512	512
#define FREQ_256	256
#define FREQ_128	128
#define FREQ_64		64
#define FREQ_32		32
#define FREQ_16		16
#define FREQ_8 		8
#define FREQ_4 		4
#define FREQ_2 		2
#define RATE_1024	0x6
#define RATE_512	0x7
#define RATE_256	0x8
#define RATE_128	0x9
#define RATE_64		0xA
#define RATE_32		0xB
#define RATE_16		0xC
#define RATE_8 		0xD
#define RATE_4 		0xE
#define RATE_2 		0xF
#define FOUR_BYTES 4


int rtc_flag=0;

/*source: https://wiki.osdev.org/RTC#Programming_the_RTC*/

/*	rtc_init()
 *	INPUT:none
 *	OUTPUT:none
 *	RETURN:none
 *	DESCRIPTION:puts rtc to default frequency
 */

 int32_t rtc_open(const uint8_t* filename){
	 
	 outb(REG_A, RTC_PORT );	/* select Status Register A, and disable NMI (by setting the 0x80 bit)*/
	 outb(RATE, CMOS_PORT );	/*write to cmos*/
	 
	 return 0; 
 	
 }

 /*	rtc_close()
 *	INPUT:fd - does nothing
 *	OUTPUT:none
 *	RETURN:none
 *	DESCRIPTION:does nothing
 */
 int32_t rtc_close(int32_t fd){

 	return 0; 	/*do nothing*/
 }
 /*rtc_read()
 *	INPUT:fd, buf, nbytes, but they dont matter
 *	OUTPUT:none
 *	RETURN:none
 *	DESCRIPTION:read from rtc
 */
 int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	 
	 rtc_flag=0; /*set the rtc flag */
	 while(rtc_flag==0){
		// printf("waiting\n");
	 }
	 // printf("1");
	 return 0;
	 
}
/*	rtc_write()
 *	INPUT:fd, frequency, num of bytes
 *	OUTPUT:none
 *	RETURN:none
 *	DESCRIPTION:changes rtc frequency
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
	  
	if(nbytes != FOUR_BYTES)		/*4 bytes have to be sent evertime, if not return -1 (error)*/
		return -1;
	  
	int rate;
	int save_buf=*((int*)buf); 

	if(save_buf==FREQ_1024 || save_buf==FREQ_2048 || save_buf==FREQ_4096 || save_buf == FREQ_8192){

		save_buf=FREQ_1024; /*limit the freq to 1024*/
	}

	/*set the rate accordingly*/
	switch(save_buf){

		case FREQ_1024:
			rate=RATE_1024;
			break;
		case FREQ_512:
			rate=RATE_512;
			break;
		case FREQ_256:
			rate=RATE_256;
			break;
		case FREQ_128:
			rate=RATE_128;
			break;
		case FREQ_64:
			rate=RATE_64;
			break;
		case FREQ_32:
			rate=RATE_32;
			break;
		case FREQ_16:
			rate=RATE_16;
			break;
		case FREQ_8:
			rate=RATE_8;
			break;
		case FREQ_4:
			rate=RATE_4;
			break;
		case FREQ_2:
			rate=RATE_2;
			break;
		default:
			return -1; /* return -1 in case of none of these frequencies*/
			break;

	}


	outb(REG_A, RTC_PORT );	/* select Status Register A, and disable NMI (by setting the 0x80 bit)*/
	outb(rate, CMOS_PORT ); /*write using that rate */


	return 0;
}
/*	rtc_init()
 *	INPUT:none
 *	OUTPUT:none
 *	RETURN:none
 *	DESCRIPTION:Initializes the rtc
 */
void rtc_int(){

    enable_irq(IRQ_8);
    outb(REG_B,RTC_PORT);		/* select register B, and disable NMI */
    char prev=inb(CMOS_PORT);	/* read the current value of register B */
    outb(REG_B ,RTC_PORT );		/* set the index again (a read will reset the index to register D) */
    outb(prev | PREVIOUS, CMOS_PORT );	/* write the previous value ORed with 0x40. This turns on bit 6 of register B */

}

/*	rtc_handler()
 *	INPUT:none
 *	OUTPUT:	write to video memory
 *	RETURN: none
 *	DESCRIPTION: handles the rtc and writes to vid memory
 */

void rtc_handler(){
	
	outb(REG_C, RTC_PORT);	/* select register C */
	inb(CMOS_PORT);			/* just throw away contents */
	rtc_flag=1;				/*set global variable*/
	send_eoi(IRQ_8);			/* send an eoi to irq 8*/
}	


