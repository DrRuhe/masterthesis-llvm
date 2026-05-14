<kernel_seidel_2d>:
test %edi,%edi
setle %al
cmp $0x3,%esi
setl %cl
or %al,%cl
jne 17fe85f <kernel_seidel_2d+0x10f>
lea 0x2468bd6(%rip),%rax # 3c67340 <g_seidel2d_A>
mov (%rax),%rax
dec %esi
lea 0xfa10(%rax),%rcx
lea -0x1(%rsi),%rdx
xor %r8d,%r8d
movsd -0x2dddf5(%rip),%xmm0 # 1520990 <.LCPI215_0>
data16 cs nopw 0x0(%rax,%rax,1)
mov $0x1,%r9d
mov %rcx,%r10
nopl 0x0(%rax)
imul $0x7d00,%r9,%r11
movsd -0x7d00(%rax,%r11,1),%xmm1
movsd -0x7cf8(%rax,%r11,1),%xmm3
movsd (%rax,%r11,1),%xmm7
movsd 0x8(%rax,%r11,1),%xmm2
movsd 0x7d00(%rax,%r11,1),%xmm4
movsd 0x7d08(%rax,%r11,1),%xmm6
xor %r11d,%r11d
nop
movapd %xmm3,%xmm5
addsd %xmm3,%xmm1
movsd -0xfa00(%r10,%r11,8),%xmm3
addsd %xmm3,%xmm1
addsd %xmm7,%xmm1
addsd %xmm2,%xmm1
movsd -0x7d00(%r10,%r11,8),%xmm2
addsd %xmm2,%xmm1
addsd %xmm4,%xmm1
addsd %xmm6,%xmm1
movapd %xmm6,%xmm4
movsd (%r10,%r11,8),%xmm6
addsd %xmm6,%xmm1
divsd %xmm0,%xmm1
movsd %xmm1,-0x7d08(%r10,%r11,8)
inc %r11
movapd %xmm1,%xmm7
movapd %xmm5,%xmm1
cmp %r11,%rdx
jne 17fe7e0 <kernel_seidel_2d+0x90>
inc %r9
add $0x7d00,%r10
cmp %rsi,%r9
jne 17fe7a0 <kernel_seidel_2d+0x50>
inc %r8d
cmp %edi,%r8d
jne 17fe790 <kernel_seidel_2d+0x40>
ret