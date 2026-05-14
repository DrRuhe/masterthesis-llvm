<kernel_bicg>:
push %rbp
push %r15
push %r14
push %rbx
push %rax
mov %esi,%ebp
test %edi,%edi
jle 17f18d4 <kernel_bicg+0xc4>
lea 0x23aea18(%rip),%rax # 3ba0240 <g_bicg_A>
mov (%rax),%r14
mov %edi,%r15d
lea 0x0(,%r15,8),%rdx
lea 0x23aea13(%rip),%rbx # 3ba0250 <g_bicg_s>
mov %rbx,%rdi
xor %esi,%esi
call 17d7150 <memset$plt>
test %ebp,%ebp
jle 17f18f6 <kernel_bicg+0xe6>
mov %ebp,%eax
xor %ecx,%ecx
lea 0x23b2556(%rip),%rdx # 3ba3db0 <g_bicg_q>
lea 0x23ba56f(%rip),%rsi # 3babdd0 <g_bicg_r>
lea 0x23b6a08(%rip),%rdi # 3ba8270 <g_bicg_p>
nopl 0x0(%rax,%rax,1)
movq $0x0,(%rdx,%rcx,8)
movsd (%rsi,%rcx,8),%xmm0
xorpd %xmm1,%xmm1
xor %r8d,%r8d
data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%r14,%r8,8),%xmm2
mulsd %xmm0,%xmm2
addsd (%rbx,%r8,8),%xmm2
movsd %xmm2,(%rbx,%r8,8)
movsd (%r14,%r8,8),%xmm2
mulsd (%rdi,%r8,8),%xmm2
addsd %xmm2,%xmm1
movsd %xmm1,(%rdx,%rcx,8)
inc %r8
cmp %r8,%r15
jne 17f1890 <kernel_bicg+0x80>
inc %rcx
add $0x3840,%r14
cmp %rax,%rcx
jne 17f1870 <kernel_bicg+0x60>
jmp 17f18f6 <kernel_bicg+0xe6>
test %ebp,%ebp
jle 17f18f6 <kernel_bicg+0xe6>
mov %ebp,%edx
shl $0x3,%rdx
lea 0x23b24cb(%rip),%rdi # 3ba3db0 <g_bicg_q>
xor %esi,%esi
add $0x8,%rsp
pop %rbx
pop %r14
pop %r15
pop %rbp
jmp 17d7150 <memset$plt>
add $0x8,%rsp
pop %rbx
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
int3litll
<memset$plt>:
mov $0x198,%r11d
jmp *0x23ad464(%rip) # 3b845c0 <memset@GLIBC_2.2.5>
int3
int3
int3
int3