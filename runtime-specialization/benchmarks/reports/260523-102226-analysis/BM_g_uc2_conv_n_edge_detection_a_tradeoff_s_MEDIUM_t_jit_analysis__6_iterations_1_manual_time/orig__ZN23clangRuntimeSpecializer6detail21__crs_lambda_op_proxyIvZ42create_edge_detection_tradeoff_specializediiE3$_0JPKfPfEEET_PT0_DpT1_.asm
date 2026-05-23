<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_edge_detection_tradeoff_specializediiE3$_0JPKfPfEEET_PT0_DpT1_>:
mov 0x4(%rdi),%ecx
mov 0x8(%rdi),%r8d
jmp 4f6fda0 <_ZNK11SobelFilter6detectEPKfPfii>
int3
int3
int3
int3
<_ZNK11SobelFilter6detectEPKfPfii>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
mov %rcx,-0x38(%rsp)
mov %rdx,-0x28(%rsp)
test %r8d,%r8d
jle 4f700dc <_ZNK11SobelFilter6detectEPKfPfii+0x33c>
cmpl $0x0,-0x38(%rsp)
jle 4f700dc <_ZNK11SobelFilter6detectEPKfPfii+0x33c>
mov %r8d,%ecx
mov -0x38(%rsp),%rdx
lea -0x1(%rdx),%eax
mov %edx,%edx
mov %r8d,%edi
mov %rdi,-0x18(%rsp)
dec %ecx
mov %ecx,-0x2c(%rsp)
lea 0x0(,%rdx,4),%rcx
mov %rcx,-0x20(%rsp)
mov %rdx,-0x10(%rsp)
lea -0x1(%rdx),%r11
xor %ebx,%ebx
xorps %xmm0,%xmm0
mov -0x28(%rsp),%r14
xor %r8d,%r8d
jmp 4f6ff54 <_ZNK11SobelFilter6detectEPKfPfii+0x1b4>
nopl 0x0(%rax,%rax,1)
xor %edi,%edi
imul -0x10(%rsp),%r8
lea -0x1(%rdi),%edx
cmp %eax,%edx
cmovge %eax,%edx
test %edx,%edx
cmovle %ebx,%edx
cmp %eax,%edi
mov %eax,%r12d
cmovl %edi,%r12d
test %r12d,%r12d
cmovle %ebx,%r12d
lea (%r12,%r10,1),%ecx
lea 0x1(%rdi),%r13d
cmp %eax,%r13d
cmovge %eax,%r13d
movss (%rsi,%rcx,4),%xmm5
test %r13d,%r13d
cmovle %ebx,%r13d
lea (%rdx,%r10,1),%ecx
add %r13d,%r10d
movss (%rsi,%r10,4),%xmm7
lea (%rdx,%r9,1),%r10d
movss (%rsi,%r10,4),%xmm6
lea (%r12,%r9,1),%r10d
add %r13d,%r9d
movss (%rsi,%r9,4),%xmm2
add %ebp,%edx
movss (%rsi,%rdx,4),%xmm4
add %ebp,%r12d
movss (%rsi,%r12,4),%xmm1
add %ebp,%r13d
xorps %xmm8,%xmm8
subss (%rsi,%rcx,4),%xmm8
movss (%rsi,%r13,4),%xmm3
movss (%rsi,%r10,4),%xmm9
movaps %xmm5,%xmm10
mulss %xmm0,%xmm10
addss %xmm8,%xmm10
addss %xmm5,%xmm5
subss %xmm5,%xmm8
addss %xmm7,%xmm10
subss %xmm7,%xmm8
movaps %xmm6,%xmm5
addss %xmm6,%xmm5
subss %xmm5,%xmm10
mulss %xmm0,%xmm6
addss %xmm8,%xmm6
mulss %xmm0,%xmm9
addss %xmm9,%xmm10
addss %xmm6,%xmm9
movaps %xmm2,%xmm5
addss %xmm2,%xmm5
addss %xmm10,%xmm5
mulss %xmm0,%xmm2
addss %xmm9,%xmm2
subss %xmm4,%xmm5
addss %xmm4,%xmm2
movaps %xmm1,%xmm4
mulss %xmm0,%xmm4
addss %xmm5,%xmm4
addss %xmm1,%xmm1
addss %xmm2,%xmm1
addss %xmm3,%xmm4
addss %xmm3,%xmm1
mulss %xmm1,%xmm1
mulss %xmm4,%xmm4
addss %xmm1,%xmm4
xorps %xmm1,%xmm1
sqrtss %xmm4,%xmm1
mov -0x28(%rsp),%rcx
lea (%rcx,%rdi,4),%rcx
movss %xmm1,(%rcx,%r8,4)
add -0x20(%rsp),%r14
mov -0x8(%rsp),%rcx
mov %rcx,%r8
cmp -0x18(%rsp),%rcx
je 4f700dc <_ZNK11SobelFilter6detectEPKfPfii+0x33c>
lea -0x1(%r8),%r10d
mov -0x2c(%rsp),%ebp
cmp %ebp,%r10d
cmovge %ebp,%r10d
test %r10d,%r10d
cmovle %ebx,%r10d
mov -0x38(%rsp),%rcx
imul %ecx,%r10d
cmp %ebp,%r8d
mov %ebp,%r9d
cmovl %r8d,%r9d
test %r9d,%r9d
cmovle %ebx,%r9d
imul %ecx,%r9d
lea 0x1(%r8),%rdx
cmp %ebp,%edx
mov %rdx,-0x8(%rsp)
cmovl %edx,%ebp
test %ebp,%ebp
cmovle %ebx,%ebp
imul %ecx,%ebp
test %eax,%eax
je 4f6fe10 <_ZNK11SobelFilter6detectEPKfPfii+0x70>
xor %r13d,%r13d
nopl 0x0(%rax)
lea -0x1(%r13),%edx
cmp %eax,%edx
cmovge %eax,%edx
test %edx,%edx
cmovle %ebx,%edx
cmp %eax,%r13d
mov %eax,%r12d
cmovl %r13d,%r12d
test %r12d,%r12d
cmovle %ebx,%r12d
lea (%r12,%r10,1),%ecx
movss (%rsi,%rcx,4),%xmm3
lea 0x1(%r13),%rdi
cmp %eax,%edi
mov %eax,%ecx
cmovl %edi,%ecx
test %ecx,%ecx
cmovle %ebx,%ecx
lea (%rcx,%r10,1),%r15d
movss (%rsi,%r15,4),%xmm7
lea (%rdx,%r9,1),%r15d
movss (%rsi,%r15,4),%xmm5
lea (%rcx,%r9,1),%r15d
movss (%rsi,%r15,4),%xmm1
lea (%rdx,%r10,1),%r15d
add %ebp,%edx
movss (%rsi,%rdx,4),%xmm6
lea (%r12,%r9,1),%edx
add %ebp,%r12d
movss (%rsi,%r12,4),%xmm2
add %ebp,%ecx
movss (%rsi,%rcx,4),%xmm4
xorps %xmm8,%xmm8
subss (%rsi,%r15,4),%xmm8
movss (%rsi,%rdx,4),%xmm9
movaps %xmm3,%xmm10
mulss %xmm0,%xmm10
addss %xmm8,%xmm10
addss %xmm3,%xmm3
subss %xmm3,%xmm8
addss %xmm7,%xmm10
subss %xmm7,%xmm8
movaps %xmm5,%xmm3
addss %xmm5,%xmm3
subss %xmm3,%xmm10
mulss %xmm0,%xmm5
addss %xmm8,%xmm5
mulss %xmm0,%xmm9
addss %xmm9,%xmm10
addss %xmm5,%xmm9
movaps %xmm1,%xmm3
addss %xmm1,%xmm3
addss %xmm10,%xmm3
mulss %xmm0,%xmm1
addss %xmm9,%xmm1
subss %xmm6,%xmm3
addss %xmm6,%xmm1
movaps %xmm2,%xmm5
mulss %xmm0,%xmm5
addss %xmm3,%xmm5
addss %xmm2,%xmm2
addss %xmm1,%xmm2
addss %xmm4,%xmm5
addss %xmm4,%xmm2
mulss %xmm2,%xmm2
mulss %xmm5,%xmm5
addss %xmm2,%xmm5
xorps %xmm1,%xmm1
sqrtss %xmm5,%xmm1
movss %xmm1,(%r14,%r13,4)
mov %rdi,%r13
cmp %rdi,%r11
jne 4f6ffb0 <_ZNK11SobelFilter6detectEPKfPfii+0x210>
jmp 4f6fe12 <_ZNK11SobelFilter6detectEPKfPfii+0x72>
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
int3
int3
int3