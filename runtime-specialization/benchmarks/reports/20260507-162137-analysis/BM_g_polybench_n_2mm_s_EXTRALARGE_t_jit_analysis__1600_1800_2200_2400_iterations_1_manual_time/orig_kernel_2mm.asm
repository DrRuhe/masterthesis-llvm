<kernel_2mm>:
test %edi,%edi
jle 17efd48 <kernel_2mm+0x4a8>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x38,%rsp
mov %esi,%ebx
mov %edi,0x8(%rsp)
mov %ecx,0x1c(%rsp)
lea 0x23a4429(%rip),%rax # 3b93cf0 <g_2mm_tmp>
mov (%rax),%rax
mov %rax,0x10(%rsp)
lea 0x23a4432(%rip),%rax # 3b93d08 <g_2mm_C>
mov (%rax),%rax
mov %rax,0x28(%rsp)
lea 0x23a442b(%rip),%rax # 3b93d10 <g_2mm_D>
mov (%rax),%rbp
test %esi,%esi
jle 17efc7a <kernel_2mm+0x3da>
test %edx,%edx
jle 17efb69 <kernel_2mm+0x2c9>
mov %rbp,0x30(%rsp)
lea 0x23a43f4(%rip),%rax # 3b93cf8 <g_2mm_A>
mov (%rax),%rcx
lea 0x23a43f2(%rip),%rax # 3b93d00 <g_2mm_B>
mov (%rax),%r13
mov 0x8(%rsp),%eax
mov %rax,0x8(%rsp)
mov %ebx,%edi
mov %edx,%r8d
mov %r8d,%r9d
and $0x7ffffffe,%r9d
mov %rcx,0x20(%rsp)
lea 0x8(%rcx),%r10
xor %r11d,%r11d
movsd -0x2cf135(%rip),%xmm0 # 1520808 <.LCPI107_0>
jmp 17ef955 <kernel_2mm+0xb5>
nop
inc %r11
add $0x44c0,%r10
cmp 0x8(%rsp),%r11
je 17efa34 <kernel_2mm+0x194>
imul $0x3840,%r11,%r15
add 0x10(%rsp),%r15
imul $0x44c0,%r11,%r12
add 0x20(%rsp),%r12
mov %r13,%rsi
xor %ecx,%ecx
jmp 17ef98c <kernel_2mm+0xec>
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %rcx
add $0x8,%rsi
cmp %rdi,%rcx
je 17ef940 <kernel_2mm+0xa0>
lea (%r15,%rcx,8),%rax
movq $0x0,(%r15,%rcx,8)
xorpd %xmm1,%xmm1
cmp $0x1,%edx
jne 17ef9b0 <kernel_2mm+0x110>
xor %ebp,%ebp
jmp 17efa03 <kernel_2mm+0x163>
data16 cs nopw 0x0(%rax,%rax,1)
mov %rsi,%r14
xor %ebp,%ebp
data16 cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%r10,%rbp,8),%xmm2
mulsd %xmm0,%xmm2
mulsd (%r14),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%rax)
movsd (%r10,%rbp,8),%xmm1
mulsd %xmm0,%xmm1
mulsd 0x3840(%r14),%xmm1
addsd %xmm2,%xmm1
movsd %xmm1,(%rax)
add $0x2,%rbp
add $0x7080,%r14
cmp %rbp,%r9
jne 17ef9c0 <kernel_2mm+0x120>
test $0x1,%r8b
je 17ef980 <kernel_2mm+0xe0>
movsd (%r12,%rbp,8),%xmm2
mulsd %xmm0,%xmm2
imul $0x3840,%rbp,%r14
add %r13,%r14
mulsd (%r14,%rcx,8),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%rax)
jmp 17ef980 <kernel_2mm+0xe0>
mov 0x1c(%rsp),%r9d
test %r9d,%r9d
mov 0x30(%rsp),%rbp
mov 0x28(%rsp),%r13
jle 17efd3a <kernel_2mm+0x49a>
mov %r9d,%ecx
test %ebx,%ebx
jle 17efca3 <kernel_2mm+0x403>
mov %ebx,%edx
mov %edx,%esi
and $0x7ffffffe,%esi
mov 0x10(%rsp),%rax
lea 0x8(%rax),%rdi
xor %r8d,%r8d
movsd -0x2cf0f5(%rip),%xmm0 # 1520980 <.LCPI107_2>
jmp 17efa95 <kernel_2mm+0x1f5>
nopw 0x0(%rax,%rax,1)
inc %r8
add $0x3840,%rdi
cmp 0x8(%rsp),%r8
je 17efd3a <kernel_2mm+0x49a>
imul $0x4b00,%r8,%r9
add %rbp,%r9
imul $0x3840,%r8,%r10
add 0x10(%rsp),%r10
mov %r13,%r11
xor %r14d,%r14d
jmp 17efacc <kernel_2mm+0x22c>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r14
add $0x8,%r11
cmp %rcx,%r14
je 17efa80 <kernel_2mm+0x1e0>
lea (%r9,%r14,8),%rax
movsd (%r9,%r14,8),%xmm1
mulsd %xmm0,%xmm1
movsd %xmm1,(%r9,%r14,8)
cmp $0x1,%ebx
jne 17efaf0 <kernel_2mm+0x250>
xor %r15d,%r15d
jmp 17efb3d <kernel_2mm+0x29d>
nopw 0x0(%rax,%rax,1)
mov %r11,%r12
xor %r15d,%r15d
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%rdi,%r15,8),%xmm2
mulsd (%r12),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%rax)
movsd (%rdi,%r15,8),%xmm1
mulsd 0x4b00(%r12),%xmm1
addsd %xmm2,%xmm1
movsd %xmm1,(%rax)
add $0x2,%r15
add $0x9600,%r12
cmp %r15,%rsi
jne 17efb00 <kernel_2mm+0x260>
test $0x1,%dl
je 17efac0 <kernel_2mm+0x220>
movsd (%r10,%r15,8),%xmm2
imul $0x4b00,%r15,%r15
add %r13,%r15
mulsd (%r15,%r14,8),%xmm2
addsd %xmm1,%xmm2
movsd %xmm2,(%rax)
jmp 17efac0 <kernel_2mm+0x220>
mov %ebx,%r15d
shl $0x3,%r15
mov 0x8(%rsp),%eax
mov %eax,%ecx
mov %rcx,%r13
and $0x7,%ecx
mov %rcx,0x20(%rsp)
cmp $0x8,%eax
jae 17efb8e <kernel_2mm+0x2ee>
xor %r14d,%r14d
jmp 17efc3f <kernel_2mm+0x39f>
and $0x7ffffff8,%r13d
xor %r14d,%r14d
mov 0x10(%rsp),%r12
nopl (%rax)
mov %r12,%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x3840(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x7080(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0xa8c0(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0xe100(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x11940(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x15180(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x189c0(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
add $0x8,%r14
add $0x1c200,%r12
cmp %r14,%r13
jne 17efba0 <kernel_2mm+0x300>
mov 0x20(%rsp),%rax
test %rax,%rax
je 17efc7a <kernel_2mm+0x3da>
imul $0x3840,%r14,%r14
add 0x10(%rsp),%r14
imul $0x3840,%rax,%r12
xor %r13d,%r13d
nop
lea (%r14,%r13,1),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
add $0x3840,%r13
cmp %r13,%r12
jne 17efc60 <kernel_2mm+0x3c0>
mov 0x1c(%rsp),%r9d
test %r9d,%r9d
mov 0x28(%rsp),%r13
mov 0x8(%rsp),%eax
jle 17efd3a <kernel_2mm+0x49a>
mov %eax,%eax
mov %rax,0x8(%rsp)
mov %r9d,%ecx
test %ebx,%ebx
jg 17efa57 <kernel_2mm+0x1b7>
mov %ecx,%edx
and $0xfffffffc,%edx
lea 0x0(,%rcx,8),%rsi
and $0xffffffffffffffe0,%rsi
lea 0x10(%rbp),%rdi
xor %r8d,%r8d
movsd -0x2cf343(%rip),%xmm0 # 1520980 <.LCPI107_2>
movapd -0x2d115b(%rip),%xmm1 # 151eb70 <.LCPI107_1>
jmp 17efce8 <kernel_2mm+0x448>
nopl (%rax)
inc %r8
add $0x4b00,%rdi
add $0x4b00,%rbp
cmp 0x8(%rsp),%r8
je 17efd3a <kernel_2mm+0x49a>
xor %eax,%eax
cmp $0x4,%r9d
jb 17efd20 <kernel_2mm+0x480>
movupd -0x10(%rdi,%rax,1),%xmm2
movupd (%rdi,%rax,1),%xmm3
mulpd %xmm1,%xmm2
mulpd %xmm1,%xmm3
movupd %xmm2,-0x10(%rdi,%rax,1)
movupd %xmm3,(%rdi,%rax,1)
add $0x20,%rax
cmp %rax,%rsi
jne 17efcf0 <kernel_2mm+0x450>
mov %rdx,%rax
cmp %ecx,%edx
je 17efcd0 <kernel_2mm+0x430>
xchg %ax,%ax
movsd 0x0(%rbp,%rax,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,0x0(%rbp,%rax,8)
inc %rax
cmp %rax,%rcx
jne 17efd20 <kernel_2mm+0x480>
jmp 17efcd0 <kernel_2mm+0x430>
add $0x38,%rsp
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
<memset$plt>:
mov $0x198,%r11d
jmp *0x23ad464(%rip) # 3b845c0 <memset@GLIBC_2.2.5>
int3
int3
int3
int3