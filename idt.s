	.file	"idt.c"
	.text
	.globl	keyboard_map
	.data
	.align 32
	.type	keyboard_map, @object
	.size	keyboard_map, 128
keyboard_map:
	.byte	0
	.byte	27
	.byte	49
	.byte	50
	.byte	51
	.byte	52
	.byte	53
	.byte	54
	.byte	55
	.byte	56
	.byte	57
	.byte	48
	.byte	45
	.byte	61
	.byte	8
	.byte	9
	.byte	113
	.byte	119
	.byte	101
	.byte	114
	.byte	116
	.byte	121
	.byte	117
	.byte	105
	.byte	111
	.byte	112
	.byte	91
	.byte	93
	.byte	10
	.byte	0
	.byte	97
	.byte	115
	.byte	100
	.byte	102
	.byte	103
	.byte	104
	.byte	106
	.byte	107
	.byte	108
	.byte	59
	.byte	39
	.byte	96
	.byte	0
	.byte	92
	.byte	122
	.byte	120
	.byte	99
	.byte	118
	.byte	98
	.byte	110
	.byte	109
	.byte	44
	.byte	46
	.byte	47
	.byte	0
	.byte	42
	.byte	0
	.byte	32
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	45
	.byte	0
	.byte	0
	.byte	0
	.byte	43
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.zero	38
	.text
	.globl	write_port
	.type	write_port, @function
write_port:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %edx
	movl	%esi, %eax
	movw	%dx, -4(%rbp)
	movb	%al, -8(%rbp)
	movzwl	-4(%rbp), %eax
#APP
# 42 "src/idt.c" 1
	movw %dx, %ax
# 0 "" 2
#NO_APP
	movzbl	-8(%rbp), %eax
#APP
# 45 "src/idt.c" 1
	mov %al, %al
# 0 "" 2
# 48 "src/idt.c" 1
	out %al, %dx
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	write_port, .-write_port
	.globl	read_port
	.type	read_port, @function
read_port:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %eax
	movw	%ax, -20(%rbp)
	movzwl	-20(%rbp), %eax
#APP
# 54 "src/idt.c" 1
	movw %dx, %ax;
# 0 "" 2
# 57 "src/idt.c" 1
	in %dx, %al
