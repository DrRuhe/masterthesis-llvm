<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4fa0056 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x76>
push %rbx
mov %rdx,%r9
mov 0xc(%rdi),%r10d
movslq 0x8(%rdi),%r11
movslq 0x4(%rdi),%rbx
movslq (%rdi),%rdi
jmp 4fa0008 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x28>
nopw 0x0(%rax,%rax,1)
add %rdi,%rsi
dec %r9
je 4fa0055 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x75>
mov (%rsi,%rbx,1),%eax
cltd
idiv %r10d
lea (%rdx,%r10,1),%eax
cltd
idiv %r10d
movsd (%rsi,%r11,1),%xmm0
movslq %edx,%rax
movsd (%rcx,%rax,8),%xmm1
ucomisd %xmm0,%xmm1
ja 4fa0040 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x60>
ucomisd (%r8,%rax,8),%xmm0
jbe 4fa0000 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x20>
jmp 4fa004d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x6d>
data16 cs nopw 0x0(%rax,%rax,1)
movsd %xmm0,(%rcx,%rax,8)
ucomisd (%r8,%rax,8),%xmm0
jbe 4fa0000 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x20>
movsd %xmm0,(%r8,%rax,8)
jmp 4fa0000 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_+0x20>
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