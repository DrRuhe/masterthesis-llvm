	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rcx
	movabsq	$-7946, %rax
	addq	%rcx, %rax
	movabsq	$112, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %r11
	movabsq	$128, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rcx
	movabsq	$80, %rdx
	movq	(%rax,%rdx), %rdx
	movq	(%rdx), %rbx
	leaq	20800(%rcx), %rdx
	leaq	41579192(%rbx), %rsi
	leaq	41579200(%r11), %rdi
	leaq	8(%r11), %r14
	leaq	41600000(%r11), %r8
	leaq	41600000(%rbx), %r9
	leaq	41600000(%rcx), %r10
	cmpq	%r9, %rdx
	setb	%dl
	cmpq	%r10, %rbx
	setb	%r10b
	andb	%dl, %r10b
	movb	%r10b, -97(%rsp)
	movq	%r14, -16(%rsp)
	cmpq	%r9, %r14
	setae	%dl
	cmpq	%r8, %rbx
	setae	%r9b
	leaq	41599992(%rcx), %r8
	orb	%dl, %r9b
	movb	%r9b, -98(%rsp)
	cmpq	%rdi, %rbx
	setae	%dl
	cmpq	%rsi, %r11
	setae	%dil
	orb	%dl, %dil
	cmpq	%r8, %rbx
	setae	%dl
	cmpq	%rsi, %rcx
	setae	%r9b
	orb	%dl, %r9b
	andb	%dil, %r9b
	leaq	20944(%rbx), %rdx
	movq	%rdx, -56(%rsp)
	leaq	20944(%rcx), %rdx
	movq	%rdx, -64(%rsp)
	leaq	20872(%rbx), %rdx
	movq	%rdx, -72(%rsp)
	leaq	20872(%rcx), %rdx
	movq	%rdx, -80(%rsp)
	leaq	120(%rbx), %rdx
	movq	%rdx, -24(%rsp)
	movq	%r11, -88(%rsp)
	leaq	120(%r11), %rdx
	movq	%rdx, -32(%rsp)
	movq	%rbx, -96(%rsp)
	leaq	8(%rbx), %rdx
	movq	%rdx, -40(%rsp)
	movabsq	$-1584, %rdx
	vmovsd	(%rax,%rdx), %xmm0
	movabsq	$-1616, %rdx
	vmovapd	(%rax,%rdx), %xmm1
	movabsq	$-1600, %rdx
	vmovapd	(%rax,%rdx), %xmm2
	movabsq	$-1576, %rdx
	vmovsd	(%rax,%rdx), %xmm3
	movabsq	$16, %rdx
	movq	(%rax,%rdx), %rax
	movq	%rax, -48(%rsp)
	xorl	%edx, %edx
	jmp	21
	movq	-8(%rsp), %rdx
	incq	%rdx
	cmpq	$1000, %rdx
	je	3093
	movq	-48(%rsp), %rax
	vmovddup	(%rax,%rdx,8), %xmm4
	movl	$102, %eax
	nopw	%cs:(%rax,%rax)
	vmovupd	%xmm4, -816(%rcx,%rax,8)
	vmovupd	%xmm4, -800(%rcx,%rax,8)
	vmovupd	%xmm4, -784(%rcx,%rax,8)
	vmovupd	%xmm4, -768(%rcx,%rax,8)
	vmovupd	%xmm4, -752(%rcx,%rax,8)
	vmovupd	%xmm4, -736(%rcx,%rax,8)
	vmovupd	%xmm4, -720(%rcx,%rax,8)
	vmovupd	%xmm4, -704(%rcx,%rax,8)
	vmovupd	%xmm4, -688(%rcx,%rax,8)
	vmovupd	%xmm4, -672(%rcx,%rax,8)
	vmovupd	%xmm4, -656(%rcx,%rax,8)
	vmovupd	%xmm4, -640(%rcx,%rax,8)
	vmovupd	%xmm4, -624(%rcx,%rax,8)
	vmovupd	%xmm4, -608(%rcx,%rax,8)
	vmovupd	%xmm4, -592(%rcx,%rax,8)
	vmovupd	%xmm4, -576(%rcx,%rax,8)
	vmovupd	%xmm4, -560(%rcx,%rax,8)
	vmovupd	%xmm4, -544(%rcx,%rax,8)
	vmovupd	%xmm4, -528(%rcx,%rax,8)
	vmovupd	%xmm4, -512(%rcx,%rax,8)
	vmovupd	%xmm4, -496(%rcx,%rax,8)
	vmovupd	%xmm4, -480(%rcx,%rax,8)
	vmovupd	%xmm4, -464(%rcx,%rax,8)
	vmovupd	%xmm4, -448(%rcx,%rax,8)
	vmovupd	%xmm4, -432(%rcx,%rax,8)
	vmovupd	%xmm4, -416(%rcx,%rax,8)
	vmovupd	%xmm4, -400(%rcx,%rax,8)
	vmovupd	%xmm4, -384(%rcx,%rax,8)
	vmovupd	%xmm4, -368(%rcx,%rax,8)
	vmovupd	%xmm4, -352(%rcx,%rax,8)
	vmovupd	%xmm4, -336(%rcx,%rax,8)
	vmovupd	%xmm4, -320(%rcx,%rax,8)
	vmovupd	%xmm4, -304(%rcx,%rax,8)
	vmovupd	%xmm4, -288(%rcx,%rax,8)
	vmovupd	%xmm4, -272(%rcx,%rax,8)
	vmovupd	%xmm4, -256(%rcx,%rax,8)
	vmovupd	%xmm4, -240(%rcx,%rax,8)
	vmovupd	%xmm4, -224(%rcx,%rax,8)
	vmovupd	%xmm4, -208(%rcx,%rax,8)
	vmovupd	%xmm4, -192(%rcx,%rax,8)
	vmovupd	%xmm4, -176(%rcx,%rax,8)
	vmovupd	%xmm4, -160(%rcx,%rax,8)
	vmovupd	%xmm4, -144(%rcx,%rax,8)
	vmovupd	%xmm4, -128(%rcx,%rax,8)
	vmovupd	%xmm4, -112(%rcx,%rax,8)
	vmovupd	%xmm4, -96(%rcx,%rax,8)
	vmovupd	%xmm4, -80(%rcx,%rax,8)
	vmovupd	%xmm4, -64(%rcx,%rax,8)
	vmovupd	%xmm4, -48(%rcx,%rax,8)
	vmovupd	%xmm4, -32(%rcx,%rax,8)
	vmovupd	%xmm4, -16(%rcx,%rax,8)
	vmovupd	%xmm4, (%rcx,%rax,8)
	addq	$104, %rax
	cmpq	$2702, %rax
	jne	-456
	movq	%rdx, -8(%rsp)
	cmpb	$0, -97(%rsp)
	je	376
	movl	$1, %eax
	movq	-80(%rsp), %rdx
	movq	-72(%rsp), %rsi
	nopw	(%rax,%rax)
	xorl	%edi, %edi
	nopw	%cs:(%rax,%rax)
	vmovsd	-72(%rsi,%rdi,8), %xmm4
	vsubsd	-20872(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-72(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -72(%rdx,%rdi,8)
	vmovsd	-64(%rsi,%rdi,8), %xmm4
	vsubsd	-20864(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-64(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -64(%rdx,%rdi,8)
	vmovsd	-56(%rsi,%rdi,8), %xmm4
	vsubsd	-20856(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-56(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -56(%rdx,%rdi,8)
	vmovsd	-48(%rsi,%rdi,8), %xmm4
	vsubsd	-20848(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-48(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -48(%rdx,%rdi,8)
	vmovsd	-40(%rsi,%rdi,8), %xmm4
	vsubsd	-20840(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-40(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -40(%rdx,%rdi,8)
	vmovsd	-32(%rsi,%rdi,8), %xmm4
	vsubsd	-20832(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-32(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -32(%rdx,%rdi,8)
	vmovsd	-24(%rsi,%rdi,8), %xmm4
	vsubsd	-20824(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-24(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -24(%rdx,%rdi,8)
	vmovsd	-16(%rsi,%rdi,8), %xmm4
	vsubsd	-20816(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-16(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -16(%rdx,%rdi,8)
	vmovsd	-8(%rsi,%rdi,8), %xmm4
	vsubsd	-20808(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	-8(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, -8(%rdx,%rdi,8)
	vmovsd	(%rsi,%rdi,8), %xmm4
	vsubsd	-20800(%rsi,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	(%rdx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, (%rdx,%rdi,8)
	addq	$10, %rdi
	cmpq	$2600, %rdi
	jne	-294
	incq	%rax
	addq	$20800, %rsi
	addq	$20800, %rdx
	cmpq	$2000, %rax
	jne	-339
	jmp	372
	nopl	(%rax,%rax)
	movl	$1, %eax
	movq	-64(%rsp), %rdx
	movq	-56(%rsp), %rsi
	nop
	xorl	%edi, %edi
	nopw	%cs:(%rax,%rax)
	vmovupd	-144(%rsi,%rdi,8), %xmm4
	vmovupd	-128(%rsi,%rdi,8), %xmm5
	vsubpd	-20944(%rsi,%rdi,8), %xmm4, %xmm4
	vsubpd	-20928(%rsi,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-144(%rdx,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-128(%rdx,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -144(%rdx,%rdi,8)
	vmovupd	%xmm5, -128(%rdx,%rdi,8)
	vmovupd	-112(%rsi,%rdi,8), %xmm4
	vmovupd	-96(%rsi,%rdi,8), %xmm5
	vsubpd	-20912(%rsi,%rdi,8), %xmm4, %xmm4
	vsubpd	-20896(%rsi,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-112(%rdx,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-96(%rdx,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -112(%rdx,%rdi,8)
	vmovupd	%xmm5, -96(%rdx,%rdi,8)
	vmovupd	-80(%rsi,%rdi,8), %xmm4
	vmovupd	-64(%rsi,%rdi,8), %xmm5
	vsubpd	-20880(%rsi,%rdi,8), %xmm4, %xmm4
	vsubpd	-20864(%rsi,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-80(%rdx,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-64(%rdx,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -80(%rdx,%rdi,8)
	vmovupd	%xmm5, -64(%rdx,%rdi,8)
	vmovupd	-48(%rsi,%rdi,8), %xmm4
	vmovupd	-32(%rsi,%rdi,8), %xmm5
	vsubpd	-20848(%rsi,%rdi,8), %xmm4, %xmm4
	vsubpd	-20832(%rsi,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-48(%rdx,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-32(%rdx,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -48(%rdx,%rdi,8)
	vmovupd	%xmm5, -32(%rdx,%rdi,8)
	vmovupd	-16(%rsi,%rdi,8), %xmm4
	vmovupd	(%rsi,%rdi,8), %xmm5
	vsubpd	-20816(%rsi,%rdi,8), %xmm4, %xmm4
	vsubpd	-20800(%rsi,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-16(%rdx,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	(%rdx,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -16(%rdx,%rdi,8)
	vmovupd	%xmm5, (%rdx,%rdi,8)
	addq	$20, %rdi
	cmpq	$2600, %rdi
	jne	-303
	incq	%rax
	addq	$20800, %rsi
	addq	$20800, %rdx
	cmpq	$2000, %rax
	jne	-348
	movq	-96(%rsp), %rdx
	movq	-88(%rsp), %rax
	movq	-40(%rsp), %r13
	movq	-16(%rsp), %rsi
	movq	-32(%rsp), %r12
	movq	-24(%rsp), %r15
	xorl	%r14d, %r14d
	jmp	58
	nop
	incq	%r14
	addq	$20800, %r15
	addq	$20800, %r12
	addq	$20800, %rsi
	addq	$20800, %r13
	addq	$20800, %rax
	addq	$20800, %rdx
	cmpq	$2000, %r14
	je	775
	movl	$1, %ebp
	cmpb	$0, -98(%rsp)
	je	268
	xorl	%edi, %edi
	nopl	(%rax,%rax)
	vmovupd	-112(%r15,%rdi,8), %xmm4
	vmovupd	-96(%r15,%rdi,8), %xmm5
	vsubpd	-120(%r15,%rdi,8), %xmm4, %xmm4
	vsubpd	-104(%r15,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-112(%r12,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-96(%r12,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -112(%r12,%rdi,8)
	vmovupd	%xmm5, -96(%r12,%rdi,8)
	cmpq	$2592, %rdi
	je	187
	vmovupd	-80(%r15,%rdi,8), %xmm4
	vmovupd	-64(%r15,%rdi,8), %xmm5
	vsubpd	-88(%r15,%rdi,8), %xmm4, %xmm4
	vsubpd	-72(%r15,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-80(%r12,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-64(%r12,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -80(%r12,%rdi,8)
	vmovupd	%xmm5, -64(%r12,%rdi,8)
	vmovupd	-48(%r15,%rdi,8), %xmm4
	vmovupd	-32(%r15,%rdi,8), %xmm5
	vsubpd	-56(%r15,%rdi,8), %xmm4, %xmm4
	vsubpd	-40(%r15,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-48(%r12,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	-32(%r12,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -48(%r12,%rdi,8)
	vmovupd	%xmm5, -32(%r12,%rdi,8)
	vmovupd	-16(%r15,%rdi,8), %xmm4
	vmovupd	(%r15,%rdi,8), %xmm5
	vsubpd	-24(%r15,%rdi,8), %xmm4, %xmm4
	vsubpd	-8(%r15,%rdi,8), %xmm5, %xmm5
	vfmadd213pd	-16(%r12,%rdi,8), %xmm1, %xmm4
	vfmadd213pd	(%r12,%rdi,8), %xmm1, %xmm5
	vmovupd	%xmm4, -16(%r12,%rdi,8)
	vmovupd	%xmm5, (%r12,%rdi,8)
	addq	$16, %rdi
	jmp	-243
	nopw	%cs:(%rax,%rax)
	movl	$2597, %ebp
	imulq	$20800, %r14, %rdi
	movq	-88(%rsp), %r8
	addq	%rdi, %r8
	addq	-96(%rsp), %rdi
	leal	(,%rbp,8), %r10d
	vmovsd	(%r10,%rdi), %xmm4
	vsubsd	-8(%r10,%rdi), %xmm4, %xmm4
	vfmadd213sd	(%r10,%r8), %xmm0, %xmm4
	vmovsd	%xmm4, (%r10,%r8)
	movl	$5, %ebx
	subl	%ebp, %ebx
	andl	$4, %ebx
	orq	$2, %rbx
	leaq	(%rsi,%r10), %r11
	addq	%r13, %r10
	addq	$-2597, %rbp
	xorl	%r8d, %r8d
	nopl	(%rax)
	vmovsd	(%r10,%r8,8), %xmm4
	vsubsd	-8(%r10,%r8,8), %xmm4, %xmm4
	vfmadd213sd	(%r11,%r8,8), %xmm0, %xmm4
	vmovsd	%xmm4, (%r11,%r8,8)
	vmovsd	8(%r10,%r8,8), %xmm4
	vsubsd	(%r10,%r8,8), %xmm4, %xmm4
	vfmadd213sd	8(%r11,%r8,8), %xmm0, %xmm4
	movq	%rbp, %rdi
	vmovsd	%xmm4, 8(%r11,%r8,8)
	addq	$2, %r8
	addq	$2, %rbp
	cmpq	%r8, %rbx
	jne	-68
	cmpb	$0, -98(%rsp)
	jne	-511
	nop
	vmovsd	20800(%rdx,%rdi,8), %xmm4
	vsubsd	20792(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20800(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20800(%rax,%rdi,8)
	vmovsd	20808(%rdx,%rdi,8), %xmm4
	vsubsd	20800(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20808(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20808(%rax,%rdi,8)
	vmovsd	20816(%rdx,%rdi,8), %xmm4
	vsubsd	20808(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20816(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20816(%rax,%rdi,8)
	vmovsd	20824(%rdx,%rdi,8), %xmm4
	vsubsd	20816(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20824(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20824(%rax,%rdi,8)
	vmovsd	20832(%rdx,%rdi,8), %xmm4
	vsubsd	20824(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20832(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20832(%rax,%rdi,8)
	vmovsd	20840(%rdx,%rdi,8), %xmm4
	vsubsd	20832(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20840(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20840(%rax,%rdi,8)
	vmovsd	20848(%rdx,%rdi,8), %xmm4
	vsubsd	20840(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20848(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20848(%rax,%rdi,8)
	vmovsd	20856(%rdx,%rdi,8), %xmm4
	vsubsd	20848(%rdx,%rdi,8), %xmm4, %xmm4
	vfmadd213sd	20856(%rax,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, 20856(%rax,%rdi,8)
	addq	$8, %rdi
	jne	-306
	jmp	-823
	nopw	(%rax,%rax)
	movq	%rcx, %rax
	movq	-88(%rsp), %rdx
	movq	-96(%rsp), %rsi
	xorl	%r12d, %r12d
	jmp	50
	nopw	%cs:(%rax,%rax)
	incq	%r12
	addq	$20800, %rsi
	addq	$20800, %rdx
	addq	$20800, %rax
	cmpq	$1999, %r12
	je	-2196
	testb	%r9b, %r9b
	je	403
	movl	$14, %r14d
	nopw	%cs:(%rax,%rax)
	vmovupd	-104(%rdx,%r14,8), %xmm4
	vmovupd	-88(%rdx,%r14,8), %xmm5
	vsubpd	-112(%rdx,%r14,8), %xmm4, %xmm4
	vsubpd	-96(%rdx,%r14,8), %xmm5, %xmm5
	vaddpd	20688(%rax,%r14,8), %xmm4, %xmm4
	vaddpd	20704(%rax,%r14,8), %xmm5, %xmm5
	vsubpd	-112(%rax,%r14,8), %xmm4, %xmm4
	vsubpd	-96(%rax,%r14,8), %xmm5, %xmm5
	vfmadd213pd	-112(%rsi,%r14,8), %xmm2, %xmm4
	vfmadd213pd	-96(%rsi,%r14,8), %xmm2, %xmm5
	vmovupd	%xmm4, -112(%rsi,%r14,8)
	vmovupd	%xmm5, -96(%rsi,%r14,8)
	cmpq	$2606, %r14
	je	297
	vmovupd	-72(%rdx,%r14,8), %xmm4
	vmovupd	-56(%rdx,%r14,8), %xmm5
	vsubpd	-80(%rdx,%r14,8), %xmm4, %xmm4
	vsubpd	-64(%rdx,%r14,8), %xmm5, %xmm5
	vaddpd	20720(%rax,%r14,8), %xmm4, %xmm4
	vaddpd	20736(%rax,%r14,8), %xmm5, %xmm5
	vsubpd	-80(%rax,%r14,8), %xmm4, %xmm4
	vsubpd	-64(%rax,%r14,8), %xmm5, %xmm5
	vfmadd213pd	-80(%rsi,%r14,8), %xmm2, %xmm4
	vfmadd213pd	-64(%rsi,%r14,8), %xmm2, %xmm5
	vmovupd	%xmm4, -80(%rsi,%r14,8)
	vmovupd	%xmm5, -64(%rsi,%r14,8)
	vmovupd	-40(%rdx,%r14,8), %xmm4
	vmovupd	-24(%rdx,%r14,8), %xmm5
	vsubpd	-48(%rdx,%r14,8), %xmm4, %xmm4
	vsubpd	-32(%rdx,%r14,8), %xmm5, %xmm5
	vaddpd	20752(%rax,%r14,8), %xmm4, %xmm4
	vaddpd	20768(%rax,%r14,8), %xmm5, %xmm5
	vsubpd	-48(%rax,%r14,8), %xmm4, %xmm4
	vsubpd	-32(%rax,%r14,8), %xmm5, %xmm5
	vfmadd213pd	-48(%rsi,%r14,8), %xmm2, %xmm4
	vfmadd213pd	-32(%rsi,%r14,8), %xmm2, %xmm5
	vmovupd	%xmm4, -48(%rsi,%r14,8)
	vmovupd	%xmm5, -32(%rsi,%r14,8)
	vmovupd	-8(%rdx,%r14,8), %xmm4
	vmovupd	8(%rdx,%r14,8), %xmm5
	vsubpd	-16(%rdx,%r14,8), %xmm4, %xmm4
	vsubpd	(%rdx,%r14,8), %xmm5, %xmm5
	vaddpd	20784(%rax,%r14,8), %xmm4, %xmm4
	vaddpd	20800(%rax,%r14,8), %xmm5, %xmm5
	vsubpd	-16(%rax,%r14,8), %xmm4, %xmm4
	vsubpd	(%rax,%r14,8), %xmm5, %xmm5
	vfmadd213pd	-16(%rsi,%r14,8), %xmm2, %xmm4
	vfmadd213pd	(%rsi,%r14,8), %xmm2, %xmm5
	vmovupd	%xmm4, -16(%rsi,%r14,8)
	vmovupd	%xmm5, (%rsi,%r14,8)
	addq	$16, %r14
	jmp	-378
	nopw	(%rax,%rax)
	xorl	%r14d, %r14d
	jmp	17
	nopw	%cs:(%rax,%rax)
	movl	$2596, %r14d
	movl	%r14d, %edi
	andl	$4, %edi
	leaq	(%rax,%r14,8), %r10
	movl	$2600, %r11d
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%rdx,%r14,8), %xmm4
	vsubsd	(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	(%r10,%r11,8), %xmm4, %xmm4
	vsubsd	-20800(%r10,%r11,8), %xmm4, %xmm4
	vfmadd213sd	(%rsi,%r14,8), %xmm3, %xmm4
	vmovsd	%xmm4, (%rsi,%r14,8)
	incq	%r14
	leaq	-2599(%r11), %r8
	xorq	%rdi, %r8
	incq	%r11
	cmpq	$7, %r8
	jne	-63
	testb	%r9b, %r9b
	jne	-568
	nopl	(%rax,%rax)
	vmovsd	8(%rdx,%r14,8), %xmm4
	vsubsd	(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	20800(%rax,%r14,8), %xmm4, %xmm4
	vsubsd	(%rax,%r14,8), %xmm4, %xmm4
	vfmadd213sd	(%rsi,%r14,8), %xmm3, %xmm4
	vmovsd	%xmm4, (%rsi,%r14,8)
	vmovsd	16(%rdx,%r14,8), %xmm4
	vsubsd	8(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	20808(%rax,%r14,8), %xmm4, %xmm4
	vsubsd	8(%rax,%r14,8), %xmm4, %xmm4
	vfmadd213sd	8(%rsi,%r14,8), %xmm3, %xmm4
	vmovsd	%xmm4, 8(%rsi,%r14,8)
	vmovsd	24(%rdx,%r14,8), %xmm4
	vsubsd	16(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	20816(%rax,%r14,8), %xmm4, %xmm4
	vsubsd	16(%rax,%r14,8), %xmm4, %xmm4
	vfmadd213sd	16(%rsi,%r14,8), %xmm3, %xmm4
	vmovsd	%xmm4, 16(%rsi,%r14,8)
	vmovsd	32(%rdx,%r14,8), %xmm4
	vsubsd	24(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	20824(%rax,%r14,8), %xmm4, %xmm4
	vsubsd	24(%rax,%r14,8), %xmm4, %xmm4
	vfmadd213sd	24(%rsi,%r14,8), %xmm3, %xmm4
	vmovsd	%xmm4, 24(%rsi,%r14,8)
	vmovsd	40(%rdx,%r14,8), %xmm4
	vsubsd	32(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	20832(%rax,%r14,8), %xmm4, %xmm4
	vsubsd	32(%rax,%r14,8), %xmm4, %xmm4
	vfmadd213sd	32(%rsi,%r14,8), %xmm3, %xmm4
	vmovsd	%xmm4, 32(%rsi,%r14,8)
	vmovsd	48(%rdx,%r14,8), %xmm4
	vsubsd	40(%rdx,%r14,8), %xmm4, %xmm4
	vaddsd	20840(%rax,%r14,8), %xmm4, %xmm4
