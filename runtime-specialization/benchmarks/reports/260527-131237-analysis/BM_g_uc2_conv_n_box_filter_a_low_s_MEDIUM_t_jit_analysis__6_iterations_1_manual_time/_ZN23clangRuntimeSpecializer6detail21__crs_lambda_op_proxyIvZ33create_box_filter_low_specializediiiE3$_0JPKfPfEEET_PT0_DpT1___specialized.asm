<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ33create_box_filter_low_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_>:
	leaq	-7(%rip), %rcx
	movabsq	$-8112, %rax
	addq	%rcx, %rax
	xorl	%ecx, %ecx
	testb	%cl, %cl
	jne	464
	xorl	%ecx, %ecx
	testb	%cl, %cl
	jne	454
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	movabsq	$-496, %rcx
	vmovss	(%rax,%rcx), %xmm0
	movl	$3840, %eax
	xorl	%ecx, %ecx
	testb	%cl, %cl
	jne	256
	movl	$3839, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	jmp	16
	nopl	(%rax)
	incq	%r8
	cmpq	%rax, %r8
	je	376
	movq	%r8, %r9
	imulq	%rax, %r9
	leaq	(%rsi,%r9,4), %r9
	xorl	%r10d, %r10d
	jmp	22
	nopl	(%rax)
	vmulss	%xmm1, %xmm0, %xmm1
	vmovss	%xmm1, (%r9,%r10,4)
	incq	%r10
	cmpq	%rax, %r10
	je	-50
	leal	-2(%r10), %r11d
	cmpl	%ecx, %r11d
	cmovgel	%ecx, %r11d
	movl	%r11d, %ebx
	sarl	$31, %ebx
	andnl	%r11d, %ebx, %r11d
	vxorps	%xmm1, %xmm1, %xmm1
	movl	$4294967294, %ebx
	jmp	21
	nopw	%cs:(%rax,%rax)
	cmpl	$2, %ebx
	leal	1(%rbx), %ebx
	je	-72
	leal	(%rbx,%r8), %ebp
	cmpl	%ecx, %ebp
	cmovgel	%ecx, %ebp
	movl	%ebp, %r14d
	sarl	$31, %r14d
	andnl	%ebp, %r14d, %ebp
	imull	$3840, %ebp, %r14d
	leal	(%r11,%r14), %r15d
	vaddss	(%rdi,%r15,4), %xmm1, %xmm1
	testb	%dl, %dl
	jne	-50
	movl	$4, %ebp
	movl	%r10d, %r15d
	nopw	(%rax,%rax)
	leal	-1(%r15), %r12d
	cmpl	%ecx, %r12d
	cmovgel	%ecx, %r12d
	movl	%r12d, %r13d
	sarl	$31, %r13d
	andnl	%r12d, %r13d, %r12d
	addl	%r14d, %r12d
	vaddss	(%rdi,%r12,4), %xmm1, %xmm1
	cmpl	%ecx, %r15d
	movl	$3839, %r12d
	cmovll	%r15d, %r12d
	movl	%r12d, %r13d
	sarl	$31, %r13d
	andnl	%r12d, %r13d, %r12d
	addl	%r14d, %r12d
	vaddss	(%rdi,%r12,4), %xmm1, %xmm1
	addl	$2, %r15d
	addl	$-2, %ebp
	jne	-75
	jmp	-144
	vxorps	%xmm1, %xmm1, %xmm1
	vmulss	%xmm1, %xmm0, %xmm0
	movl	$3840, %ecx
	andl	$-8, %ecx
	vbroadcastss	%xmm0, %xmm1
	xorl	%edx, %edx
	xorl	%edi, %edi
	jmp	20
	nopl	(%rax,%rax)
	incq	%rdi
	addq	$15360, %rsi
	cmpq	%rax, %rdi
	je	101
	testb	%dl, %dl
	je	13
	xorl	%r8d, %r8d
	jmp	72
	nopl	(%rax,%rax)
	movl	$4, %r8d
	nopw	%cs:(%rax,%rax)
	vmovups	%xmm1, -16(%rsi,%r8,4)
	vmovups	%xmm1, (%rsi,%r8,4)
	addq	$8, %r8
	cmpq	$3844, %r8
	jne	-26
	movq	%rcx, %r8
	cmpl	%eax, %ecx
	je	-81
	nopw	%cs:(%rax,%rax)
	vmovss	%xmm0, (%rsi,%r8,4)
	incq	%r8
	cmpq	$3840, %r8
	jne	-18
	jmp	-116
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
