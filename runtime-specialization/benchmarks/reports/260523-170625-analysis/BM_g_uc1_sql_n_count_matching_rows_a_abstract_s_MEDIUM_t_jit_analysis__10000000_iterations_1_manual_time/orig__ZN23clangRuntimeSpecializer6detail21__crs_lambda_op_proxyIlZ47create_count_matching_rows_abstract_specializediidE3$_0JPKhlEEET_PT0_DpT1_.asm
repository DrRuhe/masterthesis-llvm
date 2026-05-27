<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_abstract_specializediidE3$_0JPKhlEEET_PT0_DpT1_>:
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x20,%rsp
movsd 0x8(%rdi),%xmm0
lea 0x226b41f(%rip),%rax # 71c9fd8 <_ZTV18ThresholdPredicate>
add $0x10,%rax
mov %rax,0x8(%rsp)
movslq 0x4(%rdi),%r13
mov (%rdi),%rax
mov %rax,0x10(%rsp)
movsd %xmm0,0x18(%rsp)
test %rdx,%rdx
jle 4f5ec0d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_abstract_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0x6d>
mov %rdx,%rbx
mov %rsi,%r14
xor %r15d,%r15d
lea 0x8(%rsp),%r12
nopw 0x0(%rax,%rax,1)
mov 0x8(%rsp),%rax
mov %r12,%rdi
mov %r14,%rsi
call *(%rax)
movzbl %al,%eax
add %rax,%r15
add %r13,%r14
dec %rbx
jne 4f5ebf0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_abstract_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0x50>
jmp 4f5ec10 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_abstract_specializediidE3$_0JPKhlEEET_PT0_DpT1_+0x70>
xor %r15d,%r15d
mov %r15,%rax
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