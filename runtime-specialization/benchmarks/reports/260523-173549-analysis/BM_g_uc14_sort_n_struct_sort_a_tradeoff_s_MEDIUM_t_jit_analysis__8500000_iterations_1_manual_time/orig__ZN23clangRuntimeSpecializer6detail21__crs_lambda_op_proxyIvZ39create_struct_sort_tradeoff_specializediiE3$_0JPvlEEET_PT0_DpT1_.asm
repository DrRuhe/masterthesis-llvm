<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_struct_sort_tradeoff_specializediiE3$_0JPvlEEET_PT0_DpT1_>:
jmp 4faa940 <_ZNK11FieldSorter11sort_fieldsEPvl>
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
<_ZNK11FieldSorter11sort_fieldsEPvl>:
cmp $0x2,%rdx
jl 4faade7 <_ZNK11FieldSorter11sort_fieldsEPvl+0x4a7>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x558,%rsp
mov %rsi,%r12
movslq (%rdi),%r15
dec %rdx
movq $0x0,0x150(%rsp)
mov %rdx,0x158(%rsp)
mov %r15,%r14
neg %r14
mov $0x1,%eax
mov %rsi,0x38(%rsp)
mov %rdi,0x18(%rsp)
mov %r15,0x40(%rsp)
jmp 4faa9b4 <_ZNK11FieldSorter11sort_fieldsEPvl+0x74>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x38(%rsp),%r12
mov 0x30(%rsp),%rsi
mov %esi,%eax
test %esi,%esi
jle 4faadd6 <_ZNK11FieldSorter11sort_fieldsEPvl+0x496>
lea -0x1(%rax),%esi
mov %rsi,%rbx
shl $0x4,%rbx
mov 0x150(%rsp,%rbx,1),%rcx
mov 0x158(%rsp,%rbx,1),%rdx
mov %rdx,%r13
sub %rcx,%r13
jle 4faa9aa <_ZNK11FieldSorter11sort_fieldsEPvl+0x6a>
cmp $0xf,%r13
mov %rdx,0x20(%rsp)
jg 4faaac0 <_ZNK11FieldSorter11sort_fieldsEPvl+0x180>
mov %rsi,0x30(%rsp)
mov %r15,%rax
imul %rcx,%rax
add %r12,%rax
mov %rax,0x28(%rsp)
lea 0x1(%rcx),%rdx
imul %r15,%rdx
add %r12,%rdx
mov %rdx,0x8(%rsp)
mov %rcx,%rax
jmp 4faaa2f <_ZNK11FieldSorter11sort_fieldsEPvl+0xef>
xchg %ax,%ax
mov 0x40(%rsp),%r15
add %r15,0x28(%rsp)
add %r15,0x8(%rsp)
mov 0x10(%rsp),%rax
cmp 0x20(%rsp),%rax
je 4faa9a0 <_ZNK11FieldSorter11sort_fieldsEPvl+0x60>
cmp %rcx,%rax
lea 0x1(%rax),%rax
mov %rax,0x10(%rsp)
jl 4faaa10 <_ZNK11FieldSorter11sort_fieldsEPvl+0xd0>
mov 0x8(%rsp),%r13
mov 0x28(%rsp),%rbp
mov 0x10(%rsp),%rbx
nopl 0x0(%rax)
movslq 0x4(%rdi),%rax
movsd 0x0(%rbp,%rax,1),%xmm0
ucomisd 0x0(%r13,%rax,1),%xmm0
jbe 4faaa10 <_ZNK11FieldSorter11sort_fieldsEPvl+0xd0>
dec %rbx
lea 0x50(%rsp),%rdi
mov %rbp,%rsi
mov %r14,%r15
mov %rcx,%r14
mov 0x40(%rsp),%r12
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
lea 0x50(%rsp),%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%rcx
mov %r15,%r14
mov 0x18(%rsp),%rdi
add %r15,%rbp
add %r15,%r13
cmp %rcx,%rbx
jg 4faaa50 <_ZNK11FieldSorter11sort_fieldsEPvl+0x110>
jmp 4faaa10 <_ZNK11FieldSorter11sort_fieldsEPvl+0xd0>
nopw 0x0(%rax,%rax,1)
mov %rax,0x8(%rsp)
shr $1,%r13
add %rcx,%r13
mov %rcx,0x28(%rsp)
imul %r15,%rcx
add %r12,%rcx
imul %r15,%r13
add %r12,%r13
movslq 0x4(%rdi),%rax
movsd (%rax,%rcx,1),%xmm0
ucomisd (%rax,%r13,1),%xmm0
jbe 4faab37 <_ZNK11FieldSorter11sort_fieldsEPvl+0x1f7>
lea 0x50(%rsp),%rdi
mov %rcx,%rbp
mov %rcx,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
lea 0x50(%rsp),%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rcx
mov 0x20(%rsp),%rdx
mov 0x18(%rsp),%rax
movslq 0x4(%rax),%rax
movsd 0x0(%rbp,%rax,1),%xmm0
mov %rdx,%rbp
imul %r15,%rbp
add %r12,%rbp
movsd (%rax,%rbp,1),%xmm1
ucomisd %xmm1,%xmm0
jbe 4faab95 <_ZNK11FieldSorter11sort_fieldsEPvl+0x255>
lea 0x50(%rsp),%rdi
mov %rcx,%rsi
mov %rcx,0x10(%rsp)
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov 0x10(%rsp),%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x50(%rsp),%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov 0x20(%rsp),%rdx
mov 0x18(%rsp),%rax
movslq 0x4(%rax),%rax
movsd 0x0(%rbp,%rax,1),%xmm1
movsd 0x0(%r13,%rax,1),%xmm0
ucomisd %xmm1,%xmm0
jbe 4faabd5 <_ZNK11FieldSorter11sort_fieldsEPvl+0x295>
lea 0x50(%rsp),%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x50(%rsp),%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov 0x20(%rsp),%rdx
lea (%rsp,%rbx,1),%rax
add $0x150,%rax
mov %rax,0x30(%rsp)
lea -0x1(%rdx),%rbp
mov %rbp,0x48(%rsp)
imul %r15,%rbp
add %r12,%rbp
lea 0x50(%rsp),%rbx
mov %rbx,%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,0x10(%rsp)
mov %rbp,%rdi
mov %rbx,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov 0x48(%rsp),%rsi
mov 0x28(%rsp),%rax
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x18(%rsp),%rcx
movslq 0x4(%rcx),%rcx
mov 0x10(%rsp),%rdx
movsd (%rdx,%rcx,1),%xmm0
lea 0x1(%rax),%rbp
mov %r15,%rdx
imul %rbp,%rdx
add %rcx,%rdx
mov %r12,%r13
data16 data16 cs nopw 0x0(%rax,%rax,1)
inc %rbp
ucomisd 0x0(%r13,%rdx,1),%xmm0
lea 0x0(%r13,%r15,1),%r13
ja 4faac70 <_ZNK11FieldSorter11sort_fieldsEPvl+0x330>
lea -0x1(%rbp),%rdx
imul %r15,%rax
add %rax,%r13
lea -0x1(%rsi),%rax
imul %r15,%rax
add %rcx,%rax
mov %r12,%rbx
mov %rsi,%r12
nopl (%rax)
dec %r12
movsd (%rbx,%rax,1),%xmm1
add %r14,%rbx
ucomisd %xmm0,%xmm1
ja 4faaca0 <_ZNK11FieldSorter11sort_fieldsEPvl+0x360>
cmp %r12,%rdx
jge 4faad00 <_ZNK11FieldSorter11sort_fieldsEPvl+0x3c0>
imul %r15,%rsi
add %rsi,%rbx
lea 0x50(%rsp),%rdi
mov %r13,%rsi
mov %rdx,%rbp
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
lea 0x50(%rsp),%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rsi
mov %rbp,%rax
mov 0x38(%rsp),%r12
jmp 4faac40 <_ZNK11FieldSorter11sort_fieldsEPvl+0x300>
xchg %ax,%ax
lea 0x50(%rsp),%rbx
mov %rbx,%rdi
mov %r13,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov 0x10(%rsp),%r12
mov %r12,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %rbx,%rsi
mov %r15,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%rbp),%rax
mov %rax,%rcx
mov 0x28(%rsp),%rsi
sub %rsi,%rcx
mov 0x20(%rsp),%r8
mov %r8,%rdx
sub %rbp,%rdx
cmp %rdx,%rcx
jle 4faad8f <_ZNK11FieldSorter11sort_fieldsEPvl+0x44f>
mov 0x30(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %rbp,%r8
mov 0x38(%rsp),%r12
mov 0x18(%rsp),%rdi
jle 4faadca <_ZNK11FieldSorter11sort_fieldsEPvl+0x48a>
mov 0x8(%rsp),%rcx
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %rbp,0x150(%rsp,%rax,1)
mov %r8,0x158(%rsp,%rax,1)
mov %ecx,%esi
jmp 4faa9aa <_ZNK11FieldSorter11sort_fieldsEPvl+0x6a>
mov 0x30(%rsp),%rcx
mov %rbp,(%rcx)
cmp %rax,%rsi
mov 0x38(%rsp),%r12
mov 0x18(%rsp),%rdi
jge 4faadca <_ZNK11FieldSorter11sort_fieldsEPvl+0x48a>
mov 0x8(%rsp),%rdx
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %rsi,0x150(%rsp,%rcx,1)
mov %rax,0x158(%rsp,%rcx,1)
mov %edx,%esi
jmp 4faa9aa <_ZNK11FieldSorter11sort_fieldsEPvl+0x6a>
mov 0x8(%rsp),%rax
mov %eax,%esi
jmp 4faa9aa <_ZNK11FieldSorter11sort_fieldsEPvl+0x6a>
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
int3
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x24f734c(%rip) # 729cfc8 <memcpy@GLIBC_2.14>
int3
int3
int3
int3