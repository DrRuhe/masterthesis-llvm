<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_batch_delta_tradeoff_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
	movl	$224, %eax
	movl	$1023, %ecx
	nopw	(%rax,%rax)
	movl	-220(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-216(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-196(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-192(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-172(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-168(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-148(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-144(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-124(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-120(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-100(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-96(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-76(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-72(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-52(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-48(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-28(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	-24(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	movl	-4(%rdi,%rax), %edx
	andl	%ecx, %edx
	vmovsd	(%rdi,%rax), %xmm0
	vaddsd	(%rsi,%rdx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	addq	$240, %rax
	cmpq	$1200000224, %rax
	jne	-261
	retq
