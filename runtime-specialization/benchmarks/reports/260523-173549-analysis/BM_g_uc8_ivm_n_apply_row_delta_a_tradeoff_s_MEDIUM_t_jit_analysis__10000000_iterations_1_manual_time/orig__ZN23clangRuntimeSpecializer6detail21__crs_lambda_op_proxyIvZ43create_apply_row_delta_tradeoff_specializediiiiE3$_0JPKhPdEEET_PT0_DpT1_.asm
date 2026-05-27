<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ43create_apply_row_delta_tradeoff_specializediiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
mov %rdx,%rcx
movslq 0x4(%rdi),%rax
mov (%rsi,%rax,1),%eax
mov (%rdi),%r8d
cltd
idiv %r8d
lea (%rdx,%r8,1),%eax
cltd
idiv %r8d
movslq 0x8(%rdi),%rax
movsd (%rsi,%rax,1),%xmm0
movslq %edx,%rax
addsd (%rcx,%rax,8),%xmm0
movsd %xmm0,(%rcx,%rax,8)
ret