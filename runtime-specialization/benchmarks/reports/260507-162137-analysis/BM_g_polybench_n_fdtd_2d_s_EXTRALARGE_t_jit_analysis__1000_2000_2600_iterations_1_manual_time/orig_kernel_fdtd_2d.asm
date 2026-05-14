<kernel_fdtd_2d>:
test %edi,%edi
jle 17fcd6f <kernel_fdtd_2d+0xa7f>
test %edx,%edx
jle 17fcd6f <kernel_fdtd_2d+0xa7f>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x48,%rsp
lea 0x24596b3(%rip),%rax # 3c559c8 <g_fdtd2d_ey>
mov (%rax),%r11
lea 0x24596b1(%rip),%rcx # 3c559d0 <g_fdtd2d_hz>
mov (%rcx),%r12
lea -0x5140(%r12),%rcx
mov %rcx,0x8(%rsp)
lea 0x5140(%r11),%rbp
mov %edi,%ecx
mov %rcx,-0x58(%rsp)
mov %edx,%eax
mov %esi,%r10d
test %esi,%esi
setle %cl
cmp $0x1,%edx
sete %r8b
or %cl,%r8b
mov %rdx,-0x70(%rsp)
mov %r10,-0x78(%rsp)
mov %r11,-0x60(%rsp)
je 17fc400 <kernel_fdtd_2d+0x110>
cmp $0x1,%esi
jg 17fcac3 <kernel_fdtd_2d+0x7d3>
mov %eax,%ecx
and $0x7ffffffc,%ecx
mov %eax,%esi
shr $0x2,%esi
and $0x1fffffff,%esi
shl $0x5,%rsi
xor %r8d,%r8d
lea 0x2459650(%rip),%r9 # 3c559e0 <g_fdtd2d_fict>
mov -0x60(%rsp),%rdi
jmp 17fc3ae <kernel_fdtd_2d+0xbe>
nopw 0x0(%rax,%rax,1)
inc %r8
cmp -0x58(%rsp),%r8
je 17fcd61 <kernel_fdtd_2d+0xa71>
movsd (%r9,%r8,8),%xmm0
cmp $0x4,%edx
jae 17fc3c0 <kernel_fdtd_2d+0xd0>
xor %r10d,%r10d
jmp 17fc3f0 <kernel_fdtd_2d+0x100>
xchg %ax,%ax
movapd %xmm0,%xmm1
unpcklpd %xmm0,%xmm1
xor %r10d,%r10d
nopl 0x0(%rax,%rax,1)
movupd %xmm1,(%rdi,%r10,1)
movupd %xmm1,0x10(%rdi,%r10,1)
add $0x20,%r10
cmp %r10,%rsi
jne 17fc3d0 <kernel_fdtd_2d+0xe0>
mov %rcx,%r10
cmp %eax,%ecx
je 17fc3a0 <kernel_fdtd_2d+0xb0>
nopl (%rax)
movsd %xmm0,(%rdi,%r10,8)
inc %r10
cmp %r10,%rax
jne 17fc3f0 <kernel_fdtd_2d+0x100>
jmp 17fc3a0 <kernel_fdtd_2d+0xb0>
lea 0x24595b9(%rip),%rcx # 3c559c0 <g_fdtd2d_ex>
mov (%rcx),%rdi
mov %rdi,(%rsp)
lea -0x1(%rsi),%ecx
lea -0x1(%rdx),%r14d
mov %rcx,-0x38(%rsp)
imul $0x5140,%rcx,%rcx
lea 0x0(,%r14,8),%r15
lea (%rcx,%r14,8),%r8
lea (%r12,%r8,1),%rcx
add $0xffffffffffffaec0,%rcx
mov %rcx,-0x80(%rsp)
lea (%rdi,%r8,1),%rcx
add $0xffffffffffffaec8,%rcx
mov %rcx,-0x50(%rsp)
add %r11,%r8
lea 0x8(%rdi),%rcx
imul $0x5140,%r10,%r10
lea (%r10,%rax,8),%r10
lea (%rdi,%r10,1),%r9
add $0xffffffffffffaec0,%r9
lea -0x5140(%r12,%r10,1),%rbx
lea -0x1(%rax),%r13
add %r11,%r10
add $0xffffffffffffaec0,%r10
mov %eax,%edi
and $0x7ffffffc,%edi
mov %rdi,-0x8(%rsp)
cmp %rbx,%rbp
setb %bpl
cmp %r10,%r12
setb %dil
and %bpl,%dil
mov %dil,-0x40(%rsp)
mov %rcx,-0x18(%rsp)
cmp %rbx,%rcx
setb %r10b
cmp %r9,%r12
setb %cl
and %r10b,%cl
mov %cl,-0x30(%rsp)
mov %r13,-0x68(%rsp)
and $0xfffffffffffffffc,%r13
lea 0x1(%r13),%rcx
mov %rcx,-0x48(%rsp)
cmp -0x50(%rsp),%r12
setb %r10b
mov (%rsp),%rbx
mov -0x80(%rsp),%rcx
cmp %rcx,%rbx
setb %r9b
and %r10b,%r9b
cmp %r8,%r12
setb %r8b
cmp %rcx,%r11
setb %cl
and %r8b,%cl
or %r9b,%cl
mov %cl,-0x50(%rsp)
mov %r14d,%ecx
and $0xfffffffc,%ecx
mov %rcx,0x40(%rsp)
mov %eax,%ecx
shr $0x2,%ecx
and $0x1fffffff,%ecx
shl $0x5,%rcx
lea 0x5150(%r12),%rdi
mov %rdi,0x28(%rsp)
lea 0x5150(%r11),%rdi
mov %rdi,-0x28(%rsp)
and $0xffffffffffffffe0,%r15
movsd -0x2dbe78(%rip),%xmm0 # 15206c8 <.LCPI130_3>
movsd -0x2dbbe8(%rip),%xmm1 # 1520960 <.LCPI199_3>
movapd -0x2de490(%rip),%xmm2 # 151e0c0 <.LCPI199_2>
movapd -0x2dc568(%rip),%xmm3 # 151fff0 <.LCPI199_0>
lea 0x5148(%r11),%rdi
mov %rdi,0x20(%rsp)
lea 0x5148(%r12),%rdi
mov %rdi,0x18(%rsp)
lea 0x18(%r12),%rdi
mov %rdi,0x30(%rsp)
lea 0x18(%rbx),%rdi
mov %rdi,-0x20(%rsp)
mov %r12,-0x80(%rsp)
lea 0x10(%r12),%rdi
mov %rdi,0x10(%rsp)
xor %r8d,%r8d
mov %r11,%rdi
mov %rsi,-0x10(%rsp)
jmp 17fc5bd <kernel_fdtd_2d+0x2cd>
mov 0x38(%rsp),%r8
inc %r8
cmp -0x58(%rsp),%r8
mov -0x70(%rsp),%rdx
mov -0x10(%rsp),%rsi
je 17fcd61 <kernel_fdtd_2d+0xa71>
mov %r8,0x38(%rsp)
lea 0x2459417(%rip),%r9 # 3c559e0 <g_fdtd2d_fict>
movsd (%r9,%r8,8),%xmm4
cmp $0x4,%edx
jae 17fc5e0 <kernel_fdtd_2d+0x2f0>
xor %r8d,%r8d
mov -0x78(%rsp),%r12
jmp 17fc620 <kernel_fdtd_2d+0x330>
xchg %ax,%ax
movapd %xmm4,%xmm5
unpcklpd %xmm4,%xmm5
xor %r8d,%r8d
mov -0x78(%rsp),%r12
movupd %xmm5,(%rdi,%r8,1)
movupd %xmm5,0x10(%rdi,%r8,1)
add $0x20,%r8
cmp %r8,%rcx
jne 17fc5f0 <kernel_fdtd_2d+0x300>
mov -0x8(%rsp),%rdi
mov %rdi,%r8
cmp %eax,%edi
mov -0x60(%rsp),%rdi
je 17fc62e <kernel_fdtd_2d+0x33e>
nopw 0x0(%rax,%rax,1)
movsd %xmm4,(%rdi,%r8,8)
inc %r8
cmp %r8,%rax
jne 17fc620 <kernel_fdtd_2d+0x330>
cmp $0x2,%esi
jge 17fc920 <kernel_fdtd_2d+0x630>
mov -0x80(%rsp),%rsi
mov -0x18(%rsp),%r8
mov -0x20(%rsp),%r11
mov 0x30(%rsp),%rbx
xor %ebp,%ebp
jmp 17fc678 <kernel_fdtd_2d+0x388>
nop
inc %rbp
add $0x5140,%rbx
add $0x5140,%r11
add $0x5140,%r8
add $0x5140,%rsi
cmp %r12,%rbp
je 17fc7b0 <kernel_fdtd_2d+0x4c0>
cmp $0x5,%edx
setb %r9b
or -0x30(%rsp),%r9b
mov $0x1,%r10d
test $0x1,%r9b
jne 17fc709 <kernel_fdtd_2d+0x419>
xor %r9d,%r9d
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%r11,%r9,8),%xmm4
movupd (%r11,%r9,8),%xmm5
movupd -0x18(%rbx,%r9,8),%xmm6
movupd -0x10(%rbx,%r9,8),%xmm7
subpd %xmm6,%xmm7
movupd -0x8(%rbx,%r9,8),%xmm6
movupd (%rbx,%r9,8),%xmm8
subpd %xmm6,%xmm8
mulpd %xmm3,%xmm7
addpd %xmm4,%xmm7
mulpd %xmm3,%xmm8
addpd %xmm5,%xmm8
movupd %xmm7,-0x10(%r11,%r9,8)
movupd %xmm8,(%r11,%r9,8)
add $0x4,%r9
cmp %r9,%r13
jne 17fc6a0 <kernel_fdtd_2d+0x3b0>
mov -0x48(%rsp),%r10
cmp %r13,-0x68(%rsp)
je 17fc650 <kernel_fdtd_2d+0x360>
mov %r10,%r9
test $0x1,%al
jne 17fc749 <kernel_fdtd_2d+0x459>
imul $0x5140,%rbp,%r12
mov (%rsp),%r9
add %r12,%r9
add -0x80(%rsp),%r12
movsd (%r12,%r10,8),%xmm4
subsd -0x8(%r12,%r10,8),%xmm4
mov -0x78(%rsp),%r12
mulsd %xmm0,%xmm4
addsd (%r9,%r10,8),%xmm4
movsd %xmm4,(%r9,%r10,8)
lea 0x1(%r10),%r9
cmp -0x68(%rsp),%r10
je 17fc650 <kernel_fdtd_2d+0x360>
data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rsi,%r9,8),%xmm4
subsd -0x8(%rsi,%r9,8),%xmm4
mulsd %xmm0,%xmm4
addsd -0x8(%r8,%r9,8),%xmm4
movsd %xmm4,-0x8(%r8,%r9,8)
movsd 0x8(%rsi,%r9,8),%xmm4
subsd (%rsi,%r9,8),%xmm4
mulsd %xmm0,%xmm4
addsd (%r8,%r9,8),%xmm4
movsd %xmm4,(%r8,%r9,8)
add $0x2,%r9
cmp %r9,%rax
jne 17fc760 <kernel_fdtd_2d+0x470>
jmp 17fc650 <kernel_fdtd_2d+0x360>
nopw 0x0(%rax,%rax,1)
cmpl $0x2,-0x10(%rsp)
jl 17fc5a0 <kernel_fdtd_2d+0x2b0>
mov -0x18(%rsp),%rsi
mov %rdi,%r8
mov -0x80(%rsp),%r11
mov 0x10(%rsp),%rbx
mov -0x28(%rsp),%rbp
mov -0x20(%rsp),%r12
xor %r10d,%r10d
jmp 17fc818 <kernel_fdtd_2d+0x528>
nopl 0x0(%rax)
inc %r10
add $0x5140,%r12
add $0x5140,%rbp
add $0x5140,%rbx
add $0x5140,%r11
add $0x5140,%r8
add $0x5140,%rsi
cmp -0x38(%rsp),%r10
je 17fc5a0 <kernel_fdtd_2d+0x2b0>
cmpl $0x5,-0x70(%rsp)
setb %dl
or -0x50(%rsp),%dl
xor %r9d,%r9d
test $0x1,%dl
jne 17fc8e0 <kernel_fdtd_2d+0x5f0>
movupd -0x10(%rbx,%r9,1),%xmm4
movupd (%rbx,%r9,1),%xmm5
movupd -0x18(%r12,%r9,1),%xmm6
movupd -0x10(%r12,%r9,1),%xmm7
subpd %xmm6,%xmm7
movupd -0x8(%r12,%r9,1),%xmm6
movupd (%r12,%r9,1),%xmm8
subpd %xmm6,%xmm8
movupd -0x10(%rbp,%r9,1),%xmm6
addpd %xmm7,%xmm6
movupd 0x0(%rbp,%r9,1),%xmm7
addpd %xmm8,%xmm7
movupd -0x5150(%rbp,%r9,1),%xmm8
subpd %xmm8,%xmm6
movupd -0x5140(%rbp,%r9,1),%xmm8
subpd %xmm8,%xmm7
mulpd %xmm2,%xmm6
addpd %xmm4,%xmm6
mulpd %xmm2,%xmm7
addpd %xmm5,%xmm7
movupd %xmm6,-0x10(%rbx,%r9,1)
movupd %xmm7,(%rbx,%r9,1)
add $0x20,%r9
cmp %r9,%r15
jne 17fc830 <kernel_fdtd_2d+0x540>
mov 0x40(%rsp),%rdx
mov %rdx,%r9
cmp %r14d,%edx
je 17fc7e0 <kernel_fdtd_2d+0x4f0>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rsi,%r9,8),%xmm4
subsd -0x8(%rsi,%r9,8),%xmm4
addsd 0x5140(%r8,%r9,8),%xmm4
subsd (%r8,%r9,8),%xmm4
mulsd %xmm1,%xmm4
addsd (%r11,%r9,8),%xmm4
movsd %xmm4,(%r11,%r9,8)
inc %r9
cmp %r9,%r14
jne 17fc8e0 <kernel_fdtd_2d+0x5f0>
jmp 17fc7e0 <kernel_fdtd_2d+0x4f0>
nopw 0x0(%rax,%rax,1)
mov $0x1,%esi
mov 0x18(%rsp),%r11
mov 0x20(%rsp),%rbp
mov -0x28(%rsp),%rbx
mov 0x28(%rsp),%r8
jmp 17fc968 <kernel_fdtd_2d+0x678>
nopl 0x0(%rax,%rax,1)
inc %rsi
add $0x5140,%r8
add $0x5140,%rbx
add $0x5140,%rbp
add $0x5140,%r11
cmp %r12,%rsi
je 17fc637 <kernel_fdtd_2d+0x347>
cmp $0x4,%edx
setb %r9b
or -0x40(%rsp),%r9b
test $0x1,%r9b
je 17fc990 <kernel_fdtd_2d+0x6a0>
xor %r10d,%r10d
jmp 17fca13 <kernel_fdtd_2d+0x723>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
xor %r9d,%r9d
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%rbx,%r9,1),%xmm4
movupd (%rbx,%r9,1),%xmm5
movupd -0x10(%r8,%r9,1),%xmm6
movupd (%r8,%r9,1),%xmm7
movupd -0x5150(%r8,%r9,1),%xmm8
subpd %xmm8,%xmm6
movupd -0x5140(%r8,%r9,1),%xmm8
subpd %xmm8,%xmm7
mulpd %xmm3,%xmm6
addpd %xmm4,%xmm6
mulpd %xmm3,%xmm7
addpd %xmm5,%xmm7
movupd %xmm6,-0x10(%rbx,%r9,1)
movupd %xmm7,(%rbx,%r9,1)
add $0x20,%r9
cmp %r9,%rcx
jne 17fc9a0 <kernel_fdtd_2d+0x6b0>
mov -0x8(%rsp),%rdi
mov %rdi,%r10
cmp %eax,%edi
mov -0x60(%rsp),%rdi
je 17fc940 <kernel_fdtd_2d+0x650>
mov %r10,%r9
test $0x1,%al
je 17fca5f <kernel_fdtd_2d+0x76f>
imul $0x5140,%rsi,%r9
lea (%rdi,%r9,1),%r12
mov -0x80(%rsp),%rdx
add %r9,%rdx
movsd (%rdx,%r10,8),%xmm4
add 0x8(%rsp),%r9
subsd (%r9,%r10,8),%xmm4
mulsd %xmm0,%xmm4
addsd (%r12,%r10,8),%xmm4
movsd %xmm4,(%r12,%r10,8)
mov -0x78(%rsp),%r12
mov -0x70(%rsp),%rdx
mov %r10,%r9
or $0x1,%r9
cmp -0x68(%rsp),%r10
je 17fc940 <kernel_fdtd_2d+0x650>
nopw 0x0(%rax,%rax,1)
movsd -0x8(%r11,%r9,8),%xmm4
subsd -0x5148(%r11,%r9,8),%xmm4
mulsd %xmm0,%xmm4
addsd -0x8(%rbp,%r9,8),%xmm4
movsd %xmm4,-0x8(%rbp,%r9,8)
movsd (%r11,%r9,8),%xmm4
subsd -0x5140(%r11,%r9,8),%xmm4
mulsd %xmm0,%xmm4
addsd 0x0(%rbp,%r9,8),%xmm4
movsd %xmm4,0x0(%rbp,%r9,8)
add $0x2,%r9
cmp %r9,%rax
jne 17fca70 <kernel_fdtd_2d+0x780>
jmp 17fc940 <kernel_fdtd_2d+0x650>
imul $0x5140,-0x78(%rsp),%rcx
lea (%rcx,%rax,8),%rcx
mov -0x60(%rsp),%rdi
lea (%rdi,%rcx,1),%r9
add $0xffffffffffffaec0,%r9
lea (%r12,%rcx,1),%r8
add $0xffffffffffffaec0,%r8
mov %eax,%ecx
and $0x7ffffffc,%ecx
cmp %r8,%rbp
setb %r8b
cmp %r9,%r12
setb %sil
and %r8b,%sil
lea -0x1(%rax),%r9
mov %eax,%r11d
shr $0x2,%r11d
and $0x1fffffff,%r11d
shl $0x5,%r11
lea 0x5150(%r12),%r8
mov %r8,-0x30(%rsp)
lea 0x5150(%rdi),%r8
mov %r8,-0x38(%rsp)
lea 0x5148(%rdi),%r8
mov %r8,-0x40(%rsp)
mov %r12,-0x80(%rsp)
lea 0x5148(%r12),%r8
mov %r8,-0x48(%rsp)
xor %r8d,%r8d
movsd -0x2dc494(%rip),%xmm0 # 15206c8 <.LCPI130_3>
movapd -0x2dcb74(%rip),%xmm1 # 151fff0 <.LCPI199_0>
jmp 17fcb88 <kernel_fdtd_2d+0x898>
cs nopw 0x0(%rax,%rax,1)
mov -0x68(%rsp),%r8
inc %r8
cmp -0x58(%rsp),%r8
mov -0x70(%rsp),%rdx
je 17fcd61 <kernel_fdtd_2d+0xa71>
lea 0x2458e51(%rip),%r10 # 3c559e0 <g_fdtd2d_fict>
movsd (%r10,%r8,8),%xmm2
cmp $0x4,%edx
mov %r8,-0x68(%rsp)
jae 17fcbb0 <kernel_fdtd_2d+0x8c0>
xor %edx,%edx
jmp 17fcbe0 <kernel_fdtd_2d+0x8f0>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movapd %xmm2,%xmm3
unpcklpd %xmm2,%xmm3
xor %edx,%edx
nopw 0x0(%rax,%rax,1)
movupd %xmm3,(%rdi,%rdx,1)
movupd %xmm3,0x10(%rdi,%rdx,1)
add $0x20,%rdx
cmp %rdx,%r11
jne 17fcbc0 <kernel_fdtd_2d+0x8d0>
mov %rcx,%rdx
cmp %eax,%ecx
je 17fcbed <kernel_fdtd_2d+0x8fd>
nopl 0x0(%rax,%rax,1)
movsd %xmm2,(%rdi,%rdx,8)
inc %rdx
cmp %rdx,%rax
jne 17fcbe0 <kernel_fdtd_2d+0x8f0>
mov $0x1,%ebp
mov -0x48(%rsp),%r12
mov -0x40(%rsp),%r15
mov -0x38(%rsp),%r14
mov -0x30(%rsp),%rbx
jmp 17fcc3a <kernel_fdtd_2d+0x94a>
nopl 0x0(%rax,%rax,1)
inc %rbp
add $0x5140,%rbx
add $0x5140,%r14
add $0x5140,%r15
add $0x5140,%r12
cmp -0x78(%rsp),%rbp
je 17fcb70 <kernel_fdtd_2d+0x880>
cmpl $0x4,-0x70(%rsp)
setb %dl
or %sil,%dl
test $0x1,%dl
je 17fcc50 <kernel_fdtd_2d+0x960>
xor %r8d,%r8d
jmp 17fccc3 <kernel_fdtd_2d+0x9d3>
nop
xor %edx,%edx
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%r14,%rdx,1),%xmm2
movupd (%r14,%rdx,1),%xmm3
movupd -0x10(%rbx,%rdx,1),%xmm4
movupd (%rbx,%rdx,1),%xmm5
movupd -0x5150(%rbx,%rdx,1),%xmm6
subpd %xmm6,%xmm4
movupd -0x5140(%rbx,%rdx,1),%xmm6
subpd %xmm6,%xmm5
mulpd %xmm1,%xmm4
addpd %xmm2,%xmm4
mulpd %xmm1,%xmm5
addpd %xmm3,%xmm5
movupd %xmm4,-0x10(%r14,%rdx,1)
movupd %xmm5,(%r14,%rdx,1)
add $0x20,%rdx
cmp %rdx,%r11
jne 17fcc60 <kernel_fdtd_2d+0x970>
mov %rcx,%r8
cmp %eax,%ecx
je 17fcc10 <kernel_fdtd_2d+0x920>
mov %r8,%rdx
test $0x1,%al
je 17fcd06 <kernel_fdtd_2d+0xa16>
imul $0x5140,%rbp,%rdx
lea (%rdi,%rdx,1),%r10
mov -0x80(%rsp),%r13
add %rdx,%r13
movsd 0x0(%r13,%r8,8),%xmm2
add 0x8(%rsp),%rdx
subsd (%rdx,%r8,8),%xmm2
mulsd %xmm0,%xmm2
addsd (%r10,%r8,8),%xmm2
movsd %xmm2,(%r10,%r8,8)
mov %r8,%rdx
or $0x1,%rdx
cmp %r9,%r8
je 17fcc10 <kernel_fdtd_2d+0x920>
nop
movsd -0x8(%r12,%rdx,8),%xmm2
subsd -0x5148(%r12,%rdx,8),%xmm2
mulsd %xmm0,%xmm2
addsd -0x8(%r15,%rdx,8),%xmm2
movsd %xmm2,-0x8(%r15,%rdx,8)
movsd (%r12,%rdx,8),%xmm2
subsd -0x5140(%r12,%rdx,8),%xmm2
mulsd %xmm0,%xmm2
addsd (%r15,%rdx,8),%xmm2
movsd %xmm2,(%r15,%rdx,8)
add $0x2,%rdx
cmp %rdx,%rax
jne 17fcd10 <kernel_fdtd_2d+0xa20>
jmp 17fcc10 <kernel_fdtd_2d+0x920>
add $0x48,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret