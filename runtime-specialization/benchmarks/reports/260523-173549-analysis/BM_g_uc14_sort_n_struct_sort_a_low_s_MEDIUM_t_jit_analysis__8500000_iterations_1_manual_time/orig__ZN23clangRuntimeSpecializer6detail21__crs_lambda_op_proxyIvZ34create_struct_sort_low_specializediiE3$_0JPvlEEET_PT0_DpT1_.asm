<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ34create_struct_sort_low_specializediiE3$_0JPvlEEET_PT0_DpT1_>:
mov (%rdi),%eax
mov 0x4(%rdi),%ecx
mov %rsi,%rdi
mov %rdx,%rsi
mov %eax,%edx
jmp 4fa8b70 <_Z11struct_sortPvlii>
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
<_Z11struct_sortPvlii>:
cmp $0x2,%rsi
jl 4fa8ff8 <_Z11struct_sortPvlii+0x488>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x558,%rsp
mov %rdi,%rbx
dec %rsi
movq $0x0,0x150(%rsp)
mov %rsi,0x158(%rsp)
movslq %edx,%rax
movslq %ecx,%rcx
mov %rax,%r15
neg %r15
mov $0x1,%edx
mov %rdi,0x30(%rsp)
mov %rcx,0x8(%rsp)
mov %rax,(%rsp)
mov %r15,0x48(%rsp)
jmp 4fa8be4 <_Z11struct_sortPvlii+0x74>
nopl 0x0(%rax,%rax,1)
mov 0x30(%rsp),%rbx
mov 0x28(%rsp),%rsi
mov %esi,%edx
test %esi,%esi
jle 4fa8fe7 <_Z11struct_sortPvlii+0x477>
lea -0x1(%rdx),%esi
mov %rsi,%r12
shl $0x4,%r12
mov 0x150(%rsp,%r12,1),%rbp
mov 0x158(%rsp,%r12,1),%r14
mov %r14,%r13
sub %rbp,%r13
jle 4fa8bda <_Z11struct_sortPvlii+0x6a>
cmp $0xf,%r13
mov %r14,0x38(%rsp)
jg 4fa8cf0 <_Z11struct_sortPvlii+0x180>
mov %rsi,0x28(%rsp)
mov %rax,%rdx
imul %rbp,%rdx
add %rbx,%rdx
mov %rdx,0x20(%rsp)
lea 0x1(%rbp),%rdx
imul %rax,%rdx
add %rbx,%rdx
mov %rdx,0x18(%rsp)
mov %rbp,%rdx
jmp 4fa8c5e <_Z11struct_sortPvlii+0xee>
xchg %ax,%ax
mov (%rsp),%rax
add %rax,0x20(%rsp)
add %rax,0x18(%rsp)
mov 0x10(%rsp),%rdx
cmp 0x38(%rsp),%rdx
je 4fa8bd0 <_Z11struct_sortPvlii+0x60>
cmp %rbp,%rdx
lea 0x1(%rdx),%rax
mov %rax,0x10(%rsp)
jl 4fa8c40 <_Z11struct_sortPvlii+0xd0>
mov 0x18(%rsp),%r12
mov 0x20(%rsp),%r13
mov 0x10(%rsp),%rbx
nopl 0x0(%rax,%rax,1)
movsd 0x0(%r13,%rcx,1),%xmm0
ucomisd (%r12,%rcx,1),%xmm0
jbe 4fa8c40 <_Z11struct_sortPvlii+0xd0>
dec %rbx
mov %rbp,%r15
lea 0x50(%rsp),%rbp
mov %rbp,%rdi
mov %r13,%rsi
mov (%rsp),%r14
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r12,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %rbp,%rsi
mov %r15,%rbp
mov 0x48(%rsp),%r15
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov 0x8(%rsp),%rcx
add %r15,%r13
add %r15,%r12
cmp %rbp,%rbx
jg 4fa8c80 <_Z11struct_sortPvlii+0x110>
jmp 4fa8c40 <_Z11struct_sortPvlii+0xd0>
data16 cs nopw 0x0(%rax,%rax,1)
mov %rdx,0x18(%rsp)
shr $1,%r13
add %rbp,%r13
mov %rbp,0x20(%rsp)
mov %rbp,%rdx
imul %rax,%rdx
add %rbx,%rdx
imul %rax,%r13
add %rbx,%r13
movsd (%rcx,%rdx,1),%xmm0
ucomisd (%rcx,%r13,1),%xmm0
jbe 4fa8d64 <_Z11struct_sortPvlii+0x1f4>
lea 0x50(%rsp),%rdi
mov %rdx,%rbp
mov %rdx,%rsi
mov (%rsp),%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov (%rsp),%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
lea 0x50(%rsp),%rsi
mov (%rsp),%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdx
mov (%rsp),%rax
mov 0x8(%rsp),%rcx
movsd 0x0(%rbp,%rcx,1),%xmm0
mov %r14,%rbp
imul %rax,%rbp
add %rbx,%rbp
movsd (%rcx,%rbp,1),%xmm1
ucomisd %xmm1,%xmm0
jbe 4fa8dc2 <_Z11struct_sortPvlii+0x252>
lea 0x50(%rsp),%rdi
mov %rdx,%rsi
mov %rdx,0x10(%rsp)
mov (%rsp),%r14
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov 0x10(%rsp),%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x50(%rsp),%rsi
mov %r14,%rdx
mov 0x38(%rsp),%r14
call 4da5c70 <memcpy$plt>
mov 0x8(%rsp),%rcx
movsd 0x0(%rbp,%rcx,1),%xmm1
movsd 0x0(%r13,%rcx,1),%xmm0
ucomisd %xmm1,%xmm0
jbe 4fa8e06 <_Z11struct_sortPvlii+0x296>
lea 0x50(%rsp),%rdi
mov %r13,%rsi
mov (%rsp),%r14
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x50(%rsp),%rsi
mov %r14,%rdx
mov 0x38(%rsp),%r14
call 4da5c70 <memcpy$plt>
lea (%rsp,%r12,1),%rax
add $0x150,%rax
mov %rax,0x28(%rsp)
lea -0x1(%r14),%rbp
mov %rbp,0x40(%rsp)
mov (%rsp),%r14
imul %r14,%rbp
add %rbx,%rbp
lea 0x50(%rsp),%r12
mov %r12,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,0x10(%rsp)
mov %rbp,%rdi
mov %r12,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov 0x40(%rsp),%rsi
mov 0x20(%rsp),%rax
nopw 0x0(%rax,%rax,1)
mov 0x8(%rsp),%rdx
mov 0x10(%rsp),%rcx
movsd (%rcx,%rdx,1),%xmm0
lea 0x1(%rax),%r12
mov %r14,%rcx
imul %r12,%rcx
add %rdx,%rcx
mov %rbx,%r13
inc %r12
ucomisd 0x0(%r13,%rcx,1),%xmm0
lea 0x0(%r13,%r14,1),%r13
ja 4fa8e90 <_Z11struct_sortPvlii+0x320>
lea -0x1(%r12),%rcx
imul %r14,%rax
add %rax,%r13
lea -0x1(%rsi),%rax
imul %r14,%rax
add %rdx,%rax
mov %rbx,%rbp
mov %rsi,%rbx
xchg %ax,%ax
dec %rbx
movsd 0x0(%rbp,%rax,1),%xmm1
add %r15,%rbp
ucomisd %xmm0,%xmm1
ja 4fa8ec0 <_Z11struct_sortPvlii+0x350>
cmp %rbx,%rcx
jge 4fa8f20 <_Z11struct_sortPvlii+0x3b0>
imul %r14,%rsi
add %rsi,%rbp
lea 0x50(%rsp),%rdi
mov %r13,%rsi
mov %r14,%rdx
mov %rcx,%r12
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x50(%rsp),%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rsi
mov %r12,%rax
mov 0x30(%rsp),%rbx
jmp 4fa8e70 <_Z11struct_sortPvlii+0x300>
nop
lea 0x50(%rsp),%rbx
mov %rbx,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov 0x10(%rsp),%r13
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%r12),%rax
mov %rax,%rcx
mov 0x20(%rsp),%rsi
sub %rsi,%rcx
mov 0x38(%rsp),%rdi
mov %rdi,%rdx
sub %r12,%rdx
cmp %rdx,%rcx
jle 4fa8fa8 <_Z11struct_sortPvlii+0x438>
mov 0x28(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %r12,%rdi
mov 0x30(%rsp),%rbx
mov 0x18(%rsp),%rcx
jle 4fa8fa4 <_Z11struct_sortPvlii+0x434>
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %r12,0x150(%rsp,%rax,1)
mov %rdi,0x158(%rsp,%rax,1)
mov %ecx,%esi
jmp 4fa8fd9 <_Z11struct_sortPvlii+0x469>
mov 0x28(%rsp),%rcx
mov %r12,(%rcx)
cmp %rax,%rsi
mov 0x30(%rsp),%rbx
mov 0x18(%rsp),%rdx
jge 4fa8fd7 <_Z11struct_sortPvlii+0x467>
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %rsi,0x150(%rsp,%rcx,1)
mov %rax,0x158(%rsp,%rcx,1)
mov %edx,%esi
mov 0x8(%rsp),%rcx
mov (%rsp),%rax
jmp 4fa8bda <_Z11struct_sortPvlii+0x6a>
add $0x558,%rsp
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
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x24f734c(%rip) # 729cfc8 <memcpy@GLIBC_2.14>
int3
int3
int3
int3