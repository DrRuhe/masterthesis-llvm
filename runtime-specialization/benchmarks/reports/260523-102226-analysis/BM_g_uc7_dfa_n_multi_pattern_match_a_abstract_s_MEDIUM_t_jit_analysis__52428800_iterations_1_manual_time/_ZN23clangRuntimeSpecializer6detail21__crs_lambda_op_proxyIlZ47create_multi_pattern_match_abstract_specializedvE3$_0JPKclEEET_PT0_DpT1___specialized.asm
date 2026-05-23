<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_multi_pattern_match_abstract_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
	xorl	%r10d, %r10d
	testq	%rsi, %rsi
	jle	32
	pushq	%rbp
	pushq	%rbx
	movabsq	$94266014162784, %rcx
	movl	%esi, %edx
	andl	$7, %edx
	cmpq	$8, %rsi
	jae	12
	xorl	%eax, %eax
	xorl	%esi, %esi
	jmp	516
	xorl	%eax, %eax
	retq
