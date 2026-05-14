<kernel_gemm>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
mov %esi,-0x5c(%rsp)
test %edi,%edi
jle 17f34ad <kernel_gemm+0x3fd>
lea 0x23dce4b(%rip),%rax # 3bcff18 <g_gemm_C>
mov (%rax),%r10
test %edx,%edx
jle 17f33c1 <kernel_gemm+0x311>
cmpl $0x0,-0x5c(%rsp)
jle 17f34ad <kernel_gemm+0x3fd>
lea 0x23dce36(%rip),%rax # 3bcff20 <g_gemm_A>
mov (%rax),%rsi
lea 0x23dce34(%rip),%rax # 3bcff28 <g_gemm_B>
mov (%rax),%r8
mov %edi,%eax
mov %rax,-0x28(%rsp)
mov -0x5c(%rsp),%ecx
mov %ecx,%r9d
mov %edx,%edx
imul $0x47e0,%rdx,%rax
add %r8,%rax
lea (%rax,%r9,8),%rax
add $0xffffffffffffb820,%rax
mov %rax,-0x30(%rsp)
lea (%r10,%r9,8),%rax
mov %rax,-0x38(%rsp)
mov %rsi,-0x20(%rsp)
lea (%rsi,%rdx,8),%rax
mov %rax,-0x40(%rsp)
mov %r9d,%r15d
and $0x7ffffffc,%r15d
lea -0x1(%r9),%rax
mov %rax,-0x8(%rsp)
mov %ecx,%r12d
shr $0x2,%r12d
and $0x1fffffff,%r12d
shl $0x5,%r12
lea 0x10(%r10),%r13
lea 0x10(%r8),%rax
mov %rax,-0x48(%rsp)
mov %r8,-0x58(%rsp)
lea 0x8(%r8),%rax
mov %rax,-0x50(%rsp)
xor %esi,%esi
movsd -0x2d2801(%rip),%xmm0 # 1520980 <.LCPI107_2>
movsd -0x2d2981(%rip),%xmm1 # 1520808 <.LCPI107_0>
movapd -0x2d4621(%rip),%xmm2 # 151eb70 <.LCPI107_1>
mov %r10,%rax
mov %r10,-0x18(%rsp)
jmp 17f31c5 <kernel_gemm+0x115>
nopl 0x0(%rax,%rax,1)
mov -0x10(%rsp),%rsi
inc %rsi
add $0x47e0,%r13
add $0x47e0,%rax
cmp -0x28(%rsp),%rsi
mov -0x18(%rsp),%r10
je 17f34ad <kernel_gemm+0x3fd>
cmpl $0x4,-0x5c(%rsp)
jae 17f31d0 <kernel_gemm+0x120>
xor %ecx,%ecx
jmp 17f3220 <kernel_gemm+0x170>
xor %ecx,%ecx
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%r13,%rcx,1),%xmm3
movupd 0x0(%r13,%rcx,1),%xmm4
mulpd %xmm2,%xmm3
mulpd %xmm2,%xmm4
movupd %xmm3,-0x10(%r13,%rcx,1)
movupd %xmm4,0x0(%r13,%rcx,1)
add $0x20,%rcx
cmp %rcx,%r12
jne 17f31e0 <kernel_gemm+0x130>
mov %r15,%rcx
cmp %r9d,%r15d
je 17f3236 <kernel_gemm+0x186>
data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rax,%rcx,8),%xmm3
mulsd %xmm0,%xmm3
movsd %xmm3,(%rax,%rcx,8)
inc %rcx
cmp %rcx,%r9
jne 17f3220 <kernel_gemm+0x170>
imul $0x47e0,%rsi,%rcx
lea (%r10,%rcx,1),%r11
add -0x38(%rsp),%rcx
mov %rsi,-0x10(%rsp)
imul $0x5140,%rsi,%rsi
mov -0x20(%rsp),%rdi
lea (%rdi,%rsi,1),%rbx
add -0x40(%rsp),%rsi
cmp %rsi,%r11
setb %sil
cmp %rcx,%rbx
setb %dil
and %sil,%dil
cmp -0x30(%rsp),%r11
setb %sil
cmp %rcx,-0x58(%rsp)
setb %bpl
and %sil,%bpl
or %dil,%bpl
mov -0x50(%rsp),%rdi
mov -0x48(%rsp),%rcx
xor %r8d,%r8d
jmp 17f32ba <kernel_gemm+0x20a>
nopl 0x0(%rax,%rax,1)
inc %r8
add $0x47e0,%rcx
add $0x47e0,%rdi
cmp %rdx,%r8
je 17f31a0 <kernel_gemm+0xf0>
cmpl $0x4,-0x5c(%rsp)
setb %sil
lea (%rbx,%r8,8),%r10
or %bpl,%sil
test $0x1,%sil
je 17f32e0 <kernel_gemm+0x230>
xor %r14d,%r14d
jmp 17f333c <kernel_gemm+0x28c>
data16 cs nopw 0x0(%rax,%rax,1)
movsd (%r10),%xmm3
mulsd %xmm1,%xmm3
unpcklpd %xmm3,%xmm3
xor %esi,%esi
nop
movupd -0x10(%rcx,%rsi,1),%xmm4
movupd (%rcx,%rsi,1),%xmm5
movupd -0x10(%r13,%rsi,1),%xmm6
movupd 0x0(%r13,%rsi,1),%xmm7
mulpd %xmm3,%xmm4
addpd %xmm6,%xmm4
mulpd %xmm3,%xmm5
addpd %xmm7,%xmm5
movupd %xmm4,-0x10(%r13,%rsi,1)
movupd %xmm5,0x0(%r13,%rsi,1)
add $0x20,%rsi
cmp %rsi,%r12
jne 17f32f0 <kernel_gemm+0x240>
mov %r15,%r14
cmp %r9d,%r15d
je 17f32a0 <kernel_gemm+0x1f0>
mov %r14,%rsi
test $0x1,%r9b
je 17f3373 <kernel_gemm+0x2c3>
movsd (%r10),%xmm3
mulsd %xmm1,%xmm3
imul $0x47e0,%r8,%rsi
add -0x58(%rsp),%rsi
mulsd (%rsi,%r14,8),%xmm3
addsd (%r11,%r14,8),%xmm3
movsd %xmm3,(%r11,%r14,8)
mov %r14,%rsi
or $0x1,%rsi
cmp -0x8(%rsp),%r14
je 17f32a0 <kernel_gemm+0x1f0>
xchg %ax,%ax
movsd (%r10),%xmm3
mulsd %xmm1,%xmm3
mulsd -0x8(%rdi,%rsi,8),%xmm3
addsd (%rax,%rsi,8),%xmm3
movsd %xmm3,(%rax,%rsi,8)
movsd (%r10),%xmm3
mulsd %xmm1,%xmm3
mulsd (%rdi,%rsi,8),%xmm3
addsd 0x8(%rax,%rsi,8),%xmm3
movsd %xmm3,0x8(%rax,%rsi,8)
add $0x2,%rsi
cmp %rsi,%r9
jne 17f3380 <kernel_gemm+0x2d0>
jmp 17f32a0 <kernel_gemm+0x1f0>
cmpl $0x0,-0x5c(%rsp)
jle 17f34ad <kernel_gemm+0x3fd>
mov %edi,%eax
mov -0x5c(%rsp),%esi
mov %esi,%ecx
mov %ecx,%edx
and $0x7ffffffc,%edx
mov %esi,%edi
shr $0x2,%edi
and $0x1fffffff,%edi
shl $0x5,%rdi
lea 0x10(%r10),%r8
xor %r9d,%r9d
movsd -0x2d2a7a(%rip),%xmm0 # 1520980 <.LCPI107_2>
movapd -0x2d4892(%rip),%xmm1 # 151eb70 <.LCPI107_1>
jmp 17f342a <kernel_gemm+0x37a>
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r9
add $0x47e0,%r8
add $0x47e0,%r10
cmp %rax,%r9
je 17f34ad <kernel_gemm+0x3fd>
cmpl $0x4,-0x5c(%rsp)
jae 17f3440 <kernel_gemm+0x390>
xor %esi,%esi
jmp 17f3490 <kernel_gemm+0x3e0>
data16 cs nopw 0x0(%rax,%rax,1)
xor %esi,%esi
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x10(%r8,%rsi,1),%xmm2
movupd (%r8,%rsi,1),%xmm3
mulpd %xmm1,%xmm2
mulpd %xmm1,%xmm3
movupd %xmm2,-0x10(%r8,%rsi,1)
movupd %xmm3,(%r8,%rsi,1)
add $0x20,%rsi
cmp %rsi,%rdi
jne 17f3450 <kernel_gemm+0x3a0>
mov %rdx,%rsi
cmp %ecx,%edx
je 17f3410 <kernel_gemm+0x360>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%r10,%rsi,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r10,%rsi,8)
inc %rsi
cmp %rsi,%rcx
jne 17f3490 <kernel_gemm+0x3e0>
jmp 17f3410 <kernel_gemm+0x360>
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