<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ43create_apply_row_delta_abstract_specializediiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
mov %rdx,%rcx
movslq (%rdi),%rax
movslq 0x4(%rdi),%r8
mov 0x8(%rdi),%edi
mov (%rsi,%rax,1),%eax
cltd
idiv %edi
lea (%rdx,%rdi,1),%eax
cltd
idiv %edi
movsd (%rsi,%r8,1),%xmm0
movslq %edx,%rax
addsd (%rcx,%rax,8),%xmm0
movsd %xmm0,(%rcx,%rax,8)
ret
int3
int3
int3