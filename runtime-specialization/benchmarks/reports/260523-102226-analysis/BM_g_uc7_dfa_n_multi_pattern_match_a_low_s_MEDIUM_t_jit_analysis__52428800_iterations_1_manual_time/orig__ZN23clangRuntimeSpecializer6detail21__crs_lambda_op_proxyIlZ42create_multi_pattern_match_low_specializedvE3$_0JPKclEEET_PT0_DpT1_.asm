<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
xor %ecx,%ecx
test %rdx,%rdx
jle 4f84a89 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x79>
mov (%rdi),%rdi
xor %r10d,%r10d
xor %eax,%eax
xor %r8d,%r8d
jmp 4f84a3b <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x2b>
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r8
mov %r9d,%r10d
cmp %r8,%rdx
je 4f84a79 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x69>
movzbl (%rsi,%r8,1),%r9d
test %r9b,%r9b
cmovle %ecx,%r9d
movzbl %r9b,%r9d
mov %r10d,%r11d
shl $0x7,%r11d
or %r9d,%r11d
movslq %r11d,%r9
mov (%rdi,%r9,4),%r9d
cmp $0x4,%r9d
je 4f84a68 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x58>
cmp $0x2,%r9d
jne 4f84a30 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x20>
cmp $0x2,%r10d
je 4f84a30 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x20>
cmp $0x4,%r10d
je 4f84a30 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x20>
inc %rax
jmp 4f84a30 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x20>
cmp $0x4,%r9d
je 4f84a85 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x75>
cmp $0x2,%r9d
jne 4f84a88 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x78>
inc %rax
ret
xor %eax,%eax
ret
int3
int3
int3
int3