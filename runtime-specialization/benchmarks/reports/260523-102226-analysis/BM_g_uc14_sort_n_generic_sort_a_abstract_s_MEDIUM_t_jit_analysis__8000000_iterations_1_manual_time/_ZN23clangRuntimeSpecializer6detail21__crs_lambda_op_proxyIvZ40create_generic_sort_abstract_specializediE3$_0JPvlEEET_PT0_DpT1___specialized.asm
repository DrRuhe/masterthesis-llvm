<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ40create_generic_sort_abstract_specializediE3$_0JPvlEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$1112, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-8097, %rcx
	addq	%rax, %rcx
	movabsq	$0, %rax
	movq	(%rcx,%rax), %rax
	addq	$16, %rax
	movq	%rax, 16(%rsp)
	cmpq	$2, %rsi
	jge	18
	addq	$1112, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
