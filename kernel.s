	.file	"kernel.c"
	.text
	.type	outb, @function
outb:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %eax
	movl	%esi, %edx
	movw	%ax, -4(%rbp)
	movl	%edx, %eax
	movb	%al, -8(%rbp)
	movzbl	-8(%rbp), %eax
	movzwl	-4(%rbp), %edx
/APP
# 43 "src/kernel/kernel.h" 1
	out %al, %dx
# 0 "" 2
/NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	outb, .-outb
	.globl	hlt
	.type	hlt, @function
hlt:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
/APP
# 17 "src/kernel/kernel.c" 1
	hlt
# 0 "" 2
/NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	hlt, .-hlt
	.section	.rodata
.LC0:
	.string	"PANIC!\n"
.LC1:
	.string	"Error code %u\n"
	.text
	.globl	panic
	.type	panic, @function
panic:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, %eax
	movb	%al, -4(%rbp)
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movzbl	-4(%rbp), %eax
	movl	%eax, %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	call	hlt
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	panic, .-panic
	.globl	reboot
	.type	reboot, @function
reboot:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
/APP
# 31 "src/kernel/kernel.c" 1
	cli
# 0 "" 2
/NO_APP
.L6:
	movl	$100, %edi
	call	read_port
	movb	%al, -1(%rbp)
	movzbl	-1(%rbp), %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	.L5
	movl	$96, %edi
	call	read_port
.L5:
	movzbl	-1(%rbp), %eax
	andl	$1, %eax
	testl	%eax, %eax
	jne	.L6
	movl	$254, %esi
	movl	$100, %edi
	call	write_port
.L7:
/APP
# 41 "src/kernel/kernel.c" 1
	hlt
# 0 "" 2
/NO_APP
	jmp	.L7
	.cfi_endproc
.LFE8:
	.size	reboot, .-reboot
	.section	.rodata
.LC2:
	.string	"Welcome to PeetOS\n"
.LC3:
	.string	"Switching...\n"
.LC4:
	.string	"Current rsp: \n"
.LC5:
	.string	"returned to main task\n"
.LC6:
	.string	"switching to shell!\n"
	.text
	.globl	kmain
	.type	kmain, @function
kmain:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	addq	$-128, %rsp
	movq	%rdi, -120(%rbp)
	movl	$0, %eax
	call	clearScreen
	movl	$.LC2, %edi
	movl	$0, %eax
	call	printf
	movq	-120(%rbp), %rax
	movq	%rax, %rdi
	call	init_boot_info
	movl	$0, %eax
	call	init_frame_allocator
	call	init_interrupts
	movl	$64, %esi
	movl	$502, %edi
	call	outb
	call	init_heap
	movl	$0, %eax
	call	init_tasking
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	movl	$.LC4, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	call	print_task
	movl	$0, %eax
	call	yield
	movl	$.LC5, %edi
	movl	$0, %eax
	call	printf
	movq	main_task+80(%rip), %rax
	movq	%rax, %rcx
	movq	main_task+72(%rip), %rdx
	leaq	-112(%rbp), %rax
	movl	$shell, %esi
	movq	%rax, %rdi
	call	create_task
	leaq	-112(%rbp), %rax
	movq	%rax, main_task+88(%rip)
	movq	$main_task, -24(%rbp)
	movl	$.LC6, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	call	yield
.L9:
	movl	$0, %eax
	call	hlt
	jmp	.L9
	.cfi_endproc
.LFE9:
	.size	kmain, .-kmain
	.ident	"GCC: (GNU) 9.3.0"
