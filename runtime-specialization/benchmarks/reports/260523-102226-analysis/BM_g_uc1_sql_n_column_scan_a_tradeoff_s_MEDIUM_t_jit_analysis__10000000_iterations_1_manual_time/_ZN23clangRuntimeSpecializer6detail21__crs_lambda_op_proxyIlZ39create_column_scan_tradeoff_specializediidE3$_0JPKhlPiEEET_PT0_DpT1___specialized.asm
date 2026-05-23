<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_>:
	leaq	-7(%rip), %rax
	movabsq	$-8152, %rcx
	addq	%rax, %rcx
	testq	%rsi, %rsi
	jle	30
	pushq	%rbp
	pushq	%r14
	pushq	%rbx
	movabsq	$-536, %r8
	cmpq	$1, %rsi
	jne	13
	xorl	%r10d, %r10d
	xorl	%eax, %eax
	jmp	904
	xorl	%eax, %eax
	retq
