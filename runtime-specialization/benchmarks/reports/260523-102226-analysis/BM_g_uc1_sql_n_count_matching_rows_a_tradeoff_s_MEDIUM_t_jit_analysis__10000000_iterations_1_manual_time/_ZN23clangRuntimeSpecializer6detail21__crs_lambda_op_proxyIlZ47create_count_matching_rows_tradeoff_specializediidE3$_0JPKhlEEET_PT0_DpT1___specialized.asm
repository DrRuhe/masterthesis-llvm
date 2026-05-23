<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_>:
	leaq	-7(%rip), %rax
	movabsq	$-8152, %rdx
	addq	%rax, %rdx
	testq	%rsi, %rsi
	jle	37
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	movl	%esi, %ecx
	andl	$3, %ecx
	movabsq	$-536, %r8
	cmpq	$4, %rsi
	jae	12
	xorl	%eax, %eax
	xorl	%esi, %esi
	jmp	561
	xorl	%eax, %eax
	retq
