	leaq	-7(%rip), %rax
	movabsq	$-7952, %rsi
	addq	%rax, %rsi
	xorl	%eax, %eax
	movabsq	$56, %rcx
	movq	(%rsi,%rcx), %rcx
	movabsq	$-1600, %rdx
	vmovapd	(%rsi,%rdx), %xmm0
	movabsq	$8, %rdx
	movq	(%rsi,%rdx), %rdx
	movabsq	$-1584, %rdi
	vmovsd	(%rsi,%rdi), %xmm1
	jmp	80
	nopw	%cs:(%rax,%rax)
	vshufpd	$1, %xmm4, %xmm4, %xmm4
	vaddsd	31968(%rdx), %xmm4, %xmm4
	vaddsd	%xmm4, %xmm3, %xmm4
	vmulsd	%xmm1, %xmm4, %xmm4
	vmovsd	%xmm4, 31976(%rcx)
	vaddsd	%xmm3, %xmm2, %xmm2
	vaddsd	31992(%rdx), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 31984(%rcx)
	incl	%eax
	cmpl	$1000, %eax
	je	894
	vmovddup	8(%rcx), %xmm2
	xorl	%esi, %esi
	nopl	(%rax)
	vmovapd	16(%rcx,%rsi,8), %xmm3
	vmovapd	32(%rcx,%rsi,8), %xmm4
	vmovapd	48(%rcx,%rsi,8), %xmm5
	vmovapd	64(%rcx,%rsi,8), %xmm6
	vshufpd	$1, %xmm3, %xmm2, %xmm2
	vshufpd	$1, %xmm4, %xmm3, %xmm7
	vaddpd	(%rcx,%rsi,8), %xmm2, %xmm2
	vaddpd	%xmm7, %xmm3, %xmm7
	vaddpd	%xmm2, %xmm3, %xmm2
	vaddpd	%xmm7, %xmm4, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmovupd	%xmm2, 8(%rdx,%rsi,8)
	vmulpd	%xmm0, %xmm3, %xmm2
	vmovupd	%xmm2, 24(%rdx,%rsi,8)
	vshufpd	$1, %xmm5, %xmm4, %xmm2
	vshufpd	$1, %xmm6, %xmm5, %xmm3
	vaddpd	%xmm2, %xmm4, %xmm2
	vaddpd	%xmm3, %xmm5, %xmm3
	vaddpd	%xmm2, %xmm5, %xmm2
	vaddpd	%xmm3, %xmm6, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmovupd	%xmm2, 40(%rdx,%rsi,8)
	vmulpd	%xmm0, %xmm3, %xmm2
	vmovupd	%xmm2, 56(%rdx,%rsi,8)
	vmovapd	80(%rcx,%rsi,8), %xmm3
	vmovapd	96(%rcx,%rsi,8), %xmm2
	vshufpd	$1, %xmm3, %xmm6, %xmm4
	vshufpd	$1, %xmm2, %xmm3, %xmm5
	vaddpd	%xmm4, %xmm6, %xmm4
	vaddpd	%xmm5, %xmm3, %xmm5
	vaddpd	%xmm4, %xmm3, %xmm3
	vaddpd	%xmm5, %xmm2, %xmm4
	vmulpd	%xmm0, %xmm3, %xmm3
	vmulpd	%xmm0, %xmm4, %xmm4
	vmovupd	%xmm3, 72(%rdx,%rsi,8)
	vmovupd	%xmm4, 88(%rdx,%rsi,8)
	cmpq	$3984, %rsi
	je	212
	vmovapd	112(%rcx,%rsi,8), %xmm3
	vmovapd	128(%rcx,%rsi,8), %xmm4
	vmovapd	144(%rcx,%rsi,8), %xmm5
	vmovapd	160(%rcx,%rsi,8), %xmm6
	vshufpd	$1, %xmm3, %xmm2, %xmm7
	vshufpd	$1, %xmm4, %xmm3, %xmm8
	vaddpd	%xmm7, %xmm2, %xmm2
	vaddpd	%xmm3, %xmm8, %xmm7
	vaddpd	%xmm2, %xmm3, %xmm2
	vaddpd	%xmm7, %xmm4, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmovupd	%xmm2, 104(%rdx,%rsi,8)
	vmulpd	%xmm0, %xmm3, %xmm2
	vmovupd	%xmm2, 120(%rdx,%rsi,8)
	vshufpd	$1, %xmm5, %xmm4, %xmm2
	vshufpd	$1, %xmm6, %xmm5, %xmm3
	vaddpd	%xmm2, %xmm4, %xmm2
	vaddpd	%xmm3, %xmm5, %xmm3
	vaddpd	%xmm2, %xmm5, %xmm2
	vaddpd	%xmm3, %xmm6, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmovupd	%xmm2, 136(%rdx,%rsi,8)
	vmulpd	%xmm0, %xmm3, %xmm2
	vmovupd	%xmm2, 152(%rdx,%rsi,8)
	vmovapd	176(%rcx,%rsi,8), %xmm3
	vmovapd	192(%rcx,%rsi,8), %xmm2
	vshufpd	$1, %xmm3, %xmm6, %xmm4
	vshufpd	$1, %xmm2, %xmm3, %xmm5
	vaddpd	%xmm4, %xmm6, %xmm4
	vaddpd	%xmm5, %xmm3, %xmm5
	vaddpd	%xmm4, %xmm3, %xmm3
	vaddpd	%xmm5, %xmm2, %xmm4
	vmulpd	%xmm0, %xmm3, %xmm3
	vmulpd	%xmm0, %xmm4, %xmm4
	vmovupd	%xmm3, 168(%rdx,%rsi,8)
	vmovupd	%xmm4, 184(%rdx,%rsi,8)
	addq	$24, %rsi
	jmp	-398
	nop
	vshufpd	$1, %xmm2, %xmm2, %xmm2
	vaddsd	31968(%rcx), %xmm2, %xmm2
	vmovsd	31984(%rcx), %xmm3
	vaddsd	%xmm3, %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 31976(%rdx)
	vaddsd	31976(%rcx), %xmm3, %xmm3
	vaddsd	31992(%rcx), %xmm3, %xmm3
	vmulsd	%xmm1, %xmm3, %xmm3
	vmovsd	%xmm3, 31984(%rdx)
	vmovddup	8(%rdx), %xmm4
	xorl	%esi, %esi
	nopl	(%rax,%rax)
	vmovapd	16(%rdx,%rsi,8), %xmm5
	vmovapd	32(%rdx,%rsi,8), %xmm6
	vmovapd	48(%rdx,%rsi,8), %xmm7
	vmovapd	64(%rdx,%rsi,8), %xmm8
	vshufpd	$1, %xmm5, %xmm4, %xmm4
	vshufpd	$1, %xmm6, %xmm5, %xmm9
	vaddpd	(%rdx,%rsi,8), %xmm4, %xmm4
	vaddpd	%xmm5, %xmm9, %xmm9
	vaddpd	%xmm4, %xmm5, %xmm4
	vaddpd	%xmm6, %xmm9, %xmm5
	vmulpd	%xmm0, %xmm4, %xmm4
	vmovupd	%xmm4, 8(%rcx,%rsi,8)
	vmulpd	%xmm0, %xmm5, %xmm4
	vmovupd	%xmm4, 24(%rcx,%rsi,8)
	vshufpd	$1, %xmm7, %xmm6, %xmm4
	vshufpd	$1, %xmm8, %xmm7, %xmm5
	vaddpd	%xmm4, %xmm6, %xmm4
	vaddpd	%xmm5, %xmm7, %xmm5
	vaddpd	%xmm4, %xmm7, %xmm4
	vaddpd	%xmm5, %xmm8, %xmm5
	vmulpd	%xmm0, %xmm4, %xmm4
	vmovupd	%xmm4, 40(%rcx,%rsi,8)
	vmulpd	%xmm0, %xmm5, %xmm4
	vmovupd	%xmm4, 56(%rcx,%rsi,8)
	vmovapd	80(%rdx,%rsi,8), %xmm5
	vmovapd	96(%rdx,%rsi,8), %xmm4
	vshufpd	$1, %xmm5, %xmm8, %xmm6
	vshufpd	$1, %xmm4, %xmm5, %xmm7
	vaddpd	%xmm6, %xmm8, %xmm6
	vaddpd	%xmm7, %xmm5, %xmm7
	vaddpd	%xmm6, %xmm5, %xmm5
	vaddpd	%xmm7, %xmm4, %xmm6
	vmulpd	%xmm0, %xmm5, %xmm5
	vmulpd	%xmm0, %xmm6, %xmm6
	vmovupd	%xmm5, 72(%rcx,%rsi,8)
	vmovupd	%xmm6, 88(%rcx,%rsi,8)
	cmpq	$3984, %rsi
	je	-749
	vmovapd	112(%rdx,%rsi,8), %xmm5
	vmovapd	128(%rdx,%rsi,8), %xmm6
	vmovapd	144(%rdx,%rsi,8), %xmm7
	vmovapd	160(%rdx,%rsi,8), %xmm8
	vshufpd	$1, %xmm5, %xmm4, %xmm9
	vshufpd	$1, %xmm6, %xmm5, %xmm10
	vaddpd	%xmm4, %xmm9, %xmm4
	vaddpd	%xmm5, %xmm10, %xmm9
	vaddpd	%xmm4, %xmm5, %xmm4
	vaddpd	%xmm6, %xmm9, %xmm5
	vmulpd	%xmm0, %xmm4, %xmm4
	vmovupd	%xmm4, 104(%rcx,%rsi,8)
	vmulpd	%xmm0, %xmm5, %xmm4
	vmovupd	%xmm4, 120(%rcx,%rsi,8)
	vshufpd	$1, %xmm7, %xmm6, %xmm4
	vshufpd	$1, %xmm8, %xmm7, %xmm5
	vaddpd	%xmm4, %xmm6, %xmm4
	vaddpd	%xmm5, %xmm7, %xmm5
	vaddpd	%xmm4, %xmm7, %xmm4
	vaddpd	%xmm5, %xmm8, %xmm5
	vmulpd	%xmm0, %xmm4, %xmm4
	vmovupd	%xmm4, 136(%rcx,%rsi,8)
	vmulpd	%xmm0, %xmm5, %xmm4
	vmovupd	%xmm4, 152(%rcx,%rsi,8)
	vmovapd	176(%rdx,%rsi,8), %xmm5
	vmovapd	192(%rdx,%rsi,8), %xmm4
	vshufpd	$1, %xmm5, %xmm8, %xmm6
	vshufpd	$1, %xmm4, %xmm5, %xmm7
	vaddpd	%xmm6, %xmm8, %xmm6
	vaddpd	%xmm7, %xmm5, %xmm7
	vaddpd	%xmm6, %xmm5, %xmm5
	vaddpd	%xmm7, %xmm4, %xmm6
	vmulpd	%xmm0, %xmm5, %xmm5
	vmulpd	%xmm0, %xmm6, %xmm6
	vmovupd	%xmm5, 168(%rcx,%rsi,8)
	vmovupd	%xmm6, 184(%rcx,%rsi,8)
	addq	$24, %rsi
	jmp	-400
	retq
