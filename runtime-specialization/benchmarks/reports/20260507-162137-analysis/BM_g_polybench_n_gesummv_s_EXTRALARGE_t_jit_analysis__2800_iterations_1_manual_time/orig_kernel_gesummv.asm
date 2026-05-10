<kernel_gesummv>:
test %edi,%edi
jle 17f4368 <kernel_gesummv+0xc8>
lea 0x241a491(%rip),%rax # 3c0e740 <g_gesummv_A>
mov (%rax),%rax
lea 0x241a48f(%rip),%rcx # 3c0e748 <g_gesummv_B>
mov (%rcx),%rcx
mov %edi,%edx
xor %esi,%esi
lea 0x241a489(%rip),%rdi # 3c0e750 <g_gesummv_tmp>
lea 0x2425382(%rip),%r8 # 3c19650 <g_gesummv_y>
lea 0x241fbfb(%rip),%r9 # 3c13ed0 <g_gesummv_x>
movsd -0x2d395d(%rip),%xmm0 # 1520980 <.LCPI107_2>
movsd -0x2d3add(%rip),%xmm1 # 1520808 <.LCPI107_0>
data16 cs nopw 0x0(%rax,%rax,1)
movq $0x0,(%rdi,%rsi,8)
movq $0x0,(%r8,%rsi,8)
xorpd %xmm2,%xmm2
xorpd %xmm3,%xmm3
xor %r10d,%r10d
nopl 0x0(%rax,%rax,1)
movsd (%r9,%r10,8),%xmm4
movsd (%rax,%r10,8),%xmm5
mulsd %xmm4,%xmm5
addsd %xmm5,%xmm3
movsd %xmm3,(%rdi,%rsi,8)
mulsd (%rcx,%r10,8),%xmm4
addsd %xmm4,%xmm2
movsd %xmm2,(%r8,%rsi,8)
inc %r10
cmp %r10,%rdx
jne 17f4310 <kernel_gesummv+0x70>
mulsd %xmm0,%xmm2
mulsd %xmm1,%xmm3
addsd %xmm2,%xmm3
movsd %xmm3,(%r8,%rsi,8)
inc %rsi
add $0x5780,%rcx
add $0x5780,%rax
cmp %rdx,%rsi
jne 17f42f0 <kernel_gesummv+0x50>
ret
int3
int3
int3
int3
int3
int3
int3