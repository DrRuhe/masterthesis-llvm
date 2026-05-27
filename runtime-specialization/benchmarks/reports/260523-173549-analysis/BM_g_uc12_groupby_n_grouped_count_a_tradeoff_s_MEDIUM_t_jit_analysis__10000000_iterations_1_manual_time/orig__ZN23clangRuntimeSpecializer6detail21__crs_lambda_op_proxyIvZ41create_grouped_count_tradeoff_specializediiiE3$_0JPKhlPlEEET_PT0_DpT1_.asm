<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ41create_grouped_count_tradeoff_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f9bb3a <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ41create_grouped_count_tradeoff_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_+0x3a>
mov %rdx,%r8
movslq (%rdi),%r9
movslq 0x4(%rdi),%rax
add %rax,%rsi
mov 0x8(%rdi),%edi
data16 cs nopw 0x0(%rax,%rax,1)
mov (%rsi),%eax
cltd
idiv %edi
lea (%rdx,%rdi,1),%eax
cltd
idiv %edi
movslq %edx,%rax
incq (%rcx,%rax,8)
add %r9,%rsi
dec %r8
jne 4f9bb20 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ41create_grouped_count_tradeoff_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_+0x20>
ret
int3
int3
int3
int3
int3