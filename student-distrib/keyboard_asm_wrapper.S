/* keyboard_wrapper */

.text


# keyboard_wrapper()
# INPUT : none  
# OUTPUT : none
# DESCRIPTION : calls the keyboard handler


.globl keyboard_wrapper
.align 4

keyboard_wrapper:
	pushal
	cld
	call keyboard_handler
	popal
	iret

