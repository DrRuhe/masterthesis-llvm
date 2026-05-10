<kernel_jacobi_1d>:
test %edi,%edi
setle %al
cmp $0x3,%esi
setl %cl
or %al,%cl
jne 17fdb0d <kernel_jacobi_1d+0x1bd>
dec %esi
lea -0x1(%rsi),%rax
mov %rax,%rcx
and $0xfffffffffffffffc,%rcx
lea 0x1(%rcx),%rdx
xor %r8d,%r8d
lea 0x2459fb2(%rip),%r9 # 3c57930 <g_jacobi1d_A>
movsd -0x2dcd8e(%rip),%xmm0 # 1520bf8 <.LCPI207_1>
lea 0x2461ca3(%rip),%r10 # 3c5f630 <g_jacobi1d_B>
movapd -0x2df235(%rip),%xmm1 # 151e760 <.LCPI207_0>
jmp 17fd9ac <kernel_jacobi_1d+0x5c>
nopw 0x0(%rax,%rax,1)
inc %r8d
cmp %edi,%r8d
je 17fdb0d <kernel_jacobi_1d+0x1bd>
movsd 0x8(%r9),%xmm2
mov $0x1,%r11d
cmp $0x4,%rax
jb 17fda30 <kernel_jacobi_1d+0xe0>
unpcklpd %xmm2,%xmm2
xor %r11d,%r11d
data16 cs nopw 0x0(%rax,%rax,1)
movapd 0x10(%r9,%r11,8),%xmm3
shufpd $0x1,%xmm3,%xmm2
addpd (%r9,%r11,8),%xmm2
addpd %xmm3,%xmm2
mulpd %xmm1,%xmm2
movupd %xmm2,0x8(%r10,%r11,8)
movapd 0x20(%r9,%r11,8),%xmm2
movapd %xmm3,%xmm4
shufpd $0x1,%xmm2,%xmm4
addpd %xmm3,%xmm4
addpd %xmm2,%xmm4
mulpd %xmm1,%xmm4
movupd %xmm4,0x18(%r10,%r11,8)
add $0x4,%r11
cmp %r11,%rcx
jne 17fd9d0 <kernel_jacobi_1d+0x80>
cmp %rcx,%rax
je 17fda58 <kernel_jacobi_1d+0x108>
unpckhpd %xmm2,%xmm2
mov %rdx,%r11
nopl 0x0(%rax)
addsd -0x8(%r9,%r11,8),%xmm2
movsd 0x8(%r9,%r11,8),%xmm3
addsd %xmm3,%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r10,%r11,8)
inc %r11
movapd %xmm3,%xmm2
cmp %r11,%rsi
jne 17fda30 <kernel_jacobi_1d+0xe0>
movsd 0x8(%r10),%xmm2
mov $0x1,%r11d
cmp $0x4,%rax
jb 17fdae0 <kernel_jacobi_1d+0x190>
unpcklpd %xmm2,%xmm2
xor %r11d,%r11d
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movapd 0x10(%r10,%r11,8),%xmm3
shufpd $0x1,%xmm3,%xmm2
addpd (%r10,%r11,8),%xmm2
addpd %xmm3,%xmm2
mulpd %xmm1,%xmm2
movupd %xmm2,0x8(%r9,%r11,8)
movapd 0x20(%r10,%r11,8),%xmm2
movapd %xmm3,%xmm4
shufpd $0x1,%xmm2,%xmm4
addpd %xmm3,%xmm4
addpd %xmm2,%xmm4
mulpd %xmm1,%xmm4
movupd %xmm4,0x18(%r9,%r11,8)
add $0x4,%r11
cmp %r11,%rcx
jne 17fda80 <kernel_jacobi_1d+0x130>
cmp %rcx,%rax
je 17fd9a0 <kernel_jacobi_1d+0x50>
unpckhpd %xmm2,%xmm2
mov %rdx,%r11
nopl (%rax)
addsd -0x8(%r10,%r11,8),%xmm2
movsd 0x8(%r10,%r11,8),%xmm3
addsd %xmm3,%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r9,%r11,8)
inc %r11
movapd %xmm3,%xmm2
cmp %r11,%rsi
jne 17fdae0 <kernel_jacobi_1d+0x190>
jmp 17fd9a0 <kernel_jacobi_1d+0x50>
ret
int3
int3