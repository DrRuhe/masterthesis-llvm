<kernel_mvt>:
test %edi,%edi
jle 17f2af1 <kernel_mvt+0x191>
push %rbx
lea 0x23dd5a0(%rip),%rax # 3bcff10 <g_mvt_A>
mov (%rax),%rax
mov %edi,%ecx
lea -0x1(%rcx),%rdx
mov %ecx,%esi
and $0x7ffffffe,%esi
lea 0x8(%rax),%rdi
xor %r8d,%r8d
lea 0x23be181(%rip),%r9 # 3bb0b10 <g_mvt_x1>
lea 0x23cdb7a(%rip),%r10 # 3bc0510 <g_mvt_y1>
jmp 17f29af <kernel_mvt+0x4f>
nopl 0x0(%rax,%rax,1)
inc %r8
add $0x7d00,%rdi
cmp %rcx,%r8
je 17f2a1e <kernel_mvt+0xbe>
movsd (%r9,%r8,8),%xmm0
xor %r11d,%r11d
test %rdx,%rdx
je 17f29f7 <kernel_mvt+0x97>
nopl (%rax)
movsd -0x8(%rdi,%r11,8),%xmm1
mulsd (%r10,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r9,%r8,8)
movsd (%rdi,%r11,8),%xmm0
mulsd 0x8(%r10,%r11,8),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r9,%r8,8)
add $0x2,%r11
cmp %r11,%rsi
jne 17f29c0 <kernel_mvt+0x60>
test $0x1,%cl
je 17f29a0 <kernel_mvt+0x40>
imul $0x7d00,%r8,%rbx
add %rax,%rbx
movsd (%rbx,%r11,8),%xmm1
mulsd (%r10,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r9,%r8,8)
jmp 17f29a0 <kernel_mvt+0x40>
mov %ecx,%esi
and $0x7ffffffe,%esi
xor %edi,%edi
lea 0x23c5de1(%rip),%r8 # 3bb8810 <g_mvt_x2>
lea 0x23d57da(%rip),%r9 # 3bc8210 <g_mvt_y2>
mov %rax,%r10
jmp 17f2a50 <kernel_mvt+0xf0>
nopl 0x0(%rax,%rax,1)
inc %rdi
add $0x8,%r10
cmp %rcx,%rdi
je 17f2af0 <kernel_mvt+0x190>
movsd (%r8,%rdi,8),%xmm0
test %rdx,%rdx
je 17f2ac0 <kernel_mvt+0x160>
mov %r10,%rbx
xor %r11d,%r11d
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rbx),%xmm1
mulsd (%r9,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r8,%rdi,8)
movsd 0x7d00(%rbx),%xmm0
mulsd 0x8(%r9,%r11,8),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r8,%rdi,8)
add $0x2,%r11
add $0xfa00,%rbx
cmp %r11,%rsi
jne 17f2a70 <kernel_mvt+0x110>
test $0x1,%cl
je 17f2a40 <kernel_mvt+0xe0>
jmp 17f2acc <kernel_mvt+0x16c>
data16 data16 cs nopw 0x0(%rax,%rax,1)
xor %r11d,%r11d
test $0x1,%cl
je 17f2a40 <kernel_mvt+0xe0>
imul $0x7d00,%r11,%rbx
add %rax,%rbx
movsd (%rbx,%rdi,8),%xmm1
mulsd (%r9,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r8,%rdi,8)
jmp 17f2a40 <kernel_mvt+0xe0>
pop %rbx
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
int3
int3