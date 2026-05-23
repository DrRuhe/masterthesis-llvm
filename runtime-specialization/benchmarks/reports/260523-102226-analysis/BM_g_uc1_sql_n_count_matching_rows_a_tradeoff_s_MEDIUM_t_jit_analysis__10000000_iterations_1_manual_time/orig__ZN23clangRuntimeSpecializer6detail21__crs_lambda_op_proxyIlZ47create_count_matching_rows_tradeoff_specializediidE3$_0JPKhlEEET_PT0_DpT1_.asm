<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f5d478 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0x28>
push %r15
push %r14
push %rbx
movsd 0x8(%rdi),%xmm0
movslq 0x4(%rdi),%rcx
movslq (%rdi),%r8
mov %edx,%edi
and $0x3,%edi
cmp $0x4,%rdx
jae 4f5d47b <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0x2b>
xor %eax,%eax
xor %r9d,%r9d
jmp 4f5d4f7 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0xa7>
xor %eax,%eax
ret
lea (%rsi,%r8,1),%r10
movabs $0x7ffffffffffffffc,%rax
and %rax,%rdx
lea (%rcx,%rcx,2),%r11
lea 0x0(,%rcx,4),%rbx
xor %eax,%eax
xor %r9d,%r9d
nopl (%rax)
movsd (%r10),%xmm1
xor %r14d,%r14d
ucomisd %xmm0,%xmm1
seta %r14b
add %rax,%r14
movsd (%r10,%rcx,1),%xmm1
xor %eax,%eax
ucomisd %xmm0,%xmm1
seta %al
movsd (%r10,%rcx,2),%xmm1
xor %r15d,%r15d
ucomisd %xmm0,%xmm1
seta %r15b
add %rax,%r15
add %r14,%r15
movsd (%r10,%r11,1),%xmm1
xor %eax,%eax
ucomisd %xmm0,%xmm1
seta %al
add %r15,%rax
add $0x4,%r9
add %rbx,%r10
cmp %r9,%rdx
jne 4f5d4a0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0x50>
test %rdi,%rdi
je 4f5d528 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0xd8>
imul %rcx,%r9
add %r8,%r9
add %r9,%rsi
cs nopw 0x0(%rax,%rax,1)
movsd (%rsi),%xmm1
xor %edx,%edx
ucomisd %xmm0,%xmm1
seta %dl
add %rdx,%rax
add %rcx,%rsi
dec %rdi
jne 4f5d510 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_tradeoff_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0xc0>
pop %rbx
pop %r14
pop %r15
ret
int3
int3