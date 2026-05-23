<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ43create_multi_agg_delta_abstract_specializediiiiE3$_0JPKhPdS5_EEET_PT0_DpT1_>:
mov %rdx,%r8
movslq 0x4(%rdi),%r9
movslq 0x8(%rdi),%r10
mov (%rdi),%edi
mov (%rsi,%r9,1),%eax
cltd
idiv %edi
lea (%rdx,%rdi,1),%eax
cltd
idiv %edi
movsd (%rsi,%r10,1),%xmm0
movslq %edx,%rax
addsd (%r8,%rax,8),%xmm0
movsd %xmm0,(%r8,%rax,8)
mov (%rsi,%r9,1),%eax
cltd
idiv %edi
lea (%rdx,%rdi,1),%eax
cltd
idiv %edi
movslq %edx,%rax
movsd (%rcx,%rax,8),%xmm0
addsd -0x5095cc(%rip),%xmm0 # 4a86360 <.LCPI0_0>
movsd %xmm0,(%rcx,%rax,8)
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
int3
int3
int3
int3
int3