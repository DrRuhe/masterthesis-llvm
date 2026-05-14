	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$56, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-7966, %rsi
	addq	%rax, %rsi
	movabsq	$56, %rax
	movq	(%rsi,%rax), %rax
	movq	(%rax), %rbx
	movabsq	$152, %rax
	movq	(%rsi,%rax), %rax
	movq	(%rax), %r14
	xorl	%eax, %eax
	movabsq	$104, %rcx
	movq	(%rsi,%rcx), %r15
	movabsq	$-1600, %rcx
	vmovsd	(%rsi,%rcx), %xmm3
	movq	%rbx, %rcx
	nopw	%cs:(%rax,%rax)
	movq	$0, (%r15,%rax,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	movl	$478400, %edx
	nopw	%cs:(%rax,%rax)
	vaddsd	-478400(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-457600(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-436800(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-416000(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-395200(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-374400(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-353600(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-332800(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-312000(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-291200(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-270400(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-249600(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-228800(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-208000(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-187200(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-166400(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-145600(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-124800(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-104000(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-83200(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-62400(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-41600(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	-20800(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	vaddsd	(%rcx,%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	addq	$499200, %rdx
	cmpq	$62878400, %rdx
	jne	-376
	vdivsd	%xmm3, %xmm0, %xmm0
	vmovsd	%xmm0, (%r15,%rax,8)
	incq	%rax
	addq	$8, %rcx
	cmpq	$2600, %rax
	jne	-437
	xorl	%ebp, %ebp
	movabsq	$184, %rax
	movq	(%rsi,%rax), %r13
	vxorpd	%xmm4, %xmm4, %xmm4
	movabsq	$-1592, %rax
	vmovsd	(%rsi,%rax), %xmm5
	movabsq	$-1584, %rax
	vmovsd	(%rsi,%rax), %xmm6
	movabsq	$208, %rax
	movq	%rsi, (%rsp)
	movq	(%rsi,%rax), %rax
	movq	%rax, 8(%rsp)
	movq	%rbx, %r12
	vmovsd	%xmm3, 24(%rsp)
	vmovsd	%xmm5, 16(%rsp)
	vmovapd	%xmm6, 32(%rsp)
	jmp	75
	nopw	%cs:(%rax,%rax)
	callq	*8(%rsp)
	vmovapd	32(%rsp), %xmm6
	vmovsd	16(%rsp), %xmm5
	vxorpd	%xmm4, %xmm4, %xmm4
	vmovsd	24(%rsp), %xmm3
	vcmpnlesd	%xmm5, %xmm0, %xmm1
	vblendvpd	%xmm1, %xmm0, %xmm6, %xmm0
	vmovlpd	%xmm0, (%r13,%rbp,8)
	incq	%rbp
	addq	$8, %r12
	cmpq	$2600, %rbp
	je	589
	movq	$0, (%r13,%rbp,8)
	vmovsd	(%r15,%rbp,8), %xmm0
	movl	$395200, %eax
	vxorpd	%xmm1, %xmm1, %xmm1
	nopl	(%rax,%rax)
	vmovsd	-395200(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-374400(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-353600(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-332800(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-312000(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-291200(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-270400(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-249600(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-228800(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-208000(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-187200(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-166400(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-145600(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-124800(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-104000(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-83200(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-62400(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	-41600(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	vmovsd	-20800(%r12,%rax), %xmm2
	vsubsd	%xmm0, %xmm2, %xmm2
	vfmadd213sd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r13,%rbp,8)
	vmovsd	(%r12,%rax), %xmm1
	vsubsd	%xmm0, %xmm1, %xmm1
	vfmadd213sd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13,%rbp,8)
	addq	$416000, %rax
	cmpq	$62795200, %rax
	jne	-534
	vdivsd	%xmm3, %xmm1, %xmm0
	vucomisd	%xmm4, %xmm0
	jb	-644
	vsqrtsd	%xmm0, %xmm0, %xmm0
	jmp	-627
	xorl	%eax, %eax
	movabsq	$-1576, %rcx
	movq	(%rsp), %rdx
	vmovsd	(%rdx,%rcx), %xmm0
	movq	%rbx, %rcx
	nopw	%cs:(%rax,%rax)
	movl	$7, %edx
	nopw	%cs:(%rax,%rax)
	vmovsd	-56(%rcx,%rdx,8), %xmm1
	vsubsd	-56(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	-48(%rcx,%rdx,8), %xmm2
	vmovsd	%xmm1, -56(%rcx,%rdx,8)
	vmulsd	-56(%r13,%rdx,8), %xmm0, %xmm3
	vdivsd	%xmm3, %xmm1, %xmm1
	vmovsd	%xmm1, -56(%rcx,%rdx,8)
	vsubsd	-48(%r15,%rdx,8), %xmm2, %xmm1
	vmovsd	%xmm1, -48(%rcx,%rdx,8)
	vmulsd	-48(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, -48(%rcx,%rdx,8)
	vmovsd	-40(%rcx,%rdx,8), %xmm1
	vsubsd	-40(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	%xmm1, -40(%rcx,%rdx,8)
	vmulsd	-40(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, -40(%rcx,%rdx,8)
	vmovsd	-32(%rcx,%rdx,8), %xmm1
	vsubsd	-32(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	%xmm1, -32(%rcx,%rdx,8)
	vmulsd	-32(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, -32(%rcx,%rdx,8)
	vmovsd	-24(%rcx,%rdx,8), %xmm1
	vsubsd	-24(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	%xmm1, -24(%rcx,%rdx,8)
	vmulsd	-24(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, -24(%rcx,%rdx,8)
	vmovsd	-16(%rcx,%rdx,8), %xmm1
	vsubsd	-16(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	%xmm1, -16(%rcx,%rdx,8)
	vmulsd	-16(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, -16(%rcx,%rdx,8)
	vmovsd	-8(%rcx,%rdx,8), %xmm1
	vsubsd	-8(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	%xmm1, -8(%rcx,%rdx,8)
	vmulsd	-8(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, -8(%rcx,%rdx,8)
	vmovsd	(%rcx,%rdx,8), %xmm1
	vsubsd	(%r15,%rdx,8), %xmm1, %xmm1
	vmovsd	%xmm1, (%rcx,%rdx,8)
	vmulsd	(%r13,%rdx,8), %xmm0, %xmm2
	vdivsd	%xmm2, %xmm1, %xmm1
	vmovsd	%xmm1, (%rcx,%rdx,8)
	addq	$8, %rdx
	cmpq	$2607, %rdx
	jne	-301
	incq	%rax
	addq	$20800, %rcx
	cmpq	$3000, %rax
	jne	-339
	leaq	228808(%rbx), %rax
	movl	$1, %ecx
	xorl	%edx, %edx
	movabsq	$4607182418800017408, %rsi
	nopl	(%rax,%rax)
	imulq	$20800, %rdx, %rdi
	addq	%r14, %rdi
	movq	%rsi, (%rdi,%rdx,8)
	movq	%rax, %r8
	movq	%rcx, %r9
	nopw	%cs:(%rax,%rax)
	leaq	(%rdi,%r9,8), %r10
	movq	$0, (%rdi,%r9,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	xorl	%r11d, %r11d
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rbx,%r11), %xmm1
	vfmadd132sd	-228800(%r8,%r11), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	20800(%rbx,%r11), %xmm0
	vfmadd132sd	-208000(%r8,%r11), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	41600(%rbx,%r11), %xmm1
	vfmadd132sd	-187200(%r8,%r11), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	62400(%rbx,%r11), %xmm0
	vfmadd132sd	-166400(%r8,%r11), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	83200(%rbx,%r11), %xmm1
	vfmadd132sd	-145600(%r8,%r11), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	104000(%rbx,%r11), %xmm0
	vfmadd132sd	-124800(%r8,%r11), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	124800(%rbx,%r11), %xmm1
	vfmadd132sd	-104000(%r8,%r11), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	145600(%rbx,%r11), %xmm0
	vfmadd132sd	-83200(%r8,%r11), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	166400(%rbx,%r11), %xmm1
	vfmadd132sd	-62400(%r8,%r11), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	187200(%rbx,%r11), %xmm0
	vfmadd132sd	-41600(%r8,%r11), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	208000(%rbx,%r11), %xmm1
	vfmadd132sd	-20800(%r8,%r11), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	228800(%rbx,%r11), %xmm0
	vfmadd132sd	(%r8,%r11), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	addq	$249600, %r11
	cmpq	$62400000, %r11
	jne	-312
	imulq	$20800, %r9, %r10
	addq	%r14, %r10
	vmovsd	%xmm0, (%r10,%rdx,8)
	incq	%r9
	addq	$8, %r8
	cmpq	$2600, %r9
	jne	-380
	incq	%rdx
	incq	%rcx
	addq	$8, %rax
	addq	$8, %rbx
	cmpq	$2599, %rdx
	jne	-439
	movq	%rsi, 54079992(%r14)
	addq	$56, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
