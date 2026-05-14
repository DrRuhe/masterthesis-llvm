<kernel_ludcmp>:
lea 0x2435439(%rip),%rax # 3c2e850 <g_ludcmp_A>
mov (%rax),%rsi
lea 0x243543f(%rip),%rdx # 3c2e860 <g_ludcmp_b>
lea 0x243d138(%rip),%rcx # 3c36560 <g_ludcmp_x>
lea 0x2444e31(%rip),%r8 # 3c3e260 <g_ludcmp_y>
jmp 17f9020 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_>
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
<_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_>:
test %edi,%edi
jle 17f9317 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x2f7>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
mov %rsi,%rax
mov %rcx,-0x8(%rsp)
mov %edi,%ecx
lea 0x8(%rsi),%r9
xor %r10d,%r10d
mov %rsi,%r11
jmp 17f9067 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x47>
nopl 0x0(%rax,%rax,1)
inc %r10
add $0x7d00,%r9
add $0x8,%r11
cmp %rcx,%r10
je 17f91e7 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x1c7>
test %r10,%r10
je 17f9050 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x30>
mov %r10,%rbx
and $0xfffffffffffffffe,%rbx
imul $0x7d00,%r10,%r14
add %rax,%r14
mov %rax,%rsi
xor %r12d,%r12d
jmp 17f90ba <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x9a>
data16 cs nopw 0x0(%rax,%rax,1)
lea (%r14,%r12,8),%r15
imul $0x7d00,%r12,%r13
add %rax,%r13
divsd 0x0(%r13,%r12,8),%xmm0
movsd %xmm0,(%r15)
inc %r12
add $0x8,%rsi
cmp %r10,%r12
je 17f9140 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x120>
movsd (%r14,%r12,8),%xmm0
test %r12,%r12
je 17f9090 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x70>
cmp $0x1,%r12
jne 17f90d0 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0xb0>
xor %r13d,%r13d
jmp 17f9113 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0xf3>
mov %r12,%rbp
and $0xfffffffffffffffe,%rbp
mov %rsi,%r15
xor %r13d,%r13d
nopl (%rax)
movsd -0x8(%r9,%r13,8),%xmm1
mulsd (%r15),%xmm1
movsd (%r9,%r13,8),%xmm2
subsd %xmm1,%xmm0
mulsd 0x7d00(%r15),%xmm2
subsd %xmm2,%xmm0
add $0x2,%r13
add $0xfa00,%r15
cmp %r13,%rbp
jne 17f90e0 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0xc0>
test $0x1,%r12b
je 17f9090 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x70>
movsd (%r14,%r13,8),%xmm1
imul $0x7d00,%r13,%r15
add %rax,%r15
mulsd (%r15,%r12,8),%xmm1
subsd %xmm1,%xmm0
jmp 17f9090 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x70>
nopl 0x0(%rax)
mov %r11,%r15
mov %r10,%r12
jmp 17f9168 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x148>
nopl 0x0(%rax,%rax,1)
lea (%r14,%r12,8),%rsi
movsd %xmm0,(%rsi)
inc %r12
add $0x8,%r15
cmp %rcx,%r12
je 17f9050 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x30>
movsd (%r14,%r12,8),%xmm0
cmp $0x1,%r10
jne 17f9180 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x160>
xor %r13d,%r13d
jmp 17f91c2 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x1a2>
nopl 0x0(%rax)
mov %r15,%rbp
xor %r13d,%r13d
cs nopw 0x0(%rax,%rax,1)
movsd -0x8(%r9,%r13,8),%xmm1
mulsd 0x0(%rbp),%xmm1
movsd (%r9,%r13,8),%xmm2
subsd %xmm1,%xmm0
mulsd 0x7d00(%rbp),%xmm2
subsd %xmm2,%xmm0
add $0x2,%r13
add $0xfa00,%rbp
cmp %r13,%rbx
jne 17f9190 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x170>
test $0x1,%r10b
je 17f9150 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x130>
movsd (%r14,%r13,8),%xmm1
imul $0x7d00,%r13,%rsi
add %rax,%rsi
mulsd (%rsi,%r12,8),%xmm1
subsd %xmm1,%xmm0
jmp 17f9150 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x130>
lea 0x8(%rax),%r9
xor %r10d,%r10d
mov -0x8(%rsp),%rbx
jmp 17f9215 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x1f5>
data16 cs nopw 0x0(%rax,%rax,1)
movsd %xmm0,(%r8,%r10,8)
inc %r10
add $0x7d00,%r9
cmp %rcx,%r10
je 17f9290 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x270>
movsd (%rdx,%r10,8),%xmm0
test %r10,%r10
je 17f9200 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x1e0>
cmp $0x1,%r10
jne 17f9230 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x210>
xor %r11d,%r11d
jmp 17f926b <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x24b>
nopl 0x0(%rax,%rax,1)
mov %r10,%rsi
and $0xfffffffffffffffe,%rsi
xor %r11d,%r11d
nopw 0x0(%rax,%rax,1)
movsd -0x8(%r9,%r11,8),%xmm1
movsd (%r9,%r11,8),%xmm2
mulsd (%r8,%r11,8),%xmm1
mulsd 0x8(%r8,%r11,8),%xmm2
subsd %xmm1,%xmm0
subsd %xmm2,%xmm0
add $0x2,%r11
cmp %r11,%rsi
jne 17f9240 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x220>
test $0x1,%r10b
je 17f9200 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x1e0>
imul $0x7d00,%r10,%rsi
add %rax,%rsi
movsd (%rsi,%r11,8),%xmm1
mulsd (%r8,%r11,8),%xmm1
subsd %xmm1,%xmm0
jmp 17f9200 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x1e0>
lea (%rbx,%rcx,8),%rdx
imul $0x7d08,%rcx,%rsi
lea (%rsi,%rax,1),%r9
add $0xffffffffffff8300,%r9
mov %rcx,%r10
jmp 17f92dd <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x2bd>
nopl 0x0(%rax,%rax,1)
lea -0x1(%r10),%rsi
imul $0x7d00,%rsi,%r11
add %rax,%r11
divsd (%r11,%rsi,8),%xmm0
movsd %xmm0,(%rbx,%rsi,8)
add $0xfffffffffffffff8,%rdx
add $0xffffffffffff82f8,%r9
cmp $0x1,%r10
mov %rsi,%r10
jbe 17f930d <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x2ed>
movsd -0x8(%r8,%r10,8),%xmm0
cmp %rcx,%r10
jge 17f92b0 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x290>
xor %esi,%esi
nopl 0x0(%rax,%rax,1)
movsd (%r9,%rsi,8),%xmm1
mulsd (%rdx,%rsi,8),%xmm1
subsd %xmm1,%xmm0
inc %rsi
lea (%r10,%rsi,1),%r11d
cmp %r11d,%edi
jg 17f92f0 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x2d0>
jmp 17f92b0 <_ZL18__pb_ludcmp_kerneliPA4000_dPdS1_S1_+0x290>
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