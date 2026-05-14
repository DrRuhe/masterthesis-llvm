<kernel_doitgen>:
test %esi,%esi
setle %al
test %edi,%edi
setle %cl
or %al,%cl
test %edx,%edx
setle %al
or %cl,%al
jne 17f248b <kernel_doitgen+0x31b>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
lea 0x23be0f6(%rip),%rax # 3bb0290 <g_doitgen_A>
mov (%rax),%rcx
lea 0x23be0f4(%rip),%rax # 3bb0298 <g_doitgen_C4>
mov (%rax),%rax
mov %rax,-0x60(%rsp)
mov %esi,%eax
mov %rax,-0x50(%rsp)
mov %edi,%eax
mov %rax,-0x20(%rsp)
mov %rcx,%rax
sub 0x2384554(%rip),%rax # 3b76718 <g_doitgen_sum$got>
mov %rax,-0x58(%rsp)
mov %edx,%r9d
mov %r9d,%r10d
and $0x7ffffffe,%r10d
mov %r9d,%eax
and $0x7ffffffc,%eax
mov %rax,-0x28(%rsp)
mov %r9d,%eax
and $0x3,%eax
mov %rax,-0x68(%rsp)
lea 0x8(%rcx),%r13
mov %edx,%r15d
shr $0x2,%r15d
and $0x1fffffff,%r15d
shl $0x5,%r15
lea 0x10(%rcx),%r12
xor %esi,%esi
lea 0x23be08f(%rip),%rbp # 3bb02a0 <g_doitgen_sum>
mov %rcx,%rax
mov %rcx,%rdi
jmp 17f2257 <kernel_doitgen+0xe7>
nopl 0x0(%rax)
mov -0x40(%rsp),%rsi
inc %rsi
mov -0x30(%rsp),%r13
add $0x74040,%r13
mov -0x38(%rsp),%r12
add $0x74040,%r12
mov -0x48(%rsp),%rdi
add $0x74040,%rdi
cmp -0x50(%rsp),%rsi
je 17f2481 <kernel_doitgen+0x311>
mov %rsi,-0x40(%rsp)
imul $0x74040,%rsi,%r8
mov -0x58(%rsp),%rcx
add %r8,%rcx
mov %rcx,-0x18(%rsp)
mov %rdi,-0x48(%rsp)
mov %rdi,%rsi
mov %r12,-0x38(%rsp)
mov %r13,-0x30(%rsp)
xor %ecx,%ecx
jmp 17f22b8 <kernel_doitgen+0x148>
cs nopw 0x0(%rax,%rax,1)
mov -0x8(%rsp),%rcx
inc %rcx
add $0x870,%r13
add $0x870,%r12
add $0x870,%rsi
cmp -0x20(%rsp),%rcx
je 17f2220 <kernel_doitgen+0xb0>
mov %rcx,-0x8(%rsp)
imul $0x870,%rcx,%r14
mov -0x18(%rsp),%rcx
add %r14,%rcx
mov %rcx,-0x10(%rsp)
mov -0x60(%rsp),%rbx
xor %r11d,%r11d
jmp 17f22f0 <kernel_doitgen+0x180>
nopl 0x0(%rax,%rax,1)
inc %r11
add $0x8,%rbx
cmp %r9,%r11
je 17f23a0 <kernel_doitgen+0x230>
movq $0x0,0x0(%rbp,%r11,8)
xorpd %xmm0,%xmm0
cmp $0x1,%edx
jne 17f2310 <kernel_doitgen+0x1a0>
xor %edi,%edi
jmp 17f2360 <kernel_doitgen+0x1f0>
cs nopw 0x0(%rax,%rax,1)
mov %rbx,%rcx
xor %edi,%edi
data16 cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%r13,%rdi,8),%xmm1
mulsd (%rcx),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,0x0(%rbp,%r11,8)
movsd 0x0(%r13,%rdi,8),%xmm0
mulsd 0x870(%rcx),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,0x0(%rbp,%r11,8)
add $0x2,%rdi
add $0x10e0,%rcx
cmp %rdi,%r10
jne 17f2320 <kernel_doitgen+0x1b0>
test $0x1,%r9b
je 17f22e0 <kernel_doitgen+0x170>
lea (%rax,%r8,1),%rcx
add %r14,%rcx
movsd (%rcx,%rdi,8),%xmm1
imul $0x870,%rdi,%rcx
add -0x60(%rsp),%rcx
mulsd (%rcx,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,0x0(%rbp,%r11,8)
jmp 17f22e0 <kernel_doitgen+0x170>
nopl 0x0(%rax,%rax,1)
cmp $0x4,%edx
jb 17f2400 <kernel_doitgen+0x290>
mov $0x0,%edi
cmpq $0x20,-0x10(%rsp)
jb 17f23f3 <kernel_doitgen+0x283>
xor %ecx,%ecx
data16 data16 cs nopw 0x0(%rax,%rax,1)
movapd 0x0(%rbp,%rcx,1),%xmm0
movapd 0x10(%rbp,%rcx,1),%xmm1
movupd %xmm0,-0x10(%r12,%rcx,1)
movupd %xmm1,(%r12,%rcx,1)
add $0x20,%rcx
cmp %rcx,%r15
jne 17f23c0 <kernel_doitgen+0x250>
mov -0x28(%rsp),%rcx
mov %rcx,%rdi
cmp %r9d,%ecx
je 17f2290 <kernel_doitgen+0x120>
mov %rdi,%rcx
cmpq $0x0,-0x68(%rsp)
jne 17f240d <kernel_doitgen+0x29d>
jmp 17f2433 <kernel_doitgen+0x2c3>
xor %edi,%edi
mov %rdi,%rcx
cmpq $0x0,-0x68(%rsp)
je 17f2433 <kernel_doitgen+0x2c3>
mov -0x68(%rsp),%r11
mov %rdi,%rcx
data16 cs nopw 0x0(%rax,%rax,1)
movsd 0x0(%rbp,%rcx,8),%xmm0
movsd %xmm0,(%rsi,%rcx,8)
inc %rcx
dec %r11
jne 17f2420 <kernel_doitgen+0x2b0>
sub %r9,%rdi
cmp $0xfffffffffffffffc,%rdi
ja 17f2290 <kernel_doitgen+0x120>
movsd 0x0(%rbp,%rcx,8),%xmm0
movsd %xmm0,-0x10(%r12,%rcx,8)
movsd 0x8(%rbp,%rcx,8),%xmm0
movsd %xmm0,-0x8(%r12,%rcx,8)
movsd 0x10(%rbp,%rcx,8),%xmm0
movsd %xmm0,(%r12,%rcx,8)
movsd 0x18(%rbp,%rcx,8),%xmm0
movsd %xmm0,0x8(%r12,%rcx,8)
add $0x4,%rcx
cmp %rcx,%r9
jne 17f2440 <kernel_doitgen+0x2d0>
jmp 17f2290 <kernel_doitgen+0x120>
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