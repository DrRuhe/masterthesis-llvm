<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ40create_generic_sort_tradeoff_specializediE3$_0JPvlEEET_PT0_DpT1_>:
mov 0x8(%rdi),%rcx
jmp 4fa5890 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E>
int3
int3
int3
int3
int3
int3
int3
<_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x488,%rsp
mov %rsi,0x10(%rsp)
cmp $0x2,%rdx
jge 4fa58be <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x2e>
add $0x488,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
mov %rcx,%r15
movslq (%rdi),%rbx
dec %rdx
movq $0x0,0x80(%rsp)
mov %rdx,0x88(%rsp)
mov %rbx,%rax
neg %rax
mov $0x1,%edx
lea 0x30(%rsp),%r12
mov %rcx,0x20(%rsp)
mov %rbx,0x48(%rsp)
mov %rax,0x78(%rsp)
jmp 4fa5915 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x85>
nopl 0x0(%rax)
mov 0x20(%rsp),%r15
lea 0x30(%rsp),%r12
mov 0x58(%rsp),%rsi
mov %esi,%edx
test %esi,%esi
jle 4fa58ac <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x1c>
lea -0x1(%rdx),%esi
mov %rsi,%r14
shl $0x4,%r14
mov 0x80(%rsp,%r14,1),%rcx
mov 0x88(%rsp,%r14,1),%r13
mov %r13,%rbp
mov %rcx,0x28(%rsp)
sub %rcx,%rbp
jle 4fa590f <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x7f>
cmp $0xf,%rbp
mov %r13,0x50(%rsp)
jg 4fa5a20 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x190>
mov %rsi,0x58(%rsp)
mov %rbx,%rcx
mov 0x28(%rsp),%r14
imul %r14,%rcx
mov 0x10(%rsp),%rax
add %rax,%rcx
mov %rcx,0x18(%rsp)
lea 0x1(%r14),%r12
imul %rbx,%r12
add %rax,%r12
mov %r14,%rax
jmp 4fa599d <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x10d>
nopl 0x0(%rax)
mov 0x48(%rsp),%rbx
add %rbx,0x18(%rsp)
add %rbx,%r12
mov 0x8(%rsp),%rax
cmp 0x50(%rsp),%rax
je 4fa5900 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x70>
cmp %r14,%rax
lea 0x1(%rax),%rax
mov %rax,0x8(%rsp)
jl 4fa5980 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0xf0>
mov %r12,%r13
mov 0x18(%rsp),%rbp
mov 0x8(%rsp),%r15
nopl 0x0(%rax,%rax,1)
mov %rbp,%rdi
mov %r13,%rsi
call *0x20(%rsp)
test %eax,%eax
jle 4fa5980 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0xf0>
dec %r15
lea 0x30(%rsp),%r14
mov %r14,%rdi
mov %rbp,%rsi
mov 0x48(%rsp),%rbx
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r14,%rsi
mov 0x28(%rsp),%r14
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov 0x78(%rsp),%rax
add %rax,%rbp
add %rax,%r13
cmp %r14,%r15
jg 4fa59c0 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x130>
jmp 4fa5980 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0xf0>
nop
mov %rdx,0x68(%rsp)
shr $1,%rbp
mov 0x28(%rsp),%rdi
add %rdi,%rbp
imul %rbx,%rdi
mov 0x10(%rsp),%rax
add %rax,%rdi
imul %rbx,%rbp
add %rax,%rbp
mov %rdi,0x8(%rsp)
mov %rbp,%rsi
call *%r15
test %eax,%eax
jle 4fa5a86 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x1f6>
mov %r12,%rdi
mov 0x8(%rsp),%r12
mov %r12,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
lea 0x30(%rsp),%r12
mov %rbp,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r12,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
imul %rbx,%r13
add 0x10(%rsp),%r13
mov 0x8(%rsp),%rdi
mov %r13,%rsi
call *%r15
test %eax,%eax
jle 4fa5ad2 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x242>
mov %r12,%rdi
mov 0x8(%rsp),%r12
mov %r12,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
lea 0x30(%rsp),%r12
mov %r13,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r12,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
call *%r15
test %eax,%eax
jle 4fa5b09 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x279>
mov %r12,%rdi
mov %rbp,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r12,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
lea (%rsp,%r14,1),%rax
add $0x80,%rax
mov %rax,0x60(%rsp)
mov 0x50(%rsp),%rax
lea -0x1(%rax),%r14
mov %r14,0x18(%rsp)
imul %rbx,%r14
add 0x10(%rsp),%r14
mov %r12,%rdi
mov %rbp,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r14,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,0x70(%rsp)
mov %r14,%rdi
mov %r12,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov 0x28(%rsp),%r13
mov %rbx,%r14
cs nopw 0x0(%rax,%rax,1)
mov %r13,0x8(%rsp)
lea 0x1(%r13),%r12
mov %r14,%rbx
imul %r12,%rbx
mov 0x10(%rsp),%rbp
mov 0x70(%rsp),%r13
nopw 0x0(%rax,%rax,1)
lea (%rbx,%rbp,1),%rdi
mov %r13,%rsi
call *%r15
inc %r12
add %r14,%rbp
test %eax,%eax
js 4fa5b90 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x300>
lea -0x1(%r12),%rax
mov %rax,0x58(%rsp)
mov 0x8(%rsp),%rax
imul %r14,%rax
add %rax,%rbp
mov 0x18(%rsp),%r15
lea -0x1(%r15),%rcx
imul %r14,%rcx
mov %rcx,0x8(%rsp)
mov 0x10(%rsp),%r14
mov 0x78(%rsp),%rbx
cs nopw 0x0(%rax,%rax,1)
dec %r15
mov 0x8(%rsp),%rax
lea (%r14,%rax,1),%rdi
mov %r13,%rsi
call *0x20(%rsp)
add %rbx,%r14
test %eax,%eax
jg 4fa5be0 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x350>
mov 0x58(%rsp),%r13
cmp %r15,%r13
jge 4fa5c60 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x3d0>
mov 0x48(%rsp),%r12
mov 0x18(%rsp),%rax
imul %r12,%rax
add %rax,%r14
lea 0x30(%rsp),%rbx
mov %rbx,%rdi
mov %rbp,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r14,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%rdi
mov %rbx,%rsi
mov %r12,%r14
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r15,0x18(%rsp)
mov 0x20(%rsp),%r15
jmp 4fa5b70 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x2e0>
cs nopw 0x0(%rax,%rax,1)
lea 0x30(%rsp),%r14
mov %r14,%rdi
mov %rbp,%rsi
mov 0x48(%rsp),%rbx
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov 0x70(%rsp),%r15
mov %r15,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %r15,%rdi
mov %r14,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%r12),%rax
mov %rax,%rcx
mov 0x28(%rsp),%rsi
sub %rsi,%rcx
mov 0x50(%rsp),%rdi
mov %rdi,%rdx
sub %r12,%rdx
cmp %rdx,%rcx
jle 4fa5ced <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x45d>
mov 0x60(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %r12,%rdi
mov 0x20(%rsp),%r15
mov 0x68(%rsp),%rcx
jle 4fa5ce9 <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x459>
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %r12,0x80(%rsp,%rax,1)
mov %rdi,0x88(%rsp,%rax,1)
mov %ecx,%esi
jmp 4fa5d1e <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x48e>
mov 0x60(%rsp),%rcx
mov %r12,(%rcx)
cmp %rax,%rsi
mov 0x20(%rsp),%r15
mov 0x68(%rsp),%rdx
jge 4fa5d1c <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x48c>
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %rsi,0x80(%rsp,%rcx,1)
mov %rax,0x88(%rsp,%rcx,1)
mov %edx,%esi
lea 0x30(%rsp),%r12
jmp 4fa590f <_ZNK14GenericSorterT4sortEPvlPFiPKvS2_E+0x7f>
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