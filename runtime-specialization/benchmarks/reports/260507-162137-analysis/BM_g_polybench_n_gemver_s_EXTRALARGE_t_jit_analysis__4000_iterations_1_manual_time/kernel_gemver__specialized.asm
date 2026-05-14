	leaq	-7(%rip), %rax
	movabsq	$-7952, %rcx
	addq	%rax, %rcx
	movabsq	$184, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	leaq	128000000(%rax), %rdx
	vmovq	%rax, %xmm0
	vpbroadcastq	%xmm0, %ymm0
	vmovq	%rdx, %xmm1
	vpbroadcastq	%xmm1, %ymm1
	movabsq	$136, %rdx
	movq	(%rcx,%rdx), %rdx
	leaq	32000(%rdx), %rsi
	vmovq	%rsi, %xmm2
	movabsq	$160, %rsi
	movq	(%rcx,%rsi), %rsi
	leaq	32000(%rsi), %rdi
	vmovq	%rdi, %xmm3
	vpunpcklqdq	%xmm2, %xmm3, %xmm2
	movabsq	$104, %rdi
	movq	(%rcx,%rdi), %rdi
	leaq	32000(%rdi), %r8
	vmovq	%r8, %xmm3
	movabsq	$64, %r8
	movq	(%rcx,%r8), %r8
	leaq	32000(%r8), %r9
	vmovq	%r9, %xmm4
	vpunpcklqdq	%xmm3, %xmm4, %xmm3
	vinserti128	$1, %xmm2, %ymm3, %ymm2
	vmovq	%rdx, %xmm3
	vmovq	%rsi, %xmm4
	vpunpcklqdq	%xmm3, %xmm4, %xmm3
	vmovq	%rdi, %xmm4
	vmovq	%r8, %xmm5
	vpunpcklqdq	%xmm4, %xmm5, %xmm4
	vinserti128	$1, %xmm3, %ymm4, %ymm3
	movabsq	$-1600, %r9
	vpbroadcastq	(%rcx,%r9), %ymm4
	vpxor	%ymm4, %ymm3, %ymm3
	vpxor	%ymm4, %ymm1, %ymm1
	vpcmpgtq	%ymm3, %ymm1, %ymm1
	vpxor	%ymm4, %ymm2, %ymm2
	vpxor	%ymm4, %ymm0, %ymm0
	vpcmpgtq	%ymm0, %ymm2, %ymm0
	vpand	%ymm1, %ymm0, %ymm0
	vmovmskpd	%ymm0, %r9d
	testl	%r9d, %r9d
	je	470
	xorl	%r9d, %r9d
	movq	%rax, %r10
	nopl	(%rax)
	movl	$9, %r11d
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-72(%rdi,%r11,8), %xmm1
	vfmadd213sd	-72(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-72(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -72(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-64(%rdi,%r11,8), %xmm1
	vfmadd213sd	-64(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-64(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -64(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-56(%rdi,%r11,8), %xmm1
	vfmadd213sd	-56(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-56(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -56(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-48(%rdi,%r11,8), %xmm1
	vfmadd213sd	-48(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-48(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -48(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-40(%rdi,%r11,8), %xmm1
	vfmadd213sd	-40(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-40(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -40(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-32(%rdi,%r11,8), %xmm1
	vfmadd213sd	-32(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-32(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -32(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-24(%rdi,%r11,8), %xmm1
	vfmadd213sd	-24(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-24(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -24(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-16(%rdi,%r11,8), %xmm1
	vfmadd213sd	-16(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-16(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -16(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	-8(%rdi,%r11,8), %xmm1
	vfmadd213sd	-8(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	-8(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, -8(%r10,%r11,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vmovsd	(%rdi,%r11,8), %xmm1
	vfmadd213sd	(%r10,%r11,8), %xmm0, %xmm1
	vmovsd	(%rsi,%r9,8), %xmm0
	vfmadd132sd	(%rdx,%r11,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10,%r11,8)
	addq	$10, %r11
	cmpq	$4009, %r11
	jne	-413
	incq	%r9
	addq	$32000, %r10
	cmpq	$4000, %r9
	jne	-452
	jmp	367
	xorl	%r9d, %r9d
	movq	%rax, %r10
	nop
	vmovddup	(%r8,%r9,8), %xmm0
	vmovddup	(%rsi,%r9,8), %xmm1
	movl	$18, %r11d
	nopw	%cs:(%rax,%rax)
	vmovapd	-144(%rdi,%r11,8), %xmm2
	vmovapd	-128(%rdi,%r11,8), %xmm3
	vfmadd213pd	-144(%r10,%r11,8), %xmm0, %xmm2
	vfmadd213pd	-128(%r10,%r11,8), %xmm0, %xmm3
	vfmadd231pd	-144(%rdx,%r11,8), %xmm1, %xmm2
	vfmadd231pd	-128(%rdx,%r11,8), %xmm1, %xmm3
	vmovupd	%xmm2, -144(%r10,%r11,8)
	vmovupd	%xmm3, -128(%r10,%r11,8)
	vmovapd	-112(%rdi,%r11,8), %xmm2
	vmovapd	-96(%rdi,%r11,8), %xmm3
	vfmadd213pd	-112(%r10,%r11,8), %xmm0, %xmm2
	vfmadd213pd	-96(%r10,%r11,8), %xmm0, %xmm3
	vfmadd231pd	-112(%rdx,%r11,8), %xmm1, %xmm2
	vfmadd231pd	-96(%rdx,%r11,8), %xmm1, %xmm3
	vmovupd	%xmm2, -112(%r10,%r11,8)
	vmovupd	%xmm3, -96(%r10,%r11,8)
	vmovapd	-80(%rdi,%r11,8), %xmm2
	vmovapd	-64(%rdi,%r11,8), %xmm3
	vfmadd213pd	-80(%r10,%r11,8), %xmm0, %xmm2
	vfmadd213pd	-64(%r10,%r11,8), %xmm0, %xmm3
	vfmadd231pd	-80(%rdx,%r11,8), %xmm1, %xmm2
	vfmadd231pd	-64(%rdx,%r11,8), %xmm1, %xmm3
	vmovupd	%xmm2, -80(%r10,%r11,8)
	vmovupd	%xmm3, -64(%r10,%r11,8)
	vmovapd	-48(%rdi,%r11,8), %xmm2
	vmovapd	-32(%rdi,%r11,8), %xmm3
	vfmadd213pd	-48(%r10,%r11,8), %xmm0, %xmm2
	vfmadd213pd	-32(%r10,%r11,8), %xmm0, %xmm3
	vfmadd231pd	-48(%rdx,%r11,8), %xmm1, %xmm2
	vfmadd231pd	-32(%rdx,%r11,8), %xmm1, %xmm3
	vmovupd	%xmm2, -48(%r10,%r11,8)
	vmovupd	%xmm3, -32(%r10,%r11,8)
	vmovapd	-16(%rdi,%r11,8), %xmm2
	vmovapd	(%rdi,%r11,8), %xmm3
	vfmadd213pd	-16(%r10,%r11,8), %xmm0, %xmm2
	vfmadd213pd	(%r10,%r11,8), %xmm0, %xmm3
	vfmadd231pd	-16(%rdx,%r11,8), %xmm1, %xmm2
	vfmadd231pd	(%rdx,%r11,8), %xmm1, %xmm3
	vmovupd	%xmm2, -16(%r10,%r11,8)
	vmovupd	%xmm3, (%r10,%r11,8)
	addq	$20, %r11
	cmpq	$4018, %r11
	jne	-305
	incq	%r9
	addq	$32000, %r10
	cmpq	$4000, %r9
	jne	-360
	xorl	%esi, %esi
	movabsq	$8, %rdx
	movq	(%rcx,%rdx), %rdx
	movabsq	$-1592, %rdi
	vmovsd	(%rcx,%rdi), %xmm0
	movabsq	$48, %rdi
	movq	(%rcx,%rdi), %rdi
	movq	%rax, %r8
	nopl	(%rax,%rax)
	vmovsd	(%rdx,%rsi,8), %xmm1
	movl	$15, %r9d
	movq	%r8, %r10
	nop
	vmulsd	(%r10), %xmm0, %xmm2
	vfmadd132sd	-120(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	32000(%r10), %xmm0, %xmm1
	vfmadd132sd	-112(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	64000(%r10), %xmm0, %xmm2
	vfmadd132sd	-104(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	96000(%r10), %xmm0, %xmm1
	vfmadd132sd	-96(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	128000(%r10), %xmm0, %xmm2
	vfmadd132sd	-88(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	160000(%r10), %xmm0, %xmm1
	vfmadd132sd	-80(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	192000(%r10), %xmm0, %xmm2
	vfmadd132sd	-72(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	224000(%r10), %xmm0, %xmm1
	vfmadd132sd	-64(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	256000(%r10), %xmm0, %xmm2
	vfmadd132sd	-56(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	288000(%r10), %xmm0, %xmm1
	vfmadd132sd	-48(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	320000(%r10), %xmm0, %xmm2
	vfmadd132sd	-40(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	352000(%r10), %xmm0, %xmm1
	vfmadd132sd	-32(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	384000(%r10), %xmm0, %xmm2
	vfmadd132sd	-24(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	416000(%r10), %xmm0, %xmm1
	vfmadd132sd	-16(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	vmulsd	448000(%r10), %xmm0, %xmm2
	vfmadd132sd	-8(%rdi,%r9,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx,%rsi,8)
	vmulsd	480000(%r10), %xmm0, %xmm1
	vfmadd132sd	(%rdi,%r9,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx,%rsi,8)
	addq	$16, %r9
	addq	$512000, %r10
	cmpq	$4015, %r9
	jne	-355
	incq	%rsi
	addq	$8, %r8
	cmpq	$4000, %rsi
	jne	-391
	movl	$38, %esi
	movabsq	$24, %rdi
	movq	(%rcx,%rdi), %rdi
	nopw	(%rax,%rax)
	vmovapd	-304(%rdx,%rsi,8), %xmm0
	vmovapd	-288(%rdx,%rsi,8), %xmm1
	vmovapd	-272(%rdx,%rsi,8), %xmm2
	vmovapd	-256(%rdx,%rsi,8), %xmm3
	vaddpd	-304(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-288(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -304(%rdx,%rsi,8)
	vmovapd	%xmm1, -288(%rdx,%rsi,8)
	vaddpd	-272(%rdi,%rsi,8), %xmm2, %xmm0
	vaddpd	-256(%rdi,%rsi,8), %xmm3, %xmm1
	vmovapd	%xmm0, -272(%rdx,%rsi,8)
	vmovapd	%xmm1, -256(%rdx,%rsi,8)
	vmovapd	-240(%rdx,%rsi,8), %xmm0
	vmovapd	-224(%rdx,%rsi,8), %xmm1
	vaddpd	-240(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-224(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -240(%rdx,%rsi,8)
	vmovapd	%xmm1, -224(%rdx,%rsi,8)
	vmovapd	-208(%rdx,%rsi,8), %xmm0
	vmovapd	-192(%rdx,%rsi,8), %xmm1
	vaddpd	-208(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-192(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -208(%rdx,%rsi,8)
	vmovapd	%xmm1, -192(%rdx,%rsi,8)
	vmovapd	-176(%rdx,%rsi,8), %xmm0
	vmovapd	-160(%rdx,%rsi,8), %xmm1
	vaddpd	-176(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-160(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -176(%rdx,%rsi,8)
	vmovapd	%xmm1, -160(%rdx,%rsi,8)
	vmovapd	-144(%rdx,%rsi,8), %xmm0
	vmovapd	-128(%rdx,%rsi,8), %xmm1
	vaddpd	-144(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-128(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -144(%rdx,%rsi,8)
	vmovapd	%xmm1, -128(%rdx,%rsi,8)
	vmovapd	-112(%rdx,%rsi,8), %xmm0
	vmovapd	-96(%rdx,%rsi,8), %xmm1
	vaddpd	-112(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-96(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -112(%rdx,%rsi,8)
	vmovapd	%xmm1, -96(%rdx,%rsi,8)
	vmovapd	-80(%rdx,%rsi,8), %xmm0
	vmovapd	-64(%rdx,%rsi,8), %xmm1
	vaddpd	-80(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-64(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -80(%rdx,%rsi,8)
	vmovapd	%xmm1, -64(%rdx,%rsi,8)
	vmovapd	-48(%rdx,%rsi,8), %xmm0
	vmovapd	-32(%rdx,%rsi,8), %xmm1
	vaddpd	-48(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	-32(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -48(%rdx,%rsi,8)
	vmovapd	%xmm1, -32(%rdx,%rsi,8)
	vmovapd	-16(%rdx,%rsi,8), %xmm0
	vmovapd	(%rdx,%rsi,8), %xmm1
	vaddpd	-16(%rdi,%rsi,8), %xmm0, %xmm0
	vaddpd	(%rdi,%rsi,8), %xmm1, %xmm1
	vmovapd	%xmm0, -16(%rdx,%rsi,8)
	vmovapd	%xmm1, (%rdx,%rsi,8)
	addq	$40, %rsi
	cmpq	$4038, %rsi
	jne	-473
	xorl	%esi, %esi
	movabsq	$120, %rdi
	movq	(%rcx,%rdi), %rdi
	movabsq	$-1584, %r8
	vmovsd	(%rcx,%r8), %xmm0
	nopl	(%rax)
	vmovsd	(%rdi,%rsi,8), %xmm1
	movl	$15, %ecx
	nopw	(%rax,%rax)
	vmulsd	-120(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-120(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-112(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-112(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-104(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-104(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-96(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-96(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-88(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-88(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-80(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-80(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-72(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-72(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-64(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-64(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-56(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-56(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-48(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-48(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-40(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-40(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-32(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-32(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-24(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-24(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	-16(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	-16(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	vmulsd	-8(%rax,%rcx,8), %xmm0, %xmm2
	vfmadd132sd	-8(%rdx,%rcx,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdi,%rsi,8)
	vmulsd	(%rax,%rcx,8), %xmm0, %xmm1
	vfmadd132sd	(%rdx,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdi,%rsi,8)
	addq	$16, %rcx
	cmpq	$4015, %rcx
	jne	-303
	incq	%rsi
	addq	$32000, %rax
	cmpq	$4000, %rsi
	jne	-341
	vzeroupper
	retq
