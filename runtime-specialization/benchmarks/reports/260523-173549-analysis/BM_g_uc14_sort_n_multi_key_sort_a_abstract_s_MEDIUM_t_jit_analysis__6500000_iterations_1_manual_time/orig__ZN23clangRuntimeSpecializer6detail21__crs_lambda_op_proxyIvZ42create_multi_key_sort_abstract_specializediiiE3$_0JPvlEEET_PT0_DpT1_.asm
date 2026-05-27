<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_multi_key_sort_abstract_specializediiiE3$_0JPvlEEET_PT0_DpT1_>:
jmp 4fb1a00 <_ZNK19CompositeComparator4sortEPvl>
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
<_ZNK19CompositeComparator4sortEPvl>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x568,%rsp
mov %rsi,(%rsp)
cmp $0x2,%rdx
jge 4fb1a2d <_ZNK19CompositeComparator4sortEPvl+0x2d>
add $0x568,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
movslq 0xc(%rdi),%r14
dec %rdx
movq $0x0,0x160(%rsp)
mov %rdx,0x168(%rsp)
lea 0x9(%rdi),%r8
lea 0x8(%rdi),%rax
mov %rax,0x18(%rsp)
mov %r14,%rax
neg %rax
mov %rax,0x50(%rsp)
mov $0x1,%ecx
lea 0x60(%rsp),%r15
mov %rdi,0x10(%rsp)
mov %r14,0x40(%rsp)
mov %r8,0x8(%rsp)
jmp 4fb1a90 <_ZNK19CompositeComparator4sortEPvl+0x90>
nopl 0x0(%rax,%rax,1)
lea 0x60(%rsp),%r15
mov 0x58(%rsp),%rdx
mov %edx,%ecx
test %edx,%edx
jle 4fb1a1b <_ZNK19CompositeComparator4sortEPvl+0x1b>
lea -0x1(%rcx),%edx
mov %rdx,%rbx
shl $0x4,%rbx
mov 0x160(%rsp,%rbx,1),%r12
mov 0x168(%rsp,%rbx,1),%rax
mov %rax,%r13
sub %r12,%r13
jle 4fb1a8a <_ZNK19CompositeComparator4sortEPvl+0x8a>
cmp $0xf,%r13
mov %r12,0x48(%rsp)
mov %rax,0x38(%rsp)
jg 4fb1c10 <_ZNK19CompositeComparator4sortEPvl+0x210>
mov %rdx,0x58(%rsp)
mov %r14,%rsi
imul %r12,%rsi
lea 0x1(%r12),%r9
imul %r14,%r9
mov (%rsp),%rcx
mov %rcx,0x20(%rsp)
mov %r12,%rcx
mov %rsi,0x30(%rsp)
jmp 4fb1b0d <_ZNK19CompositeComparator4sortEPvl+0x10d>
xchg %ax,%ax
mov 0x40(%rsp),%r14
add %r14,0x20(%rsp)
mov 0x38(%rsp),%rax
mov 0x28(%rsp),%rcx
cmp %rax,%rcx
je 4fb1a80 <_ZNK19CompositeComparator4sortEPvl+0x80>
cmp %r12,%rcx
lea 0x1(%rcx),%rax
mov %rax,0x28(%rsp)
jl 4fb1af0 <_ZNK19CompositeComparator4sortEPvl+0xf0>
mov 0x20(%rsp),%r14
mov 0x28(%rsp),%r15
data16 cs nopw 0x0(%rax,%rax,1)
movslq (%rdi),%rax
lea (%rsi,%rax,1),%rcx
movsd (%r14,%rcx,1),%xmm0
add %r9,%rax
movsd (%r14,%rax,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fb1b4e <_ZNK19CompositeComparator4sortEPvl+0x14e>
jnp 4fb1b60 <_ZNK19CompositeComparator4sortEPvl+0x160>
seta %al
ucomisd %xmm0,%xmm1
seta %cl
mov 0x18(%rsp),%rdx
jmp 4fb1b88 <_ZNK19CompositeComparator4sortEPvl+0x188>
nop
movslq 0x4(%rdi),%rax
lea (%rsi,%rax,1),%rcx
movsd (%r14,%rcx,1),%xmm0
add %r9,%rax
movsd (%r14,%rax,1),%xmm1
ucomisd %xmm1,%xmm0
seta %al
ucomisd %xmm0,%xmm1
seta %cl
mov %r8,%rdx
movzbl %al,%eax
movzbl %cl,%ecx
sub %ecx,%eax
mov %eax,%ecx
neg %ecx
cmpb $0x0,(%rdx)
cmovne %eax,%ecx
test %ecx,%ecx
jle 4fb1af0 <_ZNK19CompositeComparator4sortEPvl+0xf0>
dec %r15
lea (%r14,%rsi,1),%rbx
lea (%r14,%r9,1),%r13
lea 0x60(%rsp),%rdi
mov %rbx,%rsi
mov 0x40(%rsp),%rbp
mov %rbp,%rdx
mov %r9,%r12
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
mov %r13,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
lea 0x60(%rsp),%rsi
mov 0x48(%rsp),%rbx
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%r9
mov 0x30(%rsp),%rsi
mov %rbx,%r12
mov 0x8(%rsp),%r8
mov 0x10(%rsp),%rdi
add 0x50(%rsp),%r14
cmp %rbx,%r15
jg 4fb1b30 <_ZNK19CompositeComparator4sortEPvl+0x130>
jmp 4fb1af0 <_ZNK19CompositeComparator4sortEPvl+0xf0>
mov %rcx,0x28(%rsp)
shr $1,%r13
add %r12,%r13
mov %r12,%rsi
imul %r14,%rsi
mov (%rsp),%rax
add %rax,%rsi
imul %r14,%r13
add %rax,%r13
movslq (%rdi),%rax
movsd (%rax,%rsi,1),%xmm0
movsd (%rax,%r13,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fb1c46 <_ZNK19CompositeComparator4sortEPvl+0x246>
jnp 4fb1c57 <_ZNK19CompositeComparator4sortEPvl+0x257>
seta %cl
ucomisd %xmm0,%xmm1
seta %dl
mov 0x18(%rsp),%r9
jmp 4fb1c78 <_ZNK19CompositeComparator4sortEPvl+0x278>
movslq 0x4(%rdi),%rcx
movsd (%rsi,%rcx,1),%xmm1
movsd 0x0(%r13,%rcx,1),%xmm2
ucomisd %xmm2,%xmm1
seta %cl
ucomisd %xmm1,%xmm2
seta %dl
mov %r8,%r9
movzbl %cl,%ecx
movzbl %dl,%edx
sub %edx,%ecx
mov %ecx,%edx
neg %edx
cmpb $0x0,(%r9)
cmovne %ecx,%edx
test %edx,%edx
jle 4fb1ccf <_ZNK19CompositeComparator4sortEPvl+0x2cf>
mov %r15,%rdi
mov %r14,%rdx
mov %rsi,%rbp
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r15,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rsi
mov 0x8(%rsp),%r8
mov 0x10(%rsp),%rdi
movslq (%rdi),%rax
movsd 0x0(%rbp,%rax,1),%xmm0
mov 0x38(%rsp),%rbp
imul %r14,%rbp
add (%rsp),%rbp
movsd (%rax,%rbp,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fb1ce9 <_ZNK19CompositeComparator4sortEPvl+0x2e9>
jnp 4fb1cfa <_ZNK19CompositeComparator4sortEPvl+0x2fa>
seta %cl
ucomisd %xmm0,%xmm1
seta %dl
mov 0x18(%rsp),%r9
jmp 4fb1d1a <_ZNK19CompositeComparator4sortEPvl+0x31a>
movslq 0x4(%rdi),%rcx
movsd (%rsi,%rcx,1),%xmm0
movsd 0x0(%rbp,%rcx,1),%xmm2
ucomisd %xmm2,%xmm0
seta %cl
ucomisd %xmm0,%xmm2
seta %dl
mov %r8,%r9
movzbl %cl,%ecx
movzbl %dl,%edx
sub %edx,%ecx
mov %ecx,%edx
neg %edx
cmpb $0x0,(%r9)
cmovne %ecx,%edx
test %edx,%edx
jle 4fb1d73 <_ZNK19CompositeComparator4sortEPvl+0x373>
mov %r15,%rdi
mov %r14,%rdx
mov %rsi,%r15
call 4da5c70 <memcpy$plt>
mov %r15,%rdi
lea 0x60(%rsp),%r15
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r15,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov 0x8(%rsp),%r8
mov 0x10(%rsp),%rdi
movslq (%rdi),%rax
movsd 0x0(%rbp,%rax,1),%xmm1
movsd 0x0(%r13,%rax,1),%xmm0
ucomisd %xmm1,%xmm0
jne 4fb1d82 <_ZNK19CompositeComparator4sortEPvl+0x382>
jnp 4fb1d93 <_ZNK19CompositeComparator4sortEPvl+0x393>
seta %al
ucomisd %xmm0,%xmm1
seta %cl
mov 0x18(%rsp),%rdx
jmp 4fb1db5 <_ZNK19CompositeComparator4sortEPvl+0x3b5>
movslq 0x4(%rdi),%rax
movsd 0x0(%r13,%rax,1),%xmm0
movsd 0x0(%rbp,%rax,1),%xmm1
ucomisd %xmm1,%xmm0
seta %al
ucomisd %xmm0,%xmm1
seta %cl
mov %r8,%rdx
movzbl %al,%eax
movzbl %cl,%ecx
sub %ecx,%eax
mov %eax,%ecx
neg %ecx
cmpb $0x0,(%rdx)
cmovne %eax,%ecx
test %ecx,%ecx
jle 4fb1df5 <_ZNK19CompositeComparator4sortEPvl+0x3f5>
mov %r15,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r15,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
lea (%rsp,%rbx,1),%rax
add $0x160,%rax
mov %rax,0x20(%rsp)
mov 0x38(%rsp),%rax
lea -0x1(%rax),%rbp
mov %rbp,0x30(%rsp)
imul %r14,%rbp
add (%rsp),%rbp
mov %r15,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r15,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%r15
nopw 0x0(%rax,%rax,1)
mov 0x10(%rsp),%rax
movslq (%rax),%rcx
movsd 0x0(%rbp,%rcx,1),%xmm0
movslq 0x4(%rax),%rax
lea 0x1(%r15),%r12
mov %r14,%rsi
imul %r12,%rsi
lea (%rsi,%rax,1),%rdi
add %rcx,%rsi
mov (%rsp),%r13
mov 0x8(%rsp),%r11
jmp 4fb1ec4 <_ZNK19CompositeComparator4sortEPvl+0x4c4>
nop
movsd 0x0(%r13,%rdi,1),%xmm1
movsd 0x0(%rbp,%rax,1),%xmm2
ucomisd %xmm2,%xmm1
seta %r8b
ucomisd %xmm1,%xmm2
seta %r9b
mov %r11,%r10
movzbl %r8b,%r8d
movzbl %r9b,%r9d
sub %r9d,%r8d
mov %r8d,%r9d
neg %r9d
cmpb $0x0,(%r10)
cmovne %r8d,%r9d
inc %r12
add %r14,%r13
test %r9d,%r9d
jns 4fb1ef0 <_ZNK19CompositeComparator4sortEPvl+0x4f0>
movsd 0x0(%r13,%rsi,1),%xmm1
ucomisd %xmm0,%xmm1
jne 4fb1ed3 <_ZNK19CompositeComparator4sortEPvl+0x4d3>
jnp 4fb1e80 <_ZNK19CompositeComparator4sortEPvl+0x480>
seta %r8b
ucomisd %xmm1,%xmm0
seta %r9b
mov 0x18(%rsp),%r10
jmp 4fb1ea0 <_ZNK19CompositeComparator4sortEPvl+0x4a0>
cs nopw 0x0(%rax,%rax,1)
lea -0x1(%r12),%r10
imul %r14,%r15
add %r15,%r13
mov 0x30(%rsp),%rdi
lea -0x1(%rdi),%rdx
imul %r14,%rdx
lea (%rdx,%rax,1),%rsi
add %rcx,%rdx
mov (%rsp),%rbx
mov %rdi,%r14
mov 0x50(%rsp),%r9
jmp 4fb1f5b <_ZNK19CompositeComparator4sortEPvl+0x55b>
xchg %ax,%ax
movsd (%rbx,%rsi,1),%xmm1
movsd 0x0(%rbp,%rax,1),%xmm2
ucomisd %xmm2,%xmm1
seta %cl
ucomisd %xmm1,%xmm2
seta %dil
mov %r11,%r8
movzbl %cl,%ecx
movzbl %dil,%edi
sub %edi,%ecx
mov %ecx,%edi
neg %edi
cmpb $0x0,(%r8)
cmovne %ecx,%edi
add %r9,%rbx
dec %r14
test %edi,%edi
jle 4fb1f80 <_ZNK19CompositeComparator4sortEPvl+0x580>
movsd (%rbx,%rdx,1),%xmm1
ucomisd %xmm0,%xmm1
jne 4fb1f68 <_ZNK19CompositeComparator4sortEPvl+0x568>
jnp 4fb1f20 <_ZNK19CompositeComparator4sortEPvl+0x520>
seta %cl
ucomisd %xmm1,%xmm0
seta %dil
mov 0x18(%rsp),%r8
jmp 4fb1f3d <_ZNK19CompositeComparator4sortEPvl+0x53d>
nopw 0x0(%rax,%rax,1)
cmp %r14,%r10
jge 4fb1fe0 <_ZNK19CompositeComparator4sortEPvl+0x5e0>
mov 0x40(%rsp),%r12
mov 0x30(%rsp),%rax
imul %r12,%rax
add %rax,%rbx
lea 0x60(%rsp),%rdi
mov %r13,%rsi
mov %r12,%rdx
mov %r10,%r15
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
lea 0x60(%rsp),%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,0x30(%rsp)
mov %r12,%r14
jmp 4fb1e50 <_ZNK19CompositeComparator4sortEPvl+0x450>
data16 data16 cs nopw 0x0(%rax,%rax,1)
lea 0x60(%rsp),%rbx
mov %rbx,%rdi
mov %r13,%rsi
mov 0x40(%rsp),%r14
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %rbx,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%r12),%rax
mov %rax,%rcx
mov 0x48(%rsp),%rsi
sub %rsi,%rcx
mov 0x38(%rsp),%r9
mov %r9,%rdx
sub %r12,%rdx
cmp %rdx,%rcx
jle 4fb206d <_ZNK19CompositeComparator4sortEPvl+0x66d>
mov 0x20(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %r12,%r9
mov 0x10(%rsp),%rdi
mov 0x8(%rsp),%r8
mov 0x28(%rsp),%rcx
jle 4fb2069 <_ZNK19CompositeComparator4sortEPvl+0x669>
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %r12,0x160(%rsp,%rax,1)
mov %r9,0x168(%rsp,%rax,1)
mov %ecx,%edx
jmp 4fb20a1 <_ZNK19CompositeComparator4sortEPvl+0x6a1>
mov 0x20(%rsp),%rcx
mov %r12,(%rcx)
cmp %rax,%rsi
mov 0x10(%rsp),%rdi
mov 0x8(%rsp),%r8
mov 0x28(%rsp),%rdx
jge 4fb20a1 <_ZNK19CompositeComparator4sortEPvl+0x6a1>
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %rsi,0x160(%rsp,%rcx,1)
mov %rax,0x168(%rsp,%rcx,1)
lea 0x60(%rsp),%r15
jmp 4fb1a8a <_ZNK19CompositeComparator4sortEPvl+0x8a>
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