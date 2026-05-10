<kernel_3mm>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x48,%rsp
mov %r8d,%r15d
mov %ecx,0x4(%rsp)
mov %rsi,0x20(%rsp)
lea 0x23a37f7(%rip),%rax # 3b93d18 <g_3mm_E>
mov (%rax),%rax
mov %rax,0x8(%rsp)
lea 0x23a3800(%rip),%rax # 3b93d30 <g_3mm_F>
mov (%rax),%rax
mov %rax,0x18(%rsp)
lea 0x23a37f9(%rip),%rax # 3b93d38 <g_3mm_C>
mov (%rax),%rax
mov %rax,0x30(%rsp)
lea 0x23a37f2(%rip),%rax # 3b93d40 <g_3mm_D>
mov (%rax),%rax
mov %rax,0x10(%rsp)
lea 0x23a37eb(%rip),%rax # 3b93d48 <g_3mm_G>
mov (%rax),%rax
mov %rax,0x28(%rsp)
mov %edi,(%rsp)
test %edi,%edi
jle 17f06a9 <kernel_3mm+0x1a9>
mov 0x20(%rsp),%rcx
test %ecx,%ecx
jle 17f07dd <kernel_3mm+0x2dd>
test %edx,%edx
jle 17f0829 <kernel_3mm+0x329>
lea 0x23a3794(%rip),%rax # 3b93d20 <g_3mm_A>
mov (%rax),%rsi
lea 0x23a3792(%rip),%rax # 3b93d28 <g_3mm_B>
mov (%rax),%rbx
mov (%rsp),%eax
mov %rax,0x38(%rsp)
mov %ecx,%edi
mov %edx,%r8d
mov %r8d,%r9d
and $0x7ffffffe,%r9d
mov %rsi,0x40(%rsp)
lea 0x8(%rsi),%r10
xor %r11d,%r11d
jmp 17f05d5 <kernel_3mm+0xd5>
xchg %ax,%ax
inc %r11
add $0x3e80,%r10
cmp 0x38(%rsp),%r11
je 17f06a9 <kernel_3mm+0x1a9>
imul $0x3840,%r11,%r12
add 0x8(%rsp),%r12
imul $0x3e80,%r11,%r13
add 0x40(%rsp),%r13
mov %rbx,%rsi
xor %eax,%eax
jmp 17f060c <kernel_3mm+0x10c>
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %rax
add $0x8,%rsi
cmp %rdi,%rax
je 17f05c0 <kernel_3mm+0xc0>
lea (%r12,%rax,8),%rbp
movq $0x0,(%r12,%rax,8)
xorpd %xmm0,%xmm0
cmp $0x1,%edx
jne 17f0630 <kernel_3mm+0x130>
xor %r14d,%r14d
jmp 17f067b <kernel_3mm+0x17b>
cs nopw 0x0(%rax,%rax,1)
mov %rsi,%rcx
xor %r14d,%r14d
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%r10,%r14,8),%xmm1
mulsd (%rcx),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,0x0(%rbp)
movsd (%r10,%r14,8),%xmm0
mulsd 0x3840(%rcx),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,0x0(%rbp)
add $0x2,%r14
add $0x7080,%rcx
cmp %r14,%r9
jne 17f0640 <kernel_3mm+0x140>
test $0x1,%r8b
je 17f0600 <kernel_3mm+0x100>
movsd 0x0(%r13,%r14,8),%xmm1
imul $0x3840,%r14,%rcx
add %rbx,%rcx
mulsd (%rcx,%rax,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,0x0(%rbp)
jmp 17f0600 <kernel_3mm+0x100>
cmpl $0x0,0x20(%rsp)
mov 0x28(%rsp),%rbp
jle 17f07ef <kernel_3mm+0x2ef>
mov 0x4(%rsp),%ecx
test %ecx,%ecx
mov 0x20(%rsp),%rax
jle 17f0c6b <kernel_3mm+0x76b>
mov %eax,%r13d
test %r15d,%r15d
jle 17f093f <kernel_3mm+0x43f>
mov %ecx,%eax
mov %r15d,%ecx
mov %ecx,%edx
and $0x7ffffffe,%edx
mov 0x30(%rsp),%rsi
add $0x8,%rsi
xor %edi,%edi
jmp 17f0703 <kernel_3mm+0x203>
inc %rdi
add $0x4b00,%rsi
cmp %r13,%rdi
je 17f0b4a <kernel_3mm+0x64a>
imul $0x44c0,%rdi,%r8
add 0x18(%rsp),%r8
imul $0x4b00,%rdi,%r9
add 0x30(%rsp),%r9
mov 0x10(%rsp),%r10
xor %r11d,%r11d
jmp 17f073c <kernel_3mm+0x23c>
data16 cs nopw 0x0(%rax,%rax,1)
inc %r11
add $0x8,%r10
cmp %rax,%r11
je 17f06f0 <kernel_3mm+0x1f0>
lea (%r8,%r11,8),%r12
movq $0x0,(%r8,%r11,8)
xorpd %xmm0,%xmm0
cmp $0x1,%r15d
jne 17f0760 <kernel_3mm+0x260>
xor %r14d,%r14d
jmp 17f07ad <kernel_3mm+0x2ad>
nopw 0x0(%rax,%rax,1)
mov %r10,%rbx
xor %r14d,%r14d
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%rsi,%r14,8),%xmm1
mulsd (%rbx),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r12)
movsd (%rsi,%r14,8),%xmm0
mulsd 0x44c0(%rbx),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r12)
add $0x2,%r14
add $0x8980,%rbx
cmp %r14,%rdx
jne 17f0770 <kernel_3mm+0x270>
test $0x1,%cl
je 17f0730 <kernel_3mm+0x230>
movsd (%r9,%r14,8),%xmm1
imul $0x44c0,%r14,%rbx
add 0x10(%rsp),%rbx
mulsd (%rbx,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r12)
jmp 17f0730 <kernel_3mm+0x230>
mov 0x4(%rsp),%edx
test %edx,%edx
mov 0x28(%rsp),%rbp
jg 17f0807 <kernel_3mm+0x307>
jmp 17f0c6b <kernel_3mm+0x76b>
cmpl $0x0,(%rsp)
setg %al
mov 0x4(%rsp),%edx
test %edx,%edx
setg %cl
test %cl,%al
je 17f0c6b <kernel_3mm+0x76b>
mov %edx,%ebx
shl $0x3,%rbx
mov (%rsp),%eax
mov %eax,%r12d
mov %r12d,%r15d
and $0x7,%r15d
dec %eax
cmp $0x7,%eax
jae 17f084c <kernel_3mm+0x34c>
xor %r13d,%r13d
jmp 17f08f8 <kernel_3mm+0x3f8>
mov %ecx,%ebx
shl $0x3,%rbx
mov (%rsp),%eax
mov %eax,%ebp
mov %ebp,%r14d
and $0x7,%r14d
cmp $0x8,%eax
jae 17f0964 <kernel_3mm+0x464>
xor %r12d,%r12d
jmp 17f0a18 <kernel_3mm+0x518>
and $0x7ffffff8,%r12d
xor %r13d,%r13d
mov %rbp,%r14
nopl 0x0(%rax)
mov %r14,%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x44c0(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x8980(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0xce40(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x11300(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x157c0(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x19c80(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x1e140(%r14),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
add $0x8,%r13
add $0x22600,%r14
cmp %r13,%r12
jne 17f0860 <kernel_3mm+0x360>
test %r15,%r15
je 17f0c6b <kernel_3mm+0x76b>
imul $0x44c0,%r13,%rax
add %rax,%rbp
imul $0x44c0,%r15,%r14
xor %r15d,%r15d
data16 cs nopw 0x0(%rax,%rax,1)
lea (%r15,%rbp,1),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
add $0x44c0,%r15
cmp %r15,%r14
jne 17f0920 <kernel_3mm+0x420>
jmp 17f0c6b <kernel_3mm+0x76b>
mov %ecx,%r15d
shl $0x3,%r15
mov %r13d,%ecx
and $0x7,%ecx
mov %rcx,0x10(%rsp)
dec %eax
cmp $0x7,%eax
jae 17f0a5f <kernel_3mm+0x55f>
xor %r14d,%r14d
jmp 17f0b0f <kernel_3mm+0x60f>
and $0x7ffffff8,%ebp
xor %r12d,%r12d
mov 0x8(%rsp),%r13
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %r13,%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x3840(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x7080(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0xa8c0(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0xe100(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x11940(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x15180(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
lea 0x189c0(%r13),%rdi
xor %esi,%esi
mov %rbx,%rdx
call 17d7150 <memset$plt>
add $0x8,%r12
add $0x1c200,%r13
cmp %r12,%rbp
jne 17f0980 <kernel_3mm+0x480>
test %r14,%r14
mov 0x28(%rsp),%rbp
je 17f06b9 <kernel_3mm+0x1b9>
mov %rbx,%r13
imul $0x3840,%r12,%rbx
add 0x8(%rsp),%rbx
imul $0x3840,%r14,%r14
xor %r12d,%r12d
nop
lea (%rbx,%r12,1),%rdi
xor %esi,%esi
mov %r13,%rdx
call 17d7150 <memset$plt>
add $0x3840,%r12
cmp %r12,%r14
jne 17f0a40 <kernel_3mm+0x540>
jmp 17f06b9 <kernel_3mm+0x1b9>
mov %r13d,%ebx
and $0x7ffffff8,%ebx
xor %r14d,%r14d
mov 0x18(%rsp),%r12
mov %r12,%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x44c0(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x8980(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0xce40(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x11300(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x157c0(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x19c80(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
lea 0x1e140(%r12),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
add $0x8,%r14
add $0x22600,%r12
cmp %r14,%rbx
jne 17f0a70 <kernel_3mm+0x570>
cmpq $0x0,0x10(%rsp)
je 17f0b4a <kernel_3mm+0x64a>
imul $0x44c0,%r14,%rbx
add 0x18(%rsp),%rbx
imul $0x44c0,0x10(%rsp),%r14
xor %r12d,%r12d
nop
lea (%rbx,%r12,1),%rdi
xor %esi,%esi
mov %r15,%rdx
call 17d7150 <memset$plt>
add $0x44c0,%r12
cmp %r12,%r14
jne 17f0b30 <kernel_3mm+0x630>
cmpl $0x0,(%rsp)
mov 0x20(%rsp),%r12
mov 0x4(%rsp),%ecx
jle 17f0c6b <kernel_3mm+0x76b>
mov (%rsp),%eax
mov %ecx,%ecx
mov %r13d,%edx
and $0x7ffffffe,%edx
mov 0x8(%rsp),%rsi
add $0x8,%rsi
xor %edi,%edi
jmp 17f0b93 <kernel_3mm+0x693>
nopl 0x0(%rax,%rax,1)
inc %rdi
add $0x3840,%rsi
cmp %rax,%rdi
je 17f0c6b <kernel_3mm+0x76b>
imul $0x44c0,%rdi,%r8
add %rbp,%r8
imul $0x3840,%rdi,%r9
add 0x8(%rsp),%r9
mov 0x18(%rsp),%r10
xor %r11d,%r11d
jmp 17f0bcc <kernel_3mm+0x6cc>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r11
add $0x8,%r10
cmp %rcx,%r11
je 17f0b80 <kernel_3mm+0x680>
lea (%r8,%r11,8),%r14
movq $0x0,(%r8,%r11,8)
xorpd %xmm0,%xmm0
cmp $0x1,%r12d
jne 17f0bf0 <kernel_3mm+0x6f0>
xor %r15d,%r15d
jmp 17f0c3b <kernel_3mm+0x73b>
nopw 0x0(%rax,%rax,1)
mov %r10,%rbx
xor %r15d,%r15d
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%rsi,%r15,8),%xmm1
mulsd (%rbx),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r14)
movsd (%rsi,%r15,8),%xmm0
mulsd 0x44c0(%rbx),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r14)
add $0x2,%r15
add $0x8980,%rbx
cmp %r15,%rdx
jne 17f0c00 <kernel_3mm+0x700>
test $0x1,%r13b
je 17f0bc0 <kernel_3mm+0x6c0>
movsd (%r9,%r15,8),%xmm1
imul $0x44c0,%r15,%rbx
add 0x18(%rsp),%rbx
mulsd (%rbx,%r11,8),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r14)
jmp 17f0bc0 <kernel_3mm+0x6c0>
add $0x48,%rsp
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
<memset$plt>:
mov $0x198,%r11d
jmp *0x23ad464(%rip) # 3b845c0 <memset@GLIBC_2.2.5>
int3
int3
int3
int3