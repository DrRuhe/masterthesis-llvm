<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_multi_key_sort_tradeoff_specializediiiE3$_0JPvlEEET_PT0_DpT1_>:
jmp 4fafd30 <_ZNK14MultiKeySorter4sortEPvl>
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
<_ZNK14MultiKeySorter4sortEPvl>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x568,%rsp
mov %rsi,0x8(%rsp)
cmp $0x2,%rdx
jge 4fafd5e <_ZNK14MultiKeySorter4sortEPvl+0x2e>
add $0x568,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
movslq (%rdi),%r15
dec %rdx
movq $0x0,0x160(%rsp)
mov %rdx,0x168(%rsp)
mov %r15,%rax
neg %rax
mov %rax,0x58(%rsp)
mov $0x1,%ecx
mov %rdi,0x10(%rsp)
mov %r15,0x30(%rsp)
jmp 4fafdab <_ZNK14MultiKeySorter4sortEPvl+0x7b>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x40(%rsp),%rdx
mov %edx,%ecx
test %edx,%edx
jle 4fafd4c <_ZNK14MultiKeySorter4sortEPvl+0x1c>
lea -0x1(%rcx),%ebx
mov %rbx,0x40(%rsp)
shl $0x4,%rbx
mov 0x160(%rsp,%rbx,1),%rbp
mov 0x168(%rsp,%rbx,1),%rax
mov %rax,%r13
sub %rbp,%r13
jle 4fafda0 <_ZNK14MultiKeySorter4sortEPvl+0x70>
cmp $0xf,%r13
mov %rax,0x18(%rsp)
jg 4faff20 <_ZNK14MultiKeySorter4sortEPvl+0x1f0>
mov %r15,%rdx
imul %rbp,%rdx
lea 0x1(%rbp),%rsi
imul %r15,%rsi
mov 0x8(%rsp),%rcx
mov %rcx,0x48(%rsp)
mov %rbp,%rcx
mov %rdx,0x28(%rsp)
mov %rsi,0x20(%rsp)
jmp 4fafe2d <_ZNK14MultiKeySorter4sortEPvl+0xfd>
cs nopw 0x0(%rax,%rax,1)
mov 0x30(%rsp),%r15
add %r15,0x48(%rsp)
mov 0x18(%rsp),%rax
mov 0x50(%rsp),%rcx
cmp %rax,%rcx
je 4fafda0 <_ZNK14MultiKeySorter4sortEPvl+0x70>
cmp %rbp,%rcx
lea 0x1(%rcx),%rax
mov %rax,0x50(%rsp)
jl 4fafe10 <_ZNK14MultiKeySorter4sortEPvl+0xe0>
mov 0x48(%rsp),%r12
mov 0x50(%rsp),%r15
data16 cs nopw 0x0(%rax,%rax,1)
movslq 0x4(%rdi),%rax
lea (%rdx,%rax,1),%rcx
movsd (%r12,%rcx,1),%xmm0
add %rsi,%rax
movsd (%r12,%rax,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fafe6f <_ZNK14MultiKeySorter4sortEPvl+0x13f>
jnp 4fafe80 <_ZNK14MultiKeySorter4sortEPvl+0x150>
seta %cl
ucomisd %xmm0,%xmm1
seta %al
test %cl,%cl
jne 4fafead <_ZNK14MultiKeySorter4sortEPvl+0x17d>
jmp 4fafe10 <_ZNK14MultiKeySorter4sortEPvl+0xe0>
nop
movslq 0x8(%rdi),%rax
lea (%rdx,%rax,1),%rcx
movsd (%r12,%rcx,1),%xmm0
add %rsi,%rax
movsd (%r12,%rax,1),%xmm1
ucomisd %xmm0,%xmm1
seta %cl
ucomisd %xmm1,%xmm0
seta %al
test %cl,%cl
je 4fafe10 <_ZNK14MultiKeySorter4sortEPvl+0xe0>
test %al,%al
jne 4fafe10 <_ZNK14MultiKeySorter4sortEPvl+0xe0>
dec %r15
lea (%r12,%rdx,1),%rbx
lea (%r12,%rsi,1),%r13
lea 0x60(%rsp),%rdi
mov %rbx,%rsi
mov %rbp,%r14
mov 0x30(%rsp),%rbp
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
mov %r13,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
lea 0x60(%rsp),%rsi
mov %rbp,%rdx
mov %r14,%rbp
call 4da5c70 <memcpy$plt>
mov 0x20(%rsp),%rsi
mov 0x28(%rsp),%rdx
mov 0x10(%rsp),%rdi
add 0x58(%rsp),%r12
cmp %r14,%r15
jg 4fafe50 <_ZNK14MultiKeySorter4sortEPvl+0x120>
jmp 4fafe10 <_ZNK14MultiKeySorter4sortEPvl+0xe0>
nopl 0x0(%rax,%rax,1)
mov %rcx,0x28(%rsp)
shr $1,%r13
add %rbp,%r13
mov %rbp,%rsi
imul %r15,%rsi
mov 0x8(%rsp),%rax
add %rax,%rsi
imul %r15,%r13
add %rax,%r13
movslq 0x4(%rdi),%rax
movsd (%rax,%rsi,1),%xmm0
movsd (%rax,%r13,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4faff58 <_ZNK14MultiKeySorter4sortEPvl+0x228>
jnp 4faff6d <_ZNK14MultiKeySorter4sortEPvl+0x23d>
seta %cl
ucomisd %xmm0,%xmm1
seta %dl
test %cl,%cl
mov %rbp,0x38(%rsp)
jne 4faff94 <_ZNK14MultiKeySorter4sortEPvl+0x264>
jmp 4faffd9 <_ZNK14MultiKeySorter4sortEPvl+0x2a9>
movslq 0x8(%rdi),%rcx
movsd (%rsi,%rcx,1),%xmm1
movsd 0x0(%r13,%rcx,1),%xmm2
ucomisd %xmm1,%xmm2
seta %cl
ucomisd %xmm2,%xmm1
seta %dl
test %cl,%cl
mov %rbp,0x38(%rsp)
je 4faffd9 <_ZNK14MultiKeySorter4sortEPvl+0x2a9>
test %dl,%dl
jne 4faffd9 <_ZNK14MultiKeySorter4sortEPvl+0x2a9>
lea 0x60(%rsp),%r14
mov %r14,%rdi
mov %rsi,%rbp
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r14,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rsi
mov 0x10(%rsp),%rdi
movslq 0x4(%rdi),%rax
movsd 0x0(%rbp,%rax,1),%xmm0
mov 0x18(%rsp),%rbp
imul %r15,%rbp
add 0x8(%rsp),%rbp
movsd (%rax,%rbp,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fafff4 <_ZNK14MultiKeySorter4sortEPvl+0x2c4>
jnp 4fb0004 <_ZNK14MultiKeySorter4sortEPvl+0x2d4>
seta %cl
ucomisd %xmm0,%xmm1
seta %dl
test %cl,%cl
jne 4fb0025 <_ZNK14MultiKeySorter4sortEPvl+0x2f5>
jmp 4fb0067 <_ZNK14MultiKeySorter4sortEPvl+0x337>
movslq 0x8(%rdi),%rcx
movsd (%rsi,%rcx,1),%xmm0
movsd 0x0(%rbp,%rcx,1),%xmm2
ucomisd %xmm0,%xmm2
seta %cl
ucomisd %xmm2,%xmm0
seta %dl
test %cl,%cl
je 4fb0067 <_ZNK14MultiKeySorter4sortEPvl+0x337>
test %dl,%dl
jne 4fb0067 <_ZNK14MultiKeySorter4sortEPvl+0x337>
lea 0x60(%rsp),%r12
mov %r12,%rdi
mov %rsi,%r14
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r12,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov 0x10(%rsp),%rdi
movslq 0x4(%rdi),%rax
movsd 0x0(%rbp,%rax,1),%xmm1
movsd 0x0(%r13,%rax,1),%xmm0
ucomisd %xmm1,%xmm0
jne 4fb0076 <_ZNK14MultiKeySorter4sortEPvl+0x346>
jnp 4fb0086 <_ZNK14MultiKeySorter4sortEPvl+0x356>
seta %cl
ucomisd %xmm0,%xmm1
seta %al
test %cl,%cl
jne 4fb00a9 <_ZNK14MultiKeySorter4sortEPvl+0x379>
jmp 4fb00dc <_ZNK14MultiKeySorter4sortEPvl+0x3ac>
movslq 0x8(%rdi),%rax
movsd 0x0(%r13,%rax,1),%xmm0
movsd 0x0(%rbp,%rax,1),%xmm1
ucomisd %xmm0,%xmm1
seta %cl
ucomisd %xmm1,%xmm0
seta %al
test %cl,%cl
je 4fb00dc <_ZNK14MultiKeySorter4sortEPvl+0x3ac>
test %al,%al
jne 4fb00dc <_ZNK14MultiKeySorter4sortEPvl+0x3ac>
lea 0x60(%rsp),%r14
mov %r14,%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r14,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
lea (%rsp,%rbx,1),%rax
add $0x160,%rax
mov %rax,0x20(%rsp)
mov 0x18(%rsp),%rax
lea -0x1(%rax),%r14
mov %r14,%rbp
imul %r15,%rbp
add 0x8(%rsp),%rbp
lea 0x60(%rsp),%rbx
mov %rbx,%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %rbx,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%r10
mov 0x38(%rsp),%r12
nopw 0x0(%rax,%rax,1)
mov 0x10(%rsp),%rax
movslq 0x4(%rax),%rcx
movsd 0x0(%rbp,%rcx,1),%xmm0
movslq 0x8(%rax),%rax
lea 0x1(%r12),%r14
mov %r15,%rsi
imul %r14,%rsi
lea (%rsi,%rax,1),%rdi
add %rcx,%rsi
mov 0x8(%rsp),%r13
nopl 0x0(%rax,%rax,1)
movsd 0x0(%r13,%rsi,1),%xmm1
ucomisd %xmm0,%xmm1
jne 4fb017f <_ZNK14MultiKeySorter4sortEPvl+0x44f>
jnp 4fb0190 <_ZNK14MultiKeySorter4sortEPvl+0x460>
seta %r8b
ucomisd %xmm1,%xmm0
jmp 4fb01a9 <_ZNK14MultiKeySorter4sortEPvl+0x479>
nopl 0x0(%rax)
movsd 0x0(%r13,%rdi,1),%xmm1
movsd 0x0(%rbp,%rax,1),%xmm2
ucomisd %xmm1,%xmm2
seta %r8b
ucomisd %xmm2,%xmm1
seta %r9b
inc %r14
add %r15,%r13
test %r9b,%r9b
je 4fb01bd <_ZNK14MultiKeySorter4sortEPvl+0x48d>
test %r8b,%r8b
je 4fb0170 <_ZNK14MultiKeySorter4sortEPvl+0x440>
lea -0x1(%r14),%r9
imul %r15,%r12
add %r12,%r13
lea -0x1(%r10),%rdx
imul %r15,%rdx
lea (%rdx,%rax,1),%rsi
add %rcx,%rdx
mov 0x8(%rsp),%rbx
mov %r10,%r15
mov 0x58(%rsp),%r8
data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd (%rbx,%rdx,1),%xmm1
ucomisd %xmm0,%xmm1
jne 4fb01fd <_ZNK14MultiKeySorter4sortEPvl+0x4cd>
jnp 4fb0210 <_ZNK14MultiKeySorter4sortEPvl+0x4e0>
seta %dil
ucomisd %xmm1,%xmm0
jmp 4fb0227 <_ZNK14MultiKeySorter4sortEPvl+0x4f7>
nopw 0x0(%rax,%rax,1)
movsd (%rbx,%rsi,1),%xmm1
movsd 0x0(%rbp,%rax,1),%xmm2
ucomisd %xmm1,%xmm2
seta %dil
ucomisd %xmm2,%xmm1
seta %cl
add %r8,%rbx
dec %r15
test %dil,%dil
je 4fb0239 <_ZNK14MultiKeySorter4sortEPvl+0x509>
test %cl,%cl
je 4fb01f0 <_ZNK14MultiKeySorter4sortEPvl+0x4c0>
cmp %r15,%r9
jge 4fb0290 <_ZNK14MultiKeySorter4sortEPvl+0x560>
mov 0x30(%rsp),%r14
imul %r14,%r10
add %r10,%rbx
lea 0x60(%rsp),%rdi
mov %r13,%rsi
mov %r14,%rdx
mov %r9,%r12
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
lea 0x60(%rsp),%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r15,%r10
mov %r14,%r15
jmp 4fb0140 <_ZNK14MultiKeySorter4sortEPvl+0x410>
cs nopw 0x0(%rax,%rax,1)
lea 0x60(%rsp),%rbx
mov %rbx,%rdi
mov %r13,%rsi
mov 0x30(%rsp),%r15
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %rbx,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%r14),%rax
mov %rax,%rcx
mov 0x38(%rsp),%rsi
sub %rsi,%rcx
mov 0x18(%rsp),%r8
mov %r8,%rdx
sub %r14,%rdx
cmp %rdx,%rcx
jle 4fb031a <_ZNK14MultiKeySorter4sortEPvl+0x5ea>
mov 0x20(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %r14,%r8
mov 0x10(%rsp),%rdi
mov 0x28(%rsp),%rcx
jle 4fb034e <_ZNK14MultiKeySorter4sortEPvl+0x61e>
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %r14,0x160(%rsp,%rax,1)
mov %r8,0x168(%rsp,%rax,1)
mov %ecx,%edx
jmp 4fafda5 <_ZNK14MultiKeySorter4sortEPvl+0x75>
mov 0x20(%rsp),%rcx
mov %r14,(%rcx)
cmp %rax,%rsi
mov 0x10(%rsp),%rdi
mov 0x28(%rsp),%rdx
jge 4fb0349 <_ZNK14MultiKeySorter4sortEPvl+0x619>
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %rsi,0x160(%rsp,%rcx,1)
mov %rax,0x168(%rsp,%rcx,1)
jmp 4fafda5 <_ZNK14MultiKeySorter4sortEPvl+0x75>
mov %ecx,%edx
jmp 4fafda5 <_ZNK14MultiKeySorter4sortEPvl+0x75>
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
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x24f734c(%rip) # 729cfc8 <memcpy@GLIBC_2.14>
int3
int3
int3
int3