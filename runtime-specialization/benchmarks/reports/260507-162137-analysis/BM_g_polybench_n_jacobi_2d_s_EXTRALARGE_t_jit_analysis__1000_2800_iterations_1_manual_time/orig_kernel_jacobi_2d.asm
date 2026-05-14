<kernel_jacobi_2d>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
mov %edi,-0x3c(%rsp)
test %edi,%edi
setle %al
cmp $0x3,%esi
setl %cl
or %al,%cl
jne 17fe425 <kernel_jacobi_2d+0x3f5>
lea 0x24692d8(%rip),%rax # 3c67330 <g_jacobi2d_A>
mov (%rax),%rax
lea 0x24692d6(%rip),%rcx # 3c67338 <g_jacobi2d_B>
mov (%rcx),%rcx
dec %esi
lea 0x5788(%rax),%rdi
imul $0x5788,%rsi,%r8
lea (%rax,%r8,1),%r9
lea (%rax,%r8,1),%r11
add $0xffffffffffffa880,%r11
lea 0x8(%rcx),%rbx
lea (%rcx,%r8,1),%r14
lea -0x1(%rsi),%rdx
lea 0x5788(%rcx),%r10
add %rcx,%r8
add $0xffffffffffffa880,%r8
lea 0x8(%rax),%r15
cmp %r9,%r10
setb %r9b
cmp %r8,%r15
setb %r8b
and %r9b,%r8b
mov %rdx,%r9
and $0xfffffffffffffffc,%r9
lea 0x1(%r9),%r10
cmp %r14,%rdi
setb %dil
cmp %r11,%rbx
setb %r11b
and %dil,%r11b
lea 0xaf18(%rax),%rdi
mov %rdi,-0x10(%rsp)
lea 0x5798(%rcx),%rdi
mov %rdi,-0x18(%rsp)
lea 0x5780(%rcx),%rdi
mov %rdi,-0x20(%rsp)
lea 0xaf00(%rax),%rdi
mov %rdi,-0x28(%rsp)
lea 0xaf18(%rcx),%rdi
mov %rdi,-0x30(%rsp)
lea 0x5798(%rax),%rdi
mov %rdi,-0x38(%rsp)
add $0x5780,%rax
mov %rax,-0x8(%rsp)
add $0xaf00,%rcx
xor %ebx,%ebx
movsd -0x2dd7f6(%rip),%xmm0 # 1520940 <.LCPI117_2>
movapd -0x2dff2e(%rip),%xmm1 # 151e210 <.LCPI211_0>
jmp 17fe14c <kernel_jacobi_2d+0x11c>
inc %ebx
cmp -0x3c(%rsp),%ebx
je 17fe425 <kernel_jacobi_2d+0x3f5>
mov $0x1,%edi
mov -0x28(%rsp),%r12
mov -0x20(%rsp),%r15
mov -0x18(%rsp),%r14
mov -0x10(%rsp),%r13
jmp 17fe198 <kernel_jacobi_2d+0x168>
nopw 0x0(%rax,%rax,1)
inc %rdi
add $0x5780,%r13
add $0x5780,%r14
add $0x5780,%r15
add $0x5780,%r12
cmp %rsi,%rdi
je 17fe2c0 <kernel_jacobi_2d+0x290>
cmp $0x4,%rdx
setb %al
or %r8b,%al
mov $0x1,%ebp
test $0x1,%al
jne 17fe270 <kernel_jacobi_2d+0x240>
xor %ebp,%ebp
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x5790(%r13,%rbp,8),%xmm2
movupd -0x5780(%r13,%rbp,8),%xmm3
movupd -0x5798(%r13,%rbp,8),%xmm4
addpd %xmm2,%xmm4
movupd -0x5788(%r13,%rbp,8),%xmm2
addpd %xmm2,%xmm3
movupd -0x5778(%r13,%rbp,8),%xmm5
addpd %xmm3,%xmm5
addpd %xmm2,%xmm4
movupd -0x10(%r13,%rbp,8),%xmm2
addpd %xmm4,%xmm2
movupd 0x0(%r13,%rbp,8),%xmm3
addpd %xmm5,%xmm3
movupd -0xaf10(%r13,%rbp,8),%xmm4
addpd %xmm2,%xmm4
movupd -0xaf00(%r13,%rbp,8),%xmm2
addpd %xmm3,%xmm2
mulpd %xmm1,%xmm4
mulpd %xmm1,%xmm2
movupd %xmm4,-0x10(%r14,%rbp,8)
movupd %xmm2,(%r14,%rbp,8)
add $0x4,%rbp
cmp %rbp,%r9
jne 17fe1c0 <kernel_jacobi_2d+0x190>
mov %r10,%rbp
cmp %r9,%rdx
je 17fe170 <kernel_jacobi_2d+0x140>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd -0x5780(%r12,%rbp,8),%xmm2
addsd -0x5788(%r12,%rbp,8),%xmm2
addsd -0x5778(%r12,%rbp,8),%xmm2
addsd (%r12,%rbp,8),%xmm2
addsd -0xaf00(%r12,%rbp,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r15,%rbp,8)
inc %rbp
cmp %rbp,%rsi
jne 17fe270 <kernel_jacobi_2d+0x240>
jmp 17fe170 <kernel_jacobi_2d+0x140>
data16 cs nopw 0x0(%rax,%rax,1)
mov $0x1,%edi
mov %rcx,%r12
mov -0x8(%rsp),%r15
mov -0x38(%rsp),%r14
mov -0x30(%rsp),%r13
jmp 17fe308 <kernel_jacobi_2d+0x2d8>
nopl 0x0(%rax)
inc %rdi
add $0x5780,%r13
add $0x5780,%r14
add $0x5780,%r15
add $0x5780,%r12
cmp %rsi,%rdi
je 17fe140 <kernel_jacobi_2d+0x110>
cmp $0x4,%rdx
setb %al
or %r11b,%al
mov $0x1,%ebp
test $0x1,%al
jne 17fe3e0 <kernel_jacobi_2d+0x3b0>
xor %ebp,%ebp
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd -0x5790(%r13,%rbp,8),%xmm2
movupd -0x5780(%r13,%rbp,8),%xmm3
movupd -0x5798(%r13,%rbp,8),%xmm4
addpd %xmm2,%xmm4
movupd -0x5788(%r13,%rbp,8),%xmm2
addpd %xmm2,%xmm3
movupd -0x5778(%r13,%rbp,8),%xmm5
addpd %xmm3,%xmm5
addpd %xmm2,%xmm4
movupd -0x10(%r13,%rbp,8),%xmm2
addpd %xmm4,%xmm2
movupd 0x0(%r13,%rbp,8),%xmm3
addpd %xmm5,%xmm3
movupd -0xaf10(%r13,%rbp,8),%xmm4
addpd %xmm2,%xmm4
movupd -0xaf00(%r13,%rbp,8),%xmm2
addpd %xmm3,%xmm2
mulpd %xmm1,%xmm4
mulpd %xmm1,%xmm2
movupd %xmm4,-0x10(%r14,%rbp,8)
movupd %xmm2,(%r14,%rbp,8)
add $0x4,%rbp
cmp %rbp,%r9
jne 17fe330 <kernel_jacobi_2d+0x300>
mov %r10,%rbp
cmp %r9,%rdx
je 17fe2e0 <kernel_jacobi_2d+0x2b0>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd -0x5780(%r12,%rbp,8),%xmm2
addsd -0x5788(%r12,%rbp,8),%xmm2
addsd -0x5778(%r12,%rbp,8),%xmm2
addsd (%r12,%rbp,8),%xmm2
addsd -0xaf00(%r12,%rbp,8),%xmm2
mulsd %xmm0,%xmm2
movsd %xmm2,(%r15,%rbp,8)
inc %rbp
cmp %rbp,%rsi
jne 17fe3e0 <kernel_jacobi_2d+0x3b0>
jmp 17fe2e0 <kernel_jacobi_2d+0x2b0>
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret