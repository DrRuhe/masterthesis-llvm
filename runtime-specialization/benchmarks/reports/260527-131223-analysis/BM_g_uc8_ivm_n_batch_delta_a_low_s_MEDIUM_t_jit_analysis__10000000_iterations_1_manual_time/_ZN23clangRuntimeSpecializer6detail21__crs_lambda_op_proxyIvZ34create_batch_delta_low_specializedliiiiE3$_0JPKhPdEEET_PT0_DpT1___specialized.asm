<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ34create_batch_delta_low_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
	xorl	%eax, %eax
	testb	%al, %al
	jne	100
	movl	$8, %eax
	nopl	(%rax,%rax)
	movl	-4(%rdi,%rax), %ecx
	leal	1023(%rcx), %edx
	testl	%ecx, %ecx
	cmovnsl	%ecx, %edx
	andl	$4294966272, %edx
	movl	%ecx, %r8d
	subl	%edx, %r8d
	addl	$1024, %r8d
	negl	%edx
	addl	%edx, %ecx
	addl	$2047, %ecx
	testl	%r8d, %r8d
	cmovnsl	%r8d, %ecx
	andl	$4294966272, %ecx
	subl	%ecx, %r8d
	vmovsd	(%rdi,%rax), %xmm0
	movslq	%r8d, %rcx
	vaddsd	(%rsi,%rcx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rcx,8)
	addq	$24, %rax
	cmpq	$1200000008, %rax
	jne	-90
	retq
