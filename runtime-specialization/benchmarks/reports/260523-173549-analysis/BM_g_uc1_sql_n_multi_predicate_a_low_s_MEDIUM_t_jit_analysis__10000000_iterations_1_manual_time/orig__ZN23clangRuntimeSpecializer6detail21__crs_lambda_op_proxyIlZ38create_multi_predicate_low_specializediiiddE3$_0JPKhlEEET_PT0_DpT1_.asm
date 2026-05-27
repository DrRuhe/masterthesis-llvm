<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f602ef <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2f>
movsd 0x10(%rdi),%xmm1
movsd 0x18(%rdi),%xmm0
movslq 0x8(%rdi),%rcx
movslq 0x4(%rdi),%r8
movslq (%rdi),%rdi
cmp $0x1,%rdx
jne 4f602f2 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x32>
xor %eax,%eax
xor %r9d,%r9d
test $0x1,%dl
jne 4f60372 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0xb2>
ret
xor %eax,%eax
ret
push %r15
push %r14
push %rbx
movabs $0x7ffffffffffffffe,%r10
and %rdx,%r10
lea (%rdi,%rcx,1),%r11
lea (%rdi,%rdi,1),%rbx
lea (%rdi,%r8,1),%r14
xor %eax,%eax
mov %rsi,%r15
xor %r9d,%r9d
jmp 4f6032c <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x6c>
nopw 0x0(%rax,%rax,1)
add $0x2,%r9
add %rbx,%r15
cmp %r9,%r10
je 4f60364 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0xa4>
movsd (%r15,%r8,1),%xmm2
ucomisd %xmm1,%xmm2
jbe 4f60347 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x87>
movsd (%r15,%rcx,1),%xmm2
ucomisd %xmm0,%xmm2
jbe 4f60347 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x87>
inc %rax
movsd (%r15,%r14,1),%xmm2
ucomisd %xmm1,%xmm2
jbe 4f60320 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x60>
movsd (%r15,%r11,1),%xmm2
ucomisd %xmm0,%xmm2
jbe 4f60320 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x60>
inc %rax
jmp 4f60320 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x60>
pop %rbx
pop %r14
pop %r15
test $0x1,%dl
je 4f602ee <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2e>
imul %rdi,%r9
add %r9,%rsi
movsd (%r8,%rsi,1),%xmm2
ucomisd %xmm1,%xmm2
jbe 4f602ee <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2e>
movsd (%rsi,%rcx,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f602ee <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x2e>
inc %rax
ret
int3
int3
int3
int3