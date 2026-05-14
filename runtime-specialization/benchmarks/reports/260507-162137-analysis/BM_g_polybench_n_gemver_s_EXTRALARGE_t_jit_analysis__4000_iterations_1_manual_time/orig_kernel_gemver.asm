<kernel_gemver>:
test %edi,%edi
jle 17f3ee6 <kernel_gemver+0x476>
push %r15
push %r14
push %r13
push %r12
push %rbx
lea 0x23dc4a8(%rip),%rax # 3bcff30 <g_gemver_A>
mov (%rax),%rax
mov %edi,%ecx
imul $0x7d08,%rcx,%rdx
lea (%rax,%rdx,1),%r10
add $0xffffffffffff8300,%r10
lea 0x23dc49a(%rip),%rdx # 3bcff40 <g_gemver_u1>
lea (%rdx,%rcx,8),%r11
lea 0x23e418f(%rip),%rsi # 3bd7c40 <g_gemver_v1>
lea (%rsi,%rcx,8),%rbx
lea 0x23ebe84(%rip),%r8 # 3bdf940 <g_gemver_u2>
lea (%r8,%rcx,8),%r14
lea 0x23f3b79(%rip),%r9 # 3be7640 <g_gemver_v2>
lea (%r9,%rcx,8),%r15
movq %rax,%xmm0
pshufd $0x44,%xmm0,%xmm4
movq %rbx,%xmm0
movq %r11,%xmm3
punpcklqdq %xmm0,%xmm3
movq %r15,%xmm0
movq %r14,%xmm5
punpcklqdq %xmm0,%xmm5
movq %r10,%xmm0
pshufd $0x44,%xmm0,%xmm1
movdqa -0x2d4f13(%rip),%xmm2 # 151ebf0 <.LCPI1085_0>
pxor %xmm2,%xmm5
pxor %xmm2,%xmm4
movdqa %xmm5,%xmm6
pcmpgtd %xmm4,%xmm6
pxor %xmm2,%xmm3
movdqa %xmm3,%xmm0
pcmpgtd %xmm4,%xmm0
movdqa %xmm0,%xmm7
shufps $0x88,%xmm6,%xmm7
pcmpeqd %xmm4,%xmm5
pcmpeqd %xmm4,%xmm3
shufps $0xdd,%xmm5,%xmm3
andps %xmm7,%xmm3
shufps $0xdd,%xmm6,%xmm0
orps %xmm3,%xmm0
movq %r9,%xmm3
movq %r8,%xmm4
punpcklqdq %xmm3,%xmm4
pxor %xmm2,%xmm4
pxor %xmm2,%xmm1
movdqa %xmm1,%xmm3
pcmpgtd %xmm4,%xmm3
movq %rsi,%xmm5
movq %rdx,%xmm6
punpcklqdq %xmm5,%xmm6
pxor %xmm2,%xmm6
movdqa %xmm1,%xmm2
pcmpgtd %xmm6,%xmm2
movdqa %xmm2,%xmm5
shufps $0x88,%xmm3,%xmm5
pcmpeqd %xmm1,%xmm4
pcmpeqd %xmm1,%xmm6
shufps $0xdd,%xmm4,%xmm6
andps %xmm5,%xmm6
shufps $0xdd,%xmm3,%xmm2
orps %xmm6,%xmm2
andps %xmm0,%xmm2
movmskps %xmm2,%r10d
mov %ecx,%r11d
and $0x7ffffffc,%r11d
mov %edi,%ebx
shr $0x2,%ebx
and $0x1fffffff,%ebx
shl $0x5,%rbx
lea 0x10(%rax),%r14
xor %r15d,%r15d
mov %rax,%r12
jmp 17f3bda <kernel_gemver+0x16a>
nop
inc %r15
add $0x7d00,%r14
add $0x7d00,%r12
cmp %rcx,%r15
je 17f3cc5 <kernel_gemver+0x255>
cmp $0x6,%edi
jb 17f3c80 <kernel_gemver+0x210>
mov $0x0,%r13d
test %r10b,%r10b
jne 17f3c90 <kernel_gemver+0x220>
movsd (%rdx,%r15,8),%xmm0
unpcklpd %xmm0,%xmm0
movsd (%r8,%r15,8),%xmm1
unpcklpd %xmm1,%xmm1
xor %r13d,%r13d
nopl 0x0(%rax)
movupd -0x10(%r14,%r13,1),%xmm2
movupd (%r14,%r13,1),%xmm3
movapd (%rsi,%r13,1),%xmm4
mulpd %xmm0,%xmm4
addpd %xmm2,%xmm4
movapd 0x10(%rsi,%r13,1),%xmm2
mulpd %xmm0,%xmm2
addpd %xmm3,%xmm2
movapd (%r9,%r13,1),%xmm3
mulpd %xmm1,%xmm3
addpd %xmm4,%xmm3
movapd 0x10(%r9,%r13,1),%xmm4
mulpd %xmm1,%xmm4
addpd %xmm2,%xmm4
movupd %xmm3,-0x10(%r14,%r13,1)
movupd %xmm4,(%r14,%r13,1)
add $0x20,%r13
cmp %r13,%rbx
jne 17f3c10 <kernel_gemver+0x1a0>
mov %r11,%r13
cmp %ecx,%r11d
jne 17f3c90 <kernel_gemver+0x220>
jmp 17f3bc0 <kernel_gemver+0x150>
nopw 0x0(%rax,%rax,1)
xor %r13d,%r13d
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rdx,%r15,8),%xmm0
mulsd (%rsi,%r13,8),%xmm0
addsd (%r12,%r13,8),%xmm0
movsd (%r8,%r15,8),%xmm1
mulsd (%r9,%r13,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r12,%r13,8)
inc %r13
cmp %r13,%rcx
jne 17f3c90 <kernel_gemver+0x220>
jmp 17f3bc0 <kernel_gemver+0x150>
lea -0x1(%rcx),%rdx
mov %ecx,%r8d
and $0x7ffffffe,%r8d
xor %r9d,%r9d
lea 0x2403363(%rip),%rsi # 3bf7040 <g_gemver_x>
movsd -0x2d3365(%rip),%xmm0 # 1520980 <.LCPI107_2>
lea 0x240b054(%rip),%r10 # 3bfed40 <g_gemver_y>
mov %rax,%r11
jmp 17f3d10 <kernel_gemver+0x2a0>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r9
add $0x8,%r11
cmp %rcx,%r9
je 17f3da0 <kernel_gemver+0x330>
movsd (%rsi,%r9,8),%xmm1
test %rdx,%rdx
je 17f3d70 <kernel_gemver+0x300>
mov %r11,%r14
xor %ebx,%ebx
movsd (%r14),%xmm2
mulsd %xmm0,%xmm2
mulsd (%r10,%rbx,8),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%rsi,%r9,8)
movsd 0x7d00(%r14),%xmm1
mulsd %xmm0,%xmm1
mulsd 0x8(%r10,%rbx,8),%xmm1
addsd %xmm2,%xmm1
movsd %xmm1,(%rsi,%r9,8)
add $0x2,%rbx
add $0xfa00,%r14
cmp %rbx,%r8
jne 17f3d20 <kernel_gemver+0x2b0>
test $0x1,%cl
je 17f3d00 <kernel_gemver+0x290>
jmp 17f3d77 <kernel_gemver+0x307>
xchg %ax,%ax
xor %ebx,%ebx
test $0x1,%cl
je 17f3d00 <kernel_gemver+0x290>
imul $0x7d00,%rbx,%r14
add %rax,%r14
movsd (%r14,%r9,8),%xmm2
mulsd %xmm0,%xmm2
mulsd (%r10,%rbx,8),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%rsi,%r9,8)
jmp 17f3d00 <kernel_gemver+0x290>
cmp $0x4,%edi
jae 17f3da9 <kernel_gemver+0x339>
xor %edi,%edi
jmp 17f3e04 <kernel_gemver+0x394>
mov %ecx,%edi
and $0x7ffffffc,%edi
mov %ecx,%r8d
shr $0x2,%r8d
and $0x1fffffff,%r8d
shl $0x5,%r8
xor %r9d,%r9d
lea 0x2412c73(%rip),%r10 # 3c06a40 <g_gemver_z>
nopl (%rax)
movapd (%rsi,%r9,1),%xmm0
movapd 0x10(%rsi,%r9,1),%xmm1
addpd (%r10,%r9,1),%xmm0
addpd 0x10(%r10,%r9,1),%xmm1
movapd %xmm0,(%rsi,%r9,1)
movapd %xmm1,0x10(%rsi,%r9,1)
add $0x20,%r9
cmp %r9,%r8
jne 17f3dd0 <kernel_gemver+0x360>
cmp %ecx,%edi
je 17f3e28 <kernel_gemver+0x3b8>
lea 0x2412c35(%rip),%r8 # 3c06a40 <g_gemver_z>
nopl 0x0(%rax,%rax,1)
movsd (%rsi,%rdi,8),%xmm0
addsd (%r8,%rdi,8),%xmm0
movsd %xmm0,(%rsi,%rdi,8)
inc %rdi
cmp %rdi,%rcx
jne 17f3e10 <kernel_gemver+0x3a0>
mov %ecx,%edi
and $0x7ffffffe,%edi
lea 0x8(%rax),%r8
xor %r9d,%r9d
lea 0x23fb502(%rip),%r10 # 3bef340 <g_gemver_w>
movsd -0x2d363e(%rip),%xmm0 # 1520808 <.LCPI107_0>
jmp 17f3e5f <kernel_gemver+0x3ef>
nopl 0x0(%rax,%rax,1)
inc %r9
add $0x7d00,%r8
cmp %rcx,%r9
je 17f3edd <kernel_gemver+0x46d>
movsd (%r10,%r9,8),%xmm1
xor %r11d,%r11d
test %rdx,%rdx
je 17f3eaf <kernel_gemver+0x43f>
nopl (%rax)
movsd -0x8(%r8,%r11,8),%xmm2
mulsd %xmm0,%xmm2
mulsd (%rsi,%r11,8),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%r10,%r9,8)
movsd (%r8,%r11,8),%xmm1
mulsd %xmm0,%xmm1
mulsd 0x8(%rsi,%r11,8),%xmm1
addsd %xmm2,%xmm1
movsd %xmm1,(%r10,%r9,8)
add $0x2,%r11
cmp %r11,%rdi
jne 17f3e70 <kernel_gemver+0x400>
test $0x1,%cl
je 17f3e50 <kernel_gemver+0x3e0>
imul $0x7d00,%r9,%rbx
add %rax,%rbx
movsd (%rbx,%r11,8),%xmm2
mulsd %xmm0,%xmm2
mulsd (%rsi,%r11,8),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%r10,%r9,8)
jmp 17f3e50 <kernel_gemver+0x3e0>
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