# 0 "" 2
#NO_APP
	movb	%al, -1(%rbp)
	movzbl	-1(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	read_port, .-read_port
	.globl	init_pic
	.type	init_pic, @function
init_pic:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$17, %esi
	movl	$32, %edi
	call	write_port
	movl	$17, %esi
	movl	$160, %edi
	call	write_port
	movl	$32, %esi
	movl	$33, %edi
	call	write_port
	movl	$40, %esi
	movl	$161, %edi
	call	write_port
	movl	$4, %esi
	movl	$33, %edi
	call	write_port
	movl	$2, %esi
	movl	$161, %edi
	call	write_port
	movl	$1, %esi
	movl	$33, %edi
	call	write_port
	movl	$1, %esi
	movl	$161, %edi
	call	write_port
	movl	$255, %esi
	movl	$33, %edi
	call	write_port
	movl	$255, %esi
	movl	$161, %edi
	call	write_port
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	init_pic, .-init_pic
	.globl	handler_complete
	.type	handler_complete, @function
handler_complete:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$32, %esi
	movl	$32, %edi
	call	write_port
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	handler_complete, .-handler_complete
	.globl	timer_handler
	.type	timer_handler, @function
timer_handler:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rcx
	pushq	%rdx
	pushq	%rax
	subq	$8, %rsp
	.cfi_offset 11, -24
	.cfi_offset 10, -32
	.cfi_offset 9, -40
	.cfi_offset 8, -48
	.cfi_offset 5, -56
	.cfi_offset 4, -64
	.cfi_offset 2, -72
	.cfi_offset 1, -80
	.cfi_offset 0, -88
	leaq	8(%rbp), %rax
	movq	%rax, -80(%rbp)
	movl	$0, %eax
	cld
	call	handler_complete
	nop
	addq	$8, %rsp
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%rbp
	.cfi_def_cfa 7, 8
	iretq
	.cfi_endproc
.LFE4:
	.size	timer_handler, .-timer_handler
	.comm	keyboardHandler,8,8
	.section	.rodata
.LC0:
	.string	"status:\n"
.LC1:
	.string	"keycode: "
	.text
	.globl	keyboard_handler
	.type	keyboard_handler, @function
keyboard_handler:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rbx
	pushq	%rcx
	pushq	%rdx
	pushq	%rax
	subq	$32, %rsp
	.cfi_offset 11, -24
	.cfi_offset 10, -32
	.cfi_offset 9, -40
	.cfi_offset 8, -48
	.cfi_offset 5, -56
	.cfi_offset 4, -64
	.cfi_offset 3, -72
	.cfi_offset 2, -80
	.cfi_offset 1, -88
	.cfi_offset 0, -96
	leaq	8(%rbp), %rax
	movq	%rax, -104(%rbp)
	movl	$0, %eax
	cld
	call	handler_complete
	movl	$100, %edi
	call	read_port
	movb	%al, -82(%rbp)
	leaq	.LC0(%rip), %rdi
	call	writeString@PLT
	movzbl	-82(%rbp), %eax
	movl	%eax, %edi
	call	writeInt@PLT
	movl	$96, %edi
	call	read_port
	movb	%al, -83(%rbp)
	leaq	.LC1(%rip), %rdi
	call	writeString@PLT
	movsbl	-83(%rbp), %eax
	movl	%eax, %edi
	call	writeInt@PLT
	cmpb	$0, -83(%rbp)
	js	.L13
	cmpb	$28, -83(%rbp)
	jne	.L10
	movb	$10, -81(%rbp)
	jmp	.L11
.L10:
	movzbl	-83(%rbp), %eax
	movzbl	%al, %eax
	movslq	%eax, %rdx
	leaq	keyboard_map(%rip), %rax
	movzbl	(%rdx,%rax), %eax
	movb	%al, -81(%rbp)
.L11:
	movq	keyboardHandler(%rip), %rax
	testq	%rax, %rax
	je	.L12
	movq	keyboardHandler(%rip), %rbx
	movsbl	-81(%rbp), %eax
	movl	%eax, %edi
	call	*%rbx
	jmp	.L14
.L12:
	movsbl	-81(%rbp), %eax
	movl	%eax, %edi
	call	writeChar@PLT
	jmp	.L14
.L13:
	nop
.L14:
	addq	$32, %rsp
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%rbp
	.cfi_def_cfa 7, 8
	iretq
	.cfi_endproc
.LFE5:
	.size	keyboard_handler, .-keyboard_handler
	.section	.rodata
.LC2:
	.string	"Breakpoint!\n"
	.text
	.globl	breakpoint_handler
	.type	breakpoint_handler, @function
breakpoint_handler:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rcx
	pushq	%rdx
	pushq	%rax
	subq	$24, %rsp
	.cfi_offset 11, -24
	.cfi_offset 10, -32
	.cfi_offset 9, -40
	.cfi_offset 8, -48
	.cfi_offset 5, -56
	.cfi_offset 4, -64
	.cfi_offset 2, -72
	.cfi_offset 1, -80
	.cfi_offset 0, -88
	leaq	8(%rbp), %rax
	movq	%rax, -88(%rbp)
	leaq	.LC2(%rip), %rdi
	cld
	call	writeString@PLT
	nop
	addq	$24, %rsp
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%rbp
	.cfi_def_cfa 7, 8
	iretq
	.cfi_endproc
.LFE6:
	.size	breakpoint_handler, .-breakpoint_handler
	.section	.rodata
.LC3:
	.string	"PANIC: double fault!\n"
.LC4:
	.string	"Error code "
	.align 8
.LC5:
	.string	"\nPress any key to continue...\n"
	.text
	.globl	double_fault_handler
	.type	double_fault_handler, @function
double_fault_handler:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rcx
	pushq	%rdx
	pushq	%rax
	subq	$24, %rsp
	.cfi_offset 11, -24
	.cfi_offset 10, -32
	.cfi_offset 9, -40
	.cfi_offset 8, -48
	.cfi_offset 5, -56
	.cfi_offset 4, -64
	.cfi_offset 2, -72
	.cfi_offset 1, -80
	.cfi_offset 0, -88
	leaq	16(%rbp), %rax
	movq	%rax, -88(%rbp)
	movq	8(%rbp), %rax
	movq	%rax, -96(%rbp)
	leaq	.LC3(%rip), %rdi
	cld
	call	writeString@PLT
	leaq	.LC4(%rip), %rdi
	call	writeString@PLT
	movq	-96(%rbp), %rax
	addl	$48, %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	writeChar@PLT
	leaq	.LC5(%rip), %rdi
	call	writeString@PLT
#APP
# 143 "src/idt.c" 1
	hlt
# 0 "" 2
#NO_APP
	nop
	addq	$24, %rsp
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%rbp
	.cfi_def_cfa 7, 8
	leaq	8(%rsp), %rsp
	iretq
	.cfi_endproc
.LFE7:
	.size	double_fault_handler, .-double_fault_handler
	.section	.rodata
.LC6:
	.string	"Page fault!\n"
.LC7:
	.string	"\n"
	.text
	.globl	page_fault_handler
	.type	page_fault_handler, @function
page_fault_handler:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rcx
	pushq	%rdx
	pushq	%rax
	subq	$24, %rsp
	.cfi_offset 11, -24
	.cfi_offset 10, -32
	.cfi_offset 9, -40
	.cfi_offset 8, -48
	.cfi_offset 5, -56
	.cfi_offset 4, -64
	.cfi_offset 2, -72
	.cfi_offset 1, -80
	.cfi_offset 0, -88
	leaq	16(%rbp), %rax
	movq	%rax, -88(%rbp)
	movq	8(%rbp), %rax
	movq	%rax, -96(%rbp)
	leaq	.LC6(%rip), %rdi
	cld
	call	writeString@PLT
	leaq	.LC4(%rip), %rdi
	call	writeString@PLT
	movq	-96(%rbp), %rax
	addl	$48, %eax
	movsbl	%al, %eax
	movl	%eax, %edi
	call	writeChar@PLT
	leaq	.LC7(%rip), %rdi
	call	writeString@PLT
	nop
	addq	$24, %rsp
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%rbp
	.cfi_def_cfa 7, 8
	leaq	8(%rsp), %rsp
	iretq
	.cfi_endproc
.LFE8:
	.size	page_fault_handler, .-page_fault_handler
	.section	.rodata
.LC8:
	.string	"interrupt!\n"
	.text
	.globl	random_handler
	.type	random_handler, @function
random_handler:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rcx
	pushq	%rdx
	pushq	%rax
	subq	$24, %rsp
	.cfi_offset 11, -24
	.cfi_offset 10, -32
	.cfi_offset 9, -40
	.cfi_offset 8, -48
	.cfi_offset 5, -56
	.cfi_offset 4, -64
	.cfi_offset 2, -72
	.cfi_offset 1, -80
	.cfi_offset 0, -88
	leaq	8(%rbp), %rax
	movq	%rax, -88(%rbp)
	leaq	.LC8(%rip), %rdi
	cld
	call	writeString@PLT
	nop
	addq	$24, %rsp
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%rbp
	.cfi_def_cfa 7, 8
	iretq
	.cfi_endproc
.LFE9:
	.size	random_handler, .-random_handler
	.globl	register_handler
	.type	register_handler, @function
register_handler:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, keyboardHandler(%rip)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	register_handler, .-register_handler
	.globl	unregister_handler
	.type	unregister_handler, @function
unregister_handler:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	$0, keyboardHandler(%rip)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	unregister_handler, .-unregister_handler
	.globl	setup_idt_entry
	.type	setup_idt_entry, @function
setup_idt_entry:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, %eax
	movw	%ax, -20(%rbp)
	movq	-16(%rbp), %rax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, (%rax)
	movq	-8(%rbp), %rax
	movw	$8, 2(%rax)
	movq	-8(%rbp), %rax
	movzwl	-20(%rbp), %edx
	movw	%dx, 4(%rax)
	movq	-16(%rbp), %rax
	shrq	$16, %rax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movw	%dx, 6(%rax)
	movq	-16(%rbp), %rax
	shrq	$32, %rax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%edx, 8(%rax)
	movq	-8(%rbp), %rax
	movl	$0, 12(%rax)
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	setup_idt_entry, .-setup_idt_entry
	.section	.rodata
	.align 8
.LC9:
	.string	"\nhere's what happens when I read 0x21: "
	.align 8
.LC10:
	.string	"\nhere's what happens when I read 0x20: "
	.align 8
.LC11:
	.string	"\nhere's what happens when I read 0x64: "
	.text
	.globl	init_interrupts
	.type	init_interrupts, @function
init_interrupts:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	$0, %eax
	call	init_pic
	movw	$3584, -2(%rbp)
	movw	$0, -2(%rbp)
	movw	$-29184, -4(%rbp)
	movw	$-28928, -6(%rbp)
	movl	$0, -17(%rbp)
	movw	$0, -23(%rbp)
	movzwl	-2(%rbp), %eax
	movw	%ax, -21(%rbp)
	movw	$0, -19(%rbp)
	movw	$0, -25(%rbp)
	movl	$0, -13(%rbp)
	movzwl	-2(%rbp), %eax
	orw	$-32512, %ax
	movw	%ax, -8(%rbp)
	movzwl	-4(%rbp), %edx
	leaq	random_handler(%rip), %rax
	movq	%rax, %rsi
	leaq	784+idt(%rip), %rdi
	call	setup_idt_entry
	movzwl	-4(%rbp), %edx
	leaq	breakpoint_handler(%rip), %rax
	movq	%rax, %rsi
	leaq	48+idt(%rip), %rdi
	call	setup_idt_entry
	movzwl	-4(%rbp), %edx
	leaq	double_fault_handler(%rip), %rax
	movq	%rax, %rsi
	leaq	128+idt(%rip), %rdi
	call	setup_idt_entry
	movzwl	-4(%rbp), %edx
	leaq	page_fault_handler(%rip), %rax
	movq	%rax, %rsi
	leaq	224+idt(%rip), %rdi
	call	setup_idt_entry
	movzwl	-6(%rbp), %edx
	leaq	timer_handler(%rip), %rax
	movq	%rax, %rsi
	leaq	512+idt(%rip), %rdi
	call	setup_idt_entry
	movzwl	-4(%rbp), %edx
	leaq	keyboard_handler(%rip), %rax
	movq	%rax, %rsi
	leaq	528+idt(%rip), %rdi
	call	setup_idt_entry
	call	load_idt@PLT
	movl	$252, %esi
	movl	$33, %edi
	call	write_port
	movl	$255, %esi
	movl	$161, %edi
	call	write_port
	movl	$33, %edi
	call	read_port
	movb	%al, -9(%rbp)
	leaq	.LC9(%rip), %rdi
	call	writeString@PLT
	movzbl	-9(%rbp), %eax
	movl	%eax, %edi
	call	writeInt@PLT
	movl	$32, %edi
	call	read_port
	movb	%al, -9(%rbp)
	leaq	.LC10(%rip), %rdi
	call	writeString@PLT
	movzbl	-9(%rbp), %eax
	movl	%eax, %edi
	call	writeInt@PLT
	movl	$100, %edi
	call	read_port
	movb	%al, -9(%rbp)
	leaq	.LC11(%rip), %rdi
	call	writeString@PLT
	movzbl	-9(%rbp), %eax
	movl	%eax, %edi
	call	writeInt@PLT
	movl	$10, %edi
	call	writeChar@PLT
	movl	$10, %edi
	call	writeChar@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	init_interrupts, .-init_interrupts
	.ident	"GCC: (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",@progbits
