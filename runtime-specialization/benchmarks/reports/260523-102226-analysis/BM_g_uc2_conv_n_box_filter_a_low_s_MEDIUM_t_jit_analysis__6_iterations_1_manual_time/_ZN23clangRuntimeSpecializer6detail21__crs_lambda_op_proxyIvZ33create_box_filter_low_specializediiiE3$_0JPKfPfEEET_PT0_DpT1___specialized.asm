<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ33create_box_filter_low_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-8210, %rcx
	addq	%rax, %rcx
	movq	%rsi, -24(%rsp)
	leaq	4(%rsi), %r11
	leaq	15364(%rdi), %rax
	movq	%rax, -32(%rsp)
	leaq	30724(%rdi), %rax
	movq	%rax, -40(%rsp)
	xorl	%r14d, %r14d
	movl	$3837, %r15d
	movl	$3841, %r9d
	vxorps	%xmm0, %xmm0, %xmm0
	movabsq	$-408, %rax
	vmovss	(%rcx,%rax), %xmm1
	movq	%rdi, %rax
	nopw	%cs:(%rax,%rax)
	movq	%rax, -48(%rsp)
	cmpl	$3, %r14d
	movl	$2, %eax
	cmovgel	%r14d, %eax
	imull	$3840, %eax, %esi
	addl	$4294959616, %esi
	cmpl	$2, %r14d
	movl	$1, %ecx
	cmovgel	%r14d, %ecx
	imull	$3840, %ecx, %edx
	addl	$4294963456, %edx
	cmpl	$3838, %r14d
	movl	$3838, %r8d
	cmovll	%r14d, %r8d
	imulq	$15360, %r8, %r8
	addq	-32(%rsp), %r8
	movq	%r8, -8(%rsp)
	cmpl	$3837, %r14d
	movl	$3837, %r8d
	cmovll	%r14d, %r8d
	imulq	$15360, %r8, %r12
	addq	-40(%rsp), %r12
	imulq	$15360, %r14, %r10
	cmpl	$3841, %eax
	cmovgel	%r9d, %eax
	imull	$3840, %eax, %r8d
	leal	-7680(%r8), %r13d
	cmpl	$3840, %ecx
	movl	$3840, %eax
	cmovgel	%eax, %ecx
	imull	$3840, %ecx, %ecx
	leal	-3840(%rcx), %r9d
	movl	%r14d, %eax
	sarl	$31, %eax
	andnl	%r14d, %eax, %eax
	cmpl	$3839, %eax
	movl	$3839, %ebx
	cmovgel	%ebx, %eax
	imull	$3840, %eax, %ebx
	testl	%r14d, %r14d
	movl	$4294967295, %eax
	cmovnsl	%r14d, %eax
	cmpl	$3838, %eax
	movl	$3838, %ebp
	cmovgel	%ebp, %eax
	imull	$3840, %eax, %eax
	vmovss	(%rdi,%r13,4), %xmm2
	vaddss	%xmm0, %xmm2, %xmm3
	vaddss	%xmm2, %xmm3, %xmm3
	vaddss	%xmm2, %xmm3, %xmm2
	vaddss	-30716(%rdi,%r8,4), %xmm2, %xmm2
	vaddss	-30712(%rdi,%r8,4), %xmm2, %xmm2
	leal	3840(%rax), %ebp
	cmpl	$-1, %r14d
	vmovss	(%rdi,%r9,4), %xmm3
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	-15356(%rdi,%rcx,4), %xmm2, %xmm2
	vaddss	-15352(%rdi,%rcx,4), %xmm2, %xmm2
	movl	$4294967294, %ecx
	movq	%r14, -16(%rsp)
	cmovgel	%r14d, %ecx
	cmpl	$3837, %ecx
	cmovgel	%r15d, %ecx
	imull	$3840, %ecx, %r8d
	vmovss	(%rdi,%rbx,4), %xmm3
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	4(%rdi,%rbx,4), %xmm2, %xmm2
	vaddss	8(%rdi,%rbx,4), %xmm2, %xmm2
	vmovss	(%rdi,%rbp,4), %xmm3
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	cltq
	vaddss	15364(%rdi,%rax,4), %xmm2, %xmm2
	vaddss	15368(%rdi,%rax,4), %xmm2, %xmm2
	leal	7680(%r8), %ecx
	vmovss	(%rdi,%rcx,4), %xmm3
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm3, %xmm2, %xmm2
	movslq	%r8d, %rax
	vaddss	30724(%rdi,%rax,4), %xmm2, %xmm2
	vaddss	30728(%rdi,%rax,4), %xmm2, %xmm2
	leaq	(%rdi,%rsi,4), %rsi
	leaq	(%rdi,%rdx,4), %rdx
	vmulss	%xmm1, %xmm2, %xmm2
	movq	-24(%rsp), %rax
	vmovss	%xmm2, (%rax,%r10)
	movq	$-3839, %r10
	nopl	(%rax)
	movq	%r11, %r15
	leal	3840(%r10), %r11d
	cmpl	$3, %r11d
	movl	$2, %r8d
	cmovgel	%r11d, %r8d
	cmpl	$3841, %r8d
	movl	$3841, %eax
	cmovgel	%eax, %r8d
	leal	-2(%r8,%r13), %eax
	vaddss	(%rdi,%rax,4), %xmm0, %xmm2
	vaddss	15356(%rsi,%r10,4), %xmm2, %xmm2
	vaddss	15360(%rsi,%r10,4), %xmm2, %xmm2
	cmpl	$3838, %r11d
	movl	$3838, %eax
	cmovll	%r11d, %eax
	leal	(%rax,%r13), %r14d
	incl	%r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	cmpl	$3837, %r11d
	movl	$3837, %r14d
	cmovael	%r14d, %r11d
	leal	2(%r11,%r13), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	-2(%r8,%r9), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	vaddss	15356(%rdx,%r10,4), %xmm2, %xmm2
	vaddss	15360(%rdx,%r10,4), %xmm2, %xmm2
	leal	1(%rax,%r9), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	2(%r11,%r9), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	-2(%r8,%rbx), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	movq	-48(%rsp), %r14
	vaddss	15356(%r14,%r10,4), %xmm2, %xmm2
	vaddss	15360(%r14,%r10,4), %xmm2, %xmm2
	leal	1(%rax,%rbx), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	2(%r11,%rbx), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	-2(%r8,%rbp), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	movq	-8(%rsp), %r14
	vaddss	15352(%r14,%r10,4), %xmm2, %xmm2
	vaddss	15356(%r14,%r10,4), %xmm2, %xmm2
	leal	1(%rax,%rbp), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	2(%r11,%rbp), %r14d
	vaddss	(%rdi,%r14,4), %xmm2, %xmm2
	leal	-2(%r8,%rcx), %r8d
	vaddss	(%rdi,%r8,4), %xmm2, %xmm2
	vaddss	15352(%r12,%r10,4), %xmm2, %xmm2
	vaddss	15356(%r12,%r10,4), %xmm2, %xmm2
	leal	1(%rax,%rcx), %eax
	vaddss	(%rdi,%rax,4), %xmm2, %xmm2
	leal	2(%r11,%rcx), %eax
	movq	%r15, %r11
	vaddss	(%rdi,%rax,4), %xmm2, %xmm2
	vmulss	%xmm1, %xmm2, %xmm2
	vmovss	%xmm2, 15356(%r15,%r10,4)
	incq	%r10
	jne	-372
	movq	-16(%rsp), %r14
	incq	%r14
	addq	$15360, %r11
	movq	-48(%rsp), %rax
	addq	$15360, %rax
	cmpq	$3840, %r14
	movl	$3837, %r15d
	movl	$3841, %r9d
	jne	-919
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
