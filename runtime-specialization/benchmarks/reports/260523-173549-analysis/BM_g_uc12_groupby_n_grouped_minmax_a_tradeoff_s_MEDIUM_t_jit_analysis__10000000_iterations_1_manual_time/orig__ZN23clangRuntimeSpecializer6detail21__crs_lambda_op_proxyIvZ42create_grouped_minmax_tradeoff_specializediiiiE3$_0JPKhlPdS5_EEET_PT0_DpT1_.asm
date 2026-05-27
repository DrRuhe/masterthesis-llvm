<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4fa0876 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x76>
push %rbx
mov %rdx,%r9
movslq (%rdi),%r10
movslq 0x4(%rdi),%r11
mov 0xc(%rdi),%ebx
movslq 0x8(%rdi),%rdi
jmp 4fa0828 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x28>
nopl 0x0(%rax)
add %r10,%rsi
dec %r9
je 4fa0875 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x75>
mov (%rsi,%r11,1),%eax
cltd
idiv %ebx
lea (%rdx,%rbx,1),%eax
cltd
idiv %ebx
movsd (%rsi,%rdi,1),%xmm0
movslq %edx,%rax
movsd (%rcx,%rax,8),%xmm1
ucomisd %xmm0,%xmm1
ja 4fa0860 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x60>
ucomisd (%r8,%rax,8),%xmm0
jbe 4fa0820 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x20>
jmp 4fa086d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x6d>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd %xmm0,(%rcx,%rax,8)
ucomisd (%r8,%rax,8),%xmm0
jbe 4fa0820 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x20>
movsd %xmm0,(%r8,%rax,8)
jmp 4fa0820 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_grouped_minmax_tradeoff_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x20>
pop %rbx
ret
int3
int3
int3
int3
int3
int3
int3
int3
int3