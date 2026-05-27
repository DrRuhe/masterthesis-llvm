<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_abstract_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f98bfb <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_abstract_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_+0x4b>
mov %rdx,%r8
mov 0xc(%rdi),%r9d
movslq 0x8(%rdi),%r10
movslq 0x4(%rdi),%r11
movslq (%rdi),%rdi
nopw 0x0(%rax,%rax,1)
mov (%rsi,%r11,1),%eax
cltd
idiv %r9d
lea (%rdx,%r9,1),%eax
cltd
idiv %r9d
movsd (%rsi,%r10,1),%xmm0
movslq %edx,%rax
addsd (%rcx,%rax,8),%xmm0
movsd %xmm0,(%rcx,%rax,8)
add %rdi,%rsi
dec %r8
jne 4f98bd0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_abstract_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_+0x20>
ret
int3
int3
int3
int3