<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_email_match_abstract_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
xor %eax,%eax
test %rdx,%rdx
jle 4f7e0b5 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_email_match_abstract_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x75>
mov (%rdi),%rdi
xor %r9d,%r9d
xor %ecx,%ecx
xor %r8d,%r8d
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movzbl (%rsi,%r8,1),%r10d
test %r10b,%r10b
cmovle %eax,%r10d
mov %r9d,%r11d
movzbl %r10b,%r9d
mov %r11d,%r10d
shl $0x7,%r10d
or %r9d,%r10d
movslq %r10d,%r9
mov (%rdi,%r9,4),%r9d
cmp $0x6,%r9d
sete %r10b
cmp $0x6,%r11d
setne %r11b
and %r10b,%r11b
movzbl %r11b,%r10d
add %r10,%rcx
inc %r8
cmp %r8,%rdx
jne 4f7e060 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_email_match_abstract_specializedvE3$_0JPKclEEET_PT0_DpT1_+0x20>
xor %eax,%eax
cmp $0x6,%r9d
sete %al
add %rax,%rcx
mov %rcx,%rax
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