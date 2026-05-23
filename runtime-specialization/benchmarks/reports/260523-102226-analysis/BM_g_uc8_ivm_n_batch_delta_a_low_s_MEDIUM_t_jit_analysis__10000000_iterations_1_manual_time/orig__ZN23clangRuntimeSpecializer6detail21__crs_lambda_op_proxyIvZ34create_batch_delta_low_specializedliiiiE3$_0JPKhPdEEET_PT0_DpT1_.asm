<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ34create_batch_delta_low_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
mov (%rdi),%r8
test %r8,%r8
jle 4f924fa <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ34create_batch_delta_low_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_+0x4a>
mov %rdx,%rcx
mov 0x8(%rdi),%r9d
movslq 0x14(%rdi),%r10
movslq 0x10(%rdi),%r11
movslq 0xc(%rdi),%rdi
nopl 0x0(%rax,%rax,1)
mov (%rsi,%rdi,1),%eax
cltd
idiv %r9d
lea (%rdx,%r9,1),%eax
cltd
idiv %r9d
movsd (%rsi,%r11,1),%xmm0
movslq %edx,%rax
addsd (%rcx,%rax,8),%xmm0
movsd %xmm0,(%rcx,%rax,8)
add %r10,%rsi
dec %r8
jne 4f924d0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ34create_batch_delta_low_specializedliiiiE3$_0JPKhPdEEET_PT0_DpT1_+0x20>
ret
int3
int3
int3
int3
int3