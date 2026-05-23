<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_multi_key_sort_abstract_specializediiiE3$_0JPvlEEET_PT0_DpT1_>:
	cmpq	$2, %rsi
	jl	2145
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$1192, %rsp
	movabsq	$140724238851608, %r11
	decq	%rsi
	movq	$0, 160(%rsp)
	movq	%rsi, 168(%rsp)
	leaq	16(%rdi), %rax
	movq	%rax, -112(%rsp)
	leaq	32(%rdi), %rax
	movq	%rax, -120(%rsp)
	leaq	48(%rdi), %rax
	movq	%rax, -128(%rsp)
	movl	$1, %eax
	jmp	17
	nop
	movl	%eax, %r9d
	movl	%r9d, %eax
	testl	%r9d, %r9d
	jle	2027
	leal	-1(%rax), %r9d
	movq	%r9, %rbx
	shlq	$4, %rbx
	movq	160(%rsp,%rbx), %r8
	movq	168(%rsp,%rbx), %rsi
	movq	%r8, %r15
	subq	%rsi, %r15
	jge	-47
	movq	%rsi, %r14
	subq	%r8, %r14
	cmpq	$15, %r14
	jg	254
	movq	%r8, %r10
	andq	$3, %r14
	je	321
	movq	%r8, %rbx
	shlq	$4, %rbx
	addq	%rdi, %rbx
	movq	%r8, %r10
	xorl	%r12d, %r12d
	jmp	31
	nopw	%cs:(%rax,%rax)
	incq	%r12
	addq	$16, %rbx
	cmpq	%r14, %r12
	je	272
	cmpq	%r8, %r10
	leaq	1(%r10), %r10
	jl	-25
	movq	%rbx, %r13
	movq	%r10, %rbp
	nop
	vmovsd	(%r13), %xmm0
	vmovsd	16(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	28
	seta	%cl
	vucomisd	%xmm0, %xmm1
	seta	%dl
	movq	%r11, %rax
	jmp	43
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%r13), %xmm0
	vmovsd	24(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	seta	%cl
	vucomisd	%xmm0, %xmm1
	seta	%dl
	leaq	1(%r11), %rax
	movzbl	%cl, %ecx
	movzbl	%dl, %edx
	subl	%edx, %ecx
	movl	%ecx, %edx
	negl	%edx
	cmpb	$0, (%rax)
	cmovnel	%ecx, %edx
	testl	%edx, %edx
	jle	-136
	leaq	16(%r13), %rax
	vmovups	(%r13), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r13)
	decq	%rbp
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r13
	cmpq	%r8, %rbp
	jg	-156
	jmp	-193
	nopw	%cs:(%rax,%rax)
	shrq	%r14
	addq	%r8, %r14
	movq	%r8, %rcx
	shlq	$4, %rcx
	leaq	(%rdi,%rcx), %r15
	shlq	$4, %r14
	leaq	(%rdi,%r14), %r10
	vmovsd	(%rdi,%rcx), %xmm0
	vmovsd	(%rdi,%r14), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	6
	jnp	844
	seta	%cl
	vucomisd	%xmm0, %xmm1
	seta	%dl
	movq	%r11, %r9
	jmp	856
	nopw	%cs:(%rax,%rax)
	cmpq	$-4, %r15
	ja	-407
	movq	%r9, -104(%rsp)
	movq	%r10, %rbx
	shlq	$4, %rbx
	leaq	(%rdi,%rbx), %r14
	movq	-112(%rsp), %rax
	leaq	(%rax,%rbx), %r15
	movq	-120(%rsp), %rax
	leaq	(%rax,%rbx), %r12
	addq	-128(%rsp), %rbx
	jmp	38
	nopw	%cs:(%rax,%rax)
	addq	$64, %r14
	addq	$64, %r15
	addq	$64, %r12
	addq	$64, %rbx
	cmpq	%rsi, %r10
	je	713
	leaq	1(%r10), %rcx
	cmpq	%r8, %r10
	jl	159
	movq	%r14, %rbp
	movq	%rcx, %r13
	nopl	(%rax)
	vmovsd	(%rbp), %xmm0
	vmovsd	16(%rbp), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	30
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	movq	%r11, %r9
	jmp	43
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%rbp), %xmm0
	vmovsd	24(%rbp), %xmm1
	vucomisd	%xmm1, %xmm0
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	leaq	1(%r11), %r9
	movzbl	%dl, %edx
	movzbl	%al, %eax
	subl	%eax, %edx
	movl	%edx, %eax
	negl	%eax
	cmpb	$0, (%r9)
	cmovnel	%edx, %eax
	testl	%eax, %eax
	jle	50
	leaq	16(%rbp), %rax
	vmovups	(%rbp), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%rbp)
	decq	%r13
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %rbp
	cmpq	%r8, %r13
	jg	-149
	leaq	2(%r10), %rbp
	cmpq	%r8, %rcx
	jl	167
	movq	%r15, %r13
	movq	%rbp, %rcx
	nopl	(%rax,%rax)
	vmovsd	(%r13), %xmm0
	vmovsd	16(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	28
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	movq	%r11, %r9
	jmp	43
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%r13), %xmm0
	vmovsd	24(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	leaq	1(%r11), %r9
	movzbl	%dl, %edx
	movzbl	%al, %eax
	subl	%eax, %edx
	movl	%edx, %eax
	negl	%eax
	cmpb	$0, (%r9)
	cmovnel	%edx, %eax
	testl	%eax, %eax
	jle	52
	leaq	16(%r13), %rax
	vmovups	(%r13), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r13)
	decq	%rcx
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r13
	cmpq	%r8, %rcx
	jg	-153
	leaq	3(%r10), %r13
	cmpq	%r8, %rbp
	jl	159
	movq	%r12, %rbp
	movq	%r13, %rcx
	nopl	(%rax)
	vmovsd	(%rbp), %xmm0
	vmovsd	16(%rbp), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	30
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	movq	%r11, %r9
	jmp	43
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%rbp), %xmm0
	vmovsd	24(%rbp), %xmm1
	vucomisd	%xmm1, %xmm0
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	leaq	1(%r11), %r9
	movzbl	%dl, %edx
	movzbl	%al, %eax
	subl	%eax, %edx
	movl	%edx, %eax
	negl	%eax
	cmpb	$0, (%r9)
	cmovnel	%edx, %eax
	testl	%eax, %eax
	jle	50
	leaq	16(%rbp), %rax
	vmovups	(%rbp), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%rbp)
	decq	%rcx
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %rbp
	cmpq	%r8, %rcx
	jg	-149
	addq	$4, %r10
	cmpq	%r8, %r13
	jl	-562
	movq	%rbx, %r13
	movq	%r10, %rcx
	nopl	(%rax,%rax)
	vmovsd	(%r13), %xmm0
	vmovsd	16(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	28
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	movq	%r11, %r9
	jmp	43
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%r13), %xmm0
	vmovsd	24(%r13), %xmm1
	vucomisd	%xmm1, %xmm0
	seta	%dl
	vucomisd	%xmm0, %xmm1
	seta	%al
	leaq	1(%r11), %r9
	movzbl	%dl, %edx
	movzbl	%al, %eax
	subl	%eax, %edx
	movl	%edx, %eax
	negl	%eax
	cmpb	$0, (%r9)
	cmovnel	%edx, %eax
	testl	%eax, %eax
	jle	-681
	leaq	16(%r13), %rax
	vmovups	(%r13), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rax), %xmm0
	vmovups	%xmm0, (%r13)
	decq	%rcx
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%rax)
	addq	$-16, %r13
	cmpq	%r8, %rcx
	jg	-157
	jmp	-738
	movq	-104(%rsp), %r9
	jmp	-1209
	vmovsd	8(%r15), %xmm1
	vmovsd	8(%r10), %xmm2
	vucomisd	%xmm2, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm2
	seta	%dl
	leaq	1(%r11), %r9
	movzbl	%cl, %ecx
	movzbl	%dl, %edx
	subl	%edx, %ecx
	movl	%ecx, %edx
	negl	%edx
	cmpb	$0, (%r9)
	cmovnel	%ecx, %edx
	testl	%edx, %edx
	jle	37
	vmovups	(%r15), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%r10), %xmm0
	vmovups	%xmm0, (%r15)
	vmovaps	-96(%rsp), %xmm0
	vmovups	%xmm0, (%r10)
	vmovsd	(%r15), %xmm0
	movq	%rsi, %rcx
	shlq	$4, %rcx
	leaq	(%rdi,%rcx), %r14
	vmovsd	(%rdi,%rcx), %xmm1
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	15
	seta	%cl
	vucomisd	%xmm0, %xmm1
	seta	%dl
	movq	%r11, %r9
	jmp	30
	vmovsd	8(%r15), %xmm0
	vmovsd	8(%r14), %xmm2
	vucomisd	%xmm2, %xmm0
	seta	%cl
	vucomisd	%xmm0, %xmm2
	seta	%dl
	leaq	1(%r11), %r9
	movzbl	%cl, %ecx
	movzbl	%dl, %edx
	subl	%edx, %ecx
	movl	%ecx, %edx
	negl	%edx
	cmpb	$0, (%r9)
	cmovnel	%ecx, %edx
	testl	%edx, %edx
	jle	37
	vmovups	(%r15), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%r14), %xmm0
	vmovups	%xmm0, (%r15)
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%r14)
	vmovsd	(%r14), %xmm1
	vmovsd	(%r10), %xmm0
	vucomisd	%xmm1, %xmm0
	jne	2
	jnp	15
	seta	%cl
	vucomisd	%xmm0, %xmm1
	seta	%dl
	movq	%r11, %r9
	jmp	30
	vmovsd	8(%r10), %xmm0
	vmovsd	8(%r14), %xmm1
	vucomisd	%xmm1, %xmm0
	seta	%cl
	vucomisd	%xmm0, %xmm1
	seta	%dl
	leaq	1(%r11), %r9
	movzbl	%cl, %ecx
	movzbl	%dl, %edx
	subl	%edx, %ecx
	movl	%ecx, %edx
	negl	%edx
	cmpb	$0, (%r9)
	cmovnel	%ecx, %edx
	testl	%edx, %edx
	jle	32
	vmovups	(%r10), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%r14), %xmm0
	vmovups	%xmm0, (%r10)
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%r14)
	addq	%rsp, %rbx
	addq	$160, %rbx
	leaq	-1(%rsi), %r15
	movq	%r15, %rcx
	shlq	$4, %rcx
	leaq	(%rdi,%rcx), %r14
	vmovups	(%r10), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rdi,%rcx), %xmm0
	vmovups	%xmm0, (%r10)
	vmovaps	-96(%rsp), %xmm0
	vmovups	%xmm0, (%rdi,%rcx)
	movq	%r8, %r13
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r14), %xmm0
	leaq	1(%r13), %r10
	shlq	$4, %r13
	addq	%rdi, %r13
	movq	%r13, %r12
	jmp	72
	nopw	%cs:(%rax,%rax)
	vmovsd	24(%r12), %xmm1
	vmovsd	8(%r14), %xmm2
	vucomisd	%xmm2, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm2
	seta	%dl
	leaq	1(%r11), %r9
	movzbl	%cl, %ecx
	movzbl	%dl, %edx
	subl	%edx, %ecx
	movl	%ecx, %edx
	negl	%edx
	cmpb	$0, (%r9)
	cmovnel	%ecx, %edx
	incq	%r10
	addq	$16, %r12
	testl	%edx, %edx
	jns	35
	vmovsd	16(%r12), %xmm1
	vucomisd	%xmm0, %xmm1
	jne	2
	jnp	-76
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%dl
	movq	%r11, %r9
	jmp	-60
	nopl	(%rax,%rax)
	leaq	-1(%r10), %r13
	movq	%r15, %rbp
	shlq	$4, %rbp
	addq	%rdi, %rbp
	jmp	59
	vmovsd	-8(%rbp), %xmm1
	vmovsd	8(%r14), %xmm2
	vucomisd	%xmm2, %xmm1
	seta	%cl
	vucomisd	%xmm1, %xmm2
	seta	%dl
	leaq	1(%r11), %r9
	movzbl	%cl, %ecx
	movzbl	%dl, %edx
	subl	%edx, %ecx
	movl	%ecx, %edx
	negl	%edx
	cmpb	$0, (%r9)
	cmovnel	%ecx, %edx
	addq	$-16, %rbp
	decq	%r15
	testl	%edx, %edx
	jle	37
	vmovsd	-16(%rbp), %xmm1
	vucomisd	%xmm0, %xmm1
	jne	2
	jnp	-72
	seta	%cl
	vucomisd	%xmm1, %xmm0
	seta	%dl
	movq	%r11, %r9
	jmp	-58
	nopw	(%rax,%rax)
	cmpq	%r15, %r13
	jge	43
	vmovups	(%r12), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%rbp), %xmm0
	vmovups	%xmm0, (%r12)
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%rbp)
	jmp	-284
	nopl	(%rax)
	vmovups	(%r12), %xmm0
	vmovaps	%xmm0, -96(%rsp)
	vmovups	(%r14), %xmm0
	vmovups	%xmm0, (%r12)
	vmovapd	-96(%rsp), %xmm0
	vmovupd	%xmm0, (%r14)
	leaq	-2(%r10), %rcx
	movq	%rcx, %rdx
	subq	%r8, %rdx
	movq	%rsi, %r9
	subq	%r10, %r9
	cmpq	%r9, %rdx
	jle	42
	movq	%rcx, 8(%rbx)
	cmpq	%r10, %rsi
	jle	-1972
	movl	%eax, %ecx
