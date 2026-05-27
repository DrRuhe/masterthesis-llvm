<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_multi_pattern_match_tradeoff_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
xor %ecx,%ecx
test %rdx,%rdx
jle 4f85fe1 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_multi_pattern_match_tradeoff_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x81>
mov (%rdi),%rdi
xor %r9d,%r9d
xor %eax,%eax
xor %r8d,%r8d
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movzbl (%rsi,%r8,1),%r10d
test %r10b,%r10b
cmovle %ecx,%r10d
movzbl %r10b,%r10d
mov %r9d,%r11d
shl $0x7,%r11d
or %r10d,%r11d
movslq %r11d,%r10
add $0xfffffffe,%r9d
test $0xfffffffd,%r9d
mov (%rdi,%r10,4),%r9d
setne %r10b
lea -0x2(%r9),%r11d
test $0xfffffffd,%r11d
sete %r11b
and %r10b,%r11b
movzbl %r11b,%r10d
add %r10,%rax
inc %r8
cmp %r8,%rdx
jne 4f85f80 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_multi_pattern_match_tradeoff_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x20>
cmp $0x4,%r9d
je 4f85fdd <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_multi_pattern_match_tradeoff_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x7d>
cmp $0x2,%r9d
jne 4f85fe0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_multi_pattern_match_tradeoff_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x80>
inc %rax
ret
xor %eax,%eax
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