<kernel_syrk>:
test %esi,%esi
jle 17f6077 <kernel_syrk+0x3c7>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
lea 0x2429137(%rip),%rax # 3c1ee00 <g_syrk_C>
mov (%rax),%r9
mov %esi,%eax
mov %rax,-0x38(%rsp)
test %edi,%edi
jle 17f5fad <kernel_syrk+0x2fd>
lea 0x2429126(%rip),%rax # 3c1ee08 <g_syrk_A>
mov (%rax),%rax
mov %edi,%ecx
lea 0x8(%r9),%rdx
mov %rdx,-0x18(%rsp)
mov %rcx,-0x8(%rsp)
lea (%rax,%rcx,8),%rcx
mov %rcx,-0x20(%rsp)
lea 0x10(%r9),%r10
mov %rax,-0x30(%rsp)
add $0x3e80,%rax
mov %rax,-0x28(%rsp)
mov $0x1,%ebx
xor %r14d,%r14d
movsd -0x2d53a2(%rip),%xmm0 # 1520980 <.LCPI107_2>
movsd -0x2d5522(%rip),%xmm1 # 1520808 <.LCPI107_0>
movapd -0x2d71c2(%rip),%xmm2 # 151eb70 <.LCPI107_1>
mov %r9,%r15
mov %r9,-0x10(%rsp)
jmp 17f5d64 <kernel_syrk+0xb4>
nopl 0x0(%rax)
inc %r14
inc %rbx
add $0x5140,%r10
add $0x5140,%r15
cmp -0x38(%rsp),%r14
mov -0x10(%rsp),%r9
je 17f606d <kernel_syrk+0x3bd>
mov %rbx,%r12
and $0xfffffffffffffffc,%r12
cmp $0x4,%rbx
jae 17f5d80 <kernel_syrk+0xd0>
xor %eax,%eax
jmp 17f5dd0 <kernel_syrk+0x120>
data16 cs nopw 0x0(%rax,%rax,1)
mov %rbx,%rax
movabs $0x7ffffffffffffffc,%rcx
and %rcx,%rax
xor %ecx,%ecx
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%r10,%rcx,8),%xmm3
movupd (%r10,%rcx,8),%xmm4
mulpd %xmm2,%xmm3
mulpd %xmm2,%xmm4
movupd %xmm3,-0x10(%r10,%rcx,8)
movupd %xmm4,(%r10,%rcx,8)
add $0x4,%rcx
cmp %rcx,%r12
jne 17f5da0 <kernel_syrk+0xf0>
jmp 17f5de3 <kernel_syrk+0x133>
nopl (%rax)
movsd (%r15,%rax,8),%xmm3
mulsd %xmm0,%xmm3
movsd %xmm3,(%r15,%rax,8)
inc %rax
cmp %rax,%rbx
jne 17f5dd0 <kernel_syrk+0x120>
imul $0x5140,%r14,%r13
add %r9,%r13
imul $0x5148,%r14,%rax
add -0x18(%rsp),%rax
imul $0x3e80,%r14,%rbp
mov -0x20(%rsp),%rcx
add %rbp,%rcx
mov -0x30(%rsp),%rdi
add %rdi,%rbp
cmp %rcx,%r13
setb %dl
cmp %rax,%rdi
setb %sil
cmp %rax,%rbp
setb %cl
or %sil,%cl
and %dl,%cl
mov %rbx,%r8
movabs $0x7ffffffffffffffc,%rax
and %rax,%r8
mov -0x28(%rsp),%r11
mov %rdi,%rsi
xor %edx,%edx
jmp 17f5e66 <kernel_syrk+0x1b6>
nopl 0x0(%rax)
inc %rdx
add $0x8,%rsi
add $0x8,%r11
cmp -0x8(%rsp),%rdx
je 17f5d40 <kernel_syrk+0x90>
cmp $0x4,%rbx
setb %al
lea 0x0(,%rdx,8),%r9
add %rbp,%r9
or %cl,%al
test $0x1,%al
je 17f5e90 <kernel_syrk+0x1e0>
xor %edi,%edi
jmp 17f5f11 <kernel_syrk+0x261>
data16 cs nopw 0x0(%rax,%rax,1)
movsd (%r9),%xmm3
mulsd %xmm1,%xmm3
unpcklpd %xmm3,%xmm3
mov %rsi,%rax
xor %edi,%edi
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rax),%xmm4
movhpd 0x3e80(%rax),%xmm4
movsd 0x7d00(%rax),%xmm5
movhpd 0xbb80(%rax),%xmm5
movupd -0x10(%r10,%rdi,8),%xmm6
movupd (%r10,%rdi,8),%xmm7
mulpd %xmm3,%xmm4
addpd %xmm6,%xmm4
mulpd %xmm3,%xmm5
addpd %xmm7,%xmm5
movupd %xmm4,-0x10(%r10,%rdi,8)
movupd %xmm5,(%r10,%rdi,8)
add $0x4,%rdi
add $0xfa00,%rax
cmp %rdi,%r12
jne 17f5eb0 <kernel_syrk+0x200>
mov %r8,%rdi
cmp %r8,%rbx
je 17f5e50 <kernel_syrk+0x1a0>
mov %rdi,%rax
test $0x1,%bl
je 17f5f48 <kernel_syrk+0x298>
movsd (%r9),%xmm3
mulsd %xmm1,%xmm3
imul $0x3e80,%rdi,%rax
add -0x30(%rsp),%rax
mulsd (%rax,%rdx,8),%xmm3
addsd 0x0(%r13,%rdi,8),%xmm3
movsd %xmm3,0x0(%r13,%rdi,8)
mov %rdi,%rax
or $0x1,%rax
cmp %rdi,%r14
je 17f5e50 <kernel_syrk+0x1a0>
imul $0x3e80,%rax,%rdi
add %r11,%rdi
nopl 0x0(%rax,%rax,1)
movsd (%r9),%xmm3
mulsd %xmm1,%xmm3
mulsd -0x3e80(%rdi),%xmm3
addsd (%r15,%rax,8),%xmm3
movsd %xmm3,(%r15,%rax,8)
movsd (%r9),%xmm3
mulsd %xmm1,%xmm3
mulsd (%rdi),%xmm3
addsd 0x8(%r15,%rax,8),%xmm3
movsd %xmm3,0x8(%r15,%rax,8)
add $0x2,%rax
add $0x7d00,%rdi
cmp %rax,%rbx
jne 17f5f60 <kernel_syrk+0x2b0>
jmp 17f5e50 <kernel_syrk+0x1a0>
lea 0x10(%r9),%rcx
mov $0x1,%edx
xor %esi,%esi
movsd -0x2d5640(%rip),%xmm0 # 1520980 <.LCPI107_2>
movapd -0x2d7458(%rip),%xmm1 # 151eb70 <.LCPI107_1>
jmp 17f5fef <kernel_syrk+0x33f>
nopw 0x0(%rax,%rax,1)
inc %rsi
inc %rdx
add $0x5140,%rcx
add $0x5140,%r9
cmp -0x38(%rsp),%rsi
je 17f606d <kernel_syrk+0x3bd>
cmp $0x4,%rdx
jae 17f6000 <kernel_syrk+0x350>
xor %eax,%eax
jmp 17f6050 <kernel_syrk+0x3a0>
nopl 0x0(%rax)
mov %rdx,%rdi
and $0xfffffffffffffffc,%rdi
mov %rdx,%rax
movabs $0x7ffffffffffffffc,%r8
and %r8,%rax
xor %r8d,%r8d
nopw 0x0(%rax,%rax,1)
movupd -0x10(%rcx,%r8,8),%xmm2
movupd (%rcx,%r8,8),%xmm3
mulpd %xmm1,%xmm2
mulpd %xmm1,%xmm3
movupd %xmm2,-0x10(%rcx,%r8,8)
movupd %xmm3,(%rcx,%r8,8)
add $0x4,%r8
cmp %r8,%rdi
jne 17f6020 <kernel_syrk+0x370>
jmp 17f6063 <kernel_syrk+0x3b3>
nopl (%rax)
movsd (%r9,%rax,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r9,%rax,8)
inc %rax
cmp %rax,%rdx
jne 17f6050 <kernel_syrk+0x3a0>
jmp 17f5fd0 <kernel_syrk+0x320>
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