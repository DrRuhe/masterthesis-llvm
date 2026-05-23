<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ40create_generic_sort_abstract_specializediE3$_0JPvlEEET_PT0_DpT1_>:
push %rax
mov (%rdi),%ecx
lea 0x2265efe(%rip),%rax # 720d758 <_ZTV18Int64AscComparator>
add $0x10,%rax
mov %rax,(%rsp)
mov %rsp,%rdi
call 4fa7360 <_ZNK6Sorter4sortEPvli>
pop %rax
ret
int3
int3
int3
int3
<_ZNK6Sorter4sortEPvli>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x488,%rsp
mov %rdi,0x28(%rsp)
cmp $0x2,%rdx
jge 4fa738e <_ZNK6Sorter4sortEPvli+0x2e>
add $0x488,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
dec %rdx
movq $0x0,0x80(%rsp)
mov %rdx,0x88(%rsp)
movslq %ecx,%rbp
mov %rbp,%rax
neg %rax
mov %rax,0x78(%rsp)
mov $0x1,%ecx
mov %rsi,0x10(%rsp)
mov %rbp,0x30(%rsp)
jmp 4fa73e0 <_ZNK6Sorter4sortEPvli+0x80>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x10(%rsp),%rsi
mov 0x38(%rsp),%rdx
mov %edx,%ecx
test %edx,%edx
jle 4fa737c <_ZNK6Sorter4sortEPvli+0x1c>
lea -0x1(%rcx),%edx
mov %rdx,%rbx
shl $0x4,%rbx
mov 0x80(%rsp,%rbx,1),%rax
mov 0x88(%rsp,%rbx,1),%r15
mov %r15,%r13
mov %rax,0x20(%rsp)
sub %rax,%r13
jle 4fa73da <_ZNK6Sorter4sortEPvli+0x7a>
cmp $0xf,%r13
mov %r15,0x70(%rsp)
jg 4fa74f0 <_ZNK6Sorter4sortEPvli+0x190>
mov %rdx,0x38(%rsp)
mov %rbp,%rax
mov 0x20(%rsp),%rbx
imul %rbx,%rax
add %rsi,%rax
mov %rax,0x18(%rsp)
lea 0x1(%rbx),%r12
imul %rbp,%r12
add %rsi,%r12
mov %rbx,%rax
jmp 4fa745d <_ZNK6Sorter4sortEPvli+0xfd>
nop
mov 0x30(%rsp),%rbp
add %rbp,0x18(%rsp)
add %rbp,%r12
mov 0x8(%rsp),%rax
cmp 0x70(%rsp),%rax
je 4fa73d0 <_ZNK6Sorter4sortEPvli+0x70>
cmp %rbx,%rax
lea 0x1(%rax),%rax
mov %rax,0x8(%rsp)
jl 4fa7440 <_ZNK6Sorter4sortEPvli+0xe0>
mov %r12,%r13
mov 0x18(%rsp),%rbp
mov 0x8(%rsp),%r15
nopl 0x0(%rax,%rax,1)
mov 0x28(%rsp),%rdi
mov (%rdi),%rax
mov %rbp,%rsi
mov %r13,%rdx
call *(%rax)
test %eax,%eax
jle 4fa7440 <_ZNK6Sorter4sortEPvli+0xe0>
dec %r15
lea 0x40(%rsp),%rbx
mov %rbx,%rdi
mov %rbp,%rsi
mov 0x30(%rsp),%r14
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov 0x20(%rsp),%rbx
mov %r14,%rdx
mov 0x78(%rsp),%r14
call 4da5c70 <memcpy$plt>
add %r14,%rbp
add %r14,%r13
cmp %rbx,%r15
jg 4fa7480 <_ZNK6Sorter4sortEPvli+0x120>
jmp 4fa7440 <_ZNK6Sorter4sortEPvli+0xe0>
data16 cs nopw 0x0(%rax,%rax,1)
mov %rcx,0x60(%rsp)
shr $1,%r13
mov 0x20(%rsp),%rcx
add %rcx,%r13
imul %rbp,%rcx
add %rsi,%rcx
imul %rbp,%r13
add %rsi,%r13
mov 0x28(%rsp),%rdi
mov (%rdi),%rax
mov %rcx,0x8(%rsp)
mov %rcx,%rsi
mov %r13,%rdx
call *(%rax)
test %eax,%eax
jle 4fa755b <_ZNK6Sorter4sortEPvli+0x1fb>
lea 0x40(%rsp),%r14
mov %r14,%rdi
mov 0x8(%rsp),%r12
mov %r12,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %r13,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r14,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%r12
mov %r15,%rbp
imul %r12,%rbp
add 0x10(%rsp),%rbp
mov 0x28(%rsp),%rdi
mov (%rdi),%rax
mov 0x8(%rsp),%rsi
mov %rbp,%rdx
call *(%rax)
test %eax,%eax
jle 4fa75b3 <_ZNK6Sorter4sortEPvli+0x253>
lea 0x40(%rsp),%rdi
mov 0x8(%rsp),%r14
mov %r14,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%rdi
mov %rbp,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x40(%rsp),%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov 0x28(%rsp),%rdi
mov (%rdi),%rax
mov %r13,%rsi
mov %rbp,%rdx
call *(%rax)
test %eax,%eax
lea 0x40(%rsp),%r14
jle 4fa75f6 <_ZNK6Sorter4sortEPvli+0x296>
mov %r14,%rdi
mov %r13,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbp,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r14,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
lea (%rsp,%rbx,1),%rax
add $0x80,%rax
mov %rax,0x58(%rsp)
dec %r15
mov %r15,0x18(%rsp)
imul %r12,%r15
mov 0x10(%rsp),%rbx
add %rbx,%r15
mov %r14,%rdi
mov %r13,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r15,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %r15,0x68(%rsp)
mov %r15,%rdi
mov %r14,%rsi
mov %r12,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%r13
mov 0x20(%rsp),%rax
mov %r12,%rbp
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rax,0x8(%rsp)
lea 0x1(%rax),%r12
mov %rbp,%rbx
imul %r12,%rbx
mov %rbp,%r15
mov 0x28(%rsp),%rbp
mov 0x68(%rsp),%r14
nopl (%rax)
lea (%rbx,%r13,1),%rsi
mov 0x0(%rbp),%rax
mov %rbp,%rdi
mov %r14,%rdx
call *(%rax)
inc %r12
add %r15,%r13
test %eax,%eax
js 4fa7680 <_ZNK6Sorter4sortEPvli+0x320>
lea -0x1(%r12),%rax
mov %rax,0x38(%rsp)
mov 0x8(%rsp),%rax
imul %r15,%rax
add %rax,%r13
mov 0x18(%rsp),%rax
lea -0x1(%rax),%rcx
imul %r15,%rcx
mov %rcx,0x8(%rsp)
mov 0x10(%rsp),%rbx
mov %rax,%r15
nopw 0x0(%rax,%rax,1)
dec %r15
mov 0x8(%rsp),%rax
lea (%rbx,%rax,1),%rsi
mov 0x0(%rbp),%rax
mov %rbp,%rdi
mov %r14,%rdx
call *(%rax)
add 0x78(%rsp),%rbx
test %eax,%eax
jg 4fa76d0 <_ZNK6Sorter4sortEPvli+0x370>
mov 0x38(%rsp),%rax
cmp %r15,%rax
jge 4fa7750 <_ZNK6Sorter4sortEPvli+0x3f0>
mov 0x30(%rsp),%rbp
mov %rax,%r12
mov 0x18(%rsp),%rax
imul %rbp,%rax
add %rax,%rbx
lea 0x40(%rsp),%r14
mov %r14,%rdi
mov %r13,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
mov %r14,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r15,0x18(%rsp)
mov %r12,%rax
mov 0x10(%rsp),%r13
jmp 4fa7660 <_ZNK6Sorter4sortEPvli+0x300>
lea 0x40(%rsp),%r14
mov %r14,%rdi
mov %r13,%rsi
mov 0x30(%rsp),%rbp
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov 0x68(%rsp),%rbx
mov %rbx,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %rbx,%rdi
mov %r14,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%r12),%rax
mov %rax,%rcx
mov 0x20(%rsp),%r8
sub %r8,%rcx
mov 0x70(%rsp),%rdi
mov %rdi,%rdx
sub %r12,%rdx
cmp %rdx,%rcx
jle 4fa77e0 <_ZNK6Sorter4sortEPvli+0x480>
mov 0x58(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %r12,%rdi
mov 0x10(%rsp),%rsi
mov 0x60(%rsp),%rcx
jle 4fa7814 <_ZNK6Sorter4sortEPvli+0x4b4>
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %r12,0x80(%rsp,%rax,1)
mov %rdi,0x88(%rsp,%rax,1)
mov %ecx,%edx
jmp 4fa73da <_ZNK6Sorter4sortEPvli+0x7a>
mov 0x58(%rsp),%rcx
mov %r12,(%rcx)
cmp %rax,%r8
mov 0x10(%rsp),%rsi
mov 0x60(%rsp),%rdx
jge 4fa780f <_ZNK6Sorter4sortEPvli+0x4af>
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %r8,0x80(%rsp,%rcx,1)
mov %rax,0x88(%rsp,%rcx,1)
jmp 4fa73da <_ZNK6Sorter4sortEPvli+0x7a>
mov %ecx,%edx
jmp 4fa73da <_ZNK6Sorter4sortEPvli+0x7a>
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