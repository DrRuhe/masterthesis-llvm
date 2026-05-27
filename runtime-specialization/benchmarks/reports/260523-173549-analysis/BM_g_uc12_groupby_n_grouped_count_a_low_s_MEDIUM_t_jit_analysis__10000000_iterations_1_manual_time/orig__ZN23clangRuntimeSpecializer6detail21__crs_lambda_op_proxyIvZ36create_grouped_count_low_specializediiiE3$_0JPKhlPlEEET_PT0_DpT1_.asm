<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ36create_grouped_count_low_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f9b67d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ36create_grouped_count_low_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_+0x3d>
mov %rdx,%r8
mov 0x8(%rdi),%r9d
movslq 0x4(%rdi),%rax
movslq (%rdi),%rdi
add %rax,%rsi
cs nopw 0x0(%rax,%rax,1)
mov (%rsi),%eax
cltd
idiv %r9d
lea (%rdx,%r9,1),%eax
cltd
idiv %r9d
movslq %edx,%rax
incq (%rcx,%rax,8)
add %rdi,%rsi
dec %r8
jne 4f9b660 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ36create_grouped_count_low_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_+0x20>
ret
int3
int3