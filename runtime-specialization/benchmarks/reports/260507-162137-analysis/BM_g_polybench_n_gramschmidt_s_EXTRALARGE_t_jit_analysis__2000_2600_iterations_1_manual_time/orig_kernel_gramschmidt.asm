<kernel_gramschmidt>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0xa8,%rsp
test %esi,%esi
jle 17f8408 <kernel_gramschmidt+0x678>
mov %edi,%ebx
lea 0x2436a86(%rip),%rax # 3c2e838 <g_gramschmidt_R>
mov (%rax),%rdi
mov %esi,%ecx
test %ebx,%ebx
mov %rcx,0x10(%rsp)
jle 17f83a4 <kernel_gramschmidt+0x614>
lea 0x2436a65(%rip),%rax # 3c2e830 <g_gramschmidt_A>
mov (%rax),%r13
lea 0x2436a6b(%rip),%rax # 3c2e840 <g_gramschmidt_Q>
mov (%rax),%r14
mov %ebx,%r15d
imul $0x5140,%r15,%rax
mov 0x10(%rsp),%rcx
lea (%rax,%rcx,8),%rcx
lea (%rcx,%r13,1),%rsi
add $0xffffffffffffaec0,%rsi
add %r14,%rcx
add $0xffffffffffffaec0,%rcx
imul $0x5148,0x10(%rsp),%rdx
add %rdi,%rdx
add $0xffffffffffffaec0,%rdx
lea 0x8(%r13),%r8
lea -0x5138(%r14,%rax,1),%rax
mov %rax,0x70(%rsp)
lea 0x8(%rdi),%rax
mov %rax,0x68(%rsp)
mov 0x10(%rsp),%rax
lea (%rdi,%rax,8),%rax
mov %rax,0x60(%rsp)
mov %r15d,%r9d
and $0x3,%r9d
mov %r15d,%eax
and $0x7ffffffc,%eax
mov %rax,0x58(%rsp)
mov %rsi,0x80(%rsp)
cmp %rsi,%r14
setb %al
cmp %rcx,%r13
setb %sil
and %al,%sil
cmp %rdx,%r14
setb %al
mov %rdi,0x98(%rsp)
cmp %rcx,%rdi
setb %cl
and %al,%cl
or %sil,%cl
mov %cl,0xf(%rsp)
mov %r15d,%ebp
and $0x7ffffffe,%ebp
lea -0x1(%r15),%rax
mov %rax,0x50(%rsp)
mov %r9,0x90(%rsp)
imul $0x5140,%r9,%rsi
mov %r15d,%eax
shr $1,%eax
and $0x3fffffff,%eax
imul $0xa280,%rax,%r12
xorpd %xmm3,%xmm3
lea 0x5148(%r13),%rax
mov %rax,0x48(%rsp)
mov $0x1,%eax
mov %rax,0x40(%rsp)
movq $0x0,0x18(%rsp)
mov %r8,0x78(%rsp)
mov %r8,0x38(%rsp)
mov %r14,0x28(%rsp)
mov %r13,0x30(%rsp)
mov %rsi,0x88(%rsp)
mov %ebx,0x24(%rsp)
jmp 17f7f37 <kernel_gramschmidt+0x1a7>
nopl 0x0(%rax,%rax,1)
incq 0x40(%rsp)
add $0x8,%r13
add $0x8,%r14
addq $0x8,0x38(%rsp)
addq $0x8,0x48(%rsp)
mov 0xa0(%rsp),%rdx
mov %rdx,0x18(%rsp)
cmp %rcx,%rdx
mov 0x88(%rsp),%rsi
je 17f8408 <kernel_gramschmidt+0x678>
cmp $0x4,%ebx
jae 17f7f50 <kernel_gramschmidt+0x1c0>
xor %eax,%eax
xorpd %xmm0,%xmm0
jmp 17f7fac <kernel_gramschmidt+0x21c>
data16 data16 cs nopw 0x0(%rax,%rax,1)
xorpd %xmm0,%xmm0
mov %r13,%rcx
xor %eax,%eax
mov 0x58(%rsp),%rdx
xchg %ax,%ax
movsd (%rcx),%xmm1
mulsd %xmm1,%xmm1
addsd %xmm0,%xmm1
movsd 0x5140(%rcx),%xmm0
mulsd %xmm0,%xmm0
addsd %xmm1,%xmm0
movsd 0xa280(%rcx),%xmm1
mulsd %xmm1,%xmm1
addsd %xmm0,%xmm1
movsd 0xf3c0(%rcx),%xmm0
mulsd %xmm0,%xmm0
addsd %xmm1,%xmm0
add $0x4,%rax
add $0x14500,%rcx
cmp %rax,%rdx
jne 17f7f60 <kernel_gramschmidt+0x1d0>
cmpq $0x0,0x90(%rsp)
je 17f7fe9 <kernel_gramschmidt+0x259>
imul $0x5140,%rax,%rax
add %r13,%rax
xor %ecx,%ecx
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rax,%rcx,1),%xmm1
mulsd %xmm1,%xmm1
addsd %xmm1,%xmm0
add $0x5140,%rcx
cmp %rcx,%rsi
jne 17f7fd0 <kernel_gramschmidt+0x240>
ucomisd %xmm3,%xmm0
jb 17f8000 <kernel_gramschmidt+0x270>
sqrtsd %xmm0,%xmm0
jmp 17f8009 <kernel_gramschmidt+0x279>
data16 cs nopw 0x0(%rax,%rax,1)
call 17d6500 <sqrt$plt>
xorpd %xmm3,%xmm3
mov 0x18(%rsp),%rdi
imul $0x5140,%rdi,%rdx
cmp $0x1,%ebx
sete %sil
mov 0x98(%rsp),%rax
add %rdx,%rax
lea (%rax,%rdi,8),%rcx
movsd %xmm0,(%rax,%rdi,8)
or 0xf(%rsp),%sil
test $0x1,%sil
je 17f8040 <kernel_gramschmidt+0x2b0>
xor %esi,%esi
jmp 17f808e <kernel_gramschmidt+0x2fe>
nop
movsd (%rcx),%xmm0
unpcklpd %xmm0,%xmm0
mov %rbp,%rsi
xor %edi,%edi
nopl (%rax)
movsd 0x0(%r13,%rdi,1),%xmm1
movhpd 0x5140(%r13,%rdi,1),%xmm1
divpd %xmm0,%xmm1
movlpd %xmm1,(%r14,%rdi,1)
movhpd %xmm1,0x5140(%r14,%rdi,1)
add $0xa280,%rdi
add $0xfffffffffffffffe,%rsi
jne 17f8050 <kernel_gramschmidt+0x2c0>
mov %rbp,%rsi
cmp %r15d,%ebp
je 17f8116 <kernel_gramschmidt+0x386>
mov %rsi,%rdi
test $0x1,%r15b
je 17f80c7 <kernel_gramschmidt+0x337>
imul $0x5140,%rsi,%rdi
mov 0x30(%rsp),%r8
add %rdi,%r8
mov 0x18(%rsp),%r9
movsd (%r8,%r9,8),%xmm0
divsd (%rcx),%xmm0
add 0x28(%rsp),%rdi
movsd %xmm0,(%rdi,%r9,8)
mov %rsi,%rdi
or $0x1,%rdi
cmp 0x50(%rsp),%rsi
je 17f8116 <kernel_gramschmidt+0x386>
mov %r15,%rsi
sub %rdi,%rsi
imul $0x5140,%rdi,%rdi
nopl 0x0(%rax,%rax,1)
movsd 0x0(%r13,%rdi,1),%xmm0
divsd (%rcx),%xmm0
movsd %xmm0,(%r14,%rdi,1)
movsd 0x5140(%r13,%rdi,1),%xmm0
divsd (%rcx),%xmm0
movsd %xmm0,0x5140(%r14,%rdi,1)
add $0xa280,%rdi
add $0xfffffffffffffffe,%rsi
jne 17f80e0 <kernel_gramschmidt+0x350>
mov 0x18(%rsp),%rcx
lea 0x1(%rcx),%rsi
mov 0x10(%rsp),%rcx
mov %rsi,0xa0(%rsp)
cmp %rcx,%rsi
jae 17f7f00 <kernel_gramschmidt+0x170>
mov 0x18(%rsp),%r10
imul $0x5148,%r10,%rsi
add 0x68(%rsp),%rsi
add 0x60(%rsp),%rdx
mov 0x78(%rsp),%rdi
lea (%rdi,%r10,8),%rdi
mov 0x28(%rsp),%r8
lea (%r8,%r10,8),%r8
mov 0x70(%rsp),%r9
lea (%r9,%r10,8),%r9
cmp %r9,%rdi
setb %r9b
mov 0x80(%rsp),%r10
cmp %r10,%r8
setb %r8b
and %r9b,%r8b
cmp %rdx,%rdi
setb %dil
cmp %r10,%rsi
setb %dl
and %dil,%dl
or %r8b,%dl
mov 0x48(%rsp),%rsi
mov 0x38(%rsp),%rdi
mov 0x40(%rsp),%r8
jmp 17f81c4 <kernel_gramschmidt+0x434>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r8
add $0x8,%rdi
add $0x8,%rsi
cmp %rcx,%r8
je 17f7f00 <kernel_gramschmidt+0x170>
lea (%rax,%r8,8),%r9
movq $0x0,(%rax,%r8,8)
xorpd %xmm0,%xmm0
cmp $0x1,%ebx
jne 17f81e0 <kernel_gramschmidt+0x450>
xor %r10d,%r10d
jmp 17f8232 <kernel_gramschmidt+0x4a2>
xchg %ax,%ax
mov $0x5140,%r11d
xor %r10d,%r10d
nopl 0x0(%rax)
movsd -0x5140(%r14,%r11,1),%xmm1
mulsd -0x5140(%rdi,%r11,1),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r9)
movsd (%r14,%r11,1),%xmm0
mulsd (%rdi,%r11,1),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r9)
add $0x2,%r10
add $0xa280,%r11
cmp %r10,%rbp
jne 17f81f0 <kernel_gramschmidt+0x460>
test $0x1,%r15b
je 17f826a <kernel_gramschmidt+0x4da>
imul $0x5140,%r10,%r10
mov 0x28(%rsp),%r11
add %r10,%r11
mov 0x18(%rsp),%rbx
movsd (%r11,%rbx,8),%xmm1
mov 0x24(%rsp),%ebx
add 0x30(%rsp),%r10
mulsd (%r10,%r8,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r9)
cmp $0x6,%ebx
setb %r10b
or %dl,%r10b
test $0x1,%r10b
je 17f8280 <kernel_gramschmidt+0x4f0>
xor %r10d,%r10d
jmp 17f82e0 <kernel_gramschmidt+0x550>
nop
movsd (%r9),%xmm0
unpcklpd %xmm0,%xmm0
xor %r10d,%r10d
nopl 0x0(%rax)
movsd (%rdi,%r10,1),%xmm1
movhpd 0x5140(%rdi,%r10,1),%xmm1
movsd (%r14,%r10,1),%xmm2
movhpd 0x5140(%r14,%r10,1),%xmm2
mulpd %xmm0,%xmm2
subpd %xmm2,%xmm1
movlpd %xmm1,(%rdi,%r10,1)
movhpd %xmm1,0x5140(%rdi,%r10,1)
add $0xa280,%r10
cmp %r10,%r12
jne 17f8290 <kernel_gramschmidt+0x500>
mov %rbp,%r10
cmp %r15d,%ebp
je 17f81b0 <kernel_gramschmidt+0x420>
mov %r10,%r11
test $0x1,%r15b
je 17f832d <kernel_gramschmidt+0x59d>
imul $0x5140,%r10,%r11
mov 0x30(%rsp),%rcx
add %r11,%rcx
movsd (%rcx,%r8,8),%xmm0
add 0x28(%rsp),%r11
mov 0x18(%rsp),%rbx
movsd (%r11,%rbx,8),%xmm1
mov 0x24(%rsp),%ebx
mulsd (%r9),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rcx,%r8,8)
mov 0x10(%rsp),%rcx
mov %r10,%r11
or $0x1,%r11
cmp 0x50(%rsp),%r10
je 17f81b0 <kernel_gramschmidt+0x420>
mov %r15,%r10
sub %r11,%r10
imul $0x5140,%r11,%r11
data16 cs nopw 0x0(%rax,%rax,1)
movsd -0x5140(%rsi,%r11,1),%xmm0
movsd (%r14,%r11,1),%xmm1
mulsd (%r9),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,-0x5140(%rsi,%r11,1)
movsd (%rsi,%r11,1),%xmm0
movsd 0x5140(%r14,%r11,1),%xmm1
mulsd (%r9),%xmm1
subsd %xmm1,%xmm0
movsd %xmm0,(%rsi,%r11,1)
add $0xa280,%r11
add $0xfffffffffffffffe,%r10
jne 17f8350 <kernel_gramschmidt+0x5c0>
jmp 17f81b0 <kernel_gramschmidt+0x420>
cmp $0x1,%esi
jne 17f841a <kernel_gramschmidt+0x68a>
mov %rdi,%r13
xor %r15d,%r15d
test $0x1,%cl
je 17f8408 <kernel_gramschmidt+0x678>
imul $0x5140,%r15,%rax
add %r13,%rax
movq $0x0,(%rax,%r15,8)
lea 0x1(%r15),%rax
cmp %rcx,%rax
jae 17f8408 <kernel_gramschmidt+0x678>
lea -0x8(,%rcx,8),%rdx
add $0x8,%r13
lea 0x0(,%r15,8),%rax
sub %rax,%rdx
imul $0x5148,%r15,%rdi
add %r13,%rdi
xor %esi,%esi
add $0xa8,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
jmp 17d7150 <memset$plt>
add $0xa8,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
lea 0x0(,%rcx,8),%rbx
mov %ecx,%r12d
and $0x7ffffffe,%r12d
mov %rdi,%r13
lea 0x5150(%rdi),%r14
add $0xfffffffffffffff0,%rbx
xor %r15d,%r15d
jmp 17f8454 <kernel_gramschmidt+0x6c4>
nop
add $0xa290,%r14
add $0xfffffffffffffff0,%rbx
cmp %r15,%r12
je 17f83af <kernel_gramschmidt+0x61f>
movq $0x0,-0x5150(%r14)
lea 0x1(%r15),%rax
cmp %rcx,%rax
jae 17f847f <kernel_gramschmidt+0x6ef>
lea 0x8(%rbx),%rdx
lea -0x5148(%r14),%rdi
xor %esi,%esi
call 17d7150 <memset$plt>
mov 0x10(%rsp),%rcx
movq $0x0,-0x8(%r14)
add $0x2,%r15
cmp %rcx,%r15
jae 17f8440 <kernel_gramschmidt+0x6b0>
mov %r14,%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
mov 0x10(%rsp),%rcx
jmp 17f8440 <kernel_gramschmidt+0x6b0>
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
<sqrt$plt>:
mov $0xd3,%r11d
jmp *0x23ada8c(%rip) # 3b83f98 <sqrt@GLIBC_2.2.5>
int3
int3
int3
int3