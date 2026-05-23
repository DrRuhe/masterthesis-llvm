<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_struct_sort_tradeoff_specializediiE3$_0JPvlEEET_PT0_DpT1_>:
	cmpq	$2, %rsi
	jl	1676
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$1240, %rsp
	movq	$0, 208(%rsp)
	decq	%rsi
	movq	%rsi, 216(%rsp)
	leaq	16(%rdi), %r11
	leaq	32(%rdi), %rax
	movq	%rax, -80(%rsp)
	leaq	48(%rdi), %rax
	movq	%rax, -88(%rsp)
	leaq	64(%rdi), %rax
	movq	%rax, -96(%rsp)
	leaq	80(%rdi), %rax
	movq	%rax, -104(%rsp)
	leaq	96(%rdi), %rax
	movq	%rax, -112(%rsp)
	leaq	112(%rdi), %rax
	movq	%rax, -120(%rsp)
	leaq	128(%rdi), %rax
	movq	%rax, -128(%rsp)
	movl	$1, %ecx
	movq	%r11, -72(%rsp)
	jmp	27
	nopw	%cs:(%rax,%rax)
	movl	%ecx, %r15d
	movl	%r15d, %ecx
	testl	%r15d, %r15d
	jle	1510
	leal	-1(%rcx), %r15d
	movq	%r15, %r8
	shlq	$4, %r8
	movq	208(%rsp,%r8), %rbx
	movq	216(%rsp,%r8), %r14
	movq	%rbx, %rsi
	subq	%r14, %rsi
	jge	-47
	movq	%r14, %rdx
	subq	%rbx, %rdx
	cmpq	$15, %rdx
	jg	142
	movq	%rbx, %r12
	andq	$7, %rdx
	je	497
	movq	%rbx, %rcx
	shlq	$4, %rcx
	addq	%r11, %rcx
	movq	%rbx, %r12
	xorl	%r8d, %r8d
	jmp	31
	nopw	%cs:(%rax,%rax)
	incq	%r8
	addq	$16, %rcx
	cmpq	%rdx, %r8
	je	448
	cmpq	%rbx, %r12
	leaq	1(%r12), %r12
	jl	-26
	movq	%rcx, %r9
	movq	%r12, %rax
	vmovsd	-16(%r9), %xmm0
	vucomisd	(%r9), %xmm0
	jbe	-45
	decq	%rax
	leaq	-16(%r9), %r10
	vmovups	(%r10), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%r9), %xmm0
	vmovups	%xmm0, (%r10)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%r9)
	movq	%r10, %r9
	cmpq	%rbx, %rax
	jg	-60
	jmp	-94
	nop
	shrq	%rdx
	addq	%rbx, %rdx
	movq	%rbx, %rsi
	shlq	$4, %rsi
	leaq	(%rdi,%rsi), %rax
	shlq	$4, %rdx
	leaq	(%rdi,%rdx), %r10
	vmovsd	(%rdi,%rsi), %xmm0
	vucomisd	(%rdi,%rdx), %xmm0
	jbe	34
	vmovups	(%rax), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%r10), %xmm0
	vmovups	%xmm0, (%rax)
	vmovaps	-48(%rsp), %xmm0
	vmovups	%xmm0, (%r10)
	vmovsd	(%rax), %xmm0
	movq	%r14, %rsi
	shlq	$4, %rsi
	leaq	(%rdi,%rsi), %rdx
	vmovsd	(%rdi,%rsi), %xmm1
	vucomisd	%xmm1, %xmm0
	jbe	32
	vmovups	(%rax), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rdx), %xmm0
	vmovups	%xmm0, (%rax)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rdx)
	vmovsd	(%rdx), %xmm1
	vmovsd	(%r10), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	30
	vmovups	(%r10), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rdx), %xmm0
	vmovups	%xmm0, (%r10)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rdx)
	leaq	(%rsp,%r8), %rdx
	addq	$208, %rdx
	leaq	-1(%r14), %r9
	movq	%r9, %rax
	shlq	$4, %rax
	leaq	(%rdi,%rax), %rsi
	vmovups	(%r10), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rdi,%rax), %xmm0
	vmovups	%xmm0, (%r10)
	vmovaps	-48(%rsp), %xmm0
	vmovups	%xmm0, (%rdi,%rax)
	movq	%rbx, %r10
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rsi), %xmm0
	leaq	1(%r10), %r8
	shlq	$4, %r10
	addq	%rdi, %r10
	movq	%r10, %r15
	nopw	%cs:(%rax,%rax)
	incq	%r8
	vucomisd	16(%r15), %xmm0
	leaq	16(%r15), %r15
	ja	-15
	leaq	-1(%r8), %r10
	movq	%r9, %rax
	shlq	$4, %rax
	addq	%rdi, %rax
	nopl	(%rax)
	decq	%r9
	vmovsd	-16(%rax), %xmm1
	addq	$-16, %rax
	vucomisd	%xmm0, %xmm1
	ja	-18
	cmpq	%r9, %r10
	jge	837
	vmovups	(%r15), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r15)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	jmp	-123
	nopl	(%rax,%rax)
	cmpq	$-8, %rsi
	ja	-583
	movq	%r15, -64(%rsp)
	movq	%r12, %r13
	shlq	$4, %r13
	leaq	(%r11,%r13), %r15
	movq	-80(%rsp), %rax
	leaq	(%rax,%r13), %rcx
	movq	-88(%rsp), %rax
	leaq	(%rax,%r13), %rdx
	movq	-96(%rsp), %rax
	leaq	(%rax,%r13), %rsi
	movq	-104(%rsp), %rax
	leaq	(%rax,%r13), %r8
	movq	-112(%rsp), %rax
	leaq	(%rax,%r13), %r9
	movq	-120(%rsp), %rax
	leaq	(%rax,%r13), %r10
	addq	-128(%rsp), %r13
	movq	%r14, -56(%rsp)
	jmp	50
	nopl	(%rax)
	subq	$-128, %r15
	subq	$-128, %rcx
	subq	$-128, %rdx
	subq	$-128, %rsi
	subq	$-128, %r8
	subq	$-128, %r9
	subq	$-128, %r10
	subq	$-128, %r13
	movq	-56(%rsp), %r14
	cmpq	%r14, %r12
	je	751
	leaq	1(%r12), %r11
	cmpq	%rbx, %r12
	jl	67
	movq	%r15, %rbp
	movq	%r11, %rax
	nop
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%rax
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %rax
	jg	-59
	leaq	2(%r12), %rax
	cmpq	%rbx, %r11
	jl	70
	movq	%rcx, %rbp
	movq	%rax, %r11
	nopl	(%rax,%rax)
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%r11
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %r11
	jg	-59
	leaq	3(%r12), %r11
	cmpq	%rbx, %rax
	jl	70
	movq	%rdx, %rbp
	movq	%r11, %rax
	nopl	(%rax,%rax)
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%rax
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %rax
	jg	-59
	leaq	4(%r12), %rax
	cmpq	%rbx, %r11
	jl	70
	movq	%rsi, %rbp
	movq	%rax, %r11
	nopl	(%rax,%rax)
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%r11
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %r11
	jg	-59
	leaq	5(%r12), %r11
	cmpq	%rbx, %rax
	jl	70
	movq	%r8, %rbp
	movq	%r11, %rax
	nopl	(%rax,%rax)
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%rax
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %rax
	jg	-59
	leaq	6(%r12), %rax
	cmpq	%rbx, %r11
	jl	70
	movq	%r9, %rbp
	movq	%rax, %r11
	nopl	(%rax,%rax)
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%r11
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %r11
	jg	-59
	leaq	7(%r12), %r11
	cmpq	%rbx, %rax
	jl	70
	movq	%r10, %rbp
	movq	%r11, %rax
	nopl	(%rax,%rax)
	vmovsd	-16(%rbp), %xmm0
	vucomisd	(%rbp), %xmm0
	jbe	47
	decq	%rax
	leaq	-16(%rbp), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	movq	%r14, %rbp
	cmpq	%rbx, %rax
	jg	-59
	addq	$8, %r12
	cmpq	%rbx, %r11
	jl	-616
	movq	%r13, %r11
	movq	%r12, %rax
	nop
	vmovsd	-16(%r11), %xmm0
	vucomisd	(%r11), %xmm0
	jbe	-641
	decq	%rax
	leaq	-16(%r11), %r14
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%r11), %xmm0
	vmovups	%xmm0, (%r14)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%r11)
	movq	%r14, %r11
	cmpq	%rbx, %rax
	jg	-64
	jmp	-693
	nopw	%cs:(%rax,%rax)
	vmovups	(%r15), %xmm0
	vmovaps	%xmm0, -48(%rsp)
	vmovups	(%rsi), %xmm0
	vmovups	%xmm0, (%r15)
	vmovapd	-48(%rsp), %xmm0
	vmovupd	%xmm0, (%rsi)
	leaq	-2(%r8), %rax
	movq	%rax, %rsi
	subq	%rbx, %rsi
	movq	%r14, %r9
	subq	%r8, %r9
	cmpq	%r9, %rsi
	jle	57
	movq	%rax, 8(%rdx)
	cmpq	%r8, %r14
	jle	-1440
	movl	%ecx, %eax
	incl	%ecx
	shlq	$4, %rax
	movq	%r8, 208(%rsp,%rax)
	movq	%r14, 216(%rsp,%rax)
	jmp	-1469
	movq	-72(%rsp), %r11
	movq	-64(%rsp), %r15
	jmp	-1481
	movq	%r8, (%rdx)
	cmpq	%rax, %rbx
	jge	-1496
	movl	%ecx, %edx
	incl	%ecx
	shlq	$4, %rdx
	movq	%rbx, 208(%rsp,%rdx)
	movq	%rax, 216(%rsp,%rdx)
	jmp	-1525
	addq	$1240, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
