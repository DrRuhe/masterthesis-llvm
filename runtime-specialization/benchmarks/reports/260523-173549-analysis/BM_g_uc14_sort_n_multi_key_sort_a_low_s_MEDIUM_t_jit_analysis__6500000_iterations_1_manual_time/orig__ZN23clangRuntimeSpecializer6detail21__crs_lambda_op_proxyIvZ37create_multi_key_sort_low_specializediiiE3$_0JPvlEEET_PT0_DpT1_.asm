<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_multi_key_sort_low_specializediiiE3$_0JPvlEEET_PT0_DpT1_>:
mov (%rdi),%eax
mov 0x4(%rdi),%ecx
mov 0x8(%rdi),%r8d
mov %rsi,%rdi
mov %rdx,%rsi
mov %eax,%edx
jmp 4fadc30 <_Z14multi_key_sortPvliii>
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
<_Z14multi_key_sortPvliii>:
cmp $0x2,%rsi
jl 4fae2af <_Z14multi_key_sortPvliii+0x67f>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x588,%rsp
mov %rdi,%r15
dec %rsi
movq $0x0,0x180(%rsp)
mov %rsi,0x188(%rsp)
movslq %edx,%r14
movslq %ecx,%rax
mov %rax,0x8(%rsp)
movslq %r8d,%rdx
mov %r14,%rax
neg %rax
mov %rax,0x50(%rsp)
mov $0x1,%edi
mov %r15,0x20(%rsp)
mov %rdx,0x10(%rsp)
mov %r14,(%rsp)
jmp 4fadcb4 <_Z14multi_key_sortPvliii+0x84>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x20(%rsp),%r15
mov 0x58(%rsp),%rcx
mov %ecx,%edi
test %ecx,%ecx
jle 4fae29e <_Z14multi_key_sortPvliii+0x66e>
lea -0x1(%rdi),%ecx
mov %rcx,%r13
shl $0x4,%r13
mov 0x180(%rsp,%r13,1),%r8
mov 0x188(%rsp,%r13,1),%rbx
mov %rbx,%r12
sub %r8,%r12
jle 4fadcaa <_Z14multi_key_sortPvliii+0x7a>
cmp $0xf,%r12
mov %rbx,0x18(%rsp)
jg 4fade50 <_Z14multi_key_sortPvliii+0x220>
mov %rcx,0x58(%rsp)
mov %r14,%rcx
imul %r8,%rcx
lea (%rdx,%rcx,1),%rax
mov %rax,0x68(%rsp)
mov 0x8(%rsp),%rax
mov %rcx,0x78(%rsp)
add %rax,%rcx
mov %rcx,0x30(%rsp)
lea 0x1(%r8),%rcx
imul %r14,%rcx
lea (%rdx,%rcx,1),%rsi
mov %rsi,0x60(%rsp)
mov %rcx,0x70(%rsp)
add %rcx,%rax
mov %rax,0x28(%rsp)
mov %r15,0x40(%rsp)
mov %r8,%rax
jmp 4fadd59 <_Z14multi_key_sortPvliii+0x129>
data16 data16 cs nopw 0x0(%rax,%rax,1)
mov (%rsp),%r14
add %r14,0x40(%rsp)
mov 0x48(%rsp),%rax
cmp 0x18(%rsp),%rax
je 4fadca0 <_Z14multi_key_sortPvliii+0x70>
cmp %r8,%rax
lea 0x1(%rax),%rax
mov %rax,0x48(%rsp)
jl 4fadd40 <_Z14multi_key_sortPvliii+0x110>
mov 0x40(%rsp),%r15
mov 0x48(%rsp),%rbx
data16 data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x30(%rsp),%rax
movsd (%r15,%rax,1),%xmm0
mov 0x28(%rsp),%rax
movsd (%r15,%rax,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fadd9e <_Z14multi_key_sortPvliii+0x16e>
jnp 4faddb0 <_Z14multi_key_sortPvliii+0x180>
seta %cl
ucomisd %xmm0,%xmm1
seta %al
test %cl,%cl
jne 4fadddc <_Z14multi_key_sortPvliii+0x1ac>
jmp 4fadd40 <_Z14multi_key_sortPvliii+0x110>
xchg %ax,%ax
mov 0x68(%rsp),%rax
movsd (%r15,%rax,1),%xmm0
mov 0x60(%rsp),%rax
movsd (%r15,%rax,1),%xmm1
ucomisd %xmm0,%xmm1
seta %cl
ucomisd %xmm1,%xmm0
seta %al
test %cl,%cl
je 4fadd40 <_Z14multi_key_sortPvliii+0x110>
test %al,%al
jne 4fadd40 <_Z14multi_key_sortPvliii+0x110>
dec %rbx
mov 0x78(%rsp),%rax
lea (%r15,%rax,1),%r12
mov 0x70(%rsp),%rax
lea (%r15,%rax,1),%r13
lea 0x80(%rsp),%rdi
mov %r8,%r14
mov %r12,%rsi
mov (%rsp),%rbp
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %r13,%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
lea 0x80(%rsp),%rsi
mov %rbp,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%r8
mov 0x10(%rsp),%rdx
add 0x50(%rsp),%r15
cmp %r14,%rbx
jg 4fadd80 <_Z14multi_key_sortPvliii+0x150>
jmp 4fadd40 <_Z14multi_key_sortPvliii+0x110>
nop
shr $1,%r12
add %r8,%r12
mov %r8,%rsi
imul %r14,%rsi
add %r15,%rsi
imul %r14,%r12
add %r15,%r12
mov 0x8(%rsp),%rax
movsd (%rax,%rsi,1),%xmm0
movsd (%rax,%r12,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fade7f <_Z14multi_key_sortPvliii+0x24f>
jnp 4fade94 <_Z14multi_key_sortPvliii+0x264>
seta %al
ucomisd %xmm0,%xmm1
seta %cl
test %al,%al
mov %rdi,0x30(%rsp)
jne 4fadeb6 <_Z14multi_key_sortPvliii+0x286>
jmp 4fadf11 <_Z14multi_key_sortPvliii+0x2e1>
movsd (%rsi,%rdx,1),%xmm1
movsd (%r12,%rdx,1),%xmm2
ucomisd %xmm1,%xmm2
seta %al
ucomisd %xmm2,%xmm1
seta %cl
test %al,%al
mov %rdi,0x30(%rsp)
je 4fadf11 <_Z14multi_key_sortPvliii+0x2e1>
test %cl,%cl
jne 4fadf11 <_Z14multi_key_sortPvliii+0x2e1>
lea 0x80(%rsp),%rbx
mov %rbx,%rdi
mov %r8,%r14
mov (%rsp),%rdx
mov %rsi,%rbp
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %r12,%rsi
mov (%rsp),%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %rbx,%rsi
mov 0x18(%rsp),%rbx
mov (%rsp),%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rsi
mov %r14,%r8
mov (%rsp),%r14
mov 0x10(%rsp),%rdx
mov 0x8(%rsp),%rax
movsd 0x0(%rbp,%rax,1),%xmm0
mov %rbx,%rbp
imul %r14,%rbp
add %r15,%rbp
mov 0x8(%rsp),%rax
movsd (%rax,%rbp,1),%xmm1
ucomisd %xmm1,%xmm0
jne 4fadf2d <_Z14multi_key_sortPvliii+0x2fd>
jnp 4fadf42 <_Z14multi_key_sortPvliii+0x312>
seta %al
ucomisd %xmm0,%xmm1
seta %cl
test %al,%al
mov %r8,0x38(%rsp)
jne 4fadf64 <_Z14multi_key_sortPvliii+0x334>
jmp 4fadfb5 <_Z14multi_key_sortPvliii+0x385>
movsd (%rsi,%rdx,1),%xmm0
movsd 0x0(%rbp,%rdx,1),%xmm2
ucomisd %xmm0,%xmm2
seta %al
ucomisd %xmm2,%xmm0
seta %cl
test %al,%al
mov %r8,0x38(%rsp)
je 4fadfb5 <_Z14multi_key_sortPvliii+0x385>
test %cl,%cl
jne 4fadfb5 <_Z14multi_key_sortPvliii+0x385>
lea 0x80(%rsp),%rdi
mov (%rsp),%rbx
mov %rbx,%rdx
mov %rsi,%r14
call 4da5c70 <memcpy$plt>
mov %r14,%rdi
mov %rbp,%rsi
mov %rbx,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x80(%rsp),%rsi
mov %rbx,%rdx
mov 0x18(%rsp),%rbx
call 4da5c70 <memcpy$plt>
mov 0x10(%rsp),%rdx
mov 0x8(%rsp),%rax
movsd 0x0(%rbp,%rax,1),%xmm1
mov 0x8(%rsp),%rax
movsd (%r12,%rax,1),%xmm0
ucomisd %xmm1,%xmm0
jne 4fadfc8 <_Z14multi_key_sortPvliii+0x398>
jnp 4fadfdc <_Z14multi_key_sortPvliii+0x3ac>
seta %cl
ucomisd %xmm0,%xmm1
seta %al
mov (%rsp),%r14
test %cl,%cl
jne 4fadffe <_Z14multi_key_sortPvliii+0x3ce>
jmp 4fae039 <_Z14multi_key_sortPvliii+0x409>
movsd (%r12,%rdx,1),%xmm0
movsd 0x0(%rbp,%rdx,1),%xmm1
ucomisd %xmm0,%xmm1
seta %cl
ucomisd %xmm1,%xmm0
seta %al
mov (%rsp),%r14
test %cl,%cl
je 4fae039 <_Z14multi_key_sortPvliii+0x409>
test %al,%al
jne 4fae039 <_Z14multi_key_sortPvliii+0x409>
lea 0x80(%rsp),%rbx
mov %rbx,%rdi
mov %r12,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
mov %rbx,%rsi
mov 0x18(%rsp),%rbx
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
lea (%rsp,%r13,1),%rax
add $0x180,%rax
mov %rax,0x28(%rsp)
lea -0x1(%rbx),%rbp
mov %rbp,%r13
imul %r14,%r13
add %r15,%r13
lea 0x80(%rsp),%rbx
mov %rbx,%rdi
mov %r12,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %rbx,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%r9
mov 0x38(%rsp),%rax
mov 0x8(%rsp),%rsi
movsd 0x0(%r13,%rsi,1),%xmm0
lea 0x1(%rax),%rbx
mov %r14,%rcx
imul %rbx,%rcx
mov 0x10(%rsp),%r8
lea (%r8,%rcx,1),%rdx
add %rsi,%rcx
mov %r15,%r12
cs nopw 0x0(%rax,%rax,1)
movsd (%r12,%rcx,1),%xmm1
ucomisd %xmm0,%xmm1
jne 4fae0ce <_Z14multi_key_sortPvliii+0x49e>
jnp 4fae0e0 <_Z14multi_key_sortPvliii+0x4b0>
seta %sil
ucomisd %xmm1,%xmm0
jmp 4fae0f9 <_Z14multi_key_sortPvliii+0x4c9>
nopl 0x0(%rax,%rax,1)
movsd (%r12,%rdx,1),%xmm1
movsd 0x0(%r13,%r8,1),%xmm2
ucomisd %xmm1,%xmm2
seta %sil
ucomisd %xmm2,%xmm1
seta %dil
inc %rbx
add %r14,%r12
test %dil,%dil
je 4fae10d <_Z14multi_key_sortPvliii+0x4dd>
test %sil,%sil
je 4fae0c0 <_Z14multi_key_sortPvliii+0x490>
lea -0x1(%rbx),%r10
imul %r14,%rax
add %rax,%r12
lea -0x1(%r9),%rax
imul %r14,%rax
lea (%r8,%rax,1),%rcx
add 0x8(%rsp),%rax
mov %r15,%rbp
mov %r9,%r15
mov 0x50(%rsp),%rdi
data16 data16 cs nopw 0x0(%rax,%rax,1)
movsd 0x0(%rbp,%rax,1),%xmm1
ucomisd %xmm0,%xmm1
jne 4fae14e <_Z14multi_key_sortPvliii+0x51e>
jnp 4fae160 <_Z14multi_key_sortPvliii+0x530>
seta %sil
ucomisd %xmm1,%xmm0
jmp 4fae179 <_Z14multi_key_sortPvliii+0x549>
nopl 0x0(%rax,%rax,1)
movsd 0x0(%rbp,%rcx,1),%xmm1
movsd 0x0(%r13,%r8,1),%xmm2
ucomisd %xmm1,%xmm2
seta %sil
ucomisd %xmm2,%xmm1
seta %dl
add %rdi,%rbp
dec %r15
test %sil,%sil
je 4fae18b <_Z14multi_key_sortPvliii+0x55b>
test %dl,%dl
je 4fae140 <_Z14multi_key_sortPvliii+0x510>
cmp %r15,%r10
jge 4fae1e0 <_Z14multi_key_sortPvliii+0x5b0>
imul %r14,%r9
add %r9,%rbp
lea 0x80(%rsp),%rdi
mov %r12,%rsi
mov %r14,%rdx
mov %r10,%rbx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %rbp,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %rbp,%rdi
lea 0x80(%rsp),%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r15,%r9
mov %rbx,%rax
mov 0x20(%rsp),%r15
jmp 4fae090 <_Z14multi_key_sortPvliii+0x460>
xchg %ax,%ax
lea 0x80(%rsp),%r15
mov %r15,%rdi
mov %r12,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r12,%rdi
mov %r13,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
mov %r13,%rdi
mov %r15,%rsi
mov %r14,%rdx
call 4da5c70 <memcpy$plt>
lea -0x2(%rbx),%rax
mov %rax,%rcx
mov 0x38(%rsp),%rsi
sub %rsi,%rcx
mov 0x18(%rsp),%rdi
mov %rdi,%rdx
sub %rbx,%rdx
cmp %rdx,%rcx
jle 4fae263 <_Z14multi_key_sortPvliii+0x633>
mov 0x28(%rsp),%rcx
mov %rax,0x8(%rcx)
cmp %rbx,%rdi
mov 0x20(%rsp),%r15
mov 0x30(%rsp),%rcx
jle 4fae261 <_Z14multi_key_sortPvliii+0x631>
mov %ecx,%eax
inc %ecx
shl $0x4,%rax
mov %rbx,0x180(%rsp,%rax,1)
mov %rdi,0x188(%rsp,%rax,1)
jmp 4fae294 <_Z14multi_key_sortPvliii+0x664>
mov 0x28(%rsp),%rcx
mov %rbx,(%rcx)
cmp %rax,%rsi
mov 0x20(%rsp),%r15
mov 0x30(%rsp),%rdx
jge 4fae292 <_Z14multi_key_sortPvliii+0x662>
mov %edx,%ecx
inc %edx
shl $0x4,%rcx
mov %rsi,0x180(%rsp,%rcx,1)
mov %rax,0x188(%rsp,%rcx,1)
mov %edx,%ecx
mov 0x10(%rsp),%rdx
jmp 4fadcaa <_Z14multi_key_sortPvliii+0x7a>
add $0x588,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x24f734c(%rip) # 729cfc8 <memcpy@GLIBC_2.14>
int3
int3
int3
int3