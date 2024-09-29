.text

.globl rtc_wrapper
.align 4

# rtc_wrapper()
# INPUT : none  
# OUTPUT : none
# DESCRIPTION : calls the rtc handler
rtc_wrapper:
	pushal
	cld
	call rtc_handler
	popal
	iret
