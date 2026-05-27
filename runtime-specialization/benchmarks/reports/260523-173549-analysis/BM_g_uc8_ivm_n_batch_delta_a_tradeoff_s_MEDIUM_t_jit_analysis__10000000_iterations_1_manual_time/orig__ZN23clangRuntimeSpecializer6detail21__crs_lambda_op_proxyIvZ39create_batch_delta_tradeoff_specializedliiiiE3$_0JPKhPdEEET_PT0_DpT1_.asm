<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_batch_delta_tradeoff_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
mov (%rdi),%r8
test %r8,%r8
jle 4f92b5a <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_batch_delta_tradeoff_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_+0x4a>
mov %rdx,%rcx
movslq 0x14(%rdi),%r9
movslq 0xc(%rdi),%r10
mov 0x8(%rdi),%r11d
movslq 0x10(%rdi),%rdi
nopl 0x0(%rax,%rax,1)
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
jne 4f92b30 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_batch_delta_tradeoff_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_+0x20>
ret
int3
int3
int3
int3
int3