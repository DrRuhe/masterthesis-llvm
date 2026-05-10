<kernel_durbin>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x7d38,%rsp
lea 0x24278d8(%rip),%r8 # 3c1ee30 <g_durbin_r>
movsd (%r8),%xmm2
xorpd -0x2d81a5(%rip),%xmm2 # 151f3c0 <.LCPI0_1>
lea 0x242f5c4(%rip),%r14 # 3c26b30 <g_durbin_y>
movlpd %xmm2,(%r14)
mov %rdi,0x8(%rsp)
cmp $0x2,%edi
jl 17f77a5 <kernel_durbin+0x265>
mov 0x8(%rsp),%rax
dec %eax
mov %rax,0x8(%rsp)
lea -0x8(%r14),%r9
movsd -0x2d6a0f(%rip),%xmm4 # 1520b88 <.LCPI0_0>
mov $0x1,%ebp
xor %r15d,%r15d
movabs $0x1ffffffffffffffe,%r12
mov %r14,%rbx
mov %r8,%r13
jmp 17f7616 <kernel_durbin+0xd6>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
lea 0x8(,%r15,8),%rdx
mov %r14,%rdi
lea 0x30(%rsp),%rsi
movapd %xmm3,0x20(%rsp)
call 17d7260 <memcpy$plt>
movapd 0x20(%rsp),%xmm2
movsd %xmm2,(%r14,%rbp,8)
inc %rbp
inc %r15
add $0x8,%r13
mov 0x18(%rsp),%r9
add $0x8,%r9
add $0x8,%rbx
cmp 0x8(%rsp),%r15
lea 0x2427826(%rip),%r8 # 3c1ee30 <g_durbin_r>
movsd 0x10(%rsp),%xmm4
je 17f77a5 <kernel_durbin+0x265>
lea 0x0(,%rbp,8),%rax
add %r8,%rax
test %r15,%r15
je 17f7790 <kernel_durbin+0x250>
mov %rbp,%rdx
and $0xfffffffffffffffe,%rdx
xorpd %xmm0,%xmm0
mov %r13,%rsi
xor %ecx,%ecx
nopw 0x0(%rax,%rax,1)
movsd (%rsi),%xmm1
mulsd (%r14,%rcx,8),%xmm1
addsd %xmm0,%xmm1
mov %rcx,%rdi
xor %r12,%rdi
movsd (%rax,%rdi,8),%xmm0
mulsd 0x8(%r14,%rcx,8),%xmm0
addsd %xmm1,%xmm0
add $0x2,%rcx
add $0xfffffffffffffff0,%rsi
cmp %rcx,%rdx
jne 17f7640 <kernel_durbin+0x100>
test $0x1,%bpl
je 17f768c <kernel_durbin+0x14c>
mov %rcx,%rdx
not %rdx
movsd (%rax,%rdx,8),%xmm1
mulsd (%r14,%rcx,8),%xmm1
addsd %xmm1,%xmm0
mulsd %xmm2,%xmm2
movsd -0x2d6b10(%rip),%xmm1 # 1520b88 <.LCPI0_0>
subsd %xmm2,%xmm1
mulsd %xmm1,%xmm4
addsd (%r8,%rbp,8),%xmm0
xorpd -0x2d82ee(%rip),%xmm0 # 151f3c0 <.LCPI0_1>
movapd %xmm0,%xmm3
divsd %xmm4,%xmm3
cmp $0x4,%rbp
mov %r9,0x18(%rsp)
movsd %xmm4,0x10(%rsp)
jae 17f76d0 <kernel_durbin+0x190>
xor %eax,%eax
jmp 17f774a <kernel_durbin+0x20a>
nopl 0x0(%rax,%rax,1)
mov %rbp,%rcx
and $0xfffffffffffffffc,%rcx
mov %rbp,%rax
movabs $0x7ffffffffffffffc,%rdx
and %rdx,%rax
movapd %xmm3,%xmm0
unpcklpd %xmm3,%xmm0
mov %r9,%rdx
xor %esi,%esi
data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%rdx),%xmm1
movupd (%rdx),%xmm2
shufpd $0x1,%xmm2,%xmm2
shufpd $0x1,%xmm1,%xmm1
mulpd %xmm0,%xmm2
addpd (%r14,%rsi,8),%xmm2
mulpd %xmm0,%xmm1
addpd 0x10(%r14,%rsi,8),%xmm1
movapd %xmm2,0x30(%rsp,%rsi,8)
movapd %xmm1,0x40(%rsp,%rsi,8)
add $0x4,%rsi
add $0xffffffffffffffe0,%rdx
cmp %rsi,%rcx
jne 17f7700 <kernel_durbin+0x1c0>
cmp %rax,%rbp
je 17f75c0 <kernel_durbin+0x80>
lea 0x0(,%rax,8),%rdx
mov %rbx,%rcx
sub %rdx,%rcx
nopl 0x0(%rax,%rax,1)
movsd (%rcx),%xmm0
mulsd %xmm3,%xmm0
addsd (%r14,%rax,8),%xmm0
movsd %xmm0,0x30(%rsp,%rax,8)
inc %rax
add $0xfffffffffffffff8,%rcx
cmp %rax,%rbp
jne 17f7760 <kernel_durbin+0x220>
jmp 17f75c0 <kernel_durbin+0x80>
data16 cs nopw 0x0(%rax,%rax,1)
xorpd %xmm0,%xmm0
xor %ecx,%ecx
test $0x1,%bpl
jne 17f7677 <kernel_durbin+0x137>
jmp 17f768c <kernel_durbin+0x14c>
add $0x7d38,%rsp
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
int3
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x23ad3dc(%rip) # 3b84648 <memcpy@GLIBC_2.14>
int3
int3
int3
int3