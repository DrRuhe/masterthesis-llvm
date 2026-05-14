<kernel_deriche>:
lea 0x245b5b9(%rip),%rax # 3c55970 <g_deriche_imgIn>
mov (%rax),%rdx
lea 0x245b5b7(%rip),%rax # 3c55978 <g_deriche_imgOut>
mov (%rax),%rcx
lea 0x245b5b5(%rip),%rax # 3c55980 <g_deriche_y1>
mov (%rax),%r8
lea 0x245b5b3(%rip),%rax # 3c55988 <g_deriche_y2>
mov (%rax),%r9
movss -0x2d9dd4(%rip),%xmm0 # 152060c <.LCPI183_0>
jmp 17f9a20 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_>
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
<_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x88,%rsp
mov %r9,0x18(%rsp)
mov %r8,0x28(%rsp)
mov %rcx,0x10(%rsp)
mov %rdx,%r13
movaps %xmm0,0x30(%rsp)
mov %esi,0xc(%rsp)
mov %edi,%ebx
xorps -0x2dae65(%rip),%xmm0 # 151ebf0 <.LCPI1085_0>
movaps %xmm0,0x50(%rsp)
call 17d6670 <expf$plt>
movaps %xmm0,%xmm2
movss -0x2d9472(%rip),%xmm1 # 15205f8 <.LCPI1208_0>
movaps %xmm1,%xmm0
subss %xmm2,%xmm0
mulss %xmm0,%xmm0
movaps %xmm0,0x60(%rsp)
movaps 0x30(%rsp),%xmm0
addss %xmm0,%xmm0
mulss %xmm0,%xmm2
addss %xmm1,%xmm2
movss %xmm2,0x24(%rsp)
call 17d6670 <expf$plt>
movss 0x24(%rsp),%xmm1
subss %xmm0,%xmm1
movaps 0x60(%rsp),%xmm0
divss %xmm1,%xmm0
movaps %xmm0,0x60(%rsp)
movaps 0x50(%rsp),%xmm0
call 17d6670 <expf$plt>
movaps 0x60(%rsp),%xmm3
mulss %xmm3,%xmm0
movss -0x2d9501(%rip),%xmm2 # 15205c8 <.LCPI284_1>
movaps 0x30(%rsp),%xmm1
addss %xmm1,%xmm2
mulss %xmm0,%xmm2
movss %xmm2,0x24(%rsp)
movss -0x2d94ec(%rip),%xmm2 # 15205f8 <.LCPI1208_0>
addss %xmm1,%xmm2
mulss %xmm0,%xmm2
movss %xmm2,0x4c(%rsp)
xorps -0x2daf09(%rip),%xmm3 # 151ebf0 <.LCPI1085_0>
movaps %xmm3,0x70(%rsp)
mulss -0x2d94b2(%rip),%xmm1 # 1520654 <.LCPI69_3>
movaps %xmm1,0x30(%rsp)
movaps %xmm1,%xmm0
call 17d6670 <expf$plt>
mulss 0x70(%rsp),%xmm0
movss %xmm0,0x70(%rsp)
movaps 0x50(%rsp),%xmm0
call 17d66e0 <exp2f$plt>
movss %xmm0,0x48(%rsp)
movaps 0x30(%rsp),%xmm0
call 17d6670 <expf$plt>
movss 0x24(%rsp),%xmm11
movaps 0x60(%rsp),%xmm10
movss 0x48(%rsp),%xmm9
movss 0x70(%rsp),%xmm8
movss 0x4c(%rsp),%xmm7
xorps -0x2daf71(%rip),%xmm0 # 151ebf0 <.LCPI1085_0>
mov %ebx,0x50(%rsp)
test %ebx,%ebx
jle 17f9eb6 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x496>
cmpl $0x0,0xc(%rsp)
mov 0x50(%rsp),%eax
jle 17fa0af <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x68f>
mov %eax,%eax
mov 0xc(%rsp),%ecx
xor %edx,%edx
mov 0x28(%rsp),%rsi
mov %r13,%rdi
nopl 0x0(%rax)
xorps %xmm2,%xmm2
xor %r8d,%r8d
xorps %xmm3,%xmm3
xorps %xmm1,%xmm1
nopl 0x0(%rax)
movaps %xmm3,%xmm4
mulss %xmm11,%xmm2
movss (%rdi,%r8,4),%xmm3
mulss %xmm10,%xmm3
addss %xmm2,%xmm3
movaps %xmm9,%xmm2
mulss %xmm4,%xmm2
addss %xmm3,%xmm2
mulss %xmm0,%xmm1
addss %xmm2,%xmm1
movss %xmm1,(%rsi,%r8,4)
movss (%rdi,%r8,4),%xmm2
inc %r8
movaps %xmm1,%xmm3
movaps %xmm4,%xmm1
cmp %r8,%rcx
jne 17f9ba0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x180>
inc %rdx
add $0x4380,%rdi
add $0x4380,%rsi
cmp %rax,%rdx
jne 17f9b90 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x170>
lea -0x1(%rcx),%rdx
xorps %xmm2,%xmm2
movaps %xmm8,%xmm1
mulss %xmm2,%xmm1
movaps %xmm7,%xmm3
mulss %xmm2,%xmm3
addss %xmm1,%xmm3
movaps %xmm9,%xmm4
mulss %xmm2,%xmm4
addss %xmm3,%xmm4
movaps %xmm0,%xmm1
mulss %xmm2,%xmm1
addss %xmm4,%xmm1
lea -0x4(%r13),%rsi
xor %edi,%edi
mov 0x18(%rsp),%r8
jmp 17f9c5a <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x23a>
nopl 0x0(%rax)
inc %rdi
add $0x4380,%rsi
add $0x4380,%r8
cmp %rax,%rdi
je 17f9d13 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x2f3>
mov %rcx,%r9
xorps %xmm3,%xmm3
xorps %xmm2,%xmm2
test $0x1,%cl
je 17f9c8c <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x26c>
imul $0x4380,%rdi,%r9
mov 0x18(%rsp),%r10
add %r9,%r10
movss %xmm1,(%r10,%rdx,4)
add %r13,%r9
movss (%r9,%rdx,4),%xmm3
mov %rdx,%r9
movaps %xmm1,%xmm2
cmpl $0x1,0xc(%rsp)
je 17f9c40 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x220>
add $0x2,%r9
xorps %xmm5,%xmm5
xorps %xmm4,%xmm4
nopl (%rax)
mulss %xmm8,%xmm5
movaps %xmm7,%xmm6
mulss %xmm3,%xmm6
addss %xmm5,%xmm6
movaps %xmm9,%xmm5
mulss %xmm2,%xmm5
addss %xmm6,%xmm5
mulss %xmm0,%xmm4
addss %xmm5,%xmm4
movss %xmm4,-0xc(%r8,%r9,4)
movss -0x8(%rsi,%r9,4),%xmm5
mulss %xmm8,%xmm3
movaps %xmm7,%xmm6
mulss %xmm5,%xmm6
addss %xmm3,%xmm6
movaps %xmm9,%xmm3
mulss %xmm4,%xmm3
addss %xmm6,%xmm3
mulss %xmm0,%xmm2
addss %xmm3,%xmm2
movss %xmm2,-0x10(%r8,%r9,4)
movss -0xc(%rsi,%r9,4),%xmm3
add $0xfffffffffffffffe,%r9
cmp $0x2,%r9
jg 17f9ca0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x280>
jmp 17f9c40 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x220>
mov 0x10(%rsp),%r15
mov %r15,%rdx
mov 0x28(%rsp),%r14
sub %r14,%rdx
mov %r15,%rsi
mov 0x18(%rsp),%rbx
sub %rbx,%rsi
cmp $0x20,%rdx
setb %dil
cmp $0x20,%rsi
setb %bpl
or %dil,%bpl
mov %ecx,%esi
and $0x7ffffff8,%esi
mov %rsi,0x30(%rsp)
lea -0x1(%rcx),%rdi
mov %ecx,%r8d
shr $0x3,%r8d
and $0xfffffff,%r8d
shl $0x5,%r8
lea 0x10(%r15),%r9
lea 0x10(%rbx),%r10
lea 0x10(%r14),%r11
lea 0x4(%r14),%r13
lea 0x4(%rbx),%r14
lea 0x4(%r15),%r12
xor %r15d,%r15d
jmp 17f9dc6 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x3a6>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r15
add $0x4380,%r9
add $0x4380,%r10
add $0x4380,%r11
add $0x4380,%r13
add $0x4380,%r14
add $0x4380,%r12
cmp %rax,%r15
je 17f9eb6 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x496>
cmpl $0x8,0xc(%rsp)
setb %sil
or %bpl,%sil
je 17f9de0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x3c0>
xor %esi,%esi
jmp 17f9e30 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x410>
nopl 0x0(%rax,%rax,1)
xor %esi,%esi
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movups -0x10(%r11,%rsi,1),%xmm1
movups (%r11,%rsi,1),%xmm2
movups -0x10(%r10,%rsi,1),%xmm3
addps %xmm1,%xmm3
movups (%r10,%rsi,1),%xmm1
addps %xmm2,%xmm1
movups %xmm3,-0x10(%r9,%rsi,1)
movups %xmm1,(%r9,%rsi,1)
add $0x20,%rsi
cmp %rsi,%r8
jne 17f9df0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x3d0>
mov 0x30(%rsp),%rbx
mov %rbx,%rsi
cmp %ecx,%ebx
je 17f9d90 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x370>
mov %rsi,%rbx
test $0x1,%cl
je 17f9e70 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x450>
imul $0x4380,%r15,%rbx
mov %rdi,%rdx
mov 0x28(%rsp),%rdi
add %rbx,%rdi
movss (%rdi,%rsi,4),%xmm1
mov 0x18(%rsp),%rdi
add %rbx,%rdi
addss (%rdi,%rsi,4),%xmm1
mov %rdx,%rdi
add 0x10(%rsp),%rbx
movss %xmm1,(%rbx,%rsi,4)
mov %rsi,%rbx
or $0x1,%rbx
cmp %rdi,%rsi
je 17f9d90 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x370>
nopl 0x0(%rax)
movss -0x4(%r13,%rbx,4),%xmm1
addss -0x4(%r14,%rbx,4),%xmm1
movss %xmm1,-0x4(%r12,%rbx,4)
movss 0x0(%r13,%rbx,4),%xmm1
addss (%r14,%rbx,4),%xmm1
movss %xmm1,(%r12,%rbx,4)
add $0x2,%rbx
cmp %rbx,%rcx
jne 17f9e80 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x460>
jmp 17f9d90 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x370>
cmpl $0x0,0xc(%rsp)
mov 0x50(%rsp),%ebx
jle 17fa09e <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x67e>
test %ebx,%ebx
jle 17fa0af <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x68f>
mov 0xc(%rsp),%eax
mov %ebx,%ecx
imul $0x4380,%rcx,%rdx
xor %esi,%esi
mov 0x28(%rsp),%rdi
mov 0x10(%rsp),%r8
cs nopw 0x0(%rax,%rax,1)
xorps %xmm2,%xmm2
xor %r9d,%r9d
xorps %xmm3,%xmm3
xorps %xmm1,%xmm1
nopl 0x0(%rax)
movaps %xmm3,%xmm4
mulss %xmm11,%xmm2
movss (%r8,%r9,1),%xmm3
mulss %xmm10,%xmm3
addss %xmm2,%xmm3
movaps %xmm9,%xmm2
mulss %xmm4,%xmm2
addss %xmm3,%xmm2
mulss %xmm0,%xmm1
addss %xmm2,%xmm1
movss %xmm1,(%rdi,%r9,1)
movss (%r8,%r9,1),%xmm2
add $0x4380,%r9
movaps %xmm1,%xmm3
movaps %xmm4,%xmm1
cmp %r9,%rdx
jne 17f9f00 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x4e0>
inc %rsi
add $0x4,%r8
add $0x4,%rdi
cmp %rax,%rsi
jne 17f9ef0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x4d0>
lea -0x1(%rcx),%rdx
xorps %xmm2,%xmm2
movaps %xmm8,%xmm1
mulss %xmm2,%xmm1
movaps %xmm7,%xmm3
mulss %xmm2,%xmm3
addss %xmm1,%xmm3
movaps %xmm9,%xmm4
mulss %xmm2,%xmm4
addss %xmm3,%xmm4
movaps %xmm0,%xmm1
mulss %xmm2,%xmm1
addss %xmm4,%xmm1
mov 0x10(%rsp),%rsi
add $0xffffffffffffbc80,%rsi
mov 0x18(%rsp),%r10
lea -0x4380(%r10),%rdi
xor %r8d,%r8d
imul $0x4380,%rdx,%r9
add %r9,%r10
jmp 17f9fd4 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x5b4>
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %r8
add $0x4,%rsi
add $0x4,%rdi
cmp %rax,%r8
je 17fa09e <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x67e>
mov %rcx,%r11
xorps %xmm3,%xmm3
xorps %xmm2,%xmm2
test $0x1,%cl
je 17f9ffc <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x5dc>
movss %xmm1,(%r10,%r8,4)
mov 0x10(%rsp),%r11
add %r9,%r11
movss (%r11,%r8,4),%xmm3
mov %rdx,%r11
movaps %xmm1,%xmm2
cmp $0x1,%ebx
je 17f9fc0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x5a0>
imul $0x4380,%r11,%r14
add $0x2,%r11
xorps %xmm5,%xmm5
xorps %xmm4,%xmm4
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mulss %xmm8,%xmm5
movaps %xmm7,%xmm6
mulss %xmm3,%xmm6
addss %xmm5,%xmm6
movaps %xmm9,%xmm5
mulss %xmm2,%xmm5
addss %xmm6,%xmm5
mulss %xmm0,%xmm4
addss %xmm5,%xmm4
movss %xmm4,(%rdi,%r14,1)
movss (%rsi,%r14,1),%xmm5
mulss %xmm8,%xmm3
movaps %xmm7,%xmm6
mulss %xmm5,%xmm6
addss %xmm3,%xmm6
movaps %xmm9,%xmm3
mulss %xmm4,%xmm3
addss %xmm6,%xmm3
mulss %xmm0,%xmm2
addss %xmm3,%xmm2
movss %xmm2,-0x4380(%rdi,%r14,1)
movss -0x4380(%rsi,%r14,1),%xmm3
add $0xfffffffffffffffe,%r11
add $0xffffffffffff7900,%r14
cmp $0x2,%r11
jg 17fa020 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x600>
jmp 17f9fc0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x5a0>
cmpl $0x0,0xc(%rsp)
setle %al
test %ebx,%ebx
setle %cl
or %al,%cl
je 17fa0c1 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x6a1>
add $0x88,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
mov %ebx,%eax
mov 0xc(%rsp),%r8d
mov %r8d,%ecx
mov 0x10(%rsp),%r15
mov %r15,%rdx
mov 0x28(%rsp),%r14
sub %r14,%rdx
mov %r15,%rsi
mov 0x18(%rsp),%rbx
sub %rbx,%rsi
cmp $0x20,%rdx
setb %dil
cmp $0x20,%rsi
setb %dl
or %dil,%dl
mov %ecx,%esi
and $0x7ffffff8,%esi
mov %rsi,0x30(%rsp)
lea -0x1(%rcx),%rdi
shr $0x3,%r8d
and $0xfffffff,%r8d
shl $0x5,%r8
lea 0x10(%r15),%r9
lea 0x10(%rbx),%r10
lea 0x10(%r14),%r11
lea 0x4(%r14),%r12
lea 0x4(%rbx),%r14
lea 0x4(%r15),%r13
xor %r15d,%r15d
jmp 17fa176 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x756>
data16 cs nopw 0x0(%rax,%rax,1)
inc %r15
add $0x4380,%r9
add $0x4380,%r10
add $0x4380,%r11
add $0x4380,%r12
add $0x4380,%r14
add $0x4380,%r13
cmp %rax,%r15
je 17fa0af <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x68f>
cmpl $0x8,0xc(%rsp)
setb %bl
or %dl,%bl
je 17fa190 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x770>
xor %ebp,%ebp
jmp 17fa1e0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x7c0>
cs nopw 0x0(%rax,%rax,1)
xor %ebx,%ebx
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movups -0x10(%r11,%rbx,1),%xmm0
movups (%r11,%rbx,1),%xmm1
movups -0x10(%r10,%rbx,1),%xmm2
addps %xmm0,%xmm2
movups (%r10,%rbx,1),%xmm0
addps %xmm1,%xmm0
movups %xmm2,-0x10(%r9,%rbx,1)
movups %xmm0,(%r9,%rbx,1)
add $0x20,%rbx
cmp %rbx,%r8
jne 17fa1a0 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x780>
mov 0x30(%rsp),%rsi
mov %rsi,%rbp
cmp %ecx,%esi
je 17fa140 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x720>
mov %rbp,%rbx
test $0x1,%cl
je 17fa22a <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x80a>
imul $0x4380,%r15,%rbx
mov %edx,%esi
mov %rax,%rdx
mov %rdi,%rax
mov 0x28(%rsp),%rdi
add %rbx,%rdi
movss (%rdi,%rbp,4),%xmm0
mov 0x18(%rsp),%rdi
add %rbx,%rdi
addss (%rdi,%rbp,4),%xmm0
mov %rax,%rdi
mov %rdx,%rax
mov %esi,%edx
add 0x10(%rsp),%rbx
movss %xmm0,(%rbx,%rbp,4)
mov %rbp,%rbx
or $0x1,%rbx
cmp %rdi,%rbp
je 17fa140 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x720>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
movss -0x4(%r12,%rbx,4),%xmm0
addss -0x4(%r14,%rbx,4),%xmm0
movss %xmm0,-0x4(%r13,%rbx,4)
movss (%r12,%rbx,4),%xmm0
addss (%r14,%rbx,4),%xmm0
movss %xmm0,0x0(%r13,%rbx,4)
add $0x2,%rbx
cmp %rbx,%rcx
jne 17fa240 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x820>
jmp 17fa140 <_ZL19__pb_deriche_kerneliifPA4320_fS0_S0_S0_+0x720>
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
<exp2f$plt>:
mov $0xf1,%r11d
jmp *0x23ad99c(%rip) # 3b84088 <exp2f@GLIBC_2.27>
int3
int3
int3
int3
<expf$plt>:
mov $0xea,%r11d
jmp *0x23ad9d4(%rip) # 3b84050 <expf@GLIBC_2.27>
int3
int3
int3
int3