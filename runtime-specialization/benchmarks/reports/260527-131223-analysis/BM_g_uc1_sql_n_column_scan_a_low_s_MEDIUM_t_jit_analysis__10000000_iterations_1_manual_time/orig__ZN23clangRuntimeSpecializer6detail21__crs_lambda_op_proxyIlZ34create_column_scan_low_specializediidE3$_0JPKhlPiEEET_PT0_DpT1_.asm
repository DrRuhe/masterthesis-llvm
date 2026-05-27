<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_>:
test %rdx,%rdx
jle 4f78179 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x29>
movsd 0x8(%rdi),%xmm0
movslq 0x4(%rdi),%rax
movslq (%rdi),%rdi
add %rax,%rsi
cmp $0x1,%rdx
jne 4f7817c <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x2c>
xor %eax,%eax
xor %r8d,%r8d
test $0x1,%dl
jne 4f781f9 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0xa9>
ret
xor %eax,%eax
ret
push %rbx
movabs $0x7ffffffffffffffe,%r9
and %rdx,%r9
lea (%rdi,%rdi,1),%r10
xor %eax,%eax
mov %rsi,%r11
xor %r8d,%r8d
jmp 4f781ac <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x5c>
nopl 0x0(%rax,%rax,1)
add $0x2,%r8
add %r10,%r11
cmp %r8,%r9
je 4f781ef <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x9f>
movsd (%r11),%xmm1
ucomisd %xmm0,%xmm1
ja 4f781d0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x80>
movsd (%r11,%rdi,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f781a0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x50>
jmp 4f781e3 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x93>
data16 cs nopw 0x0(%rax,%rax,1)
mov %r8d,(%rcx,%rax,4)
inc %rax
movsd (%r11,%rdi,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f781a0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x50>
lea 0x1(%r8),%ebx
mov %ebx,(%rcx,%rax,4)
inc %rax
jmp 4f781a0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x50>
pop %rbx
test $0x1,%dl
je 4f78178 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x28>
imul %r8,%rdi
movsd (%rsi,%rdi,1),%xmm1
ucomisd %xmm0,%xmm1
jbe 4f78178 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x28>
mov %r8d,(%rcx,%rax,4)
inc %rax
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