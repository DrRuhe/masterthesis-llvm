<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f61a3e <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2e>
movslq 0x18(%rdi),%r9
movslq (%rdi),%r8
movsd 0x8(%rdi),%xmm1
movsd 0x10(%rdi),%xmm0
movslq 0x4(%rdi),%rcx
cmp $0x1,%rdx
jne 4f61a41 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x31>
xor %eax,%eax
xor %edi,%edi
test $0x1,%dl
jne 4f61ac2 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0xb2>
ret
xor %eax,%eax
ret
push %r15
push %r14
push %rbx
movabs $0x7ffffffffffffffe,%r10
and %rdx,%r10
lea (%r9,%rcx,1),%r11
lea (%r9,%r9,1),%rbx
lea (%r9,%r8,1),%r14
xor %eax,%eax
mov %rsi,%r15
xor %edi,%edi
jmp 4f61a7c <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x6c>
nopl 0x0(%rax,%rax,1)
add $0x2,%rdi
add %rbx,%r15
cmp %rdi,%r10
je 4f61ab4 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0xa4>
movsd (%r15,%r8,1),%xmm2
ucomisd %xmm1,%xmm2
jbe 4f61a97 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x87>
movsd (%r15,%rcx,1),%xmm2
ucomisd %xmm0,%xmm2
jbe 4f61a97 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x87>
inc %rax
movsd (%r15,%r14,1),%xmm2
ucomisd %xmm1,%xmm2
jbe 4f61a70 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x60>
movsd (%r15,%r11,1),%xmm2
ucomisd %xmm0,%xmm2
jbe 4f61a70 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x60>
inc %rax
jmp 4f61a70 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x60>
pop %rbx
pop %r14
pop %r15
test $0x1,%dl
je 4f61a3d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2d>
imul %r9,%rdi
add %rdi,%rsi
movsd (%r8,%rsi,1),%xmm2
ucomisd %xmm1,%xmm2
jbe 4f61a3d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2d>
movsd (%rsi,%rcx,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f61a3d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_tradeoff_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2d>
inc %rax
ret
int3
int3
int3
int3