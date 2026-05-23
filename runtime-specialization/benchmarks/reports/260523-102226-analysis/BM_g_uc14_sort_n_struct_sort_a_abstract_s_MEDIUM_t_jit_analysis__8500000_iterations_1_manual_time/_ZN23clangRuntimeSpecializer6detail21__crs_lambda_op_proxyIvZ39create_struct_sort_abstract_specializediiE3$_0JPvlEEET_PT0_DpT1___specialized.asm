<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_struct_sort_abstract_specializediiE3$_0JPvlEEET_PT0_DpT1_>:
	subq	$24, %rsp
	movq	%rsi, %rdx
	movq	%rdi, %rsi
	leaq	-7(%rip), %rax
	movabsq	$-10626, %rcx
	addq	%rax, %rcx
	movabsq	$0, %rax
	movq	(%rcx,%rax), %rax
	addq	$16, %rax
	movq	%rax, (%rsp)
	movl	$0, 8(%rsp)
	movl	$16, 16(%rsp)
	movabsq	$7624, %rax
	addq	%rcx, %rax
	movq	%rsp, %rdi
	callq	*%rax
	addq	$24, %rsp
	retq
