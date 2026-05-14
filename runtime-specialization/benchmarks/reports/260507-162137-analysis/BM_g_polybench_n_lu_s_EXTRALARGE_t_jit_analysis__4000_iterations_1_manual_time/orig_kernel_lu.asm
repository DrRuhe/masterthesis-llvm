<kernel_lu>:
test %edi,%edi
jle 17f8c5f <kernel_lu+0x1ff>
push %r15
push %r14
push %r12
push %rbx
lea 0x2435dd2(%rip),%rax # 3c2e848 <g_lu_A>
mov (%rax),%rax
mov %edi,%ecx
lea 0x8(%rax),%rdx
xor %esi,%esi
mov %rax,%rdi
jmp 17f8aa7 <kernel_lu+0x47>
cs nopw 0x0(%rax,%rax,1)
inc %rsi
add $0x7d00,%rdx
add $0x8,%rdi
cmp %rcx,%rsi
je 17f8c58 <kernel_lu+0x1f8>
test %rsi,%rsi
je 17f8a90 <kernel_lu+0x30>
mov %rsi,%r8
and $0xfffffffffffffffe,%r8
imul $0x7d00,%rsi,%r9
add %rax,%r9
mov %rax,%r10
xor %r11d,%r11d
jmp 17f8afb <kernel_lu+0x9b>
data16 cs nopw 0x0(%rax,%rax,1)
movsd (%r9),%xmm0
imul $0x7d00,%r11,%rbx
add %rax,%rbx
divsd (%rbx,%r11,8),%xmm0
movsd %xmm0,(%r9,%r11,8)
inc %r11
add $0x8,%r10
cmp %rsi,%r11
je 17f8ba0 <kernel_lu+0x140>
test %r11,%r11
je 17f8ad0 <kernel_lu+0x70>
lea (%r9,%r11,8),%rbx
movsd (%r9,%r11,8),%xmm0
cmp $0x1,%r11
jne 17f8b20 <kernel_lu+0xc0>
xor %r14d,%r14d
jmp 17f8b6d <kernel_lu+0x10d>
data16 cs nopw 0x0(%rax,%rax,1)
mov %r11,%r15
and $0xfffffffffffffffe,%r15
mov %r10,%r12
xor %r14d,%r14d
nopl (%rax)
movsd -0x8(%rdx,%r14,8),%xmm1
mulsd (%r12),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rbx)
movsd (%rdx,%r14,8),%xmm1
mulsd 0x7d00(%r12),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rbx)
add $0x2,%r14
add $0xfa00,%r12
cmp %r14,%r15
jne 17f8b30 <kernel_lu+0xd0>
test $0x1,%r11b
je 17f8ad5 <kernel_lu+0x75>
movsd (%r9,%r14,8),%xmm1
imul $0x7d00,%r14,%r14
add %rax,%r14
mulsd (%r14,%r11,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rbx)
jmp 17f8ad5 <kernel_lu+0x75>
nopw 0x0(%rax,%rax,1)
mov %rdi,%r10
mov %rsi,%r11
jmp 17f8bc0 <kernel_lu+0x160>
nopl 0x0(%rax,%rax,1)
inc %r11
add $0x8,%r10
cmp %rcx,%r11
je 17f8a90 <kernel_lu+0x30>
lea (%r9,%r11,8),%rbx
movsd (%r9,%r11,8),%xmm0
cmp $0x1,%rsi
jne 17f8be0 <kernel_lu+0x180>
xor %r14d,%r14d
jmp 17f8c2b <kernel_lu+0x1cb>
data16 cs nopw 0x0(%rax,%rax,1)
mov %r10,%r15
xor %r14d,%r14d
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%rdx,%r14,8),%xmm1
mulsd (%r15),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rbx)
movsd (%rdx,%r14,8),%xmm1
mulsd 0x7d00(%r15),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rbx)
add $0x2,%r14
add $0xfa00,%r15
cmp %r14,%r8
jne 17f8bf0 <kernel_lu+0x190>
test $0x1,%sil
je 17f8bb0 <kernel_lu+0x150>
movsd (%r9,%r14,8),%xmm1
imul $0x7d00,%r14,%r14
add %rax,%r14
mulsd (%r14,%r11,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rbx)
jmp 17f8bb0 <kernel_lu+0x150>
pop %rbx
pop %r12
pop %r14
pop %r15
ret