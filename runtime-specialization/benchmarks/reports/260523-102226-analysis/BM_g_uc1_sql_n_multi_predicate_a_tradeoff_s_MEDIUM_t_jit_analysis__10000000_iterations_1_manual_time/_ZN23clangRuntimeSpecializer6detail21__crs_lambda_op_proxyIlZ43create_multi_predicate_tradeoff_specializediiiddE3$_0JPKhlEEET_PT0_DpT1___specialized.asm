<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_>:
	leaq	-7(%rip), %rax
	movabsq	$-8152, %rcx
	addq	%rax, %rcx
	testq	%rsi, %rsi
	jle	27
	pushq	%rbx
	movabsq	$-536, %rdx
	cmpq	$1, %rsi
	jne	13
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	jmp	460
	xorl	%eax, %eax
	retq
