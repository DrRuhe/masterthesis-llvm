<kernel_heat_3d>:
mov %esi,%edi
lea 0x245a347(%rip),%rax # 3c57920 <g_heat3d_A>
mov (%rax),%rsi
lea 0x245a345(%rip),%rax # 3c57928 <g_heat3d_B>
mov (%rax),%rdx
jmp 17fced0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_>
int3
int3
int3
int3
int3
<_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x18,%rsp
mov %rdi,-0x20(%rsp)
lea -0x1(%rdi),%eax
imul $0x648,%rax,%rdi
lea -0x1(%rax),%rcx
lea 0x4e848(%rdx),%r8
mov %r8,-0x60(%rsp)
lea (%rdx,%rdi,1),%r8
add $0x4dbc0,%r8
mov %r8,0x10(%rsp)
lea (%rsi,%rdi,1),%r8
add $0x9bdc0,%r8
mov %r8,0x8(%rsp)
mov %rcx,%r11
and $0xfffffffffffffffe,%r11
mov %rcx,%r8
or $0x1,%r8
mov %r8,-0x58(%rsp)
lea (%rsi,%rdi,1),%r8
add $0x4dbc0,%r8
mov %r8,(%rsp)
lea 0x648(%rdx),%r8
mov %r8,-0x8(%rsp)
lea 0x9bdc0(%rdx,%rdi,1),%rdi
mov %rdi,-0x10(%rsp)
lea 0x4e840(%rdx),%rdi
mov %rdi,-0x38(%rsp)
lea 0x9ca48(%rdx),%rdi
mov %rdi,-0x40(%rsp)
add $0x9ca40,%rdx
mov %rdx,-0x28(%rsp)
lea 0x648(%rsi),%rdx
mov %rdx,-0x18(%rsp)
lea 0x4e848(%rsi),%rdx
mov %rdx,-0x68(%rsp)
lea 0x9ca48(%rsi),%rdx
mov %rdx,-0x48(%rsp)
lea 0x9ca40(%rsi),%rdx
mov %rdx,-0x50(%rsp)
add $0x4e840,%rsi
mov %rsi,-0x30(%rsp)
movsd -0x2dc7d0(%rip),%xmm0 # 15207f0 <.LCPI203_2>
movsd -0x2dc768(%rip),%xmm1 # 1520860 <.LCPI182_3>
movapd -0x2dce80(%rip),%xmm2 # 1520150 <.LCPI203_0>
movapd -0x2de508(%rip),%xmm3 # 151ead0 <.LCPI203_1>
mov $0x1,%edx
jmp 17fcff2 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x122>
nop
mov -0x6c(%rsp),%edx
inc %edx
cmp $0x3e9,%edx
je 17fd43d <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x56d>
mov %edx,-0x6c(%rsp)
cmpl $0x3,-0x20(%rsp)
jl 17fcfe0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x110>
mov $0x1,%edx
mov -0x38(%rsp),%r10
mov -0x50(%rsp),%r9
mov -0x60(%rsp),%r13
mov -0x48(%rsp),%rbp
xor %r12d,%r12d
jmp 17fd055 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x185>
nopl 0x0(%rax,%rax,1)
mov -0x78(%rsp),%rdx
inc %rdx
inc %r12
add $0x4e200,%rbp
add $0x4e200,%r13
add $0x4e200,%r9
mov -0x80(%rsp),%r10
add $0x4e200,%r10
cmp %rax,%rdx
je 17fd220 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x350>
mov %rdx,-0x78(%rsp)
imul $0x4e200,%r12,%rdx
mov -0x60(%rsp),%rsi
add %rdx,%rsi
mov 0x10(%rsp),%rdi
add %rdx,%rdi
mov -0x18(%rsp),%r8
add %rdx,%r8
add 0x8(%rsp),%rdx
cmp %rdx,%rsi
setb %dl
cmp %rdi,%r8
setb %r15b
and %dl,%r15b
mov %r10,-0x80(%rsp)
mov %r10,%rsi
mov %r9,%rdi
mov %r13,%r8
mov %rbp,%r10
mov $0x1,%edx
jmp 17fd0d8 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x208>
cs nopw 0x0(%rax,%rax,1)
inc %rdx
add $0x640,%r10
add $0x640,%r8
add $0x640,%rdi
add $0x640,%rsi
cmp %rax,%rdx
je 17fd020 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x150>
cmp $0x2,%rcx
setb %r14b
or %r15b,%r14b
mov $0x1,%ebx
test $0x1,%r14b
jne 17fd1a0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x2d0>
xor %ebx,%ebx
data16 data16 cs nopw 0x0(%rax,%rax,1)
movupd (%r10,%rbx,8),%xmm4
movupd -0x4e200(%r10,%rbx,8),%xmm5
movapd %xmm5,%xmm6
mulpd %xmm2,%xmm6
addpd %xmm6,%xmm4
movupd -0x9c400(%r10,%rbx,8),%xmm7
addpd %xmm4,%xmm7
movupd -0x4dbc0(%r10,%rbx,8),%xmm4
addpd %xmm6,%xmm4
movupd -0x4e840(%r10,%rbx,8),%xmm8
addpd %xmm4,%xmm8
mulpd %xmm3,%xmm8
mulpd %xmm3,%xmm7
addpd %xmm8,%xmm7
movupd -0x4e1f8(%r10,%rbx,8),%xmm4
addpd %xmm6,%xmm4
movupd -0x4e208(%r10,%rbx,8),%xmm6
addpd %xmm4,%xmm6
mulpd %xmm3,%xmm6
addpd %xmm7,%xmm6
addpd %xmm5,%xmm6
movupd %xmm6,(%r8,%rbx,8)
add $0x2,%rbx
cmp %rbx,%r11
jne 17fd100 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x230>
mov -0x58(%rsp),%rbx
cmp %r11,%rcx
je 17fd0b0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x1e0>
xchg %ax,%ax
movsd -0x4e200(%rdi,%rbx,8),%xmm4
movapd %xmm4,%xmm5
mulsd %xmm0,%xmm5
movsd (%rdi,%rbx,8),%xmm6
addsd %xmm5,%xmm6
addsd -0x9c400(%rdi,%rbx,8),%xmm6
movsd -0x4dbc0(%rdi,%rbx,8),%xmm7
addsd %xmm5,%xmm7
addsd -0x4e840(%rdi,%rbx,8),%xmm7
mulsd %xmm1,%xmm7
mulsd %xmm1,%xmm6
addsd -0x4e1f8(%rdi,%rbx,8),%xmm5
addsd -0x4e208(%rdi,%rbx,8),%xmm5
addsd %xmm7,%xmm6
mulsd %xmm1,%xmm5
addsd %xmm6,%xmm5
addsd %xmm4,%xmm5
movsd %xmm5,(%rsi,%rbx,8)
inc %rbx
cmp %rbx,%rax
jne 17fd1a0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x2d0>
jmp 17fd0b0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x1e0>
data16 cs nopw 0x0(%rax,%rax,1)
mov $0x1,%edx
mov -0x30(%rsp),%r12
mov -0x28(%rsp),%rdi
mov -0x68(%rsp),%r8
mov -0x40(%rsp),%r10
xor %ebx,%ebx
jmp 17fd275 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x3a5>
nopl (%rax)
mov -0x78(%rsp),%rdx
inc %rdx
inc %rbx
add $0x4e200,%r10
add $0x4e200,%r8
add $0x4e200,%rdi
mov -0x80(%rsp),%r12
add $0x4e200,%r12
cmp %rax,%rdx
je 17fcfe0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x110>
mov %rdx,-0x78(%rsp)
imul $0x4e200,%rbx,%rdx
mov -0x68(%rsp),%rsi
lea (%rsi,%rdx,1),%r9
mov (%rsp),%rsi
lea (%rsi,%rdx,1),%r14
mov -0x8(%rsp),%rsi
lea (%rsi,%rdx,1),%r15
add -0x10(%rsp),%rdx
cmp %rdx,%r9
setb %dl
cmp %r14,%r15
setb %r15b
and %dl,%r15b
mov %r12,-0x80(%rsp)
mov %rdi,%r9
mov %r8,%r13
mov %r10,%rbp
mov $0x1,%edx
jmp 17fd2f8 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x428>
data16 cs nopw 0x0(%rax,%rax,1)
inc %rdx
add $0x640,%rbp
add $0x640,%r13
add $0x640,%r9
add $0x640,%r12
cmp %rax,%rdx
je 17fd240 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x370>
cmp $0x2,%rcx
setb %sil
or %r15b,%sil
mov $0x1,%r14d
test $0x1,%sil
jne 17fd3c0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x4f0>
xor %r14d,%r14d
cs nopw 0x0(%rax,%rax,1)
movupd 0x0(%rbp,%r14,8),%xmm4
movupd -0x4e200(%rbp,%r14,8),%xmm5
movapd %xmm5,%xmm6
mulpd %xmm2,%xmm6
addpd %xmm6,%xmm4
movupd -0x9c400(%rbp,%r14,8),%xmm7
addpd %xmm4,%xmm7
movupd -0x4dbc0(%rbp,%r14,8),%xmm4
addpd %xmm6,%xmm4
movupd -0x4e840(%rbp,%r14,8),%xmm8
addpd %xmm4,%xmm8
mulpd %xmm3,%xmm8
mulpd %xmm3,%xmm7
addpd %xmm8,%xmm7
movupd -0x4e1f8(%rbp,%r14,8),%xmm4
addpd %xmm6,%xmm4
movupd -0x4e208(%rbp,%r14,8),%xmm6
addpd %xmm4,%xmm6
mulpd %xmm3,%xmm6
addpd %xmm7,%xmm6
addpd %xmm5,%xmm6
movupd %xmm6,0x0(%r13,%r14,8)
add $0x2,%r14
cmp %r14,%r11
jne 17fd320 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x450>
mov -0x58(%rsp),%r14
cmp %r11,%rcx
je 17fd2d0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x400>
movsd -0x4e200(%r9,%r14,8),%xmm4
movapd %xmm4,%xmm5
mulsd %xmm0,%xmm5
movsd (%r9,%r14,8),%xmm6
addsd %xmm5,%xmm6
addsd -0x9c400(%r9,%r14,8),%xmm6
movsd -0x4dbc0(%r9,%r14,8),%xmm7
addsd %xmm5,%xmm7
addsd -0x4e840(%r9,%r14,8),%xmm7
mulsd %xmm1,%xmm7
mulsd %xmm1,%xmm6
addsd -0x4e1f8(%r9,%r14,8),%xmm5
addsd -0x4e208(%r9,%r14,8),%xmm5
addsd %xmm7,%xmm6
mulsd %xmm1,%xmm5
addsd %xmm6,%xmm5
addsd %xmm4,%xmm5
movsd %xmm5,(%r12,%r14,8)
inc %r14
cmp %r14,%rax
jne 17fd3c0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x4f0>
jmp 17fd2d0 <_ZL18__pb_heat3d_kerneliiPA200_A200_dS1_+0x400>
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