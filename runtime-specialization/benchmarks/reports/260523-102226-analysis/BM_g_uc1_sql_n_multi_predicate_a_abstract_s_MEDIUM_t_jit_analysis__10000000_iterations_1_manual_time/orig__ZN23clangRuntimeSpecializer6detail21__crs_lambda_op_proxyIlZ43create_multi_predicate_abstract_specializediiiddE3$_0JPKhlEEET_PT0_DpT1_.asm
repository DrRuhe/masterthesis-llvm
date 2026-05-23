<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_abstract_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_>:
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x40,%rsp
mov %rdx,%rbx
movsd 0x10(%rdi),%xmm0
mov 0x4(%rdi),%eax
mov 0x8(%rdi),%ecx
movsd 0x18(%rdi),%xmm1
lea 0x226b4c1(%rip),%rdx # 71ce698 <_ZTV12AndPredicate>
add $0x10,%rdx
mov %rdx,0x8(%rsp)
lea 0x2266df1(%rip),%rdx # 71c9fd8 <_ZTV18ThresholdPredicate>
add $0x10,%rdx
mov %rdx,0x10(%rsp)
mov %eax,0x18(%rsp)
movsd %xmm0,0x20(%rsp)
mov %rdx,0x28(%rsp)
mov %ecx,0x30(%rsp)
movsd %xmm1,0x38(%rsp)
test %rbx,%rbx
jle 4f6323d <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_abstract_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x8d>
mov %rsi,%r14
movslq (%rdi),%r13
xor %r15d,%r15d
lea 0x8(%rsp),%r12
nopl 0x0(%rax)
mov 0x8(%rsp),%rax
mov %r12,%rdi
mov %r14,%rsi
call *(%rax)
movzbl %al,%eax
add %rax,%r15
add %r13,%r14
dec %rbx
jne 4f63220 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_abstract_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x70>
jmp 4f63240 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ43create_multi_predicate_abstract_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_+0x90>
xor %r15d,%r15d
mov %r15,%rax
add $0x40,%rsp
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