<kernel_adi>:
lea 0x245a009(%rip),%rax # 3c559a0 <g_adi_u>
mov (%rax),%rdx
lea 0x245a007(%rip),%rax # 3c559a8 <g_adi_v>
mov (%rax),%rcx
lea 0x245a005(%rip),%rax # 3c559b0 <g_adi_p>
mov (%rax),%r8
lea 0x245a003(%rip),%rax # 3c559b8 <g_adi_q>
mov (%rax),%r9
jmp 17fb1f0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_>
int3
int3
int3
<_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x30,%rsp
mov %rcx,-0x58(%rsp)
mov %rdx,-0x60(%rsp)
mov %rsi,-0x28(%rsp)
mov %edi,-0x7c(%rsp)
test %edi,%edi
jle 17fb846 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x656>
mov -0x28(%rsp),%rax
cvtsi2sd %eax,%xmm0
movsd -0x2da6a3(%rip),%xmm8 # 1520b88 <.LCPI0_0>
movapd %xmm8,%xmm2
divsd %xmm0,%xmm2
cvtsi2sdl -0x7c(%rsp),%xmm1
movapd %xmm8,%xmm0
divsd %xmm1,%xmm0
movapd %xmm0,%xmm1
addsd %xmm0,%xmm1
mulsd %xmm2,%xmm2
divsd %xmm2,%xmm1
divsd %xmm2,%xmm0
movsd -0x2dab97(%rip),%xmm2 # 15206c8 <.LCPI130_3>
movapd %xmm1,%xmm3
mulsd %xmm2,%xmm3
addsd %xmm8,%xmm1
mulsd %xmm0,%xmm2
addsd %xmm8,%xmm0
lea -0x1(%rax),%ecx
movapd -0x2dbec0(%rip),%xmm4 # 151f3c0 <.LCPI0_1>
movapd %xmm3,%xmm5
xorpd %xmm4,%xmm5
xorpd %xmm2,%xmm4
movapd %xmm2,%xmm6
addsd %xmm2,%xmm6
addsd %xmm8,%xmm6
movslq %ecx,%rdx
mov %rdx,-0x10(%rsp)
lea -0x2(%rax),%eax
movapd %xmm3,%xmm7
addsd %xmm3,%xmm7
addsd %xmm8,%xmm7
lea 0x3e80(,%rcx,8),%rdx
mov %rdx,-0x20(%rsp)
mov -0x60(%rsp),%rdx
lea 0x3e88(%rdx,%rax,8),%rsi
mov %rsi,0x28(%rsp)
lea 0x3e88(%r8),%rsi
mov %rsi,-0x30(%rsp)
lea 0x3e90(%rdx),%rsi
mov %rsi,0x18(%rsp)
lea 0x3e88(%r9),%rsi
mov %rsi,-0x38(%rsp)
mov %rcx,-0x8(%rsp)
lea -0x1(%rcx),%rbp
lea 0x3e80(%r8),%rcx
mov %rcx,-0x40(%rsp)
lea 0x3e80(%r9),%rcx
mov %rcx,-0x48(%rsp)
mov %rax,-0x18(%rsp)
imul $0x3e80,%rax,%rax
mov -0x58(%rsp),%rcx
add %rcx,%rax
add $0x3e88,%rax
mov %rax,0x10(%rsp)
lea 0x7d08(%rcx),%rax
mov %rax,0x8(%rsp)
lea 0x3e80(%rdx),%rax
mov %rax,(%rsp)
mov $0x1,%eax
mov %r9,-0x78(%rsp)
mov %r8,-0x50(%rsp)
jmp 17fb372 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x182>
nopl 0x0(%rax)
mov 0x20(%rsp),%rax
cmp -0x7c(%rsp),%eax
lea 0x1(%rax),%eax
je 17fb846 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x656>
mov %rax,0x20(%rsp)
cmpl $0x3,-0x28(%rsp)
jl 17fb360 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x170>
mov $0x1,%r12d
mov 0x10(%rsp),%rdx
mov -0x48(%rsp),%r15
mov -0x40(%rsp),%rdi
mov -0x38(%rsp),%r14
mov 0x18(%rsp),%rax
mov -0x30(%rsp),%r10
xor %ecx,%ecx
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rdx,-0x68(%rsp)
mov %rcx,-0x70(%rsp)
imul $0x3e80,%rcx,%rcx
mov -0x50(%rsp),%r8
lea (%r8,%rcx,1),%rbx
add $0x3e80,%rbx
mov -0x20(%rsp),%rdx
lea (%rdx,%rcx,1),%rsi
mov -0x78(%rsp),%r9
lea (%r9,%rsi,1),%r13
mov -0x58(%rsp),%r11
movabs $0x3ff0000000000000,%rdx
mov %rdx,(%r11,%r12,8)
imul $0x3e80,%r12,%rdx
movq $0x0,(%r8,%rdx,1)
movsd (%r11,%r12,8),%xmm8
movsd %xmm8,(%r9,%rdx,1)
cmp %r13,%rbx
jae 17fb4c0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x2d0>
add %r8,%rsi
lea (%r9,%rcx,1),%rdx
add $0x3e80,%rdx
cmp %rsi,%rdx
jae 17fb4c0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x2d0>
mov %rax,%rsi
xor %ecx,%ecx
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%r10,%rcx,8),%xmm8
mulsd %xmm3,%xmm8
addsd %xmm1,%xmm8
movapd %xmm5,%xmm9
divsd %xmm8,%xmm9
movsd %xmm9,(%r10,%rcx,8)
movsd -0x8(%rsi),%xmm9
mulsd %xmm6,%xmm9
movsd -0x10(%rsi),%xmm10
mulsd %xmm4,%xmm10
addsd %xmm9,%xmm10
movsd (%rsi),%xmm9
mulsd %xmm4,%xmm9
addsd %xmm10,%xmm9
movsd -0x8(%r14,%rcx,8),%xmm10
mulsd %xmm5,%xmm10
addsd %xmm9,%xmm10
divsd %xmm8,%xmm10
movsd %xmm10,(%r14,%rcx,8)
inc %rcx
add $0x3e80,%rsi
cmp %rcx,%rbp
jne 17fb440 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x250>
jmp 17fb54d <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x35d>
nopl 0x0(%rax,%rax,1)
movsd 0x3e80(%r8,%rcx,1),%xmm9
movsd 0x3e80(%r9,%rcx,1),%xmm8
mov %rax,%rsi
xor %ebx,%ebx
nopl 0x0(%rax)
mulsd %xmm3,%xmm9
addsd %xmm1,%xmm9
movapd %xmm5,%xmm10
divsd %xmm9,%xmm10
movsd %xmm10,(%r10,%rbx,8)
movsd -0x8(%rsi),%xmm11
mulsd %xmm6,%xmm11
movsd -0x10(%rsi),%xmm12
mulsd %xmm4,%xmm12
addsd %xmm11,%xmm12
movsd (%rsi),%xmm11
mulsd %xmm4,%xmm11
addsd %xmm12,%xmm11
mulsd %xmm5,%xmm8
addsd %xmm11,%xmm8
divsd %xmm9,%xmm8
movsd %xmm8,(%r14,%rbx,8)
inc %rbx
add $0x3e80,%rsi
movapd %xmm10,%xmm9
cmp %rbx,%rbp
jne 17fb4e0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x2f0>
imul $0x3e80,-0x10(%rsp),%rcx
add -0x58(%rsp),%rcx
movabs $0x3ff0000000000000,%rdx
mov %rdx,(%rcx,%r12,8)
inc %r12
mov -0x68(%rsp),%rdx
mov %rdx,%rcx
mov -0x18(%rsp),%rsi
nopl 0x0(%rax)
movsd (%rdi,%rsi,8),%xmm8
mulsd (%rcx),%xmm8
addsd (%r15,%rsi,8),%xmm8
movsd %xmm8,-0x3e80(%rcx)
add $0xffffffffffffc180,%rcx
cmp $0x1,%rsi
lea -0x1(%rsi),%rsi
jg 17fb580 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x390>
mov -0x70(%rsp),%rcx
inc %rcx
add $0x3e80,%r10
add $0x8,%rax
add $0x3e80,%r14
add $0x3e80,%rdi
add $0x3e80,%r15
add $0x8,%rdx
cmp -0x8(%rsp),%r12
jne 17fb3b0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x1c0>
mov $0x1,%eax
mov (%rsp),%rdi
mov -0x48(%rsp),%r10
mov -0x40(%rsp),%r11
mov -0x38(%rsp),%r14
mov -0x30(%rsp),%r15
mov 0x8(%rsp),%r12
xor %ecx,%ecx
mov -0x50(%rsp),%r8
nopl 0x0(%rax,%rax,1)
mov %rcx,-0x70(%rsp)
imul $0x3e80,%rcx,%rbx
lea (%r8,%rbx,1),%rdx
add $0x3e80,%rdx
mov -0x20(%rsp),%rcx
lea (%rcx,%rbx,1),%r13
mov -0x78(%rsp),%rsi
lea (%rsi,%r13,1),%r9
mov %rax,-0x68(%rsp)
imul $0x3e80,%rax,%rsi
mov -0x60(%rsp),%rcx
movabs $0x3ff0000000000000,%rax
mov %rax,(%rcx,%rsi,1)
movq $0x0,(%r8,%rsi,1)
movsd (%rcx,%rsi,1),%xmm8
mov -0x78(%rsp),%r8
movsd %xmm8,(%r8,%rsi,1)
mov -0x50(%rsp),%r8
cmp %r9,%rdx
mov -0x78(%rsp),%r9
jae 17fb720 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x530>
add %r8,%r13
lea (%r9,%rbx,1),%rdx
add $0x3e80,%rdx
cmp %r13,%rdx
jae 17fb720 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x530>
xor %r13d,%r13d
xchg %ax,%ax
movsd -0x8(%r15,%r13,8),%xmm8
mulsd %xmm2,%xmm8
addsd %xmm0,%xmm8
movapd %xmm4,%xmm9
divsd %xmm8,%xmm9
movsd %xmm9,(%r15,%r13,8)
movsd -0x3e80(%r12,%r13,8),%xmm9
mulsd %xmm7,%xmm9
movsd -0x7d00(%r12,%r13,8),%xmm10
mulsd %xmm5,%xmm10
addsd %xmm9,%xmm10
movsd (%r12,%r13,8),%xmm9
mulsd %xmm5,%xmm9
addsd %xmm10,%xmm9
movsd -0x8(%r14,%r13,8),%xmm10
mulsd %xmm4,%xmm10
addsd %xmm9,%xmm10
divsd %xmm8,%xmm10
movsd %xmm10,(%r14,%r13,8)
inc %r13
cmp %r13,%rbp
jne 17fb6a0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x4b0>
jmp 17fb7af <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x5bf>
nopl (%rax)
movsd 0x3e80(%r8,%rbx,1),%xmm9
movsd 0x3e80(%r9,%rbx,1),%xmm8
xor %r13d,%r13d
nopw 0x0(%rax,%rax,1)
mulsd %xmm2,%xmm9
addsd %xmm0,%xmm9
movapd %xmm4,%xmm10
divsd %xmm9,%xmm10
movsd %xmm10,(%r15,%r13,8)
movsd -0x3e80(%r12,%r13,8),%xmm11
mulsd %xmm7,%xmm11
movsd -0x7d00(%r12,%r13,8),%xmm12
mulsd %xmm5,%xmm12
addsd %xmm11,%xmm12
movsd (%r12,%r13,8),%xmm11
mulsd %xmm5,%xmm11
addsd %xmm12,%xmm11
mulsd %xmm4,%xmm8
addsd %xmm11,%xmm8
divsd %xmm9,%xmm8
movsd %xmm8,(%r14,%r13,8)
inc %r13
movapd %xmm10,%xmm9
cmp %r13,%rbp
jne 17fb740 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x550>
add -0x60(%rsp),%rsi
mov -0x10(%rsp),%rcx
movabs $0x3ff0000000000000,%rax
mov %rax,(%rsi,%rcx,8)
mov 0x28(%rsp),%rcx
movsd (%rcx,%rbx,1),%xmm8
mov -0x18(%rsp),%rsi
nopw 0x0(%rax,%rax,1)
mulsd (%r11,%rsi,8),%xmm8
addsd (%r10,%rsi,8),%xmm8
movsd %xmm8,(%rdi,%rsi,8)
cmp $0x1,%rsi
lea -0x1(%rsi),%rsi
jg 17fb7e0 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x5f0>
mov -0x68(%rsp),%rax
inc %rax
mov -0x70(%rsp),%rcx
inc %rcx
add $0x3e80,%r12
add $0x3e80,%r15
add $0x3e80,%r14
add $0x3e80,%r11
add $0x3e80,%r10
add $0x3e80,%rdi
cmp -0x8(%rsp),%rax
jne 17fb610 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x420>
jmp 17fb360 <_ZL15__pb_adi_kerneliiPA2000_dS0_S0_S0_+0x170>
add $0x30,%rsp
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