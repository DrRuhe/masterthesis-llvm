<kernel_cholesky>:
test %edi,%edi
jle 17f70c0 <kernel_cholesky+0x1f0>
push %r15
push %r14
push %r13
push %r12
push %rbx
lea 0x2427f38(%rip),%rax # 3c1ee20 <g_cholesky_A>
mov (%rax),%rbx
mov %edi,%r14d
lea 0x8(%rbx),%r15
xor %r12d,%r12d
xorpd %xmm2,%xmm2
mov %r15,%r13
jmp 17f6f2e <kernel_cholesky+0x5e>
xchg %ax,%ax
call 17d6500 <sqrt$plt>
xorpd %xmm2,%xmm2
imul $0x7d00,%r12,%rax
add %rbx,%rax
lea (%rax,%r12,8),%rax
movsd %xmm0,(%rax)
inc %r12
add $0x7d00,%r13
cmp %r14,%r12
je 17f70b7 <kernel_cholesky+0x1e7>
test %r12,%r12
je 17f70a0 <kernel_cholesky+0x1d0>
mov %r12,%rcx
and $0xfffffffffffffffe,%rcx
imul $0x7d00,%r12,%rax
add %rbx,%rax
mov %r15,%rdx
xor %esi,%esi
jmp 17f6f7b <kernel_cholesky+0xab>
nop
movsd (%rax),%xmm0
imul $0x7d00,%rsi,%rdi
add %rbx,%rdi
divsd (%rdi,%rsi,8),%xmm0
movsd %xmm0,(%rax,%rsi,8)
inc %rsi
add $0x7d00,%rdx
cmp %r12,%rsi
je 17f7020 <kernel_cholesky+0x150>
test %rsi,%rsi
je 17f6f50 <kernel_cholesky+0x80>
lea (%rax,%rsi,8),%rdi
movsd (%rax,%rsi,8),%xmm0
cmp $0x1,%rsi
jne 17f6fa0 <kernel_cholesky+0xd0>
xor %r8d,%r8d
jmp 17f6fe4 <kernel_cholesky+0x114>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rsi,%r9
and $0xfffffffffffffffe,%r9
xor %r8d,%r8d
nopw 0x0(%rax,%rax,1)
movsd -0x8(%r13,%r8,8),%xmm1
mulsd -0x8(%rdx,%r8,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rdi)
movsd 0x0(%r13,%r8,8),%xmm1
mulsd (%rdx,%r8,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rdi)
add $0x2,%r8
cmp %r8,%r9
jne 17f6fb0 <kernel_cholesky+0xe0>
test $0x1,%sil
je 17f6f54 <kernel_cholesky+0x84>
movsd (%rax,%r8,8),%xmm1
imul $0x7d00,%rsi,%r9
add %rbx,%r9
mulsd (%r9,%r8,8),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rdi)
jmp 17f6f54 <kernel_cholesky+0x84>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
lea (%rax,%r12,8),%rdx
movsd (%rax,%r12,8),%xmm0
xor %esi,%esi
cmp $0x1,%r12
je 17f706f <kernel_cholesky+0x19f>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%r13,%rsi,8),%xmm1
mulsd %xmm1,%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rdx)
movsd 0x0(%r13,%rsi,8),%xmm1
mulsd %xmm1,%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rdx)
add $0x2,%rsi
cmp %rsi,%rcx
jne 17f7040 <kernel_cholesky+0x170>
test $0x1,%r12b
je 17f7086 <kernel_cholesky+0x1b6>
movsd (%rax,%rsi,8),%xmm1
mulsd %xmm1,%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rdx)
ucomisd %xmm2,%xmm0
jb 17f6f00 <kernel_cholesky+0x30>
jmp 17f70ae <kernel_cholesky+0x1de>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rbx),%xmm0
ucomisd %xmm2,%xmm0
jb 17f6f00 <kernel_cholesky+0x30>
sqrtsd %xmm0,%xmm0
jmp 17f6f09 <kernel_cholesky+0x39>
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
int3
int3
int3
int3
int3
int3
<sqrt$plt>:
mov $0xd3,%r11d
jmp *0x23ada8c(%rip) # 3b83f98 <sqrt@GLIBC_2.2.5>
int3
int3
int3
int3