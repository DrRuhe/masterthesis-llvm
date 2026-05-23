<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
movslq 0x8(%rdi),%rax
mov %rax,-0x28(%rsp)
test %rax,%rax
jle 4f6e19f <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x21f>
mov 0x4(%rdi),%eax
mov %rax,-0x30(%rsp)
test %eax,%eax
jle 4f6e19f <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x21f>
mov (%rdi),%edi
lea 0x1(,%rdi,2),%eax
imul %eax,%eax
cvtsi2ss %eax,%xmm1
movss -0x4e81fc(%rip),%xmm0 # 4a85dc8 <.LCPI0_0>
divss %xmm1,%xmm0
test %edi,%edi
js 4f6e101 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x181>
mov %edi,%eax
neg %eax
mov %eax,-0x1c(%rsp)
mov -0x28(%rsp),%rax
lea -0x1(%rax),%r9d
mov -0x30(%rsp),%rax
lea -0x1(%rax),%r10d
mov $0x2,%eax
sub %edi,%eax
mov %eax,-0x20(%rsp)
lea (%rdi,%rdi,1),%ecx
xor %r14d,%r14d
xor %r15d,%r15d
mov %rdx,-0x18(%rsp)
jmp 4f6e023 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0xa3>
data16 cs nopw 0x0(%rax,%rax,1)
inc %r15
cmp -0x28(%rsp),%r15
mov -0x18(%rsp),%rdx
je 4f6e19f <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x21f>
mov %r15,%rax
mov -0x30(%rsp),%r8
imul %r8,%rax
lea (%rdx,%rax,4),%rax
mov %rax,-0x10(%rsp)
mov %r8,%rdx
mov -0x20(%rsp),%ebp
xor %r13d,%r13d
jmp 4f6e06e <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0xee>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mulss %xmm0,%xmm1
mov -0x10(%rsp),%rax
mov -0x8(%rsp),%r13
movss %xmm1,(%rax,%r13,4)
inc %r13
inc %ebp
cmp %rdx,%r13
je 4f6e010 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x90>
mov %r13,-0x8(%rsp)
sub %edi,%r13d
cmp %r10d,%r13d
cmovge %r10d,%r13d
test %r13d,%r13d
cmovle %r14d,%r13d
xorps %xmm1,%xmm1
mov -0x1c(%rsp),%eax
jmp 4f6e097 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x117>
nopl (%rax)
cmp %edi,%eax
lea 0x1(%rax),%eax
je 4f6e050 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0xd0>
lea (%rax,%r15,1),%r8d
cmp %r9d,%r8d
cmovge %r9d,%r8d
test %r8d,%r8d
cmovle %r14d,%r8d
imul %edx,%r8d
lea (%r8,%r13,1),%ebx
addss (%rsi,%rbx,4),%xmm1
test %edi,%edi
je 4f6e090 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x110>
mov %ecx,%ebx
mov %ebp,%r12d
nop
lea -0x1(%r12),%r11d
cmp %r10d,%r11d
cmovge %r10d,%r11d
test %r11d,%r11d
cmovle %r14d,%r11d
add %r8d,%r11d
addss (%rsi,%r11,4),%xmm1
cmp %r10d,%r12d
mov %r10d,%r11d
cmovl %r12d,%r11d
test %r11d,%r11d
cmovle %r14d,%r11d
add %r8d,%r11d
addss (%rsi,%r11,4),%xmm1
add $0x2,%r12d
add $0xfffffffe,%ebx
jne 4f6e0c0 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x140>
jmp 4f6e090 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x110>
xorps %xmm1,%xmm1
mulss %xmm1,%xmm0
mov -0x30(%rsp),%rcx
mov %ecx,%eax
and $0x7ffffff8,%eax
movaps %xmm0,%xmm1
shufps $0x0,%xmm0,%xmm1
mov %ecx,%esi
shr $0x3,%esi
and $0xfffffff,%esi
shl $0x5,%rsi
lea 0x10(%rdx),%rdi
lea 0x0(,%rcx,4),%r8
xor %r9d,%r9d
jmp 4f6e150 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x1d0>
nopl 0x0(%rax,%rax,1)
inc %r9
add %r8,%rdi
add %r8,%rdx
cmp -0x28(%rsp),%r9
je 4f6e19f <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x21f>
cmpl $0x8,-0x30(%rsp)
jae 4f6e160 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x1e0>
xor %ecx,%ecx
mov -0x30(%rsp),%r10
jmp 4f6e190 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x210>
xor %ecx,%ecx
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movups %xmm1,-0x10(%rdi,%rcx,1)
movups %xmm1,(%rdi,%rcx,1)
add $0x20,%rcx
cmp %rcx,%rsi
jne 4f6e170 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x1f0>
mov %rax,%rcx
mov -0x30(%rsp),%r10
cmp %r10d,%eax
je 4f6e140 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x1c0>
nop
movss %xmm0,(%rdx,%rcx,4)
inc %rcx
cmp %rcx,%r10
jne 4f6e190 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x210>
jmp 4f6e140 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_box_filter_abstract_specializediiiE3$_0JPKfPfEEET_PT0_DpT1_+0x1c0>
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
int3
int3
int3
int3
int3
int3