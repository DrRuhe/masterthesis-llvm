<kernel_syr2k>:
test %esi,%esi
jle 17f5787 <kernel_syr2k+0x3d7>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
lea 0x2429a1f(%rip),%rax # 3c1ede8 <g_syr2k_C>
mov (%rax),%r10
mov %esi,%eax
mov %rax,-0x48(%rsp)
test %edi,%edi
jle 17f56c0 <kernel_syr2k+0x310>
lea 0x2429a0e(%rip),%rax # 3c1edf0 <g_syr2k_A>
mov (%rax),%rax
mov %rax,-0x20(%rsp)
lea 0x2429a07(%rip),%rax # 3c1edf8 <g_syr2k_B>
mov (%rax),%rax
mov %rax,-0x28(%rsp)
movabs $0x7ffffffffffffffc,%rax
mov %edi,%ecx
mov %rcx,-0x8(%rsp)
lea 0x0(,%rcx,8),%rcx
mov %rcx,-0x30(%rsp)
lea 0x8(%r10),%rcx
mov %rcx,-0x38(%rsp)
lea 0x10(%r10),%r11
mov $0x1,%ebx
xor %esi,%esi
movsd -0x2d4ab3(%rip),%xmm0 # 1520980 <.LCPI107_2>
add $0x2,%rax
mov %rax,-0x40(%rsp)
movsd -0x2d4c3c(%rip),%xmm1 # 1520808 <.LCPI107_0>
movapd -0x2d5d5c(%rip),%xmm2 # 151f6f0 <.LCPI143_2>
movapd -0x2d68e4(%rip),%xmm3 # 151eb70 <.LCPI107_1>
mov %r10,%r12
mov %r10,-0x18(%rsp)
jmp 17f5489 <kernel_syr2k+0xd9>
xchg %ax,%ax
mov -0x10(%rsp),%rsi
inc %rsi
inc %rbx
add $0x5140,%r11
add $0x5140,%r12
cmp -0x48(%rsp),%rsi
mov -0x18(%rsp),%r10
je 17f577d <kernel_syr2k+0x3cd>
cmp $0x4,%rbx
jae 17f54a0 <kernel_syr2k+0xf0>
xor %ecx,%ecx
jmp 17f54f0 <kernel_syr2k+0x140>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rbx,%rax
and $0xfffffffffffffffc,%rax
mov %rbx,%rcx
movabs $0x7ffffffffffffffc,%rdx
and %rdx,%rcx
xor %edx,%edx
nopl 0x0(%rax)
movupd -0x10(%r11,%rdx,8),%xmm4
movupd (%r11,%rdx,8),%xmm5
mulpd %xmm3,%xmm4
mulpd %xmm3,%xmm5
movupd %xmm4,-0x10(%r11,%rdx,8)
movupd %xmm5,(%r11,%rdx,8)
add $0x4,%rdx
cmp %rdx,%rax
jne 17f54c0 <kernel_syr2k+0x110>
jmp 17f5503 <kernel_syr2k+0x153>
nopl (%rax)
movsd (%r12,%rcx,8),%xmm4
mulsd %xmm0,%xmm4
movsd %xmm4,(%r12,%rcx,8)
inc %rcx
cmp %rcx,%rbx
jne 17f54f0 <kernel_syr2k+0x140>
mov %rbx,%r13
and $0xfffffffffffffffe,%r13
imul $0x5140,%rsi,%rax
add %r10,%rax
imul $0x5148,%rsi,%rcx
add -0x38(%rsp),%rcx
mov %rsi,-0x10(%rsp)
imul $0x3e80,%rsi,%rbp
mov -0x20(%rsp),%r14
lea (%r14,%rbp,1),%r8
mov -0x30(%rsp),%rdx
add %rbp,%rdx
lea (%r14,%rdx,1),%rsi
mov -0x28(%rsp),%r15
add %r15,%rdx
add %r15,%rbp
cmp %rsi,%rax
setb %sil
cmp %rcx,%r8
setb %dil
cmp %rcx,%r14
setb %r10b
or %dil,%r10b
and %sil,%r10b
cmp %rdx,%rax
setb %al
cmp %rcx,%r15
setb %dl
and %al,%dl
cmp %rcx,%rbp
setb %r9b
and %al,%r9b
or %dl,%r9b
or %r10b,%r9b
mov %rbx,%rax
and -0x40(%rsp),%rax
mov %r15,%rsi
mov %r14,%rdx
xor %ecx,%ecx
jmp 17f55b6 <kernel_syr2k+0x206>
nopl 0x0(%rax)
inc %rcx
add $0x8,%rdx
add $0x8,%rsi
cmp -0x8(%rsp),%rcx
je 17f5460 <kernel_syr2k+0xb0>
cmp $0x4,%rbx
setb %dil
lea 0x0(,%rcx,8),%r15
add %rbp,%r15
lea (%r8,%rcx,8),%r14
or %r9b,%dil
test $0x1,%dil
je 17f55f0 <kernel_syr2k+0x240>
xor %r10d,%r10d
imul $0x3e80,%r10,%rdi
jmp 17f5680 <kernel_syr2k+0x2d0>
data16 cs nopw 0x0(%rax,%rax,1)
movsd (%r15),%xmm4
unpcklpd %xmm4,%xmm4
movsd (%r14),%xmm5
unpcklpd %xmm5,%xmm5
xor %r10d,%r10d
xor %edi,%edi
nopw 0x0(%rax,%rax,1)
movsd (%rdx,%r10,1),%xmm6
movhpd 0x3e80(%rdx,%r10,1),%xmm6
mulpd %xmm2,%xmm6
mulpd %xmm4,%xmm6
movsd (%rsi,%r10,1),%xmm7
movhpd 0x3e80(%rsi,%r10,1),%xmm7
mulpd %xmm2,%xmm7
mulpd %xmm5,%xmm7
addpd %xmm6,%xmm7
movupd (%r12,%rdi,8),%xmm6
addpd %xmm7,%xmm6
movupd %xmm6,(%r12,%rdi,8)
add $0x2,%rdi
add $0x7d00,%r10
cmp %rdi,%r13
jne 17f5610 <kernel_syr2k+0x260>
mov %rax,%r10
cmp %rax,%rbx
je 17f55a0 <kernel_syr2k+0x1f0>
imul $0x3e80,%r10,%rdi
nopw 0x0(%rax,%rax,1)
movsd (%rdx,%rdi,1),%xmm4
mulsd %xmm1,%xmm4
movsd (%rsi,%rdi,1),%xmm5
mulsd %xmm1,%xmm5
mulsd (%r14),%xmm5
mulsd (%r15),%xmm4
addsd %xmm5,%xmm4
addsd (%r12,%r10,8),%xmm4
movsd %xmm4,(%r12,%r10,8)
inc %r10
add $0x3e80,%rdi
cmp %r10,%rbx
jne 17f5680 <kernel_syr2k+0x2d0>
jmp 17f55a0 <kernel_syr2k+0x1f0>
movabs $0x7ffffffffffffffc,%rcx
lea 0x10(%r10),%rdx
mov $0x1,%esi
xor %edi,%edi
movsd -0x2d4d5d(%rip),%xmm0 # 1520980 <.LCPI107_2>
movapd -0x2d6b75(%rip),%xmm1 # 151eb70 <.LCPI107_1>
jmp 17f570b <kernel_syr2k+0x35b>
nopw 0x0(%rax,%rax,1)
inc %rdi
inc %rsi
add $0x5140,%rdx
add $0x5140,%r10
cmp -0x48(%rsp),%rdi
je 17f577d <kernel_syr2k+0x3cd>
cmp $0x4,%rsi
jae 17f5720 <kernel_syr2k+0x370>
xor %r8d,%r8d
jmp 17f5760 <kernel_syr2k+0x3b0>
cs nopw 0x0(%rax,%rax,1)
mov %rsi,%rax
and $0xfffffffffffffffc,%rax
mov %rsi,%r8
and %rcx,%r8
xor %r9d,%r9d
movupd -0x10(%rdx,%r9,8),%xmm2
movupd (%rdx,%r9,8),%xmm3
mulpd %xmm1,%xmm2
mulpd %xmm1,%xmm3
movupd %xmm2,-0x10(%rdx,%r9,8)
movupd %xmm3,(%rdx,%r9,8)
add $0x4,%r9
cmp %r9,%rax
jne 17f5730 <kernel_syr2k+0x380>
jmp 17f5773 <kernel_syr2k+0x3c3>
nopl (%rax)
movsd (%r10,%r8,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r10,%r8,8)
inc %r8
cmp %r8,%rsi
jne 17f5760 <kernel_syr2k+0x3b0>
jmp 17f56f0 <kernel_syr2k+0x340>
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