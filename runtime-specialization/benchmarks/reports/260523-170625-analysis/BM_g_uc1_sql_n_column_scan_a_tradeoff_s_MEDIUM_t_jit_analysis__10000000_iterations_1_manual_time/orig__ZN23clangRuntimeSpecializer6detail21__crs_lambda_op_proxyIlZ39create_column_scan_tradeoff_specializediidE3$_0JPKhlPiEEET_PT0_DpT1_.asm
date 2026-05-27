<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f6635f <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x1f>
movsd 0x8(%rdi),%xmm0
movslq 0x4(%rdi),%r8
cmp $0x1,%rdx
jne 4f66362 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x22>
xor %eax,%eax
xor %r9d,%r9d
test $0x1,%dl
jne 4f663d9 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x99>
ret
xor %eax,%eax
ret
push %rbp
push %r14
push %rbx
movabs $0x7ffffffffffffffe,%r10
and %rdx,%r10
lea (%r8,%r8,1),%r11
xor %eax,%eax
mov %rsi,%rbx
xor %r9d,%r9d
jmp 4f6639c <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x5c>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
add $0x2,%r9
add %r11,%rbx
cmp %r9,%r10
je 4f663d0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x90>
movslq (%rdi),%r14
movsd (%rbx,%r14,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f663b2 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x72>
mov %r9d,(%rcx,%rax,4)
inc %rax
movslq (%rdi),%r14
add %r8,%r14
movsd (%rbx,%r14,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f66390 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x50>
lea 0x1(%r9),%ebp
mov %ebp,(%rcx,%rax,4)
inc %rax
jmp 4f66390 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x50>
pop %rbx
pop %r14
pop %rbp
test $0x1,%dl
je 4f6635e <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x1e>
imul %r9,%r8
add %r8,%rsi
movslq (%rdi),%rdx
movsd (%rdx,%rsi,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f6635e <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_tradeoff_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x1e>
mov %r9d,(%rcx,%rax,4)
inc %rax
ret
int3
int3
int3
int3
int3
int3