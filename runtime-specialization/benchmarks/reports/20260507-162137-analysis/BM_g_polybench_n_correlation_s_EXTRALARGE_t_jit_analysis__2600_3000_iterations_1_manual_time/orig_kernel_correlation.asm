<kernel_correlation>:
cvtsi2sd %esi,%xmm0
lea 0x2396035(%rip),%rax # 3b84910 <g_corr_data>
mov (%rax),%rdx
lea 0x2396033(%rip),%rax # 3b84918 <g_corr_corr>
mov (%rax),%rcx
lea 0x2396031(%rip),%r8 # 3b84920 <g_corr_mean>
lea 0x239b16a(%rip),%r9 # 3b89a60 <g_corr_stddev>
jmp 17edf80 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_>
int3
int3
int3
int3
int3
<_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x58,%rsp
mov %r9,%r12
mov %r8,%r13
mov %rcx,0x28(%rsp)
mov %rdx,0x10(%rsp)
movapd %xmm0,%xmm2
mov %esi,0xc(%rsp)
mov %edi,%eax
mov %rax,0x18(%rsp)
mov %rdi,0x30(%rsp)
test %edi,%edi
movsd %xmm0,0x20(%rsp)
jle 17ee21a <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x29a>
mov 0xc(%rsp),%ebx
test %ebx,%ebx
jle 17ee43f <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x4bf>
mov %ebx,%r15d
mov %r15d,%eax
and $0x3,%eax
mov %r15d,%ecx
and $0x7ffffffc,%ecx
imul $0x5140,%rax,%rdx
xor %esi,%esi
mov 0x10(%rsp),%rdi
mov 0x18(%rsp),%r10
jmp 17ee01b <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x9b>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
divsd %xmm2,%xmm0
movsd %xmm0,0x0(%r13,%rsi,8)
inc %rsi
add $0x8,%rdi
cmp %r10,%rsi
je 17ee0de <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x15e>
movq $0x0,0x0(%r13,%rsi,8)
xorpd %xmm0,%xmm0
cmp $0x4,%ebx
jae 17ee040 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0xc0>
xor %r8d,%r8d
jmp 17ee09c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x11c>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rdi,%r9
xor %r8d,%r8d
cs nopw 0x0(%rax,%rax,1)
addsd (%r9),%xmm0
movsd %xmm0,0x0(%r13,%rsi,8)
addsd 0x5140(%r9),%xmm0
movsd %xmm0,0x0(%r13,%rsi,8)
addsd 0xa280(%r9),%xmm0
movsd %xmm0,0x0(%r13,%rsi,8)
addsd 0xf3c0(%r9),%xmm0
movsd %xmm0,0x0(%r13,%rsi,8)
add $0x4,%r8
add $0x14500,%r9
cmp %r8,%rcx
jne 17ee050 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0xd0>
test %rax,%rax
je 17ee000 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x80>
imul $0x5140,%r8,%r8
add %rdi,%r8
xor %r9d,%r9d
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
addsd (%r8,%r9,1),%xmm0
movsd %xmm0,0x0(%r13,%rsi,8)
add $0x5140,%r9
cmp %r9,%rdx
jne 17ee0c0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x140>
jmp 17ee000 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x80>
mov %r15d,%ebx
and $0x7ffffffe,%ebx
xor %r14d,%r14d
xorpd %xmm3,%xmm3
movsd -0x2cd926(%rip),%xmm4 # 15207d0 <.LCPI103_0>
movsd -0x2cd576(%rip),%xmm5 # 1520b88 <.LCPI0_0>
mov 0x10(%rsp),%rbp
jmp 17ee15c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x1dc>
data16 cs nopw 0x0(%rax,%rax,1)
call 17d6500 <sqrt$plt>
movsd -0x2cd595(%rip),%xmm5 # 1520b88 <.LCPI0_0>
movsd -0x2cd955(%rip),%xmm4 # 15207d0 <.LCPI103_0>
xorpd %xmm3,%xmm3
movsd 0x20(%rsp),%xmm2
movapd %xmm0,%xmm1
cmpnlesd %xmm4,%xmm1
andpd %xmm1,%xmm0
andnpd %xmm5,%xmm1
orpd %xmm0,%xmm1
movlpd %xmm1,(%r12,%r14,8)
inc %r14
add $0x8,%rbp
cmp 0x18(%rsp),%r14
je 17ee21a <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x29a>
movq $0x0,(%r12,%r14,8)
cmpl $0x1,0xc(%rsp)
jne 17ee180 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x200>
xorpd %xmm0,%xmm0
xor %eax,%eax
jmp 17ee1d6 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x256>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
xorpd %xmm0,%xmm0
mov %rbp,%rcx
xor %eax,%eax
nopl 0x0(%rax)
movsd (%rcx),%xmm1
subsd 0x0(%r13,%r14,8),%xmm1
mulsd %xmm1,%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r12,%r14,8)
movsd 0x5140(%rcx),%xmm0
subsd 0x0(%r13,%r14,8),%xmm0
mulsd %xmm0,%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r12,%r14,8)
add $0x2,%rax
add $0xa280,%rcx
cmp %rax,%rbx
jne 17ee190 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x210>
test $0x1,%r15b
je 17ee203 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x283>
imul $0x5140,%rax,%rax
add 0x10(%rsp),%rax
movsd (%rax,%r14,8),%xmm1
subsd 0x0(%r13,%r14,8),%xmm1
mulsd %xmm1,%xmm1
addsd %xmm1,%xmm0
movsd %xmm0,(%r12,%r14,8)
divsd %xmm2,%xmm0
ucomisd %xmm3,%xmm0
jb 17ee110 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x190>
sqrtsd %xmm0,%xmm0
jmp 17ee12f <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x1af>
mov 0xc(%rsp),%ebx
test %ebx,%ebx
jle 17ee416 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x496>
mov 0x30(%rsp),%rax
test %eax,%eax
mov 0x18(%rsp),%rbp
jle 17ee45b <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x4db>
mov %ebx,%eax
mov %rax,0x50(%rsp)
mov %ebp,%ebx
and $0x7ffffffe,%ebx
mov 0x10(%rsp),%rax
lea 0x8(%rax),%r14
movq $0x0,0x40(%rsp)
xorps %xmm1,%xmm1
sqrtsd %xmm2,%xmm1
xorpd %xmm3,%xmm3
movsd %xmm1,0x48(%rsp)
jmp 17ee2ad <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x32d>
nopl 0x0(%rax)
lea 0x0(,%r15,8),%rax
add %rbp,%rax
mulsd (%r12,%r15,8),%xmm0
divsd %xmm0,%xmm4
movsd %xmm4,(%rax)
mov 0x18(%rsp),%rbp
mov 0x40(%rsp),%rcx
inc %rcx
add $0x5140,%r14
mov %rcx,0x40(%rsp)
cmp 0x50(%rsp),%rcx
je 17ee3ee <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x46e>
cmpl $0x1,0x30(%rsp)
jne 17ee320 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x3a0>
xor %r15d,%r15d
test $0x1,%bpl
je 17ee28e <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x30e>
imul $0x5140,0x40(%rsp),%rbp
add 0x10(%rsp),%rbp
movsd 0x0(%rbp,%r15,8),%xmm4
subsd 0x0(%r13,%r15,8),%xmm4
movsd %xmm4,0x0(%rbp,%r15,8)
ucomisd %xmm3,%xmm2
movapd %xmm1,%xmm0
jae 17ee270 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x2f0>
movapd %xmm2,%xmm0
movsd %xmm4,0x38(%rsp)
call 17d6500 <sqrt$plt>
movsd 0x38(%rsp),%xmm4
xorpd %xmm3,%xmm3
movsd 0x48(%rsp),%xmm1
movsd 0x20(%rsp),%xmm2
jmp 17ee270 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x2f0>
data16 data16 cs nopw 0x0(%rax,%rax,1)
xor %r15d,%r15d
jmp 17ee34e <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x3ce>
data16 cs nopw 0x0(%rax,%rax,1)
mulsd 0x8(%r12,%r15,8),%xmm0
divsd %xmm0,%xmm4
movsd %xmm4,(%r14,%r15,8)
add $0x2,%r15
cmp %r15,%rbx
je 17ee2b7 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x337>
movsd -0x8(%r14,%r15,8),%xmm4
subsd 0x0(%r13,%r15,8),%xmm4
movsd %xmm4,-0x8(%r14,%r15,8)
ucomisd %xmm3,%xmm2
movapd %xmm1,%xmm0
jae 17ee392 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x412>
movapd %xmm2,%xmm0
movsd %xmm4,0x38(%rsp)
call 17d6500 <sqrt$plt>
movsd 0x38(%rsp),%xmm4
xorpd %xmm3,%xmm3
movsd 0x48(%rsp),%xmm1
movsd 0x20(%rsp),%xmm2
mulsd (%r12,%r15,8),%xmm0
divsd %xmm0,%xmm4
movsd %xmm4,-0x8(%r14,%r15,8)
movsd (%r14,%r15,8),%xmm4
subsd 0x8(%r13,%r15,8),%xmm4
movsd %xmm4,(%r14,%r15,8)
ucomisd %xmm3,%xmm2
movapd %xmm1,%xmm0
jae 17ee330 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x3b0>
movapd %xmm2,%xmm0
movsd %xmm4,0x38(%rsp)
call 17d6500 <sqrt$plt>
movsd 0x38(%rsp),%xmm4
xorpd %xmm3,%xmm3
movsd 0x48(%rsp),%xmm1
movsd 0x20(%rsp),%xmm2
jmp 17ee330 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x3b0>
mov $0x1,%dl
movabs $0x3ff0000000000000,%r15
mov 0xc(%rsp),%ebx
mov 0x30(%rsp),%r14
lea -0x1(%r14),%eax
cmp $0x2,%r14d
jge 17ee568 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x5e8>
jmp 17ee77c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7fc>
xor %edx,%edx
mov 0x18(%rsp),%rbp
mov 0x30(%rsp),%r14
movabs $0x3ff0000000000000,%r15
lea -0x1(%r14),%eax
cmp $0x2,%r14d
jge 17ee568 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x5e8>
jmp 17ee77c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7fc>
xorpd %xmm0,%xmm0
divsd %xmm2,%xmm0
mov 0x30(%rsp),%r14
cmp $0x4,%r14d
mov 0x18(%rsp),%rbp
jae 17ee46c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x4ec>
xor %eax,%eax
jmp 17ee4b0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x530>
dec %eax
movabs $0x3ff0000000000000,%r15
jmp 17ee77c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7fc>
mov %ebp,%eax
and $0x7ffffffc,%eax
movapd %xmm0,%xmm1
unpcklpd %xmm0,%xmm1
mov %ebp,%ecx
shr $0x2,%ecx
and $0x1fffffff,%ecx
shl $0x5,%rcx
xor %edx,%edx
nopl 0x0(%rax)
movupd %xmm1,0x0(%r13,%rdx,1)
movupd %xmm1,0x10(%r13,%rdx,1)
add $0x20,%rdx
cmp %rdx,%rcx
jne 17ee490 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x510>
cmp %ebp,%eax
je 17ee4bf <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x53f>
nopl 0x0(%rax,%rax,1)
movsd %xmm0,0x0(%r13,%rax,8)
inc %rax
cmp %rax,%rbp
jne 17ee4b0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x530>
xorpd %xmm1,%xmm1
xorpd %xmm0,%xmm0
divsd %xmm2,%xmm0
ucomisd %xmm1,%xmm0
jb 17ee4d7 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x557>
sqrtsd %xmm0,%xmm0
jmp 17ee4dc <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x55c>
call 17d6500 <sqrt$plt>
movapd %xmm0,%xmm1
cmpnlesd -0x2cdd19(%rip),%xmm1 # 15207d0 <.LCPI103_0>
andpd %xmm1,%xmm0
movsd -0x2cd96d(%rip),%xmm2 # 1520b88 <.LCPI0_0>
andnpd %xmm2,%xmm1
orpd %xmm0,%xmm1
cmp $0x4,%r14d
jae 17ee507 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x587>
xor %eax,%eax
jmp 17ee540 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x5c0>
mov %ebp,%eax
and $0x7ffffffc,%eax
movapd %xmm1,%xmm0
unpcklpd %xmm1,%xmm0
xor %ecx,%ecx
nopl 0x0(%rax,%rax,1)
movupd %xmm0,(%r12,%rcx,8)
movupd %xmm0,0x10(%r12,%rcx,8)
add $0x4,%rcx
cmp %rcx,%rax
jne 17ee520 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x5a0>
cmp %ebp,%eax
je 17ee54e <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x5ce>
nopw 0x0(%rax,%rax,1)
movsd %xmm1,(%r12,%rax,8)
inc %rax
cmp %rax,%rbp
jne 17ee540 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x5c0>
xor %edx,%edx
movabs $0x3ff0000000000000,%r15
lea -0x1(%r14),%eax
cmp $0x2,%r14d
jl 17ee77c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7fc>
mov %eax,%ecx
test %dl,%dl
je 17ee6a8 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x728>
mov %eax,0x20(%rsp)
mov %ebx,%edx
mov %edx,%esi
and $0x7ffffffe,%esi
mov $0x1,%edi
mov $0x5148,%r8d
xor %r9d,%r9d
xor %r10d,%r10d
jmp 17ee5ba <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x63a>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %rdi
add $0x8,%r9
add $0x8,%r8
mov %r11,%r10
cmp %rcx,%r11
mov %rax,%r15
je 17ee778 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7f8>
imul $0x5140,%r10,%rbx
add 0x28(%rsp),%rbx
mov %r15,%rax
mov %r15,(%rbx,%r10,8)
lea 0x1(%r10),%r11
mov %r8,%r14
mov %rdi,%r15
jmp 17ee600 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x680>
nopl 0x0(%rax)
imul $0x5140,%r15,%r12
add 0x28(%rsp),%r12
movsd %xmm0,(%r12,%r10,8)
inc %r15
add $0x8,%r14
cmp 0x18(%rsp),%r15
je 17ee5a0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x620>
lea (%rbx,%r15,8),%r12
movq $0x0,(%rbx,%r15,8)
cmpl $0x1,0xc(%rsp)
jne 17ee620 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x6a0>
xorpd %xmm0,%xmm0
xor %r13d,%r13d
jmp 17ee676 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x6f6>
nopl 0x0(%rax)
xorpd %xmm0,%xmm0
mov 0x10(%rsp),%rbp
xor %r13d,%r13d
nopl 0x0(%rax)
movsd 0x0(%rbp,%r9,1),%xmm1
mulsd -0x5140(%rbp,%r14,1),%xmm1
addsd %xmm0,%xmm1
movsd %xmm1,(%r12)
movsd 0x5140(%rbp,%r9,1),%xmm0
mulsd 0x0(%rbp,%r14,1),%xmm0
addsd %xmm1,%xmm0
movsd %xmm0,(%r12)
add $0x2,%r13
add $0xa280,%rbp
cmp %r13,%rsi
jne 17ee630 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x6b0>
test $0x1,%dl
je 17ee5e0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x660>
imul $0x5140,%r13,%r13
add 0x10(%rsp),%r13
movsd 0x0(%r13,%r10,8),%xmm1
mulsd 0x0(%r13,%r15,8),%xmm1
addsd %xmm1,%xmm0
movsd %xmm0,(%r12)
jmp 17ee5e0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x660>
lea -0x2(%rbp),%rdx
mov 0x28(%rsp),%rdi
lea 0x8(%rdi),%rsi
add $0x5140,%rdi
mov $0x1,%r8d
xor %r9d,%r9d
jmp 17ee6ea <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x76a>
nopw 0x0(%rax,%rax,1)
inc %r9
inc %r8
add $0x5140,%rsi
add $0x8,%rdi
cmp %rcx,%r9
je 17ee77c <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7fc>
imul $0x5140,%r9,%r11
add 0x28(%rsp),%r11
mov %r15,(%r11,%r9,8)
mov %r9d,%ebx
sub %ebp,%ebx
mov %r8,%r10
test $0x1,%bl
jne 17ee727 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7a7>
movq $0x0,(%r11,%r8,8)
imul $0x5140,%r8,%r10
add 0x28(%rsp),%r10
movq $0x0,(%r10,%r9,8)
lea 0x1(%r8),%r10
cmp %r9,%rdx
je 17ee6d0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x750>
imul $0x5140,%r10,%r11
add %rdi,%r11
cs nopw 0x0(%rax,%rax,1)
movq $0x0,-0x8(%rsi,%r10,8)
movq $0x0,-0x5140(%r11)
movq $0x0,(%rsi,%r10,8)
movq $0x0,(%r11)
add $0x2,%r10
add $0xa280,%r11
cmp %r10,%rbp
jne 17ee740 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x7c0>
jmp 17ee6d0 <_ZL23__pb_correlation_kerneliidPA2600_dS0_PdS1_+0x750>
mov 0x20(%rsp),%eax
cltq
imul $0x5140,%rax,%rcx
mov 0x28(%rsp),%rdx
add %rcx,%rdx
mov %r15,(%rdx,%rax,8)
add $0x58,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
<sqrt$plt>:
mov $0xd3,%r11d
jmp *0x23ada8c(%rip) # 3b83f98 <sqrt@GLIBC_2.2.5>
int3
int3
int3
int3