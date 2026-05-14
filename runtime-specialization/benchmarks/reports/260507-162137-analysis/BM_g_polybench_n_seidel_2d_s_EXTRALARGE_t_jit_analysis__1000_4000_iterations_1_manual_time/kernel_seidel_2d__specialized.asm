	leaq	-7(%rip), %rax
	movabsq	$-7968, %rdi
	addq	%rax, %rdi
	movabsq	$8, %rax
	movq	(%rdi,%rax), %rax
	movq	(%rax), %rax
	leaq	64040(%rax), %rcx
	leaq	96040(%rax), %rdx
	xorl	%esi, %esi
	movabsq	$-1584, %r8
	vmovsd	(%rdi,%r8), %xmm0
	jmp	23
	nopw	(%rax,%rax)
	incl	%esi
	cmpl	$1000, %esi
	je	796
	movl	$1, %edi
	movq	%rdx, %r8
	movq	%rcx, %r9
	jmp	32
	nopl	(%rax,%rax)
	addq	$2, %rdi
	addq	$64000, %r9
	addq	$64000, %r8
	cmpq	$3999, %rdi
	je	-59
	imulq	$32000, %rdi, %r10
	vmovsd	-32000(%rax,%r10), %xmm3
	vmovsd	-31992(%rax,%r10), %xmm2
	vmovsd	(%rax,%r10), %xmm1
	vmovsd	8(%rax,%r10), %xmm7
	vmovsd	32000(%rax,%r10), %xmm4
	vmovsd	32008(%rax,%r10), %xmm5
	movq	$-3996, %r11
	vmovapd	%xmm1, %xmm8
	nopw	%cs:(%rax,%rax)
	vaddsd	%xmm3, %xmm2, %xmm9
	vmovsd	-32056(%r9,%r11,8), %xmm3
	vmovsd	-32048(%r9,%r11,8), %xmm6
	vaddsd	%xmm3, %xmm9, %xmm9
	vaddsd	%xmm9, %xmm8, %xmm8
	vaddsd	%xmm7, %xmm8, %xmm7
	vmovsd	-56(%r9,%r11,8), %xmm8
	vaddsd	%xmm7, %xmm8, %xmm7
	vaddsd	%xmm7, %xmm4, %xmm4
	vaddsd	%xmm4, %xmm5, %xmm7
	vmovsd	31944(%r9,%r11,8), %xmm4
	vaddsd	%xmm7, %xmm4, %xmm7
	vdivsd	%xmm0, %xmm7, %xmm7
	vmovsd	%xmm7, -64(%r9,%r11,8)
	vaddsd	%xmm3, %xmm2, %xmm2
	vaddsd	%xmm6, %xmm2, %xmm2
	vaddsd	%xmm7, %xmm2, %xmm2
	vaddsd	%xmm2, %xmm8, %xmm2
	vmovsd	-48(%r9,%r11,8), %xmm7
	vaddsd	%xmm2, %xmm7, %xmm2
	vaddsd	%xmm2, %xmm5, %xmm2
	vaddsd	%xmm2, %xmm4, %xmm2
	vmovsd	31952(%r9,%r11,8), %xmm5
	vaddsd	%xmm2, %xmm5, %xmm2
	vdivsd	%xmm0, %xmm2, %xmm8
	vmovsd	%xmm8, -56(%r9,%r11,8)
	testq	%r11, %r11
	je	154
	vaddsd	%xmm6, %xmm3, %xmm9
	vmovsd	-32040(%r9,%r11,8), %xmm3
	vmovsd	-32032(%r9,%r11,8), %xmm2
	vaddsd	%xmm3, %xmm9, %xmm9
	vaddsd	%xmm9, %xmm8, %xmm8
	vaddsd	%xmm7, %xmm8, %xmm7
	vmovsd	-40(%r9,%r11,8), %xmm8
	vaddsd	%xmm7, %xmm8, %xmm7
	vaddsd	%xmm7, %xmm4, %xmm4
	vaddsd	%xmm4, %xmm5, %xmm7
	vmovsd	31960(%r9,%r11,8), %xmm4
	vaddsd	%xmm7, %xmm4, %xmm7
	vdivsd	%xmm0, %xmm7, %xmm7
	vmovsd	%xmm7, -48(%r9,%r11,8)
	vaddsd	%xmm3, %xmm6, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vaddsd	%xmm7, %xmm6, %xmm6
	vaddsd	%xmm6, %xmm8, %xmm6
	vmovsd	-32(%r9,%r11,8), %xmm7
	vaddsd	%xmm6, %xmm7, %xmm6
	vaddsd	%xmm6, %xmm5, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm6
	vmovsd	31968(%r9,%r11,8), %xmm5
	vaddsd	%xmm6, %xmm5, %xmm6
	vdivsd	%xmm0, %xmm6, %xmm8
	vmovsd	%xmm8, -40(%r9,%r11,8)
	addq	$4, %r11
	jmp	-300
	nopl	(%rax)
	vmovsd	8(%r10,%rax), %xmm2
	vmovsd	32000(%r10,%rax), %xmm7
	vmovsd	32008(%r10,%rax), %xmm6
	vmovsd	64000(%r10,%rax), %xmm3
	vmovsd	64008(%r10,%rax), %xmm4
	movq	$-3996, %r10
	nopw	%cs:(%rax,%rax)
	vaddsd	%xmm1, %xmm2, %xmm8
	vmovsd	-32056(%r8,%r10,8), %xmm1
	vmovsd	-32048(%r8,%r10,8), %xmm5
	vaddsd	%xmm1, %xmm8, %xmm8
	vaddsd	%xmm7, %xmm8, %xmm7
	vaddsd	%xmm7, %xmm6, %xmm6
	vmovsd	-56(%r8,%r10,8), %xmm7
	vaddsd	%xmm6, %xmm7, %xmm6
	vaddsd	%xmm6, %xmm3, %xmm3
	vaddsd	%xmm3, %xmm4, %xmm6
	vmovsd	31944(%r8,%r10,8), %xmm3
	vaddsd	%xmm6, %xmm3, %xmm6
	vdivsd	%xmm0, %xmm6, %xmm6
	vmovsd	%xmm6, -64(%r8,%r10,8)
	vaddsd	%xmm1, %xmm2, %xmm2
	vaddsd	%xmm5, %xmm2, %xmm2
	vaddsd	%xmm6, %xmm2, %xmm2
	vaddsd	%xmm2, %xmm7, %xmm2
	vmovsd	-48(%r8,%r10,8), %xmm6
	vaddsd	%xmm2, %xmm6, %xmm2
	vaddsd	%xmm2, %xmm4, %xmm2
	vaddsd	%xmm2, %xmm3, %xmm2
	vmovsd	31952(%r8,%r10,8), %xmm4
	vaddsd	%xmm2, %xmm4, %xmm2
	vdivsd	%xmm0, %xmm2, %xmm7
	vmovsd	%xmm7, -56(%r8,%r10,8)
	testq	%r10, %r10
	je	-629
	vaddsd	%xmm5, %xmm1, %xmm8
	vmovsd	-32040(%r8,%r10,8), %xmm1
	vmovsd	-32032(%r8,%r10,8), %xmm2
	vaddsd	%xmm1, %xmm8, %xmm8
	vaddsd	%xmm7, %xmm8, %xmm7
	vaddsd	%xmm7, %xmm6, %xmm6
	vmovsd	-40(%r8,%r10,8), %xmm7
	vaddsd	%xmm6, %xmm7, %xmm6
	vaddsd	%xmm6, %xmm3, %xmm3
	vaddsd	%xmm3, %xmm4, %xmm6
	vmovsd	31960(%r8,%r10,8), %xmm3
	vaddsd	%xmm6, %xmm3, %xmm6
	vdivsd	%xmm0, %xmm6, %xmm6
	vmovsd	%xmm6, -48(%r8,%r10,8)
	vaddsd	%xmm1, %xmm5, %xmm5
	vaddsd	%xmm2, %xmm5, %xmm5
	vaddsd	%xmm6, %xmm5, %xmm5
	vaddsd	%xmm5, %xmm7, %xmm5
	vmovsd	-32(%r8,%r10,8), %xmm6
	vaddsd	%xmm5, %xmm6, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vaddsd	%xmm4, %xmm3, %xmm5
	vmovsd	31968(%r8,%r10,8), %xmm4
	vaddsd	%xmm5, %xmm4, %xmm5
	vdivsd	%xmm0, %xmm5, %xmm7
	vmovsd	%xmm7, -40(%r8,%r10,8)
	addq	$4, %r10
	jmp	-298
	retq
