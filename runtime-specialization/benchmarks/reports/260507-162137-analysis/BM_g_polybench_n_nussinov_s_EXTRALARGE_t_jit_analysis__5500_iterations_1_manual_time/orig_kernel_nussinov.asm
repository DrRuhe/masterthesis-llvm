<kernel_nussinov>:
test %edi,%edi
jle 17fb090 <kernel_nussinov+0x120>
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
lea 0x245aa0f(%rip),%rax # 3c55998 <g_nussinov_table>
mov (%rax),%rax
mov %edi,%r8d
imul $0x55f4,%r8,%rsi
lea (%rsi,%rax,1),%rdx
add $0x55f0,%rdx
mov %rax,-0x10(%rsp)
add %rax,%rsi
add $0xffffffffffffaa10,%rsi
mov %r8,-0x8(%rsp)
jmp 17fafd8 <kernel_nussinov+0x68>
nopw 0x0(%rax,%rax,1)
add $0xffffffffffffaa0c,%rdx
add $0xffffffffffffaa0c,%rsi
cmp $0x2,%r9
jl 17fb086 <kernel_nussinov+0x116>
mov %r8,%r9
dec %r8
cmp -0x8(%rsp),%r9
jge 17fafc0 <kernel_nussinov+0x50>
imul $0x55f0,%r8,%r10
mov -0x10(%rsp),%rax
add %rax,%r10
imul $0x55f0,%r9,%r11
add %rax,%r11
mov %rdx,%rbx
mov %r9,%r14
jmp 17fb01c <kernel_nussinov+0xac>
cs nopw 0x0(%rax,%rax,1)
inc %r14
add $0x4,%rbx
cmp %r14d,%edi
jle 17fafc0 <kernel_nussinov+0x50>
lea -0x1(%r14),%r15d
mov (%r10,%r14,4),%r12d
mov (%r10,%r15,4),%ebp
cmp %ebp,%r12d
cmovg %r12d,%ebp
mov (%r11,%r14,4),%r12d
cmp %r12d,%ebp
cmovle %r12d,%ebp
mov %ebp,(%r10,%r14,4)
mov (%r11,%r15,4),%r15d
cmp %r15d,%ebp
cmovle %r15d,%ebp
mov %ebp,(%r10,%r14,4)
cmp %r14,%r9
jae 17fb010 <kernel_nussinov+0xa0>
lea (%r10,%r14,4),%r15
mov $0x1,%r12d
mov %rbx,%r13
nop
mov 0x0(%r13),%eax
add -0x4(%rsi,%r12,4),%eax
lea (%r9,%r12,1),%ecx
cmp %eax,%ebp
cmovle %eax,%ebp
mov %ebp,(%r15)
inc %r12
add $0x55f0,%r13
cmp %r14,%rcx
jne 17fb060 <kernel_nussinov+0xf0>
jmp 17fb010 <kernel_nussinov+0xa0>
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
int3
int3
int3
int3