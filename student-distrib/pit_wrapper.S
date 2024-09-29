.text

.globl pit_wrapper
.align 4


# pit_wrapper()
# INPUT : none  
# OUTPUT : none
# DESCRIPTION : calls the pit handler


pit_wrapper:
	pushl %eax
	pushl %ecx
	pushl %edx
	pushl %ebx
	# pushl %esp
	# pushl %ebp
	pushl %esi
	pushl %edi
	//cli
	call pit_handler
	//sti
	popl %edi
	popl %esi
	# popl %ebp
	# popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	popl %eax
	iret
	
