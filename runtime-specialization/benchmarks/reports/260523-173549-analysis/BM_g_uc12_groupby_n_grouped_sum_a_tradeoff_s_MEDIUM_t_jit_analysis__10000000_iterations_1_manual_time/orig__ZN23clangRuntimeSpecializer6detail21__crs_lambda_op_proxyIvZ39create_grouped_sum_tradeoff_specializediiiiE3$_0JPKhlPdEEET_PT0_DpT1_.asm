<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_tradeoff_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f9743a <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_tradeoff_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_+0x4a>
mov %rdx,%r8
movslq (%rdi),%r9
movslq 0x4(%rdi),%r10
mov 0xc(%rdi),%r11d
movslq 0x8(%rdi),%rdi
nopw 0x0(%rax,%rax,1)
mov (%rsi,%r10,1),%eax
cltd
idiv %r11d
lea (%rdx,%r11,1),%eax
cltd
idiv %r11d
movsd (%rsi,%rdi,1),%xmm0
movslq %edx,%rax
addsd (%rcx,%rax,8),%xmm0
movsd %xmm0,(%rcx,%rax,8)
add %r9,%rsi
dec %r8
jne 4f97410 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_tradeoff_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_+0x20>
ret
int3
int3
int3
int3
int3