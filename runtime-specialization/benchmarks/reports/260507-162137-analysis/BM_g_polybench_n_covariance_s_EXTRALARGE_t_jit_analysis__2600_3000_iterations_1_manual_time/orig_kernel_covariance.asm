<kernel_covariance>:
test %edi,%edi
jle 17ef1b7 <kernel_covariance+0x4c7>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
cvtsi2sd %esi,%xmm0
lea 0x239fe9b(%rip),%rax # 3b8eba8 <g_cov_cov>
mov (%rax),%rax
mov %edi,%ecx
test %esi,%esi
jle 17ef092 <kernel_covariance+0x3a2>
lea 0x239fe7f(%rip),%rdx # 3b8eba0 <g_cov_data>
mov (%rdx),%rdx
mov %esi,%r8d
mov %r8d,%r10d
and $0x3,%r10d
mov %r8d,%r11d
and $0x7ffffffc,%r11d
imul $0x5140,%r10,%rbx
xor %r14d,%r14d
lea 0x239fe67(%rip),%r9 # 3b8ebb0 <g_cov_mean>
mov %rdx,%r15
jmp 17eed6a <kernel_covariance+0x7a>
xchg %ax,%ax
divsd %xmm0,%xmm1
movsd %xmm1,(%r9,%r14,8)
inc %r14
add $0x8,%r15
cmp %rcx,%r14
je 17eee0e <kernel_covariance+0x11e>
movq $0x0,(%r9,%r14,8)
xorpd %xmm1,%xmm1
cmp $0x4,%esi
jae 17eed80 <kernel_covariance+0x90>
xor %r12d,%r12d
jmp 17eedd9 <kernel_covariance+0xe9>
mov %r15,%r13
xor %r12d,%r12d
cs nopw 0x0(%rax,%rax,1)
addsd 0x0(%r13),%xmm1
movsd %xmm1,(%r9,%r14,8)
addsd 0x5140(%r13),%xmm1
movsd %xmm1,(%r9,%r14,8)
addsd 0xa280(%r13),%xmm1
movsd %xmm1,(%r9,%r14,8)
addsd 0xf3c0(%r13),%xmm1
movsd %xmm1,(%r9,%r14,8)
add $0x4,%r12
add $0x14500,%r13
cmp %r12,%r11
jne 17eed90 <kernel_covariance+0xa0>
test %r10,%r10
je 17eed50 <kernel_covariance+0x60>
imul $0x5140,%r12,%r13
add %r15,%r13
xor %r12d,%r12d
nop
addsd 0x0(%r13,%r12,1),%xmm1
movsd %xmm1,(%r9,%r14,8)
add $0x5140,%r12
cmp %r12,%rbx
jne 17eedf0 <kernel_covariance+0x100>
jmp 17eed50 <kernel_covariance+0x60>
imul $0x5140,%r8,%r10
add %rdx,%r10
lea (%r10,%rcx,8),%rbx
add $0xffffffffffffaec0,%rbx
lea (%r9,%rcx,8),%r10
cmp %r10,%rdx
setb %r11b
cmp %r9,%rbx
seta %r10b
and %r11b,%r10b
mov %r10b,-0x9(%rsp)
mov %ecx,%ebx
and $0x7ffffffc,%ebx
lea -0x1(%rcx),%r10
mov %r10,-0x8(%rsp)
mov %ecx,%r14d
shr $0x2,%r14d
and $0x1fffffff,%r14d
shl $0x5,%r14
lea 0x10(%rdx),%r15
xor %r12d,%r12d
mov %rdx,%r13
mov %edi,%r10d
jmp 17eee8d <kernel_covariance+0x19d>
nop
inc %r12
add $0x5140,%r15
add $0x5140,%r13
cmp %r8,%r12
mov %r10d,%edi
je 17eef77 <kernel_covariance+0x287>
cmp $0x4,%edi
setb %r11b
or -0x9(%rsp),%r11b
test $0x1,%r11b
je 17eeeb0 <kernel_covariance+0x1c0>
xor %r11d,%r11d
jmp 17eeefb <kernel_covariance+0x20b>
data16 data16 cs nopw 0x0(%rax,%rax,1)
xor %r11d,%r11d
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%r15,%r11,1),%xmm0
movupd (%r15,%r11,1),%xmm1
subpd (%r9,%r11,1),%xmm0
subpd 0x10(%r9,%r11,1),%xmm1
movupd %xmm0,-0x10(%r15,%r11,1)
movupd %xmm1,(%r15,%r11,1)
add $0x20,%r11
cmp %r11,%r14
jne 17eeec0 <kernel_covariance+0x1d0>
mov %rbx,%r11
cmp %ecx,%ebx
je 17eee70 <kernel_covariance+0x180>
mov %r11,%rbp
test $0x1,%cl
je 17eef28 <kernel_covariance+0x238>
imul $0x5140,%r12,%rbp
add %rdx,%rbp
movsd 0x0(%rbp,%r11,8),%xmm0
subsd (%r9,%r11,8),%xmm0
movsd %xmm0,0x0(%rbp,%r11,8)
mov %r11,%rbp
or $0x1,%rbp
cmp -0x8(%rsp),%r11
je 17eee70 <kernel_covariance+0x180>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd 0x0(%r13,%rbp,8),%xmm0
subsd (%r9,%rbp,8),%xmm0
movsd 0x8(%r13,%rbp,8),%xmm1
movsd %xmm0,0x0(%r13,%rbp,8)
subsd 0x8(%r9,%rbp,8),%xmm1
movsd %xmm1,0x8(%r13,%rbp,8)
add $0x2,%rbp
cmp %rbp,%rcx
jne 17eef40 <kernel_covariance+0x250>
jmp 17eee70 <kernel_covariance+0x180>
lea -0x1(%rsi),%edi
xorps %xmm0,%xmm0
cvtsi2sd %edi,%xmm0
mov %r8d,%edi
and $0x7ffffffe,%edi
xor %r9d,%r9d
xor %r10d,%r10d
jmp 17eefb0 <kernel_covariance+0x2c0>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r10
add $0x8,%r9
cmp %rcx,%r10
je 17ef1ad <kernel_covariance+0x4bd>
imul $0x5140,%r10,%r11
add %rax,%r11
mov %r9,%rbx
mov %r10,%r14
jmp 17eeff5 <kernel_covariance+0x305>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
divsd %xmm0,%xmm1
movsd %xmm1,(%r15)
imul $0x5140,%r14,%r15
add %rax,%r15
movsd %xmm1,(%r15,%r10,8)
inc %r14
add $0x8,%rbx
cmp %rcx,%r14
je 17eefa0 <kernel_covariance+0x2b0>
lea (%r11,%r14,8),%r15
movq $0x0,(%r11,%r14,8)
xorpd %xmm1,%xmm1
cmp $0x1,%esi
jne 17ef010 <kernel_covariance+0x320>
xor %r12d,%r12d
jmp 17ef064 <kernel_covariance+0x374>
nop
mov %rdx,%r13
xor %r12d,%r12d
cs nopw 0x0(%rax,%rax,1)
movsd 0x0(%r13,%r9,1),%xmm2
mulsd 0x0(%r13,%rbx,1),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%r15)
movsd 0x5140(%r13,%r9,1),%xmm1
mulsd 0x5140(%r13,%rbx,1),%xmm1
addsd %xmm2,%xmm1
movsd %xmm1,(%r15)
add $0x2,%r12
add $0xa280,%r13
cmp %r12,%rdi
jne 17ef020 <kernel_covariance+0x330>
test $0x1,%r8b
je 17eefd0 <kernel_covariance+0x2e0>
imul $0x5140,%r12,%r12
add %rdx,%r12
movsd (%r12,%r10,8),%xmm2
mulsd (%r12,%r14,8),%xmm2
addsd %xmm2,%xmm1
movsd %xmm1,(%r15)
jmp 17eefd0 <kernel_covariance+0x2e0>
xorpd %xmm1,%xmm1
divsd %xmm0,%xmm1
cmp $0x4,%edi
jae 17ef0a3 <kernel_covariance+0x3b3>
xor %edx,%edx
jmp 17ef0ea <kernel_covariance+0x3fa>
mov %ecx,%edx
and $0x7ffffffc,%edx
movapd %xmm1,%xmm2
unpcklpd %xmm1,%xmm2
mov %ecx,%esi
shr $0x2,%esi
and $0x1fffffff,%esi
shl $0x5,%rsi
xor %edi,%edi
lea 0x239fae5(%rip),%r8 # 3b8ebb0 <g_cov_mean>
nopl 0x0(%rax,%rax,1)
movapd %xmm2,(%r8,%rdi,1)
movapd %xmm2,0x10(%r8,%rdi,1)
add $0x20,%rdi
cmp %rdi,%rsi
jne 17ef0d0 <kernel_covariance+0x3e0>
cmp %ecx,%edx
je 17ef10d <kernel_covariance+0x41d>
lea 0x239fabf(%rip),%rsi # 3b8ebb0 <g_cov_mean>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd %xmm1,(%rsi,%rdx,8)
inc %rdx
cmp %rdx,%rcx
jne 17ef100 <kernel_covariance+0x410>
addsd -0x2ce545(%rip),%xmm0 # 1520bd0 <.LCPI0_2>
xorpd %xmm1,%xmm1
divsd %xmm0,%xmm1
lea -0x1(%rcx),%rdx
lea 0x8(%rax),%rsi
xor %edi,%edi
mov %rax,%r8
jmp 17ef143 <kernel_covariance+0x453>
nopl 0x0(%rax)
inc %rdi
add $0x8,%r8
add $0x5140,%rsi
cmp %rcx,%rdi
je 17ef1ad <kernel_covariance+0x4bd>
mov %ecx,%r10d
sub %edi,%r10d
mov %rdi,%r9
test $0x1,%r10b
je 17ef166 <kernel_covariance+0x476>
imul $0x5140,%rdi,%r9
add %rax,%r9
movsd %xmm1,(%r9,%rdi,8)
lea 0x1(%rdi),%r9
cmp %rdx,%rdi
je 17ef130 <kernel_covariance+0x440>
imul $0x5140,%r9,%r10
add %r8,%r10
data16 cs nopw 0x0(%rax,%rax,1)
movsd %xmm1,-0x8(%rsi,%r9,8)
movsd %xmm1,(%r10)
movsd %xmm1,(%rsi,%r9,8)
movsd %xmm1,0x5140(%r10)
add $0x2,%r9
add $0xa280,%r10
cmp %r9,%rcx
jne 17ef180 <kernel_covariance+0x490>
jmp 17ef130 <kernel_covariance+0x440>
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