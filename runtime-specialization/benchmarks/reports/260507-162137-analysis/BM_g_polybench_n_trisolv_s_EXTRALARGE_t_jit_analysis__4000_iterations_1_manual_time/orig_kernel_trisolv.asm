<kernel_trisolv>:
test %edi,%edi
jle 17f98a6 <kernel_trisolv+0xf6>
lea 0x244c7a1(%rip),%rax # 3c45f60 <g_trisolv_L>
mov (%rax),%rax
mov %edi,%ecx
lea 0x8(%rax),%rdx
xor %esi,%esi
lea 0x245449f(%rip),%rdi # 3c4dc70 <g_trisolv_b>
lea 0x244c798(%rip),%r8 # 3c45f70 <g_trisolv_x>
jmp 17f9809 <kernel_trisolv+0x59>
nopw 0x0(%rax,%rax,1)
imul $0x7d00,%rsi,%r9
add %rax,%r9
divsd (%r9,%rsi,8),%xmm0
movsd %xmm0,(%r8,%rsi,8)
inc %rsi
add $0x7d00,%rdx
cmp %rcx,%rsi
je 17f98a6 <kernel_trisolv+0xf6>
movsd (%rdi,%rsi,8),%xmm0
movsd %xmm0,(%r8,%rsi,8)
test %rsi,%rsi
je 17f97e0 <kernel_trisolv+0x30>
cmp $0x1,%rsi
jne 17f9830 <kernel_trisolv+0x80>
xor %r9d,%r9d
jmp 17f9877 <kernel_trisolv+0xc7>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rsi,%r10
and $0xfffffffffffffffe,%r10
xor %r9d,%r9d
nopw 0x0(%rax,%rax,1)
movsd -0x8(%rdx,%r9,8),%xmm1
mulsd (%r8,%r9,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%r8,%rsi,8)
movsd (%rdx,%r9,8),%xmm1
mulsd 0x8(%r8,%r9,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%r8,%rsi,8)
add $0x2,%r9
cmp %r9,%r10
jne 17f9840 <kernel_trisolv+0x90>
test $0x1,%sil
je 17f97e0 <kernel_trisolv+0x30>
imul $0x7d00,%rsi,%r10
add %rax,%r10
movsd (%r10,%r9,8),%xmm1
mulsd (%r8,%r9,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%r8,%rsi,8)
jmp 17f97e0 <kernel_trisolv+0x30>
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