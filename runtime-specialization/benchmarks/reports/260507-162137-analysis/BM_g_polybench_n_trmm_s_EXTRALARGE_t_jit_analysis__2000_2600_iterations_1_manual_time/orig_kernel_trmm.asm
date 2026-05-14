<kernel_trmm>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
test %edi,%edi
setle %al
mov %esi,-0x4c(%rsp)
test %esi,%esi
setle %cl
or %al,%cl
jne 17f68fd <kernel_trmm+0x26d>
lea 0x2428759(%rip),%rax # 3c1ee10 <g_trmm_A>
mov (%rax),%rcx
lea 0x2428757(%rip),%rax # 3c1ee18 <g_trmm_B>
mov (%rax),%r12
mov %edi,%edx
mov -0x4c(%rsp),%eax
mov %eax,%edi
lea -0x2(%rdx),%r8
mov %edi,%esi
and $0x7ffffffc,%esi
mov %rsi,-0x30(%rsp)
shr $0x2,%eax
and $0x1fffffff,%eax
shl $0x5,%rax
mov %rax,-0x38(%rsp)
lea 0x10(%r12),%r11
mov %rcx,-0x20(%rsp)
lea 0x3e80(%rcx),%rax
mov %rax,-0x48(%rsp)
mov $0x1,%r14d
xor %r13d,%r13d
movsd -0x2d5f0d(%rip),%xmm0 # 1520808 <.LCPI107_0>
movapd -0x2d702d(%rip),%xmm1 # 151f6f0 <.LCPI143_2>
mov %r12,-0x28(%rsp)
mov %rdx,-0x40(%rsp)
jmp 17f675a <kernel_trmm+0xca>
nopl 0x0(%rax)
inc %r14
add $0x5140,%r11
add $0x5140,%r12
addq $0x8,-0x48(%rsp)
mov -0x40(%rsp),%rdx
mov -0x18(%rsp),%r13
cmp %rdx,%r13
je 17f68fd <kernel_trmm+0x26d>
lea 0x1(%r13),%rax
mov %rax,-0x18(%rsp)
cmp %rdx,%rax
jae 17f6880 <kernel_trmm+0x1f0>
mov %r13,%rbp
sub %rdx,%rbp
imul $0x3e80,%r14,%rax
add -0x20(%rsp),%rax
lea (%rax,%r13,8),%rax
mov %rax,-0x8(%rsp)
lea 0x1(%r14),%rax
mov %rax,-0x10(%rsp)
imul $0x5140,%r13,%rcx
mov -0x28(%rsp),%r15
add %r15,%rcx
imul $0x5140,%r14,%rsi
add %r15,%rsi
xor %r10d,%r10d
jmp 17f67c8 <kernel_trmm+0x138>
xchg %ax,%ax
mulsd %xmm0,%xmm2
movsd %xmm2,(%rdx)
inc %r10
add $0x8,%r15
cmp %rdi,%r10
je 17f6730 <kernel_trmm+0xa0>
lea (%rcx,%r10,8),%rdx
movsd (%rcx,%r10,8),%xmm3
test $0x1,%bpl
jne 17f6800 <kernel_trmm+0x170>
mov -0x8(%rsp),%rax
movsd (%rax),%xmm2
mulsd (%rsi,%r10,8),%xmm2
addsd %xmm3,%xmm2
movsd %xmm2,(%rdx)
mov -0x10(%rsp),%r9
movapd %xmm2,%xmm3
cmp %r13,%r8
je 17f67b0 <kernel_trmm+0x120>
jmp 17f6808 <kernel_trmm+0x178>
nop
mov %r14,%r9
cmp %r13,%r8
je 17f67b0 <kernel_trmm+0x120>
mov -0x40(%rsp),%rbx
sub %r9,%rbx
imul $0x5140,%r9,%rax
add %r15,%rax
imul $0x3e80,%r9,%r9
add -0x48(%rsp),%r9
movapd %xmm3,%xmm2
nopw 0x0(%rax,%rax,1)
movsd -0x3e80(%r9),%xmm3
mulsd (%rax),%xmm3
addsd %xmm2,%xmm3
movsd %xmm3,(%rdx)
movsd (%r9),%xmm2
mulsd 0x5140(%rax),%xmm2
addsd %xmm3,%xmm2
movsd %xmm2,(%rdx)
add $0xa280,%rax
add $0x7d00,%r9
add $0xfffffffffffffffe,%rbx
jne 17f6830 <kernel_trmm+0x1a0>
jmp 17f67b0 <kernel_trmm+0x120>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
cmpl $0x4,-0x4c(%rsp)
jae 17f6890 <kernel_trmm+0x200>
xor %eax,%eax
jmp 17f68e0 <kernel_trmm+0x250>
nopl 0x0(%rax,%rax,1)
xor %eax,%eax
mov -0x38(%rsp),%rcx
nopw 0x0(%rax,%rax,1)
movupd -0x10(%r11,%rax,1),%xmm2
movupd (%r11,%rax,1),%xmm3
mulpd %xmm1,%xmm2
mulpd %xmm1,%xmm3
movupd %xmm2,-0x10(%r11,%rax,1)
movupd %xmm3,(%r11,%rax,1)
add $0x20,%rax
cmp %rax,%rcx
jne 17f68a0 <kernel_trmm+0x210>
mov -0x30(%rsp),%rcx
mov %rcx,%rax
cmp %edi,%ecx
je 17f6730 <kernel_trmm+0xa0>
nopl 0x0(%rax,%rax,1)
movsd (%r12,%rax,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r12,%rax,8)
inc %rax
cmp %rax,%rdi
jne 17f68e0 <kernel_trmm+0x250>
jmp 17f6730 <kernel_trmm+0xa0>
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