/* syscall_wrapper */

#define NUM_SYSCALLS	10
#define ERROR_NUM		-1

.text

.globl syscall_wrapper
.align 4

syscall_wrapper:
	//pushal
	pushl %eax
	pushl %ecx
	pushl %edx
	pushl %ebx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi

	cld
	/* syscalls are numbered 1-10 */
	cmpl $NUM_SYSCALLS, %eax
	ja badcode
	// calls start at 1
	cmpl $1, %eax
	jb badcode
	/* push areguments */
	pushl %edx
	pushl %ecx
	pushl %ebx
	sti
	jmp *syscall_table(,%eax,4)
	//cli
	
	syscall_table:
	.long badcode, halt, execute, read, write, open, close, getargs, vidmap
	
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	/* avoid popping eax - return val */
	addl $4, %esp

	iret
	
badcode:
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	movl $ERROR_NUM, %eax
	iret


halt:
	mov %bl, %cl
	xorl %ebx, %ebx
	mov %cl, %bl
	call system_halt
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret

execute:
	call system_execute
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret

read:
	call system_read
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	//popal
	iret
	
write:
	call system_write
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret

open:
	call system_open
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret
	
close:
	call system_close
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret

getargs:
	call system_getargs
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret
	
vidmap:
	call system_vidmap
	addl $12, %esp
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	addl $4, %esp
	iret

/*	
set_handler:
	call system_set_handler
	popal
	iret
	
sigreturn:
	call system_sigreturn
	popal
	iret
*/
