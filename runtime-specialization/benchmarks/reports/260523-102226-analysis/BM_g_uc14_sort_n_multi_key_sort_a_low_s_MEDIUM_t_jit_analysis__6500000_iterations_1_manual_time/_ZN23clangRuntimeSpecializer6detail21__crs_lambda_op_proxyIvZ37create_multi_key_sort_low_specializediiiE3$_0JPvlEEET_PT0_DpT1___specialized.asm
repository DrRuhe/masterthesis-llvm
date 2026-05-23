<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_multi_key_sort_low_specializediiiE3$_0JPvlEEET_PT0_DpT1_>:
	cmpq	$2, %rsi
	jl	1795
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$1176, %rsp
	decq	%rsi
	movq	$0, 144(%rsp)
	movq	%rsi, 152(%rsp)
	leaq	16(%rdi), %rax
	movq	%rax, -120(%rsp)
	leaq	32(%rdi), %rax
	movq	%rax, -128(%rsp)
	leaq	48(%rdi), %rdx
	movl	$1, %r10d
	jmp	15
	movl	%r10d, %r9d
	movl	%r9d, %r10d
	testl	%r9d, %r9d
	jle	1693
	leal	-1(%r10), %r9d
	movq	%r9, %r11
	shlq	$4, %r11
	movq	144(%rsp,%r11), %rsi
	movq	152(%rsp,%r11), %r8
	movq	%rsi, %rbx
	subq	%r8, %rbx
	jge	-47
	movq	%r8, %r14
	subq	%rsi, %r14
	cmpq	$15, %r14
	jg	206
	movq	%rsi, %r10
	andq	$3, %r14
	je	257
	movq	%rsi, %r11
	shlq	$4, %r11
	addq	%rdi, %r11
	movq	%rsi, %r10
	xorl	%r15d, %r15d
	jmp	31
	nopw	%cs:(%rax,%rax)
	incq	%r15
	addq	$16, %r11
	cmpq	%r14, %r15
	je	208
	cmpq	%rsi, %r10
	leaq	1(%r10), %r10
	jl	-25
	movq	%r11, %r12
	movq	%r10, %r13
	nop
	vmovsd	(%r12), %xmm0
	vmovsd	16(%r12), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	11
	seta	%bpl
	vucomisd	%xmm0, %xmm1
	jmp	27
	nop
	vmovsd	8(%r12), %xmm0
	vmovsd	24(%r12), %xmm1
	vucomisd	%xmm0, %xmm1
	seta	%bpl
	vucomisd	%xmm1, %xmm0
	seta	%al
	testb	%bpl, %bpl
	je	-98
	testb	%al, %al
	jne	-102
	leaq	16(%r12), %rax
	vmovups	(%r12), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r12)
	decq	%r13
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r12
	cmpq	%rsi, %r13
	jg	-119
	jmp	-156
	nopl	(%rax)
	shrq	%r14
	addq	%rsi, %r14
	movq	%rsi, %rax
	shlq	$4, %rax
	leaq	(%rdi,%rax), %r9
	shlq	$4, %r14
	leaq	(%rdi,%r14), %rbx
	vmovsd	(%rdi,%rax), %xmm0
	vmovsd	(%rdi,%r14), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	6
	jnp	674
	seta	%al
	vucomisd	%xmm0, %xmm1
	jmp	684
	nopl	(%rax)
	cmpq	$-4, %rbx
	ja	-343
	movq	%r10, %r11
	shlq	$4, %r11
	leaq	(%rdi,%r11), %rbx
	movq	-120(%rsp), %rax
	leaq	(%rax,%r11), %r14
	movq	-128(%rsp), %rax
	leaq	(%rax,%r11), %r15
	addq	%rdx, %r11
	jmp	29
	nopl	(%rax)
	addq	$64, %rbx
	addq	$64, %r14
	addq	$64, %r15
	addq	$64, %r11
	cmpq	%r8, %r10
	je	-406
	leaq	1(%r10), %r12
	cmpq	%rsi, %r10
	jl	124
	movq	%rbx, %r13
	movq	%r12, %rbp
	nopl	(%rax)
	vmovsd	(%r13), %xmm0
	vmovsd	16(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	12
	seta	%cl
	vucomisd	%xmm0, %xmm1
	jmp	26
	nopl	(%rax)
	vmovsd	8(%r13), %xmm0
	vmovsd	24(%r13), %xmm1
	vucomisd	%xmm0, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%al
	testb	%cl, %cl
	je	52
	testb	%al, %al
	jne	48
	leaq	16(%r13), %rax
	vmovups	(%r13), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r13)
	decq	%rbp
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r13
	cmpq	%rsi, %rbp
	jg	-114
	leaq	2(%r10), %r13
	cmpq	%rsi, %r12
	jl	134
	movq	%r14, %r12
	movq	%r13, %rbp
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r12), %xmm0
	vmovsd	16(%r12), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	11
	seta	%cl
	vucomisd	%xmm0, %xmm1
	jmp	27
	nop
	vmovsd	8(%r12), %xmm0
	vmovsd	24(%r12), %xmm1
	vucomisd	%xmm0, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%al
	testb	%cl, %cl
	je	53
	testb	%al, %al
	jne	49
	leaq	16(%r12), %rax
	vmovups	(%r12), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r12)
	decq	%rbp
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r12
	cmpq	%rsi, %rbp
	jg	-117
	leaq	3(%r10), %r12
	cmpq	%rsi, %r13
	jl	128
	movq	%r15, %r13
	movq	%r12, %rbp
	nopl	(%rax,%rax)
	vmovsd	(%r13), %xmm0
	vmovsd	16(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	12
	seta	%cl
	vucomisd	%xmm0, %xmm1
	jmp	26
	nopl	(%rax)
	vmovsd	8(%r13), %xmm0
	vmovsd	24(%r13), %xmm1
	vucomisd	%xmm0, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%al
	testb	%cl, %cl
	je	52
	testb	%al, %al
	jne	48
	leaq	16(%r13), %rax
	vmovups	(%r13), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r13)
	decq	%rbp
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r13
	cmpq	%rsi, %rbp
	jg	-114
	addq	$4, %r10
	cmpq	%rsi, %r12
	jl	-463
	movq	%r11, %r12
	movq	%r10, %r13
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r12), %xmm0
	vmovsd	16(%r12), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	11
	seta	%cl
	vucomisd	%xmm0, %xmm1
	jmp	27
	nop
	vmovsd	8(%r12), %xmm0
	vmovsd	24(%r12), %xmm1
	vucomisd	%xmm0, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%al
	testb	%cl, %cl
	je	-548
	testb	%al, %al
	jne	-556
	leaq	16(%r12), %rax
	vmovups	(%r12), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r12)
	decq	%r13
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r12
	cmpq	%rsi, %r13
	jg	-125
	jmp	-610
	vmovsd	8(%r9), %xmm1
	vmovsd	8(%rbx), %xmm2
	vucomisd	%xmm1, %xmm2
	seta	%al
	vucomisd	%xmm2, %xmm1
	seta	%cl
	testb	%al, %al
	je	39
	testb	%cl, %cl
	jne	35
	vmovups	(%r9), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rbx), %xmm0
	vmovups	%xmm0, (%r9)
	vmovaps	-112(%rsp), %xmm0
	vmovups	%xmm0, (%rbx)
	vmovsd	(%r9), %xmm0
	movq	%r8, %rax
	shlq	$4, %rax
	leaq	(%rdi,%rax), %r14
	vmovsd	(%rdi,%rax), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	9
	seta	%al
	vucomisd	%xmm0, %xmm1
	jmp	23
	vmovsd	8(%r9), %xmm0
	vmovsd	8(%r14), %xmm2
	vucomisd	%xmm0, %xmm2
	seta	%al
	vucomisd	%xmm2, %xmm0
	seta	%cl
	testb	%al, %al
	je	41
	testb	%cl, %cl
	jne	37
	vmovups	(%r9), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%r14), %xmm0
	vmovups	%xmm0, (%r9)
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%r14)
	vmovsd	(%r14), %xmm1
	vmovsd	(%rbx), %xmm0
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	9
	seta	%cl
	vucomisd	%xmm0, %xmm1
	jmp	22
	vmovsd	8(%rbx), %xmm0
	vmovsd	8(%r14), %xmm1
	vucomisd	%xmm0, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%al
	testb	%cl, %cl
	je	34
	testb	%al, %al
	jne	30
	vmovups	(%rbx), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%r14), %xmm0
	vmovups	%xmm0, (%rbx)
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%r14)
	leaq	(%rsp,%r11), %r9
	addq	$144, %r9
	leaq	-1(%r8), %r14
	movq	%r14, %rax
	shlq	$4, %rax
	leaq	(%rdi,%rax), %r11
	vmovups	(%rbx), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%rdi,%rax), %xmm0
	vmovups	%xmm0, (%rbx)
	vmovaps	-112(%rsp), %xmm0
	vmovups	%xmm0, (%rdi,%rax)
	movq	%rsi, %r12
	nopl	(%rax)
	vmovsd	(%r11), %xmm0
	leaq	1(%r12), %rbx
	shlq	$4, %r12
	addq	%rdi, %r12
	movq	%r12, %r15
	nopw	%cs:(%rax,%rax)
	vmovsd	16(%r15), %xmm1
	vucomisd	%xmm0, %xmm1
	jne	2
	jnp	18
	seta	%bpl
	vucomisd	%xmm1, %xmm0
	jmp	32
	nopl	(%rax,%rax)
	vmovsd	24(%r15), %xmm1
	vmovsd	8(%r11), %xmm2
	vucomisd	%xmm1, %xmm2
	seta	%bpl
	vucomisd	%xmm2, %xmm1
	seta	%al
	incq	%rbx
	addq	$16, %r15
	testb	%al, %al
	je	5
	testb	%bpl, %bpl
	je	-75
	leaq	-1(%rbx), %r12
	movq	%r14, %r13
	shlq	$4, %r13
	addq	%rdi, %r13
	nopl	(%rax)
	vmovsd	-16(%r13), %xmm1
	vucomisd	%xmm0, %xmm1
	jne	2
	jnp	18
	seta	%al
	vucomisd	%xmm1, %xmm0
	jmp	32
	nopw	(%rax,%rax)
	vmovsd	-8(%r13), %xmm1
	vmovsd	8(%r11), %xmm2
	vucomisd	%xmm1, %xmm2
	seta	%al
	vucomisd	%xmm2, %xmm1
	seta	%bpl
	addq	$-16, %r13
	decq	%r14
	testb	%al, %al
	je	5
	testb	%bpl, %bpl
	je	-75
	cmpq	%r14, %r12
	jge	48
	vmovups	(%r15), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%r13), %xmm0
	vmovups	%xmm0, (%r15)
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%r13)
	jmp	-247
	nopw	(%rax,%rax)
	vmovups	(%r15), %xmm0
	vmovaps	%xmm0, -112(%rsp)
	vmovups	(%r11), %xmm0
	vmovups	%xmm0, (%r15)
	vmovapd	-112(%rsp), %xmm0
	vmovupd	%xmm0, (%r11)
	leaq	-2(%rbx), %r11
	movq	%r11, %rax
	subq	%rsi, %rax
	movq	%r8, %rcx
	subq	%rbx, %rcx
	cmpq	%rcx, %rax
	jle	44
	movq	%r11, 8(%r9)
	cmpq	%rbx, %r8
	jle	-1634
	movl	%r10d, %eax
	incl	%r10d
	shlq	$4, %rax
	movq	%rbx, 144(%rsp,%rax)
	movq	%r8, 152(%rsp,%rax)
	jmp	-1665
	movq	%rbx, (%r9)
	cmpq	%r11, %rsi
	jge	-1677
	movl	%r10d, %eax
	incl	%r10d
	shlq	$4, %rax
	movq	%rsi, 144(%rsp,%rax)
	movq	%r11, 152(%rsp,%rax)
	jmp	-1708
	addq	$1176, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
