<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_edge_detection_low_specializediiE3$_0JPKfPfEEET_PT0_DpT1_>:
mov (%rdi),%eax
mov 0x4(%rdi),%ecx
mov %rsi,%rdi
mov %rdx,%rsi
mov %eax,%edx
jmp 4f6e1d0 <_Z17sobel_edge_detectPKfPfii>
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
<_Z17sobel_edge_detectPKfPfii>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
mov %rdx,-0x40(%rsp)
mov %rsi,-0x30(%rsp)
test %ecx,%ecx
jle 4f6e46e <_Z17sobel_edge_detectPKfPfii+0x29e>
cmpl $0x0,-0x40(%rsp)
jle 4f6e46e <_Z17sobel_edge_detectPKfPfii+0x29e>
mov %ecx,%edx
mov -0x40(%rsp),%rcx
lea -0x1(%rcx),%eax
mov %ecx,%ecx
mov %edx,%esi
mov %rsi,-0x20(%rsp)
dec %edx
mov %edx,-0x34(%rsp)
lea 0x0(,%rcx,4),%rdx
mov %rdx,-0x28(%rsp)
mov %rcx,-0x18(%rsp)
lea -0x1(%rcx),%r11
xor %ebx,%ebx
movss -0x4e8400(%rip),%xmm0 # 4a85e30 <.LCPI0_0>
mov -0x30(%rsp),%r14
xor %esi,%esi
jmp 4f6e33e <_Z17sobel_edge_detectPKfPfii+0x16e>
nopl 0x0(%rax)
xor %r8d,%r8d
mov -0x8(%rsp),%r12
imul -0x18(%rsp),%r12
lea -0x1(%r8),%r13d
cmp %eax,%r13d
cmovge %eax,%r13d
test %r13d,%r13d
cmovle %ebx,%r13d
lea 0x0(%r13,%rbp,1),%edx
movss (%rdi,%rdx,4),%xmm1
cmp %eax,%r8d
mov %eax,%edx
cmovl %r8d,%edx
test %edx,%edx
cmovle %ebx,%edx
lea (%rdx,%rbp,1),%esi
lea 0x1(%r8),%r9d
cmp %eax,%r9d
cmovge %eax,%r9d
test %r9d,%r9d
cmovle %ebx,%r9d
add %r9d,%ebp
movss (%rdi,%rbp,4),%xmm4
lea (%r10,%r13,1),%r15d
movss (%rdi,%r15,4),%xmm5
add %r9d,%r10d
movss (%rdi,%r10,4),%xmm2
add %ecx,%r13d
movss (%rdi,%r13,4),%xmm6
add %ecx,%edx
movss (%rdi,%rdx,4),%xmm3
add %ecx,%r9d
movss (%rdi,%r9,4),%xmm7
movss (%rdi,%rsi,4),%xmm8
mulss %xmm0,%xmm8
subss %xmm1,%xmm8
subss %xmm4,%xmm8
subss %xmm1,%xmm4
addss %xmm5,%xmm5
subss %xmm5,%xmm4
addss %xmm2,%xmm2
addss %xmm4,%xmm2
subss %xmm6,%xmm2
addss %xmm7,%xmm2
addss %xmm6,%xmm8
addss %xmm3,%xmm3
addss %xmm8,%xmm3
addss %xmm7,%xmm3
mulss %xmm3,%xmm3
mulss %xmm2,%xmm2
addss %xmm3,%xmm2
xorps %xmm1,%xmm1
sqrtss %xmm2,%xmm1
mov -0x30(%rsp),%rcx
lea (%rcx,%r8,4),%rcx
movss %xmm1,(%rcx,%r12,4)
add -0x28(%rsp),%r14
mov -0x10(%rsp),%rcx
mov %rcx,%rsi
cmp -0x20(%rsp),%rcx
je 4f6e46e <_Z17sobel_edge_detectPKfPfii+0x29e>
lea -0x1(%rsi),%ebp
mov -0x34(%rsp),%ecx
cmp %ecx,%ebp
cmovge %ecx,%ebp
test %ebp,%ebp
cmovle %ebx,%ebp
mov -0x40(%rsp),%rdx
imul %edx,%ebp
cmp %ecx,%esi
mov %ecx,%r10d
cmovl %esi,%r10d
test %r10d,%r10d
cmovle %ebx,%r10d
imul %edx,%r10d
mov %rsi,-0x8(%rsp)
inc %rsi
cmp %ecx,%esi
mov %rsi,-0x10(%rsp)
cmovl %esi,%ecx
test %ecx,%ecx
cmovle %ebx,%ecx
imul %edx,%ecx
test %eax,%eax
je 4f6e240 <_Z17sobel_edge_detectPKfPfii+0x70>
xor %r13d,%r13d
lea -0x1(%r13),%esi
cmp %eax,%esi
cmovge %eax,%esi
test %esi,%esi
cmovle %ebx,%esi
lea (%rsi,%rbp,1),%r12d
cmp %eax,%r13d
mov %eax,%edx
cmovl %r13d,%edx
test %edx,%edx
cmovle %ebx,%edx
lea 0x1(%r13),%r8
cmp %eax,%r8d
mov %eax,%r9d
cmovl %r8d,%r9d
movss (%rdi,%r12,4),%xmm2
test %r9d,%r9d
cmovle %ebx,%r9d
lea (%rdx,%rbp,1),%r12d
lea (%r9,%rbp,1),%r15d
movss (%rdi,%r15,4),%xmm4
lea (%rsi,%r10,1),%r15d
movss (%rdi,%r15,4),%xmm5
lea (%r9,%r10,1),%r15d
movss (%rdi,%r15,4),%xmm1
add %ecx,%esi
movss (%rdi,%rsi,4),%xmm6
add %ecx,%edx
movss (%rdi,%rdx,4),%xmm3
add %ecx,%r9d
movss (%rdi,%r9,4),%xmm7
movss (%rdi,%r12,4),%xmm8
mulss %xmm0,%xmm8
subss %xmm2,%xmm8
subss %xmm4,%xmm8
subss %xmm2,%xmm4
addss %xmm5,%xmm5
subss %xmm5,%xmm4
addss %xmm1,%xmm1
addss %xmm4,%xmm1
subss %xmm6,%xmm1
addss %xmm7,%xmm1
addss %xmm6,%xmm8
addss %xmm3,%xmm3
addss %xmm8,%xmm3
addss %xmm7,%xmm3
mulss %xmm3,%xmm3
mulss %xmm1,%xmm1
addss %xmm3,%xmm1
sqrtss %xmm1,%xmm1
movss %xmm1,(%r14,%r13,4)
mov %r8,%r13
cmp %r8,%r11
jne 4f6e390 <_Z17sobel_edge_detectPKfPfii+0x1c0>
jmp 4f6e243 <_Z17sobel_edge_detectPKfPfii+0x73>
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