<kernel_atax>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x18,%rsp
mov %edi,%ebp
test %esi,%esi
jle 17f134d <kernel_atax+0x25d>
mov %esi,%ebx
lea 0x23a2c3f(%rip),%rax # 3b93d50 <g_atax_A>
mov (%rax),%rax
mov %rax,(%rsp)
mov %esi,%r12d
lea 0x0(,%r12,8),%rdx
lea 0x23a70f6(%rip),%r14 # 3b98220 <g_atax_y>
mov %r14,%rdi
xor %esi,%esi
call 17d7150 <memset$plt>
test %ebp,%ebp
jle 17f1373 <kernel_atax+0x283>
mov %ebp,%ecx
lea (%r14,%r12,8),%rax
mov %rcx,0x10(%rsp)
imul $0x44c0,%rcx,%rcx
mov (%rsp),%r15
add %r15,%rcx
lea (%rcx,%r12,8),%rdx
add $0xffffffffffffbb40,%rdx
mov %r12d,%ecx
and $0x7ffffffe,%ecx
cmp %r14,%rdx
seta %sil
cmp %rax,%r15
setb %dl
and %sil,%dl
mov %r12d,%eax
and $0x7ffffffc,%eax
lea -0x1(%r12),%rsi
mov %rsi,0x8(%rsp)
lea 0x8(%r15),%r8
mov %r12d,%r9d
shr $0x2,%r9d
and $0x1fffffff,%r9d
shl $0x5,%r9
lea 0x10(%r15),%r10
xor %r11d,%r11d
lea 0x23ab531(%rip),%r13 # 3b9c6e0 <g_atax_tmp>
lea 0x23a2baa(%rip),%rbp # 3b93d60 <g_atax_x>
jmp 17f11e3 <kernel_atax+0xf3>
nopl 0x0(%rax,%rax,1)
inc %r11
add $0x44c0,%r8
add $0x44c0,%r10
add $0x44c0,%r15
cmp 0x10(%rsp),%r11
je 17f1373 <kernel_atax+0x283>
movq $0x0,0x0(%r13,%r11,8)
xorpd %xmm0,%xmm0
xor %edi,%edi
cmp $0x1,%ebx
je 17f1238 <kernel_atax+0x148>
nopw 0x0(%rax,%rax,1)
movsd -0x8(%r8,%rdi,8),%xmm1
mulsd 0x0(%rbp,%rdi,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,0x0(%r13,%r11,8)
movsd (%r8,%rdi,8),%xmm0
mulsd 0x8(%rbp,%rdi,8),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,0x0(%r13,%r11,8)
add $0x2,%rdi
cmp %rdi,%rcx
jne 17f1200 <kernel_atax+0x110>
test $0x1,%r12b
je 17f125f <kernel_atax+0x16f>
imul $0x44c0,%r11,%rsi
add (%rsp),%rsi
movsd (%rsi,%rdi,8),%xmm1
mulsd 0x0(%rbp,%rdi,8),%xmm1
addsd %xmm1,%xmm0
movsd %xmm0,0x0(%r13,%r11,8)
cmp $0x4,%ebx
setb %sil
or %dl,%sil
test $0x1,%sil
je 17f1280 <kernel_atax+0x190>
xor %esi,%esi
jmp 17f12d4 <kernel_atax+0x1e4>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movapd %xmm0,%xmm1
unpcklpd %xmm0,%xmm1
xor %esi,%esi
nopw 0x0(%rax,%rax,1)
movupd -0x10(%r10,%rsi,1),%xmm2
mulpd %xmm1,%xmm2
addpd (%r14,%rsi,1),%xmm2
movupd (%r10,%rsi,1),%xmm3
mulpd %xmm1,%xmm3
addpd 0x10(%r14,%rsi,1),%xmm3
movapd %xmm2,(%r14,%rsi,1)
movapd %xmm3,0x10(%r14,%rsi,1)
add $0x20,%rsi
cmp %rsi,%r9
jne 17f1290 <kernel_atax+0x1a0>
mov %rax,%rsi
cmp %r12d,%eax
je 17f11c0 <kernel_atax+0xd0>
mov %rsi,%rdi
test $0x1,%r12b
je 17f1304 <kernel_atax+0x214>
imul $0x44c0,%r11,%rdi
add (%rsp),%rdi
movsd (%rdi,%rsi,8),%xmm1
mulsd %xmm0,%xmm1
addsd (%r14,%rsi,8),%xmm1
movsd %xmm1,(%r14,%rsi,8)
mov %rsi,%rdi
or $0x1,%rdi
cmp 0x8(%rsp),%rsi
je 17f11c0 <kernel_atax+0xd0>
nop
movsd (%r15,%rdi,8),%xmm1
mulsd %xmm0,%xmm1
addsd (%r14,%rdi,8),%xmm1
movsd %xmm1,(%r14,%rdi,8)
movsd 0x8(%r15,%rdi,8),%xmm1
mulsd %xmm0,%xmm1
addsd 0x8(%r14,%rdi,8),%xmm1
movsd %xmm1,0x8(%r14,%rdi,8)
add $0x2,%rdi
cmp %rdi,%r12
jne 17f1310 <kernel_atax+0x220>
jmp 17f11c0 <kernel_atax+0xd0>
test %ebp,%ebp
jle 17f1373 <kernel_atax+0x283>
mov %ebp,%edx
shl $0x3,%rdx
lea 0x23ab382(%rip),%rdi # 3b9c6e0 <g_atax_tmp>
xor %esi,%esi
add $0x18,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
jmp 17d7150 <memset$plt>
add $0x18,%rsp
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
int3
int3
int3
int3
int3
<memset$plt>:
mov $0x198,%r11d
jmp *0x23ad464(%rip) # 3b845c0 <memset@GLIBC_2.2.5>
int3
int3
int3
int3