<kernel_floyd_warshall>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
mov %edi,-0x44(%rsp)
test %edi,%edi
jle 17fab59 <kernel_floyd_warshall+0x269>
lea 0x245b083(%rip),%rax # 3c55990 <g_fw_path>
mov (%rax),%rax
mov -0x44(%rsp),%esi
mov %esi,%ecx
imul $0x5784,%rcx,%rdx
add %rax,%rdx
add $0xffffffffffffa880,%rdx
mov %rdx,-0x20(%rsp)
imul $0x5780,%rcx,%rdx
add %rax,%rdx
add $0xffffffffffffa884,%rdx
mov %rdx,-0x28(%rsp)
lea (%rax,%rcx,4),%rdx
mov %rdx,-0x30(%rsp)
mov %ecx,%edx
and $0x7ffffff8,%edx
mov %rdx,-0x10(%rsp)
lea -0x1(%rcx),%rdx
mov %rdx,-0x8(%rsp)
mov %esi,%r11d
shr $0x3,%r11d
and $0xfffffff,%r11d
shl $0x5,%r11
lea 0x10(%rax),%r13
lea 0x4(%rax),%rdx
mov %rdx,-0x40(%rsp)
xor %r15d,%r15d
mov %rax,%r12
mov %r13,-0x38(%rsp)
jmp 17fa9aa <kernel_floyd_warshall+0xba>
nopl (%rax)
inc %r15
add $0x5780,%r13
add $0x5780,%r12
cmp %rcx,%r15
je 17fab59 <kernel_floyd_warshall+0x269>
lea (%rax,%r15,4),%rdx
mov -0x28(%rsp),%rsi
lea (%rsi,%r15,4),%rsi
imul $0x5780,%r15,%r8
lea (%rax,%r8,1),%r9
add -0x30(%rsp),%r8
cmp %rsi,%rax
setb %sil
mov -0x20(%rsp),%rdi
cmp %rdi,%rdx
setb %dl
and %sil,%dl
cmp %r8,%rax
setb %r8b
mov %r9,-0x18(%rsp)
cmp %rdi,%r9
setb %sil
and %r8b,%sil
or %dl,%sil
mov -0x40(%rsp),%r14
mov -0x38(%rsp),%rbx
xor %r8d,%r8d
jmp 17faa2a <kernel_floyd_warshall+0x13a>
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r8
add $0x5780,%rbx
add $0x5780,%r14
cmp %rcx,%r8
je 17fa990 <kernel_floyd_warshall+0xa0>
cmpl $0x8,-0x44(%rsp)
setb %r9b
imul $0x5780,%r8,%rbp
add %rax,%rbp
lea 0x0(,%r15,4),%rdx
add %rbp,%rdx
or %sil,%r9b
test $0x1,%r9b
je 17faa60 <kernel_floyd_warshall+0x170>
xor %r9d,%r9d
jmp 17faae1 <kernel_floyd_warshall+0x1f1>
nopl 0x0(%rax)
movd (%rdx),%xmm0
pshufd $0x0,%xmm0,%xmm0
xor %r10d,%r10d
nopl 0x0(%rax)
movdqu -0x10(%rbx,%r10,1),%xmm1
movdqu (%rbx,%r10,1),%xmm2
movdqu -0x10(%r13,%r10,1),%xmm3
movdqu 0x0(%r13,%r10,1),%xmm4
paddd %xmm0,%xmm3
paddd %xmm0,%xmm4
movdqa %xmm3,%xmm5
pcmpgtd %xmm1,%xmm5
pand %xmm5,%xmm1
pandn %xmm3,%xmm5
por %xmm1,%xmm5
movdqa %xmm4,%xmm1
pcmpgtd %xmm2,%xmm1
pand %xmm1,%xmm2
pandn %xmm4,%xmm1
por %xmm2,%xmm1
movdqu %xmm5,-0x10(%rbx,%r10,1)
movdqu %xmm1,(%rbx,%r10,1)
add $0x20,%r10
cmp %r10,%r11
jne 17faa70 <kernel_floyd_warshall+0x180>
mov -0x10(%rsp),%rdi
mov %rdi,%r9
cmp %ecx,%edi
je 17faa10 <kernel_floyd_warshall+0x120>
mov %r9,%r10
test $0x1,%cl
je 17fab0d <kernel_floyd_warshall+0x21d>
mov -0x18(%rsp),%rdi
mov (%rdi,%r9,4),%r10d
add (%rdx),%r10d
mov 0x0(%rbp,%r9,4),%edi
cmp %r10d,%edi
cmovl %edi,%r10d
mov %r10d,0x0(%rbp,%r9,4)
mov %r9,%r10
or $0x1,%r10
cmp -0x8(%rsp),%r9
je 17faa10 <kernel_floyd_warshall+0x120>
nopl 0x0(%rax,%rax,1)
mov (%r12,%r10,4),%edi
add (%rdx),%edi
mov -0x4(%r14,%r10,4),%r9d
mov (%r14,%r10,4),%ebp
cmp %edi,%r9d
cmovl %r9d,%edi
mov %edi,-0x4(%r14,%r10,4)
mov 0x4(%r12,%r10,4),%edi
add (%rdx),%edi
cmp %edi,%ebp
cmovl %ebp,%edi
mov %edi,(%r14,%r10,4)
add $0x2,%r10
cmp %r10,%rcx
jne 17fab20 <kernel_floyd_warshall+0x230>
jmp 17faa10 <kernel_floyd_warshall+0x120>
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
int3
int3
int3