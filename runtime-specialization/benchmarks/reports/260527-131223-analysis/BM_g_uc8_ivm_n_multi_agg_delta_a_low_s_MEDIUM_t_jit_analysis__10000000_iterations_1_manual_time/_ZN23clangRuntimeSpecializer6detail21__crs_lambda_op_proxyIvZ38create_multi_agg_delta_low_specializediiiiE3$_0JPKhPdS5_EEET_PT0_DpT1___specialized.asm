<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_multi_agg_delta_low_specializediiiiE3$_0JPKhPdS5_EEET_PT0_DpT1_>:
	leaq	-7(%rip), %rcx
	movabsq	$-8160, %rax
	addq	%rcx, %rax
	movl	4(%rdi), %ecx
	leal	1023(%rcx), %r8d
	testl	%ecx, %ecx
	cmovnsl	%ecx, %r8d
	andl	$4294966272, %r8d
	movl	%ecx, %r9d
	subl	%r8d, %r9d
	addl	$1024, %r9d
	negl	%r8d
	addl	%r8d, %ecx
	addl	$2047, %ecx
	testl	%r9d, %r9d
	cmovnsl	%r9d, %ecx
	andl	$4294966272, %ecx
	subl	%ecx, %r9d
	vmovsd	8(%rdi), %xmm0
	movslq	%r9d, %rcx
	vaddsd	(%rsi,%rcx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rcx,8)
	vmovsd	(%rdx,%rcx,8), %xmm0
	movabsq	$-448, %rsi
	vaddsd	(%rax,%rsi), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%rcx,8)
	retq
