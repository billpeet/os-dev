	.file	"long_mode_loader.c"
	.text
	.section	.rodata
.LC0:
	.string	"ERR: "
	.text
	.globl	error
	.type	error, @function
error:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$20, %esp
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	8(%ebp), %edx
	movb	%dl, -20(%ebp)
	movl	$753664, -8(%ebp)
	leal	.LC0@GOTOFF(%eax), %eax
	movl	%eax, -4(%ebp)
	movl	$0, -12(%ebp)
	jmp	.L2
.L3:
	movl	-12(%ebp), %edx
	movl	-4(%ebp), %eax
	addl	%edx, %eax
	movl	-12(%ebp), %edx
	addl	%edx, %edx
	movl	%edx, %ecx
	movl	-8(%ebp), %edx
	addl	%ecx, %edx
	movzbl	(%eax), %eax
	movb	%al, (%edx)
	movl	-12(%ebp), %eax
	addl	%eax, %eax
	leal	1(%eax), %edx
	movl	-8(%ebp), %eax
	addl	%edx, %eax
	movb	$79, (%eax)
	addl	$1, -12(%ebp)
.L2:
	movl	-12(%ebp), %edx
	movl	-4(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	testb	%al, %al
	jne	.L3
	movzbl	-20(%ebp), %eax
	leal	48(%eax), %edx
	movl	-12(%ebp), %eax
	addl	%eax, %eax
	movl	%eax, %ecx
	movl	-8(%ebp), %eax
	addl	%ecx, %eax
	movb	%dl, (%eax)
	movl	-12(%ebp), %eax
	addl	%eax, %eax
	leal	1(%eax), %edx
	movl	-8(%ebp), %eax
	addl	%edx, %eax
	movb	$79, (%eax)
#APP
# 14 "src/long_mode_loader.c" 1
	hlt
# 0 "" 2
#NO_APP
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	error, .-error
	.comm	p4_table,4096,4096
	.comm	p3_table,4096,4096
	.comm	p2_table,4096,4096
	.globl	init_page_tables
	.type	init_page_tables, @function
init_page_tables:
.LFB1:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	subl	$16, %esp
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	p3_table@GOT(%eax), %edx
	movl	%edx, -16(%ebp)
	orl	$3, -16(%ebp)
	movl	-16(%ebp), %edx
	movl	$0, %ecx
	movl	p4_table@GOT(%eax), %ebx
	movl	%edx, (%ebx)
	movl	%ecx, 4(%ebx)
	movl	p2_table@GOT(%eax), %edx
	movl	%edx, -12(%ebp)
	orl	$3, -12(%ebp)
	movl	-12(%ebp), %edx
	movl	$0, %ecx
	movl	p3_table@GOT(%eax), %ebx
	movl	%edx, (%ebx)
	movl	%ecx, 4(%ebx)
	movl	$0, -20(%ebp)
	jmp	.L5
.L6:
	movl	-20(%ebp), %edx
	sall	$21, %edx
	orb	$-125, %dl
	movl	$0, %ecx
	movl	p2_table@GOT(%eax), %ebx
	movl	-20(%ebp), %esi
	movl	%edx, (%ebx,%esi,8)
	movl	%ecx, 4(%ebx,%esi,8)
	addl	$1, -20(%ebp)
.L5:
	cmpl	$511, -20(%ebp)
	jbe	.L6
	nop
	addl	$16, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	init_page_tables, .-init_page_tables
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
.LFB2:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
.LFE2:
	.ident	"GCC: (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",@progbits
