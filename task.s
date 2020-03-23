	.file	"task.c"
	.text
	.comm	running_task,8,8
	.comm	main_task,104,32
	.comm	task1,104,32
	.comm	task2,104,32
	.data
	.align 4
	.type	task_id, @object
	.size	task_id, 4
task_id:
	.long	1
	.section	.rodata
.LC0:
	.string	"In other task\n"
.LC1:
	.string	"Switching back to main\n"
	.text
	.type	other_main, @function
other_main:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	call	yield
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	other_main, .-other_main
	.section	.rodata
.LC2:
	.string	"task2\n"
	.text
	.type	other_main2, @function
other_main2:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$.LC2, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	call	yield
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	other_main2, .-other_main2
	.globl	create_task
	.type	create_task, @function
create_task:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	%rcx, -48(%rbp)
	movq	-24(%rbp), %rax
	movq	$0, (%rax)
	movq	-24(%rbp), %rax
	movq	$0, 8(%rax)
	movq	-24(%rbp), %rax
	movq	$0, 16(%rax)
	movq	-24(%rbp), %rax
	movq	$0, 24(%rax)
	movq	-24(%rbp), %rax
	movq	$0, 32(%rax)
	movq	-24(%rbp), %rax
	movq	$0, 40(%rax)
	movq	-24(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rdx, 72(%rax)
	movq	-32(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 64(%rax)
	movq	-48(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 80(%rax)
	movl	$0, %eax
	call	allocate_page
	addq	$4096, %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movq	64(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, 48(%rax)
	movq	-24(%rbp), %rax
	movq	$0, 88(%rax)
	movl	task_id(%rip), %eax
	leal	1(%rax), %edx
	movl	%edx, task_id(%rip)
	movl	%eax, %edx
	movq	-24(%rbp), %rax
	movl	%edx, 96(%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	create_task, .-create_task
	.section	.rodata
.LC3:
	.string	"cr3: %x, flags: %x\n"
	.text
	.globl	init_tasking
	.type	init_tasking, @function
init_tasking:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
/APP
# 46 "src/kernel/task.c" 1
	mov %cr3, %rax; mov %rax, main_task+80(%rip);
# 0 "" 2
# 48 "src/kernel/task.c" 1
	pushf; mov (%rsp), %rax; mov %rax, main_task+72(%rip); popf;
# 0 "" 2
/NO_APP
	movq	main_task+72(%rip), %rdx
	movq	main_task+80(%rip), %rax
	movq	%rax, %rsi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	movq	main_task+80(%rip), %rax
	movq	%rax, %rdx
	movq	main_task+72(%rip), %rax
	movq	%rdx, %rcx
	movq	%rax, %rdx
	movl	$other_main, %esi
	movl	$task1, %edi
	call	create_task
	movq	$task1, main_task+88(%rip)
	movq	$main_task, task1+88(%rip)
	movq	$main_task, running_task(%rip)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	init_tasking, .-init_tasking
	.section	.rodata
	.align 8
.LC4:
	.string	"rax: %u, flags: %x, rip: %x, cr3: %x, rsp: %x\n"
	.text
	.globl	dump_task
	.type	dump_task, @function
dump_task:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	48(%rax), %rdi
	movq	-8(%rbp), %rax
	movq	80(%rax), %rsi
	movq	-8(%rbp), %rax
	movq	64(%rax), %rcx
	movq	-8(%rbp), %rax
	movq	72(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	%rdi, %r9
	movq	%rsi, %r8
	movq	%rax, %rsi
	movl	$.LC4, %edi
	movl	$0, %eax
	call	printf
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	dump_task, .-dump_task
	.section	.rodata
.LC5:
	.string	"Saved rsp: %x\n"
	.align 8
.LC6:
	.string	"switching from %u to %u: last=%p, run=%p, run addr %p\n"
	.text
	.globl	yield
	.type	yield, @function
yield:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	running_task(%rip), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rdx
/APP
# 6 "src/kernel/x86.h" 1
	mov %rsp, %rax
	add $24, %rax
	mov %rax, 48(%rdx)
# 0 "" 2
/NO_APP
	nop
	movq	running_task(%rip), %rax
	movq	48(%rax), %rax
	movq	%rax, %rsi
	movl	$.LC5, %edi
	movl	$0, %eax
	call	printf
	movq	running_task(%rip), %rax
	movq	%rax, -8(%rbp)
	movq	running_task(%rip), %rax
	movq	88(%rax), %rax
	movq	%rax, running_task(%rip)
	movq	running_task(%rip), %rax
	movq	64(%rax), %rsi
	movq	running_task(%rip), %rax
	movq	%rax, %rdi
	movq	-8(%rbp), %rcx
	movq	running_task(%rip), %rax
	movl	96(%rax), %edx
	movq	-8(%rbp), %rax
	movl	96(%rax), %eax
	movq	%rsi, %r9
	movq	%rdi, %r8
	movl	%eax, %esi
	movl	$.LC6, %edi
	movl	$0, %eax
	call	printf
	movq	running_task(%rip), %rax
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	switch_task
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	yield, .-yield
	.ident	"GCC: (GNU) 9.3.0"
