<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_>:
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x20,%rsp
lea 0x226b41c(%rip),%rax # 71d3080 <_ZTV15BufferCollector>
add $0x10,%rax
mov %rax,0x8(%rsp)
mov %rcx,0x10(%rsp)
movq $0x0,0x18(%rsp)
test %rdx,%rdx
jle 4f67ce1 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x91>
mov %rdx,%rbx
mov %rsi,%r14
movsd 0x8(%rdi),%xmm1
movslq 0x4(%rdi),%rax
movslq (%rdi),%r13
add %rax,%r14
xor %r15d,%r15d
lea 0x8(%rsp),%r12
movsd %xmm1,(%rsp)
jmp 4f67cbb <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x6b>
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r15
add %r13,%r14
cmp %r15,%rbx
je 4f67cda <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x8a>
movsd (%r14),%xmm0
ucomisd %xmm1,%xmm0
jbe 4f67cb0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x60>
mov 0x8(%rsp),%rax
mov %r12,%rdi
mov %r15,%rsi
call *(%rax)
movsd (%rsp),%xmm1
jmp 4f67cb0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x60>
mov 0x18(%rsp),%rax
jmp 4f67ce3 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_+0x93>
xor %eax,%eax
add $0x20,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
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
int3