<kernel_symm>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
test %edi,%edi
setle %al
mov %esi,-0x24(%rsp)
test %esi,%esi
setle %cl
or %al,%cl
jne 17f4d8d <kernel_symm+0x30d>
lea 0x242a329(%rip),%rax # 3c1edd0 <g_symm_C>
mov (%rax),%rax
lea 0x242a327(%rip),%rcx # 3c1edd8 <g_symm_A>
mov (%rcx),%r10
lea 0x242a325(%rip),%rcx # 3c1ede0 <g_symm_B>
mov (%rcx),%r11
mov %edi,%esi
mov -0x24(%rsp),%r9d
mov %r9d,%r8d
lea (%rax,%r8,8),%rcx
lea (%r11,%r8,8),%rdx
mov %rsi,-0x10(%rsp)
imul $0x3e88,%rsi,%rsi
add %r10,%rsi
add $0xffffffffffffc180,%rsi
cmp %rdx,%rax
setb %dl
cmp %rcx,%r11
setb %dil
and %dl,%dil
cmp %rsi,%rax
setb %dl
cmp %rcx,%r10
setb %cl
and %dl,%cl
or %dil,%cl
mov %cl,-0x25(%rsp)
mov %r8d,%ecx
and $0x7ffffffc,%ecx
mov %rcx,-0x20(%rsp)
lea -0x1(%r8),%rcx
mov %rcx,-0x18(%rsp)
mov %r9d,%ebx
shr $0x2,%ebx
and $0x1fffffff,%ebx
shl $0x5,%rbx
xor %r14d,%r14d
movsd -0x2d4335(%rip),%xmm0 # 1520808 <.LCPI107_0>
movsd -0x2d41c5(%rip),%xmm1 # 1520980 <.LCPI107_2>
xorpd %xmm2,%xmm2
movapd -0x2d5461(%rip),%xmm3 # 151f6f0 <.LCPI143_2>
movapd -0x2d5fe9(%rip),%xmm4 # 151eb70 <.LCPI107_1>
xorpd %xmm5,%xmm5
mov %r10,-0x8(%rsp)
mov %r10,%rcx
xor %r12d,%r12d
jmp 17f4b8c <kernel_symm+0x10c>
nopw 0x0(%rax,%rax,1)
inc %r12
add $0xa28,%r14
add $0x3e80,%rcx
cmp -0x10(%rsp),%r12
je 17f4d8d <kernel_symm+0x30d>
imul $0x3e80,%r12,%rdx
add -0x8(%rsp),%rdx
lea (%rdx,%r12,8),%r13
test %r12,%r12
je 17f4c50 <kernel_symm+0x1d0>
mov %rax,%rbp
mov %r11,%rdx
xor %r10d,%r10d
xchg %ax,%ax
imul $0x5140,%r12,%r9
lea (%r11,%r9,1),%rsi
lea (%rsi,%r10,8),%rsi
xorpd %xmm6,%xmm6
mov %rcx,%r15
xor %edi,%edi
nopl 0x0(%rax,%rax,1)
movsd (%rsi),%xmm7
mulsd %xmm0,%xmm7
mulsd (%r15),%xmm7
addsd 0x0(%rbp,%rdi,8),%xmm7
movsd %xmm7,0x0(%rbp,%rdi,8)
movsd (%rdx,%rdi,8),%xmm7
mulsd (%r15),%xmm7
addsd %xmm7,%xmm6
add $0xa28,%rdi
add $0x8,%r15
cmp %rdi,%r14
jne 17f4bd0 <kernel_symm+0x150>
add %rax,%r9
movsd (%rsi),%xmm7
mulsd %xmm0,%xmm7
mulsd 0x0(%r13),%xmm7
movsd (%r9,%r10,8),%xmm8
mulsd %xmm1,%xmm8
addsd %xmm7,%xmm8
mulsd %xmm0,%xmm6
addsd %xmm8,%xmm6
movsd %xmm6,(%r9,%r10,8)
inc %r10
add $0x8,%rdx
add $0x8,%rbp
cmp %r8,%r10
jne 17f4bb0 <kernel_symm+0x130>
jmp 17f4b70 <kernel_symm+0xf0>
cmpl $0x4,-0x24(%rsp)
setb %dl
or -0x25(%rsp),%dl
test $0x1,%dl
je 17f4c68 <kernel_symm+0x1e8>
xor %esi,%esi
jmp 17f4cef <kernel_symm+0x26f>
movsd 0x0(%r13),%xmm6
unpcklpd %xmm6,%xmm6
xor %edx,%edx
data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd (%rax,%rdx,1),%xmm7
movupd 0x10(%rax,%rdx,1),%xmm8
movupd (%r11,%rdx,1),%xmm9
movupd 0x10(%r11,%rdx,1),%xmm10
mulpd %xmm3,%xmm9
mulpd %xmm3,%xmm10
mulpd %xmm6,%xmm9
mulpd %xmm6,%xmm10
mulpd %xmm4,%xmm7
addpd %xmm9,%xmm7
mulpd %xmm4,%xmm8
addpd %xmm10,%xmm8
addpd %xmm5,%xmm7
addpd %xmm5,%xmm8
movupd %xmm7,(%rax,%rdx,1)
movupd %xmm8,0x10(%rax,%rdx,1)
add $0x20,%rdx
cmp %rdx,%rbx
jne 17f4c80 <kernel_symm+0x200>
mov -0x20(%rsp),%rdx
mov %rdx,%rsi
cmp %r8d,%edx
je 17f4b70 <kernel_symm+0xf0>
mov %rsi,%rdx
test $0x1,%r8b
je 17f4d25 <kernel_symm+0x2a5>
movsd (%r11,%rsi,8),%xmm6
mulsd %xmm0,%xmm6
mulsd 0x0(%r13),%xmm6
movsd (%rax,%rsi,8),%xmm7
mulsd %xmm1,%xmm7
addsd %xmm6,%xmm7
addsd %xmm2,%xmm7
movsd %xmm7,(%rax,%rsi,8)
mov %rsi,%rdx
or $0x1,%rdx
cmp -0x18(%rsp),%rsi
je 17f4b70 <kernel_symm+0xf0>
movsd (%r11,%rdx,8),%xmm6
mulsd %xmm0,%xmm6
mulsd 0x0(%r13),%xmm6
movsd (%rax,%rdx,8),%xmm7
mulsd %xmm1,%xmm7
addsd %xmm6,%xmm7
addsd %xmm2,%xmm7
movsd %xmm7,(%rax,%rdx,8)
movsd 0x8(%r11,%rdx,8),%xmm6
mulsd %xmm0,%xmm6
mulsd 0x0(%r13),%xmm6
movsd 0x8(%rax,%rdx,8),%xmm7
mulsd %xmm1,%xmm7
addsd %xmm6,%xmm7
addsd %xmm2,%xmm7
movsd %xmm7,0x8(%rax,%rdx,8)
add $0x2,%rdx
cmp %rdx,%r8
jne 17f4d30 <kernel_symm+0x2b0>
jmp 17f4b70 <kernel_symm+0xf0>
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