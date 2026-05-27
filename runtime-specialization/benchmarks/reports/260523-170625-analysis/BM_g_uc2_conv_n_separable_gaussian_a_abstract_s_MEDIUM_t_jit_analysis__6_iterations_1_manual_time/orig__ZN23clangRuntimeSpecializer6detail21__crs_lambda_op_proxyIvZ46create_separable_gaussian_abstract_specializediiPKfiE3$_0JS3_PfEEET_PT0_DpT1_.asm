<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ46create_separable_gaussian_abstract_specializediiPKfiE3$_0JS3_PfEEET_PT0_DpT1_>:
push %r15
push %r14
push %rbx
sub $0x40,%rsp
mov %rdi,%rbx
mov 0x14(%rdi),%eax
mov %eax,0x1c(%rsp)
test %eax,%eax
js 4f78de4 <_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ46create_separable_gaussian_abstract_specializediiPKfiE3$_0JS3_PfEEET_PT0_DpT1_+0x44>
add %eax,%eax
xor %ecx,%ecx
test %eax,%eax
cmovg %eax,%ecx
lea 0x4(,%rcx,4),%rax
lea 0x8(%rsp),%rdi
mov %rsi,%r14
mov %rbx,%rsi
mov %rdx,%r15
mov %rax,%rdx
call 4da5c70 <memcpy$plt>
mov %r14,%rsi
mov %r15,%rdx
lea 0x22679ed(%rip),%rax # 71e07d8 <_ZTV14GaussianKernel>
add $0x10,%rax
mov %rax,0x20(%rsp)
movups 0x8(%rsp),%xmm0
movups %xmm0,0x28(%rsp)
mov 0x18(%rsp),%rax
mov %rax,0x38(%rsp)
mov 0x18(%rbx),%ecx
mov 0x1c(%rbx),%r8d
lea 0x20(%rsp),%rdi
call 4f77760 <_ZNK9Convolver8convolveEPKfPfii>
add $0x40,%rsp
pop %rbx
pop %r14
pop %r15
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
<_ZNK9Convolver8convolveEPKfPfii>:
push %rbp
push %r15
push %r14
push %r13
push %r12
push %rbx
sub $0x78,%rsp
mov %r8d,%ebp
mov %ecx,%r12d
mov %rdx,0x10(%rsp)
mov %rsi,%rbx
mov %r8d,%eax
imul %ecx,%eax
xorps %xmm0,%xmm0
movaps %xmm0,0x30(%rsp)
movq $0x0,0x40(%rsp)
test %eax,%eax
je 4f777da <_ZNK9Convolver8convolveEPKfPfii+0x7a>
mov %rdi,(%rsp)
js 4f77aed <_ZNK9Convolver8convolveEPKfPfii+0x38d>
movslq %eax,%r15
lea 0x0(,%r15,4),%r14
mov %r14,%rdi
call 71af8b0 <_Znwm>
mov %rax,%r13
lea (%rax,%r15,4),%r15
mov %rax,%rdi
xor %esi,%esi
mov %r14,%rdx
call 4da5b60 <memset$plt>
mov (%rsp),%rdi
test %ebp,%ebp
mov %r15,0x50(%rsp)
jg 4f777ed <_ZNK9Convolver8convolveEPKfPfii+0x8d>
jmp 4f77abb <_ZNK9Convolver8convolveEPKfPfii+0x35b>
xor %r15d,%r15d
xor %r13d,%r13d
test %ebp,%ebp
mov %r15,0x50(%rsp)
jle 4f77abb <_ZNK9Convolver8convolveEPKfPfii+0x35b>
test %r12d,%r12d
jle 4f77abb <_ZNK9Convolver8convolveEPKfPfii+0x35b>
mov 0x1c(%rdi),%eax
movslq %eax,%rcx
mov %r12d,%r14d
mov %rcx,0x8(%rsp)
test %ecx,%ecx
js 4f77a8d <_ZNK9Convolver8convolveEPKfPfii+0x32d>
mov %eax,%ecx
neg %ecx
lea -0x1(%r12),%esi
movslq %ecx,%rdx
mov %ebp,%ecx
mov %rcx,0x18(%rsp)
add %rax,%rdx
mov %rdx,0x70(%rsp)
lea (%rdi,%rdx,4),%r8
add $0x10,%r8
mov $0x2,%ecx
sub %eax,%ecx
mov %rcx,0x60(%rsp)
lea (%rax,%rax,1),%r10d
mov $0x1,%ecx
mov %rax,(%rsp)
sub %eax,%ecx
mov %rcx,0x58(%rsp)
xor %ecx,%ecx
xorps %xmm0,%xmm0
xor %r9d,%r9d
mov %r13,0x20(%rsp)
mov %r14,0x28(%rsp)
jmp 4f77888 <_ZNK9Convolver8convolveEPKfPfii+0x128>
data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
mov 0x68(%rsp),%r9
inc %r9
cmp 0x18(%rsp),%r9
mov 0x20(%rsp),%r13
je 4f7795f <_ZNK9Convolver8convolveEPKfPfii+0x1ff>
mov %r9,0x68(%rsp)
imul %r14,%r9
lea 0x0(,%r9,4),%r11
add %r13,%r11
mov 0x58(%rsp),%r15
mov 0x60(%rsp),%rdx
xor %r13d,%r13d
jmp 4f778cc <_ZNK9Convolver8convolveEPKfPfii+0x16c>
nopl 0x0(%rax,%rax,1)
movss %xmm1,(%r11,%r13,4)
inc %r13
inc %rdx
inc %r15
mov 0x28(%rsp),%r14
cmp %r14,%r13
mov %rax,%rdi
je 4f77870 <_ZNK9Convolver8convolveEPKfPfii+0x110>
mov %r13d,%r14d
sub (%rsp),%r14d
cmp %esi,%r14d
cmovge %esi,%r14d
test %r14d,%r14d
cmovle %ecx,%r14d
add %r9d,%r14d
movss (%rbx,%r14,4),%xmm1
mov %rdi,%rax
mov 0x70(%rsp),%r14
mulss 0x8(%rdi,%r14,4),%xmm1
addss %xmm0,%xmm1
cmpl $0x0,0x8(%rsp)
je 4f778b0 <_ZNK9Convolver8convolveEPKfPfii+0x150>
xor %r14d,%r14d
nopw 0x0(%rax,%rax,1)
lea (%r15,%r14,1),%edi
cmp %esi,%edi
cmovge %esi,%edi
test %edi,%edi
cmovle %ecx,%edi
add %r9d,%edi
movss (%rbx,%rdi,4),%xmm2
mulss -0x4(%r8,%r14,4),%xmm2
lea (%rdx,%r14,1),%edi
cmp %esi,%edi
cmovge %esi,%edi
addss %xmm1,%xmm2
test %edi,%edi
cmovle %ecx,%edi
add %r9d,%edi
movss (%rbx,%rdi,4),%xmm1
mulss (%r8,%r14,4),%xmm1
addss %xmm2,%xmm1
add $0x2,%r14
cmp %r14d,%r10d
jne 4f77910 <_ZNK9Convolver8convolveEPKfPfii+0x1b0>
jmp 4f778b0 <_ZNK9Convolver8convolveEPKfPfii+0x150>
dec %ebp
mov (%rsp),%r14
lea (%rdi,%r14,4),%rsi
mov %r14,%rdi
mov 0x8(%rsp),%rdx
sub %rdx,%rdi
mov %rdx,%rcx
shl $0x2,%rcx
sub %rcx,%rsi
add $0x10,%rsi
mov $0x2,%r8d
sub %r14d,%r8d
add %edx,%edx
mov $0x1,%r9d
sub %r14d,%r9d
xor %ecx,%ecx
xorps %xmm0,%xmm0
xor %ebx,%ebx
jmp 4f779b9 <_ZNK9Convolver8convolveEPKfPfii+0x259>
nop
mov 0x8(%rsp),%rbx
inc %rbx
inc %r8
inc %r9
cmp 0x18(%rsp),%rbx
je 4f77ac0 <_ZNK9Convolver8convolveEPKfPfii+0x360>
mov %r12d,%r11d
imul %ebx,%r11d
mov 0x10(%rsp),%r10
lea (%r10,%r11,4),%r11
mov %rbx,0x8(%rsp)
sub %r14d,%ebx
cmp %ebp,%ebx
cmovge %ebp,%ebx
test %ebx,%ebx
cmovle %ecx,%ebx
imul %r12d,%ebx
xor %r15d,%r15d
jmp 4f77a09 <_ZNK9Convolver8convolveEPKfPfii+0x2a9>
data16 data16 cs nopw 0x0(%rax,%rax,1)
movss %xmm1,(%r11,%r15,4)
inc %r15
cmp 0x28(%rsp),%r15
mov 0x20(%rsp),%r13
mov (%rsp),%r14
je 4f779a0 <_ZNK9Convolver8convolveEPKfPfii+0x240>
lea 0x0(%r13,%r15,4),%r13
movss 0x0(%r13,%rbx,4),%xmm1
mulss 0x8(%rax,%rdi,4),%xmm1
addss %xmm0,%xmm1
test %r14d,%r14d
je 4f779f0 <_ZNK9Convolver8convolveEPKfPfii+0x290>
xor %r14d,%r14d
nopw 0x0(%rax,%rax,1)
lea (%r9,%r14,1),%r10d
cmp %ebp,%r10d
cmovge %ebp,%r10d
test %r10d,%r10d
cmovle %ecx,%r10d
imul %r12d,%r10d
movss 0x0(%r13,%r10,4),%xmm2
mulss -0x4(%rsi,%r14,4),%xmm2
lea (%r8,%r14,1),%r10d
cmp %ebp,%r10d
cmovge %ebp,%r10d
addss %xmm1,%xmm2
test %r10d,%r10d
cmovle %ecx,%r10d
imul %r12d,%r10d
movss 0x0(%r13,%r10,4),%xmm1
mulss (%rsi,%r14,4),%xmm1
addss %xmm2,%xmm1
add $0x2,%r14
cmp %r14d,%edx
jne 4f77a30 <_ZNK9Convolver8convolveEPKfPfii+0x2d0>
jmp 4f779f0 <_ZNK9Convolver8convolveEPKfPfii+0x290>
mov %ebp,%ebx
mov %r14,%rdx
imul %rbx,%rdx
shl $0x2,%rdx
mov %r13,%rdi
xor %esi,%esi
call 4da5b60 <memset$plt>
imul %rbx,%r14
shl $0x2,%r14
mov 0x10(%rsp),%rdi
xor %esi,%esi
mov %r14,%rdx
call 4da5b60 <memset$plt>
test %r13,%r13
je 4f77ade <_ZNK9Convolver8convolveEPKfPfii+0x37e>
mov 0x50(%rsp),%rsi
sub %r13,%rsi
mov %r13,%rdi
add $0x78,%rsp
pop %rbx
pop %r12
pop %r13
pop %r14
pop %r15
pop %rbp
jmp 71adab0 <_ZdlPvm>
add $0x78,%rsp
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
je 4f77b16 <_ZNK9Convolver8convolveEPKfPfii+0x3b6>
mov %rdi,0x38(%rsp)
mov 0x40(%rsp),%rsi
sub %rdi,%rsi
call 71adab0 <_ZdlPvm>
mov %rbx,%rdi
call 4da5100 <_Unwind_Resume$plt>
int3
int3
<memcpy$plt>:
mov $0x1a9,%r11d
jmp *0x24f734c(%rip) # 729cfc8 <memcpy@GLIBC_2.14>
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