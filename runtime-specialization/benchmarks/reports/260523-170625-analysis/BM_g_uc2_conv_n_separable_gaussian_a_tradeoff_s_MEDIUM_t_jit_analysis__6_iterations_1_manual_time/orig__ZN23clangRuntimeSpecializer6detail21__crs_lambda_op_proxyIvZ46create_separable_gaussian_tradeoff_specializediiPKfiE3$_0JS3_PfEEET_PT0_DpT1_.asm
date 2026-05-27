<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ46create_separable_gaussian_tradeoff_specializediiPKfiE3$_0JS3_PfEEET_PT0_DpT1_>:
mov 0x44(%rdi),%ecx
mov 0x48(%rdi),%r8d
jmp 4f75d80 <_ZNK15SeparableFilter5applyEPKfPfii>
int3
int3
int3
int3
<_ZNK15SeparableFilter5applyEPKfPfii>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x58,%rsp
mov %r8d,%ebp
mov %ecx,%r12d
mov %rdx,0x18(%rsp)
mov %rsi,0x50(%rsp)
mov %rdi,%r13
mov %r8d,%eax
imul %ecx,%eax
xorps %xmm0,%xmm0
movaps %xmm0,0x30(%rsp)
movq $0x0,0x40(%rsp)
test %eax,%eax
je 4f75df8 <_ZNK15SeparableFilter5applyEPKfPfii+0x78>
js 4f760f8 <_ZNK15SeparableFilter5applyEPKfPfii+0x378>
movslq %eax,%rbx
lea 0x0(,%rbx,4),%r14
mov %r14,%rdi
call 71af8b0 <_Znwm>
lea (%rax,%rbx,4),%rbx
mov %rax,(%rsp)
mov %rax,%rdi
xor %esi,%esi
mov %r14,%rdx
call 4da5b60 <memset$plt>
test %ebp,%ebp
mov %rbx,0x48(%rsp)
jg 4f75e0f <_ZNK15SeparableFilter5applyEPKfPfii+0x8f>
jmp 4f760c3 <_ZNK15SeparableFilter5applyEPKfPfii+0x343>
xor %ebx,%ebx
movq $0x0,(%rsp)
test %ebp,%ebp
mov %rbx,0x48(%rsp)
jle 4f760c3 <_ZNK15SeparableFilter5applyEPKfPfii+0x343>
test %r12d,%r12d
jle 4f760c3 <_ZNK15SeparableFilter5applyEPKfPfii+0x343>
mov 0x40(%r13),%edi
mov %r12d,%r15d
mov %ebp,%r14d
test %edi,%edi
jle 4f76096 <_ZNK15SeparableFilter5applyEPKfPfii+0x316>
mov %edi,%eax
shr $0x1f,%eax
add %edi,%eax
sar $1,%eax
neg %eax
mov %rax,0x8(%rsp)
lea -0x1(%r12),%esi
lea -0x1(%rdi),%rax
mov %rax,0x28(%rsp)
mov %rdi,%rdx
and $0x7ffffffe,%edi
xor %r8d,%r8d
xor %ecx,%ecx
mov %r14,0x20(%rsp)
jmp 4f75e76 <_ZNK15SeparableFilter5applyEPKfPfii+0xf6>
nopl (%rax)
mov 0x10(%rsp),%rcx
inc %rcx
mov 0x20(%rsp),%r14
cmp %r14,%rcx
je 4f75f69 <_ZNK15SeparableFilter5applyEPKfPfii+0x1e9>
mov %rcx,0x10(%rsp)
imul %r15,%rcx
mov 0x50(%rsp),%rax
lea (%rax,%rcx,4),%r10
mov (%rsp),%r9
lea (%r9,%rcx,4),%r11
mov 0x8(%rsp),%rax
mov %eax,%r14d
xor %r9d,%r9d
jmp 4f75eb4 <_ZNK15SeparableFilter5applyEPKfPfii+0x134>
nopl (%rax)
movss %xmm0,(%r11,%r9,4)
inc %r9
inc %r14d
mov %rax,%r15
cmp %rax,%r9
je 4f75e60 <_ZNK15SeparableFilter5applyEPKfPfii+0xe0>
mov %r15,%rax
cmpq $0x0,0x28(%rsp)
je 4f75f30 <_ZNK15SeparableFilter5applyEPKfPfii+0x1b0>
xorps %xmm0,%xmm0
mov %r14d,%r15d
xor %ecx,%ecx
nopw 0x0(%rax,%rax,1)
cmp %esi,%r15d
mov %esi,%ebx
cmovl %r15d,%ebx
test %ebx,%ebx
cmovle %r8d,%ebx
movss (%r10,%rbx,4),%xmm1
mulss 0x0(%r13,%rcx,4),%xmm1
lea 0x1(%r15),%ebx
cmp %esi,%ebx
cmovge %esi,%ebx
test %ebx,%ebx
cmovle %r8d,%ebx
addss %xmm0,%xmm1
movss (%r10,%rbx,4),%xmm0
mulss 0x4(%r13,%rcx,4),%xmm0
addss %xmm1,%xmm0
add $0x2,%rcx
add $0x2,%r15d
cmp %rcx,%rdi
jne 4f75ed0 <_ZNK15SeparableFilter5applyEPKfPfii+0x150>
test $0x1,%dl
je 4f75ea0 <_ZNK15SeparableFilter5applyEPKfPfii+0x120>
jmp 4f75f3e <_ZNK15SeparableFilter5applyEPKfPfii+0x1be>
nopl 0x0(%rax,%rax,1)
xorps %xmm0,%xmm0
xor %ecx,%ecx
test $0x1,%dl
je 4f75ea0 <_ZNK15SeparableFilter5applyEPKfPfii+0x120>
mov 0x8(%rsp),%rbx
add %r9d,%ebx
add %ecx,%ebx
cmp %esi,%ebx
cmovge %esi,%ebx
test %ebx,%ebx
cmovle %r8d,%ebx
movss (%r10,%rbx,4),%xmm1
mulss 0x0(%r13,%rcx,4),%xmm1
addss %xmm1,%xmm0
jmp 4f75ea0 <_ZNK15SeparableFilter5applyEPKfPfii+0x120>
dec %ebp
mov %edx,%esi
and $0x7ffffffe,%esi
xor %edi,%edi
mov 0x8(%rsp),%rcx
mov %ecx,%r8d
xor %r9d,%r9d
jmp 4f75fa9 <_ZNK15SeparableFilter5applyEPKfPfii+0x229>
data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov %rax,%r15
mov 0x10(%rsp),%r9
inc %r9
inc %r8d
cmp 0x20(%rsp),%r9
je 4f760ca <_ZNK15SeparableFilter5applyEPKfPfii+0x34a>
mov 0x8(%rsp),%rcx
lea (%rcx,%r9,1),%r10d
mov %r9,0x10(%rsp)
mov %r9,%rcx
imul %r15,%rcx
mov 0x18(%rsp),%r9
lea (%r9,%rcx,4),%r11
xor %r14d,%r14d
jmp 4f75fde <_ZNK15SeparableFilter5applyEPKfPfii+0x25e>
nopl 0x0(%rax)
movss %xmm0,(%r11,%r14,4)
inc %r14
cmp %rax,%r14
je 4f75f90 <_ZNK15SeparableFilter5applyEPKfPfii+0x210>
mov (%rsp),%rcx
lea (%rcx,%r14,4),%r9
cmpq $0x0,0x28(%rsp)
je 4f76060 <_ZNK15SeparableFilter5applyEPKfPfii+0x2e0>
xorps %xmm0,%xmm0
mov %r8d,%r15d
xor %ecx,%ecx
cs nopw 0x0(%rax,%rax,1)
cmp %ebp,%r15d
mov %ebp,%ebx
cmovl %r15d,%ebx
test %ebx,%ebx
cmovle %edi,%ebx
imul %r12d,%ebx
movss (%r9,%rbx,4),%xmm1
mulss 0x0(%r13,%rcx,4),%xmm1
lea 0x1(%r15),%ebx
cmp %ebp,%ebx
cmovge %ebp,%ebx
addss %xmm0,%xmm1
test %ebx,%ebx
cmovle %edi,%ebx
imul %r12d,%ebx
movss (%r9,%rbx,4),%xmm0
mulss 0x4(%r13,%rcx,4),%xmm0
addss %xmm1,%xmm0
add $0x2,%rcx
add $0x2,%r15d
cmp %rcx,%rsi
jne 4f76000 <_ZNK15SeparableFilter5applyEPKfPfii+0x280>
test $0x1,%dl
je 4f75fd0 <_ZNK15SeparableFilter5applyEPKfPfii+0x250>
jmp 4f7606e <_ZNK15SeparableFilter5applyEPKfPfii+0x2ee>
xchg %ax,%ax
xorps %xmm0,%xmm0
xor %ecx,%ecx
test $0x1,%dl
je 4f75fd0 <_ZNK15SeparableFilter5applyEPKfPfii+0x250>
lea (%r10,%rcx,1),%ebx
cmp %ebp,%ebx
cmovge %ebp,%ebx
test %ebx,%ebx
cmovle %edi,%ebx
imul %r12d,%ebx
movss (%r9,%rbx,4),%xmm1
mulss 0x0(%r13,%rcx,4),%xmm1
addss %xmm1,%xmm0
jmp 4f75fd0 <_ZNK15SeparableFilter5applyEPKfPfii+0x250>
mov %r14,%rdx
imul %r15,%rdx
shl $0x2,%rdx
mov (%rsp),%rdi
xor %esi,%esi
call 4da5b60 <memset$plt>
imul %r15,%r14
shl $0x2,%r14
mov 0x18(%rsp),%rdi
xor %esi,%esi
mov %r14,%rdx
call 4da5b60 <memset$plt>
cmpq $0x0,(%rsp)
je 4f760e9 <_ZNK15SeparableFilter5applyEPKfPfii+0x369>
mov 0x48(%rsp),%rsi
mov (%rsp),%rdi
sub %rdi,%rsi
add $0x58,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
jmp 71adab0 <_ZdlPvm>
add $0x58,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
ret
lea 0x30(%rsp),%rdi
call 4f75930 <_ZNKSt3__16vectorIfNS_9allocatorIfEEE20__throw_length_errorB8ne180100Ev>
mov %rax,%rbx
mov 0x30(%rsp),%rdi
test %rdi,%rdi
je 4f76121 <_ZNK15SeparableFilter5applyEPKfPfii+0x3a1>
mov %rdi,0x38(%rsp)
mov 0x40(%rsp),%rsi
sub %rdi,%rsi
call 71adab0 <_ZdlPvm>
mov %rbx,%rdi
call 4da5100 <_Unwind_Resume$plt>
int3
int3
int3
int3
int3
int3
int3
<_Unwind_Resume$plt>:
mov $0xf2,%r11d
jmp *0x24f7904(%rip) # 729ca10 <_Unwind_Resume@GCC_3.0>
int3
int3
int3
int3
<_ZNKSt3__16vectorIfNS_9allocatorIfEEE20__throw_length_errorB8ne180100Ev>:
push %rax
lea -0x484b8e(%rip),%rdi # 4af0daa <.L.str>
call 4da98d0 <_ZNSt3__120__throw_length_errorB8ne180100EPKc>
int3
int3
int3
<_ZdlPvm>:
endbr64
jmp 71ada90 <_ZdlPv>
int3
int3
int3
int3
int3
int3
int3
<_Znwm>:
endbr64
test %rdi,%rdi
mov $0x1,%eax
push %rbx
cmovne %rdi,%rax
mov %rax,%rbx
mov %rbx,%rdi
call 4da58c0 <malloc$plt>
test %rax,%rax
je 71af8d3 <_Znwm+0x23>
pop %rbx
ret
call 71af870 <_ZSt15get_new_handlerv>
test %rax,%rax
je 71af87c <_Znwm.cold>
call *%rax
jmp 71af8c4 <_Znwm+0x14>
<memset$plt>:
mov $0x198,%r11d
jmp *0x24f73d4(%rip) # 729cf40 <memset@GLIBC_2.2.5>
int3
int3
int3
int3
<_ZNSt3__120__throw_length_errorB8ne180100EPKc>:
push %r14
push %rbx
push %rax
mov %rdi,%r14
mov $0x10,%edi
call 71adf70 <__cxa_allocate_exception>
mov %rax,%rbx
mov %rax,%rdi
mov %r14,%rsi
call 4da9920 <_ZNSt12length_errorC2B8ne180100EPKc>
lea 0x24e1fba(%rip),%rsi # 728b8b0 <_ZTVN10__cxxabiv120__si_class_type_infoE@Base>
lea 0x24064b3(%rip),%rdx # 71afdb0 <_ZNSt12length_errorD1Ev>
mov %rbx,%rdi
call 71af5b0 <__cxa_throw>
mov %rax,%r14
mov %rbx,%rdi
call 71adfd0 <__cxa_free_exception>
mov %r14,%rdi
call 4da5100 <_Unwind_Resume$plt>
int3
int3
int3
int3
int3
int3
int3
int3
<_ZdlPv>:
endbr64
jmp 4da5340 <free$plt>
int3
int3
int3
int3
int3
int3
int3
<_ZSt15get_new_handlerv>:
endbr64
mov 0x3e2a85(%rip),%rax # 7592300 <_ZN12_GLOBAL__N_113__new_handlerE>
ret
<malloc$plt>:
mov $0x16e,%r11d
jmp *0x24f7524(%rip) # 729cdf0 <malloc@GLIBC_2.2.5>
int3
int3
int3
int3
<_ZNSt12length_errorC2B8ne180100EPKc>:
push %rbx
mov %rdi,%rbx
call 71b0410 <_ZNSt11logic_errorC1EPKc>
lea 0x24e2088(%rip),%rax # 728b9b8 <_ZTVSt12length_error>
add $0x10,%rax
mov %rax,(%rbx)
pop %rbx
ret
int3
int3
int3
int3
int3
int3
int3
<__cxa_allocate_exception>:
endbr64
push %rbx
lea 0x80(%rdi),%rbx
mov %rbx,%rdi
call 4da58c0 <malloc$plt>
test %rax,%rax
je 71adfbd <__cxa_allocate_exception+0x4d>
lea 0x8(%rax),%rdi
movq $0x0,(%rax)
lea 0x80(%rax),%rdx
and $0xfffffffffffffff8,%rdi
movq $0x0,0x78(%rax)
sub %rdi,%rax
lea 0x80(%rax),%ecx
xor %eax,%eax
shr $0x3,%ecx
rep stos %rax,%es:(%rdi)
mov %rdx,%rax
pop %rbx
ret
mov %rbx,%rdi
call 71ade90 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0>
test %rax,%rax
jne 71adf89 <__cxa_allocate_exception+0x19>
call 71af490 <_ZSt9terminatev>
int3
<__cxa_free_exception>:
endbr64
mov 0x3e4315(%rip),%rax # 75922f0 <_ZN12_GLOBAL__N_114emergency_poolE+0x30>
mov 0x3e4316(%rip),%rdx # 75922f8 <_ZN12_GLOBAL__N_114emergency_poolE+0x38>
add $0xffffffffffffff80,%rdi
add %rax,%rdx
cmp %rdx,%rdi
jae 71ae000 <__cxa_free_exception+0x30>
cmp %rdi,%rax
jae 71ae000 <__cxa_free_exception+0x30>
jmp 71add90 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0>
nopl 0x0(%rax,%rax,1)
jmp 4da5340 <free$plt>
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
<__cxa_throw>:
endbr64
push %r12
mov %rdx,%r12
push %rbp
mov %rsi,%rbp
push %rbx
mov %rdi,%rbx
call 71ae630 <__cxa_get_globals>
mov %r12,%rdx
mov %rbp,%rsi
mov %rbx,%rdi
addl $0x1,0x8(%rax)
call 71af560 <__cxa_init_primary_exception>
movl $0x1,(%rax)
lea 0x60(%rax),%rbx
mov %rbx,%rdi
call 4da5190 <_Unwind_RaiseException$plt>
mov %rbx,%rdi
call 71ae390 <__cxa_begin_catch>
call 71af490 <_ZSt9terminatev>
int3
int3
int3
int3
int3
int3
int3
int3
int3
<free$plt>:
mov $0x116,%r11d
jmp *0x24f77e4(%rip) # 729cb30 <free@GLIBC_2.2.5>
int3
int3
int3
int3
<_ZNSt11logic_errorC1EPKc>:
endbr64
push %r13
push %r12
push %rbp
mov %rdi,%rbp
push %rbx
sub $0x18,%rsp
lea 0xdb518(%rip),%rax # 728b940 <_ZTVSt11logic_error>
add $0x10,%rax
mov %rax,(%rdi)
test %rsi,%rsi
je 71b04d4 <_ZNSt11logic_errorC1EPKc+0xc4>
mov %rsi,%rdi
mov %rsi,%rbx
call 4da5620 <strlen$plt>
mov %rax,%r12
test %rax,%rax
jne 71b0468 <_ZNSt11logic_errorC1EPKc+0x58>
lea 0x3e1ece(%rip),%rax # 7592320 <_ZNSs4_Rep20_S_empty_rep_storageE>
lea 0x18(%rax),%rcx
mov %rcx,0x8(%rbp)
add $0x18,%rsp
pop %rbx
pop %rbp
pop %r12
pop %r13
ret
nopl (%rax)
lea 0xf(%rsp),%rdx
xor %esi,%esi
mov %rax,%rdi
call 71b2630 <_ZNSs4_Rep9_S_createEmmRKSaIcE>
mov %rax,%r13
lea 0x18(%rax),%rcx
cmp $0x1,%r12
je 71b04b0 <_ZNSt11logic_errorC1EPKc+0xa0>
mov %rcx,%rdi
mov %r12,%rdx
mov %rbx,%rsi
call 4da5c70 <memcpy$plt>
mov %rax,%rcx
cmp 0xe459c(%rip),%r13 # 7294a38 <_ZNSs4_Rep20_S_empty_rep_storageE$got>
jne 71b04c0 <_ZNSt11logic_errorC1EPKc+0xb0>
mov %rcx,0x8(%rbp)
add $0x18,%rsp
pop %rbx
pop %rbp
pop %r12
pop %r13
ret
nopl (%rax)
movzbl (%rbx),%eax
mov %al,0x18(%r13)
jmp 71b0495 <_ZNSt11logic_errorC1EPKc+0x85>
nopl 0x0(%rax)
movl $0x0,0x10(%r13)
mov %r12,0x0(%r13)
movb $0x0,0x18(%r13,%r12,1)
jmp 71b049e <_ZNSt11logic_errorC1EPKc+0x8e>
lea -0x240d6f3(%rip),%rdi # 4da2de8 <.LC0>
call 71b4a7f <_ZSt19__throw_logic_errorPKc>
endbr64
mov %rax,%rbx
jmp 71b03fa <_ZNSt11logic_errorC2EPKc.cold>
int3
int3
int3
int3
<_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0>:
push %rbp
lea 0x3e4428(%rip),%rbp # 75922c0 <_ZN12_GLOBAL__N_114emergency_poolE>
push %rbx
mov %rdi,%rbx
mov %rbp,%rdi
sub $0x8,%rsp
call 4da57a0 <pthread_mutex_lock$plt>
test %eax,%eax
jne 71ade84 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0.cold>
lea 0x10(%rbx),%rax
mov $0x10,%edx
mov 0x3e4428(%rip),%rbx # 75922e8 <_ZN12_GLOBAL__N_114emergency_poolE+0x28>
cmp %rdx,%rax
cmovb %rdx,%rax
lea 0x3e441a(%rip),%rdx # 75922e8 <_ZN12_GLOBAL__N_114emergency_poolE+0x28>
add $0xf,%rax
and $0xfffffffffffffff0,%rax
test %rbx,%rbx
jne 71adeed <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0+0x5d>
jmp 71adf12 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0+0x82>
nopl (%rax)
lea 0x8(%rbx),%rdx
mov 0x8(%rbx),%rbx
test %rbx,%rbx
je 71adf12 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0+0x82>
cmp %rax,(%rbx)
jb 71adee0 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0+0x50>
mov (%rbx),%rcx
mov 0x8(%rbx),%rdi
mov %rcx,%rsi
sub %rax,%rsi
cmp $0xf,%rsi
ja 71adf30 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0+0xa0>
mov %rcx,(%rbx)
mov %rdi,%rcx
mov %rcx,(%rdx)
add $0x10,%rbx
mov %rbp,%rdi
call 4da5c10 <pthread_mutex_unlock$plt>
test %eax,%eax
jne 71ade89 <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0.cold+0x5>
add $0x8,%rsp
mov %rbx,%rax
pop %rbx
pop %rbp
ret
nopl 0x0(%rax)
lea (%rbx,%rax,1),%rcx
mov %rdi,0x8(%rcx)
mov (%rdx),%rbx
mov %rsi,(%rcx)
mov %rax,(%rbx)
jmp 71adf0b <_ZN12_GLOBAL__N_14pool8allocateEm.constprop.0+0x7b>
<_ZSt9terminatev>:
endbr64
sub $0x8,%rsp
call 71af480 <_ZSt13get_terminatev>
mov %rax,%rdi
call 71af430 <_ZN10__cxxabiv111__terminateEPFvvE>
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
<_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0>:
push %rbp
lea 0x3e4528(%rip),%rbp # 75922c0 <_ZN12_GLOBAL__N_114emergency_poolE>
push %rbx
mov %rdi,%rbx
mov %rbp,%rdi
sub $0x8,%rsp
call 4da57a0 <pthread_mutex_lock$plt>
test %eax,%eax
jne 71add85 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0.cold+0x5>
mov 0x3e4531(%rip),%rax # 75922e8 <_ZN12_GLOBAL__N_114emergency_poolE+0x28>
mov -0x10(%rbx),%r9
lea -0x10(%rbx),%rdi
test %rax,%rax
je 71ade40 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xb0>
lea (%rdi,%r9,1),%rsi
cmp %rax,%rsi
jb 71ade40 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xb0>
mov 0x8(%rax),%rdx
cmp %rsi,%rax
je 71ade70 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xe0>
lea 0x3e4507(%rip),%r8 # 75922e8 <_ZN12_GLOBAL__N_114emergency_poolE+0x28>
test %rdx,%rdx
jne 71ade03 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0x73>
jmp 71ade1b <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0x8b>
nopl 0x0(%rax,%rax,1)
mov 0x8(%rdx),%rcx
lea 0x8(%rax),%r8
mov %rdx,%rax
test %rcx,%rcx
je 71ade18 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0x88>
mov %rcx,%rdx
cmp %rsi,%rdx
jb 71addf0 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0x60>
cmp %rdx,%rsi
jne 71ade18 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0x88>
mov 0x8(%rsi),%rdx
add (%rsi),%r9
mov %rdx,0x8(%rax)
mov (%r8),%rax
mov (%rax),%rdx
lea (%rax,%rdx,1),%rcx
cmp %rcx,%rdi
je 71ade68 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xd8>
mov 0x8(%rax),%rax
mov %r9,-0x10(%rbx)
mov %rax,-0x8(%rbx)
mov (%r8),%rax
mov %rdi,0x8(%rax)
jmp 71ade4b <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xbb>
nopl 0x0(%rax)
mov %rax,-0x8(%rbx)
mov %rdi,0x3e449d(%rip) # 75922e8 <_ZN12_GLOBAL__N_114emergency_poolE+0x28>
mov %rbp,%rdi
call 4da5c10 <pthread_mutex_unlock$plt>
test %eax,%eax
jne 71add80 <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0.cold>
add $0x8,%rsp
pop %rbx
pop %rbp
ret
nopw 0x0(%rax,%rax,1)
add %r9,%rdx
mov %rdx,(%rax)
jmp 71ade4b <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xbb>
add (%rax),%r9
mov %rdx,-0x8(%rbx)
mov %r9,-0x10(%rbx)
mov %rdi,0x3e4466(%rip) # 75922e8 <_ZN12_GLOBAL__N_114emergency_poolE+0x28>
jmp 71ade4b <_ZN12_GLOBAL__N_14pool4freeEPv.constprop.0+0xbb>
<_Unwind_RaiseException$plt>:
mov $0xfb,%r11d
jmp *0x24f78bc(%rip) # 729ca58 <_Unwind_RaiseException@GCC_3.0>
int3
int3
int3
int3
<__cxa_begin_catch>:
endbr64
push %rbx
mov %rdi,%rbx
call 71ae630 <__cxa_get_globals>
lea -0x50(%rbx),%rsi
movabs $0xb8b1aabcbcd4d500,%rdx
add (%rbx),%rdx
mov (%rax),%rcx
cmp $0x1,%rdx
jbe 71ae3c8 <__cxa_begin_catch+0x38>
test %rcx,%rcx
jne 71ae3fa <__cxa_begin_catch+0x6a>
mov %rsi,(%rax)
mov %rcx,%rax
pop %rbx
ret
nopl 0x0(%rax)
mov -0x28(%rbx),%r8d
mov $0x1,%edi
sub %r8d,%edi
lea 0x1(%r8),%edx
test %r8d,%r8d
cmovs %edi,%edx
mov %edx,-0x28(%rbx)
subl $0x1,0x8(%rax)
cmp %rsi,%rcx
je 71ae3f1 <__cxa_begin_catch+0x61>
mov %rcx,-0x30(%rbx)
mov %rsi,(%rax)
mov -0x8(%rbx),%rcx
pop %rbx
mov %rcx,%rax
ret
call 71af490 <_ZSt9terminatev>
int3
<__cxa_get_globals>:
endbr64
sub $0x8,%rsp
xor %eax,%eax
mov %fs:(%rax),%rax
sub $0x2b70,%rax
add $0x8,%rsp
add $0x2b60,%rax
ret
int3
<__cxa_init_primary_exception>:
endbr64
push %rbx
mov %rdi,%rbx
movl $0x0,-0x80(%rdi)
mov %rsi,-0x70(%rdi)
mov %rdx,-0x68(%rdi)
call 71af4f0 <_ZSt14get_unexpectedv>
mov %rax,-0x60(%rbx)
call 71af480 <_ZSt13get_terminatev>
mov %rax,-0x58(%rbx)
movabs $0x474e5543432b2b00,%rax
mov %rax,-0x20(%rbx)
lea -0x7e(%rip),%rax # 71af520 <_ZL23__gxx_exception_cleanup19_Unwind_Reason_CodeP17_Unwind_Exception>
mov %rax,-0x18(%rbx)
lea -0x80(%rbx),%rax
pop %rbx
ret
int3
int3
int3
int3
int3
int3
int3
int3
<_ZNSs4_Rep9_S_createEmmRKSaIcE>:
endbr64
movabs $0x3ffffffffffffff9,%rax
push %rbx
cmp %rdi,%rax
jb 71b269e <_ZNSs4_Rep9_S_createEmmRKSaIcE+0x6e>
mov %rdi,%rbx
cmp %rdi,%rsi
jae 71b2688 <_ZNSs4_Rep9_S_createEmmRKSaIcE+0x58>
lea (%rsi,%rsi,1),%rax
cmp %rax,%rdi
cmovb %rax,%rbx
lea 0x39(%rbx),%rax
cmp $0x1000,%rax
jbe 71b2688 <_ZNSs4_Rep9_S_createEmmRKSaIcE+0x58>
cmp %rbx,%rsi
jae 71b2688 <_ZNSs4_Rep9_S_createEmmRKSaIcE+0x58>
and $0xfff,%eax
add $0x1000,%rbx
sub %rax,%rbx
movabs $0x3ffffffffffffff9,%rax
cmp %rax,%rbx
cmova %rax,%rbx
lea 0x19(%rbx),%rdi
call 71af8b0 <_Znwm>
mov %rbx,0x8(%rax)
movl $0x0,0x10(%rax)
pop %rbx
ret
lea -0x259305a(%rip),%rdi # 4c1f64b <.LC4>
call 71b4b84 <_ZSt20__throw_length_errorPKc>
int3
int3
int3
int3
int3
int3
<_ZNSt11logic_errorC2EPKc.cold>:
mov %rbp,%rdi
call 71ae4e0 <_ZNSt9exceptionD1Ev>
mov %rbx,%rdi
call 4da5100 <_Unwind_Resume$plt>
int3
int3
int3
int3
int3
int3
<_ZSt19__throw_logic_errorPKc>:
endbr64
push %rbp
mov %rdi,%rbp
mov $0x10,%edi
push %rbx
push %rax
call 71adf70 <__cxa_allocate_exception>
mov %rbp,%rdi
mov %rax,%rbx
call 4da5740 <gettext$plt>
mov %rbx,%rdi
mov %rax,%rsi
call 71b0410 <_ZNSt11logic_errorC1EPKc>
lea -0x4e20(%rip),%rdx # 71afc90 <_ZNSt11logic_errorD1Ev>
lea 0xd6db1(%rip),%rsi # 728b868 <_ZTVN10__cxxabiv120__si_class_type_infoE@Base>
mov %rbx,%rdi
call 71af5b0 <__cxa_throw>
endbr64
mov %rax,%rbp
mov %rbx,%rdi
call 71adfd0 <__cxa_free_exception>
mov %rbp,%rdi
call 4da5100 <_Unwind_Resume$plt>
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x24f734c(%rip) # 729cfc8 <memcpy@GLIBC_2.14>
int3
int3
int3
int3
<strlen$plt>:
mov $0x144,%r11d
jmp *0x24f7674(%rip) # 729cca0 <strlen@GLIBC_2.2.5>
int3
int3
int3
int3
<pthread_mutex_lock$plt>:
mov $0x15c,%r11d
jmp *0x24f75b4(%rip) # 729cd60 <pthread_mutex_lock@GLIBC_2.2.5>
int3
int3
int3
int3
<pthread_mutex_unlock$plt>:
mov $0x1a3,%r11d
jmp *0x24f737c(%rip) # 729cf98 <pthread_mutex_unlock@GLIBC_2.2.5>
int3
int3
int3
int3
<_ZN10__cxxabiv111__terminateEPFvvE>:
endbr64
sub $0x8,%rsp
call *%rdi
jmp 71af423 <_ZN10__cxxabiv111__terminateEPFvvE.cold+0x5>
endbr64
mov %rax,%rdi
jmp 71af41e <_ZN10__cxxabiv111__terminateEPFvvE.cold>
int3
int3
int3
int3
int3
<_ZSt13get_terminatev>:
endbr64
lea 0xecdbd(%rip),%rax # 729c248 <_ZN10__cxxabiv119__terminate_handlerE>
mov (%rax),%rax
ret
int3
<_ZSt14get_unexpectedv>:
endbr64
lea 0xecd55(%rip),%rax # 729c250 <_ZN10__cxxabiv120__unexpected_handlerE>
mov (%rax),%rax
ret
int3
<_ZSt20__throw_length_errorPKc>:
endbr64
push %rbp
mov %rdi,%rbp
mov $0x10,%edi
push %rbx
push %rax
call 71adf70 <__cxa_allocate_exception>
mov %rbp,%rdi
mov %rax,%rbx
call 4da5740 <gettext$plt>
mov %rbx,%rdi
mov %rax,%rsi
call 71b0530 <_ZNSt12length_errorC1EPKc>
lea -0x4e05(%rip),%rdx # 71afdb0 <_ZNSt12length_errorD1Ev>
lea 0xd6cf4(%rip),%rsi # 728b8b0 <_ZTVN10__cxxabiv120__si_class_type_infoE@Base>
mov %rbx,%rdi
call 71af5b0 <__cxa_throw>
endbr64
mov %rax,%rbp
mov %rbx,%rdi
call 71adfd0 <__cxa_free_exception>
mov %rbp,%rdi
call 4da5100 <_Unwind_Resume$plt>
<_ZNSt9exceptionD1Ev>:
endbr64
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
<gettext$plt>:
mov $0x156,%r11d
jmp *0x24f75e4(%rip) # 729cd30 <gettext@GLIBC_2.2.5>
int3
int3
int3
int3
<_ZN10__cxxabiv111__terminateEPFvvE.cold>:
call 71ae390 <__cxa_begin_catch>
call 4da5d50 <abort$plt>
int3
int3
int3
int3
int3
int3
int3
int3
<_ZNSt12length_errorC1EPKc>:
endbr64
push %rbx
mov %rdi,%rbx
call 71b0410 <_ZNSt11logic_errorC1EPKc>
lea 0xdb474(%rip),%rax # 728b9b8 <_ZTVSt12length_error>
add $0x10,%rax
mov %rax,(%rbx)
pop %rbx
ret
int3
int3
int3
<abort$plt>:
mov $0x1b7,%r11d
jmp *0x24f72dc(%rip) # 729d038 <abort@GLIBC_2.2.5>
int3
int3
int3
int3