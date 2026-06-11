00000000006ad060 <sqlite3VdbeExec>:
  6ad060:	55                   	push   %rbp
  6ad061:	41 57                	push   %r15
  6ad063:	41 56                	push   %r14
  6ad065:	41 55                	push   %r13
  6ad067:	41 54                	push   %r12
  6ad069:	53                   	push   %rbx
  6ad06a:	48 81 ec 28 01 00 00 	sub    $0x128,%rsp
  6ad071:	49 89 fe             	mov    %rdi,%r14
  6ad074:	4c 8b bf 88 00 00 00 	mov    0x88(%rdi),%r15
  6ad07b:	4c 8b 27             	mov    (%rdi),%r12
  6ad07e:	4c 8b 6f 68          	mov    0x68(%rdi),%r13
  6ad082:	41 0f b6 6c 24 64    	movzbl 0x64(%r12),%ebp
  6ad088:	8b 87 d0 00 00 00    	mov    0xd0(%rdi),%eax
  6ad08e:	85 c0                	test   %eax,%eax
  6ad090:	0f 84 cc 00 00 00    	je     6ad162 <sqlite3VdbeExec+0x102>
  6ad096:	49 63 54 24 28       	movslq 0x28(%r12),%rdx
  6ad09b:	48 85 d2             	test   %rdx,%rdx
  6ad09e:	0f 8e be 00 00 00    	jle    6ad162 <sqlite3VdbeExec+0x102>
  6ad0a4:	49 8b 4c 24 20       	mov    0x20(%r12),%rcx
  6ad0a9:	a8 01                	test   $0x1,%al
  6ad0ab:	74 14                	je     6ad0c1 <sqlite3VdbeExec+0x61>
  6ad0ad:	48 8b 71 08          	mov    0x8(%rcx),%rsi
  6ad0b1:	48 85 f6             	test   %rsi,%rsi
  6ad0b4:	74 0b                	je     6ad0c1 <sqlite3VdbeExec+0x61>
  6ad0b6:	48 8b 3e             	mov    (%rsi),%rdi
  6ad0b9:	48 8b 76 08          	mov    0x8(%rsi),%rsi
  6ad0bd:	48 89 7e 08          	mov    %rdi,0x8(%rsi)
  6ad0c1:	83 fa 03             	cmp    $0x3,%edx
  6ad0c4:	0f 82 98 00 00 00    	jb     6ad162 <sqlite3VdbeExec+0x102>
  6ad0ca:	be 02 00 00 00       	mov    $0x2,%esi
  6ad0cf:	83 fa 03             	cmp    $0x3,%edx
  6ad0d2:	74 6b                	je     6ad13f <sqlite3VdbeExec+0xdf>
  6ad0d4:	89 d7                	mov    %edx,%edi
  6ad0d6:	81 e7 fe ff ff 7f    	and    $0x7ffffffe,%edi
  6ad0dc:	48 83 c7 fc          	add    $0xfffffffffffffffc,%rdi
  6ad0e0:	4c 8d 41 68          	lea    0x68(%rcx),%r8
  6ad0e4:	41 b9 02 00 00 00    	mov    $0x2,%r9d
  6ad0ea:	eb 14                	jmp    6ad100 <sqlite3VdbeExec+0xa0>
  6ad0ec:	0f 1f 40 00          	nopl   0x0(%rax)
  6ad0f0:	49 83 c0 40          	add    $0x40,%r8
  6ad0f4:	4d 8d 51 fd          	lea    -0x3(%r9),%r10
  6ad0f8:	49 ff c1             	inc    %r9
  6ad0fb:	49 39 fa             	cmp    %rdi,%r10
  6ad0fe:	74 3b                	je     6ad13b <sqlite3VdbeExec+0xdb>
  6ad100:	4c 89 ce             	mov    %r9,%rsi
  6ad103:	0f a3 f0             	bt     %esi,%eax
  6ad106:	73 14                	jae    6ad11c <sqlite3VdbeExec+0xbc>
  6ad108:	4d 8b 48 e0          	mov    -0x20(%r8),%r9
  6ad10c:	4d 85 c9             	test   %r9,%r9
  6ad10f:	74 0b                	je     6ad11c <sqlite3VdbeExec+0xbc>
  6ad111:	4d 8b 11             	mov    (%r9),%r10
  6ad114:	4d 8b 49 08          	mov    0x8(%r9),%r9
  6ad118:	4d 89 51 08          	mov    %r10,0x8(%r9)
  6ad11c:	4c 8d 4e 01          	lea    0x1(%rsi),%r9
  6ad120:	44 0f a3 c8          	bt     %r9d,%eax
  6ad124:	73 ca                	jae    6ad0f0 <sqlite3VdbeExec+0x90>
  6ad126:	4d 8b 10             	mov    (%r8),%r10
  6ad129:	4d 85 d2             	test   %r10,%r10
  6ad12c:	74 c2                	je     6ad0f0 <sqlite3VdbeExec+0x90>
  6ad12e:	4d 8b 1a             	mov    (%r10),%r11
  6ad131:	4d 8b 52 08          	mov    0x8(%r10),%r10
  6ad135:	4d 89 5a 08          	mov    %r11,0x8(%r10)
  6ad139:	eb b5                	jmp    6ad0f0 <sqlite3VdbeExec+0x90>
  6ad13b:	48 83 c6 02          	add    $0x2,%rsi
  6ad13f:	f6 c2 01             	test   $0x1,%dl
  6ad142:	74 1e                	je     6ad162 <sqlite3VdbeExec+0x102>
  6ad144:	0f a3 f0             	bt     %esi,%eax
  6ad147:	73 19                	jae    6ad162 <sqlite3VdbeExec+0x102>
  6ad149:	48 c1 e6 05          	shl    $0x5,%rsi
  6ad14d:	48 8b 44 31 08       	mov    0x8(%rcx,%rsi,1),%rax
  6ad152:	48 85 c0             	test   %rax,%rax
  6ad155:	74 0b                	je     6ad162 <sqlite3VdbeExec+0x102>
  6ad157:	48 8b 08             	mov    (%rax),%rcx
  6ad15a:	48 8b 40 08          	mov    0x8(%rax),%rax
  6ad15e:	48 89 48 08          	mov    %rcx,0x8(%rax)
  6ad162:	49 83 bc 24 08 02 00 	cmpq   $0x0,0x208(%r12)
  6ad169:	00 00 
  6ad16b:	74 17                	je     6ad184 <sqlite3VdbeExec+0x124>
  6ad16d:	41 8b 86 e4 00 00 00 	mov    0xe4(%r14),%eax
  6ad174:	41 8b 9c 24 18 02 00 	mov    0x218(%r12),%ebx
  6ad17b:	00 
  6ad17c:	31 d2                	xor    %edx,%edx
  6ad17e:	f7 f3                	div    %ebx
  6ad180:	29 d3                	sub    %edx,%ebx
  6ad182:	eb 07                	jmp    6ad18b <sqlite3VdbeExec+0x12b>
  6ad184:	48 c7 c3 ff ff ff ff 	mov    $0xffffffffffffffff,%rbx
  6ad18b:	41 83 7e 34 07       	cmpl   $0x7,0x34(%r14)
  6ad190:	4c 89 74 24 30       	mov    %r14,0x30(%rsp)
  6ad195:	4c 89 7c 24 58       	mov    %r15,0x58(%rsp)
  6ad19a:	75 3c                	jne    6ad1d8 <sqlite3VdbeExec+0x178>
  6ad19c:	4d 89 fd             	mov    %r15,%r13
  6ad19f:	31 c0                	xor    %eax,%eax
  6ad1a1:	45 31 ff             	xor    %r15d,%r15d
  6ad1a4:	48 89 44 24 48       	mov    %rax,0x48(%rsp)
  6ad1a9:	41 80 7c 24 67 00    	cmpb   $0x0,0x67(%r12)
  6ad1af:	75 0c                	jne    6ad1bd <sqlite3VdbeExec+0x15d>
  6ad1b1:	41 80 7c 24 68 00    	cmpb   $0x0,0x68(%r12)
  6ad1b7:	0f 84 c6 00 00 00    	je     6ad283 <sqlite3VdbeExec+0x223>
  6ad1bd:	48 8d 35 57 10 fa ff 	lea    -0x5efa9(%rip),%rsi        # 64e21b <.L.str.16>
  6ad1c4:	4c 89 f7             	mov    %r14,%rdi
  6ad1c7:	31 c0                	xor    %eax,%eax
  6ad1c9:	e8 12 59 ff ff       	call   6a2ae0 <sqlite3VdbeError>
  6ad1ce:	b8 07 00 00 00       	mov    $0x7,%eax
  6ad1d3:	e9 0c 92 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6ad1d8:	41 c7 46 34 00 00 00 	movl   $0x0,0x34(%r14)
  6ad1df:	00 
  6ad1e0:	49 c7 46 48 00 00 00 	movq   $0x0,0x48(%r14)
  6ad1e7:	00 
  6ad1e8:	41 c7 84 24 90 02 00 	movl   $0x0,0x290(%r12)
  6ad1ef:	00 00 00 00 00 
  6ad1f4:	41 8b 84 24 90 01 00 	mov    0x190(%r12),%eax
  6ad1fb:	00 
  6ad1fc:	85 c0                	test   %eax,%eax
  6ad1fe:	74 19                	je     6ad219 <sqlite3VdbeExec+0x1b9>
  6ad200:	4d 89 fd             	mov    %r15,%r13
  6ad203:	48 c7 44 24 48 00 00 	movq   $0x0,0x48(%rsp)
  6ad20a:	00 00 
  6ad20c:	b8 09 00 00 00       	mov    $0x9,%eax
  6ad211:	45 31 ff             	xor    %r15d,%r15d
  6ad214:	e9 cb 91 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6ad219:	4c 89 6c 24 28       	mov    %r13,0x28(%rsp)
  6ad21e:	48 c7 84 24 d8 00 00 	movq   $0x0,0xd8(%rsp)
  6ad225:	00 00 00 00 00 
  6ad22a:	40 88 6c 24 1f       	mov    %bpl,0x1f(%rsp)
  6ad22f:	49 63 46 30          	movslq 0x30(%r14),%rax
  6ad233:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6ad237:	4d 8d 2c c7          	lea    (%r15,%rax,8),%r13
  6ad23b:	49 8d 86 28 01 00 00 	lea    0x128(%r14),%rax
  6ad242:	48 89 84 24 20 01 00 	mov    %rax,0x120(%rsp)
  6ad249:	00 
  6ad24a:	49 8d 86 a8 00 00 00 	lea    0xa8(%r14),%rax
  6ad251:	48 89 84 24 d0 00 00 	mov    %rax,0xd0(%rsp)
  6ad258:	00 
  6ad259:	45 31 ff             	xor    %r15d,%r15d
  6ad25c:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6ad263:	00 00 00 00 
  6ad267:	48 c7 44 24 48 00 00 	movq   $0x0,0x48(%rsp)
  6ad26e:	00 00 
  6ad270:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ad277:	00 00 
  6ad279:	4c 89 64 24 10       	mov    %r12,0x10(%rsp)
  6ad27e:	e9 9d 00 00 00       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ad283:	41 c6 44 24 67 01    	movb   $0x1,0x67(%r12)
  6ad289:	41 83 bc 24 dc 00 00 	cmpl   $0x0,0xdc(%r12)
  6ad290:	00 00 
  6ad292:	7e 0c                	jle    6ad2a0 <sqlite3VdbeExec+0x240>
  6ad294:	41 c7 84 24 90 01 00 	movl   $0x1,0x190(%r12)
  6ad29b:	00 01 00 00 00 
  6ad2a0:	41 ff 84 24 98 01 00 	incl   0x198(%r12)
  6ad2a7:	00 
  6ad2a8:	66 41 c7 84 24 9c 01 	movw   $0x0,0x19c(%r12)
  6ad2af:	00 00 00 00 
  6ad2b3:	49 8b bc 24 58 01 00 	mov    0x158(%r12),%rdi
  6ad2ba:	00 
  6ad2bb:	48 85 ff             	test   %rdi,%rdi
  6ad2be:	0f 84 f9 fe ff ff    	je     6ad1bd <sqlite3VdbeExec+0x15d>
  6ad2c4:	48 8d 35 50 0f fa ff 	lea    -0x5f0b0(%rip),%rsi        # 64e21b <.L.str.16>
  6ad2cb:	31 c0                	xor    %eax,%eax
  6ad2cd:	e8 5e dc fc ff       	call   67af30 <sqlite3ErrorMsg>
  6ad2d2:	49 8b 84 24 58 01 00 	mov    0x158(%r12),%rax
  6ad2d9:	00 
  6ad2da:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
  6ad2e0:	c7 40 18 07 00 00 00 	movl   $0x7,0x18(%rax)
  6ad2e7:	48 8b 80 08 01 00 00 	mov    0x108(%rax),%rax
  6ad2ee:	48 85 c0             	test   %rax,%rax
  6ad2f1:	0f 84 c6 fe ff ff    	je     6ad1bd <sqlite3VdbeExec+0x15d>
  6ad2f7:	ff 40 34             	incl   0x34(%rax)
  6ad2fa:	eb e4                	jmp    6ad2e0 <sqlite3VdbeExec+0x280>
  6ad2fc:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ad303:	00 00 
  6ad305:	85 c0                	test   %eax,%eax
  6ad307:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6ad30c:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ad311:	0f 85 a0 87 00 00    	jne    6b5ab7 <sqlite3VdbeExec+0x8a57>
  6ad317:	49 83 c5 18          	add    $0x18,%r13
  6ad31b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  6ad320:	49 ff c7             	inc    %r15
  6ad323:	4c 89 3c 24          	mov    %r15,(%rsp)
  6ad327:	45 0f b6 7d 00       	movzbl 0x0(%r13),%r15d
  6ad32c:	49 81 ff b8 00 00 00 	cmp    $0xb8,%r15
  6ad333:	0f 87 47 47 00 00    	ja     6b1a80 <sqlite3VdbeExec+0x4a20>
  6ad339:	4c 89 6c 24 08       	mov    %r13,0x8(%rsp)
  6ad33e:	48 8d 0d b7 a6 b9 ff 	lea    -0x465949(%rip),%rcx        # 2479fc <RuntimeSpecializeableIR_data+0x1fdd0c>
  6ad345:	4a 63 04 b9          	movslq (%rcx,%r15,4),%rax
  6ad349:	48 01 c8             	add    %rcx,%rax
  6ad34c:	ff e0                	jmp    *%rax
  6ad34e:	48 8b 74 24 08       	mov    0x8(%rsp),%rsi
  6ad353:	4c 63 6e 04          	movslq 0x4(%rsi),%r13
  6ad357:	49 6b c5 38          	imul   $0x38,%r13,%rax
  6ad35b:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6ad360:	4c 8d 04 02          	lea    (%rdx,%rax,1),%r8
  6ad364:	4c 63 66 0c          	movslq 0xc(%rsi),%r12
  6ad368:	49 6b cc 38          	imul   $0x38,%r12,%rcx
  6ad36c:	48 8d 3c 0a          	lea    (%rdx,%rcx,1),%rdi
  6ad370:	44 0f b7 54 02 14    	movzwl 0x14(%rdx,%rax,1),%r10d
  6ad376:	0f b7 6c 0a 14       	movzwl 0x14(%rdx,%rcx,1),%ebp
  6ad37b:	41 0f b7 ca          	movzwl %r10w,%ecx
  6ad37f:	0f b7 c5             	movzwl %bp,%eax
  6ad382:	89 c2                	mov    %eax,%edx
  6ad384:	21 ca                	and    %ecx,%edx
  6ad386:	f6 c2 04             	test   $0x4,%dl
  6ad389:	0f 85 05 03 00 00    	jne    6ad694 <sqlite3VdbeExec+0x634>
  6ad38f:	41 89 c1             	mov    %eax,%r9d
  6ad392:	41 09 c9             	or     %ecx,%r9d
  6ad395:	0f b7 76 02          	movzwl 0x2(%rsi),%esi
  6ad399:	41 f6 c1 01          	test   $0x1,%r9b
  6ad39d:	0f 85 3e 08 00 00    	jne    6adbe1 <sqlite3VdbeExec+0xb81>
  6ad3a3:	83 e6 47             	and    $0x47,%esi
  6ad3a6:	66 83 fe 43          	cmp    $0x43,%si
  6ad3aa:	4c 89 44 24 38       	mov    %r8,0x38(%rsp)
  6ad3af:	48 89 7c 24 40       	mov    %rdi,0x40(%rsp)
  6ad3b4:	0f 82 7f 3e 00 00    	jb     6b1239 <sqlite3VdbeExec+0x41d9>
  6ad3ba:	41 f6 c1 02          	test   $0x2,%r9b
  6ad3be:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad3c3:	4c 89 54 24 50       	mov    %r10,0x50(%rsp)
  6ad3c8:	0f 85 3e 50 00 00    	jne    6b240c <sqlite3VdbeExec+0x53ac>
  6ad3ce:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ad3d3:	e9 61 6c 00 00       	jmp    6b4039 <sqlite3VdbeExec+0x6fd9>
  6ad3d8:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ad3dd:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6ad3e1:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad3e5:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6ad3ea:	48 8d 3c 02          	lea    (%rdx,%rax,1),%rdi
  6ad3ee:	0f b7 6c 02 14       	movzwl 0x14(%rdx,%rax,1),%ebp
  6ad3f3:	48 63 41 08          	movslq 0x8(%rcx),%rax
  6ad3f7:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad3fb:	4c 8d 2c 02          	lea    (%rdx,%rax,1),%r13
  6ad3ff:	0f b7 44 02 14       	movzwl 0x14(%rdx,%rax,1),%eax
  6ad404:	48 63 49 0c          	movslq 0xc(%rcx),%rcx
  6ad408:	4c 6b e1 38          	imul   $0x38,%rcx,%r12
  6ad40c:	49 01 d4             	add    %rdx,%r12
  6ad40f:	0f b7 c8             	movzwl %ax,%ecx
  6ad412:	89 ea                	mov    %ebp,%edx
  6ad414:	21 ca                	and    %ecx,%edx
  6ad416:	f6 c2 04             	test   $0x4,%dl
  6ad419:	0f 84 73 07 00 00    	je     6adb92 <sqlite3VdbeExec+0xb32>
  6ad41f:	48 8b 37             	mov    (%rdi),%rsi
  6ad422:	49 8b 45 00          	mov    0x0(%r13),%rax
  6ad426:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6ad42b:	41 0f b6 cf          	movzbl %r15b,%ecx
  6ad42f:	83 c1 95             	add    $0xffffff95,%ecx
  6ad432:	83 f9 03             	cmp    $0x3,%ecx
  6ad435:	0f 87 7d 40 00 00    	ja     6b14b8 <sqlite3VdbeExec+0x4458>
  6ad43b:	48 8d 15 ca a8 b9 ff 	lea    -0x465736(%rip),%rdx        # 247d0c <RuntimeSpecializeableIR_data+0x1fe01c>
  6ad442:	48 63 0c 8a          	movslq (%rdx,%rcx,4),%rcx
  6ad446:	48 01 d1             	add    %rdx,%rcx
  6ad449:	ff e1                	jmp    *%rcx
  6ad44b:	48 85 f6             	test   %rsi,%rsi
  6ad44e:	0f 88 12 65 00 00    	js     6b3966 <sqlite3VdbeExec+0x6906>
  6ad454:	48 85 c0             	test   %rax,%rax
  6ad457:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad45b:	0f 8e 2c 65 00 00    	jle    6b398d <sqlite3VdbeExec+0x692d>
  6ad461:	48 b9 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rcx
  6ad468:	ff ff 7f 
  6ad46b:	48 29 c1             	sub    %rax,%rcx
  6ad46e:	48 39 f1             	cmp    %rsi,%rcx
  6ad471:	0f 82 d4 3f 00 00    	jb     6b144b <sqlite3VdbeExec+0x43eb>
  6ad477:	e9 11 65 00 00       	jmp    6b398d <sqlite3VdbeExec+0x692d>
  6ad47c:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ad481:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6ad485:	4c 6b e8 38          	imul   $0x38,%rax,%r13
  6ad489:	48 63 41 08          	movslq 0x8(%rcx),%rax
  6ad48d:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad491:	48 63 49 0c          	movslq 0xc(%rcx),%rcx
  6ad495:	48 6b f9 38          	imul   $0x38,%rcx,%rdi
  6ad499:	4c 8b 44 24 28       	mov    0x28(%rsp),%r8
  6ad49e:	4c 01 c7             	add    %r8,%rdi
  6ad4a1:	43 8b 4c 28 14       	mov    0x14(%r8,%r13,1),%ecx
  6ad4a6:	41 0f b7 54 00 14    	movzwl 0x14(%r8,%rax,1),%edx
  6ad4ac:	89 d6                	mov    %edx,%esi
  6ad4ae:	09 ce                	or     %ecx,%esi
  6ad4b0:	40 f6 c6 01          	test   $0x1,%sil
  6ad4b4:	0f 85 58 06 00 00    	jne    6adb12 <sqlite3VdbeExec+0xab2>
  6ad4ba:	4d 01 c5             	add    %r8,%r13
  6ad4bd:	4c 01 c0             	add    %r8,%rax
  6ad4c0:	f6 c2 24             	test   $0x24,%dl
  6ad4c3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad4c7:	0f 84 54 3e 00 00    	je     6b1321 <sqlite3VdbeExec+0x42c1>
  6ad4cd:	4c 8b 20             	mov    (%rax),%r12
  6ad4d0:	0f b7 c1             	movzwl %cx,%eax
  6ad4d3:	a8 24                	test   $0x24,%al
  6ad4d5:	0f 84 83 3e 00 00    	je     6b135e <sqlite3VdbeExec+0x42fe>
  6ad4db:	49 8b 45 00          	mov    0x0(%r13),%rax
  6ad4df:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad4e4:	41 0f b6 55 00       	movzbl 0x0(%r13),%edx
  6ad4e9:	83 fa 68             	cmp    $0x68,%edx
  6ad4ec:	0f 84 f2 82 00 00    	je     6b57e4 <sqlite3VdbeExec+0x8784>
  6ad4f2:	83 fa 67             	cmp    $0x67,%edx
  6ad4f5:	0f 85 e8 42 00 00    	jne    6b17e3 <sqlite3VdbeExec+0x4783>
  6ad4fb:	49 21 c4             	and    %rax,%r12
  6ad4fe:	e9 e4 82 00 00       	jmp    6b57e7 <sqlite3VdbeExec+0x8787>
  6ad503:	4c 8b 2c 24          	mov    (%rsp),%r13
  6ad507:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ad50b:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6ad510:	48 63 4a 04          	movslq 0x4(%rdx),%rcx
  6ad514:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6ad518:	48 8b 48 30          	mov    0x30(%rax),%rcx
  6ad51c:	48 89 8c 24 e8 00 00 	mov    %rcx,0xe8(%rsp)
  6ad523:	00 
  6ad524:	0f b7 4a 10          	movzwl 0x10(%rdx),%ecx
  6ad528:	66 89 8c 24 04 01 00 	mov    %cx,0x104(%rsp)
  6ad52f:	00 
  6ad530:	31 ed                	xor    %ebp,%ebp
  6ad532:	41 80 ff 2a          	cmp    $0x2a,%r15b
  6ad536:	b9 00 00 00 00       	mov    $0x0,%ecx
  6ad53b:	19 c9                	sbb    %ecx,%ecx
  6ad53d:	88 8c 24 06 01 00 00 	mov    %cl,0x106(%rsp)
  6ad544:	48 63 4a 0c          	movslq 0xc(%rdx),%rcx
  6ad548:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6ad54c:	48 03 4c 24 28       	add    0x28(%rsp),%rcx
  6ad551:	48 89 8c 24 f0 00 00 	mov    %rcx,0xf0(%rsp)
  6ad558:	00 
  6ad559:	4c 8b 78 28          	mov    0x28(%rax),%r15
  6ad55d:	4c 89 ff             	mov    %r15,%rdi
  6ad560:	e8 6b 6f fe ff       	call   6944d0 <getCellInfo>
  6ad565:	41 8b 47 40          	mov    0x40(%r15),%eax
  6ad569:	85 c0                	test   %eax,%eax
  6ad56b:	0f 8e 90 84 00 00    	jle    6b5a01 <sqlite3VdbeExec+0x89a1>
  6ad571:	66 c7 44 24 74 00 00 	movw   $0x0,0x74(%rsp)
  6ad578:	4c 89 64 24 78       	mov    %r12,0x78(%rsp)
  6ad57d:	c7 84 24 80 00 00 00 	movl   $0x0,0x80(%rsp)
  6ad584:	00 00 00 00 
  6ad588:	41 0f b7 4f 44       	movzwl 0x44(%r15),%ecx
  6ad58d:	49 8b 77 38          	mov    0x38(%r15),%rsi
  6ad591:	49 8b 97 88 00 00 00 	mov    0x88(%r15),%rdx
  6ad598:	8b 52 58             	mov    0x58(%rdx),%edx
  6ad59b:	29 f2                	sub    %esi,%edx
  6ad59d:	85 d2                	test   %edx,%edx
  6ad59f:	0f 4f ea             	cmovg  %edx,%ebp
  6ad5a2:	39 d1                	cmp    %edx,%ecx
  6ad5a4:	0f 4e e9             	cmovle %ecx,%ebp
  6ad5a7:	48 89 74 24 68       	mov    %rsi,0x68(%rsp)
  6ad5ac:	39 e8                	cmp    %ebp,%eax
  6ad5ae:	0f 86 7f 05 00 00    	jbe    6adb33 <sqlite3VdbeExec+0xad3>
  6ad5b4:	48 8d 4c 24 60       	lea    0x60(%rsp),%rcx
  6ad5b9:	4c 89 ff             	mov    %r15,%rdi
  6ad5bc:	31 f6                	xor    %esi,%esi
  6ad5be:	89 c2                	mov    %eax,%edx
  6ad5c0:	e8 7b 43 ff ff       	call   6a1940 <sqlite3VdbeMemFromBtree>
  6ad5c5:	85 c0                	test   %eax,%eax
  6ad5c7:	0f 85 5f 84 00 00    	jne    6b5a2c <sqlite3VdbeExec+0x89cc>
  6ad5cd:	8b 44 24 70          	mov    0x70(%rsp),%eax
  6ad5d1:	48 8b 74 24 68       	mov    0x68(%rsp),%rsi
  6ad5d6:	e9 63 05 00 00       	jmp    6adb3e <sqlite3VdbeExec+0xade>
  6ad5db:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ad5df:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad5e4:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ad5e8:	48 8b 2c c8          	mov    (%rax,%rcx,8),%rbp
  6ad5ec:	66 c7 45 02 00 00    	movw   $0x0,0x2(%rbp)
  6ad5f2:	c7 45 18 00 00 00 00 	movl   $0x0,0x18(%rbp)
  6ad5f9:	80 7d 04 00          	cmpb   $0x0,0x4(%rbp)
  6ad5fd:	0f 84 c2 06 00 00    	je     6adcc5 <sqlite3VdbeExec+0xc65>
  6ad603:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ad607:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad60b:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ad610:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6ad614:	0f b7 54 01 14       	movzwl 0x14(%rcx,%rax,1),%edx
  6ad619:	89 d1                	mov    %edx,%ecx
  6ad61b:	83 e1 2e             	and    $0x2e,%ecx
  6ad61e:	89 d0                	mov    %edx,%eax
  6ad620:	66 83 f9 02          	cmp    $0x2,%cx
  6ad624:	0f 85 51 79 00 00    	jne    6b4f7b <sqlite3VdbeExec+0x7f1b>
  6ad62a:	89 54 24 38          	mov    %edx,0x38(%rsp)
  6ad62e:	4d 8b 6c 24 08       	mov    0x8(%r12),%r13
  6ad633:	41 8b 54 24 10       	mov    0x10(%r12),%edx
  6ad638:	41 0f b6 4c 24 16    	movzbl 0x16(%r12),%ecx
  6ad63e:	48 8d b4 24 e8 00 00 	lea    0xe8(%rsp),%rsi
  6ad645:	00 
  6ad646:	4c 89 ef             	mov    %r13,%rdi
  6ad649:	89 54 24 40          	mov    %edx,0x40(%rsp)
  6ad64d:	89 4c 24 50          	mov    %ecx,0x50(%rsp)
  6ad651:	e8 7a 3a fd ff       	call   6810d0 <sqlite3AtoF>
  6ad656:	85 c0                	test   %eax,%eax
  6ad658:	0f 8e 13 79 00 00    	jle    6b4f71 <sqlite3VdbeExec+0x7f11>
  6ad65e:	f2 0f 10 8c 24 e8 00 	movsd  0xe8(%rsp),%xmm1
  6ad665:	00 00 
  6ad667:	83 f8 01             	cmp    $0x1,%eax
  6ad66a:	0f 85 e4 78 00 00    	jne    6b4f54 <sqlite3VdbeExec+0x7ef4>
  6ad670:	48 b8 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rax
  6ad677:	ff ff 7f 
  6ad67a:	f2 0f 10 05 ae de f9 	movsd  -0x62152(%rip),%xmm0        # 64b530 <.LCPI1090_0>
  6ad681:	ff 
  6ad682:	66 0f 2e c1          	ucomisd %xmm1,%xmm0
  6ad686:	0f 86 8d 68 00 00    	jbe    6b3f19 <sqlite3VdbeExec+0x6eb9>
  6ad68c:	48 ff c0             	inc    %rax
  6ad68f:	e9 94 68 00 00       	jmp    6b3f28 <sqlite3VdbeExec+0x6ec8>
  6ad694:	49 8b 00             	mov    (%r8),%rax
  6ad697:	48 39 07             	cmp    %rax,(%rdi)
  6ad69a:	0f 8e 76 05 00 00    	jle    6adc16 <sqlite3VdbeExec+0xbb6>
  6ad6a0:	48 8b 05 79 f3 0d 00 	mov    0xdf379(%rip),%rax        # 78ca20 <sqlite3aGTb>
  6ad6a7:	42 80 3c 38 00       	cmpb   $0x0,(%rax,%r15,1)
  6ad6ac:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ad6b1:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad6b5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad6ba:	0f 85 ac 81 00 00    	jne    6b586c <sqlite3VdbeExec+0x880c>
  6ad6c0:	c7 84 24 b0 00 00 00 	movl   $0x1,0xb0(%rsp)
  6ad6c7:	01 00 00 00 
  6ad6cb:	49 83 c5 18          	add    $0x18,%r13
  6ad6cf:	e9 4c fc ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ad6d4:	41 ff 86 dc 00 00 00 	incl   0xdc(%r14)
  6ad6db:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ad6df:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad6e4:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ad6e8:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ad6ec:	c7 44 24 60 01 00 00 	movl   $0x1,0x60(%rsp)
  6ad6f3:	00 
  6ad6f4:	41 80 3c 24 01       	cmpb   $0x1,(%r12)
  6ad6f9:	0f 85 54 05 00 00    	jne    6adc53 <sqlite3VdbeExec+0xbf3>
  6ad6ff:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6ad704:	4c 89 e7             	mov    %r12,%rdi
  6ad707:	e8 f4 99 00 00       	call   6b7100 <sqlite3VdbeSorterRewind>
  6ad70c:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad710:	e9 ad 63 00 00       	jmp    6b3ac2 <sqlite3VdbeExec+0x6a62>
  6ad715:	41 0f b6 44 24 6e    	movzbl 0x6e(%r12),%eax
  6ad71b:	a8 41                	test   $0x41,%al
  6ad71d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad722:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad726:	0f 84 39 72 00 00    	je     6b4965 <sqlite3VdbeExec+0x7905>
  6ad72c:	41 80 be c5 00 00 00 	cmpb   $0xfe,0xc5(%r14)
  6ad733:	fe 
  6ad734:	0f 84 2b 72 00 00    	je     6b4965 <sqlite3VdbeExec+0x7905>
  6ad73a:	49 8b 4d 10          	mov    0x10(%r13),%rcx
  6ad73e:	48 85 c9             	test   %rcx,%rcx
  6ad741:	75 10                	jne    6ad753 <sqlite3VdbeExec+0x6f3>
  6ad743:	49 8b 8e f8 00 00 00 	mov    0xf8(%r14),%rcx
  6ad74a:	48 85 c9             	test   %rcx,%rcx
  6ad74d:	0f 84 12 72 00 00    	je     6b4965 <sqlite3VdbeExec+0x7905>
  6ad753:	a8 40                	test   $0x40,%al
  6ad755:	0f 85 17 63 00 00    	jne    6b3a72 <sqlite3VdbeExec+0x6a12>
  6ad75b:	41 83 bc 24 dc 00 00 	cmpl   $0x2,0xdc(%r12)
  6ad762:	00 02 
  6ad764:	0f 8c e3 71 00 00    	jl     6b494d <sqlite3VdbeExec+0x78ed>
  6ad76a:	48 8d 35 48 88 fa ff 	lea    -0x577b8(%rip),%rsi        # 655fb9 <.L.str.303>
  6ad771:	4c 89 e7             	mov    %r12,%rdi
  6ad774:	48 89 ca             	mov    %rcx,%rdx
  6ad777:	31 c0                	xor    %eax,%eax
  6ad779:	e8 a2 14 fd ff       	call   67ec20 <sqlite3MPrintf>
  6ad77e:	49 89 c4             	mov    %rax,%r12
  6ad781:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6ad786:	48 8b b0 f8 00 00 00 	mov    0xf8(%rax),%rsi
  6ad78d:	bf 01 00 00 00       	mov    $0x1,%edi
  6ad792:	4c 89 f2             	mov    %r14,%rdx
  6ad795:	4c 89 e1             	mov    %r12,%rcx
  6ad798:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6ad79d:	ff 90 f0 00 00 00    	call   *0xf0(%rax)
  6ad7a3:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6ad7a8:	4c 89 e6             	mov    %r12,%rsi
  6ad7ab:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ad7b0:	e8 0b ce fc ff       	call   67a5c0 <sqlite3DbFree>
  6ad7b5:	e9 ab 71 00 00       	jmp    6b4965 <sqlite3VdbeExec+0x7905>
  6ad7ba:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6ad7bf:	4c 8b 60 10          	mov    0x10(%rax),%r12
  6ad7c3:	48 63 40 0c          	movslq 0xc(%rax),%rax
  6ad7c7:	4c 6b e8 38          	imul   $0x38,%rax,%r13
  6ad7cb:	4c 03 6c 24 28       	add    0x28(%rsp),%r13
  6ad7d0:	4d 39 2c 24          	cmp    %r13,(%r12)
  6ad7d4:	0f 84 b3 39 00 00    	je     6b118d <sqlite3VdbeExec+0x412d>
  6ad7da:	4d 89 74 24 18       	mov    %r14,0x18(%r12)
  6ad7df:	4d 89 2c 24          	mov    %r13,(%r12)
  6ad7e3:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6ad7e8:	41 88 44 24 28       	mov    %al,0x28(%r12)
  6ad7ed:	41 0f b7 44 24 2a    	movzwl 0x2a(%r12),%eax
  6ad7f3:	48 85 c0             	test   %rax,%rax
  6ad7f6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad7fa:	0f 84 79 48 00 00    	je     6b2079 <sqlite3VdbeExec+0x5019>
  6ad800:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ad805:	48 63 49 08          	movslq 0x8(%rcx),%rcx
  6ad809:	48 6b d1 38          	imul   $0x38,%rcx,%rdx
  6ad80d:	66 83 f8 04          	cmp    $0x4,%ax
  6ad811:	0f 83 36 4a 00 00    	jae    6b224d <sqlite3VdbeExec+0x51ed>
  6ad817:	48 89 c1             	mov    %rax,%rcx
  6ad81a:	e9 c5 4a 00 00       	jmp    6b22e4 <sqlite3VdbeExec+0x5284>
  6ad81f:	49 8b 56 68          	mov    0x68(%r14),%rdx
  6ad823:	4c 8b 7c 24 08       	mov    0x8(%rsp),%r15
  6ad828:	41 8b 47 08          	mov    0x8(%r15),%eax
  6ad82c:	48 63 c8             	movslq %eax,%rcx
  6ad82f:	4c 6b e9 38          	imul   $0x38,%rcx,%r13
  6ad833:	4e 8d 24 2a          	lea    (%rdx,%r13,1),%r12
  6ad837:	42 f6 44 2a 15 90    	testb  $0x90,0x15(%rdx,%r13,1)
  6ad83d:	0f 84 59 39 00 00    	je     6b119c <sqlite3VdbeExec+0x413c>
  6ad843:	4c 89 e7             	mov    %r12,%rdi
  6ad846:	49 89 d6             	mov    %rdx,%r14
  6ad849:	e8 12 a2 09 00       	call   747a60 <out2PrereleaseWithClear>
  6ad84e:	4c 89 f2             	mov    %r14,%rdx
  6ad851:	41 8b 47 08          	mov    0x8(%r15),%eax
  6ad855:	e9 4a 39 00 00       	jmp    6b11a4 <sqlite3VdbeExec+0x4144>
  6ad85a:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad85f:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ad863:	4c 6b e0 38          	imul   $0x38,%rax,%r12
  6ad867:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6ad86c:	49 01 d4             	add    %rdx,%r12
  6ad86f:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ad873:	48 85 c0             	test   %rax,%rax
  6ad876:	0f 84 ac 39 00 00    	je     6b1228 <sqlite3VdbeExec+0x41c8>
  6ad87c:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad880:	4c 8d 3c 02          	lea    (%rdx,%rax,1),%r15
  6ad884:	4d 8b 75 10          	mov    0x10(%r13),%r14
  6ad888:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6ad88c:	48 8d 4c 24 68       	lea    0x68(%rsp),%rcx
  6ad891:	66 0f 11 41 20       	movupd %xmm0,0x20(%rcx)
  6ad896:	66 0f 11 41 10       	movupd %xmm0,0x10(%rcx)
  6ad89b:	f6 44 02 15 90       	testb  $0x90,0x15(%rdx,%rax,1)
  6ad8a0:	0f 84 da 47 00 00    	je     6b2080 <sqlite3VdbeExec+0x5020>
  6ad8a6:	4c 89 ff             	mov    %r15,%rdi
  6ad8a9:	e8 92 3c ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6ad8ae:	e9 d4 47 00 00       	jmp    6b2087 <sqlite3VdbeExec+0x5027>
  6ad8b3:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad8b8:	45 0f b7 75 02       	movzwl 0x2(%r13),%r14d
  6ad8bd:	46 8d 3c f5 00 00 00 	lea    0x0(,%r14,8),%r15d
  6ad8c4:	00 
  6ad8c5:	49 8d 77 68          	lea    0x68(%r15),%rsi
  6ad8c9:	4c 89 e7             	mov    %r12,%rdi
  6ad8cc:	e8 7f cf fc ff       	call   67a850 <sqlite3DbMallocRawNN>
  6ad8d1:	4c 89 e1             	mov    %r12,%rcx
  6ad8d4:	48 85 c0             	test   %rax,%rax
  6ad8d7:	0f 84 e9 81 00 00    	je     6b5ac6 <sqlite3VdbeExec+0x8a66>
  6ad8dd:	49 89 c4             	mov    %rax,%r12
  6ad8e0:	4c 01 f8             	add    %r15,%rax
  6ad8e3:	48 83 c0 30          	add    $0x30,%rax
  6ad8e7:	49 89 04 24          	mov    %rax,(%r12)
  6ad8eb:	66 43 c7 44 3c 44 01 	movw   $0x1,0x44(%r12,%r15,1)
  6ad8f2:	00 
  6ad8f3:	4b 89 4c 3c 48       	mov    %rcx,0x48(%r12,%r15,1)
  6ad8f8:	43 c7 44 3c 50 00 00 	movl   $0x0,0x50(%r12,%r15,1)
  6ad8ff:	00 00 
  6ad901:	49 c7 44 24 10 00 00 	movq   $0x0,0x10(%r12)
  6ad908:	00 00 
  6ad90a:	49 8b 45 10          	mov    0x10(%r13),%rax
  6ad90e:	49 89 44 24 08       	mov    %rax,0x8(%r12)
  6ad913:	4c 89 e8             	mov    %r13,%rax
  6ad916:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6ad91b:	48 c1 e8 03          	shr    $0x3,%rax
  6ad91f:	69 c0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%eax
  6ad925:	41 89 44 24 20       	mov    %eax,0x20(%r12)
  6ad92a:	48 8b 44 24 30       	mov    0x30(%rsp),%rax
  6ad92f:	49 89 44 24 18       	mov    %rax,0x18(%r12)
  6ad934:	41 c6 44 24 29 00    	movb   $0x0,0x29(%r12)
  6ad93a:	41 c7 44 24 24 00 00 	movl   $0x0,0x24(%r12)
  6ad941:	00 00 
  6ad943:	0f b6 4c 24 1f       	movzbl 0x1f(%rsp),%ecx
  6ad948:	41 88 4c 24 28       	mov    %cl,0x28(%r12)
  6ad94d:	66 45 89 74 24 2a    	mov    %r14w,0x2a(%r12)
  6ad953:	49 89 c6             	mov    %rax,%r14
  6ad956:	4d 89 65 10          	mov    %r12,0x10(%r13)
  6ad95a:	66 41 c7 45 00 a3 f1 	movw   $0xf1a3,0x0(%r13)
  6ad961:	e9 9a 17 00 00       	jmp    6af100 <sqlite3VdbeExec+0x20a0>
  6ad966:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ad96a:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad96f:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ad973:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ad977:	49 8b 44 24 28       	mov    0x28(%r12),%rax
  6ad97c:	80 38 00             	cmpb   $0x0,(%rax)
  6ad97f:	74 10                	je     6ad991 <sqlite3VdbeExec+0x931>
  6ad981:	4c 89 e7             	mov    %r12,%rdi
  6ad984:	e8 57 a5 ff ff       	call   6a7ee0 <sqlite3VdbeHandleMovedCursor>
  6ad989:	85 c0                	test   %eax,%eax
  6ad98b:	0f 85 7a 82 00 00    	jne    6b5c0b <sqlite3VdbeExec+0x8bab>
  6ad991:	41 80 7c 24 02 00    	cmpb   $0x0,0x2(%r12)
  6ad997:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ad99b:	0f 84 4a 3b 00 00    	je     6b14eb <sqlite3VdbeExec+0x448b>
  6ad9a1:	49 63 45 08          	movslq 0x8(%r13),%rax
  6ad9a5:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad9a9:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ad9ae:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6ad9b2:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6ad9b7:	0f 84 85 48 00 00    	je     6b2242 <sqlite3VdbeExec+0x51e2>
  6ad9bd:	e8 7e 3b ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6ad9c2:	e9 cd 77 00 00       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6ad9c7:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ad9cc:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ad9d0:	48 85 c0             	test   %rax,%rax
  6ad9d3:	7e 1d                	jle    6ad9f2 <sqlite3VdbeExec+0x992>
  6ad9d5:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ad9d9:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6ad9de:	c7 44 02 10 00 00 00 	movl   $0x0,0x10(%rdx,%rax,1)
  6ad9e5:	00 
  6ad9e6:	48 8d 0d 82 29 fa ff 	lea    -0x5d67e(%rip),%rcx        # 65036f <.L.str.18>
  6ad9ed:	48 89 4c 02 08       	mov    %rcx,0x8(%rdx,%rax,1)
  6ad9f2:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ad9f6:	49 63 75 04          	movslq 0x4(%r13),%rsi
  6ad9fa:	4c 8b 24 f0          	mov    (%rax,%rsi,8),%r12
  6ad9fe:	4d 85 e4             	test   %r12,%r12
  6ada01:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ada05:	74 18                	je     6ada1f <sqlite3VdbeExec+0x9bf>
  6ada07:	41 f6 44 24 05 08    	testb  $0x8,0x5(%r12)
  6ada0d:	75 10                	jne    6ada1f <sqlite3VdbeExec+0x9bf>
  6ada0f:	41 0f bf 44 24 40    	movswl 0x40(%r12),%eax
  6ada15:	41 39 45 08          	cmp    %eax,0x8(%r13)
  6ada19:	0f 8e 99 5c 00 00    	jle    6b36b8 <sqlite3VdbeExec+0x6658>
  6ada1f:	41 8b 55 08          	mov    0x8(%r13),%edx
  6ada23:	4c 89 f7             	mov    %r14,%rdi
  6ada26:	31 c9                	xor    %ecx,%ecx
  6ada28:	e8 63 91 00 00       	call   6b6b90 <allocateCursor>
  6ada2d:	48 85 c0             	test   %rax,%rax
  6ada30:	0f 84 38 81 00 00    	je     6b5b6e <sqlite3VdbeExec+0x8b0e>
  6ada36:	49 89 c4             	mov    %rax,%r12
  6ada39:	80 48 05 01          	orb    $0x1,0x5(%rax)
  6ada3d:	48 8b 54 24 10       	mov    0x10(%rsp),%rdx
  6ada42:	48 8b 3a             	mov    (%rdx),%rdi
  6ada45:	4c 89 e8             	mov    %r13,%rax
  6ada48:	4d 8d 6c 24 08       	lea    0x8(%r12),%r13
  6ada4d:	44 0f b7 40 02       	movzwl 0x2(%rax),%r8d
  6ada52:	41 83 c8 05          	or     $0x5,%r8d
  6ada56:	31 f6                	xor    %esi,%esi
  6ada58:	4c 89 e9             	mov    %r13,%rcx
  6ada5b:	41 b9 1e 04 00 00    	mov    $0x41e,%r9d
  6ada61:	e8 0a 3e fe ff       	call   691870 <sqlite3BtreeOpen>
  6ada66:	85 c0                	test   %eax,%eax
  6ada68:	0f 85 cb 7f 00 00    	jne    6b5a39 <sqlite3VdbeExec+0x89d9>
  6ada6e:	49 8b 7d 00          	mov    0x0(%r13),%rdi
  6ada72:	80 7f 11 00          	cmpb   $0x0,0x11(%rdi)
  6ada76:	75 0a                	jne    6ada82 <sqlite3VdbeExec+0xa22>
  6ada78:	80 7f 10 01          	cmpb   $0x1,0x10(%rdi)
  6ada7c:	0f 87 ac 5d 00 00    	ja     6b382e <sqlite3VdbeExec+0x67ce>
  6ada82:	be 01 00 00 00       	mov    $0x1,%esi
  6ada87:	31 d2                	xor    %edx,%edx
  6ada89:	e8 e2 50 fe ff       	call   692b70 <btreeBeginTrans>
  6ada8e:	85 c0                	test   %eax,%eax
  6ada90:	48 89 c2             	mov    %rax,%rdx
  6ada93:	0f 85 3b 5f 00 00    	jne    6b39d4 <sqlite3VdbeExec+0x6974>
  6ada99:	e9 b8 5d 00 00       	jmp    6b3856 <sqlite3VdbeExec+0x67f6>
  6ada9e:	49 8b 46 78          	mov    0x78(%r14),%rax
  6adaa2:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adaa7:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6adaab:	4c 8b 34 c8          	mov    (%rax,%rcx,8),%r14
  6adaaf:	49 8b 7e 28          	mov    0x28(%r14),%rdi
  6adab3:	c7 44 24 60 00 00 00 	movl   $0x0,0x60(%rsp)
  6adaba:	00 
  6adabb:	41 80 ff 89          	cmp    $0x89,%r15b
  6adabf:	0f 85 15 03 00 00    	jne    6addda <sqlite3VdbeExec+0xd7a>
  6adac5:	41 c7 46 1c ff ff ff 	movl   $0xffffffff,0x1c(%r14)
  6adacc:	ff 
  6adacd:	80 3f 00             	cmpb   $0x0,(%rdi)
  6adad0:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adad4:	0f 85 13 03 00 00    	jne    6added <sqlite3VdbeExec+0xd8d>
  6adada:	e9 fc 65 00 00       	jmp    6b40db <sqlite3VdbeExec+0x707b>
  6adadf:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adae4:	49 63 45 04          	movslq 0x4(%r13),%rax
  6adae8:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adaec:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6adaf1:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6adaf5:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6adafa:	a8 24                	test   $0x24,%al
  6adafc:	0f 84 f7 37 00 00    	je     6b12f9 <sqlite3VdbeExec+0x4299>
  6adb02:	45 31 ff             	xor    %r15d,%r15d
  6adb05:	48 83 3f 00          	cmpq   $0x0,(%rdi)
  6adb09:	41 0f 95 c7          	setne  %r15b
  6adb0d:	e9 fc 77 00 00       	jmp    6b530e <sqlite3VdbeExec+0x82ae>
  6adb12:	f6 47 15 90          	testb  $0x90,0x15(%rdi)
  6adb16:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adb1a:	0f 84 71 38 00 00    	je     6b1391 <sqlite3VdbeExec+0x4331>
  6adb20:	e8 1b 3a ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6adb25:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adb2a:	49 83 c5 18          	add    $0x18,%r13
  6adb2e:	e9 ed f7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6adb33:	66 c7 44 24 74 10 40 	movw   $0x4010,0x74(%rsp)
  6adb3a:	89 44 24 70          	mov    %eax,0x70(%rsp)
  6adb3e:	48 8d 94 24 e8 00 00 	lea    0xe8(%rsp),%rdx
  6adb45:	00 
  6adb46:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6adb4d:	00 00 
  6adb4f:	89 c7                	mov    %eax,%edi
  6adb51:	31 c9                	xor    %ecx,%ecx
  6adb53:	e8 58 ac ff ff       	call   6a87b0 <sqlite3VdbeRecordCompareWithSkip>
  6adb58:	89 c5                	mov    %eax,%ebp
  6adb5a:	83 bc 24 80 00 00 00 	cmpl   $0x0,0x80(%rsp)
  6adb61:	00 
  6adb62:	74 0a                	je     6adb6e <sqlite3VdbeExec+0xb0e>
  6adb64:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6adb69:	e8 82 32 ff ff       	call   6a0df0 <vdbeMemClear>
  6adb6e:	89 e8                	mov    %ebp,%eax
  6adb70:	f7 d8                	neg    %eax
  6adb72:	ff c5                	inc    %ebp
  6adb74:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6adb79:	f6 01 01             	testb  $0x1,(%rcx)
  6adb7c:	0f 44 e8             	cmove  %eax,%ebp
  6adb7f:	85 ed                	test   %ebp,%ebp
  6adb81:	4d 89 ef             	mov    %r13,%r15
  6adb84:	49 89 cd             	mov    %rcx,%r13
  6adb87:	0f 8f df 7c 00 00    	jg     6b586c <sqlite3VdbeExec+0x880c>
  6adb8d:	e9 85 f7 ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6adb92:	09 e9                	or     %ebp,%ecx
  6adb94:	f6 c1 01             	test   $0x1,%cl
  6adb97:	0f 85 80 59 00 00    	jne    6b351d <sqlite3VdbeExec+0x64bd>
  6adb9d:	66 83 e5 2d          	and    $0x2d,%bp
  6adba1:	75 12                	jne    6adbb5 <sqlite3VdbeExec+0xb55>
  6adba3:	49 89 ff             	mov    %rdi,%r15
  6adba6:	e8 d5 9e 09 00       	call   747a80 <computeNumericType>
  6adbab:	4c 89 ff             	mov    %r15,%rdi
  6adbae:	89 c5                	mov    %eax,%ebp
  6adbb0:	41 0f b7 45 14       	movzwl 0x14(%r13),%eax
  6adbb5:	66 83 e0 2d          	and    $0x2d,%ax
  6adbb9:	75 0e                	jne    6adbc9 <sqlite3VdbeExec+0xb69>
  6adbbb:	49 89 ff             	mov    %rdi,%r15
  6adbbe:	4c 89 ef             	mov    %r13,%rdi
  6adbc1:	e8 ba 9e 09 00       	call   747a80 <computeNumericType>
  6adbc6:	4c 89 ff             	mov    %r15,%rdi
  6adbc9:	21 e8                	and    %ebp,%eax
  6adbcb:	a8 04                	test   $0x4,%al
  6adbcd:	0f 84 78 38 00 00    	je     6b144b <sqlite3VdbeExec+0x43eb>
  6adbd3:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6adbd8:	44 0f b6 38          	movzbl (%rax),%r15d
  6adbdc:	e9 3e f8 ff ff       	jmp    6ad41f <sqlite3VdbeExec+0x3bf>
  6adbe1:	40 84 f6             	test   %sil,%sil
  6adbe4:	0f 88 c0 36 00 00    	js     6b12aa <sqlite3VdbeExec+0x424a>
  6adbea:	40 f6 c6 10          	test   $0x10,%sil
  6adbee:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6adbf3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adbf7:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adbfc:	0f 85 6a 7c 00 00    	jne    6b586c <sqlite3VdbeExec+0x880c>
  6adc02:	c7 84 24 b0 00 00 00 	movl   $0x1,0xb0(%rsp)
  6adc09:	01 00 00 00 
  6adc0d:	49 83 c5 18          	add    $0x18,%r13
  6adc11:	e9 0a f7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6adc16:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6adc1b:	48 8b 0c 24          	mov    (%rsp),%rcx
  6adc1f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adc24:	0f 8d b5 36 00 00    	jge    6b12df <sqlite3VdbeExec+0x427f>
  6adc2a:	48 8b 05 df ed 0d 00 	mov    0xdeddf(%rip),%rax        # 78ca10 <sqlite3aLTb>
  6adc31:	42 80 3c 38 00       	cmpb   $0x0,(%rax,%r15,1)
  6adc36:	0f 85 b5 36 00 00    	jne    6b12f1 <sqlite3VdbeExec+0x4291>
  6adc3c:	c7 84 24 b0 00 00 00 	movl   $0xffffffff,0xb0(%rsp)
  6adc43:	ff ff ff ff 
  6adc47:	49 89 cf             	mov    %rcx,%r15
  6adc4a:	49 83 c5 18          	add    $0x18,%r13
  6adc4e:	e9 cd f6 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6adc53:	4d 8b 6c 24 28       	mov    0x28(%r12),%r13
  6adc58:	4c 89 ef             	mov    %r13,%rdi
  6adc5b:	e8 70 70 fe ff       	call   694cd0 <moveToRoot>
  6adc60:	83 f8 10             	cmp    $0x10,%eax
  6adc63:	0f 84 24 3e 00 00    	je     6b1a8d <sqlite3VdbeExec+0x4a2d>
  6adc69:	85 c0                	test   %eax,%eax
  6adc6b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adc6f:	0f 85 39 5e 00 00    	jne    6b3aae <sqlite3VdbeExec+0x6a4e>
  6adc75:	c7 44 24 60 00 00 00 	movl   $0x0,0x60(%rsp)
  6adc7c:	00 
  6adc7d:	49 8b 85 88 00 00 00 	mov    0x88(%r13),%rax
  6adc84:	80 78 08 00          	cmpb   $0x0,0x8(%rax)
  6adc88:	0f 85 1e 5e 00 00    	jne    6b3aac <sqlite3VdbeExec+0x6a4c>
  6adc8e:	0f b7 48 1a          	movzwl 0x1a(%rax),%ecx
  6adc92:	48 8b 50 50          	mov    0x50(%rax),%rdx
  6adc96:	48 8b 40 60          	mov    0x60(%rax),%rax
  6adc9a:	41 0f b7 75 56       	movzwl 0x56(%r13),%esi
  6adc9f:	0f b6 3c 70          	movzbl (%rax,%rsi,2),%edi
  6adca3:	c1 e7 08             	shl    $0x8,%edi
  6adca6:	0f b6 44 70 01       	movzbl 0x1(%rax,%rsi,2),%eax
  6adcab:	09 f8                	or     %edi,%eax
  6adcad:	21 c8                	and    %ecx,%eax
  6adcaf:	8b 34 02             	mov    (%rdx,%rax,1),%esi
  6adcb2:	0f ce                	bswap  %esi
  6adcb4:	4c 89 ef             	mov    %r13,%rdi
  6adcb7:	e8 54 76 fe ff       	call   695310 <moveToChild>
  6adcbc:	85 c0                	test   %eax,%eax
  6adcbe:	74 bd                	je     6adc7d <sqlite3VdbeExec+0xc1d>
  6adcc0:	e9 e9 5d 00 00       	jmp    6b3aae <sqlite3VdbeExec+0x6a4e>
  6adcc5:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6adcc9:	48 8b 45 30          	mov    0x30(%rbp),%rax
  6adccd:	44 0f b6 77 03       	movzbl 0x3(%rdi),%r14d
  6adcd2:	41 0f b7 4d 10       	movzwl 0x10(%r13),%ecx
  6adcd7:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6adcdc:	44 89 f8             	mov    %r15d,%eax
  6adcdf:	83 e0 01             	and    $0x1,%eax
  6adce2:	31 d2                	xor    %edx,%edx
  6adce4:	83 f8 01             	cmp    $0x1,%eax
  6adce7:	19 d2                	sbb    %edx,%edx
  6adce9:	66 89 4c 24 7c       	mov    %cx,0x7c(%rsp)
  6adcee:	80 ca 01             	or     $0x1,%dl
  6adcf1:	88 54 24 7e          	mov    %dl,0x7e(%rsp)
  6adcf5:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6adcf9:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adcfd:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6add02:	48 89 44 24 68       	mov    %rax,0x68(%rsp)
  6add07:	c6 84 24 82 00 00 00 	movb   $0x0,0x82(%rsp)
  6add0e:	00 
  6add0f:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6add14:	48 8d 94 24 c4 00 00 	lea    0xc4(%rsp),%rdx
  6add1b:	00 
  6add1c:	e8 3f 77 fe ff       	call   695460 <sqlite3BtreeIndexMoveto>
  6add21:	85 c0                	test   %eax,%eax
  6add23:	0f 85 c3 7c 00 00    	jne    6b59ec <sqlite3VdbeExec+0x898c>
  6add29:	41 83 e6 02          	and    $0x2,%r14d
  6add2d:	45 89 f4             	mov    %r14d,%r12d
  6add30:	41 d1 ec             	shr    $1,%r12d
  6add33:	45 85 f6             	test   %r14d,%r14d
  6add36:	0f 84 9a 56 00 00    	je     6b33d6 <sqlite3VdbeExec+0x6376>
  6add3c:	b9 01 00 00 00       	mov    $0x1,%ecx
  6add41:	80 bc 24 82 00 00 00 	cmpb   $0x0,0x82(%rsp)
  6add48:	00 
  6add49:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6add4e:	0f 84 5c 73 00 00    	je     6b50b0 <sqlite3VdbeExec+0x8050>
  6add54:	e9 69 72 00 00       	jmp    6b4fc2 <sqlite3VdbeExec+0x7f62>
  6add59:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6add5e:	48 63 42 04          	movslq 0x4(%rdx),%rax
  6add62:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6add66:	48 8b 7c 24 28       	mov    0x28(%rsp),%rdi
  6add6b:	66 c7 44 07 14 04 00 	movw   $0x4,0x14(%rdi,%rax,1)
  6add72:	48 63 0c 07          	movslq (%rdi,%rax,1),%rcx
  6add76:	48 8b 74 24 58       	mov    0x58(%rsp),%rsi
  6add7b:	48 29 f2             	sub    %rsi,%rdx
  6add7e:	48 c1 ea 03          	shr    $0x3,%rdx
  6add82:	69 d2 ab aa aa aa    	imul   $0xaaaaaaab,%edx,%edx
  6add88:	48 63 d2             	movslq %edx,%rdx
  6add8b:	48 89 14 07          	mov    %rdx,(%rdi,%rax,1)
  6add8f:	48 8d 04 49          	lea    (%rcx,%rcx,2),%rax
  6add93:	4c 8d 2c c6          	lea    (%rsi,%rax,8),%r13
  6add97:	4c 8b 3c 24          	mov    (%rsp),%r15
  6add9b:	49 83 c5 18          	add    $0x18,%r13
  6add9f:	e9 7c f5 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6adda4:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adda9:	49 63 45 04          	movslq 0x4(%r13),%rax
  6addad:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6addb1:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6addb6:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6addba:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6addbf:	a8 24                	test   $0x24,%al
  6addc1:	0f 84 ee 45 00 00    	je     6b23b5 <sqlite3VdbeExec+0x5355>
  6addc7:	48 83 3f 00          	cmpq   $0x0,(%rdi)
  6addcb:	e9 54 3d 00 00       	jmp    6b1b24 <sqlite3VdbeExec+0x4ac4>
  6addd0:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6addd5:	e9 94 30 00 00       	jmp    6b0e6e <sqlite3VdbeExec+0x3e0e>
  6addda:	80 3f 00             	cmpb   $0x0,(%rdi)
  6adddd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adde1:	75 0a                	jne    6added <sqlite3VdbeExec+0xd8d>
  6adde3:	f6 47 01 08          	testb  $0x8,0x1(%rdi)
  6adde7:	0f 85 d6 62 00 00    	jne    6b40c3 <sqlite3VdbeExec+0x7063>
  6added:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6addf2:	e8 f9 70 fe ff       	call   694ef0 <btreeLast>
  6addf7:	8b 54 24 60          	mov    0x60(%rsp),%edx
  6addfb:	41 88 56 02          	mov    %dl,0x2(%r14)
  6addff:	41 c6 46 03 00       	movb   $0x0,0x3(%r14)
  6ade04:	41 c7 46 18 00 00 00 	movl   $0x0,0x18(%r14)
  6ade0b:	00 
  6ade0c:	85 c0                	test   %eax,%eax
  6ade0e:	0f 85 eb 83 00 00    	jne    6b61ff <sqlite3VdbeExec+0x919f>
  6ade14:	41 83 7d 08 00       	cmpl   $0x0,0x8(%r13)
  6ade19:	0f 9f c1             	setg   %cl
  6ade1c:	85 d2                	test   %edx,%edx
  6ade1e:	0f 95 c0             	setne  %al
  6ade21:	20 c8                	and    %cl,%al
  6ade23:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ade2a:	00 00 
  6ade2c:	3c 01                	cmp    $0x1,%al
  6ade2e:	0f 85 a7 62 00 00    	jne    6b40db <sqlite3VdbeExec+0x707b>
  6ade34:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ade3b:	00 00 
  6ade3d:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6ade42:	e9 25 7a 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6ade47:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ade4b:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ade50:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ade54:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ade58:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
  6ade5d:	e8 0e 74 fe ff       	call   695270 <sqlite3BtreeNext>
  6ade62:	e9 86 0a 00 00       	jmp    6ae8ed <sqlite3VdbeExec+0x188d>
  6ade67:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ade6c:	49 63 4d 10          	movslq 0x10(%r13),%rcx
  6ade70:	48 85 c9             	test   %rcx,%rcx
  6ade73:	0f 8e dd 45 00 00    	jle    6b2456 <sqlite3VdbeExec+0x53f6>
  6ade79:	48 ba ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rdx
  6ade80:	ff ff 7f 
  6ade83:	49 63 75 0c          	movslq 0xc(%r13),%rsi
  6ade87:	48 01 f1             	add    %rsi,%rcx
  6ade8a:	48 6b fe 38          	imul   $0x38,%rsi,%rdi
  6ade8e:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6ade93:	31 c0                	xor    %eax,%eax
  6ade95:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ade99:	eb 18                	jmp    6adeb3 <sqlite3VdbeExec+0xe53>
  6ade9b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  6adea0:	48 03 07             	add    (%rdi),%rax
  6adea3:	48 ff c6             	inc    %rsi
  6adea6:	48 83 c7 38          	add    $0x38,%rdi
  6adeaa:	48 39 ce             	cmp    %rcx,%rsi
  6adead:	0f 8d a9 45 00 00    	jge    6b245c <sqlite3VdbeExec+0x53fc>
  6adeb3:	44 0f b7 47 14       	movzwl 0x14(%rdi),%r8d
  6adeb8:	41 f6 c0 24          	test   $0x24,%r8b
  6adebc:	75 e2                	jne    6adea0 <sqlite3VdbeExec+0xe40>
  6adebe:	41 f6 c0 08          	test   $0x8,%r8b
  6adec2:	75 11                	jne    6aded5 <sqlite3VdbeExec+0xe75>
  6adec4:	49 83 e0 12          	and    $0x12,%r8
  6adec8:	74 d9                	je     6adea3 <sqlite3VdbeExec+0xe43>
  6adeca:	4c 01 c0             	add    %r8,%rax
  6adecd:	48 05 fd 0f 00 00    	add    $0xffd,%rax
  6aded3:	eb ce                	jmp    6adea3 <sqlite3VdbeExec+0xe43>
  6aded5:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6aded9:	f2 0f 10 0d 4f d6 f9 	movsd  -0x629b1(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6adee0:	ff 
  6adee1:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6adee5:	76 09                	jbe    6adef0 <sqlite3VdbeExec+0xe90>
  6adee7:	4c 8d 42 01          	lea    0x1(%rdx),%r8
  6adeeb:	4c 01 c0             	add    %r8,%rax
  6adeee:	eb b3                	jmp    6adea3 <sqlite3VdbeExec+0xe43>
  6adef0:	66 0f 2e 05 50 d7 f9 	ucomisd -0x628b0(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6adef7:	ff 
  6adef8:	49 89 d0             	mov    %rdx,%r8
  6adefb:	77 05                	ja     6adf02 <sqlite3VdbeExec+0xea2>
  6adefd:	f2 4c 0f 2c c0       	cvttsd2si %xmm0,%r8
  6adf02:	4c 01 c0             	add    %r8,%rax
  6adf05:	eb 9c                	jmp    6adea3 <sqlite3VdbeExec+0xe43>
  6adf07:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adf0c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6adf10:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adf14:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6adf19:	f6 44 01 14 04       	testb  $0x4,0x14(%rcx,%rax,1)
  6adf1e:	0f 85 7c 3b 00 00    	jne    6b1aa0 <sqlite3VdbeExec+0x4a40>
  6adf24:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adf28:	49 83 c5 18          	add    $0x18,%r13
  6adf2c:	e9 ef f3 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6adf31:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adf36:	41 83 7d 04 00       	cmpl   $0x0,0x4(%r13)
  6adf3b:	0f 84 7a 45 00 00    	je     6b24bb <sqlite3VdbeExec+0x545b>
  6adf41:	49 83 bc 24 f0 02 00 	cmpq   $0x0,0x2f0(%r12)
  6adf48:	00 00 
  6adf4a:	e9 71 45 00 00       	jmp    6b24c0 <sqlite3VdbeExec+0x5460>
  6adf4f:	49 8b 46 78          	mov    0x78(%r14),%rax
  6adf53:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adf58:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6adf5c:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6adf60:	48 85 c0             	test   %rax,%rax
  6adf63:	0f 84 75 45 00 00    	je     6b24de <sqlite3VdbeExec+0x547e>
  6adf69:	80 78 02 00          	cmpb   $0x0,0x2(%rax)
  6adf6d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6adf71:	0f 84 a0 f3 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6adf77:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6adf7b:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adf7f:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6adf84:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6adf88:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6adf8d:	0f 84 d5 67 00 00    	je     6b4768 <sqlite3VdbeExec+0x7708>
  6adf93:	e8 a8 35 ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6adf98:	e9 cf 78 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6adf9d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adfa2:	49 63 45 04          	movslq 0x4(%r13),%rax
  6adfa6:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adfaa:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6adfaf:	f6 44 01 14 01       	testb  $0x1,0x14(%rcx,%rax,1)
  6adfb4:	e9 6b 3b 00 00       	jmp    6b1b24 <sqlite3VdbeExec+0x4ac4>
  6adfb9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adfbe:	49 63 45 04          	movslq 0x4(%r13),%rax
  6adfc2:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adfc6:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6adfcb:	f6 44 01 14 01       	testb  $0x1,0x14(%rcx,%rax,1)
  6adfd0:	e9 df 60 00 00       	jmp    6b40b4 <sqlite3VdbeExec+0x7054>
  6adfd5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6adfda:	49 63 45 04          	movslq 0x4(%r13),%rax
  6adfde:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6adfe2:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6adfe7:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6adfeb:	0f b7 4c 01 14       	movzwl 0x14(%rcx,%rax,1),%ecx
  6adff0:	0f b7 c1             	movzwl %cx,%eax
  6adff3:	a8 24                	test   $0x24,%al
  6adff5:	0f 84 42 45 00 00    	je     6b253d <sqlite3VdbeExec+0x54dd>
  6adffb:	49 8b 04 24          	mov    (%r12),%rax
  6adfff:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae003:	49 89 04 24          	mov    %rax,(%r12)
  6ae007:	81 e1 40 f2 ff ff    	and    $0xfffff240,%ecx
  6ae00d:	83 c9 04             	or     $0x4,%ecx
  6ae010:	66 41 89 4c 24 14    	mov    %cx,0x14(%r12)
  6ae016:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6ae01a:	48 01 c1             	add    %rax,%rcx
  6ae01d:	e9 97 46 00 00       	jmp    6b26b9 <sqlite3VdbeExec+0x5659>
  6ae022:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae027:	41 8b 55 04          	mov    0x4(%r13),%edx
  6ae02b:	49 63 45 08          	movslq 0x8(%r13),%rax
  6ae02f:	48 6b c8 38          	imul   $0x38,%rax,%rcx
  6ae033:	48 03 4c 24 28       	add    0x28(%rsp),%rcx
  6ae038:	48 85 c0             	test   %rax,%rax
  6ae03b:	48 0f 44 c8          	cmove  %rax,%rcx
  6ae03f:	48 8b bc 24 d0 00 00 	mov    0xd0(%rsp),%rdi
  6ae046:	00 
  6ae047:	4c 89 e6             	mov    %r12,%rsi
  6ae04a:	e8 b1 b2 00 00       	call   6b9300 <sqlite3RunVacuum>
  6ae04f:	e9 5a 21 00 00       	jmp    6b01ae <sqlite3VdbeExec+0x314e>
  6ae054:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae059:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae05d:	48 6b c8 38          	imul   $0x38,%rax,%rcx
  6ae061:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6ae066:	48 8d 04 0a          	lea    (%rdx,%rcx,1),%rax
  6ae06a:	48 8b 0c 0a          	mov    (%rdx,%rcx,1),%rcx
  6ae06e:	48 85 c9             	test   %rcx,%rcx
  6ae071:	0f 84 f9 44 00 00    	je     6b2570 <sqlite3VdbeExec+0x5510>
  6ae077:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae07b:	0f 8e eb 77 00 00    	jle    6b586c <sqlite3VdbeExec+0x880c>
  6ae081:	48 ff c9             	dec    %rcx
  6ae084:	48 89 08             	mov    %rcx,(%rax)
  6ae087:	e9 e0 77 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6ae08c:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae090:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae095:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ae099:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ae09d:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
  6ae0a2:	80 67 01 f1          	andb   $0xf1,0x1(%rdi)
  6ae0a6:	66 c7 47 46 00 00    	movw   $0x0,0x46(%rdi)
  6ae0ac:	80 3f 00             	cmpb   $0x0,(%rdi)
  6ae0af:	0f 84 c8 44 00 00    	je     6b257d <sqlite3VdbeExec+0x551d>
  6ae0b5:	e8 b6 80 fe ff       	call   696170 <btreePrevious>
  6ae0ba:	e9 2e 08 00 00       	jmp    6ae8ed <sqlite3VdbeExec+0x188d>
  6ae0bf:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae0c4:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae0c8:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae0cc:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae0d1:	f6 44 01 14 01       	testb  $0x1,0x14(%rcx,%rax,1)
  6ae0d6:	0f 85 c9 32 00 00    	jne    6b13a5 <sqlite3VdbeExec+0x4345>
  6ae0dc:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ae0e0:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae0e4:	f6 44 01 14 01       	testb  $0x1,0x14(%rcx,%rax,1)
  6ae0e9:	0f 85 b6 32 00 00    	jne    6b13a5 <sqlite3VdbeExec+0x4345>
  6ae0ef:	49 63 45 08          	movslq 0x8(%r13),%rax
  6ae0f3:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae0f7:	eb 1b                	jmp    6ae114 <sqlite3VdbeExec+0x10b4>
  6ae0f9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae0fe:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae102:	48 85 c0             	test   %rax,%rax
  6ae105:	0f 84 a8 44 00 00    	je     6b25b3 <sqlite3VdbeExec+0x5553>
  6ae10b:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae10f:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae114:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6ae118:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6ae11d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae121:	0f 84 3c 55 00 00    	je     6b3663 <sqlite3VdbeExec+0x6603>
  6ae127:	31 f6                	xor    %esi,%esi
  6ae129:	e8 b2 34 ff ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6ae12e:	49 83 c5 18          	add    $0x18,%r13
  6ae132:	e9 e9 f1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ae137:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae13c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae140:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae144:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae149:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6ae14d:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6ae152:	a8 10                	test   $0x10,%al
  6ae154:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae158:	0f 84 fd 5f 00 00    	je     6b415b <sqlite3VdbeExec+0x70fb>
  6ae15e:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6ae163:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6ae168:	e8 23 a3 fd ff       	call   688490 <sqlite3RowSetNext>
  6ae16d:	85 c0                	test   %eax,%eax
  6ae16f:	0f 84 e0 5f 00 00    	je     6b4155 <sqlite3VdbeExec+0x70f5>
  6ae175:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ae179:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae17d:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae182:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6ae186:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  6ae18b:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6ae190:	0f 84 9e 6a 00 00    	je     6b4c34 <sqlite3VdbeExec+0x7bd4>
  6ae196:	e8 45 34 ff ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6ae19b:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae1a0:	e9 e2 2c 00 00       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6ae1a5:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae1a9:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ae1ae:	48 63 49 04          	movslq 0x4(%rcx),%rcx
  6ae1b2:	4c 8b 2c c8          	mov    (%rax,%rcx,8),%r13
  6ae1b6:	4d 85 ed             	test   %r13,%r13
  6ae1b9:	74 2a                	je     6ae1e5 <sqlite3VdbeExec+0x1185>
  6ae1bb:	41 8b 45 3c          	mov    0x3c(%r13),%eax
  6ae1bf:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ae1c4:	3b 41 08             	cmp    0x8(%rcx),%eax
  6ae1c7:	75 1c                	jne    6ae1e5 <sqlite3VdbeExec+0x1185>
  6ae1c9:	49 8b 7d 28          	mov    0x28(%r13),%rdi
  6ae1cd:	e8 0e 35 fe ff       	call   6916e0 <sqlite3BtreeClearCursor>
  6ae1d2:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6ae1d7:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6ae1dc:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae1e0:	e9 7d 2f 00 00       	jmp    6b1162 <sqlite3VdbeExec+0x4102>
  6ae1e5:	41 0f b7 86 c8 00 00 	movzwl 0xc8(%r14),%eax
  6ae1ec:	00 
  6ae1ed:	83 e0 03             	and    $0x3,%eax
  6ae1f0:	66 83 f8 01          	cmp    $0x1,%ax
  6ae1f4:	0f 84 96 78 00 00    	je     6b5a90 <sqlite3VdbeExec+0x8a30>
  6ae1fa:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6ae1ff:	8b 72 08             	mov    0x8(%rdx),%esi
  6ae202:	48 63 6a 0c          	movslq 0xc(%rdx),%rbp
  6ae206:	49 8b 4c 24 20       	mov    0x20(%r12),%rcx
  6ae20b:	48 89 e8             	mov    %rbp,%rax
  6ae20e:	48 c1 e0 05          	shl    $0x5,%rax
  6ae212:	48 8b 7c 01 08       	mov    0x8(%rcx,%rax,1),%rdi
  6ae217:	48 89 7c 24 20       	mov    %rdi,0x20(%rsp)
  6ae21c:	45 31 ed             	xor    %r13d,%r13d
  6ae21f:	41 80 ff 71          	cmp    $0x71,%r15b
  6ae223:	75 46                	jne    6ae26b <sqlite3VdbeExec+0x120b>
  6ae225:	48 01 c1             	add    %rax,%rcx
  6ae228:	0f b7 42 02          	movzwl 0x2(%rdx),%eax
  6ae22c:	48 8b 49 18          	mov    0x18(%rcx),%rcx
  6ae230:	0f b6 49 70          	movzbl 0x70(%rcx),%ecx
  6ae234:	41 3a 8e c5 00 00 00 	cmp    0xc5(%r14),%cl
  6ae23b:	73 07                	jae    6ae244 <sqlite3VdbeExec+0x11e4>
  6ae23d:	41 88 8e c5 00 00 00 	mov    %cl,0xc5(%r14)
  6ae244:	41 89 c7             	mov    %eax,%r15d
  6ae247:	41 83 e7 08          	and    $0x8,%r15d
  6ae24b:	41 83 cf 04          	or     $0x4,%r15d
  6ae24f:	a8 10                	test   $0x10,%al
  6ae251:	74 1b                	je     6ae26e <sqlite3VdbeExec+0x120e>
  6ae253:	4c 6b e6 38          	imul   $0x38,%rsi,%r12
  6ae257:	4c 8b 74 24 28       	mov    0x28(%rsp),%r14
  6ae25c:	4b 8d 3c 26          	lea    (%r14,%r12,1),%rdi
  6ae260:	e8 1b 2e ff ff       	call   6a1080 <sqlite3VdbeMemIntegerify>
  6ae265:	43 8b 34 26          	mov    (%r14,%r12,1),%esi
  6ae269:	eb 03                	jmp    6ae26e <sqlite3VdbeExec+0x120e>
  6ae26b:	45 31 ff             	xor    %r15d,%r15d
  6ae26e:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6ae273:	0f b6 40 01          	movzbl 0x1(%rax),%eax
  6ae277:	49 89 ec             	mov    %rbp,%r12
  6ae27a:	49 89 f6             	mov    %rsi,%r14
  6ae27d:	3d fd 00 00 00       	cmp    $0xfd,%eax
  6ae282:	0f 84 4f 2e 00 00    	je     6b10d7 <sqlite3VdbeExec+0x4077>
  6ae288:	3d f8 00 00 00       	cmp    $0xf8,%eax
  6ae28d:	0f 85 51 2e 00 00    	jne    6b10e4 <sqlite3VdbeExec+0x4084>
  6ae293:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6ae298:	48 8b 68 10          	mov    0x10(%rax),%rbp
  6ae29c:	44 0f b7 6d 08       	movzwl 0x8(%rbp),%r13d
  6ae2a1:	e9 45 2e 00 00       	jmp    6b10eb <sqlite3VdbeExec+0x408b>
  6ae2a6:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae2ab:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae2af:	48 6b f0 38          	imul   $0x38,%rax,%rsi
  6ae2b3:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae2b8:	48 01 ce             	add    %rcx,%rsi
  6ae2bb:	49 63 45 08          	movslq 0x8(%r13),%rax
  6ae2bf:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae2c3:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6ae2c7:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6ae2cc:	0f 84 ee 42 00 00    	je     6b25c0 <sqlite3VdbeExec+0x5560>
  6ae2d2:	ba 00 40 00 00       	mov    $0x4000,%edx
  6ae2d7:	e8 44 35 ff ff       	call   6a1820 <vdbeClrCopy>
  6ae2dc:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae2e0:	49 83 c5 18          	add    $0x18,%r13
  6ae2e4:	e9 37 f0 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ae2e9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae2ee:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae2f2:	48 6b f8 38          	imul   $0x38,%rax,%rdi
  6ae2f6:	49 63 45 08          	movslq 0x8(%r13),%rax
  6ae2fa:	4c 6b e0 38          	imul   $0x38,%rax,%r12
  6ae2fe:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
  6ae303:	49 01 c4             	add    %rax,%r12
  6ae306:	f6 44 38 14 01       	testb  $0x1,0x14(%rax,%rdi,1)
  6ae30b:	0f 85 ab 37 00 00    	jne    6b1abc <sqlite3VdbeExec+0x4a5c>
  6ae311:	48 01 c7             	add    %rax,%rdi
  6ae314:	31 f6                	xor    %esi,%esi
  6ae316:	e8 a5 2c ff ff       	call   6a0fc0 <sqlite3VdbeBooleanValue>
  6ae31b:	31 f6                	xor    %esi,%esi
  6ae31d:	85 c0                	test   %eax,%eax
  6ae31f:	40 0f 94 c6          	sete   %sil
  6ae323:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6ae329:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae32d:	0f 84 53 5e 00 00    	je     6b4186 <sqlite3VdbeExec+0x7126>
  6ae333:	4c 89 e7             	mov    %r12,%rdi
  6ae336:	e8 a5 32 ff ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6ae33b:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae340:	49 83 c5 18          	add    $0x18,%r13
  6ae344:	e9 d7 ef ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ae349:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae34e:	4d 63 75 04          	movslq 0x4(%r13),%r14
  6ae352:	49 63 45 08          	movslq 0x8(%r13),%rax
  6ae356:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae35a:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae35f:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6ae363:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6ae368:	0f 84 8a 42 00 00    	je     6b25f8 <sqlite3VdbeExec+0x5598>
  6ae36e:	4c 89 e7             	mov    %r12,%rdi
  6ae371:	e8 ca 31 ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6ae376:	e9 85 42 00 00       	jmp    6b2600 <sqlite3VdbeExec+0x55a0>
  6ae37b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae37f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae384:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae388:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae38c:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae391:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6ae395:	4d 63 75 0c          	movslq 0xc(%r13),%r14
  6ae399:	41 8b 6d 10          	mov    0x10(%r13),%ebp
  6ae39d:	f6 44 01 14 10       	testb  $0x10,0x14(%rcx,%rax,1)
  6ae3a2:	75 10                	jne    6ae3b4 <sqlite3VdbeExec+0x1354>
  6ae3a4:	4c 89 e7             	mov    %r12,%rdi
  6ae3a7:	e8 24 33 ff ff       	call   6a16d0 <sqlite3VdbeMemSetRowSet>
  6ae3ac:	85 c0                	test   %eax,%eax
  6ae3ae:	0f 85 b5 77 00 00    	jne    6b5b69 <sqlite3VdbeExec+0x8b09>
  6ae3b4:	4d 6b f6 38          	imul   $0x38,%r14,%r14
  6ae3b8:	4c 03 74 24 28       	add    0x28(%rsp),%r14
  6ae3bd:	85 ed                	test   %ebp,%ebp
  6ae3bf:	0f 84 86 59 00 00    	je     6b3d4b <sqlite3VdbeExec+0x6ceb>
  6ae3c5:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6ae3ca:	49 8b 16             	mov    (%r14),%rdx
  6ae3cd:	89 ee                	mov    %ebp,%esi
  6ae3cf:	e8 dc a2 fd ff       	call   6886b0 <sqlite3RowSetTest>
  6ae3d4:	85 c0                	test   %eax,%eax
  6ae3d6:	0f 84 6b 59 00 00    	je     6b3d47 <sqlite3VdbeExec+0x6ce7>
  6ae3dc:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6ae3e1:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae3e6:	e9 81 74 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6ae3eb:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae3ef:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae3f4:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ae3f8:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6ae3fc:	48 85 c0             	test   %rax,%rax
  6ae3ff:	0f 84 69 2a 00 00    	je     6b0e6e <sqlite3VdbeExec+0x3e0e>
  6ae405:	80 78 02 00          	cmpb   $0x0,0x2(%rax)
  6ae409:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae40d:	0f 85 5f 2a 00 00    	jne    6b0e72 <sqlite3VdbeExec+0x3e12>
  6ae413:	e9 ff ee ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6ae418:	48 b8 00 00 00 00 ff 	movabs $0xffffffff00000000,%rax
  6ae41f:	ff ff ff 
  6ae422:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6ae427:	c7 44 24 68 ff ff ff 	movl   $0xffffffff,0x68(%rsp)
  6ae42e:	ff 
  6ae42f:	41 8b 44 24 28       	mov    0x28(%r12),%eax
  6ae434:	45 31 c0             	xor    %r8d,%r8d
  6ae437:	85 c0                	test   %eax,%eax
  6ae439:	0f 8e f1 41 00 00    	jle    6b2630 <sqlite3VdbeExec+0x55d0>
  6ae43f:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ae444:	8b 51 08             	mov    0x8(%rcx),%edx
  6ae447:	89 54 24 38          	mov    %edx,0x38(%rsp)
  6ae44b:	44 8b 49 04          	mov    0x4(%rcx),%r9d
  6ae44f:	4d 89 cd             	mov    %r9,%r13
  6ae452:	49 c1 e5 05          	shl    $0x5,%r13
  6ae456:	bd 01 00 00 00       	mov    $0x1,%ebp
  6ae45b:	45 31 e4             	xor    %r12d,%r12d
  6ae45e:	4c 8d 5c 24 68       	lea    0x68(%rsp),%r11
  6ae463:	4c 8d 54 24 64       	lea    0x64(%rsp),%r10
  6ae468:	4c 89 4c 24 40       	mov    %r9,0x40(%rsp)
  6ae46d:	0f 1f 00             	nopl   (%rax)
  6ae470:	41 83 f9 0c          	cmp    $0xc,%r9d
  6ae474:	0f 95 c1             	setne  %cl
  6ae477:	4d 39 e5             	cmp    %r12,%r13
  6ae47a:	0f 95 c2             	setne  %dl
  6ae47d:	45 31 f6             	xor    %r14d,%r14d
  6ae480:	84 d1                	test   %dl,%cl
  6ae482:	0f 85 37 01 00 00    	jne    6ae5bf <sqlite3VdbeExec+0x155f>
  6ae488:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6ae48d:	48 8b 40 20          	mov    0x20(%rax),%rax
  6ae491:	4a 8b 44 20 08       	mov    0x8(%rax,%r12,1),%rax
  6ae496:	31 c9                	xor    %ecx,%ecx
  6ae498:	41 be 00 00 00 00    	mov    $0x0,%r14d
  6ae49e:	48 85 c0             	test   %rax,%rax
  6ae4a1:	0f 84 f9 00 00 00    	je     6ae5a0 <sqlite3VdbeExec+0x1540>
  6ae4a7:	48 8b 30             	mov    (%rax),%rsi
  6ae4aa:	48 8b 40 08          	mov    0x8(%rax),%rax
  6ae4ae:	48 89 70 08          	mov    %rsi,0x8(%rax)
  6ae4b2:	41 be 06 00 00 00    	mov    $0x6,%r14d
  6ae4b8:	80 78 24 00          	cmpb   $0x0,0x24(%rax)
  6ae4bc:	0f 85 de 00 00 00    	jne    6ae5a0 <sqlite3VdbeExec+0x1540>
  6ae4c2:	4c 8b 38             	mov    (%rax),%r15
  6ae4c5:	49 8b bf 28 01 00 00 	mov    0x128(%r15),%rdi
  6ae4cc:	48 85 ff             	test   %rdi,%rdi
  6ae4cf:	4c 89 44 24 20       	mov    %r8,0x20(%rsp)
  6ae4d4:	74 10                	je     6ae4e6 <sqlite3VdbeExec+0x1486>
  6ae4d6:	83 7c 24 38 00       	cmpl   $0x0,0x38(%rsp)
  6ae4db:	74 7a                	je     6ae557 <sqlite3VdbeExec+0x14f7>
  6ae4dd:	49 8b 8f e8 00 00 00 	mov    0xe8(%r15),%rcx
  6ae4e4:	eb 73                	jmp    6ae559 <sqlite3VdbeExec+0x14f9>
  6ae4e6:	41 80 7f 09 05       	cmpb   $0x5,0x9(%r15)
  6ae4eb:	41 be 00 00 00 00    	mov    $0x0,%r14d
  6ae4f1:	0f 85 a9 00 00 00    	jne    6ae5a0 <sqlite3VdbeExec+0x1540>
  6ae4f7:	48 8d 05 ea d2 f9 ff 	lea    -0x62d16(%rip),%rax        # 64b7e8 <.L.str.234>
  6ae4fe:	45 31 f6             	xor    %r14d,%r14d
  6ae501:	48 89 f7             	mov    %rsi,%rdi
  6ae504:	48 89 74 24 50       	mov    %rsi,0x50(%rsp)
  6ae509:	48 89 c6             	mov    %rax,%rsi
  6ae50c:	31 d2                	xor    %edx,%edx
  6ae50e:	31 c9                	xor    %ecx,%ecx
  6ae510:	45 31 c0             	xor    %r8d,%r8d
  6ae513:	4c 89 94 24 b8 00 00 	mov    %r10,0xb8(%rsp)
  6ae51a:	00 
  6ae51b:	4c 89 9c 24 c8 00 00 	mov    %r11,0xc8(%rsp)
  6ae522:	00 
  6ae523:	e8 88 6e fd ff       	call   6853b0 <sqlite3_exec>
  6ae528:	31 c9                	xor    %ecx,%ecx
  6ae52a:	4c 8b 9c 24 c8 00 00 	mov    0xc8(%rsp),%r11
  6ae531:	00 
  6ae532:	4c 8b 94 24 b8 00 00 	mov    0xb8(%rsp),%r10
  6ae539:	00 
  6ae53a:	4c 8b 4c 24 40       	mov    0x40(%rsp),%r9
  6ae53f:	4c 8b 44 24 20       	mov    0x20(%rsp),%r8
  6ae544:	48 8b 74 24 50       	mov    0x50(%rsp),%rsi
  6ae549:	49 8b bf 28 01 00 00 	mov    0x128(%r15),%rdi
  6ae550:	48 85 ff             	test   %rdi,%rdi
  6ae553:	75 81                	jne    6ae4d6 <sqlite3VdbeExec+0x1476>
  6ae555:	eb 49                	jmp    6ae5a0 <sqlite3VdbeExec+0x1540>
  6ae557:	31 c9                	xor    %ecx,%ecx
  6ae559:	4d 8b 87 f0 00 00 00 	mov    0xf0(%r15),%r8
  6ae560:	45 0f b6 4f 0f       	movzbl 0xf(%r15),%r9d
  6ae565:	41 8b 87 c8 00 00 00 	mov    0xc8(%r15),%eax
  6ae56c:	8b 54 24 38          	mov    0x38(%rsp),%edx
  6ae570:	41 53                	push   %r11
  6ae572:	41 52                	push   %r10
  6ae574:	41 ff b7 18 01 00 00 	push   0x118(%r15)
  6ae57b:	50                   	push   %rax
  6ae57c:	e8 5f 00 fe ff       	call   68e5e0 <sqlite3WalCheckpoint>
  6ae581:	48 83 c4 20          	add    $0x20,%rsp
  6ae585:	41 89 c6             	mov    %eax,%r14d
  6ae588:	4c 8b 44 24 20       	mov    0x20(%rsp),%r8
  6ae58d:	4c 8b 4c 24 40       	mov    0x40(%rsp),%r9
  6ae592:	31 c9                	xor    %ecx,%ecx
  6ae594:	66 66 66 2e 0f 1f 84 	data16 data16 cs nopw 0x0(%rax,%rax,1)
  6ae59b:	00 00 00 00 00 
  6ae5a0:	41 83 fe 05          	cmp    $0x5,%r14d
  6ae5a4:	44 0f 44 f1          	cmove  %ecx,%r14d
  6ae5a8:	b8 01 00 00 00       	mov    $0x1,%eax
  6ae5ad:	44 0f 44 c0          	cmove  %eax,%r8d
  6ae5b1:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6ae5b6:	8b 40 28             	mov    0x28(%rax),%eax
  6ae5b9:	45 31 d2             	xor    %r10d,%r10d
  6ae5bc:	45 31 db             	xor    %r11d,%r11d
  6ae5bf:	48 63 c8             	movslq %eax,%rcx
  6ae5c2:	48 39 cd             	cmp    %rcx,%rbp
  6ae5c5:	7d 10                	jge    6ae5d7 <sqlite3VdbeExec+0x1577>
  6ae5c7:	49 83 c4 20          	add    $0x20,%r12
  6ae5cb:	48 ff c5             	inc    %rbp
  6ae5ce:	45 85 f6             	test   %r14d,%r14d
  6ae5d1:	0f 84 99 fe ff ff    	je     6ae470 <sqlite3VdbeExec+0x1410>
  6ae5d7:	45 85 f6             	test   %r14d,%r14d
  6ae5da:	0f 94 c0             	sete   %al
  6ae5dd:	45 85 c0             	test   %r8d,%r8d
  6ae5e0:	0f 95 c1             	setne  %cl
  6ae5e3:	84 c8                	test   %cl,%al
  6ae5e5:	0f 85 3f 3b 00 00    	jne    6b212a <sqlite3VdbeExec+0x50ca>
  6ae5eb:	4c 89 f0             	mov    %r14,%rax
  6ae5ee:	83 f8 05             	cmp    $0x5,%eax
  6ae5f1:	0f 84 33 3b 00 00    	je     6b212a <sqlite3VdbeExec+0x50ca>
  6ae5f7:	85 c0                	test   %eax,%eax
  6ae5f9:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae5fe:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae602:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae607:	0f 85 f2 7b 00 00    	jne    6b61ff <sqlite3VdbeExec+0x919f>
  6ae60d:	4c 63 44 24 60       	movslq 0x60(%rsp),%r8
  6ae612:	e9 2b 3b 00 00       	jmp    6b2142 <sqlite3VdbeExec+0x50e2>
  6ae617:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae61c:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ae620:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ae627:	00 00 
  6ae629:	48 85 c0             	test   %rax,%rax
  6ae62c:	0f 84 2f 40 00 00    	je     6b2661 <sqlite3VdbeExec+0x5601>
  6ae632:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae636:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae63b:	48 8b 14 01          	mov    (%rcx,%rax,1),%rdx
  6ae63f:	e9 1f 40 00 00       	jmp    6b2663 <sqlite3VdbeExec+0x5603>
  6ae644:	49 8b 46 68          	mov    0x68(%r14),%rax
  6ae648:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae64d:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6ae651:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6ae655:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6ae659:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6ae65e:	0f 84 31 40 00 00    	je     6b2695 <sqlite3VdbeExec+0x5635>
  6ae664:	4c 89 e7             	mov    %r12,%rdi
  6ae667:	e8 f4 93 09 00       	call   747a60 <out2PrereleaseWithClear>
  6ae66c:	e9 2c 40 00 00       	jmp    6b269d <sqlite3VdbeExec+0x563d>
  6ae671:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae675:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae67a:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ae67e:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6ae682:	48 c1 e1 05          	shl    $0x5,%rcx
  6ae686:	49 8b 54 24 20       	mov    0x20(%r12),%rdx
  6ae68b:	48 8b 4c 0a 08       	mov    0x8(%rdx,%rcx,1),%rcx
  6ae690:	48 8b 31             	mov    (%rcx),%rsi
  6ae693:	48 8b 51 08          	mov    0x8(%rcx),%rdx
  6ae697:	48 89 72 08          	mov    %rsi,0x8(%rdx)
  6ae69b:	48 83 f8 0f          	cmp    $0xf,%rax
  6ae69f:	0f 85 3d 34 00 00    	jne    6b1ae2 <sqlite3VdbeExec+0x4a82>
  6ae6a5:	48 8b 02             	mov    (%rdx),%rax
  6ae6a8:	8b 69 1c             	mov    0x1c(%rcx),%ebp
  6ae6ab:	03 a8 84 00 00 00    	add    0x84(%rax),%ebp
  6ae6b1:	e9 3a 34 00 00       	jmp    6b1af0 <sqlite3VdbeExec+0x4a90>
  6ae6b6:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6ae6bb:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae6c0:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ae6c4:	48 c1 e1 05          	shl    $0x5,%rcx
  6ae6c8:	4c 8d 34 08          	lea    (%rax,%rcx,1),%r14
  6ae6cc:	48 8b 7c 08 08       	mov    0x8(%rax,%rcx,1),%rdi
  6ae6d1:	41 8b 75 08          	mov    0x8(%r13),%esi
  6ae6d5:	41 8b 55 0c          	mov    0xc(%r13),%edx
  6ae6d9:	e8 92 e5 fe ff       	call   69cc70 <sqlite3BtreeUpdateMeta>
  6ae6de:	41 89 c7             	mov    %eax,%r15d
  6ae6e1:	41 8b 45 08          	mov    0x8(%r13),%eax
  6ae6e5:	83 f8 02             	cmp    $0x2,%eax
  6ae6e8:	0f 84 20 4f 00 00    	je     6b360e <sqlite3VdbeExec+0x65ae>
  6ae6ee:	83 f8 01             	cmp    $0x1,%eax
  6ae6f1:	0f 85 23 4f 00 00    	jne    6b361a <sqlite3VdbeExec+0x65ba>
  6ae6f7:	41 8b 45 0c          	mov    0xc(%r13),%eax
  6ae6fb:	41 0f b7 4d 02       	movzwl 0x2(%r13),%ecx
  6ae700:	29 c8                	sub    %ecx,%eax
  6ae702:	49 8b 4e 18          	mov    0x18(%r14),%rcx
  6ae706:	89 01                	mov    %eax,(%rcx)
  6ae708:	41 80 4c 24 2c 01    	orb    $0x1,0x2c(%r12)
  6ae70e:	41 8b 75 04          	mov    0x4(%r13),%esi
  6ae712:	4c 89 e7             	mov    %r12,%rdi
  6ae715:	e8 e6 83 00 00       	call   6b6b00 <sqlite3FkClearTriggerCache>
  6ae71a:	e9 fb 4e 00 00       	jmp    6b361a <sqlite3VdbeExec+0x65ba>
  6ae71f:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6ae726:	00 00 
  6ae728:	41 80 7c 24 67 00    	cmpb   $0x0,0x67(%r12)
  6ae72e:	0f 85 7d 76 00 00    	jne    6b5db1 <sqlite3VdbeExec+0x8d51>
  6ae734:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae738:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae73d:	49 8b 45 10          	mov    0x10(%r13),%rax
  6ae741:	4c 8b 60 10          	mov    0x10(%rax),%r12
  6ae745:	bd 06 00 00 00       	mov    $0x6,%ebp
  6ae74a:	4d 85 e4             	test   %r12,%r12
  6ae74d:	0f 84 dd 7a 00 00    	je     6b6230 <sqlite3VdbeExec+0x91d0>
  6ae753:	49 8b 04 24          	mov    (%r12),%rax
  6ae757:	48 85 c0             	test   %rax,%rax
  6ae75a:	0f 84 d0 7a 00 00    	je     6b6230 <sqlite3VdbeExec+0x91d0>
  6ae760:	48 8b 40 68          	mov    0x68(%rax),%rax
  6ae764:	48 85 c0             	test   %rax,%rax
  6ae767:	0f 84 4f 5e 00 00    	je     6b45bc <sqlite3VdbeExec+0x755c>
  6ae76d:	41 8b 75 08          	mov    0x8(%r13),%esi
  6ae771:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
  6ae776:	0f b6 49 6c          	movzbl 0x6c(%rcx),%ecx
  6ae77a:	88 4c 24 20          	mov    %cl,0x20(%rsp)
  6ae77e:	49 8b 56 70          	mov    0x70(%r14),%rdx
  6ae782:	85 f6                	test   %esi,%esi
  6ae784:	0f 8e 99 5d 00 00    	jle    6b4523 <sqlite3VdbeExec+0x74c3>
  6ae78a:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6ae78e:	4c 6b c1 38          	imul   $0x38,%rcx,%r8
  6ae792:	4c 03 44 24 28       	add    0x28(%rsp),%r8
  6ae797:	83 fe 04             	cmp    $0x4,%esi
  6ae79a:	0f 83 10 5d 00 00    	jae    6b44b0 <sqlite3VdbeExec+0x7450>
  6ae7a0:	31 c9                	xor    %ecx,%ecx
  6ae7a2:	4c 89 c7             	mov    %r8,%rdi
  6ae7a5:	e9 69 5d 00 00       	jmp    6b4513 <sqlite3VdbeExec+0x74b3>
  6ae7aa:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae7ae:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae7b3:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6ae7b7:	4c 89 f7             	mov    %r14,%rdi
  6ae7ba:	4c 8b 34 c8          	mov    (%rax,%rcx,8),%r14
  6ae7be:	41 8b 75 04          	mov    0x4(%r13),%esi
  6ae7c2:	41 0f bf 56 40       	movswl 0x40(%r14),%edx
  6ae7c7:	31 c9                	xor    %ecx,%ecx
  6ae7c9:	e8 c2 83 00 00       	call   6b6b90 <allocateCursor>
  6ae7ce:	48 85 c0             	test   %rax,%rax
  6ae7d1:	0f 84 fc 72 00 00    	je     6b5ad3 <sqlite3VdbeExec+0x8a73>
  6ae7d7:	c6 40 02 01          	movb   $0x1,0x2(%rax)
  6ae7db:	0f b6 48 05          	movzbl 0x5(%rax),%ecx
  6ae7df:	80 c9 01             	or     $0x1,%cl
  6ae7e2:	88 48 05             	mov    %cl,0x5(%rax)
  6ae7e5:	49 8b 56 30          	mov    0x30(%r14),%rdx
  6ae7e9:	48 89 50 30          	mov    %rdx,0x30(%rax)
  6ae7ed:	41 0f b6 56 04       	movzbl 0x4(%r14),%edx
  6ae7f2:	88 50 04             	mov    %dl,0x4(%rax)
  6ae7f5:	41 8b 56 3c          	mov    0x3c(%r14),%edx
  6ae7f9:	89 50 3c             	mov    %edx,0x3c(%rax)
  6ae7fc:	41 0f b6 56 05       	movzbl 0x5(%r14),%edx
  6ae801:	80 e2 04             	and    $0x4,%dl
  6ae804:	80 e1 fb             	and    $0xfb,%cl
  6ae807:	08 d1                	or     %dl,%cl
  6ae809:	88 48 05             	mov    %cl,0x5(%rax)
  6ae80c:	49 8b 56 08          	mov    0x8(%r14),%rdx
  6ae810:	48 89 50 08          	mov    %rdx,0x8(%rax)
  6ae814:	80 c9 08             	or     $0x8,%cl
  6ae817:	88 48 05             	mov    %cl,0x5(%rax)
  6ae81a:	41 80 4e 05 08       	orb    $0x8,0x5(%r14)
  6ae81f:	48 8b 70 08          	mov    0x8(%rax),%rsi
  6ae823:	48 8b 48 28          	mov    0x28(%rax),%rcx
  6ae827:	8b 50 3c             	mov    0x3c(%rax),%edx
  6ae82a:	48 8b 40 30          	mov    0x30(%rax),%rax
  6ae82e:	80 7e 11 00          	cmpb   $0x0,0x11(%rsi)
  6ae832:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae836:	0f 84 a4 4b 00 00    	je     6b33e0 <sqlite3VdbeExec+0x6380>
  6ae83c:	4c 8b 06             	mov    (%rsi),%r8
  6ae83f:	48 8b 7e 08          	mov    0x8(%rsi),%rdi
  6ae843:	4c 89 47 08          	mov    %r8,0x8(%rdi)
  6ae847:	e9 98 4b 00 00       	jmp    6b33e4 <sqlite3VdbeExec+0x6384>
  6ae84c:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae850:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6ae855:	48 63 4a 04          	movslq 0x4(%rdx),%rcx
  6ae859:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ae85d:	48 63 42 08          	movslq 0x8(%rdx),%rax
  6ae861:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae865:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6ae86a:	4c 8d 2c 01          	lea    (%rcx,%rax,1),%r13
  6ae86e:	f6 44 01 15 04       	testb  $0x4,0x15(%rcx,%rax,1)
  6ae873:	74 10                	je     6ae885 <sqlite3VdbeExec+0x1825>
  6ae875:	4c 89 ef             	mov    %r13,%rdi
  6ae878:	e8 43 21 ff ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6ae87d:	85 c0                	test   %eax,%eax
  6ae87f:	0f 85 df 77 00 00    	jne    6b6064 <sqlite3VdbeExec+0x9004>
  6ae885:	4c 89 e7             	mov    %r12,%rdi
  6ae888:	4c 89 ee             	mov    %r13,%rsi
  6ae88b:	e8 80 8f 00 00       	call   6b7810 <sqlite3VdbeSorterWrite>
  6ae890:	85 c0                	test   %eax,%eax
  6ae892:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae896:	0f 84 66 51 00 00    	je     6b3a02 <sqlite3VdbeExec+0x69a2>
  6ae89c:	e9 98 71 00 00       	jmp    6b5a39 <sqlite3VdbeExec+0x89d9>
  6ae8a1:	4d 89 f7             	mov    %r14,%r15
  6ae8a4:	49 8b 46 68          	mov    0x68(%r14),%rax
  6ae8a8:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6ae8ad:	49 63 4e 08          	movslq 0x8(%r14),%rcx
  6ae8b1:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6ae8b5:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6ae8b9:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6ae8be:	0f 84 17 3e 00 00    	je     6b26db <sqlite3VdbeExec+0x567b>
  6ae8c4:	4c 89 e7             	mov    %r12,%rdi
  6ae8c7:	e8 94 91 09 00       	call   747a60 <out2PrereleaseWithClear>
  6ae8cc:	e9 12 3e 00 00       	jmp    6b26e3 <sqlite3VdbeExec+0x5683>
  6ae8d1:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae8d5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae8da:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6ae8de:	4c 89 e7             	mov    %r12,%rdi
  6ae8e1:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ae8e5:	4c 89 e6             	mov    %r12,%rsi
  6ae8e8:	e8 e3 8d 00 00       	call   6b76d0 <sqlite3VdbeSorterNext>
  6ae8ed:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae8f1:	41 c7 44 24 18 00 00 	movl   $0x0,0x18(%r12)
  6ae8f8:	00 00 
  6ae8fa:	83 f8 65             	cmp    $0x65,%eax
  6ae8fd:	74 2e                	je     6ae92d <sqlite3VdbeExec+0x18cd>
  6ae8ff:	85 c0                	test   %eax,%eax
  6ae901:	0f 85 2b 79 00 00    	jne    6b6232 <sqlite3VdbeExec+0x91d2>
  6ae907:	41 c6 44 24 02 00    	movb   $0x0,0x2(%r12)
  6ae90d:	41 0f b7 45 02       	movzwl 0x2(%r13),%eax
  6ae912:	41 ff 84 86 d4 00 00 	incl   0xd4(%r14,%rax,4)
  6ae919:	00 
  6ae91a:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ae921:	00 00 
  6ae923:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae928:	e9 45 25 00 00       	jmp    6b0e72 <sqlite3VdbeExec+0x3e12>
  6ae92d:	41 c6 44 24 02 01    	movb   $0x1,0x2(%r12)
  6ae933:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6ae93a:	00 00 
  6ae93c:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae941:	e9 41 25 00 00       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6ae946:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae94b:	e9 d5 52 00 00       	jmp    6b3c25 <sqlite3VdbeExec+0x6bc5>
  6ae950:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae955:	49 63 45 04          	movslq 0x4(%r13),%rax
  6ae959:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6ae95d:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6ae962:	48 8b 0c 06          	mov    (%rsi,%rax,1),%rcx
  6ae966:	48 ba ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rdx
  6ae96d:	ff ff 7f 
  6ae970:	48 ff c2             	inc    %rdx
  6ae973:	48 39 d1             	cmp    %rdx,%rcx
  6ae976:	0f 85 9f 31 00 00    	jne    6b1b1b <sqlite3VdbeExec+0x4abb>
  6ae97c:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae980:	49 83 c5 18          	add    $0x18,%r13
  6ae984:	e9 97 e9 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ae989:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae98d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6ae992:	49 63 4d 1c          	movslq 0x1c(%r13),%rcx
  6ae996:	4c 8b 24 c8          	mov    (%rax,%rcx,8),%r12
  6ae99a:	49 8b 44 24 28       	mov    0x28(%r12),%rax
  6ae99f:	80 38 00             	cmpb   $0x0,(%rax)
  6ae9a2:	4c 8b 3c 24          	mov    (%rsp),%r15
  6ae9a6:	0f 84 ef 3d 00 00    	je     6b279b <sqlite3VdbeExec+0x573b>
  6ae9ac:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6ae9b1:	49 83 c5 18          	add    $0x18,%r13
  6ae9b5:	e9 66 e9 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6ae9ba:	49 8b 46 78          	mov    0x78(%r14),%rax
  6ae9be:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6ae9c3:	48 63 49 04          	movslq 0x4(%rcx),%rcx
  6ae9c7:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6ae9cb:	48 8b 40 28          	mov    0x28(%rax),%rax
  6ae9cf:	80 78 58 00          	cmpb   $0x0,0x58(%rax)
  6ae9d3:	0f 84 6b 3e 00 00    	je     6b2844 <sqlite3VdbeExec+0x57e4>
  6ae9d9:	4c 8b 2c 24          	mov    (%rsp),%r13
  6ae9dd:	48 8b 40 18          	mov    0x18(%rax),%rax
  6ae9e1:	48 8b 48 10          	mov    0x10(%rax),%rcx
  6ae9e5:	48 8b 50 18          	mov    0x18(%rax),%rdx
  6ae9e9:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6ae9ed:	48 8d 0c 80          	lea    (%rax,%rax,4),%rcx
  6ae9f1:	48 c1 e1 04          	shl    $0x4,%rcx
  6ae9f5:	48 8d 04 0a          	lea    (%rdx,%rcx,1),%rax
  6ae9f9:	48 83 c0 14          	add    $0x14,%rax
  6ae9fd:	4c 8b 7c 0a 28       	mov    0x28(%rdx,%rcx,1),%r15
  6aea02:	e9 49 3e 00 00       	jmp    6b2850 <sqlite3VdbeExec+0x57f0>
  6aea07:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aea0c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6aea10:	49 8b 75 10          	mov    0x10(%r13),%rsi
  6aea14:	49 8b 4c 24 20       	mov    0x20(%r12),%rcx
  6aea19:	48 c1 e0 05          	shl    $0x5,%rax
  6aea1d:	48 8b 7c 01 18       	mov    0x18(%rcx,%rax,1),%rdi
  6aea22:	48 83 c7 20          	add    $0x20,%rdi
  6aea26:	31 d2                	xor    %edx,%edx
  6aea28:	e8 83 46 fd ff       	call   6830b0 <sqlite3HashInsert>
  6aea2d:	48 85 c0             	test   %rax,%rax
  6aea30:	4c 8b 3c 24          	mov    (%rsp),%r15
  6aea34:	0f 84 9e 52 00 00    	je     6b3cd8 <sqlite3VdbeExec+0x6c78>
  6aea3a:	48 8b 48 18          	mov    0x18(%rax),%rcx
  6aea3e:	48 8b 51 10          	mov    0x10(%rcx),%rdx
  6aea42:	48 39 c2             	cmp    %rax,%rdx
  6aea45:	0f 84 7a 52 00 00    	je     6b3cc5 <sqlite3VdbeExec+0x6c65>
  6aea4b:	48 85 d2             	test   %rdx,%rdx
  6aea4e:	0f 84 79 52 00 00    	je     6b3ccd <sqlite3VdbeExec+0x6c6d>
  6aea54:	48 89 d1             	mov    %rdx,%rcx
  6aea57:	48 8b 52 28          	mov    0x28(%rdx),%rdx
  6aea5b:	48 39 c2             	cmp    %rax,%rdx
  6aea5e:	75 eb                	jne    6aea4b <sqlite3VdbeExec+0x19eb>
  6aea60:	48 8b 50 28          	mov    0x28(%rax),%rdx
  6aea64:	48 89 51 28          	mov    %rdx,0x28(%rcx)
  6aea68:	e9 60 52 00 00       	jmp    6b3ccd <sqlite3VdbeExec+0x6c6d>
  6aea6d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aea72:	41 8b 75 04          	mov    0x4(%r13),%esi
  6aea76:	41 8b 55 0c          	mov    0xc(%r13),%edx
  6aea7a:	4c 89 f7             	mov    %r14,%rdi
  6aea7d:	b9 03 00 00 00       	mov    $0x3,%ecx
  6aea82:	e8 09 81 00 00       	call   6b6b90 <allocateCursor>
  6aea87:	48 85 c0             	test   %rax,%rax
  6aea8a:	0f 84 82 74 00 00    	je     6b5f12 <sqlite3VdbeExec+0x8eb2>
  6aea90:	c6 40 02 01          	movb   $0x1,0x2(%rax)
  6aea94:	41 8b 4d 08          	mov    0x8(%r13),%ecx
  6aea98:	89 48 1c             	mov    %ecx,0x1c(%rax)
  6aea9b:	c6 40 04 01          	movb   $0x1,0x4(%rax)
  6aea9f:	48 8d 0d d2 18 0e 00 	lea    0xe18d2(%rip),%rcx        # 790378 <sqlite3BtreeFakeValidCursor.fakeCursor>
  6aeaa6:	48 89 48 28          	mov    %rcx,0x28(%rax)
  6aeaaa:	4c 8b 3c 24          	mov    (%rsp),%r15
  6aeaae:	49 83 c5 18          	add    $0x18,%r13
  6aeab2:	e9 69 e8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6aeab7:	49 8b 46 78          	mov    0x78(%r14),%rax
  6aeabb:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6aeac0:	48 63 4a 04          	movslq 0x4(%rdx),%rcx
  6aeac4:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6aeac8:	0f b7 40 06          	movzwl 0x6(%rax),%eax
  6aeacc:	39 42 10             	cmp    %eax,0x10(%rdx)
  6aeacf:	0f 8e 56 22 00 00    	jle    6b0d2b <sqlite3VdbeExec+0x3ccb>
  6aead5:	49 8b 46 78          	mov    0x78(%r14),%rax
  6aead9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aeade:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6aeae2:	4c 8b 3c c8          	mov    (%rax,%rcx,8),%r15
  6aeae6:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6aeaea:	48 6b f8 38          	imul   $0x38,%rax,%rdi
  6aeaee:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6aeaf3:	48 89 7c 24 68       	mov    %rdi,0x68(%rsp)
  6aeaf8:	41 8b 45 10          	mov    0x10(%r13),%eax
  6aeafc:	66 89 44 24 7c       	mov    %ax,0x7c(%rsp)
  6aeb01:	a9 ff ff 00 00       	test   $0xffff,%eax
  6aeb06:	0f 84 ad 00 00 00    	je     6aebb9 <sqlite3VdbeExec+0x1b59>
  6aeb0c:	49 8b 47 30          	mov    0x30(%r15),%rax
  6aeb10:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6aeb15:	c6 44 24 7e 00       	movb   $0x0,0x7e(%rsp)
  6aeb1a:	49 8b 7f 28          	mov    0x28(%r15),%rdi
  6aeb1e:	49 8d 57 1c          	lea    0x1c(%r15),%rdx
  6aeb22:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6aeb27:	e8 34 69 fe ff       	call   695460 <sqlite3BtreeIndexMoveto>
  6aeb2c:	85 c0                	test   %eax,%eax
  6aeb2e:	0f 85 c8 74 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6aeb34:	41 8b 4f 1c          	mov    0x1c(%r15),%ecx
  6aeb38:	85 c9                	test   %ecx,%ecx
  6aeb3a:	41 0f 95 47 02       	setne  0x2(%r15)
  6aeb3f:	41 c6 47 03 00       	movb   $0x0,0x3(%r15)
  6aeb44:	41 c7 47 18 00 00 00 	movl   $0x0,0x18(%r15)
  6aeb4b:	00 
  6aeb4c:	41 0f b6 45 00       	movzbl 0x0(%r13),%eax
  6aeb51:	83 f8 1d             	cmp    $0x1d,%eax
  6aeb54:	75 12                	jne    6aeb68 <sqlite3VdbeExec+0x1b08>
  6aeb56:	85 c9                	test   %ecx,%ecx
  6aeb58:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6aeb5d:	0f 85 86 5e 00 00    	jne    6b49e9 <sqlite3VdbeExec+0x7989>
  6aeb63:	e9 f7 1e 00 00       	jmp    6b0a5f <sqlite3VdbeExec+0x39ff>
  6aeb68:	85 c9                	test   %ecx,%ecx
  6aeb6a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6aeb6f:	0f 85 ea 1e 00 00    	jne    6b0a5f <sqlite3VdbeExec+0x39ff>
  6aeb75:	83 f8 1a             	cmp    $0x1a,%eax
  6aeb78:	0f 84 20 52 00 00    	je     6b3d9e <sqlite3VdbeExec+0x6d3e>
  6aeb7e:	83 f8 1b             	cmp    $0x1b,%eax
  6aeb81:	0f 85 62 5e 00 00    	jne    6b49e9 <sqlite3VdbeExec+0x7989>
  6aeb87:	0f b7 4c 24 7c       	movzwl 0x7c(%rsp),%ecx
  6aeb8c:	48 85 c9             	test   %rcx,%rcx
  6aeb8f:	0f 84 54 5e 00 00    	je     6b49e9 <sqlite3VdbeExec+0x7989>
  6aeb95:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  6aeb9a:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6aeb9e:	31 d2                	xor    %edx,%edx
  6aeba0:	f6 44 10 14 01       	testb  $0x1,0x14(%rax,%rdx,1)
  6aeba5:	0f 85 b4 1e 00 00    	jne    6b0a5f <sqlite3VdbeExec+0x39ff>
  6aebab:	48 83 c2 38          	add    $0x38,%rdx
  6aebaf:	48 39 d1             	cmp    %rdx,%rcx
  6aebb2:	75 ec                	jne    6aeba0 <sqlite3VdbeExec+0x1b40>
  6aebb4:	e9 30 5e 00 00       	jmp    6b49e9 <sqlite3VdbeExec+0x7989>
  6aebb9:	f6 47 15 04          	testb  $0x4,0x15(%rdi)
  6aebbd:	74 0d                	je     6aebcc <sqlite3VdbeExec+0x1b6c>
  6aebbf:	e8 fc 1d ff ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6aebc4:	85 c0                	test   %eax,%eax
  6aebc6:	0f 85 3c 73 00 00    	jne    6b5f08 <sqlite3VdbeExec+0x8ea8>
  6aebcc:	4d 8b 77 30          	mov    0x30(%r15),%r14
  6aebd0:	41 0f b7 46 06       	movzwl 0x6(%r14),%eax
  6aebd5:	48 6b f0 38          	imul   $0x38,%rax,%rsi
  6aebd9:	48 83 c6 60          	add    $0x60,%rsi
  6aebdd:	49 8b 7e 10          	mov    0x10(%r14),%rdi
  6aebe1:	48 85 ff             	test   %rdi,%rdi
  6aebe4:	0f 84 81 32 00 00    	je     6b1e6b <sqlite3VdbeExec+0x4e0b>
  6aebea:	e8 61 bc fc ff       	call   67a850 <sqlite3DbMallocRawNN>
  6aebef:	e9 7f 32 00 00       	jmp    6b1e73 <sqlite3VdbeExec+0x4e13>
  6aebf4:	49 8b 46 78          	mov    0x78(%r14),%rax
  6aebf8:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aebfd:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6aec01:	48 8b 3c c8          	mov    (%rax,%rcx,8),%rdi
  6aec05:	80 7f 03 00          	cmpb   $0x0,0x3(%rdi)
  6aec09:	0f 84 dd 3c 00 00    	je     6b28ec <sqlite3VdbeExec+0x588c>
  6aec0f:	e8 5c 92 ff ff       	call   6a7e70 <sqlite3VdbeFinishMoveto>
  6aec14:	85 c0                	test   %eax,%eax
  6aec16:	4c 8b 3c 24          	mov    (%rsp),%r15
  6aec1a:	0f 85 c4 77 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6aec20:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6aec27:	00 00 
  6aec29:	49 83 c5 18          	add    $0x18,%r13
  6aec2d:	e9 ee e6 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6aec32:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aec37:	41 8b 75 04          	mov    0x4(%r13),%esi
  6aec3b:	49 8b 55 10          	mov    0x10(%r13),%rdx
  6aec3f:	4c 89 e7             	mov    %r12,%rdi
  6aec42:	e8 69 a3 00 00       	call   6b8fb0 <sqlite3UnlinkAndDeleteTrigger>
  6aec47:	4c 8b 3c 24          	mov    (%rsp),%r15
  6aec4b:	49 83 c5 18          	add    $0x18,%r13
  6aec4f:	e9 cc e6 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6aec54:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aec59:	49 63 45 04          	movslq 0x4(%r13),%rax
  6aec5d:	49 8b 75 10          	mov    0x10(%r13),%rsi
  6aec61:	49 8b 4c 24 20       	mov    0x20(%r12),%rcx
  6aec66:	48 c1 e0 05          	shl    $0x5,%rax
  6aec6a:	48 8b 7c 01 18       	mov    0x18(%rcx,%rax,1),%rdi
  6aec6f:	48 83 c7 08          	add    $0x8,%rdi
  6aec73:	31 d2                	xor    %edx,%edx
  6aec75:	e8 36 44 fd ff       	call   6830b0 <sqlite3HashInsert>
  6aec7a:	48 85 c0             	test   %rax,%rax
  6aec7d:	0f 84 6d 50 00 00    	je     6b3cf0 <sqlite3VdbeExec+0x6c90>
  6aec83:	49 83 bc 24 00 03 00 	cmpq   $0x0,0x300(%r12)
  6aec8a:	00 00 
  6aec8c:	0f 84 55 50 00 00    	je     6b3ce7 <sqlite3VdbeExec+0x6c87>
  6aec92:	4c 89 e7             	mov    %r12,%rdi
  6aec95:	48 89 c6             	mov    %rax,%rsi
  6aec98:	e8 a3 df 02 00       	call   6dcc40 <deleteTable>
  6aec9d:	e9 4e 50 00 00       	jmp    6b3cf0 <sqlite3VdbeExec+0x6c90>
  6aeca2:	49 bf 00 00 00 00 00 	movabs $0x1000000000000,%r15
  6aeca9:	00 01 00 
  6aecac:	49 bd 00 00 00 00 00 	movabs $0x800000000000,%r13
  6aecb3:	80 00 00 
  6aecb6:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6aecbb:	48 8b 68 10          	mov    0x10(%rax),%rbp
  6aecbf:	48 63 40 04          	movslq 0x4(%rax),%rax
  6aecc3:	4c 6b e0 38          	imul   $0x38,%rax,%r12
  6aecc7:	4c 03 64 24 28       	add    0x28(%rsp),%r12
  6aeccc:	0f b6 45 00          	movzbl 0x0(%rbp),%eax
  6aecd0:	48 ff c5             	inc    %rbp
  6aecd3:	eb 39                	jmp    6aed0e <sqlite3VdbeExec+0x1cae>
  6aecd5:	0f 57 c0             	xorps  %xmm0,%xmm0
  6aecd8:	f2 48 0f 2a c1       	cvtsi2sd %rcx,%xmm0
  6aecdd:	f2 41 0f 11 04 24    	movsd  %xmm0,(%r12)
  6aece3:	66 b9 08 00          	mov    $0x8,%cx
  6aece7:	66 ba f1 ff          	mov    $0xfff1,%dx
  6aeceb:	0f bf d2             	movswl %dx,%edx
  6aecee:	21 c2                	and    %eax,%edx
  6aecf0:	0f b7 c1             	movzwl %cx,%eax
  6aecf3:	09 d0                	or     %edx,%eax
  6aecf5:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6aecfb:	0f b6 45 00          	movzbl 0x0(%rbp),%eax
  6aecff:	49 83 c4 38          	add    $0x38,%r12
  6aed03:	48 ff c5             	inc    %rbp
  6aed06:	84 c0                	test   %al,%al
  6aed08:	0f 84 98 6c 00 00    	je     6b59a6 <sqlite3VdbeExec+0x8946>
  6aed0e:	0f be f0             	movsbl %al,%esi
  6aed11:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6aed16:	4c 89 e7             	mov    %r12,%rdi
  6aed19:	e8 72 e2 ff ff       	call   6acf90 <applyAffinity>
  6aed1e:	80 7d ff 45          	cmpb   $0x45,-0x1(%rbp)
  6aed22:	75 d7                	jne    6aecfb <sqlite3VdbeExec+0x1c9b>
  6aed24:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6aed2a:	a8 04                	test   $0x4,%al
  6aed2c:	74 cd                	je     6aecfb <sqlite3VdbeExec+0x1c9b>
  6aed2e:	49 8b 0c 24          	mov    (%r12),%rcx
  6aed32:	4a 8d 14 29          	lea    (%rcx,%r13,1),%rdx
  6aed36:	4c 39 fa             	cmp    %r15,%rdx
  6aed39:	73 9a                	jae    6aecd5 <sqlite3VdbeExec+0x1c75>
  6aed3b:	66 b9 20 00          	mov    $0x20,%cx
  6aed3f:	66 ba db ff          	mov    $0xffdb,%dx
  6aed43:	eb a6                	jmp    6aeceb <sqlite3VdbeExec+0x1c8b>
  6aed45:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aed4a:	4d 8b 7d 10          	mov    0x10(%r13),%r15
  6aed4e:	41 0f b7 47 36       	movzwl 0x36(%r15),%eax
  6aed53:	66 85 c0             	test   %ax,%ax
  6aed56:	0f 8e 9d 3b 00 00    	jle    6b28f9 <sqlite3VdbeExec+0x5899>
  6aed5c:	4c 89 e9             	mov    %r13,%rcx
  6aed5f:	4d 8b 6f 08          	mov    0x8(%r15),%r13
  6aed63:	48 63 49 04          	movslq 0x4(%rcx),%rcx
  6aed67:	4c 6b e1 38          	imul   $0x38,%rcx,%r12
  6aed6b:	4c 03 64 24 28       	add    0x28(%rsp),%r12
  6aed70:	49 83 c5 0e          	add    $0xe,%r13
  6aed74:	31 ed                	xor    %ebp,%ebp
  6aed76:	e9 fa 27 00 00       	jmp    6b1575 <sqlite3VdbeExec+0x4515>
  6aed7b:	49 8b 46 68          	mov    0x68(%r14),%rax
  6aed7f:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6aed84:	49 63 4e 08          	movslq 0x8(%r14),%rcx
  6aed88:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6aed8c:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6aed90:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6aed95:	0f 84 6b 3b 00 00    	je     6b2906 <sqlite3VdbeExec+0x58a6>
  6aed9b:	4c 89 e7             	mov    %r12,%rdi
  6aed9e:	e8 bd 8c 09 00       	call   747a60 <out2PrereleaseWithClear>
  6aeda3:	e9 66 3b 00 00       	jmp    6b290e <sqlite3VdbeExec+0x58ae>
  6aeda8:	49 8b 46 78          	mov    0x78(%r14),%rax
  6aedac:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aedb1:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6aedb5:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6aedb9:	48 8b 40 28          	mov    0x28(%rax),%rax
  6aedbd:	80 48 01 40          	orb    $0x40,0x1(%rax)
  6aedc1:	4c 8b 3c 24          	mov    (%rsp),%r15
  6aedc5:	49 83 c5 18          	add    $0x18,%r13
  6aedc9:	e9 52 e5 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6aedce:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aedd3:	4d 63 7d 04          	movslq 0x4(%r13),%r15
  6aedd7:	4c 89 f0             	mov    %r14,%rax
  6aedda:	4d 63 75 0c          	movslq 0xc(%r13),%r14
  6aedde:	48 8b 40 68          	mov    0x68(%rax),%rax
  6aede2:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6aede6:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6aedea:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6aedee:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6aedf3:	0f 84 86 3b 00 00    	je     6b297f <sqlite3VdbeExec+0x591f>
  6aedf9:	4c 89 e7             	mov    %r12,%rdi
  6aedfc:	e8 5f 8c 09 00       	call   747a60 <out2PrereleaseWithClear>
  6aee01:	e9 81 3b 00 00       	jmp    6b2987 <sqlite3VdbeExec+0x5927>
  6aee06:	49 83 7e 50 00       	cmpq   $0x0,0x50(%r14)
  6aee0b:	0f 8f 39 70 00 00    	jg     6b5e4a <sqlite3VdbeExec+0x8dea>
  6aee11:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6aee18:	00 00 
  6aee1a:	e9 d9 60 00 00       	jmp    6b4ef8 <sqlite3VdbeExec+0x7e98>
  6aee1f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aee24:	4d 8b 4d 10          	mov    0x10(%r13),%r9
  6aee28:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6aee2c:	66 41 83 7d 02 00    	cmpw   $0x0,0x2(%r13)
  6aee32:	4c 8b 3c 24          	mov    (%rsp),%r15
  6aee36:	74 23                	je     6aee5b <sqlite3VdbeExec+0x1dfb>
  6aee38:	49 8b 8e 08 01 00 00 	mov    0x108(%r14),%rcx
  6aee3f:	48 85 c9             	test   %rcx,%rcx
  6aee42:	74 17                	je     6aee5b <sqlite3VdbeExec+0x1dfb>
  6aee44:	49 8b 51 20          	mov    0x20(%r9),%rdx
  6aee48:	48 39 51 30          	cmp    %rdx,0x30(%rcx)
  6aee4c:	0f 84 c5 e4 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6aee52:	48 8b 49 08          	mov    0x8(%rcx),%rcx
  6aee56:	48 85 c9             	test   %rcx,%rcx
  6aee59:	75 ed                	jne    6aee48 <sqlite3VdbeExec+0x1de8>
  6aee5b:	41 8b 8e 18 01 00 00 	mov    0x118(%r14),%ecx
  6aee62:	41 3b 8c 24 b0 00 00 	cmp    0xb0(%r12),%ecx
  6aee69:	00 
  6aee6a:	0f 8d e8 70 00 00    	jge    6b5f58 <sqlite3VdbeExec+0x8ef8>
  6aee70:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6aee74:	4c 8b 7c 24 28       	mov    0x28(%rsp),%r15
  6aee79:	49 01 c7             	add    %rax,%r15
  6aee7c:	41 f6 47 14 10       	testb  $0x10,0x14(%r15)
  6aee81:	0f 85 16 35 00 00    	jne    6b239d <sqlite3VdbeExec+0x533d>
  6aee87:	41 8b 41 10          	mov    0x10(%r9),%eax
  6aee8b:	83 f8 01             	cmp    $0x1,%eax
  6aee8e:	89 c5                	mov    %eax,%ebp
  6aee90:	41 13 69 0c          	adc    0xc(%r9),%ebp
  6aee94:	4c 89 4c 24 28       	mov    %r9,0x28(%rsp)
  6aee99:	41 8b 49 08          	mov    0x8(%r9),%ecx
  6aee9d:	6b d5 38             	imul   $0x38,%ebp,%edx
  6aeea0:	8d 71 07             	lea    0x7(%rcx),%esi
  6aeea3:	83 c1 0e             	add    $0xe,%ecx
  6aeea6:	85 f6                	test   %esi,%esi
  6aeea8:	0f 49 ce             	cmovns %esi,%ecx
  6aeeab:	c1 f9 03             	sar    $0x3,%ecx
  6aeeae:	8d 04 c1             	lea    (%rcx,%rax,8),%eax
  6aeeb1:	44 8d 34 02          	lea    (%rdx,%rax,1),%r14d
  6aeeb5:	41 83 c6 70          	add    $0x70,%r14d
  6aeeb9:	4d 63 ee             	movslq %r14d,%r13
  6aeebc:	4c 89 e7             	mov    %r12,%rdi
  6aeebf:	4c 89 ee             	mov    %r13,%rsi
  6aeec2:	e8 89 b9 fc ff       	call   67a850 <sqlite3DbMallocRawNN>
  6aeec7:	48 85 c0             	test   %rax,%rax
  6aeeca:	0f 84 50 70 00 00    	je     6b5f20 <sqlite3VdbeExec+0x8ec0>
  6aeed0:	49 89 c4             	mov    %rax,%r12
  6aeed3:	48 89 c7             	mov    %rax,%rdi
  6aeed6:	31 f6                	xor    %esi,%esi
  6aeed8:	4c 89 ea             	mov    %r13,%rdx
  6aeedb:	e8 70 b6 fa ff       	call   65a550 <memset$plt>
  6aeee0:	41 f6 47 15 90       	testb  $0x90,0x15(%r15)
  6aeee5:	75 07                	jne    6aeeee <sqlite3VdbeExec+0x1e8e>
  6aeee7:	41 83 7f 20 00       	cmpl   $0x0,0x20(%r15)
  6aeeec:	74 08                	je     6aeef6 <sqlite3VdbeExec+0x1e96>
  6aeeee:	4c 89 ff             	mov    %r15,%rdi
  6aeef1:	e8 fa 1e ff ff       	call   6a0df0 <vdbeMemClear>
  6aeef6:	66 41 c7 47 14 10 10 	movw   $0x1010,0x14(%r15)
  6aeefd:	4d 89 67 08          	mov    %r12,0x8(%r15)
  6aef01:	45 89 77 10          	mov    %r14d,0x10(%r15)
  6aef05:	48 8d 05 d4 60 ff ff 	lea    -0x9f2c(%rip),%rax        # 6a4fe0 <sqlite3VdbeFrameMemDel>
  6aef0c:	49 89 47 30          	mov    %rax,0x30(%r15)
  6aef10:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6aef15:	4d 89 34 24          	mov    %r14,(%r12)
  6aef19:	41 89 6c 24 58       	mov    %ebp,0x58(%r12)
  6aef1e:	4c 8b 4c 24 28       	mov    0x28(%rsp),%r9
  6aef23:	41 8b 49 10          	mov    0x10(%r9),%ecx
  6aef27:	41 89 4c 24 5c       	mov    %ecx,0x5c(%r12)
  6aef2c:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6aef31:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6aef36:	48 c1 e8 03          	shr    $0x3,%rax
  6aef3a:	69 c0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%eax
  6aef40:	41 89 44 24 4c       	mov    %eax,0x4c(%r12)
  6aef45:	49 8b 46 68          	mov    0x68(%r14),%rax
  6aef49:	49 89 44 24 18       	mov    %rax,0x18(%r12)
  6aef4e:	41 8b 46 24          	mov    0x24(%r14),%eax
  6aef52:	41 89 44 24 54       	mov    %eax,0x54(%r12)
  6aef57:	49 8b 46 78          	mov    0x78(%r14),%rax
  6aef5b:	49 89 44 24 20       	mov    %rax,0x20(%r12)
  6aef60:	41 8b 46 28          	mov    0x28(%r14),%eax
  6aef64:	41 89 44 24 48       	mov    %eax,0x48(%r12)
  6aef69:	49 8b 86 88 00 00 00 	mov    0x88(%r14),%rax
  6aef70:	49 89 44 24 10       	mov    %rax,0x10(%r12)
  6aef75:	41 8b 86 90 00 00 00 	mov    0x90(%r14),%eax
  6aef7c:	41 89 44 24 50       	mov    %eax,0x50(%r12)
  6aef81:	49 8b 41 20          	mov    0x20(%r9),%rax
  6aef85:	49 89 44 24 30       	mov    %rax,0x30(%r12)
  6aef8a:	85 ed                	test   %ebp,%ebp
  6aef8c:	0f 84 e1 57 00 00    	je     6b4773 <sqlite3VdbeExec+0x7713>
  6aef92:	48 63 c5             	movslq %ebp,%rax
  6aef95:	48 6b f8 38          	imul   $0x38,%rax,%rdi
  6aef99:	49 8d 74 24 70       	lea    0x70(%r12),%rsi
  6aef9e:	4c 8d 47 c8          	lea    -0x38(%rdi),%r8
  6aefa2:	4c 89 c0             	mov    %r8,%rax
  6aefa5:	48 c1 e8 03          	shr    $0x3,%rax
  6aefa9:	48 ba 93 24 49 92 24 	movabs $0x2492492492492493,%rdx
  6aefb0:	49 92 24 
  6aefb3:	48 f7 e2             	mul    %rdx
  6aefb6:	89 d0                	mov    %edx,%eax
  6aefb8:	f7 d0                	not    %eax
  6aefba:	a8 07                	test   $0x7,%al
  6aefbc:	74 1d                	je     6aefdb <sqlite3VdbeExec+0x1f7b>
  6aefbe:	ff c2                	inc    %edx
  6aefc0:	83 e2 07             	and    $0x7,%edx
  6aefc3:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6aefc8:	66 c7 46 14 00 00    	movw   $0x0,0x14(%rsi)
  6aefce:	48 89 46 18          	mov    %rax,0x18(%rsi)
  6aefd2:	48 83 c6 38          	add    $0x38,%rsi
  6aefd6:	48 ff ca             	dec    %rdx
  6aefd9:	75 ed                	jne    6aefc8 <sqlite3VdbeExec+0x1f68>
  6aefdb:	49 81 f8 88 01 00 00 	cmp    $0x188,%r8
  6aefe2:	0f 82 8d 57 00 00    	jb     6b4775 <sqlite3VdbeExec+0x7715>
  6aefe8:	49 8d 04 3c          	lea    (%r12,%rdi,1),%rax
  6aefec:	48 83 c0 70          	add    $0x70,%rax
  6aeff0:	48 8b 54 24 10       	mov    0x10(%rsp),%rdx
  6aeff5:	66 c7 46 14 00 00    	movw   $0x0,0x14(%rsi)
  6aeffb:	48 89 56 18          	mov    %rdx,0x18(%rsi)
  6aefff:	66 c7 46 4c 00 00    	movw   $0x0,0x4c(%rsi)
  6af005:	48 89 56 50          	mov    %rdx,0x50(%rsi)
  6af009:	66 c7 86 84 00 00 00 	movw   $0x0,0x84(%rsi)
  6af010:	00 00 
  6af012:	48 89 96 88 00 00 00 	mov    %rdx,0x88(%rsi)
  6af019:	66 c7 86 bc 00 00 00 	movw   $0x0,0xbc(%rsi)
  6af020:	00 00 
  6af022:	48 89 96 c0 00 00 00 	mov    %rdx,0xc0(%rsi)
  6af029:	66 c7 86 f4 00 00 00 	movw   $0x0,0xf4(%rsi)
  6af030:	00 00 
  6af032:	48 89 96 f8 00 00 00 	mov    %rdx,0xf8(%rsi)
  6af039:	66 c7 86 2c 01 00 00 	movw   $0x0,0x12c(%rsi)
  6af040:	00 00 
  6af042:	48 89 96 30 01 00 00 	mov    %rdx,0x130(%rsi)
  6af049:	66 c7 86 64 01 00 00 	movw   $0x0,0x164(%rsi)
  6af050:	00 00 
  6af052:	48 89 96 68 01 00 00 	mov    %rdx,0x168(%rsi)
  6af059:	66 c7 86 9c 01 00 00 	movw   $0x0,0x19c(%rsi)
  6af060:	00 00 
  6af062:	48 89 96 a0 01 00 00 	mov    %rdx,0x1a0(%rsi)
  6af069:	48 81 c6 c0 01 00 00 	add    $0x1c0,%rsi
  6af070:	48 39 c6             	cmp    %rax,%rsi
  6af073:	75 80                	jne    6aeff5 <sqlite3VdbeExec+0x1f95>
  6af075:	e9 fb 56 00 00       	jmp    6b4775 <sqlite3VdbeExec+0x7715>
  6af07a:	49 8b 46 78          	mov    0x78(%r14),%rax
  6af07e:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af083:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6af087:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6af08b:	48 8b 48 10          	mov    0x10(%rax),%rcx
  6af08f:	48 8d 51 01          	lea    0x1(%rcx),%rdx
  6af093:	48 89 50 10          	mov    %rdx,0x10(%rax)
  6af097:	48 85 c9             	test   %rcx,%rcx
  6af09a:	e9 85 2a 00 00       	jmp    6b1b24 <sqlite3VdbeExec+0x4ac4>
  6af09f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af0a4:	41 8b 75 04          	mov    0x4(%r13),%esi
  6af0a8:	41 8b 55 08          	mov    0x8(%r13),%edx
  6af0ac:	4c 89 f7             	mov    %r14,%rdi
  6af0af:	b9 01 00 00 00       	mov    $0x1,%ecx
  6af0b4:	e8 d7 7a 00 00       	call   6b6b90 <allocateCursor>
  6af0b9:	48 85 c0             	test   %rax,%rax
  6af0bc:	0f 84 50 6e 00 00    	je     6b5f12 <sqlite3VdbeExec+0x8eb2>
  6af0c2:	49 8b 4d 10          	mov    0x10(%r13),%rcx
  6af0c6:	48 89 48 30          	mov    %rcx,0x30(%rax)
  6af0ca:	41 8b 75 0c          	mov    0xc(%r13),%esi
  6af0ce:	4c 89 e7             	mov    %r12,%rdi
  6af0d1:	48 89 c2             	mov    %rax,%rdx
  6af0d4:	e8 c7 7c 00 00       	call   6b6da0 <sqlite3VdbeSorterInit>
  6af0d9:	85 c0                	test   %eax,%eax
  6af0db:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af0df:	0f 85 ff 72 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6af0e5:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6af0ec:	00 00 
  6af0ee:	49 83 c5 18          	add    $0x18,%r13
  6af0f2:	e9 29 e2 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af0f7:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af0fc:	4d 8b 65 10          	mov    0x10(%r13),%r12
  6af100:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6af104:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af108:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6af10d:	49 39 44 24 10       	cmp    %rax,0x10(%r12)
  6af112:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af116:	74 2e                	je     6af146 <sqlite3VdbeExec+0x20e6>
  6af118:	49 89 44 24 10       	mov    %rax,0x10(%r12)
  6af11d:	41 0f b7 4c 24 2a    	movzwl 0x2a(%r12),%ecx
  6af123:	48 85 c9             	test   %rcx,%rcx
  6af126:	0f 84 18 25 00 00    	je     6b1644 <sqlite3VdbeExec+0x45e4>
  6af12c:	49 63 55 08          	movslq 0x8(%r13),%rdx
  6af130:	48 6b f2 38          	imul   $0x38,%rdx,%rsi
  6af134:	66 83 f9 04          	cmp    $0x4,%cx
  6af138:	0f 83 a9 27 00 00    	jae    6b18e7 <sqlite3VdbeExec+0x4887>
  6af13e:	48 89 ca             	mov    %rcx,%rdx
  6af141:	e9 2e 28 00 00       	jmp    6b1974 <sqlite3VdbeExec+0x4914>
  6af146:	41 0f b7 4c 24 2a    	movzwl 0x2a(%r12),%ecx
  6af14c:	e9 52 28 00 00       	jmp    6b19a3 <sqlite3VdbeExec+0x4943>
  6af151:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af156:	41 8b 45 0c          	mov    0xc(%r13),%eax
  6af15a:	a8 ff                	test   $0xff,%al
  6af15c:	75 0c                	jne    6af16a <sqlite3VdbeExec+0x210a>
  6af15e:	41 f6 44 24 34 04    	testb  $0x4,0x34(%r12)
  6af164:	0f 85 4c 50 00 00    	jne    6b41b6 <sqlite3VdbeExec+0x7156>
  6af16a:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6af16e:	49 8b 54 24 20       	mov    0x20(%r12),%rdx
  6af173:	48 c1 e1 05          	shl    $0x5,%rcx
  6af177:	48 8b 7c 0a 08       	mov    0x8(%rdx,%rcx,1),%rdi
  6af17c:	41 8b 75 08          	mov    0x8(%r13),%esi
  6af180:	0f b6 d0             	movzbl %al,%edx
  6af183:	e8 e8 f7 fe ff       	call   69e970 <sqlite3BtreeLockTable>
  6af188:	85 c0                	test   %eax,%eax
  6af18a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af18e:	0f 85 9f 6d 00 00    	jne    6b5f33 <sqlite3VdbeExec+0x8ed3>
  6af194:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6af19b:	00 00 
  6af19d:	49 83 c5 18          	add    $0x18,%r13
  6af1a1:	e9 7a e1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af1a6:	49 8b 46 78          	mov    0x78(%r14),%rax
  6af1aa:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af1af:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6af1b3:	4c 8b 3c c8          	mov    (%rax,%rcx,8),%r15
  6af1b7:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af1bb:	41 0f b7 45 02       	movzwl 0x2(%r13),%eax
  6af1c0:	a8 01                	test   $0x1,%al
  6af1c2:	74 04                	je     6af1c8 <sqlite3VdbeExec+0x2168>
  6af1c4:	49 ff 46 38          	incq   0x38(%r14)
  6af1c8:	4c 6b e1 38          	imul   $0x38,%rcx,%r12
  6af1cc:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6af1d1:	49 01 d4             	add    %rdx,%r12
  6af1d4:	41 f6 44 24 15 04    	testb  $0x4,0x15(%r12)
  6af1da:	74 1a                	je     6af1f6 <sqlite3VdbeExec+0x2196>
  6af1dc:	4c 89 e7             	mov    %r12,%rdi
  6af1df:	e8 dc 17 ff ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6af1e4:	85 c0                	test   %eax,%eax
  6af1e6:	0f 85 10 6e 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6af1ec:	41 0f b7 45 02       	movzwl 0x2(%r13),%eax
  6af1f1:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6af1f6:	49 63 4c 24 10       	movslq 0x10(%r12),%rcx
  6af1fb:	48 89 4c 24 68       	mov    %rcx,0x68(%rsp)
  6af200:	49 8b 4c 24 08       	mov    0x8(%r12),%rcx
  6af205:	48 89 4c 24 60       	mov    %rcx,0x60(%rsp)
  6af20a:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6af20e:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af212:	48 01 d1             	add    %rdx,%rcx
  6af215:	48 89 4c 24 78       	mov    %rcx,0x78(%rsp)
  6af21a:	41 0f b7 4d 10       	movzwl 0x10(%r13),%ecx
  6af21f:	66 89 8c 24 80 00 00 	mov    %cx,0x80(%rsp)
  6af226:	00 
  6af227:	0f b7 d0             	movzwl %ax,%edx
  6af22a:	31 c9                	xor    %ecx,%ecx
  6af22c:	f6 c2 10             	test   $0x10,%dl
  6af22f:	74 04                	je     6af235 <sqlite3VdbeExec+0x21d5>
  6af231:	41 8b 4f 1c          	mov    0x1c(%r15),%ecx
  6af235:	49 8b 7f 28          	mov    0x28(%r15),%rdi
  6af239:	81 e2 8a 00 00 00    	and    $0x8a,%edx
  6af23f:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6af244:	e8 47 71 fe ff       	call   696390 <sqlite3BtreeInsert>
  6af249:	41 c7 47 18 00 00 00 	movl   $0x0,0x18(%r15)
  6af250:	00 
  6af251:	85 c0                	test   %eax,%eax
  6af253:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6af258:	0f 84 8b 57 00 00    	je     6b49e9 <sqlite3VdbeExec+0x7989>
  6af25e:	e9 56 6c 00 00       	jmp    6b5eb9 <sqlite3VdbeExec+0x8e59>
  6af263:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af268:	45 8b 45 08          	mov    0x8(%r13),%r8d
  6af26c:	49 8b 55 10          	mov    0x10(%r13),%rdx
  6af270:	49 63 45 04          	movslq 0x4(%r13),%rax
  6af274:	4c 6b f0 38          	imul   $0x38,%rax,%r14
  6af278:	4c 8b 7c 24 28       	mov    0x28(%rsp),%r15
  6af27d:	4b 8d 2c 37          	lea    (%r15,%r14,1),%rbp
  6af281:	4c 89 e7             	mov    %r12,%rdi
  6af284:	4f 8d 24 37          	lea    (%r15,%r14,1),%r12
  6af288:	49 83 c4 38          	add    $0x38,%r12
  6af28c:	48 8b 47 20          	mov    0x20(%rdi),%rax
  6af290:	41 0f b7 4d 02       	movzwl 0x2(%r13),%ecx
  6af295:	c1 e1 05             	shl    $0x5,%ecx
  6af298:	48 8b 74 08 08       	mov    0x8(%rax,%rcx,1),%rsi
  6af29d:	48 83 c2 04          	add    $0x4,%rdx
  6af2a1:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6af2a5:	48 6b c8 38          	imul   $0x38,%rax,%rcx
  6af2a9:	4c 01 f9             	add    %r15,%rcx
  6af2ac:	47 8b 0c 37          	mov    (%r15,%r14,1),%r9d
  6af2b0:	41 ff c1             	inc    %r9d
  6af2b3:	48 8d 44 24 60       	lea    0x60(%rsp),%rax
  6af2b8:	4c 8d 94 24 e8 00 00 	lea    0xe8(%rsp),%r10
  6af2bf:	00 
  6af2c0:	50                   	push   %rax
  6af2c1:	41 52                	push   %r10
  6af2c3:	e8 48 dc fe ff       	call   69cf10 <sqlite3BtreeIntegrityCheck>
  6af2c8:	48 83 c4 10          	add    $0x10,%rsp
  6af2cc:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6af2d1:	43 f6 44 37 4d 90    	testb  $0x90,0x4d(%r15,%r14,1)
  6af2d7:	0f 84 04 37 00 00    	je     6b29e1 <sqlite3VdbeExec+0x5981>
  6af2dd:	4c 89 e7             	mov    %r12,%rdi
  6af2e0:	e8 5b 22 ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6af2e5:	e9 fd 36 00 00       	jmp    6b29e7 <sqlite3VdbeExec+0x5987>
  6af2ea:	49 8b 46 68          	mov    0x68(%r14),%rax
  6af2ee:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af2f3:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af2f7:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af2fb:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6af2ff:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6af304:	0f 84 68 37 00 00    	je     6b2a72 <sqlite3VdbeExec+0x5a12>
  6af30a:	4c 89 e7             	mov    %r12,%rdi
  6af30d:	e8 4e 87 09 00       	call   747a60 <out2PrereleaseWithClear>
  6af312:	e9 63 37 00 00       	jmp    6b2a7a <sqlite3VdbeExec+0x5a1a>
  6af317:	4c 8b 2c 24          	mov    (%rsp),%r13
  6af31b:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6af320:	8b 69 0c             	mov    0xc(%rcx),%ebp
  6af323:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6af327:	48 63 49 08          	movslq 0x8(%rcx),%rcx
  6af32b:	4c 6b e1 38          	imul   $0x38,%rcx,%r12
  6af32f:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6af334:	49 01 cc             	add    %rcx,%r12
  6af337:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af33b:	4c 8d 3c 01          	lea    (%rcx,%rax,1),%r15
  6af33f:	49 83 c7 20          	add    $0x20,%r15
  6af343:	eb 1b                	jmp    6af360 <sqlite3VdbeExec+0x2300>
  6af345:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  6af34c:	00 00 00 00 
  6af350:	49 83 c4 38          	add    $0x38,%r12
  6af354:	49 83 c7 38          	add    $0x38,%r15
  6af358:	ff cd                	dec    %ebp
  6af35a:	0f 84 da 24 00 00    	je     6b183a <sqlite3VdbeExec+0x47da>
  6af360:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6af366:	75 08                	jne    6af370 <sqlite3VdbeExec+0x2310>
  6af368:	41 83 7c 24 20 00    	cmpl   $0x0,0x20(%r12)
  6af36e:	74 08                	je     6af378 <sqlite3VdbeExec+0x2318>
  6af370:	4c 89 e7             	mov    %r12,%rdi
  6af373:	e8 78 1a ff ff       	call   6a0df0 <vdbeMemClear>
  6af378:	49 8d 47 e0          	lea    -0x20(%r15),%rax
  6af37c:	48 8b 48 30          	mov    0x30(%rax),%rcx
  6af380:	49 89 4c 24 30       	mov    %rcx,0x30(%r12)
  6af385:	66 0f 10 00          	movupd (%rax),%xmm0
  6af389:	66 0f 10 48 10       	movupd 0x10(%rax),%xmm1
  6af38e:	f3 0f 6f 50 20       	movdqu 0x20(%rax),%xmm2
  6af393:	f3 41 0f 7f 54 24 20 	movdqu %xmm2,0x20(%r12)
  6af39a:	66 41 0f 11 4c 24 10 	movupd %xmm1,0x10(%r12)
  6af3a1:	66 41 0f 11 04 24    	movupd %xmm0,(%r12)
  6af3a7:	66 41 c7 47 f4 01 00 	movw   $0x1,-0xc(%r15)
  6af3ae:	41 c7 07 00 00 00 00 	movl   $0x0,(%r15)
  6af3b5:	41 f6 44 24 15 40    	testb  $0x40,0x15(%r12)
  6af3bb:	74 93                	je     6af350 <sqlite3VdbeExec+0x22f0>
  6af3bd:	4c 89 e7             	mov    %r12,%rdi
  6af3c0:	e8 6b 0a fd ff       	call   67fe30 <sqlite3VdbeMemMakeWriteable>
  6af3c5:	85 c0                	test   %eax,%eax
  6af3c7:	74 87                	je     6af350 <sqlite3VdbeExec+0x22f0>
  6af3c9:	e9 07 66 00 00       	jmp    6b59d5 <sqlite3VdbeExec+0x8975>
  6af3ce:	41 ff 84 24 e0 00 00 	incl   0xe0(%r12)
  6af3d5:	00 
  6af3d6:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af3db:	41 8b 75 04          	mov    0x4(%r13),%esi
  6af3df:	49 8b 55 10          	mov    0x10(%r13),%rdx
  6af3e3:	4c 89 e7             	mov    %r12,%rdi
  6af3e6:	e8 85 ab 00 00       	call   6b9f70 <sqlite3VtabCallDestroy>
  6af3eb:	41 ff 8c 24 e0 00 00 	decl   0xe0(%r12)
  6af3f2:	00 
  6af3f3:	85 c0                	test   %eax,%eax
  6af3f5:	0f 84 ee 55 00 00    	je     6b49e9 <sqlite3VdbeExec+0x7989>
  6af3fb:	e9 b9 6a 00 00       	jmp    6b5eb9 <sqlite3VdbeExec+0x8e59>
  6af400:	49 8b 46 68          	mov    0x68(%r14),%rax
  6af404:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af409:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af40d:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af411:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6af415:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6af41a:	0f 84 82 36 00 00    	je     6b2aa2 <sqlite3VdbeExec+0x5a42>
  6af420:	4c 89 e7             	mov    %r12,%rdi
  6af423:	e8 38 86 09 00       	call   747a60 <out2PrereleaseWithClear>
  6af428:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6af42e:	0f 94 c0             	sete   %al
  6af431:	e9 76 36 00 00       	jmp    6b2aac <sqlite3VdbeExec+0x5a4c>
  6af436:	49 8b 46 68          	mov    0x68(%r14),%rax
  6af43a:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af43f:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af443:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af447:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6af44b:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6af450:	0f 84 e0 36 00 00    	je     6b2b36 <sqlite3VdbeExec+0x5ad6>
  6af456:	4c 89 e7             	mov    %r12,%rdi
  6af459:	e8 02 86 09 00       	call   747a60 <out2PrereleaseWithClear>
  6af45e:	e9 db 36 00 00       	jmp    6b2b3e <sqlite3VdbeExec+0x5ade>
  6af463:	0f 57 c0             	xorps  %xmm0,%xmm0
  6af466:	0f 29 44 24 70       	movaps %xmm0,0x70(%rsp)
  6af46b:	0f 29 84 24 80 00 00 	movaps %xmm0,0x80(%rsp)
  6af472:	00 
  6af473:	0f 29 44 24 60       	movaps %xmm0,0x60(%rsp)
  6af478:	48 c7 84 24 90 00 00 	movq   $0x0,0x90(%rsp)
  6af47f:	00 00 00 00 00 
  6af484:	4c 89 64 24 78       	mov    %r12,0x78(%rsp)
  6af489:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af48e:	49 63 45 08          	movslq 0x8(%r13),%rax
  6af492:	48 6b c8 38          	imul   $0x38,%rax,%rcx
  6af496:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6af49b:	48 8b 44 0e 10       	mov    0x10(%rsi,%rcx,1),%rax
  6af4a0:	48 89 44 24 70       	mov    %rax,0x70(%rsp)
  6af4a5:	66 0f 10 04 0e       	movupd (%rsi,%rcx,1),%xmm0
  6af4aa:	66 0f 29 44 24 60    	movapd %xmm0,0x60(%rsp)
  6af4b0:	0f b7 54 24 74       	movzwl 0x74(%rsp),%edx
  6af4b5:	89 d0                	mov    %edx,%eax
  6af4b7:	25 ff ef 00 00       	and    $0xefff,%eax
  6af4bc:	66 89 44 24 74       	mov    %ax,0x74(%rsp)
  6af4c1:	45 31 ff             	xor    %r15d,%r15d
  6af4c4:	f6 c2 12             	test   $0x12,%dl
  6af4c7:	74 25                	je     6af4ee <sqlite3VdbeExec+0x248e>
  6af4c9:	48 01 f1             	add    %rsi,%rcx
  6af4cc:	f6 41 15 20          	testb  $0x20,0x15(%rcx)
  6af4d0:	75 1c                	jne    6af4ee <sqlite3VdbeExec+0x248e>
  6af4d2:	0d 00 40 00 00       	or     $0x4000,%eax
  6af4d7:	66 89 44 24 74       	mov    %ax,0x74(%rsp)
  6af4dc:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6af4e1:	e8 4a 09 fd ff       	call   67fe30 <sqlite3VdbeMemMakeWriteable>
  6af4e6:	41 89 c7             	mov    %eax,%r15d
  6af4e9:	0f b7 44 24 74       	movzwl 0x74(%rsp),%eax
  6af4ee:	89 c1                	mov    %eax,%ecx
  6af4f0:	f7 d1                	not    %ecx
  6af4f2:	f7 c1 02 02 00 00    	test   $0x202,%ecx
  6af4f8:	0f 85 35 26 00 00    	jne    6b1b33 <sqlite3VdbeExec+0x4ad3>
  6af4fe:	80 7c 24 76 01       	cmpb   $0x1,0x76(%rsp)
  6af503:	0f 85 2a 26 00 00    	jne    6b1b33 <sqlite3VdbeExec+0x4ad3>
  6af509:	48 8b 54 24 68       	mov    0x68(%rsp),%rdx
  6af50e:	e9 3a 26 00 00       	jmp    6b1b4d <sqlite3VdbeExec+0x4aed>
  6af513:	49 8b 46 78          	mov    0x78(%r14),%rax
  6af517:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af51c:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6af520:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6af524:	0f b7 48 06          	movzwl 0x6(%rax),%ecx
  6af528:	41 8b 55 08          	mov    0x8(%r13),%edx
  6af52c:	39 ca                	cmp    %ecx,%edx
  6af52e:	0f 8e 3e 26 00 00    	jle    6b1b72 <sqlite3VdbeExec+0x4b12>
  6af534:	66 89 50 06          	mov    %dx,0x6(%rax)
  6af538:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af53c:	49 83 c5 18          	add    $0x18,%r13
  6af540:	e9 db dd ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af545:	4d 89 f7             	mov    %r14,%r15
  6af548:	49 8b 46 68          	mov    0x68(%r14),%rax
  6af54c:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6af551:	49 63 4e 08          	movslq 0x8(%r14),%rcx
  6af555:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af559:	48 8d 3c 08          	lea    (%rax,%rcx,1),%rdi
  6af55d:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6af562:	48 89 7c 24 38       	mov    %rdi,0x38(%rsp)
  6af567:	0f 84 31 36 00 00    	je     6b2b9e <sqlite3VdbeExec+0x5b3e>
  6af56d:	e8 ee 84 09 00       	call   747a60 <out2PrereleaseWithClear>
  6af572:	e9 2d 36 00 00       	jmp    6b2ba4 <sqlite3VdbeExec+0x5b44>
  6af577:	49 8b 46 78          	mov    0x78(%r14),%rax
  6af57b:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af580:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6af584:	48 8b 34 c8          	mov    (%rax,%rcx,8),%rsi
  6af588:	48 85 f6             	test   %rsi,%rsi
  6af58b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af58f:	0f 84 5b 41 00 00    	je     6b36f0 <sqlite3VdbeExec+0x6690>
  6af595:	f6 46 05 10          	testb  $0x10,0x5(%rsi)
  6af599:	0f 85 49 41 00 00    	jne    6b36e8 <sqlite3VdbeExec+0x6688>
  6af59f:	0f b6 06             	movzbl (%rsi),%eax
  6af5a2:	85 c0                	test   %eax,%eax
  6af5a4:	0f 84 75 59 00 00    	je     6b4f1f <sqlite3VdbeExec+0x7ebf>
  6af5aa:	83 f8 02             	cmp    $0x2,%eax
  6af5ad:	0f 84 57 59 00 00    	je     6b4f0a <sqlite3VdbeExec+0x7eaa>
  6af5b3:	83 f8 01             	cmp    $0x1,%eax
  6af5b6:	0f 85 34 41 00 00    	jne    6b36f0 <sqlite3VdbeExec+0x6690>
  6af5bc:	49 8b 3e             	mov    (%r14),%rdi
  6af5bf:	e8 8c 6c ff ff       	call   6a6250 <sqlite3VdbeSorterClose>
  6af5c4:	e9 27 41 00 00       	jmp    6b36f0 <sqlite3VdbeExec+0x6690>
  6af5c9:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6af5d0:	00 00 
  6af5d2:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af5d7:	49 63 45 08          	movslq 0x8(%r13),%rax
  6af5db:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af5df:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6af5e4:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6af5e8:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6af5ed:	0f 84 56 36 00 00    	je     6b2c49 <sqlite3VdbeExec+0x5be9>
  6af5f3:	4c 89 e7             	mov    %r12,%rdi
  6af5f6:	e8 45 1f ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6af5fb:	e9 51 36 00 00       	jmp    6b2c51 <sqlite3VdbeExec+0x5bf1>
  6af600:	49 8b 46 68          	mov    0x68(%r14),%rax
  6af604:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6af609:	48 63 49 08          	movslq 0x8(%rcx),%rcx
  6af60d:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af611:	4c 8d 2c 08          	lea    (%rax,%rcx,1),%r13
  6af615:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6af61a:	0f 84 c4 36 00 00    	je     6b2ce4 <sqlite3VdbeExec+0x5c84>
  6af620:	4c 89 ef             	mov    %r13,%rdi
  6af623:	e8 38 84 09 00       	call   747a60 <out2PrereleaseWithClear>
  6af628:	e9 be 36 00 00       	jmp    6b2ceb <sqlite3VdbeExec+0x5c8b>
  6af62d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af632:	49 63 45 04          	movslq 0x4(%r13),%rax
  6af636:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af63a:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af63e:	48 6b f9 38          	imul   $0x38,%rcx,%rdi
  6af642:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6af647:	48 01 cf             	add    %rcx,%rdi
  6af64a:	f6 44 01 15 08       	testb  $0x8,0x15(%rcx,%rax,1)
  6af64f:	0f 85 3a 25 00 00    	jne    6b1b8f <sqlite3VdbeExec+0x4b2f>
  6af655:	f6 47 15 90          	testb  $0x90,0x15(%rdi)
  6af659:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af65d:	0f 84 60 4b 00 00    	je     6b41c3 <sqlite3VdbeExec+0x7163>
  6af663:	e8 d8 1e ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6af668:	49 83 c5 18          	add    $0x18,%r13
  6af66c:	e9 af dc ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af671:	49 8b 46 78          	mov    0x78(%r14),%rax
  6af675:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af67a:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6af67e:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6af682:	48 8b 40 28          	mov    0x28(%rax),%rax
  6af686:	80 60 01 bf          	andb   $0xbf,0x1(%rax)
  6af68a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af68e:	49 83 c5 18          	add    $0x18,%r13
  6af692:	e9 89 dc ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af697:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af69c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6af6a0:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af6a4:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6af6a9:	66 81 64 01 14 ff f7 	andw   $0xf7ff,0x14(%rcx,%rax,1)
  6af6b0:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af6b4:	49 83 c5 18          	add    $0x18,%r13
  6af6b8:	e9 63 dc ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af6bd:	49 8b 46 78          	mov    0x78(%r14),%rax
  6af6c1:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6af6c6:	48 63 4a 04          	movslq 0x4(%rdx),%rcx
  6af6ca:	4c 8b 2c c8          	mov    (%rax,%rcx,8),%r13
  6af6ce:	48 63 42 0c          	movslq 0xc(%rdx),%rax
  6af6d2:	4c 6b e0 38          	imul   $0x38,%rax,%r12
  6af6d6:	4c 03 64 24 28       	add    0x28(%rsp),%r12
  6af6db:	41 80 7d 02 00       	cmpb   $0x0,0x2(%r13)
  6af6e0:	0f 84 9c 36 00 00    	je     6b2d82 <sqlite3VdbeExec+0x5d22>
  6af6e6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af6ea:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6af6f0:	0f 84 0d 46 00 00    	je     6b3d03 <sqlite3VdbeExec+0x6ca3>
  6af6f6:	4c 89 e7             	mov    %r12,%rdi
  6af6f9:	e8 42 1e ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6af6fe:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af703:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6af708:	49 83 c5 18          	add    $0x18,%r13
  6af70c:	e9 0f dc ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af711:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af716:	49 63 45 04          	movslq 0x4(%r13),%rax
  6af71a:	48 6b c8 38          	imul   $0x38,%rax,%rcx
  6af71e:	49 63 45 08          	movslq 0x8(%r13),%rax
  6af722:	48 6b d0 38          	imul   $0x38,%rax,%rdx
  6af726:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6af72b:	48 8d 04 16          	lea    (%rsi,%rdx,1),%rax
  6af72f:	f6 44 0e 14 01       	testb  $0x1,0x14(%rsi,%rcx,1)
  6af734:	0f b7 54 16 14       	movzwl 0x14(%rsi,%rdx,1),%edx
  6af739:	0f 85 73 24 00 00    	jne    6b1bb2 <sqlite3VdbeExec+0x4b52>
  6af73f:	48 01 f1             	add    %rsi,%rcx
  6af742:	81 ca 00 08 00 00    	or     $0x800,%edx
  6af748:	66 89 50 14          	mov    %dx,0x14(%rax)
  6af74c:	0f b6 09             	movzbl (%rcx),%ecx
  6af74f:	88 48 17             	mov    %cl,0x17(%rax)
  6af752:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af756:	49 83 c5 18          	add    $0x18,%r13
  6af75a:	e9 c1 db ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af75f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af764:	49 63 4d 10          	movslq 0x10(%r13),%rcx
  6af768:	48 85 c9             	test   %rcx,%rcx
  6af76b:	0f 8e 95 36 00 00    	jle    6b2e06 <sqlite3VdbeExec+0x5da6>
  6af771:	48 ba ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rdx
  6af778:	ff ff 7f 
  6af77b:	49 63 75 0c          	movslq 0xc(%r13),%rsi
  6af77f:	48 01 f1             	add    %rsi,%rcx
  6af782:	48 6b fe 38          	imul   $0x38,%rsi,%rdi
  6af786:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6af78b:	31 c0                	xor    %eax,%eax
  6af78d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af791:	eb 20                	jmp    6af7b3 <sqlite3VdbeExec+0x2753>
  6af793:	66 66 66 66 2e 0f 1f 	data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
  6af79a:	84 00 00 00 00 00 
  6af7a0:	48 03 07             	add    (%rdi),%rax
  6af7a3:	48 ff c6             	inc    %rsi
  6af7a6:	48 83 c7 38          	add    $0x38,%rdi
  6af7aa:	48 39 ce             	cmp    %rcx,%rsi
  6af7ad:	0f 8d 59 36 00 00    	jge    6b2e0c <sqlite3VdbeExec+0x5dac>
  6af7b3:	44 0f b7 47 14       	movzwl 0x14(%rdi),%r8d
  6af7b8:	41 f6 c0 24          	test   $0x24,%r8b
  6af7bc:	75 e2                	jne    6af7a0 <sqlite3VdbeExec+0x2740>
  6af7be:	41 f6 c0 08          	test   $0x8,%r8b
  6af7c2:	75 11                	jne    6af7d5 <sqlite3VdbeExec+0x2775>
  6af7c4:	49 83 e0 12          	and    $0x12,%r8
  6af7c8:	74 d9                	je     6af7a3 <sqlite3VdbeExec+0x2743>
  6af7ca:	4c 01 c0             	add    %r8,%rax
  6af7cd:	48 05 fd 0f 00 00    	add    $0xffd,%rax
  6af7d3:	eb ce                	jmp    6af7a3 <sqlite3VdbeExec+0x2743>
  6af7d5:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6af7d9:	f2 0f 10 0d 4f bd f9 	movsd  -0x642b1(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6af7e0:	ff 
  6af7e1:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6af7e5:	76 09                	jbe    6af7f0 <sqlite3VdbeExec+0x2790>
  6af7e7:	4c 8d 42 01          	lea    0x1(%rdx),%r8
  6af7eb:	4c 01 c0             	add    %r8,%rax
  6af7ee:	eb b3                	jmp    6af7a3 <sqlite3VdbeExec+0x2743>
  6af7f0:	66 0f 2e 05 50 be f9 	ucomisd -0x641b0(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6af7f7:	ff 
  6af7f8:	49 89 d0             	mov    %rdx,%r8
  6af7fb:	77 05                	ja     6af802 <sqlite3VdbeExec+0x27a2>
  6af7fd:	f2 4c 0f 2c c0       	cvttsd2si %xmm0,%r8
  6af802:	4c 01 c0             	add    %r8,%rax
  6af805:	eb 9c                	jmp    6af7a3 <sqlite3VdbeExec+0x2743>
  6af807:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af80c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6af810:	48 85 c0             	test   %rax,%rax
  6af813:	0f 88 3c 36 00 00    	js     6b2e55 <sqlite3VdbeExec+0x5df5>
  6af819:	49 8b 4e 78          	mov    0x78(%r14),%rcx
  6af81d:	48 8b 04 c1          	mov    (%rcx,%rax,8),%rax
  6af821:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6af825:	0f b7 50 42          	movzwl 0x42(%rax),%edx
  6af829:	39 d1                	cmp    %edx,%ecx
  6af82b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af82f:	0f 8d d4 3e 00 00    	jge    6b3709 <sqlite3VdbeExec+0x66a9>
  6af835:	8b 44 88 70          	mov    0x70(%rax,%rcx,4),%eax
  6af839:	48 83 f8 0c          	cmp    $0xc,%rax
  6af83d:	0f 82 ff 4e 00 00    	jb     6b4742 <sqlite3VdbeExec+0x76e2>
  6af843:	31 c9                	xor    %ecx,%ecx
  6af845:	a8 01                	test   $0x1,%al
  6af847:	0f 94 c1             	sete   %cl
  6af84a:	8d 04 8d 04 00 00 00 	lea    0x4(,%rcx,4),%eax
  6af851:	e9 f7 4e 00 00       	jmp    6b474d <sqlite3VdbeExec+0x76ed>
  6af856:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af85b:	41 83 7d 04 00       	cmpl   $0x0,0x4(%r13)
  6af860:	0f 84 21 36 00 00    	je     6b2e87 <sqlite3VdbeExec+0x5e27>
  6af866:	41 8b 45 08          	mov    0x8(%r13),%eax
  6af86a:	ff c0                	inc    %eax
  6af86c:	41 8b 8e c8 00 00 00 	mov    0xc8(%r14),%ecx
  6af873:	83 e0 03             	and    $0x3,%eax
  6af876:	83 e1 fc             	and    $0xfffffffc,%ecx
  6af879:	09 c1                	or     %eax,%ecx
  6af87b:	66 41 89 8e c8 00 00 	mov    %cx,0xc8(%r14)
  6af882:	00 
  6af883:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af887:	49 83 c5 18          	add    $0x18,%r13
  6af88b:	e9 90 da ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af890:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6af897:	00 00 
  6af899:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6af89e:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af8a3:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af8a7:	48 c1 e1 05          	shl    $0x5,%rcx
  6af8ab:	48 8b 7c 08 08       	mov    0x8(%rax,%rcx,1),%rdi
  6af8b0:	41 8b 75 04          	mov    0x4(%r13),%esi
  6af8b4:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6af8b9:	e8 82 cc fe ff       	call   69c540 <sqlite3BtreeClearTable>
  6af8be:	41 8b 55 0c          	mov    0xc(%r13),%edx
  6af8c2:	48 85 d2             	test   %rdx,%rdx
  6af8c5:	74 1a                	je     6af8e1 <sqlite3VdbeExec+0x2881>
  6af8c7:	48 8b 4c 24 60       	mov    0x60(%rsp),%rcx
  6af8cc:	49 01 4e 38          	add    %rcx,0x38(%r14)
  6af8d0:	85 d2                	test   %edx,%edx
  6af8d2:	7e 0d                	jle    6af8e1 <sqlite3VdbeExec+0x2881>
  6af8d4:	48 6b f2 38          	imul   $0x38,%rdx,%rsi
  6af8d8:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6af8dd:	48 01 0c 32          	add    %rcx,(%rdx,%rsi,1)
  6af8e1:	85 c0                	test   %eax,%eax
  6af8e3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6af8e7:	0f 85 f7 6a 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6af8ed:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6af8f4:	00 00 
  6af8f6:	49 83 c5 18          	add    $0x18,%r13
  6af8fa:	e9 21 da ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6af8ff:	49 8b 46 68          	mov    0x68(%r14),%rax
  6af903:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af908:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6af90c:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6af910:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6af914:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6af919:	0f 84 a6 35 00 00    	je     6b2ec5 <sqlite3VdbeExec+0x5e65>
  6af91f:	4c 89 e7             	mov    %r12,%rdi
  6af922:	e8 39 81 09 00       	call   747a60 <out2PrereleaseWithClear>
  6af927:	e9 a1 35 00 00       	jmp    6b2ecd <sqlite3VdbeExec+0x5e6d>
  6af92c:	4c 89 e7             	mov    %r12,%rdi
  6af92f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af934:	4d 8b 65 10          	mov    0x10(%r13),%r12
  6af938:	4c 89 e6             	mov    %r12,%rsi
  6af93b:	e8 30 a2 00 00       	call   6b9b70 <sqlite3VtabBegin>
  6af940:	4d 85 e4             	test   %r12,%r12
  6af943:	74 19                	je     6af95e <sqlite3VdbeExec+0x28fe>
  6af945:	49 8b 74 24 10       	mov    0x10(%r12),%rsi
  6af94a:	4c 89 f7             	mov    %r14,%rdi
  6af94d:	4d 89 f7             	mov    %r14,%r15
  6af950:	49 89 c6             	mov    %rax,%r14
  6af953:	e8 b8 a1 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6af958:	4c 89 f0             	mov    %r14,%rax
  6af95b:	4d 89 fe             	mov    %r15,%r14
  6af95e:	85 c0                	test   %eax,%eax
  6af960:	0f 84 cd 22 00 00    	je     6b1c33 <sqlite3VdbeExec+0x4bd3>
  6af966:	e9 91 66 00 00       	jmp    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6af96b:	4d 8b 74 24 30       	mov    0x30(%r12),%r14
  6af970:	4c 89 f0             	mov    %r14,%rax
  6af973:	48 0d 00 00 00 04    	or     $0x4000000,%rax
  6af979:	49 89 44 24 30       	mov    %rax,0x30(%r12)
  6af97e:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6af983:	48 8b 41 10          	mov    0x10(%rcx),%rax
  6af987:	4c 8b 60 10          	mov    0x10(%rax),%r12
  6af98b:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6af98f:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af993:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6af998:	4c 8d 2c 01          	lea    (%rcx,%rax,1),%r13
  6af99c:	f6 44 01 14 02       	testb  $0x2,0x14(%rcx,%rax,1)
  6af9a1:	0f 85 22 22 00 00    	jne    6b1bc9 <sqlite3VdbeExec+0x4b69>
  6af9a7:	41 c6 45 16 01       	movb   $0x1,0x16(%r13)
  6af9ac:	e9 34 22 00 00       	jmp    6b1be5 <sqlite3VdbeExec+0x4b85>
  6af9b1:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6af9b6:	49 63 45 04          	movslq 0x4(%r13),%rax
  6af9ba:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6af9be:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6af9c3:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6af9c7:	4d 63 75 08          	movslq 0x8(%r13),%r14
  6af9cb:	f6 44 01 14 10       	testb  $0x10,0x14(%rcx,%rax,1)
  6af9d0:	75 10                	jne    6af9e2 <sqlite3VdbeExec+0x2982>
  6af9d2:	4c 89 e7             	mov    %r12,%rdi
  6af9d5:	e8 f6 1c ff ff       	call   6a16d0 <sqlite3VdbeMemSetRowSet>
  6af9da:	85 c0                	test   %eax,%eax
  6af9dc:	0f 85 26 65 00 00    	jne    6b5f08 <sqlite3VdbeExec+0x8ea8>
  6af9e2:	49 6b c6 38          	imul   $0x38,%r14,%rax
  6af9e6:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6af9eb:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6af9f0:	48 8b 30             	mov    (%rax),%rsi
  6af9f3:	e8 88 89 fd ff       	call   688380 <sqlite3RowSetInsert>
  6af9f8:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6af9fd:	e9 3a 22 00 00       	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6afa02:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6afa09:	00 00 
  6afa0b:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afa10:	49 8b 45 10          	mov    0x10(%r13),%rax
  6afa14:	4c 8b 60 10          	mov    0x10(%rax),%r12
  6afa18:	b8 06 00 00 00       	mov    $0x6,%eax
  6afa1d:	4d 85 e4             	test   %r12,%r12
  6afa20:	0f 84 d1 65 00 00    	je     6b5ff7 <sqlite3VdbeExec+0x8f97>
  6afa26:	4d 8b 34 24          	mov    (%r12),%r14
  6afa2a:	4d 85 f6             	test   %r14,%r14
  6afa2d:	0f 84 c4 65 00 00    	je     6b5ff7 <sqlite3VdbeExec+0x8f97>
  6afa33:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6afa38:	4c 89 e7             	mov    %r12,%rdi
  6afa3b:	41 ff 56 30          	call   *0x30(%r14)
  6afa3f:	89 c5                	mov    %eax,%ebp
  6afa41:	4c 8b 7c 24 30       	mov    0x30(%rsp),%r15
  6afa46:	4c 89 ff             	mov    %r15,%rdi
  6afa49:	4c 89 e6             	mov    %r12,%rsi
  6afa4c:	e8 bf a0 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6afa51:	48 89 e8             	mov    %rbp,%rax
  6afa54:	85 c0                	test   %eax,%eax
  6afa56:	0f 85 9b 65 00 00    	jne    6b5ff7 <sqlite3VdbeExec+0x8f97>
  6afa5c:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
  6afa61:	4c 89 20             	mov    %r12,(%rax)
  6afa64:	41 8b 75 04          	mov    0x4(%r13),%esi
  6afa68:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6afa6f:	00 00 
  6afa71:	4c 89 ff             	mov    %r15,%rdi
  6afa74:	31 d2                	xor    %edx,%edx
  6afa76:	b9 02 00 00 00       	mov    $0x2,%ecx
  6afa7b:	e8 10 71 00 00       	call   6b6b90 <allocateCursor>
  6afa80:	48 85 c0             	test   %rax,%rax
  6afa83:	0f 84 76 64 00 00    	je     6b5eff <sqlite3VdbeExec+0x8e9f>
  6afa89:	48 8b 4c 24 60       	mov    0x60(%rsp),%rcx
  6afa8e:	48 89 48 28          	mov    %rcx,0x28(%rax)
  6afa92:	41 ff 44 24 08       	incl   0x8(%r12)
  6afa97:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6afa9c:	e9 9b 21 00 00       	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6afaa1:	49 8b 46 78          	mov    0x78(%r14),%rax
  6afaa5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afaaa:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6afaae:	4c 8b 34 c8          	mov    (%rax,%rcx,8),%r14
  6afab2:	e8 79 98 fc ff       	call   679330 <sqlite3_initialize>
  6afab7:	85 c0                	test   %eax,%eax
  6afab9:	0f 85 14 60 00 00    	jne    6b5ad3 <sqlite3VdbeExec+0x8a73>
  6afabf:	bf 10 00 00 00       	mov    $0x10,%edi
  6afac4:	e8 17 a8 fc ff       	call   67a2e0 <sqlite3Malloc>
  6afac9:	48 85 c0             	test   %rax,%rax
  6afacc:	4c 8b 3c 24          	mov    (%rsp),%r15
  6afad0:	0f 84 93 60 00 00    	je     6b5b69 <sqlite3VdbeExec+0x8b09>
  6afad6:	49 89 c4             	mov    %rax,%r12
  6afad9:	49 8b 46 28          	mov    0x28(%r14),%rax
  6afadd:	49 89 04 24          	mov    %rax,(%r12)
  6afae1:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6afae5:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6afae9:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6afaee:	49 89 44 24 08       	mov    %rax,0x8(%r12)
  6afaf3:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6afaf8:	49 8b 46 68          	mov    0x68(%r14),%rax
  6afafc:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6afb00:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6afb04:	4c 8d 2c 08          	lea    (%rax,%rcx,1),%r13
  6afb08:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6afb0d:	0f 84 77 3a 00 00    	je     6b358a <sqlite3VdbeExec+0x652a>
  6afb13:	4c 89 ef             	mov    %r13,%rdi
  6afb16:	e8 45 7f 09 00       	call   747a60 <out2PrereleaseWithClear>
  6afb1b:	e9 71 3a 00 00       	jmp    6b3591 <sqlite3VdbeExec+0x6531>
  6afb20:	48 8b 74 24 08       	mov    0x8(%rsp),%rsi
  6afb25:	48 63 46 08          	movslq 0x8(%rsi),%rax
  6afb29:	49 8b 4e 78          	mov    0x78(%r14),%rcx
  6afb2d:	48 63 56 04          	movslq 0x4(%rsi),%rdx
  6afb31:	4c 8b 3c d1          	mov    (%rcx,%rdx,8),%r15
  6afb35:	48 63 4e 0c          	movslq 0xc(%rsi),%rcx
  6afb39:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6afb3d:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6afb42:	4c 8b 24 0a          	mov    (%rdx,%rcx,1),%r12
  6afb46:	4c 89 64 24 68       	mov    %r12,0x68(%rsp)
  6afb4b:	80 7e 01 fb          	cmpb   $0xfb,0x1(%rsi)
  6afb4f:	0f 85 f6 1a 00 00    	jne    6b164b <sqlite3VdbeExec+0x45eb>
  6afb55:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
  6afb5a:	48 83 b9 38 01 00 00 	cmpq   $0x0,0x138(%rcx)
  6afb61:	00 
  6afb62:	0f 84 e3 1a 00 00    	je     6b164b <sqlite3VdbeExec+0x45eb>
  6afb68:	48 8b 49 20          	mov    0x20(%rcx),%rcx
  6afb6c:	49 0f be 57 01       	movsbq 0x1(%r15),%rdx
  6afb71:	48 c1 e2 05          	shl    $0x5,%rdx
  6afb75:	4c 8b 2c 11          	mov    (%rcx,%rdx,1),%r13
  6afb79:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6afb7e:	48 8b 69 10          	mov    0x10(%rcx),%rbp
  6afb82:	e9 c9 1a 00 00       	jmp    6b1650 <sqlite3VdbeExec+0x45f0>
  6afb87:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6afb8e:	00 00 
  6afb90:	c7 84 24 e8 00 00 00 	movl   $0x0,0xe8(%rsp)
  6afb97:	00 00 00 00 
  6afb9b:	4d 89 f5             	mov    %r14,%r13
  6afb9e:	49 8b 46 68          	mov    0x68(%r14),%rax
  6afba2:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6afba7:	49 63 4e 08          	movslq 0x8(%r14),%rcx
  6afbab:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6afbaf:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6afbb3:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6afbb8:	0f 84 40 33 00 00    	je     6b2efe <sqlite3VdbeExec+0x5e9e>
  6afbbe:	4c 89 e7             	mov    %r12,%rdi
  6afbc1:	e8 9a 7e 09 00       	call   747a60 <out2PrereleaseWithClear>
  6afbc6:	e9 3b 33 00 00       	jmp    6b2f06 <sqlite3VdbeExec+0x5ea6>
  6afbcb:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afbd0:	4d 63 65 04          	movslq 0x4(%r13),%r12
  6afbd4:	49 c1 e4 03          	shl    $0x3,%r12
  6afbd8:	4d 03 66 78          	add    0x78(%r14),%r12
  6afbdc:	41 8b 6d 08          	mov    0x8(%r13),%ebp
  6afbe0:	41 8b 46 2c          	mov    0x2c(%r14),%eax
  6afbe4:	4c 8b 3c 24          	mov    (%rsp),%r15
  6afbe8:	4d 8b 24 24          	mov    (%r12),%r12
  6afbec:	4d 8b 44 24 50       	mov    0x50(%r12),%r8
  6afbf1:	41 39 44 24 18       	cmp    %eax,0x18(%r12)
  6afbf6:	75 0f                	jne    6afc07 <sqlite3VdbeExec+0x2ba7>
  6afbf8:	49 8b 44 24 28       	mov    0x28(%r12),%rax
  6afbfd:	80 38 00             	cmpb   $0x0,(%rax)
  6afc00:	75 2a                	jne    6afc2c <sqlite3VdbeExec+0x2bcc>
  6afc02:	e9 72 48 00 00       	jmp    6b4479 <sqlite3VdbeExec+0x7419>
  6afc07:	41 80 7c 24 02 00    	cmpb   $0x0,0x2(%r12)
  6afc0d:	0f 85 1e 48 00 00    	jne    6b4431 <sqlite3VdbeExec+0x73d1>
  6afc13:	4d 8b 6c 24 28       	mov    0x28(%r12),%r13
  6afc18:	41 80 7c 24 03 00    	cmpb   $0x0,0x3(%r12)
  6afc1e:	75 7f                	jne    6afc9f <sqlite3VdbeExec+0x2c3f>
  6afc20:	41 80 7d 00 00       	cmpb   $0x0,0x0(%r13)
  6afc25:	75 16                	jne    6afc3d <sqlite3VdbeExec+0x2bdd>
  6afc27:	e9 e5 49 00 00       	jmp    6b4611 <sqlite3VdbeExec+0x75b1>
  6afc2c:	4c 89 e7             	mov    %r12,%rdi
  6afc2f:	e8 ac 82 ff ff       	call   6a7ee0 <sqlite3VdbeHandleMovedCursor>
  6afc34:	85 c0                	test   %eax,%eax
  6afc36:	74 1a                	je     6afc52 <sqlite3VdbeExec+0x2bf2>
  6afc38:	e9 f5 65 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6afc3d:	4c 89 e7             	mov    %r12,%rdi
  6afc40:	e8 9b 82 ff ff       	call   6a7ee0 <sqlite3VdbeHandleMovedCursor>
  6afc45:	85 c0                	test   %eax,%eax
  6afc47:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afc4c:	0f 85 e0 65 00 00    	jne    6b6232 <sqlite3VdbeExec+0x91d2>
  6afc52:	4d 8b 44 24 50       	mov    0x50(%r12),%r8
  6afc57:	41 8b 46 2c          	mov    0x2c(%r14),%eax
  6afc5b:	41 39 44 24 18       	cmp    %eax,0x18(%r12)
  6afc60:	75 0f                	jne    6afc71 <sqlite3VdbeExec+0x2c11>
  6afc62:	49 8b 44 24 28       	mov    0x28(%r12),%rax
  6afc67:	80 38 00             	cmpb   $0x0,(%rax)
  6afc6a:	75 c0                	jne    6afc2c <sqlite3VdbeExec+0x2bcc>
  6afc6c:	e9 ff 47 00 00       	jmp    6b4470 <sqlite3VdbeExec+0x7410>
  6afc71:	41 80 7c 24 02 00    	cmpb   $0x0,0x2(%r12)
  6afc77:	0f 85 ab 47 00 00    	jne    6b4428 <sqlite3VdbeExec+0x73c8>
  6afc7d:	4d 8b 6c 24 28       	mov    0x28(%r12),%r13
  6afc82:	41 80 7c 24 03 00    	cmpb   $0x0,0x3(%r12)
  6afc88:	75 0c                	jne    6afc96 <sqlite3VdbeExec+0x2c36>
  6afc8a:	41 80 7d 00 00       	cmpb   $0x0,0x0(%r13)
  6afc8f:	75 ac                	jne    6afc3d <sqlite3VdbeExec+0x2bdd>
  6afc91:	e9 72 49 00 00       	jmp    6b4608 <sqlite3VdbeExec+0x75a8>
  6afc96:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6afc9d:	00 00 
  6afc9f:	49 8b 4c 24 08       	mov    0x8(%r12),%rcx
  6afca4:	48 85 c9             	test   %rcx,%rcx
  6afca7:	0f 84 03 5b 00 00    	je     6b57b0 <sqlite3VdbeExec+0x8750>
  6afcad:	8d 55 01             	lea    0x1(%rbp),%edx
  6afcb0:	8b 0c 91             	mov    (%rcx,%rdx,4),%ecx
  6afcb3:	85 c9                	test   %ecx,%ecx
  6afcb5:	0f 84 f5 5a 00 00    	je     6b57b0 <sqlite3VdbeExec+0x8750>
  6afcbb:	49 83 c4 20          	add    $0x20,%r12
  6afcbf:	ff c9                	dec    %ecx
  6afcc1:	89 cd                	mov    %ecx,%ebp
  6afcc3:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afcc8:	e9 1b ff ff ff       	jmp    6afbe8 <sqlite3VdbeExec+0x2b88>
  6afccd:	4d 89 f7             	mov    %r14,%r15
  6afcd0:	49 8b 86 08 01 00 00 	mov    0x108(%r14),%rax
  6afcd7:	4c 8b 6c 24 28       	mov    0x28(%rsp),%r13
  6afcdc:	4c 89 e9             	mov    %r13,%rcx
  6afcdf:	48 85 c0             	test   %rax,%rax
  6afce2:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6afce7:	74 17                	je     6afd00 <sqlite3VdbeExec+0x2ca0>
  6afce9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
  6afcf0:	48 89 c1             	mov    %rax,%rcx
  6afcf3:	48 8b 40 08          	mov    0x8(%rax),%rax
  6afcf7:	48 85 c0             	test   %rax,%rax
  6afcfa:	75 f4                	jne    6afcf0 <sqlite3VdbeExec+0x2c90>
  6afcfc:	48 8b 49 18          	mov    0x18(%rcx),%rcx
  6afd00:	49 63 46 04          	movslq 0x4(%r14),%rax
  6afd04:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6afd08:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6afd0c:	0f b7 4c 01 14       	movzwl 0x14(%rcx,%rax,1),%ecx
  6afd11:	0f b7 c1             	movzwl %cx,%eax
  6afd14:	a8 24                	test   $0x24,%al
  6afd16:	0f 84 44 32 00 00    	je     6b2f60 <sqlite3VdbeExec+0x5f00>
  6afd1c:	49 8b 04 24          	mov    (%r12),%rax
  6afd20:	49 89 04 24          	mov    %rax,(%r12)
  6afd24:	81 e1 40 f2 ff ff    	and    $0xfffff240,%ecx
  6afd2a:	83 c9 04             	or     $0x4,%ecx
  6afd2d:	66 41 89 4c 24 14    	mov    %cx,0x14(%r12)
  6afd33:	49 63 46 08          	movslq 0x8(%r14),%rax
  6afd37:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6afd3b:	4c 89 e9             	mov    %r13,%rcx
  6afd3e:	49 01 c5             	add    %rax,%r13
  6afd41:	0f b7 4c 01 14       	movzwl 0x14(%rcx,%rax,1),%ecx
  6afd46:	0f b7 c1             	movzwl %cx,%eax
  6afd49:	a8 24                	test   $0x24,%al
  6afd4b:	0f 84 3e 32 00 00    	je     6b2f8f <sqlite3VdbeExec+0x5f2f>
  6afd51:	49 8b 45 00          	mov    0x0(%r13),%rax
  6afd55:	4d 89 fe             	mov    %r15,%r14
  6afd58:	4c 8b 3c 24          	mov    (%rsp),%r15
  6afd5c:	49 89 45 00          	mov    %rax,0x0(%r13)
  6afd60:	81 e1 40 f2 ff ff    	and    $0xfffff240,%ecx
  6afd66:	83 c9 04             	or     $0x4,%ecx
  6afd69:	66 41 89 4d 14       	mov    %cx,0x14(%r13)
  6afd6e:	49 39 04 24          	cmp    %rax,(%r12)
  6afd72:	0f 8d 39 06 00 00    	jge    6b03b1 <sqlite3VdbeExec+0x3351>
  6afd78:	49 89 04 24          	mov    %rax,(%r12)
  6afd7c:	e9 74 48 00 00       	jmp    6b45f5 <sqlite3VdbeExec+0x7595>
  6afd81:	49 8b 46 78          	mov    0x78(%r14),%rax
  6afd85:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afd8a:	49 63 75 04          	movslq 0x4(%r13),%rsi
  6afd8e:	48 8b 04 f0          	mov    (%rax,%rsi,8),%rax
  6afd92:	48 85 c0             	test   %rax,%rax
  6afd95:	4c 8b 3c 24          	mov    (%rsp),%r15
  6afd99:	75 35                	jne    6afdd0 <sqlite3VdbeExec+0x2d70>
  6afd9b:	4c 89 f7             	mov    %r14,%rdi
  6afd9e:	ba 01 00 00 00       	mov    $0x1,%edx
  6afda3:	b9 03 00 00 00       	mov    $0x3,%ecx
  6afda8:	e8 e3 6d 00 00       	call   6b6b90 <allocateCursor>
  6afdad:	48 85 c0             	test   %rax,%rax
  6afdb0:	0f 84 4e 65 00 00    	je     6b6304 <sqlite3VdbeExec+0x92a4>
  6afdb6:	c7 40 1c 00 00 00 00 	movl   $0x0,0x1c(%rax)
  6afdbd:	c6 40 04 01          	movb   $0x1,0x4(%rax)
  6afdc1:	80 48 05 08          	orb    $0x8,0x5(%rax)
  6afdc5:	48 8d 0d ac 05 0e 00 	lea    0xe05ac(%rip),%rcx        # 790378 <sqlite3BtreeFakeValidCursor.fakeCursor>
  6afdcc:	48 89 48 28          	mov    %rcx,0x28(%rax)
  6afdd0:	c6 40 02 01          	movb   $0x1,0x2(%rax)
  6afdd4:	c7 40 18 00 00 00 00 	movl   $0x0,0x18(%rax)
  6afddb:	80 38 00             	cmpb   $0x0,(%rax)
  6afdde:	0f 85 33 d5 ff ff    	jne    6ad317 <sqlite3VdbeExec+0x2b7>
  6afde4:	48 8b 78 28          	mov    0x28(%rax),%rdi
  6afde8:	e8 f3 18 fe ff       	call   6916e0 <sqlite3BtreeClearCursor>
  6afded:	49 83 c5 18          	add    $0x18,%r13
  6afdf1:	e9 2a d5 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6afdf6:	4c 8b 2c 24          	mov    (%rsp),%r13
  6afdfa:	41 fe 44 24 70       	incb   0x70(%r12)
  6afdff:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6afe06:	00 00 
  6afe08:	4d 8b b4 24 f8 01 00 	mov    0x1f8(%r12),%r14
  6afe0f:	00 
  6afe10:	41 0f b6 6c 24 6e    	movzbl 0x6e(%r12),%ebp
  6afe16:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6afe1b:	8b 40 04             	mov    0x4(%rax),%eax
  6afe1e:	a8 01                	test   $0x1,%al
  6afe20:	74 12                	je     6afe34 <sqlite3VdbeExec+0x2dd4>
  6afe22:	49 c7 84 24 f8 01 00 	movq   $0x0,0x1f8(%r12)
  6afe29:	00 00 00 00 00 
  6afe2e:	41 c6 44 24 6e 00    	movb   $0x0,0x6e(%r12)
  6afe34:	45 8b bc 24 e0 02 00 	mov    0x2e0(%r12),%r15d
  6afe3b:	00 
  6afe3c:	a8 02                	test   $0x2,%al
  6afe3e:	74 10                	je     6afe50 <sqlite3VdbeExec+0x2df0>
  6afe40:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6afe45:	8b 40 08             	mov    0x8(%rax),%eax
  6afe48:	41 89 84 24 e0 02 00 	mov    %eax,0x2e0(%r12)
  6afe4f:	00 
  6afe50:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6afe55:	48 8b 70 10          	mov    0x10(%rax),%rsi
  6afe59:	4c 8d 44 24 60       	lea    0x60(%rsp),%r8
  6afe5e:	4c 89 e7             	mov    %r12,%rdi
  6afe61:	31 d2                	xor    %edx,%edx
  6afe63:	31 c9                	xor    %ecx,%ecx
  6afe65:	e8 46 55 fd ff       	call   6853b0 <sqlite3_exec>
  6afe6a:	41 fe 4c 24 70       	decb   0x70(%r12)
  6afe6f:	4d 89 b4 24 f8 01 00 	mov    %r14,0x1f8(%r12)
  6afe76:	00 
  6afe77:	41 88 6c 24 6e       	mov    %bpl,0x6e(%r12)
  6afe7c:	45 89 bc 24 e0 02 00 	mov    %r15d,0x2e0(%r12)
  6afe83:	00 
  6afe84:	4c 8b 64 24 60       	mov    0x60(%rsp),%r12
  6afe89:	4d 85 e4             	test   %r12,%r12
  6afe8c:	0f 85 45 5f 00 00    	jne    6b5dd7 <sqlite3VdbeExec+0x8d77>
  6afe92:	85 c0                	test   %eax,%eax
  6afe94:	0f 85 3d 5f 00 00    	jne    6b5dd7 <sqlite3VdbeExec+0x8d77>
  6afe9a:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6afea1:	00 00 
  6afea3:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6afea8:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6afead:	4d 89 ef             	mov    %r13,%r15
  6afeb0:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afeb5:	49 83 c5 18          	add    $0x18,%r13
  6afeb9:	e9 62 d4 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6afebe:	41 f6 44 24 32 08    	testb  $0x8,0x32(%r12)
  6afec4:	0f 85 84 1d 00 00    	jne    6b1c4e <sqlite3VdbeExec+0x4bee>
  6afeca:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6afecf:	41 83 7d 04 00       	cmpl   $0x0,0x4(%r13)
  6afed4:	49 63 45 08          	movslq 0x8(%r13),%rax
  6afed8:	4c 8b 3c 24          	mov    (%rsp),%r15
  6afedc:	0f 84 84 43 00 00    	je     6b4266 <sqlite3VdbeExec+0x7206>
  6afee2:	49 01 84 24 f0 02 00 	add    %rax,0x2f0(%r12)
  6afee9:	00 
  6afeea:	49 83 c5 18          	add    $0x18,%r13
  6afeee:	e9 2d d4 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6afef3:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6afef8:	8b 6a 08             	mov    0x8(%rdx),%ebp
  6afefb:	49 8b 46 78          	mov    0x78(%r14),%rax
  6afeff:	48 63 4a 04          	movslq 0x4(%rdx),%rcx
  6aff03:	4c 8b 2c c8          	mov    (%rax,%rcx,8),%r13
  6aff07:	80 7a 01 fb          	cmpb   $0xfb,0x1(%rdx)
  6aff0b:	0f 85 23 18 00 00    	jne    6b1734 <sqlite3VdbeExec+0x46d4>
  6aff11:	49 83 bc 24 38 01 00 	cmpq   $0x0,0x138(%r12)
  6aff18:	00 00 
  6aff1a:	0f 84 14 18 00 00    	je     6b1734 <sqlite3VdbeExec+0x46d4>
  6aff20:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6aff25:	49 0f be 4d 01       	movsbq 0x1(%r13),%rcx
  6aff2a:	48 c1 e1 05          	shl    $0x5,%rcx
  6aff2e:	4c 8b 24 08          	mov    (%rax,%rcx,1),%r12
  6aff32:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6aff37:	4c 8b 78 10          	mov    0x10(%rax),%r15
  6aff3b:	f6 40 02 02          	testb  $0x2,0x2(%rax)
  6aff3f:	0f 84 f5 17 00 00    	je     6b173a <sqlite3VdbeExec+0x46da>
  6aff45:	41 80 7d 04 00       	cmpb   $0x0,0x4(%r13)
  6aff4a:	0f 84 ea 17 00 00    	je     6b173a <sqlite3VdbeExec+0x46da>
  6aff50:	49 8b 7d 28          	mov    0x28(%r13),%rdi
  6aff54:	48 89 7c 24 20       	mov    %rdi,0x20(%rsp)
  6aff59:	e8 72 45 fe ff       	call   6944d0 <getCellInfo>
  6aff5e:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6aff63:	48 8b 40 30          	mov    0x30(%rax),%rax
  6aff67:	49 89 45 48          	mov    %rax,0x48(%r13)
  6aff6b:	e9 ca 17 00 00       	jmp    6b173a <sqlite3VdbeExec+0x46da>
  6aff70:	49 8b 46 78          	mov    0x78(%r14),%rax
  6aff74:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6aff79:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6aff7d:	4c 8b 34 c8          	mov    (%rax,%rcx,8),%r14
  6aff81:	4d 8b 66 28          	mov    0x28(%r14),%r12
  6aff85:	49 8b 46 30          	mov    0x30(%r14),%rax
  6aff89:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6aff8e:	41 0f b7 45 0c       	movzwl 0xc(%r13),%eax
  6aff93:	66 89 44 24 7c       	mov    %ax,0x7c(%rsp)
  6aff98:	c6 44 24 7e 00       	movb   $0x0,0x7e(%rsp)
  6aff9d:	49 63 45 08          	movslq 0x8(%r13),%rax
  6affa1:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6affa5:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6affaa:	48 89 44 24 68       	mov    %rax,0x68(%rsp)
  6affaf:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6affb4:	48 8d 94 24 e8 00 00 	lea    0xe8(%rsp),%rdx
  6affbb:	00 
  6affbc:	4c 89 e7             	mov    %r12,%rdi
  6affbf:	e8 9c 54 fe ff       	call   695460 <sqlite3BtreeIndexMoveto>
  6affc4:	85 c0                	test   %eax,%eax
  6affc6:	0f 85 6b 5e 00 00    	jne    6b5e37 <sqlite3VdbeExec+0x8dd7>
  6affcc:	83 bc 24 e8 00 00 00 	cmpl   $0x0,0xe8(%rsp)
  6affd3:	00 
  6affd4:	4c 8b 3c 24          	mov    (%rsp),%r15
  6affd8:	0f 84 8e 34 00 00    	je     6b346c <sqlite3VdbeExec+0x640c>
  6affde:	66 41 83 7d 02 00    	cmpw   $0x0,0x2(%r13)
  6affe4:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6affe9:	0f 84 97 34 00 00    	je     6b3486 <sqlite3VdbeExec+0x6426>
  6affef:	b8 01 00 00 10       	mov    $0x10000001,%eax
  6afff4:	41 23 44 24 30       	and    0x30(%r12),%eax
  6afff9:	83 f8 01             	cmp    $0x1,%eax
  6afffc:	0f 84 84 34 00 00    	je     6b3486 <sqlite3VdbeExec+0x6426>
  6b0002:	e9 cd 61 00 00       	jmp    6b61d4 <sqlite3VdbeExec+0x9174>
  6b0007:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b000c:	41 8b 45 04          	mov    0x4(%r13),%eax
  6b0010:	4c 63 f8             	movslq %eax,%r15
  6b0013:	49 83 7d 10 00       	cmpq   $0x0,0x10(%r13)
  6b0018:	0f 84 a5 2f 00 00    	je     6b2fc3 <sqlite3VdbeExec+0x5f63>
  6b001e:	4c 89 64 24 60       	mov    %r12,0x60(%rsp)
  6b0023:	44 89 7c 24 70       	mov    %r15d,0x70(%rsp)
  6b0028:	48 8b 84 24 d0 00 00 	mov    0xd0(%rsp),%rax
  6b002f:	00 
  6b0030:	48 89 44 24 68       	mov    %rax,0x68(%rsp)
  6b0035:	c7 44 24 78 00 00 00 	movl   $0x0,0x78(%rsp)
  6b003c:	00 
  6b003d:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6b0042:	49 c1 e7 05          	shl    $0x5,%r15
  6b0046:	4a 8b 4c 38 08       	mov    0x8(%rax,%r15,1),%rcx
  6b004b:	48 8b 49 08          	mov    0x8(%rcx),%rcx
  6b004f:	8b 49 40             	mov    0x40(%rcx),%ecx
  6b0052:	89 8c 24 80 00 00 00 	mov    %ecx,0x80(%rsp)
  6b0059:	4a 8b 14 38          	mov    (%rax,%r15,1),%rdx
  6b005d:	4d 8b 45 10          	mov    0x10(%r13),%r8
  6b0061:	48 8d 35 af f3 f9 ff 	lea    -0x60c51(%rip),%rsi        # 64f417 <.L.str.296>
  6b0068:	48 8d 0d 91 2d fa ff 	lea    -0x5d26f(%rip),%rcx        # 652e00 <.L.str.295>
  6b006f:	4c 89 e7             	mov    %r12,%rdi
  6b0072:	31 c0                	xor    %eax,%eax
  6b0074:	e8 a7 eb fc ff       	call   67ec20 <sqlite3MPrintf>
  6b0079:	4c 89 e7             	mov    %r12,%rdi
  6b007c:	48 85 c0             	test   %rax,%rax
  6b007f:	0f 84 3e 5f 00 00    	je     6b5fc3 <sqlite3VdbeExec+0x8f63>
  6b0085:	49 89 c4             	mov    %rax,%r12
  6b0088:	4d 89 f7             	mov    %r14,%r15
  6b008b:	c6 87 c5 00 00 00 01 	movb   $0x1,0xc5(%rdi)
  6b0092:	c7 44 24 74 00 00 00 	movl   $0x0,0x74(%rsp)
  6b0099:	00 
  6b009a:	c7 44 24 7c 00 00 00 	movl   $0x0,0x7c(%rsp)
  6b00a1:	00 
  6b00a2:	48 8d 15 37 88 00 00 	lea    0x8837(%rip),%rdx        # 6b88e0 <sqlite3InitCallback>
  6b00a9:	48 8d 4c 24 60       	lea    0x60(%rsp),%rcx
  6b00ae:	48 89 c6             	mov    %rax,%rsi
  6b00b1:	45 31 c0             	xor    %r8d,%r8d
  6b00b4:	e8 f7 52 fd ff       	call   6853b0 <sqlite3_exec>
  6b00b9:	85 c0                	test   %eax,%eax
  6b00bb:	75 04                	jne    6b00c1 <sqlite3VdbeExec+0x3061>
  6b00bd:	8b 44 24 74          	mov    0x74(%rsp),%eax
  6b00c1:	48 89 c1             	mov    %rax,%rcx
  6b00c4:	8b 44 24 7c          	mov    0x7c(%rsp),%eax
  6b00c8:	49 89 ce             	mov    %rcx,%r14
  6b00cb:	09 c8                	or     %ecx,%eax
  6b00cd:	75 2c                	jne    6b00fb <sqlite3VdbeExec+0x309b>
  6b00cf:	48 8d 35 ad cd f9 ff 	lea    -0x63253(%rip),%rsi        # 64ce83 <.L.str.754>
  6b00d6:	48 8d 15 56 15 fa ff 	lea    -0x5eaaa(%rip),%rdx        # 651633 <.L.str.755>
  6b00dd:	4c 8d 05 a3 e6 f9 ff 	lea    -0x6195d(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b00e4:	41 be 0b 00 00 00    	mov    $0xb,%r14d
  6b00ea:	bf 0b 00 00 00       	mov    $0xb,%edi
  6b00ef:	b9 01 89 01 00       	mov    $0x18901,%ecx
  6b00f4:	31 c0                	xor    %eax,%eax
  6b00f6:	e8 85 ee fc ff       	call   67ef80 <sqlite3_log>
  6b00fb:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b0100:	4c 89 e6             	mov    %r12,%rsi
  6b0103:	49 89 fc             	mov    %rdi,%r12
  6b0106:	e8 45 a3 fc ff       	call   67a450 <sqlite3DbFreeNN>
  6b010b:	41 c6 84 24 c5 00 00 	movb   $0x0,0xc5(%r12)
  6b0112:	00 00 
  6b0114:	4c 89 f0             	mov    %r14,%rax
  6b0117:	4d 89 fe             	mov    %r15,%r14
  6b011a:	e9 e6 2e 00 00       	jmp    6b3005 <sqlite3VdbeExec+0x5fa5>
  6b011f:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b0123:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0128:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b012c:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b0130:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b0134:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b0139:	0f 84 e4 2e 00 00    	je     6b3023 <sqlite3VdbeExec+0x5fc3>
  6b013f:	4c 89 e7             	mov    %r12,%rdi
  6b0142:	e8 19 79 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b0147:	e9 df 2e 00 00       	jmp    6b302b <sqlite3VdbeExec+0x5fcb>
  6b014c:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b0150:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0155:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b0159:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b015d:	80 38 01             	cmpb   $0x1,(%rax)
  6b0160:	48 8b 70 28          	mov    0x28(%rax),%rsi
  6b0164:	0f 85 02 1b 00 00    	jne    6b1c6c <sqlite3VdbeExec+0x4c0c>
  6b016a:	4c 89 e7             	mov    %r12,%rdi
  6b016d:	e8 9e 79 00 00       	call   6b7b10 <sqlite3VdbeSorterReset>
  6b0172:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0176:	49 83 c5 18          	add    $0x18,%r13
  6b017a:	e9 a1 d1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b017f:	49 8b 46 38          	mov    0x38(%r14),%rax
  6b0183:	49 89 44 24 78       	mov    %rax,0x78(%r12)
  6b0188:	49 01 84 24 80 00 00 	add    %rax,0x80(%r12)
  6b018f:	00 
  6b0190:	49 c7 46 38 00 00 00 	movq   $0x0,0x38(%r14)
  6b0197:	00 
  6b0198:	e9 5b 4d 00 00       	jmp    6b4ef8 <sqlite3VdbeExec+0x7e98>
  6b019d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b01a2:	41 8b 75 04          	mov    0x4(%r13),%esi
  6b01a6:	4c 89 e7             	mov    %r12,%rdi
  6b01a9:	e8 52 8a 00 00       	call   6b8c00 <sqlite3AnalysisLoad>
  6b01ae:	85 c0                	test   %eax,%eax
  6b01b0:	0f 84 33 48 00 00    	je     6b49e9 <sqlite3VdbeExec+0x7989>
  6b01b6:	e9 fe 5c 00 00       	jmp    6b5eb9 <sqlite3VdbeExec+0x8e59>
  6b01bb:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b01bf:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b01c4:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b01c8:	48 8b 3c c8          	mov    (%rax,%rcx,8),%rdi
  6b01cc:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b01d0:	48 6b f0 38          	imul   $0x38,%rax,%rsi
  6b01d4:	48 03 74 24 28       	add    0x28(%rsp),%rsi
  6b01d9:	41 8b 55 10          	mov    0x10(%r13),%edx
  6b01dd:	c7 44 24 60 00 00 00 	movl   $0x0,0x60(%rsp)
  6b01e4:	00 
  6b01e5:	48 8d 4c 24 60       	lea    0x60(%rsp),%rcx
  6b01ea:	e8 51 6d 00 00       	call   6b6f40 <sqlite3VdbeSorterCompare>
  6b01ef:	83 7c 24 60 00       	cmpl   $0x0,0x60(%rsp)
  6b01f4:	ba 07 00 00 00       	mov    $0x7,%edx
  6b01f9:	b9 0d 00 00 00       	mov    $0xd,%ecx
  6b01fe:	0f 44 ca             	cmove  %edx,%ecx
  6b0201:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b0206:	85 c0                	test   %eax,%eax
  6b0208:	b8 0c 00 00 00       	mov    $0xc,%eax
  6b020d:	0f 44 c1             	cmove  %ecx,%eax
  6b0210:	83 f8 07             	cmp    $0x7,%eax
  6b0213:	0f 84 fd 1c 00 00    	je     6b1f16 <sqlite3VdbeExec+0x4eb6>
  6b0219:	83 f8 0d             	cmp    $0xd,%eax
  6b021c:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0220:	0f 84 46 56 00 00    	je     6b586c <sqlite3VdbeExec+0x880c>
  6b0226:	e9 f9 5b 00 00       	jmp    6b5e24 <sqlite3VdbeExec+0x8dc4>
  6b022b:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b022f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0234:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b0238:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b023c:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b0240:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b0245:	0f 84 25 2e 00 00    	je     6b3070 <sqlite3VdbeExec+0x6010>
  6b024b:	4c 89 e7             	mov    %r12,%rdi
  6b024e:	e8 0d 78 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b0253:	e9 20 2e 00 00       	jmp    6b3078 <sqlite3VdbeExec+0x6018>
  6b0258:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b025d:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0261:	48 6b e8 38          	imul   $0x38,%rax,%rbp
  6b0265:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b0269:	48 89 44 24 50       	mov    %rax,0x50(%rsp)
  6b026e:	4c 6b f8 38          	imul   $0x38,%rax,%r15
  6b0272:	4d 63 45 0c          	movslq 0xc(%r13),%r8
  6b0276:	4d 6b e0 38          	imul   $0x38,%r8,%r12
  6b027a:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b027f:	49 01 cc             	add    %rcx,%r12
  6b0282:	8b 54 29 14          	mov    0x14(%rcx,%rbp,1),%edx
  6b0286:	42 8b 74 39 14       	mov    0x14(%rcx,%r15,1),%esi
  6b028b:	89 f0                	mov    %esi,%eax
  6b028d:	89 54 24 38          	mov    %edx,0x38(%rsp)
  6b0291:	09 d0                	or     %edx,%eax
  6b0293:	a8 01                	test   $0x1,%al
  6b0295:	0f 85 f9 19 00 00    	jne    6b1c94 <sqlite3VdbeExec+0x4c34>
  6b029b:	48 01 cd             	add    %rcx,%rbp
  6b029e:	49 01 cf             	add    %rcx,%r15
  6b02a1:	0f b7 44 24 38       	movzwl 0x38(%rsp),%eax
  6b02a6:	a8 12                	test   $0x12,%al
  6b02a8:	4c 89 7c 24 40       	mov    %r15,0x40(%rsp)
  6b02ad:	4c 89 84 24 b8 00 00 	mov    %r8,0xb8(%rsp)
  6b02b4:	00 
  6b02b5:	0f 84 b8 3f 00 00    	je     6b4273 <sqlite3VdbeExec+0x7213>
  6b02bb:	a9 00 04 00 00       	test   $0x400,%eax
  6b02c0:	0f 84 e1 3f 00 00    	je     6b42a7 <sqlite3VdbeExec+0x7247>
  6b02c6:	48 89 ef             	mov    %rbp,%rdi
  6b02c9:	e8 f2 06 ff ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6b02ce:	e9 af 3f 00 00       	jmp    6b4282 <sqlite3VdbeExec+0x7222>
  6b02d3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b02d7:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b02dc:	8b 69 0c             	mov    0xc(%rcx),%ebp
  6b02df:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6b02e3:	4c 6b e0 38          	imul   $0x38,%rax,%r12
  6b02e7:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b02ec:	49 01 d4             	add    %rdx,%r12
  6b02ef:	48 63 41 08          	movslq 0x8(%rcx),%rax
  6b02f3:	4c 6b e8 38          	imul   $0x38,%rax,%r13
  6b02f7:	49 01 d5             	add    %rdx,%r13
  6b02fa:	41 f6 45 15 90       	testb  $0x90,0x15(%r13)
  6b02ff:	75 1e                	jne    6b031f <sqlite3VdbeExec+0x32bf>
  6b0301:	eb 33                	jmp    6b0336 <sqlite3VdbeExec+0x32d6>
  6b0303:	66 66 66 66 2e 0f 1f 	data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
  6b030a:	84 00 00 00 00 00 
  6b0310:	49 83 c5 38          	add    $0x38,%r13
  6b0314:	49 83 c4 38          	add    $0x38,%r12
  6b0318:	41 f6 45 15 90       	testb  $0x90,0x15(%r13)
  6b031d:	74 17                	je     6b0336 <sqlite3VdbeExec+0x32d6>
  6b031f:	4c 89 ef             	mov    %r13,%rdi
  6b0322:	4c 89 e6             	mov    %r12,%rsi
  6b0325:	ba 00 40 00 00       	mov    $0x4000,%edx
  6b032a:	e8 f1 14 ff ff       	call   6a1820 <vdbeClrCopy>
  6b032f:	41 0f b7 45 14       	movzwl 0x14(%r13),%eax
  6b0334:	eb 3a                	jmp    6b0370 <sqlite3VdbeExec+0x3310>
  6b0336:	49 8b 44 24 10       	mov    0x10(%r12),%rax
  6b033b:	49 89 45 10          	mov    %rax,0x10(%r13)
  6b033f:	66 41 0f 10 04 24    	movupd (%r12),%xmm0
  6b0345:	66 41 0f 11 45 00    	movupd %xmm0,0x0(%r13)
  6b034b:	41 f6 44 24 15 20    	testb  $0x20,0x15(%r12)
  6b0351:	41 0f b7 45 14       	movzwl 0x14(%r13),%eax
  6b0356:	75 18                	jne    6b0370 <sqlite3VdbeExec+0x3310>
  6b0358:	25 ff 8f ff ff       	and    $0xffff8fff,%eax
  6b035d:	0d 00 40 00 00       	or     $0x4000,%eax
  6b0362:	66 41 89 45 14       	mov    %ax,0x14(%r13)
  6b0367:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
  6b036e:	00 00 
  6b0370:	a9 00 40 00 00       	test   $0x4000,%eax
  6b0375:	74 15                	je     6b038c <sqlite3VdbeExec+0x332c>
  6b0377:	4c 89 ef             	mov    %r13,%rdi
  6b037a:	e8 b1 fa fc ff       	call   67fe30 <sqlite3VdbeMemMakeWriteable>
  6b037f:	85 c0                	test   %eax,%eax
  6b0381:	0f 85 47 56 00 00    	jne    6b59ce <sqlite3VdbeExec+0x896e>
  6b0387:	41 0f b7 45 14       	movzwl 0x14(%r13),%eax
  6b038c:	a9 00 08 00 00       	test   $0x800,%eax
  6b0391:	74 15                	je     6b03a8 <sqlite3VdbeExec+0x3348>
  6b0393:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b0398:	f6 41 02 02          	testb  $0x2,0x2(%rcx)
  6b039c:	74 0a                	je     6b03a8 <sqlite3VdbeExec+0x3348>
  6b039e:	25 ff f7 00 00       	and    $0xf7ff,%eax
  6b03a3:	66 41 89 45 14       	mov    %ax,0x14(%r13)
  6b03a8:	83 ed 01             	sub    $0x1,%ebp
  6b03ab:	0f 83 5f ff ff ff    	jae    6b0310 <sqlite3VdbeExec+0x32b0>
  6b03b1:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b03b6:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b03bb:	49 83 c5 18          	add    $0x18,%r13
  6b03bf:	e9 5c cf ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b03c4:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b03c9:	41 f6 45 02 01       	testb  $0x1,0x2(%r13)
  6b03ce:	0f 85 d6 18 00 00    	jne    6b1caa <sqlite3VdbeExec+0x4c4a>
  6b03d4:	31 c0                	xor    %eax,%eax
  6b03d6:	e9 d7 18 00 00       	jmp    6b1cb2 <sqlite3VdbeExec+0x4c52>
  6b03db:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b03e0:	4d 63 7d 08          	movslq 0x8(%r13),%r15
  6b03e4:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b03e8:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b03ec:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b03f1:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6b03f5:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6b03fa:	a8 24                	test   $0x24,%al
  6b03fc:	0f 84 83 2c 00 00    	je     6b3085 <sqlite3VdbeExec+0x6025>
  6b0402:	31 c0                	xor    %eax,%eax
  6b0404:	48 83 3f 00          	cmpq   $0x0,(%rdi)
  6b0408:	0f 95 c0             	setne  %al
  6b040b:	e9 13 54 00 00       	jmp    6b5823 <sqlite3VdbeExec+0x87c3>
  6b0410:	4d 89 f7             	mov    %r14,%r15
  6b0413:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b0417:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6b041c:	49 63 4e 08          	movslq 0x8(%r14),%rcx
  6b0420:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b0424:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b0428:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b042d:	0f 84 74 2c 00 00    	je     6b30a7 <sqlite3VdbeExec+0x6047>
  6b0433:	4c 89 e7             	mov    %r12,%rdi
  6b0436:	e8 25 76 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b043b:	e9 6f 2c 00 00       	jmp    6b30af <sqlite3VdbeExec+0x604f>
  6b0440:	49 8b 86 80 00 00 00 	mov    0x80(%r14),%rax
  6b0447:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b044c:	48 63 49 04          	movslq 0x4(%rcx),%rcx
  6b0450:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b0454:	4c 8d 34 08          	lea    (%rax,%rcx,1),%r14
  6b0458:	49 83 c6 c8          	add    $0xffffffffffffffc8,%r14
  6b045c:	0f b7 54 08 dc       	movzwl -0x24(%rax,%rcx,1),%edx
  6b0461:	f6 c2 12             	test   $0x12,%dl
  6b0464:	74 21                	je     6b0487 <sqlite3VdbeExec+0x3427>
  6b0466:	48 01 c8             	add    %rcx,%rax
  6b0469:	8b 48 d8             	mov    -0x28(%rax),%ecx
  6b046c:	f7 c2 00 04 00 00    	test   $0x400,%edx
  6b0472:	74 03                	je     6b0477 <sqlite3VdbeExec+0x3417>
  6b0474:	41 03 0e             	add    (%r14),%ecx
  6b0477:	48 8b 40 e0          	mov    -0x20(%rax),%rax
  6b047b:	3b 88 88 00 00 00    	cmp    0x88(%rax),%ecx
  6b0481:	0f 8f 87 5e 00 00    	jg     6b630e <sqlite3VdbeExec+0x92ae>
  6b0487:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b048c:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b0490:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0494:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b0499:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6b049d:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6b04a2:	74 08                	je     6b04ac <sqlite3VdbeExec+0x344c>
  6b04a4:	4c 89 e7             	mov    %r12,%rdi
  6b04a7:	e8 94 10 ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b04ac:	49 8b 46 10          	mov    0x10(%r14),%rax
  6b04b0:	49 89 44 24 10       	mov    %rax,0x10(%r12)
  6b04b5:	66 41 0f 10 06       	movupd (%r14),%xmm0
  6b04ba:	66 41 0f 11 04 24    	movupd %xmm0,(%r12)
  6b04c0:	b8 bf 8f ff ff       	mov    $0xffff8fbf,%eax
  6b04c5:	41 23 44 24 14       	and    0x14(%r12),%eax
  6b04ca:	0d 40 20 00 00       	or     $0x2040,%eax
  6b04cf:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b04d5:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b04da:	e9 5d 17 00 00       	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6b04df:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b04e4:	48 63 41 04          	movslq 0x4(%rcx),%rax
  6b04e8:	4c 8b 69 10          	mov    0x10(%rcx),%r13
  6b04ec:	4c 6b f8 38          	imul   $0x38,%rax,%r15
  6b04f0:	4c 03 7c 24 28       	add    0x28(%rsp),%r15
  6b04f5:	48 63 69 08          	movslq 0x8(%rcx),%rbp
  6b04f9:	48 63 41 0c          	movslq 0xc(%rcx),%rax
  6b04fd:	48 89 44 24 38       	mov    %rax,0x38(%rsp)
  6b0502:	4d 85 ed             	test   %r13,%r13
  6b0505:	74 58                	je     6b055f <sqlite3VdbeExec+0x34ff>
  6b0507:	41 0f b6 45 00       	movzbl 0x0(%r13),%eax
  6b050c:	49 ff c5             	inc    %r13
  6b050f:	4d 89 fc             	mov    %r15,%r12
  6b0512:	eb 1c                	jmp    6b0530 <sqlite3VdbeExec+0x34d0>
  6b0514:	66 66 66 2e 0f 1f 84 	data16 data16 cs nopw 0x0(%rax,%rax,1)
  6b051b:	00 00 00 00 00 
  6b0520:	49 83 c4 38          	add    $0x38,%r12
  6b0524:	41 0f b6 45 00       	movzbl 0x0(%r13),%eax
  6b0529:	49 ff c5             	inc    %r13
  6b052c:	84 c0                	test   %al,%al
  6b052e:	74 2f                	je     6b055f <sqlite3VdbeExec+0x34ff>
  6b0530:	0f be f0             	movsbl %al,%esi
  6b0533:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6b0538:	4c 89 e7             	mov    %r12,%rdi
  6b053b:	e8 50 ca ff ff       	call   6acf90 <applyAffinity>
  6b0540:	41 80 7d ff 45       	cmpb   $0x45,-0x1(%r13)
  6b0545:	75 d9                	jne    6b0520 <sqlite3VdbeExec+0x34c0>
  6b0547:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b054d:	a8 04                	test   $0x4,%al
  6b054f:	74 cf                	je     6b0520 <sqlite3VdbeExec+0x34c0>
  6b0551:	83 e0 db             	and    $0xffffffdb,%eax
  6b0554:	83 c8 20             	or     $0x20,%eax
  6b0557:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b055d:	eb c1                	jmp    6b0520 <sqlite3VdbeExec+0x34c0>
  6b055f:	4c 6b c5 38          	imul   $0x38,%rbp,%r8
  6b0563:	4b 8d 2c 38          	lea    (%r8,%r15,1),%rbp
  6b0567:	48 83 c5 c8          	add    $0xffffffffffffffc8,%rbp
  6b056b:	48 6b 7c 24 38 38    	imul   $0x38,0x38(%rsp),%rdi
  6b0571:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6b0576:	45 31 d2             	xor    %r10d,%r10d
  6b0579:	41 b9 ff ff ff ff    	mov    $0xffffffff,%r9d
  6b057f:	45 31 ed             	xor    %r13d,%r13d
  6b0582:	45 31 db             	xor    %r11d,%r11d
  6b0585:	4c 89 44 24 38       	mov    %r8,0x38(%rsp)
  6b058a:	eb 20                	jmp    6b05ac <sqlite3VdbeExec+0x354c>
  6b058c:	0f 1f 40 00          	nopl   0x0(%rax)
  6b0590:	c1 e0 15             	shl    $0x15,%eax
  6b0593:	c1 f8 1f             	sar    $0x1f,%eax
  6b0596:	83 e0 0a             	and    $0xa,%eax
  6b0599:	89 45 24             	mov    %eax,0x24(%rbp)
  6b059c:	41 ff c5             	inc    %r13d
  6b059f:	4c 39 fd             	cmp    %r15,%rbp
  6b05a2:	48 8d 6d c8          	lea    -0x38(%rbp),%rbp
  6b05a6:	0f 84 14 0b 00 00    	je     6b10c0 <sqlite3VdbeExec+0x4060>
  6b05ac:	0f b7 45 14          	movzwl 0x14(%rbp),%eax
  6b05b0:	a8 01                	test   $0x1,%al
  6b05b2:	75 dc                	jne    6b0590 <sqlite3VdbeExec+0x3530>
  6b05b4:	a8 24                	test   $0x24,%al
  6b05b6:	74 2f                	je     6b05e7 <sqlite3VdbeExec+0x3587>
  6b05b8:	48 8b 4d 00          	mov    0x0(%rbp),%rcx
  6b05bc:	48 89 ca             	mov    %rcx,%rdx
  6b05bf:	48 c1 fa 3f          	sar    $0x3f,%rdx
  6b05c3:	48 31 ca             	xor    %rcx,%rdx
  6b05c6:	41 ff c5             	inc    %r13d
  6b05c9:	48 83 fa 7f          	cmp    $0x7f,%rdx
  6b05cd:	77 60                	ja     6b062f <sqlite3VdbeExec+0x35cf>
  6b05cf:	48 83 f9 01          	cmp    $0x1,%rcx
  6b05d3:	77 73                	ja     6b0648 <sqlite3VdbeExec+0x35e8>
  6b05d5:	41 80 be c5 00 00 00 	cmpb   $0x4,0xc5(%r14)
  6b05dc:	04 
  6b05dd:	72 69                	jb     6b0648 <sqlite3VdbeExec+0x35e8>
  6b05df:	83 c9 08             	or     $0x8,%ecx
  6b05e2:	89 4d 24             	mov    %ecx,0x24(%rbp)
  6b05e5:	eb b8                	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b05e7:	a8 08                	test   $0x8,%al
  6b05e9:	75 6c                	jne    6b0657 <sqlite3VdbeExec+0x35f7>
  6b05eb:	8b 75 10             	mov    0x10(%rbp),%esi
  6b05ee:	89 c1                	mov    %eax,%ecx
  6b05f0:	d1 e9                	shr    $1,%ecx
  6b05f2:	83 e1 01             	and    $0x1,%ecx
  6b05f5:	44 8d 24 71          	lea    (%rcx,%rsi,2),%r12d
  6b05f9:	41 83 c4 0c          	add    $0xc,%r12d
  6b05fd:	a9 00 04 00 00       	test   $0x400,%eax
  6b0602:	0f 84 31 01 00 00    	je     6b0739 <sqlite3VdbeExec+0x36d9>
  6b0608:	48 63 4d 00          	movslq 0x0(%rbp),%rcx
  6b060c:	45 8d 24 4c          	lea    (%r12,%rcx,2),%r12d
  6b0610:	4d 85 db             	test   %r11,%r11
  6b0613:	0f 84 a0 00 00 00    	je     6b06b9 <sqlite3VdbeExec+0x3659>
  6b0619:	48 89 74 24 50       	mov    %rsi,0x50(%rsp)
  6b061e:	01 f1                	add    %esi,%ecx
  6b0620:	85 c9                	test   %ecx,%ecx
  6b0622:	0f 8e 96 00 00 00    	jle    6b06be <sqlite3VdbeExec+0x365e>
  6b0628:	89 ce                	mov    %ecx,%esi
  6b062a:	e9 9c 00 00 00       	jmp    6b06cb <sqlite3VdbeExec+0x366b>
  6b062f:	48 81 fa ff 7f 00 00 	cmp    $0x7fff,%rdx
  6b0636:	77 32                	ja     6b066a <sqlite3VdbeExec+0x360a>
  6b0638:	49 83 c3 02          	add    $0x2,%r11
  6b063c:	c7 45 24 02 00 00 00 	movl   $0x2,0x24(%rbp)
  6b0643:	e9 57 ff ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b0648:	49 ff c3             	inc    %r11
  6b064b:	c7 45 24 01 00 00 00 	movl   $0x1,0x24(%rbp)
  6b0652:	e9 48 ff ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b0657:	41 ff c5             	inc    %r13d
  6b065a:	49 83 c3 08          	add    $0x8,%r11
  6b065e:	c7 45 24 07 00 00 00 	movl   $0x7,0x24(%rbp)
  6b0665:	e9 35 ff ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b066a:	48 81 fa ff ff 7f 00 	cmp    $0x7fffff,%rdx
  6b0671:	77 10                	ja     6b0683 <sqlite3VdbeExec+0x3623>
  6b0673:	49 83 c3 03          	add    $0x3,%r11
  6b0677:	c7 45 24 03 00 00 00 	movl   $0x3,0x24(%rbp)
  6b067e:	e9 1c ff ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b0683:	49 8d b1 01 00 00 80 	lea    -0x7fffffff(%r9),%rsi
  6b068a:	48 39 f2             	cmp    %rsi,%rdx
  6b068d:	73 10                	jae    6b069f <sqlite3VdbeExec+0x363f>
  6b068f:	49 83 c3 04          	add    $0x4,%r11
  6b0693:	c7 45 24 04 00 00 00 	movl   $0x4,0x24(%rbp)
  6b069a:	e9 00 ff ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b069f:	48 c1 ea 2f          	shr    $0x2f,%rdx
  6b06a3:	0f 85 ca 00 00 00    	jne    6b0773 <sqlite3VdbeExec+0x3713>
  6b06a9:	49 83 c3 06          	add    $0x6,%r11
  6b06ad:	c7 45 24 05 00 00 00 	movl   $0x5,0x24(%rbp)
  6b06b4:	e9 e6 fe ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b06b9:	49 01 ca             	add    %rcx,%r10
  6b06bc:	eb 7b                	jmp    6b0739 <sqlite3VdbeExec+0x36d9>
  6b06be:	be 01 00 00 00       	mov    $0x1,%esi
  6b06c3:	a8 10                	test   $0x10,%al
  6b06c5:	75 04                	jne    6b06cb <sqlite3VdbeExec+0x366b>
  6b06c7:	89 ce                	mov    %ecx,%esi
  6b06c9:	eb 6e                	jmp    6b0739 <sqlite3VdbeExec+0x36d9>
  6b06cb:	4c 89 9c 24 c8 00 00 	mov    %r11,0xc8(%rsp)
  6b06d2:	00 
  6b06d3:	4c 89 54 24 40       	mov    %r10,0x40(%rsp)
  6b06d8:	48 89 bc 24 b8 00 00 	mov    %rdi,0xb8(%rsp)
  6b06df:	00 
  6b06e0:	48 89 ef             	mov    %rbp,%rdi
  6b06e3:	ba 01 00 00 00       	mov    $0x1,%edx
  6b06e8:	e8 c3 fe fe ff       	call   6a05b0 <sqlite3VdbeMemGrow>
  6b06ed:	85 c0                	test   %eax,%eax
  6b06ef:	0f 85 30 58 00 00    	jne    6b5f25 <sqlite3VdbeExec+0x8ec5>
  6b06f5:	48 63 7d 10          	movslq 0x10(%rbp),%rdi
  6b06f9:	48 03 7d 08          	add    0x8(%rbp),%rdi
  6b06fd:	48 63 55 00          	movslq 0x0(%rbp),%rdx
  6b0701:	31 f6                	xor    %esi,%esi
  6b0703:	e8 48 9e fa ff       	call   65a550 <memset$plt>
  6b0708:	8b 45 00             	mov    0x0(%rbp),%eax
  6b070b:	01 45 10             	add    %eax,0x10(%rbp)
  6b070e:	80 65 15 f9          	andb   $0xf9,0x15(%rbp)
  6b0712:	48 8b 74 24 50       	mov    0x50(%rsp),%rsi
  6b0717:	01 c6                	add    %eax,%esi
  6b0719:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b071e:	48 8b bc 24 b8 00 00 	mov    0xb8(%rsp),%rdi
  6b0725:	00 
  6b0726:	4c 8b 54 24 40       	mov    0x40(%rsp),%r10
  6b072b:	4c 8b 9c 24 c8 00 00 	mov    0xc8(%rsp),%r11
  6b0732:	00 
  6b0733:	41 b9 ff ff ff ff    	mov    $0xffffffff,%r9d
  6b0739:	b8 01 00 00 00       	mov    $0x1,%eax
  6b073e:	41 81 fc 80 00 00 00 	cmp    $0x80,%r12d
  6b0745:	72 1b                	jb     6b0762 <sqlite3VdbeExec+0x3702>
  6b0747:	44 89 e1             	mov    %r12d,%ecx
  6b074a:	48 89 ca             	mov    %rcx,%rdx
  6b074d:	0f 1f 00             	nopl   (%rax)
  6b0750:	48 c1 ea 07          	shr    $0x7,%rdx
  6b0754:	ff c0                	inc    %eax
  6b0756:	48 81 f9 ff 3f 00 00 	cmp    $0x3fff,%rcx
  6b075d:	48 89 d1             	mov    %rdx,%rcx
  6b0760:	77 ee                	ja     6b0750 <sqlite3VdbeExec+0x36f0>
  6b0762:	89 f1                	mov    %esi,%ecx
  6b0764:	49 01 cb             	add    %rcx,%r11
  6b0767:	41 01 c5             	add    %eax,%r13d
  6b076a:	44 89 65 24          	mov    %r12d,0x24(%rbp)
  6b076e:	e9 2c fe ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b0773:	49 83 c3 08          	add    $0x8,%r11
  6b0777:	a8 20                	test   $0x20,%al
  6b0779:	75 0c                	jne    6b0787 <sqlite3VdbeExec+0x3727>
  6b077b:	c7 45 24 06 00 00 00 	movl   $0x6,0x24(%rbp)
  6b0782:	e9 18 fe ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b0787:	0f 57 c0             	xorps  %xmm0,%xmm0
  6b078a:	f2 48 0f 2a c1       	cvtsi2sd %rcx,%xmm0
  6b078f:	f2 0f 11 45 00       	movsd  %xmm0,0x0(%rbp)
  6b0794:	83 e0 d7             	and    $0xffffffd7,%eax
  6b0797:	83 c8 08             	or     $0x8,%eax
  6b079a:	66 89 45 14          	mov    %ax,0x14(%rbp)
  6b079e:	c7 45 24 07 00 00 00 	movl   $0x7,0x24(%rbp)
  6b07a5:	e9 f5 fd ff ff       	jmp    6b059f <sqlite3VdbeExec+0x353f>
  6b07aa:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b07af:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b07b3:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b07b7:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b07bc:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6b07c0:	f6 44 01 15 04       	testb  $0x4,0x15(%rcx,%rax,1)
  6b07c5:	74 10                	je     6b07d7 <sqlite3VdbeExec+0x3777>
  6b07c7:	4c 89 e7             	mov    %r12,%rdi
  6b07ca:	e8 f1 01 ff ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6b07cf:	85 c0                	test   %eax,%eax
  6b07d1:	0f 85 25 58 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6b07d7:	41 0f b6 75 08       	movzbl 0x8(%r13),%esi
  6b07dc:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6b07e1:	4c 89 e7             	mov    %r12,%rdi
  6b07e4:	e8 a7 0a ff ff       	call   6a1290 <sqlite3VdbeMemCast>
  6b07e9:	85 c0                	test   %eax,%eax
  6b07eb:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b07ef:	0f 84 9f 49 00 00    	je     6b5194 <sqlite3VdbeExec+0x8134>
  6b07f5:	e9 38 5a 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b07fa:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b07fe:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0803:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b0807:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b080b:	48 8b 70 28          	mov    0x28(%rax),%rsi
  6b080f:	41 83 7d 0c 00       	cmpl   $0x0,0xc(%r13)
  6b0814:	0f 84 e1 28 00 00    	je     6b30fb <sqlite3VdbeExec+0x609b>
  6b081a:	48 89 f7             	mov    %rsi,%rdi
  6b081d:	e8 3e 55 fe ff       	call   695d60 <sqlite3BtreeRowCountEst>
  6b0822:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6b0827:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b082b:	e9 f6 28 00 00       	jmp    6b3126 <sqlite3VdbeExec+0x60c6>
  6b0830:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0835:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0839:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b083d:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b0842:	4c 8d 24 01          	lea    (%rcx,%rax,1),%r12
  6b0846:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6b084b:	a8 04                	test   $0x4,%al
  6b084d:	0f 85 7b 0b 00 00    	jne    6b13ce <sqlite3VdbeExec+0x436e>
  6b0853:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6b0858:	4c 89 e7             	mov    %r12,%rdi
  6b085b:	be 43 00 00 00       	mov    $0x43,%esi
  6b0860:	e8 2b c7 ff ff       	call   6acf90 <applyAffinity>
  6b0865:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b086b:	a8 04                	test   $0x4,%al
  6b086d:	0f 85 5b 0b 00 00    	jne    6b13ce <sqlite3VdbeExec+0x436e>
  6b0873:	41 83 7d 08 00       	cmpl   $0x0,0x8(%r13)
  6b0878:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b087c:	0f 84 ed 59 00 00    	je     6b626f <sqlite3VdbeExec+0x920f>
  6b0882:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b0887:	e9 e0 4f 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b088c:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b0890:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0895:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b0899:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b089d:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b08a1:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b08a6:	0f 84 a2 28 00 00    	je     6b314e <sqlite3VdbeExec+0x60ee>
  6b08ac:	4c 89 e7             	mov    %r12,%rdi
  6b08af:	e8 ac 71 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b08b4:	e9 9d 28 00 00       	jmp    6b3156 <sqlite3VdbeExec+0x60f6>
  6b08b9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b08be:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6b08c2:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b08c7:	e9 29 06 00 00       	jmp    6b0ef5 <sqlite3VdbeExec+0x3e95>
  6b08cc:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b08d0:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b08d5:	48 63 49 04          	movslq 0x4(%rcx),%rcx
  6b08d9:	4d 89 f5             	mov    %r14,%r13
  6b08dc:	4c 8b 34 c8          	mov    (%rax,%rcx,8),%r14
  6b08e0:	41 80 7e 02 00       	cmpb   $0x0,0x2(%r14)
  6b08e5:	0f 84 88 28 00 00    	je     6b3173 <sqlite3VdbeExec+0x6113>
  6b08eb:	4d 89 ee             	mov    %r13,%r14
  6b08ee:	e9 05 46 00 00       	jmp    6b4ef8 <sqlite3VdbeExec+0x7e98>
  6b08f3:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b08f8:	48 63 40 0c          	movslq 0xc(%rax),%rax
  6b08fc:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0900:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b0905:	f6 44 01 14 01       	testb  $0x1,0x14(%rcx,%rax,1)
  6b090a:	0f 84 1b 04 00 00    	je     6b0d2b <sqlite3VdbeExec+0x3ccb>
  6b0910:	4d 8b be 08 01 00 00 	mov    0x108(%r14),%r15
  6b0917:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b091c:	41 8b 45 04          	mov    0x4(%r13),%eax
  6b0920:	4d 85 ff             	test   %r15,%r15
  6b0923:	0f 84 23 52 00 00    	je     6b5b4c <sqlite3VdbeExec+0x8aec>
  6b0929:	85 c0                	test   %eax,%eax
  6b092b:	0f 85 4c 52 00 00    	jne    6b5b7d <sqlite3VdbeExec+0x8b1d>
  6b0931:	49 8b 47 08          	mov    0x8(%r15),%rax
  6b0935:	49 89 86 08 01 00 00 	mov    %rax,0x108(%r14)
  6b093c:	41 ff 8e 18 01 00 00 	decl   0x118(%r14)
  6b0943:	49 8b 46 38          	mov    0x38(%r14),%rax
  6b0947:	49 89 44 24 78       	mov    %rax,0x78(%r12)
  6b094c:	49 01 84 24 80 00 00 	add    %rax,0x80(%r12)
  6b0953:	00 
  6b0954:	4d 8b 27             	mov    (%r15),%r12
  6b0957:	4c 89 e7             	mov    %r12,%rdi
  6b095a:	e8 91 5a ff ff       	call   6a63f0 <closeCursorsInFrame>
  6b095f:	49 8b 47 10          	mov    0x10(%r15),%rax
  6b0963:	49 89 84 24 88 00 00 	mov    %rax,0x88(%r12)
  6b096a:	00 
  6b096b:	41 8b 47 50          	mov    0x50(%r15),%eax
  6b096f:	41 89 84 24 90 00 00 	mov    %eax,0x90(%r12)
  6b0976:	00 
  6b0977:	49 8b 47 18          	mov    0x18(%r15),%rax
  6b097b:	49 89 44 24 68       	mov    %rax,0x68(%r12)
  6b0980:	41 8b 47 54          	mov    0x54(%r15),%eax
  6b0984:	41 89 44 24 24       	mov    %eax,0x24(%r12)
  6b0989:	49 8b 47 20          	mov    0x20(%r15),%rax
  6b098d:	49 89 44 24 78       	mov    %rax,0x78(%r12)
  6b0992:	41 8b 47 48          	mov    0x48(%r15),%eax
  6b0996:	41 89 44 24 28       	mov    %eax,0x28(%r12)
  6b099b:	49 8b 47 38          	mov    0x38(%r15),%rax
  6b099f:	49 8b 3c 24          	mov    (%r12),%rdi
  6b09a3:	48 89 47 38          	mov    %rax,0x38(%rdi)
  6b09a7:	49 8b 47 60          	mov    0x60(%r15),%rax
  6b09ab:	49 89 44 24 38       	mov    %rax,0x38(%r12)
  6b09b0:	49 8b 47 68          	mov    0x68(%r15),%rax
  6b09b4:	48 89 47 78          	mov    %rax,0x78(%rdi)
  6b09b8:	49 8d b4 24 28 01 00 	lea    0x128(%r12),%rsi
  6b09bf:	00 
  6b09c0:	ba ff ff ff ff       	mov    $0xffffffff,%edx
  6b09c5:	31 c9                	xor    %ecx,%ecx
  6b09c7:	e8 84 4c ff ff       	call   6a5650 <sqlite3VdbeDeleteAuxData>
  6b09cc:	49 8b 47 40          	mov    0x40(%r15),%rax
  6b09d0:	49 89 84 24 28 01 00 	mov    %rax,0x128(%r12)
  6b09d7:	00 
  6b09d8:	49 c7 47 40 00 00 00 	movq   $0x0,0x40(%r15)
  6b09df:	00 
  6b09e0:	41 8b 47 4c          	mov    0x4c(%r15),%eax
  6b09e4:	41 83 7d 08 04       	cmpl   $0x4,0x8(%r13)
  6b09e9:	49 8b 8e 88 00 00 00 	mov    0x88(%r14),%rcx
  6b09f0:	75 0c                	jne    6b09fe <sqlite3VdbeExec+0x399e>
  6b09f2:	48 98                	cltq
  6b09f4:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b09f8:	8b 44 c1 08          	mov    0x8(%rcx,%rax,8),%eax
  6b09fc:	ff c8                	dec    %eax
  6b09fe:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0a02:	49 8b 56 68          	mov    0x68(%r14),%rdx
  6b0a06:	48 89 54 24 28       	mov    %rdx,0x28(%rsp)
  6b0a0b:	48 98                	cltq
  6b0a0d:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b0a11:	48 89 4c 24 58       	mov    %rcx,0x58(%rsp)
  6b0a16:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b0a1a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b0a1f:	49 83 c5 18          	add    $0x18,%r13
  6b0a23:	e9 f8 c8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b0a28:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6b0a2d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0a32:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b0a36:	48 c1 e1 05          	shl    $0x5,%rcx
  6b0a3a:	48 8b 7c 08 08       	mov    0x8(%rax,%rcx,1),%rdi
  6b0a3f:	e8 5c 28 fe ff       	call   6932a0 <sqlite3BtreeIncrVacuum>
  6b0a44:	83 f8 65             	cmp    $0x65,%eax
  6b0a47:	74 16                	je     6b0a5f <sqlite3VdbeExec+0x39ff>
  6b0a49:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b0a4e:	85 c0                	test   %eax,%eax
  6b0a50:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0a54:	0f 84 bd c8 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b0a5a:	e9 28 57 00 00       	jmp    6b6187 <sqlite3VdbeExec+0x9127>
  6b0a5f:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b0a66:	00 00 
  6b0a68:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0a6c:	e9 fb 4d 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b0a71:	83 bc 24 b0 00 00 00 	cmpl   $0x0,0xb0(%rsp)
  6b0a78:	00 
  6b0a79:	0f 88 4c 27 00 00    	js     6b31cb <sqlite3VdbeExec+0x616b>
  6b0a7f:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b0a84:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0a88:	0f 84 90 32 00 00    	je     6b3d1e <sqlite3VdbeExec+0x6cbe>
  6b0a8e:	48 63 40 0c          	movslq 0xc(%rax),%rax
  6b0a92:	e9 d9 4d 00 00       	jmp    6b5870 <sqlite3VdbeExec+0x8810>
  6b0a97:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0a9c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0aa0:	48 6b f8 38          	imul   $0x38,%rax,%rdi
  6b0aa4:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
  6b0aa9:	f6 44 38 14 24       	testb  $0x24,0x14(%rax,%rdi,1)
  6b0aae:	0f 84 3e 27 00 00    	je     6b31f2 <sqlite3VdbeExec+0x6192>
  6b0ab4:	48 01 c7             	add    %rax,%rdi
  6b0ab7:	e8 44 06 ff ff       	call   6a1100 <sqlite3VdbeMemRealify>
  6b0abc:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0ac0:	49 83 c5 18          	add    $0x18,%r13
  6b0ac4:	e9 57 c8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b0ac9:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b0acd:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0ad2:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b0ad6:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b0ada:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b0ade:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b0ae3:	0f 84 16 27 00 00    	je     6b31ff <sqlite3VdbeExec+0x619f>
  6b0ae9:	4c 89 e7             	mov    %r12,%rdi
  6b0aec:	e8 6f 6f 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b0af1:	e9 11 27 00 00       	jmp    6b3207 <sqlite3VdbeExec+0x61a7>
  6b0af6:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0afb:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0aff:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0b03:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6b0b07:	48 ff c9             	dec    %rcx
  6b0b0a:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b0b0f:	48 89 0c 02          	mov    %rcx,(%rdx,%rax,1)
  6b0b13:	66 c7 44 02 14 04 00 	movw   $0x4,0x14(%rdx,%rax,1)
  6b0b1a:	41 83 7d 08 00       	cmpl   $0x0,0x8(%r13)
  6b0b1f:	e9 90 35 00 00       	jmp    6b40b4 <sqlite3VdbeExec+0x7054>
  6b0b24:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0b29:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0b2d:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0b31:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b0b35:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b0b39:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b0b3e:	48 8d 3c 0a          	lea    (%rdx,%rcx,1),%rdi
  6b0b42:	48 8b 34 02          	mov    (%rdx,%rax,1),%rsi
  6b0b46:	f6 44 0a 15 90       	testb  $0x90,0x15(%rdx,%rcx,1)
  6b0b4b:	0f 85 ec 4c 00 00    	jne    6b583d <sqlite3VdbeExec+0x87dd>
  6b0b51:	48 89 37             	mov    %rsi,(%rdi)
  6b0b54:	66 c7 47 14 04 00    	movw   $0x4,0x14(%rdi)
  6b0b5a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0b5e:	49 83 c5 18          	add    $0x18,%r13
  6b0b62:	e9 b9 c7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b0b67:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0b6c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0b70:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0b74:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b0b79:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6b0b7d:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6b0b82:	a8 24                	test   $0x24,%al
  6b0b84:	0f 84 cc 26 00 00    	je     6b3256 <sqlite3VdbeExec+0x61f6>
  6b0b8a:	31 c0                	xor    %eax,%eax
  6b0b8c:	48 83 3f 00          	cmpq   $0x0,(%rdi)
  6b0b90:	0f 95 c0             	setne  %al
  6b0b93:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0b97:	e9 c8 4c 00 00       	jmp    6b5864 <sqlite3VdbeExec+0x8804>
  6b0b9c:	49 8b 8e 88 00 00 00 	mov    0x88(%r14),%rcx
  6b0ba3:	49 8b 86 08 01 00 00 	mov    0x108(%r14),%rax
  6b0baa:	48 85 c0             	test   %rax,%rax
  6b0bad:	0f 84 c9 26 00 00    	je     6b327c <sqlite3VdbeExec+0x621c>
  6b0bb3:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0bb8:	4c 89 ea             	mov    %r13,%rdx
  6b0bbb:	48 29 ca             	sub    %rcx,%rdx
  6b0bbe:	48 c1 fa 03          	sar    $0x3,%rdx
  6b0bc2:	48 bf ab aa aa aa aa 	movabs $0xaaaaaaaaaaaaaaab,%rdi
  6b0bc9:	aa aa aa 
  6b0bcc:	48 0f af fa          	imul   %rdx,%rdi
  6b0bd0:	48 8b 40 28          	mov    0x28(%rax),%rax
  6b0bd4:	89 fa                	mov    %edi,%edx
  6b0bd6:	89 f9                	mov    %edi,%ecx
  6b0bd8:	80 e1 07             	and    $0x7,%cl
  6b0bdb:	be 01 00 00 00       	mov    $0x1,%esi
  6b0be0:	d3 e6                	shl    %cl,%esi
  6b0be2:	c1 ea 03             	shr    $0x3,%edx
  6b0be5:	0f b6 0c 10          	movzbl (%rax,%rdx,1),%ecx
  6b0be9:	83 e7 07             	and    $0x7,%edi
  6b0bec:	0f a3 f9             	bt     %edi,%ecx
  6b0bef:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0bf3:	0f 82 73 4c 00 00    	jb     6b586c <sqlite3VdbeExec+0x880c>
  6b0bf9:	40 08 ce             	or     %cl,%sil
  6b0bfc:	40 88 34 10          	mov    %sil,(%rax,%rdx,1)
  6b0c00:	49 8b 86 88 00 00 00 	mov    0x88(%r14),%rax
  6b0c07:	8b 40 04             	mov    0x4(%rax),%eax
  6b0c0a:	41 89 45 04          	mov    %eax,0x4(%r13)
  6b0c0e:	49 83 c5 18          	add    $0x18,%r13
  6b0c12:	e9 09 c7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b0c17:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0c1c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0c20:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0c24:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b0c29:	8b 4c 02 14          	mov    0x14(%rdx,%rax,1),%ecx
  6b0c2d:	83 e1 c0             	and    $0xffffffc0,%ecx
  6b0c30:	ff c1                	inc    %ecx
  6b0c32:	66 89 4c 02 14       	mov    %cx,0x14(%rdx,%rax,1)
  6b0c37:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0c3b:	49 83 c5 18          	add    $0x18,%r13
  6b0c3f:	e9 dc c6 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b0c44:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b0c49:	48 63 41 0c          	movslq 0xc(%rcx),%rax
  6b0c4d:	48 6b d0 38          	imul   $0x38,%rax,%rdx
  6b0c51:	4d 8b 46 70          	mov    0x70(%r14),%r8
  6b0c55:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b0c59:	48 63 49 04          	movslq 0x4(%rcx),%rcx
  6b0c5d:	4c 8b 3c c8          	mov    (%rax,%rcx,8),%r15
  6b0c61:	4d 8b 67 28          	mov    0x28(%r15),%r12
  6b0c65:	4d 8b 2c 24          	mov    (%r12),%r13
  6b0c69:	49 8b 6d 00          	mov    0x0(%r13),%rbp
  6b0c6d:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
  6b0c72:	48 8b 4c 10 38       	mov    0x38(%rax,%rdx,1),%rcx
  6b0c77:	8b 34 10             	mov    (%rax,%rdx,1),%esi
  6b0c7a:	85 c9                	test   %ecx,%ecx
  6b0c7c:	0f 8e 4e 2b 00 00    	jle    6b37d0 <sqlite3VdbeExec+0x6770>
  6b0c82:	89 c8                	mov    %ecx,%eax
  6b0c84:	25 ff ff ff 7f       	and    $0x7fffffff,%eax
  6b0c89:	48 83 f8 04          	cmp    $0x4,%rax
  6b0c8d:	0f 83 89 2a 00 00    	jae    6b371c <sqlite3VdbeExec+0x66bc>
  6b0c93:	31 ff                	xor    %edi,%edi
  6b0c95:	e9 08 2b 00 00       	jmp    6b37a2 <sqlite3VdbeExec+0x6742>
  6b0c9a:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0c9f:	45 8b 7d 04          	mov    0x4(%r13),%r15d
  6b0ca3:	4d 8b 65 10          	mov    0x10(%r13),%r12
  6b0ca7:	45 85 ff             	test   %r15d,%r15d
  6b0caa:	0f 84 ef 25 00 00    	je     6b329f <sqlite3VdbeExec+0x623f>
  6b0cb0:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b0cb5:	48 8b a8 d8 02 00 00 	mov    0x2d8(%rax),%rbp
  6b0cbc:	48 85 ed             	test   %rbp,%rbp
  6b0cbf:	0f 84 4f 4f 00 00    	je     6b5c14 <sqlite3VdbeExec+0x8bb4>
  6b0cc5:	c7 44 24 38 00 00 00 	movl   $0x0,0x38(%rsp)
  6b0ccc:	00 
  6b0ccd:	48 8b 45 00          	mov    0x0(%rbp),%rax
  6b0cd1:	31 c9                	xor    %ecx,%ecx
  6b0cd3:	eb 1e                	jmp    6b0cf3 <sqlite3VdbeExec+0x3c93>
  6b0cd5:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  6b0cdc:	00 00 00 00 
  6b0ce0:	48 8d 3d 19 9b b9 ff 	lea    -0x4664e7(%rip),%rdi        # 24a800 <sqlite3UpperToLower>
  6b0ce7:	0f b6 14 3a          	movzbl (%rdx,%rdi,1),%edx
  6b0ceb:	3a 14 3e             	cmp    (%rsi,%rdi,1),%dl
  6b0cee:	75 1b                	jne    6b0d0b <sqlite3VdbeExec+0x3cab>
  6b0cf0:	48 ff c1             	inc    %rcx
  6b0cf3:	0f b6 14 08          	movzbl (%rax,%rcx,1),%edx
  6b0cf7:	41 0f b6 34 0c       	movzbl (%r12,%rcx,1),%esi
  6b0cfc:	40 38 f2             	cmp    %sil,%dl
  6b0cff:	75 df                	jne    6b0ce0 <sqlite3VdbeExec+0x3c80>
  6b0d01:	48 85 d2             	test   %rdx,%rdx
  6b0d04:	75 ea                	jne    6b0cf0 <sqlite3VdbeExec+0x3c90>
  6b0d06:	e9 45 0b 00 00       	jmp    6b1850 <sqlite3VdbeExec+0x47f0>
  6b0d0b:	ff 44 24 38          	incl   0x38(%rsp)
  6b0d0f:	48 8b 6d 18          	mov    0x18(%rbp),%rbp
  6b0d13:	48 85 ed             	test   %rbp,%rbp
  6b0d16:	75 b5                	jne    6b0ccd <sqlite3VdbeExec+0x3c6d>
  6b0d18:	e9 f7 4e 00 00       	jmp    6b5c14 <sqlite3VdbeExec+0x8bb4>
  6b0d1d:	83 bc 24 b0 00 00 00 	cmpl   $0x0,0xb0(%rsp)
  6b0d24:	00 
  6b0d25:	0f 84 aa 25 00 00    	je     6b32d5 <sqlite3VdbeExec+0x6275>
  6b0d2b:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0d30:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0d34:	49 83 c5 18          	add    $0x18,%r13
  6b0d38:	e9 e3 c5 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b0d3d:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6b0d42:	48 63 42 04          	movslq 0x4(%rdx),%rax
  6b0d46:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0d4a:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6b0d4f:	48 8b 0c 06          	mov    (%rsi,%rax,1),%rcx
  6b0d53:	49 2b 96 88 00 00 00 	sub    0x88(%r14),%rdx
  6b0d5a:	48 c1 ea 03          	shr    $0x3,%rdx
  6b0d5e:	69 d2 ab aa aa aa    	imul   $0xaaaaaaab,%edx,%edx
  6b0d64:	ff ca                	dec    %edx
  6b0d66:	48 63 d2             	movslq %edx,%rdx
  6b0d69:	48 89 14 06          	mov    %rdx,(%rsi,%rax,1)
  6b0d6d:	48 8d 04 49          	lea    (%rcx,%rcx,2),%rax
  6b0d71:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b0d76:	48 63 44 c1 08       	movslq 0x8(%rcx,%rax,8),%rax
  6b0d7b:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b0d7f:	e9 59 24 00 00       	jmp    6b31dd <sqlite3VdbeExec+0x617d>
  6b0d84:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b0d88:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0d8d:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b0d91:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b0d95:	4c 8b 60 28          	mov    0x28(%rax),%r12
  6b0d99:	4c 89 e7             	mov    %r12,%rdi
  6b0d9c:	e8 2f 3f fe ff       	call   694cd0 <moveToRoot>
  6b0da1:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  6b0da8:	83 f8 10             	cmp    $0x10,%eax
  6b0dab:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0daf:	0f 84 b0 2d 00 00    	je     6b3b65 <sqlite3VdbeExec+0x6b05>
  6b0db5:	85 c0                	test   %eax,%eax
  6b0db7:	0f 85 75 54 00 00    	jne    6b6232 <sqlite3VdbeExec+0x91d2>
  6b0dbd:	49 8b 84 24 88 00 00 	mov    0x88(%r12),%rax
  6b0dc4:	00 
  6b0dc5:	80 78 08 00          	cmpb   $0x0,0x8(%rax)
  6b0dc9:	0f 85 82 2d 00 00    	jne    6b3b51 <sqlite3VdbeExec+0x6af1>
  6b0dcf:	0f b7 48 1a          	movzwl 0x1a(%rax),%ecx
  6b0dd3:	48 8b 50 50          	mov    0x50(%rax),%rdx
  6b0dd7:	48 8b 40 60          	mov    0x60(%rax),%rax
  6b0ddb:	41 0f b7 74 24 56    	movzwl 0x56(%r12),%esi
  6b0de1:	0f b6 3c 70          	movzbl (%rax,%rsi,2),%edi
  6b0de5:	c1 e7 08             	shl    $0x8,%edi
  6b0de8:	0f b6 44 70 01       	movzbl 0x1(%rax,%rsi,2),%eax
  6b0ded:	09 f8                	or     %edi,%eax
  6b0def:	21 c8                	and    %ecx,%eax
  6b0df1:	8b 34 02             	mov    (%rdx,%rax,1),%esi
  6b0df4:	0f ce                	bswap  %esi
  6b0df6:	4c 89 e7             	mov    %r12,%rdi
  6b0df9:	e8 12 45 fe ff       	call   695310 <moveToChild>
  6b0dfe:	85 c0                	test   %eax,%eax
  6b0e00:	74 bb                	je     6b0dbd <sqlite3VdbeExec+0x3d5d>
  6b0e02:	e9 2b 54 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b0e07:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0e0c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0e10:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0e14:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b0e19:	48 8b 0c 02          	mov    (%rdx,%rax,1),%rcx
  6b0e1d:	48 85 c9             	test   %rcx,%rcx
  6b0e20:	0f 8e c8 24 00 00    	jle    6b32ee <sqlite3VdbeExec+0x628e>
  6b0e26:	48 01 d0             	add    %rdx,%rax
  6b0e29:	49 63 55 0c          	movslq 0xc(%r13),%rdx
  6b0e2d:	48 29 d1             	sub    %rdx,%rcx
  6b0e30:	48 89 08             	mov    %rcx,(%rax)
  6b0e33:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0e37:	e9 30 4a 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b0e3c:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0e41:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b0e45:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b0e49:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b0e4e:	66 c7 44 02 14 04 00 	movw   $0x4,0x14(%rdx,%rax,1)
  6b0e55:	4c 89 e9             	mov    %r13,%rcx
  6b0e58:	48 2b 4c 24 58       	sub    0x58(%rsp),%rcx
  6b0e5d:	48 c1 e9 03          	shr    $0x3,%rcx
  6b0e61:	69 c9 ab aa aa aa    	imul   $0xaaaaaaab,%ecx,%ecx
  6b0e67:	48 63 c9             	movslq %ecx,%rcx
  6b0e6a:	48 89 0c 02          	mov    %rcx,(%rdx,%rax,1)
  6b0e6e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0e72:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b0e76:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b0e7a:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b0e7f:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b0e83:	49 83 c5 e8          	add    $0xffffffffffffffe8,%r13
  6b0e87:	41 8b 84 24 90 01 00 	mov    0x190(%r12),%eax
  6b0e8e:	00 
  6b0e8f:	85 c0                	test   %eax,%eax
  6b0e91:	0f 85 2d 4b 00 00    	jne    6b59c4 <sqlite3VdbeExec+0x8964>
  6b0e97:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
  6b0e9e:	00 00 
  6b0ea0:	49 39 df             	cmp    %rbx,%r15
  6b0ea3:	0f 82 6e c4 ff ff    	jb     6ad317 <sqlite3VdbeExec+0x2b7>
  6b0ea9:	49 8b 84 24 08 02 00 	mov    0x208(%r12),%rax
  6b0eb0:	00 
  6b0eb1:	48 85 c0             	test   %rax,%rax
  6b0eb4:	0f 84 5d c4 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b0eba:	41 8b 8c 24 18 02 00 	mov    0x218(%r12),%ecx
  6b0ec1:	00 
  6b0ec2:	48 01 cb             	add    %rcx,%rbx
  6b0ec5:	49 8b bc 24 10 02 00 	mov    0x210(%r12),%rdi
  6b0ecc:	00 
  6b0ecd:	ff d0                	call   *%rax
  6b0ecf:	85 c0                	test   %eax,%eax
  6b0ed1:	74 cd                	je     6b0ea0 <sqlite3VdbeExec+0x3e40>
  6b0ed3:	e9 e5 4a 00 00       	jmp    6b59bd <sqlite3VdbeExec+0x895d>
  6b0ed8:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b0edd:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6b0ee1:	48 6b c1 38          	imul   $0x38,%rcx,%rax
  6b0ee5:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b0eea:	f6 44 02 14 24       	testb  $0x24,0x14(%rdx,%rax,1)
  6b0eef:	0f 84 06 24 00 00    	je     6b32fb <sqlite3VdbeExec+0x629b>
  6b0ef5:	48 6b c1 38          	imul   $0x38,%rcx,%rax
  6b0ef9:	4c 8b 24 02          	mov    (%rdx,%rax,1),%r12
  6b0efd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b0f01:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b0f05:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b0f09:	4c 8b 34 c8          	mov    (%rax,%rcx,8),%r14
  6b0f0d:	49 8b 7e 28          	mov    0x28(%r14),%rdi
  6b0f11:	c7 84 24 e4 00 00 00 	movl   $0x0,0xe4(%rsp)
  6b0f18:	00 00 00 00 
  6b0f1c:	48 8d 8c 24 e4 00 00 	lea    0xe4(%rsp),%rcx
  6b0f23:	00 
  6b0f24:	4c 89 e6             	mov    %r12,%rsi
  6b0f27:	31 d2                	xor    %edx,%edx
  6b0f29:	e8 42 40 fe ff       	call   694f70 <sqlite3BtreeTableMoveto>
  6b0f2e:	4d 89 66 48          	mov    %r12,0x48(%r14)
  6b0f32:	66 41 c7 46 02 00 00 	movw   $0x0,0x2(%r14)
  6b0f39:	41 c7 46 18 00 00 00 	movl   $0x0,0x18(%r14)
  6b0f40:	00 
  6b0f41:	8b 8c 24 e4 00 00 00 	mov    0xe4(%rsp),%ecx
  6b0f48:	41 89 4e 1c          	mov    %ecx,0x1c(%r14)
  6b0f4c:	85 c9                	test   %ecx,%ecx
  6b0f4e:	0f 84 a8 c3 ff ff    	je     6ad2fc <sqlite3VdbeExec+0x29c>
  6b0f54:	41 83 7d 08 00       	cmpl   $0x0,0x8(%r13)
  6b0f59:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b0f5e:	0f 84 3d 4d 00 00    	je     6b5ca1 <sqlite3VdbeExec+0x8c41>
  6b0f64:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b0f69:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b0f6e:	e9 f9 48 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b0f73:	c7 44 24 60 00 00 00 	movl   $0x0,0x60(%rsp)
  6b0f7a:	00 
  6b0f7b:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6b0f80:	8b 72 08             	mov    0x8(%rdx),%esi
  6b0f83:	85 f6                	test   %esi,%esi
  6b0f85:	74 1f                	je     6b0fa6 <sqlite3VdbeExec+0x3f46>
  6b0f87:	49 8b 44 24 30       	mov    0x30(%r12),%rax
  6b0f8c:	48 b9 00 00 00 00 02 	movabs $0x200000000,%rcx
  6b0f93:	00 00 00 
  6b0f96:	48 81 c9 00 00 10 00 	or     $0x100000,%rcx
  6b0f9d:	48 85 c8             	test   %rcx,%rax
  6b0fa0:	0f 85 34 50 00 00    	jne    6b5fda <sqlite3VdbeExec+0x8f7a>
  6b0fa6:	4d 8b 7c 24 20       	mov    0x20(%r12),%r15
  6b0fab:	4c 63 6a 04          	movslq 0x4(%rdx),%r13
  6b0faf:	49 c1 e5 05          	shl    $0x5,%r13
  6b0fb3:	4f 8b 64 2f 08       	mov    0x8(%r15,%r13,1),%r12
  6b0fb8:	4d 85 e4             	test   %r12,%r12
  6b0fbb:	0f 84 a0 00 00 00    	je     6b1061 <sqlite3VdbeExec+0x4001>
  6b0fc1:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6b0fc6:	4c 89 e7             	mov    %r12,%rdi
  6b0fc9:	e8 42 1b fe ff       	call   692b10 <sqlite3BtreeBeginTrans>
  6b0fce:	85 c0                	test   %eax,%eax
  6b0fd0:	0f 85 57 50 00 00    	jne    6b602d <sqlite3VdbeExec+0x8fcd>
  6b0fd6:	41 f6 86 c8 00 00 00 	testb  $0x20,0xc8(%r14)
  6b0fdd:	20 
  6b0fde:	0f 84 8a 00 00 00    	je     6b106e <sqlite3VdbeExec+0x400e>
  6b0fe4:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b0fe9:	83 78 08 00          	cmpl   $0x0,0x8(%rax)
  6b0fed:	74 7f                	je     6b106e <sqlite3VdbeExec+0x400e>
  6b0fef:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b0ff4:	80 78 65 00          	cmpb   $0x0,0x65(%rax)
  6b0ff8:	74 0e                	je     6b1008 <sqlite3VdbeExec+0x3fa8>
  6b0ffa:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b0fff:	83 b8 d4 00 00 00 02 	cmpl   $0x2,0xd4(%rax)
  6b1006:	7c 66                	jl     6b106e <sqlite3VdbeExec+0x400e>
  6b1008:	41 8b 56 40          	mov    0x40(%r14),%edx
  6b100c:	85 d2                	test   %edx,%edx
  6b100e:	75 1d                	jne    6b102d <sqlite3VdbeExec+0x3fcd>
  6b1010:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b1015:	8b 90 ec 02 00 00    	mov    0x2ec(%rax),%edx
  6b101b:	ff c2                	inc    %edx
  6b101d:	89 90 ec 02 00 00    	mov    %edx,0x2ec(%rax)
  6b1023:	03 90 e8 02 00 00    	add    0x2e8(%rax),%edx
  6b1029:	41 89 56 40          	mov    %edx,0x40(%r14)
  6b102d:	ff ca                	dec    %edx
  6b102f:	48 8b 6c 24 10       	mov    0x10(%rsp),%rbp
  6b1034:	48 89 ef             	mov    %rbp,%rdi
  6b1037:	31 f6                	xor    %esi,%esi
  6b1039:	e8 42 59 00 00       	call   6b6980 <sqlite3VtabSavepoint>
  6b103e:	85 c0                	test   %eax,%eax
  6b1040:	75 0c                	jne    6b104e <sqlite3VdbeExec+0x3fee>
  6b1042:	41 8b 76 40          	mov    0x40(%r14),%esi
  6b1046:	4c 89 e7             	mov    %r12,%rdi
  6b1049:	e8 22 31 fe ff       	call   694170 <sqlite3BtreeBeginStmt>
  6b104e:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b1053:	66 0f 10 85 f0 02 00 	movupd 0x2f0(%rbp),%xmm0
  6b105a:	00 
  6b105b:	66 41 0f 11 46 58    	movupd %xmm0,0x58(%r14)
  6b1061:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b1066:	85 c0                	test   %eax,%eax
  6b1068:	0f 85 f6 4f 00 00    	jne    6b6064 <sqlite3VdbeExec+0x9004>
  6b106e:	4c 8b 74 24 08       	mov    0x8(%rsp),%r14
  6b1073:	66 41 83 7e 02 00    	cmpw   $0x0,0x2(%r14)
  6b1079:	74 22                	je     6b109d <sqlite3VdbeExec+0x403d>
  6b107b:	8b 6c 24 60          	mov    0x60(%rsp),%ebp
  6b107f:	41 3b 6e 0c          	cmp    0xc(%r14),%ebp
  6b1083:	0f 85 a9 4b 00 00    	jne    6b5c32 <sqlite3VdbeExec+0x8bd2>
  6b1089:	4d 01 ef             	add    %r13,%r15
  6b108c:	49 8b 47 18          	mov    0x18(%r15),%rax
  6b1090:	8b 40 04             	mov    0x4(%rax),%eax
  6b1093:	41 3b 46 10          	cmp    0x10(%r14),%eax
  6b1097:	0f 85 95 4b 00 00    	jne    6b5c32 <sqlite3VdbeExec+0x8bd2>
  6b109d:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b10a4:	00 00 
  6b10a6:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b10ab:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b10af:	4d 89 f5             	mov    %r14,%r13
  6b10b2:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b10b7:	49 83 c5 18          	add    $0x18,%r13
  6b10bb:	e9 60 c2 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b10c0:	41 83 fd 7e          	cmp    $0x7e,%r13d
  6b10c4:	0f 8f ac 0c 00 00    	jg     6b1d76 <sqlite3VdbeExec+0x4d16>
  6b10ca:	41 ff c5             	inc    %r13d
  6b10cd:	4c 8b 4c 24 10       	mov    0x10(%rsp),%r9
  6b10d2:	e9 ff 0c 00 00       	jmp    6b1dd6 <sqlite3VdbeExec+0x4d76>
  6b10d7:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b10dc:	44 8b 68 10          	mov    0x10(%rax),%r13d
  6b10e0:	31 ed                	xor    %ebp,%ebp
  6b10e2:	eb 07                	jmp    6b10eb <sqlite3VdbeExec+0x408b>
  6b10e4:	31 ed                	xor    %ebp,%ebp
  6b10e6:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b10eb:	8b 70 04             	mov    0x4(%rax),%esi
  6b10ee:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
  6b10f3:	44 89 ea             	mov    %r13d,%edx
  6b10f6:	31 c9                	xor    %ecx,%ecx
  6b10f8:	e8 93 5a 00 00       	call   6b6b90 <allocateCursor>
  6b10fd:	48 85 c0             	test   %rax,%rax
  6b1100:	0f 84 94 49 00 00    	je     6b5a9a <sqlite3VdbeExec+0x8a3a>
  6b1106:	49 89 c5             	mov    %rax,%r13
  6b1109:	44 88 60 01          	mov    %r12b,0x1(%rax)
  6b110d:	c6 40 02 01          	movb   $0x1,0x2(%rax)
  6b1111:	80 48 05 04          	orb    $0x4,0x5(%rax)
  6b1115:	44 89 70 3c          	mov    %r14d,0x3c(%rax)
  6b1119:	4c 8b 40 28          	mov    0x28(%rax),%r8
  6b111d:	48 8b 7c 24 20       	mov    0x20(%rsp),%rdi
  6b1122:	80 7f 11 00          	cmpb   $0x0,0x11(%rdi)
  6b1126:	74 0b                	je     6b1133 <sqlite3VdbeExec+0x40d3>
  6b1128:	48 8b 07             	mov    (%rdi),%rax
  6b112b:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
  6b112f:	48 89 41 08          	mov    %rax,0x8(%rcx)
  6b1133:	41 0f b7 d7          	movzwl %r15w,%edx
  6b1137:	44 89 f6             	mov    %r14d,%esi
  6b113a:	48 89 e9             	mov    %rbp,%rcx
  6b113d:	e8 0e 31 fe ff       	call   694250 <btreeCursor>
  6b1142:	49 89 6d 30          	mov    %rbp,0x30(%r13)
  6b1146:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6b114b:	80 7a 01 f8          	cmpb   $0xf8,0x1(%rdx)
  6b114f:	41 0f 95 45 04       	setne  0x4(%r13)
  6b1154:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1159:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b115d:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1162:	49 8b 75 28          	mov    0x28(%r13),%rsi
  6b1166:	0f b6 4a 02          	movzbl 0x2(%rdx),%ecx
  6b116a:	80 e1 03             	and    $0x3,%cl
  6b116d:	88 4e 03             	mov    %cl,0x3(%rsi)
  6b1170:	85 c0                	test   %eax,%eax
  6b1172:	49 89 d5             	mov    %rdx,%r13
  6b1175:	0f 85 69 52 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6b117b:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b1182:	00 00 
  6b1184:	49 83 c5 18          	add    $0x18,%r13
  6b1188:	e9 93 c1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b118d:	41 0f b7 44 24 2a    	movzwl 0x2a(%r12),%eax
  6b1193:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1197:	e9 77 11 00 00       	jmp    6b2313 <sqlite3VdbeExec+0x52b3>
  6b119c:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b11a3:	00 
  6b11a4:	41 8b 6f 0c          	mov    0xc(%r15),%ebp
  6b11a8:	45 31 f6             	xor    %r14d,%r14d
  6b11ab:	41 83 7f 04 00       	cmpl   $0x0,0x4(%r15)
  6b11b0:	41 0f 95 c6          	setne  %r14b
  6b11b4:	41 c1 e6 08          	shl    $0x8,%r14d
  6b11b8:	41 ff c6             	inc    %r14d
  6b11bb:	66 45 89 74 24 14    	mov    %r14w,0x14(%r12)
  6b11c1:	41 c7 44 24 10 00 00 	movl   $0x0,0x10(%r12)
  6b11c8:	00 00 
  6b11ca:	29 c5                	sub    %eax,%ebp
  6b11cc:	7e 50                	jle    6b121e <sqlite3VdbeExec+0x41be>
  6b11ce:	4e 8d 24 2a          	lea    (%rdx,%r13,1),%r12
  6b11d2:	49 83 c4 38          	add    $0x38,%r12
  6b11d6:	ff c5                	inc    %ebp
  6b11d8:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b11dd:	eb 23                	jmp    6b1202 <sqlite3VdbeExec+0x41a2>
  6b11df:	90                   	nop
  6b11e0:	4c 89 e7             	mov    %r12,%rdi
  6b11e3:	e8 58 03 ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b11e8:	66 45 89 74 24 14    	mov    %r14w,0x14(%r12)
  6b11ee:	41 c7 44 24 10 00 00 	movl   $0x0,0x10(%r12)
  6b11f5:	00 00 
  6b11f7:	49 83 c4 38          	add    $0x38,%r12
  6b11fb:	ff cd                	dec    %ebp
  6b11fd:	83 fd 01             	cmp    $0x1,%ebp
  6b1200:	7e 12                	jle    6b1214 <sqlite3VdbeExec+0x41b4>
  6b1202:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6b1208:	75 d6                	jne    6b11e0 <sqlite3VdbeExec+0x4180>
  6b120a:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b1211:	00 
  6b1212:	eb d4                	jmp    6b11e8 <sqlite3VdbeExec+0x4188>
  6b1214:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1219:	e9 1e 0a 00 00       	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6b121e:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1223:	e9 7e 47 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b1228:	49 8b 75 10          	mov    0x10(%r13),%rsi
  6b122c:	4c 89 e7             	mov    %r12,%rdi
  6b122f:	e8 1c fa fe ff       	call   6a0c50 <sqlite3VdbeMemFinalize>
  6b1234:	e9 92 0e 00 00       	jmp    6b20cb <sqlite3VdbeExec+0x506b>
  6b1239:	41 f6 c1 02          	test   $0x2,%r9b
  6b123d:	0f 84 98 11 00 00    	je     6b23db <sqlite3VdbeExec+0x537b>
  6b1243:	66 83 fe 42          	cmp    $0x42,%si
  6b1247:	0f 85 8e 11 00 00    	jne    6b23db <sqlite3VdbeExec+0x537b>
  6b124d:	f6 c1 02             	test   $0x2,%cl
  6b1250:	0f 85 8f 2d 00 00    	jne    6b3fe5 <sqlite3VdbeExec+0x6f85>
  6b1256:	f6 c1 2c             	test   $0x2c,%cl
  6b1259:	0f 84 94 2d 00 00    	je     6b3ff3 <sqlite3VdbeExec+0x6f93>
  6b125f:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b1264:	4c 89 c7             	mov    %r8,%rdi
  6b1267:	ba 01 00 00 00       	mov    $0x1,%edx
  6b126c:	4d 89 d7             	mov    %r10,%r15
  6b126f:	e8 1c f8 fe ff       	call   6a0a90 <sqlite3VdbeMemStringify>
  6b1274:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
  6b1279:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b127e:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b1283:	41 23 40 14          	and    0x14(%r8),%eax
  6b1287:	41 81 e7 bd 0d 00 00 	and    $0xdbd,%r15d
  6b128e:	42 8d 0c 38          	lea    (%rax,%r15,1),%ecx
  6b1292:	83 c1 02             	add    $0x2,%ecx
  6b1295:	42 8d 14 38          	lea    (%rax,%r15,1),%edx
  6b1299:	45 39 e5             	cmp    %r12d,%r13d
  6b129c:	0f 44 e9             	cmove  %ecx,%ebp
  6b129f:	0f b7 c5             	movzwl %bp,%eax
  6b12a2:	41 89 d2             	mov    %edx,%r10d
  6b12a5:	e9 49 2d 00 00       	jmp    6b3ff3 <sqlite3VdbeExec+0x6f93>
  6b12aa:	a9 00 01 00 00       	test   $0x100,%eax
  6b12af:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b12b4:	0f 85 35 11 00 00    	jne    6b23ef <sqlite3VdbeExec+0x538f>
  6b12ba:	83 e2 01             	and    $0x1,%edx
  6b12bd:	0f 84 2c 11 00 00    	je     6b23ef <sqlite3VdbeExec+0x538f>
  6b12c3:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6b12ca:	00 00 00 00 
  6b12ce:	48 8d 05 43 b7 0d 00 	lea    0xdb743(%rip),%rax        # 78ca18 <sqlite3aEQb>
  6b12d5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b12da:	e9 c1 2d 00 00       	jmp    6b40a0 <sqlite3VdbeExec+0x7040>
  6b12df:	48 8b 05 32 b7 0d 00 	mov    0xdb732(%rip),%rax        # 78ca18 <sqlite3aEQb>
  6b12e6:	42 80 3c 38 00       	cmpb   $0x0,(%rax,%r15,1)
  6b12eb:	0f 84 4f 26 00 00    	je     6b3940 <sqlite3VdbeExec+0x68e0>
  6b12f1:	49 89 cf             	mov    %rcx,%r15
  6b12f4:	e9 73 45 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b12f9:	41 bf 02 00 00 00    	mov    $0x2,%r15d
  6b12ff:	a8 01                	test   $0x1,%al
  6b1301:	0f 85 07 40 00 00    	jne    6b530e <sqlite3VdbeExec+0x82ae>
  6b1307:	a8 08                	test   $0x8,%al
  6b1309:	0f 85 da 2d 00 00    	jne    6b40e9 <sqlite3VdbeExec+0x7089>
  6b130f:	a8 12                	test   $0x12,%al
  6b1311:	0f 84 e1 3f 00 00    	je     6b52f8 <sqlite3VdbeExec+0x8298>
  6b1317:	e8 84 fc fe ff       	call   6a0fa0 <memRealValue>
  6b131c:	e9 db 3f 00 00       	jmp    6b52fc <sqlite3VdbeExec+0x829c>
  6b1321:	f6 c2 08             	test   $0x8,%dl
  6b1324:	0f 85 c1 11 00 00    	jne    6b24eb <sqlite3VdbeExec+0x548b>
  6b132a:	f6 c2 12             	test   $0x12,%dl
  6b132d:	0f 84 52 2a 00 00    	je     6b3d85 <sqlite3VdbeExec+0x6d25>
  6b1333:	48 83 78 08 00       	cmpq   $0x0,0x8(%rax)
  6b1338:	0f 84 47 2a 00 00    	je     6b3d85 <sqlite3VdbeExec+0x6d25>
  6b133e:	49 89 fe             	mov    %rdi,%r14
  6b1341:	48 89 c7             	mov    %rax,%rdi
  6b1344:	e8 07 fc fe ff       	call   6a0f50 <memIntValue>
  6b1349:	4c 89 f7             	mov    %r14,%rdi
  6b134c:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1351:	49 89 c4             	mov    %rax,%r12
  6b1354:	41 0f b7 4d 14       	movzwl 0x14(%r13),%ecx
  6b1359:	e9 72 c1 ff ff       	jmp    6ad4d0 <sqlite3VdbeExec+0x470>
  6b135e:	a8 08                	test   $0x8,%al
  6b1360:	0f 85 ad 11 00 00    	jne    6b2513 <sqlite3VdbeExec+0x54b3>
  6b1366:	a8 12                	test   $0x12,%al
  6b1368:	0f 84 10 2a 00 00    	je     6b3d7e <sqlite3VdbeExec+0x6d1e>
  6b136e:	49 83 7d 08 00       	cmpq   $0x0,0x8(%r13)
  6b1373:	0f 84 05 2a 00 00    	je     6b3d7e <sqlite3VdbeExec+0x6d1e>
  6b1379:	49 89 fe             	mov    %rdi,%r14
  6b137c:	4c 89 ef             	mov    %r13,%rdi
  6b137f:	e8 cc fb fe ff       	call   6a0f50 <memIntValue>
  6b1384:	4c 89 f7             	mov    %r14,%rdi
  6b1387:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b138c:	e9 4e c1 ff ff       	jmp    6ad4df <sqlite3VdbeExec+0x47f>
  6b1391:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b1397:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b139c:	49 83 c5 18          	add    $0x18,%r13
  6b13a0:	e9 7b bf ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b13a5:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b13a9:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b13ad:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6b13b1:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6b13b6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b13ba:	0f 84 97 25 00 00    	je     6b3957 <sqlite3VdbeExec+0x68f7>
  6b13c0:	e8 7b 01 ff ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b13c5:	49 83 c5 18          	add    $0x18,%r13
  6b13c9:	e9 52 bf ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b13ce:	25 40 f2 ff ff       	and    $0xfffff240,%eax
  6b13d3:	83 c8 04             	or     $0x4,%eax
  6b13d6:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b13dc:	e9 5b 08 00 00       	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6b13e1:	49 89 ff             	mov    %rdi,%r15
  6b13e4:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b13e9:	e8 42 16 fd ff       	call   682a30 <sqlite3MulInt64>
  6b13ee:	eb 4c                	jmp    6b143c <sqlite3VdbeExec+0x43dc>
  6b13f0:	48 85 f6             	test   %rsi,%rsi
  6b13f3:	0f 84 24 21 00 00    	je     6b351d <sqlite3VdbeExec+0x64bd>
  6b13f9:	48 83 fe ff          	cmp    $0xffffffffffffffff,%rsi
  6b13fd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1401:	75 12                	jne    6b1415 <sqlite3VdbeExec+0x43b5>
  6b1403:	48 b9 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rcx
  6b140a:	ff ff 7f 
  6b140d:	48 ff c1             	inc    %rcx
  6b1410:	48 39 c8             	cmp    %rcx,%rax
  6b1413:	74 36                	je     6b144b <sqlite3VdbeExec+0x43eb>
  6b1415:	48 89 c1             	mov    %rax,%rcx
  6b1418:	48 09 f1             	or     %rsi,%rcx
  6b141b:	48 c1 e9 20          	shr    $0x20,%rcx
  6b141f:	0f 84 77 3a 00 00    	je     6b4e9c <sqlite3VdbeExec+0x7e3c>
  6b1425:	48 99                	cqto
  6b1427:	48 f7 fe             	idiv   %rsi
  6b142a:	e9 a5 31 00 00       	jmp    6b45d4 <sqlite3VdbeExec+0x7574>
  6b142f:	49 89 ff             	mov    %rdi,%r15
  6b1432:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b1437:	e8 84 15 fd ff       	call   6829c0 <sqlite3SubInt64>
  6b143c:	4c 89 ff             	mov    %r15,%rdi
  6b143f:	85 c0                	test   %eax,%eax
  6b1441:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1445:	0f 84 8e 31 00 00    	je     6b45d9 <sqlite3VdbeExec+0x7579>
  6b144b:	0f b7 47 14          	movzwl 0x14(%rdi),%eax
  6b144f:	a8 08                	test   $0x8,%al
  6b1451:	75 12                	jne    6b1465 <sqlite3VdbeExec+0x4405>
  6b1453:	a8 24                	test   $0x24,%al
  6b1455:	0f 84 70 21 00 00    	je     6b35cb <sqlite3VdbeExec+0x656b>
  6b145b:	0f 57 d2             	xorps  %xmm2,%xmm2
  6b145e:	f2 48 0f 2a 17       	cvtsi2sdq (%rdi),%xmm2
  6b1463:	eb 04                	jmp    6b1469 <sqlite3VdbeExec+0x4409>
  6b1465:	f2 0f 10 17          	movsd  (%rdi),%xmm2
  6b1469:	41 0f b7 45 14       	movzwl 0x14(%r13),%eax
  6b146e:	a8 08                	test   $0x8,%al
  6b1470:	75 13                	jne    6b1485 <sqlite3VdbeExec+0x4425>
  6b1472:	a8 24                	test   $0x24,%al
  6b1474:	0f 84 6d 21 00 00    	je     6b35e7 <sqlite3VdbeExec+0x6587>
  6b147a:	0f 57 c0             	xorps  %xmm0,%xmm0
  6b147d:	f2 49 0f 2a 45 00    	cvtsi2sdq 0x0(%r13),%xmm0
  6b1483:	eb 06                	jmp    6b148b <sqlite3VdbeExec+0x442b>
  6b1485:	f2 41 0f 10 45 00    	movsd  0x0(%r13),%xmm0
  6b148b:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b1490:	0f b6 00             	movzbl (%rax),%eax
  6b1493:	83 c0 95             	add    $0xffffff95,%eax
  6b1496:	83 f8 03             	cmp    $0x3,%eax
  6b1499:	0f 87 38 20 00 00    	ja     6b34d7 <sqlite3VdbeExec+0x6477>
  6b149f:	48 8d 0d 76 68 b9 ff 	lea    -0x46978a(%rip),%rcx        # 247d1c <RuntimeSpecializeableIR_data+0x1fe02c>
  6b14a6:	48 63 04 81          	movslq (%rcx,%rax,4),%rax
  6b14aa:	48 01 c8             	add    %rcx,%rax
  6b14ad:	ff e0                	jmp    *%rax
  6b14af:	f2 0f 58 d0          	addsd  %xmm0,%xmm2
  6b14b3:	e9 fb 1f 00 00       	jmp    6b34b3 <sqlite3VdbeExec+0x6453>
  6b14b8:	48 85 f6             	test   %rsi,%rsi
  6b14bb:	0f 84 5c 20 00 00    	je     6b351d <sqlite3VdbeExec+0x64bd>
  6b14c1:	48 83 fe ff          	cmp    $0xffffffffffffffff,%rsi
  6b14c5:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b14ca:	48 0f 45 ce          	cmovne %rsi,%rcx
  6b14ce:	48 89 c2             	mov    %rax,%rdx
  6b14d1:	48 09 ca             	or     %rcx,%rdx
  6b14d4:	48 c1 ea 20          	shr    $0x20,%rdx
  6b14d8:	0f 84 ec 30 00 00    	je     6b45ca <sqlite3VdbeExec+0x756a>
  6b14de:	48 99                	cqto
  6b14e0:	48 f7 f9             	idiv   %rcx
  6b14e3:	48 89 d0             	mov    %rdx,%rax
  6b14e6:	e9 e5 30 00 00       	jmp    6b45d0 <sqlite3VdbeExec+0x7570>
  6b14eb:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6b14f2:	00 00 
  6b14f4:	49 8b 74 24 28       	mov    0x28(%r12),%rsi
  6b14f9:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6b14fe:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b1503:	e8 78 80 ff ff       	call   6a9580 <sqlite3VdbeIdxRowid>
  6b1508:	85 c0                	test   %eax,%eax
  6b150a:	0f 85 22 4d 00 00    	jne    6b6232 <sqlite3VdbeExec+0x91d2>
  6b1510:	41 80 7d 00 8d       	cmpb   $0x8d,0x0(%r13)
  6b1515:	0f 85 47 20 00 00    	jne    6b3562 <sqlite3VdbeExec+0x6502>
  6b151b:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b151f:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6b1523:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b1527:	c6 40 02 00          	movb   $0x0,0x2(%rax)
  6b152b:	48 8b 4c 24 60       	mov    0x60(%rsp),%rcx
  6b1530:	48 89 48 48          	mov    %rcx,0x48(%rax)
  6b1534:	c6 40 03 01          	movb   $0x1,0x3(%rax)
  6b1538:	c7 40 18 00 00 00 00 	movl   $0x0,0x18(%rax)
  6b153f:	49 8b 4d 10          	mov    0x10(%r13),%rcx
  6b1543:	48 89 48 08          	mov    %rcx,0x8(%rax)
  6b1547:	4c 89 60 20          	mov    %r12,0x20(%rax)
  6b154b:	e9 44 3c 00 00       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6b1550:	a8 02                	test   $0x2,%al
  6b1552:	0f 84 f0 44 00 00    	je     6b5a48 <sqlite3VdbeExec+0x89e8>
  6b1558:	49 83 c4 38          	add    $0x38,%r12
  6b155c:	41 0f b7 47 36       	movzwl 0x36(%r15),%eax
  6b1561:	48 ff c5             	inc    %rbp
  6b1564:	48 0f bf c8          	movswq %ax,%rcx
  6b1568:	49 83 c5 10          	add    $0x10,%r13
  6b156c:	48 39 cd             	cmp    %rcx,%rbp
  6b156f:	0f 8d 31 44 00 00    	jge    6b59a6 <sqlite3VdbeExec+0x8946>
  6b1575:	41 0f b7 4d 00       	movzwl 0x0(%r13),%ecx
  6b157a:	f6 c1 60             	test   $0x60,%cl
  6b157d:	74 16                	je     6b1595 <sqlite3VdbeExec+0x4535>
  6b157f:	f6 c1 20             	test   $0x20,%cl
  6b1582:	75 dd                	jne    6b1561 <sqlite3VdbeExec+0x4501>
  6b1584:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b1589:	83 79 0c 00          	cmpl   $0x0,0xc(%rcx)
  6b158d:	74 06                	je     6b1595 <sqlite3VdbeExec+0x4535>
  6b158f:	49 83 c4 38          	add    $0x38,%r12
  6b1593:	eb cc                	jmp    6b1561 <sqlite3VdbeExec+0x4501>
  6b1595:	41 0f be 75 fb       	movsbl -0x5(%r13),%esi
  6b159a:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6b159f:	4c 89 e7             	mov    %r12,%rdi
  6b15a2:	e8 e9 b9 ff ff       	call   6acf90 <applyAffinity>
  6b15a7:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b15ad:	a8 01                	test   $0x1,%al
  6b15af:	75 a7                	jne    6b1558 <sqlite3VdbeExec+0x44f8>
  6b15b1:	41 0f b6 4d fa       	movzbl -0x6(%r13),%ecx
  6b15b6:	c1 e9 04             	shr    $0x4,%ecx
  6b15b9:	8d 51 fe             	lea    -0x2(%rcx),%edx
  6b15bc:	83 fa 04             	cmp    $0x4,%edx
  6b15bf:	77 97                	ja     6b1558 <sqlite3VdbeExec+0x44f8>
  6b15c1:	48 8d 35 30 67 b9 ff 	lea    -0x4698d0(%rip),%rsi        # 247cf8 <RuntimeSpecializeableIR_data+0x1fe008>
  6b15c8:	48 63 14 96          	movslq (%rsi,%rdx,4),%rdx
  6b15cc:	48 01 f2             	add    %rsi,%rdx
  6b15cf:	ff e2                	jmp    *%rdx
  6b15d1:	a8 04                	test   $0x4,%al
  6b15d3:	75 83                	jne    6b1558 <sqlite3VdbeExec+0x44f8>
  6b15d5:	e9 6e 44 00 00       	jmp    6b5a48 <sqlite3VdbeExec+0x89e8>
  6b15da:	a8 10                	test   $0x10,%al
  6b15dc:	0f 85 76 ff ff ff    	jne    6b1558 <sqlite3VdbeExec+0x44f8>
  6b15e2:	e9 61 44 00 00       	jmp    6b5a48 <sqlite3VdbeExec+0x89e8>
  6b15e7:	a8 04                	test   $0x4,%al
  6b15e9:	75 0d                	jne    6b15f8 <sqlite3VdbeExec+0x4598>
  6b15eb:	a8 28                	test   $0x28,%al
  6b15ed:	0f 85 65 ff ff ff    	jne    6b1558 <sqlite3VdbeExec+0x44f8>
  6b15f3:	e9 50 44 00 00       	jmp    6b5a48 <sqlite3VdbeExec+0x89e8>
  6b15f8:	49 8b 0c 24          	mov    (%r12),%rcx
  6b15fc:	48 ba 00 00 00 00 00 	movabs $0x800000000000,%rdx
  6b1603:	80 00 00 
  6b1606:	48 01 ca             	add    %rcx,%rdx
  6b1609:	48 c1 ea 30          	shr    $0x30,%rdx
  6b160d:	75 0a                	jne    6b1619 <sqlite3VdbeExec+0x45b9>
  6b160f:	66 b9 20 00          	mov    $0x20,%cx
  6b1613:	66 ba db ff          	mov    $0xffdb,%dx
  6b1617:	eb 16                	jmp    6b162f <sqlite3VdbeExec+0x45cf>
  6b1619:	0f 57 c0             	xorps  %xmm0,%xmm0
  6b161c:	f2 48 0f 2a c1       	cvtsi2sd %rcx,%xmm0
  6b1621:	f2 41 0f 11 04 24    	movsd  %xmm0,(%r12)
  6b1627:	66 b9 08 00          	mov    $0x8,%cx
  6b162b:	66 ba f3 ff          	mov    $0xfff3,%dx
  6b162f:	0f bf d2             	movswl %dx,%edx
  6b1632:	21 c2                	and    %eax,%edx
  6b1634:	0f b7 c1             	movzwl %cx,%eax
  6b1637:	09 d0                	or     %edx,%eax
  6b1639:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b163f:	e9 14 ff ff ff       	jmp    6b1558 <sqlite3VdbeExec+0x44f8>
  6b1644:	31 c9                	xor    %ecx,%ecx
  6b1646:	e9 58 03 00 00       	jmp    6b19a3 <sqlite3VdbeExec+0x4943>
  6b164b:	31 ed                	xor    %ebp,%ebp
  6b164d:	45 31 ed             	xor    %r13d,%r13d
  6b1650:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b1655:	0f b7 51 02          	movzwl 0x2(%rcx),%edx
  6b1659:	f6 c2 01             	test   $0x1,%dl
  6b165c:	74 12                	je     6b1670 <sqlite3VdbeExec+0x4610>
  6b165e:	49 ff 46 38          	incq   0x38(%r14)
  6b1662:	f6 c2 20             	test   $0x20,%dl
  6b1665:	74 09                	je     6b1670 <sqlite3VdbeExec+0x4610>
  6b1667:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
  6b166c:	4c 89 61 38          	mov    %r12,0x38(%rcx)
  6b1670:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b1674:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6b1679:	48 8b 48 08          	mov    0x8(%rax),%rcx
  6b167d:	48 89 4c 24 70       	mov    %rcx,0x70(%rsp)
  6b1682:	8b 48 10             	mov    0x10(%rax),%ecx
  6b1685:	89 8c 24 84 00 00 00 	mov    %ecx,0x84(%rsp)
  6b168c:	31 f6                	xor    %esi,%esi
  6b168e:	b9 00 00 00 00       	mov    $0x0,%ecx
  6b1693:	f6 c2 10             	test   $0x10,%dl
  6b1696:	74 04                	je     6b169c <sqlite3VdbeExec+0x463c>
  6b1698:	41 8b 4f 1c          	mov    0x1c(%r15),%ecx
  6b169c:	f6 40 15 04          	testb  $0x4,0x15(%rax)
  6b16a0:	74 02                	je     6b16a4 <sqlite3VdbeExec+0x4644>
  6b16a2:	8b 30                	mov    (%rax),%esi
  6b16a4:	89 b4 24 88 00 00 00 	mov    %esi,0x88(%rsp)
  6b16ab:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6b16b2:	00 00 
  6b16b4:	49 8b 7f 28          	mov    0x28(%r15),%rdi
  6b16b8:	81 e2 8a 00 00 00    	and    $0x8a,%edx
  6b16be:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6b16c3:	e8 c8 4c fe ff       	call   696390 <sqlite3BtreeInsert>
  6b16c8:	41 c6 47 03 00       	movb   $0x0,0x3(%r15)
  6b16cd:	41 c7 47 18 00 00 00 	movl   $0x0,0x18(%r15)
  6b16d4:	00 
  6b16d5:	85 c0                	test   %eax,%eax
  6b16d7:	0f 85 4c 4c 00 00    	jne    6b6329 <sqlite3VdbeExec+0x92c9>
  6b16dd:	48 85 ed             	test   %rbp,%rbp
  6b16e0:	74 32                	je     6b1714 <sqlite3VdbeExec+0x46b4>
  6b16e2:	4c 8b 4c 24 10       	mov    0x10(%rsp),%r9
  6b16e7:	49 8b b9 30 01 00 00 	mov    0x130(%r9),%rdi
  6b16ee:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b16f3:	0f b7 40 02          	movzwl 0x2(%rax),%eax
  6b16f7:	83 e0 04             	and    $0x4,%eax
  6b16fa:	c1 e8 02             	shr    $0x2,%eax
  6b16fd:	8d 34 80             	lea    (%rax,%rax,4),%esi
  6b1700:	83 c6 12             	add    $0x12,%esi
  6b1703:	48 8b 4d 00          	mov    0x0(%rbp),%rcx
  6b1707:	4c 89 ea             	mov    %r13,%rdx
  6b170a:	4d 89 e0             	mov    %r12,%r8
  6b170d:	41 ff 91 38 01 00 00 	call   *0x138(%r9)
  6b1714:	48 8b 84 24 d8 00 00 	mov    0xd8(%rsp),%rax
  6b171b:	00 
  6b171c:	ff c0                	inc    %eax
  6b171e:	48 89 84 24 d8 00 00 	mov    %rax,0xd8(%rsp)
  6b1725:	00 
  6b1726:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b172d:	00 00 
  6b172f:	e9 72 42 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b1734:	45 31 ff             	xor    %r15d,%r15d
  6b1737:	45 31 e4             	xor    %r12d,%r12d
  6b173a:	49 8b 7d 28          	mov    0x28(%r13),%rdi
  6b173e:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b1743:	0f b6 70 02          	movzbl 0x2(%rax),%esi
  6b1747:	e8 64 9e fe ff       	call   69b5b0 <sqlite3BtreeDelete>
  6b174c:	49 c7 45 18 00 00 00 	movq   $0x0,0x18(%r13)
  6b1753:	00 
  6b1754:	85 c0                	test   %eax,%eax
  6b1756:	0f 85 cd 4b 00 00    	jne    6b6329 <sqlite3VdbeExec+0x92c9>
  6b175c:	48 8b 84 24 d8 00 00 	mov    0xd8(%rsp),%rax
  6b1763:	00 
  6b1764:	ff c0                	inc    %eax
  6b1766:	48 89 84 24 d8 00 00 	mov    %rax,0xd8(%rsp)
  6b176d:	00 
  6b176e:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b1775:	00 00 
  6b1777:	40 f6 c5 01          	test   $0x1,%bpl
  6b177b:	0f 84 25 42 00 00    	je     6b59a6 <sqlite3VdbeExec+0x8946>
  6b1781:	49 ff 46 38          	incq   0x38(%r14)
  6b1785:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b178a:	48 8b 80 38 01 00 00 	mov    0x138(%rax),%rax
  6b1791:	48 85 c0             	test   %rax,%rax
  6b1794:	48 8b 2c 24          	mov    (%rsp),%rbp
  6b1798:	0f 84 b8 1c 00 00    	je     6b3456 <sqlite3VdbeExec+0x63f6>
  6b179e:	4d 85 ff             	test   %r15,%r15
  6b17a1:	0f 84 af 1c 00 00    	je     6b3456 <sqlite3VdbeExec+0x63f6>
  6b17a7:	41 f6 47 30 80       	testb  $0x80,0x30(%r15)
  6b17ac:	0f 85 a4 1c 00 00    	jne    6b3456 <sqlite3VdbeExec+0x63f6>
  6b17b2:	4c 8b 4c 24 10       	mov    0x10(%rsp),%r9
  6b17b7:	49 8b b9 30 01 00 00 	mov    0x130(%r9),%rdi
  6b17be:	49 8b 0f             	mov    (%r15),%rcx
  6b17c1:	4d 8b 45 48          	mov    0x48(%r13),%r8
  6b17c5:	be 09 00 00 00       	mov    $0x9,%esi
  6b17ca:	4c 89 e2             	mov    %r12,%rdx
  6b17cd:	4d 89 cc             	mov    %r9,%r12
  6b17d0:	ff d0                	call   *%rax
  6b17d2:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b17d7:	49 89 ef             	mov    %rbp,%r15
  6b17da:	49 83 c5 18          	add    $0x18,%r13
  6b17de:	e9 3d bb ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b17e3:	48 85 c0             	test   %rax,%rax
  6b17e6:	0f 84 fb 3f 00 00    	je     6b57e7 <sqlite3VdbeExec+0x8787>
  6b17ec:	40 b6 d3             	mov    $0xd3,%sil
  6b17ef:	40 28 d6             	sub    %dl,%sil
  6b17f2:	49 89 c0             	mov    %rax,%r8
  6b17f5:	49 f7 d8             	neg    %r8
  6b17f8:	48 83 f8 c1          	cmp    $0xffffffffffffffc1,%rax
  6b17fc:	b9 40 00 00 00       	mov    $0x40,%ecx
  6b1801:	49 0f 4d c8          	cmovge %r8,%rcx
  6b1805:	48 85 c0             	test   %rax,%rax
  6b1808:	40 0f b6 f6          	movzbl %sil,%esi
  6b180c:	0f b6 d2             	movzbl %dl,%edx
  6b180f:	0f 48 d6             	cmovs  %esi,%edx
  6b1812:	48 0f 49 c8          	cmovns %rax,%rcx
  6b1816:	48 83 f9 40          	cmp    $0x40,%rcx
  6b181a:	0f 8c 6d 25 00 00    	jl     6b3d8d <sqlite3VdbeExec+0x6d2d>
  6b1820:	4d 85 e4             	test   %r12,%r12
  6b1823:	0f 98 c0             	sets   %al
  6b1826:	80 fa 69             	cmp    $0x69,%dl
  6b1829:	0f 95 c1             	setne  %cl
  6b182c:	20 c1                	and    %al,%cl
  6b182e:	44 0f b6 e1          	movzbl %cl,%r12d
  6b1832:	49 f7 dc             	neg    %r12
  6b1835:	e9 ad 3f 00 00       	jmp    6b57e7 <sqlite3VdbeExec+0x8787>
  6b183a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b183f:	4d 89 ef             	mov    %r13,%r15
  6b1842:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b1847:	49 83 c5 18          	add    $0x18,%r13
  6b184b:	e9 d0 ba ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1850:	41 83 ff 01          	cmp    $0x1,%r15d
  6b1854:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1859:	75 0f                	jne    6b186a <sqlite3VdbeExec+0x480a>
  6b185b:	41 83 bc 24 d8 00 00 	cmpl   $0x0,0xd8(%r12)
  6b1862:	00 00 
  6b1864:	0f 8f 0d 48 00 00    	jg     6b6077 <sqlite3VdbeExec+0x9017>
  6b186a:	31 d2                	xor    %edx,%edx
  6b186c:	48 83 7d 18 00       	cmpq   $0x0,0x18(%rbp)
  6b1871:	c7 44 24 40 00 00 00 	movl   $0x0,0x40(%rsp)
  6b1878:	00 
  6b1879:	0f 85 cd 08 00 00    	jne    6b214c <sqlite3VdbeExec+0x50ec>
  6b187f:	41 83 ff 01          	cmp    $0x1,%r15d
  6b1883:	0f 95 c0             	setne  %al
  6b1886:	41 80 7c 24 6d 00    	cmpb   $0x0,0x6d(%r12)
  6b188c:	0f 95 c1             	setne  %cl
  6b188f:	89 4c 24 40          	mov    %ecx,0x40(%rsp)
  6b1893:	0f 94 c1             	sete   %cl
  6b1896:	08 c1                	or     %al,%cl
  6b1898:	0f 85 ae 08 00 00    	jne    6b214c <sqlite3VdbeExec+0x50ec>
  6b189e:	4c 89 f7             	mov    %r14,%rdi
  6b18a1:	be 01 00 00 00       	mov    $0x1,%esi
  6b18a6:	e8 25 51 ff ff       	call   6a69d0 <sqlite3VdbeCheckFk>
  6b18ab:	85 c0                	test   %eax,%eax
  6b18ad:	0f 85 c4 4a 00 00    	jne    6b6377 <sqlite3VdbeExec+0x9317>
  6b18b3:	41 c6 44 24 65 01    	movb   $0x1,0x65(%r12)
  6b18b9:	4c 89 f7             	mov    %r14,%rdi
  6b18bc:	e8 6f 51 ff ff       	call   6a6a30 <sqlite3VdbeHalt>
  6b18c1:	83 f8 05             	cmp    $0x5,%eax
  6b18c4:	0f 84 b9 4a 00 00    	je     6b6383 <sqlite3VdbeExec+0x9323>
  6b18ca:	41 8b 46 34          	mov    0x34(%r14),%eax
  6b18ce:	85 c0                	test   %eax,%eax
  6b18d0:	0f 85 df 4a 00 00    	jne    6b63b5 <sqlite3VdbeExec+0x9355>
  6b18d6:	41 c6 44 24 6d 00    	movb   $0x0,0x6d(%r12)
  6b18dc:	b0 01                	mov    $0x1,%al
  6b18de:	89 44 24 40          	mov    %eax,0x40(%rsp)
  6b18e2:	e9 3d 25 00 00       	jmp    6b3e24 <sqlite3VdbeExec+0x6dc4>
  6b18e7:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b18ec:	4c 8d 04 32          	lea    (%rdx,%rsi,1),%r8
  6b18f0:	89 cf                	mov    %ecx,%edi
  6b18f2:	83 e7 fc             	and    $0xfffffffc,%edi
  6b18f5:	89 ca                	mov    %ecx,%edx
  6b18f7:	83 e2 03             	and    $0x3,%edx
  6b18fa:	49 89 c9             	mov    %rcx,%r9
  6b18fd:	49 89 fa             	mov    %rdi,%r10
  6b1900:	66 49 0f 6e c1       	movq   %r9,%xmm0
  6b1905:	66 0f 70 c0 44       	pshufd $0x44,%xmm0,%xmm0
  6b190a:	66 0f 6f 0d de 9a f9 	movdqa -0x66522(%rip),%xmm1        # 64b3f0 <.LCPI830_2>
  6b1911:	ff 
  6b1912:	66 0f 6f d0          	movdqa %xmm0,%xmm2
  6b1916:	66 0f f4 d1          	pmuludq %xmm1,%xmm2
  6b191a:	66 0f 73 d0 20       	psrlq  $0x20,%xmm0
  6b191f:	66 0f f4 c1          	pmuludq %xmm1,%xmm0
  6b1923:	66 0f 73 f0 20       	psllq  $0x20,%xmm0
  6b1928:	66 49 0f 6e c8       	movq   %r8,%xmm1
  6b192d:	66 0f 70 c9 44       	pshufd $0x44,%xmm1,%xmm1
  6b1932:	66 0f d4 ca          	paddq  %xmm2,%xmm1
  6b1936:	66 0f d4 c8          	paddq  %xmm0,%xmm1
  6b193a:	66 0f 6f 05 8e 98 f9 	movdqa -0x66772(%rip),%xmm0        # 64b1d0 <.LCPI830_3>
  6b1941:	ff 
  6b1942:	66 0f d4 c1          	paddq  %xmm1,%xmm0
  6b1946:	66 0f d4 0d 32 99 f9 	paddq  -0x666ce(%rip),%xmm1        # 64b280 <.LCPI830_4>
  6b194d:	ff 
  6b194e:	66 0f 70 c0 4e       	pshufd $0x4e,%xmm0,%xmm0
  6b1953:	f3 43 0f 7f 44 cc 20 	movdqu %xmm0,0x20(%r12,%r9,8)
  6b195a:	66 0f 70 c1 4e       	pshufd $0x4e,%xmm1,%xmm0
  6b195f:	f3 43 0f 7f 44 cc 10 	movdqu %xmm0,0x10(%r12,%r9,8)
  6b1966:	49 83 c1 fc          	add    $0xfffffffffffffffc,%r9
  6b196a:	49 83 c2 fc          	add    $0xfffffffffffffffc,%r10
  6b196e:	75 90                	jne    6b1900 <sqlite3VdbeExec+0x48a0>
  6b1970:	39 cf                	cmp    %ecx,%edi
  6b1972:	74 2f                	je     6b19a3 <sqlite3VdbeExec+0x4943>
  6b1974:	48 6b fa 38          	imul   $0x38,%rdx,%rdi
  6b1978:	48 01 f7             	add    %rsi,%rdi
  6b197b:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6b1980:	48 01 fe             	add    %rdi,%rsi
  6b1983:	48 83 c6 c8          	add    $0xffffffffffffffc8,%rsi
  6b1987:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
  6b198e:	00 00 
  6b1990:	49 89 74 d4 28       	mov    %rsi,0x28(%r12,%rdx,8)
  6b1995:	48 83 c6 c8          	add    $0xffffffffffffffc8,%rsi
  6b1999:	48 83 fa 01          	cmp    $0x1,%rdx
  6b199d:	48 8d 52 ff          	lea    -0x1(%rdx),%rdx
  6b19a1:	77 ed                	ja     6b1990 <sqlite3VdbeExec+0x4930>
  6b19a3:	ff 40 10             	incl   0x10(%rax)
  6b19a6:	49 8d 54 24 30       	lea    0x30(%r12),%rdx
  6b19ab:	41 83 7d 04 00       	cmpl   $0x0,0x4(%r13)
  6b19b0:	49 8b 44 24 08       	mov    0x8(%r12),%rax
  6b19b5:	0f b7 f1             	movzwl %cx,%esi
  6b19b8:	b9 18 00 00 00       	mov    $0x18,%ecx
  6b19bd:	41 b8 30 00 00 00    	mov    $0x30,%r8d
  6b19c3:	4c 0f 44 c1          	cmove  %rcx,%r8
  6b19c7:	4c 89 e7             	mov    %r12,%rdi
  6b19ca:	42 ff 14 00          	call   *(%rax,%r8,1)
  6b19ce:	41 83 7c 24 24 00    	cmpl   $0x0,0x24(%r12)
  6b19d4:	74 2b                	je     6b1a01 <sqlite3VdbeExec+0x49a1>
  6b19d6:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b19db:	7e 5d                	jle    6b1a3a <sqlite3VdbeExec+0x49da>
  6b19dd:	49 8b 3c 24          	mov    (%r12),%rdi
  6b19e1:	48 85 ff             	test   %rdi,%rdi
  6b19e4:	74 3c                	je     6b1a22 <sqlite3VdbeExec+0x49c2>
  6b19e6:	8b 47 14             	mov    0x14(%rdi),%eax
  6b19e9:	89 c1                	mov    %eax,%ecx
  6b19eb:	f7 d1                	not    %ecx
  6b19ed:	f7 c1 02 02 00 00    	test   $0x202,%ecx
  6b19f3:	75 1a                	jne    6b1a0f <sqlite3VdbeExec+0x49af>
  6b19f5:	80 7f 16 01          	cmpb   $0x1,0x16(%rdi)
  6b19f9:	75 14                	jne    6b1a0f <sqlite3VdbeExec+0x49af>
  6b19fb:	48 8b 57 08          	mov    0x8(%rdi),%rdx
  6b19ff:	eb 23                	jmp    6b1a24 <sqlite3VdbeExec+0x49c4>
  6b1a01:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1a06:	49 83 c5 18          	add    $0x18,%r13
  6b1a0a:	e9 11 b9 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1a0f:	a8 01                	test   $0x1,%al
  6b1a11:	75 0f                	jne    6b1a22 <sqlite3VdbeExec+0x49c2>
  6b1a13:	be 01 00 00 00       	mov    $0x1,%esi
  6b1a18:	e8 83 00 ff ff       	call   6a1aa0 <valueToText>
  6b1a1d:	48 89 c2             	mov    %rax,%rdx
  6b1a20:	eb 02                	jmp    6b1a24 <sqlite3VdbeExec+0x49c4>
  6b1a22:	31 d2                	xor    %edx,%edx
  6b1a24:	48 8d 35 1a 46 fa ff 	lea    -0x5b9e6(%rip),%rsi        # 656045 <.L.str.277>
  6b1a2b:	4c 89 f7             	mov    %r14,%rdi
  6b1a2e:	31 c0                	xor    %eax,%eax
  6b1a30:	e8 ab 10 ff ff       	call   6a2ae0 <sqlite3VdbeError>
  6b1a35:	41 8b 44 24 24       	mov    0x24(%r12),%eax
  6b1a3a:	41 80 7c 24 29 00    	cmpb   $0x0,0x29(%r12)
  6b1a40:	0f 84 e7 1f 00 00    	je     6b3a2d <sqlite3VdbeExec+0x69cd>
  6b1a46:	4d 89 f7             	mov    %r14,%r15
  6b1a49:	49 89 c6             	mov    %rax,%r14
  6b1a4c:	49 63 45 ec          	movslq -0x14(%r13),%rax
  6b1a50:	48 85 c0             	test   %rax,%rax
  6b1a53:	0f 84 c4 1f 00 00    	je     6b3a1d <sqlite3VdbeExec+0x69bd>
  6b1a59:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b1a5d:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b1a62:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6b1a66:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6b1a6b:	0f 84 9f 1f 00 00    	je     6b3a10 <sqlite3VdbeExec+0x69b0>
  6b1a71:	be 01 00 00 00       	mov    $0x1,%esi
  6b1a76:	e8 65 fb fe ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6b1a7b:	e9 9d 1f 00 00       	jmp    6b3a1d <sqlite3VdbeExec+0x69bd>
  6b1a80:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1a84:	49 83 c5 18          	add    $0x18,%r13
  6b1a88:	e9 93 b8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1a8d:	c7 44 24 60 01 00 00 	movl   $0x1,0x60(%rsp)
  6b1a94:	00 
  6b1a95:	31 c0                	xor    %eax,%eax
  6b1a97:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1a9b:	e9 0e 20 00 00       	jmp    6b3aae <sqlite3VdbeExec+0x6a4e>
  6b1aa0:	48 01 c8             	add    %rcx,%rax
  6b1aa3:	48 8b 00             	mov    (%rax),%rax
  6b1aa6:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b1aaa:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b1aaf:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b1ab3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1ab7:	e9 5b b8 ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6b1abc:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6b1ac2:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1ac6:	0f 84 d4 26 00 00    	je     6b41a0 <sqlite3VdbeExec+0x7140>
  6b1acc:	4c 89 e7             	mov    %r12,%rdi
  6b1acf:	e8 6c fa fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b1ad4:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1ad9:	49 83 c5 18          	add    $0x18,%r13
  6b1add:	e9 3e b8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1ae2:	48 8b 4a 18          	mov    0x18(%rdx),%rcx
  6b1ae6:	48 8b 49 50          	mov    0x50(%rcx),%rcx
  6b1aea:	8b 6c 81 24          	mov    0x24(%rcx,%rax,4),%ebp
  6b1aee:	0f cd                	bswap  %ebp
  6b1af0:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b1af4:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b1af8:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b1afc:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b1b00:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b1b05:	0f 84 c0 0b 00 00    	je     6b26cb <sqlite3VdbeExec+0x566b>
  6b1b0b:	4c 89 e7             	mov    %r12,%rdi
  6b1b0e:	e8 4d 5f 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b1b13:	48 63 c5             	movslq %ebp,%rax
  6b1b16:	e9 46 16 00 00       	jmp    6b3161 <sqlite3VdbeExec+0x6101>
  6b1b1b:	48 01 f0             	add    %rsi,%rax
  6b1b1e:	48 ff c9             	dec    %rcx
  6b1b21:	48 89 08             	mov    %rcx,(%rax)
  6b1b24:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1b28:	0f 85 e9 b7 ff ff    	jne    6ad317 <sqlite3VdbeExec+0x2b7>
  6b1b2e:	e9 39 3d 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b1b33:	a8 01                	test   $0x1,%al
  6b1b35:	0f 85 ce 1f 00 00    	jne    6b3b09 <sqlite3VdbeExec+0x6aa9>
  6b1b3b:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b1b40:	be 01 00 00 00       	mov    $0x1,%esi
  6b1b45:	e8 56 ff fe ff       	call   6a1aa0 <valueToText>
  6b1b4a:	48 89 c2             	mov    %rax,%rdx
  6b1b4d:	4c 89 f8             	mov    %r15,%rax
  6b1b50:	48 85 d2             	test   %rdx,%rdx
  6b1b53:	0f 84 b3 1f 00 00    	je     6b3b0c <sqlite3VdbeExec+0x6aac>
  6b1b59:	41 8b 75 04          	mov    0x4(%r13),%esi
  6b1b5d:	4c 89 e7             	mov    %r12,%rdi
  6b1b60:	48 8b 8c 24 d0 00 00 	mov    0xd0(%rsp),%rcx
  6b1b67:	00 
  6b1b68:	e8 a3 81 00 00       	call   6b9d10 <sqlite3VtabCallCreate>
  6b1b6d:	e9 9a 1f 00 00       	jmp    6b3b0c <sqlite3VdbeExec+0x6aac>
  6b1b72:	41 8b 55 0c          	mov    0xc(%r13),%edx
  6b1b76:	39 ca                	cmp    %ecx,%edx
  6b1b78:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1b7c:	0f 8d 95 b7 ff ff    	jge    6ad317 <sqlite3VdbeExec+0x2b7>
  6b1b82:	66 89 50 06          	mov    %dx,0x6(%rax)
  6b1b86:	49 83 c5 18          	add    $0x18,%r13
  6b1b8a:	e9 91 b7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1b8f:	48 01 c8             	add    %rcx,%rax
  6b1b92:	0f b6 70 17          	movzbl 0x17(%rax),%esi
  6b1b96:	f6 47 15 90          	testb  $0x90,0x15(%rdi)
  6b1b9a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1b9e:	0f 84 2e 26 00 00    	je     6b41d2 <sqlite3VdbeExec+0x7172>
  6b1ba4:	e8 37 fa fe ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6b1ba9:	49 83 c5 18          	add    $0x18,%r13
  6b1bad:	e9 6e b7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1bb2:	81 e2 ff f7 00 00    	and    $0xf7ff,%edx
  6b1bb8:	66 89 50 14          	mov    %dx,0x14(%rax)
  6b1bbc:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1bc0:	49 83 c5 18          	add    $0x18,%r13
  6b1bc4:	e9 57 b7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1bc9:	41 80 7d 16 01       	cmpb   $0x1,0x16(%r13)
  6b1bce:	74 15                	je     6b1be5 <sqlite3VdbeExec+0x4b85>
  6b1bd0:	4c 89 ef             	mov    %r13,%rdi
  6b1bd3:	be 01 00 00 00       	mov    $0x1,%esi
  6b1bd8:	e8 e3 db fc ff       	call   67f7c0 <sqlite3VdbeMemTranslate>
  6b1bdd:	85 c0                	test   %eax,%eax
  6b1bdf:	0f 85 eb 43 00 00    	jne    6b5fd0 <sqlite3VdbeExec+0x8f70>
  6b1be5:	49 8b 04 24          	mov    (%r12),%rax
  6b1be9:	49 8b 75 08          	mov    0x8(%r13),%rsi
  6b1bed:	4c 89 e7             	mov    %r12,%rdi
  6b1bf0:	ff 90 98 00 00 00    	call   *0x98(%rax)
  6b1bf6:	41 89 c7             	mov    %eax,%r15d
  6b1bf9:	41 f7 c6 00 00 00 04 	test   $0x4000000,%r14d
  6b1c00:	75 09                	jne    6b1c0b <sqlite3VdbeExec+0x4bab>
  6b1c02:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b1c07:	80 60 33 fb          	andb   $0xfb,0x33(%rax)
  6b1c0b:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1c10:	4c 89 f7             	mov    %r14,%rdi
  6b1c13:	4c 89 e6             	mov    %r12,%rsi
  6b1c16:	e8 f5 7e ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6b1c1b:	41 80 a6 c8 00 00 00 	andb   $0xfc,0xc8(%r14)
  6b1c22:	fc 
  6b1c23:	4c 89 f8             	mov    %r15,%rax
  6b1c26:	85 c0                	test   %eax,%eax
  6b1c28:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b1c2d:	0f 85 c9 43 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6b1c33:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b1c3a:	00 00 
  6b1c3c:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1c41:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1c45:	49 83 c5 18          	add    $0x18,%r13
  6b1c49:	e9 d2 b6 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1c4e:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b1c53:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b1c57:	49 01 84 24 f8 02 00 	add    %rax,0x2f8(%r12)
  6b1c5e:	00 
  6b1c5f:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1c63:	49 83 c5 18          	add    $0x18,%r13
  6b1c67:	e9 b4 b6 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1c6c:	48 8b 7e 08          	mov    0x8(%rsi),%rdi
  6b1c70:	8b 76 50             	mov    0x50(%rsi),%esi
  6b1c73:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b1c7a:	00 00 
  6b1c7c:	31 d2                	xor    %edx,%edx
  6b1c7e:	e8 bd a8 fe ff       	call   69c540 <sqlite3BtreeClearTable>
  6b1c83:	85 c0                	test   %eax,%eax
  6b1c85:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1c89:	0f 84 88 b6 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b1c8f:	e9 23 3e 00 00       	jmp    6b5ab7 <sqlite3VdbeExec+0x8a57>
  6b1c94:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6b1c9a:	0f 84 1f 26 00 00    	je     6b42bf <sqlite3VdbeExec+0x725f>
  6b1ca0:	4c 89 e7             	mov    %r12,%rdi
  6b1ca3:	e8 98 f8 fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b1ca8:	eb 92                	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6b1caa:	49 8b 45 f8          	mov    -0x8(%r13),%rax
  6b1cae:	48 83 c0 04          	add    $0x4,%rax
  6b1cb2:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1cb6:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6b1cba:	48 89 4c 24 38       	mov    %rcx,0x38(%rsp)
  6b1cbf:	48 85 c9             	test   %rcx,%rcx
  6b1cc2:	0f 8e 4f b6 ff ff    	jle    6ad317 <sqlite3VdbeExec+0x2b7>
  6b1cc8:	41 8b 4d 04          	mov    0x4(%r13),%ecx
  6b1ccc:	48 89 8c 24 b0 00 00 	mov    %rcx,0xb0(%rsp)
  6b1cd3:	00 
  6b1cd4:	41 8b 4d 08          	mov    0x8(%r13),%ecx
  6b1cd8:	89 4c 24 40          	mov    %ecx,0x40(%rsp)
  6b1cdc:	49 8b 6d 10          	mov    0x10(%r13),%rbp
  6b1ce0:	45 31 ff             	xor    %r15d,%r15d
  6b1ce3:	48 89 44 24 50       	mov    %rax,0x50(%rsp)
  6b1ce8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
  6b1cef:	00 
  6b1cf0:	48 85 c0             	test   %rax,%rax
  6b1cf3:	74 06                	je     6b1cfb <sqlite3VdbeExec+0x4c9b>
  6b1cf5:	42 8b 04 b8          	mov    (%rax,%r15,4),%eax
  6b1cf9:	eb 03                	jmp    6b1cfe <sqlite3VdbeExec+0x4c9e>
  6b1cfb:	44 89 f8             	mov    %r15d,%eax
  6b1cfe:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6b1d03:	4a 8b 54 fd 20       	mov    0x20(%rbp,%r15,8),%rdx
  6b1d08:	48 8b 4d 18          	mov    0x18(%rbp),%rcx
  6b1d0c:	46 0f b6 34 39       	movzbl (%rcx,%r15,1),%r14d
  6b1d11:	48 8b 8c 24 b0 00 00 	mov    0xb0(%rsp),%rcx
  6b1d18:	00 
  6b1d19:	01 c1                	add    %eax,%ecx
  6b1d1b:	4c 6b e1 38          	imul   $0x38,%rcx,%r12
  6b1d1f:	49 01 f4             	add    %rsi,%r12
  6b1d22:	03 44 24 40          	add    0x40(%rsp),%eax
  6b1d26:	4c 6b e8 38          	imul   $0x38,%rax,%r13
  6b1d2a:	49 01 f5             	add    %rsi,%r13
  6b1d2d:	4c 89 e7             	mov    %r12,%rdi
  6b1d30:	4c 89 ee             	mov    %r13,%rsi
  6b1d33:	e8 38 67 ff ff       	call   6a8470 <sqlite3MemCompare>
  6b1d38:	85 c0                	test   %eax,%eax
  6b1d3a:	0f 85 39 19 00 00    	jne    6b3679 <sqlite3VdbeExec+0x6619>
  6b1d40:	49 ff c7             	inc    %r15
  6b1d43:	4c 39 7c 24 38       	cmp    %r15,0x38(%rsp)
  6b1d48:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1d4d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1d52:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b1d57:	48 8b 44 24 50       	mov    0x50(%rsp),%rax
  6b1d5c:	75 92                	jne    6b1cf0 <sqlite3VdbeExec+0x4c90>
  6b1d5e:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6b1d65:	00 00 00 00 
  6b1d69:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1d6d:	49 83 c5 18          	add    $0x18,%r13
  6b1d71:	e9 aa b5 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1d76:	b8 01 00 00 00       	mov    $0x1,%eax
  6b1d7b:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b1d80:	41 83 fd 7f          	cmp    $0x7f,%r13d
  6b1d84:	4c 8b 4c 24 10       	mov    0x10(%rsp),%r9
  6b1d89:	74 18                	je     6b1da3 <sqlite3VdbeExec+0x4d43>
  6b1d8b:	44 89 ea             	mov    %r13d,%edx
  6b1d8e:	48 89 d6             	mov    %rdx,%rsi
  6b1d91:	48 c1 ee 07          	shr    $0x7,%rsi
  6b1d95:	ff c1                	inc    %ecx
  6b1d97:	48 81 fa ff 3f 00 00 	cmp    $0x3fff,%rdx
  6b1d9e:	48 89 f2             	mov    %rsi,%rdx
  6b1da1:	77 ee                	ja     6b1d91 <sqlite3VdbeExec+0x4d31>
  6b1da3:	41 01 cd             	add    %ecx,%r13d
  6b1da6:	41 81 fd 80 00 00 00 	cmp    $0x80,%r13d
  6b1dad:	72 1d                	jb     6b1dcc <sqlite3VdbeExec+0x4d6c>
  6b1daf:	49 63 d5             	movslq %r13d,%rdx
  6b1db2:	b8 01 00 00 00       	mov    $0x1,%eax
  6b1db7:	48 89 d6             	mov    %rdx,%rsi
  6b1dba:	48 c1 ee 07          	shr    $0x7,%rsi
  6b1dbe:	ff c0                	inc    %eax
  6b1dc0:	48 81 fa ff 3f 00 00 	cmp    $0x3fff,%rdx
  6b1dc7:	48 89 f2             	mov    %rsi,%rdx
  6b1dca:	77 ee                	ja     6b1dba <sqlite3VdbeExec+0x4d5a>
  6b1dcc:	31 d2                	xor    %edx,%edx
  6b1dce:	39 c1                	cmp    %eax,%ecx
  6b1dd0:	0f 9c c2             	setl   %dl
  6b1dd3:	41 01 d5             	add    %edx,%r13d
  6b1dd6:	49 63 ed             	movslq %r13d,%rbp
  6b1dd9:	49 01 eb             	add    %rbp,%r11
  6b1ddc:	4b 8d 04 13          	lea    (%r11,%r10,1),%rax
  6b1de0:	48 63 4f 20          	movslq 0x20(%rdi),%rcx
  6b1de4:	48 39 c8             	cmp    %rcx,%rax
  6b1de7:	7e 46                	jle    6b1e2f <sqlite3VdbeExec+0x4dcf>
  6b1de9:	49 63 89 88 00 00 00 	movslq 0x88(%r9),%rcx
  6b1df0:	48 39 c8             	cmp    %rcx,%rax
  6b1df3:	0f 8f 15 45 00 00    	jg     6b630e <sqlite3VdbeExec+0x92ae>
  6b1df9:	44 89 de             	mov    %r11d,%esi
  6b1dfc:	49 89 fe             	mov    %rdi,%r14
  6b1dff:	4c 89 54 24 40       	mov    %r10,0x40(%rsp)
  6b1e04:	4d 89 dc             	mov    %r11,%r12
  6b1e07:	e8 14 eb fe ff       	call   6a0920 <sqlite3VdbeMemClearAndResize>
  6b1e0c:	4d 89 e3             	mov    %r12,%r11
  6b1e0f:	4c 8b 54 24 40       	mov    0x40(%rsp),%r10
  6b1e14:	4c 89 f7             	mov    %r14,%rdi
  6b1e17:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b1e1c:	85 c0                	test   %eax,%eax
  6b1e1e:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1e23:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b1e28:	74 0d                	je     6b1e37 <sqlite3VdbeExec+0x4dd7>
  6b1e2a:	e9 94 43 00 00       	jmp    6b61c3 <sqlite3VdbeExec+0x9163>
  6b1e2f:	48 8b 47 28          	mov    0x28(%rdi),%rax
  6b1e33:	48 89 47 08          	mov    %rax,0x8(%rdi)
  6b1e37:	44 89 5f 10          	mov    %r11d,0x10(%rdi)
  6b1e3b:	66 c7 47 14 10 00    	movw   $0x10,0x14(%rdi)
  6b1e41:	4d 85 d2             	test   %r10,%r10
  6b1e44:	74 09                	je     6b1e4f <sqlite3VdbeExec+0x4def>
  6b1e46:	44 89 17             	mov    %r10d,(%rdi)
  6b1e49:	66 c7 47 14 10 04    	movw   $0x410,0x14(%rdi)
  6b1e4f:	4c 8b 67 08          	mov    0x8(%rdi),%r12
  6b1e53:	41 83 fd 7f          	cmp    $0x7f,%r13d
  6b1e57:	0f 8f c6 00 00 00    	jg     6b1f23 <sqlite3VdbeExec+0x4ec3>
  6b1e5d:	4d 8d 6c 24 01       	lea    0x1(%r12),%r13
  6b1e62:	41 88 2c 24          	mov    %bpl,(%r12)
  6b1e66:	e9 ce 00 00 00       	jmp    6b1f39 <sqlite3VdbeExec+0x4ed9>
  6b1e6b:	48 89 f7             	mov    %rsi,%rdi
  6b1e6e:	e8 6d 84 fc ff       	call   67a2e0 <sqlite3Malloc>
  6b1e73:	49 89 c4             	mov    %rax,%r12
  6b1e76:	48 85 c0             	test   %rax,%rax
  6b1e79:	0f 84 89 40 00 00    	je     6b5f08 <sqlite3VdbeExec+0x8ea8>
  6b1e7f:	49 8d 44 24 28       	lea    0x28(%r12),%rax
  6b1e84:	49 89 44 24 08       	mov    %rax,0x8(%r12)
  6b1e89:	4d 89 34 24          	mov    %r14,(%r12)
  6b1e8d:	41 0f b7 46 06       	movzwl 0x6(%r14),%eax
  6b1e92:	ff c0                	inc    %eax
  6b1e94:	66 41 89 44 24 1c    	mov    %ax,0x1c(%r12)
  6b1e9a:	49 8b 7f 30          	mov    0x30(%r15),%rdi
  6b1e9e:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  6b1ea3:	8b 70 10             	mov    0x10(%rax),%esi
  6b1ea6:	48 8b 50 08          	mov    0x8(%rax),%rdx
  6b1eaa:	4c 89 e1             	mov    %r12,%rcx
  6b1ead:	e8 6e 62 ff ff       	call   6a8120 <sqlite3VdbeRecordUnpack>
  6b1eb2:	41 c6 44 24 1e 00    	movb   $0x0,0x1e(%r12)
  6b1eb8:	49 8b 7f 28          	mov    0x28(%r15),%rdi
  6b1ebc:	49 8d 57 1c          	lea    0x1c(%r15),%rdx
  6b1ec0:	4c 89 e6             	mov    %r12,%rsi
  6b1ec3:	e8 98 35 fe ff       	call   695460 <sqlite3BtreeIndexMoveto>
  6b1ec8:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b1ecd:	4c 3b a7 e8 01 00 00 	cmp    0x1e8(%rdi),%r12
  6b1ed4:	73 1a                	jae    6b1ef0 <sqlite3VdbeExec+0x4e90>
  6b1ed6:	4c 3b a7 d8 01 00 00 	cmp    0x1d8(%rdi),%r12
  6b1edd:	0f 83 b3 17 00 00    	jae    6b3696 <sqlite3VdbeExec+0x6636>
  6b1ee3:	4c 3b a7 e0 01 00 00 	cmp    0x1e0(%rdi),%r12
  6b1eea:	0f 83 30 28 00 00    	jae    6b4720 <sqlite3VdbeExec+0x76c0>
  6b1ef0:	48 83 bf 00 03 00 00 	cmpq   $0x0,0x300(%rdi)
  6b1ef7:	00 
  6b1ef8:	0f 84 04 1a 00 00    	je     6b3902 <sqlite3VdbeExec+0x68a2>
  6b1efe:	4c 89 e6             	mov    %r12,%rsi
  6b1f01:	49 89 c6             	mov    %rax,%r14
  6b1f04:	e8 d7 85 fc ff       	call   67a4e0 <measureAllocationSize>
  6b1f09:	4c 89 f0             	mov    %r14,%rax
  6b1f0c:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b1f11:	e9 16 cc ff ff       	jmp    6aeb2c <sqlite3VdbeExec+0x1acc>
  6b1f16:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b1f1a:	49 83 c5 18          	add    $0x18,%r13
  6b1f1e:	e9 fd b3 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b1f23:	4c 89 e7             	mov    %r12,%rdi
  6b1f26:	48 89 ee             	mov    %rbp,%rsi
  6b1f29:	e8 d2 01 fd ff       	call   682100 <sqlite3PutVarint>
  6b1f2e:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b1f33:	4c 63 e8             	movslq %eax,%r13
  6b1f36:	4d 01 e5             	add    %r12,%r13
  6b1f39:	49 01 ec             	add    %rbp,%r12
  6b1f3c:	31 ed                	xor    %ebp,%ebp
  6b1f3e:	eb 12                	jmp    6b1f52 <sqlite3VdbeExec+0x4ef2>
  6b1f40:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b1f45:	48 83 c5 38          	add    $0x38,%rbp
  6b1f49:	49 39 e8             	cmp    %rbp,%r8
  6b1f4c:	0f 84 54 3a 00 00    	je     6b59a6 <sqlite3VdbeExec+0x8946>
  6b1f52:	41 8b 74 2f 24       	mov    0x24(%r15,%rbp,1),%esi
  6b1f57:	48 83 fe 07          	cmp    $0x7,%rsi
  6b1f5b:	0f 87 7f 00 00 00    	ja     6b1fe0 <sqlite3VdbeExec+0x4f80>
  6b1f61:	41 88 75 00          	mov    %sil,0x0(%r13)
  6b1f65:	49 ff c5             	inc    %r13
  6b1f68:	48 85 f6             	test   %rsi,%rsi
  6b1f6b:	74 d8                	je     6b1f45 <sqlite3VdbeExec+0x4ee5>
  6b1f6d:	49 8b 04 2f          	mov    (%r15,%rbp,1),%rax
  6b1f71:	48 8d 0d 78 8c b9 ff 	lea    -0x467388(%rip),%rcx        # 24abf0 <sqlite3SmallTypeSizes>
  6b1f78:	0f b6 0c 0e          	movzbl (%rsi,%rcx,1),%ecx
  6b1f7c:	8d 51 ff             	lea    -0x1(%rcx),%edx
  6b1f7f:	83 fa 05             	cmp    $0x5,%edx
  6b1f82:	77 10                	ja     6b1f94 <sqlite3VdbeExec+0x4f34>
  6b1f84:	48 8d 35 55 5d b9 ff 	lea    -0x46a2ab(%rip),%rsi        # 247ce0 <RuntimeSpecializeableIR_data+0x1fdff0>
  6b1f8b:	48 63 14 96          	movslq (%rsi,%rdx,4),%rdx
  6b1f8f:	48 01 f2             	add    %rsi,%rdx
  6b1f92:	ff e2                	jmp    *%rdx
  6b1f94:	89 c2                	mov    %eax,%edx
  6b1f96:	66 c1 c2 08          	rol    $0x8,%dx
  6b1f9a:	66 41 89 54 24 06    	mov    %dx,0x6(%r12)
  6b1fa0:	48 c1 e8 10          	shr    $0x10,%rax
  6b1fa4:	89 c2                	mov    %eax,%edx
  6b1fa6:	66 c1 c2 08          	rol    $0x8,%dx
  6b1faa:	66 41 89 54 24 04    	mov    %dx,0x4(%r12)
  6b1fb0:	48 c1 e8 10          	shr    $0x10,%rax
  6b1fb4:	41 88 44 24 03       	mov    %al,0x3(%r12)
  6b1fb9:	48 c1 e8 08          	shr    $0x8,%rax
  6b1fbd:	41 88 44 24 02       	mov    %al,0x2(%r12)
  6b1fc2:	48 c1 e8 08          	shr    $0x8,%rax
  6b1fc6:	41 88 44 24 01       	mov    %al,0x1(%r12)
  6b1fcb:	48 c1 e8 08          	shr    $0x8,%rax
  6b1fcf:	41 88 04 24          	mov    %al,(%r12)
  6b1fd3:	49 01 cc             	add    %rcx,%r12
  6b1fd6:	e9 6a ff ff ff       	jmp    6b1f45 <sqlite3VdbeExec+0x4ee5>
  6b1fdb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  6b1fe0:	83 fe 7f             	cmp    $0x7f,%esi
  6b1fe3:	77 3d                	ja     6b2022 <sqlite3VdbeExec+0x4fc2>
  6b1fe5:	41 88 75 00          	mov    %sil,0x0(%r13)
  6b1fe9:	49 ff c5             	inc    %r13
  6b1fec:	83 fe 0e             	cmp    $0xe,%esi
  6b1fef:	0f 82 50 ff ff ff    	jb     6b1f45 <sqlite3VdbeExec+0x4ee5>
  6b1ff5:	49 63 54 2f 10       	movslq 0x10(%r15,%rbp,1),%rdx
  6b1ffa:	48 85 d2             	test   %rdx,%rdx
  6b1ffd:	0f 8e 42 ff ff ff    	jle    6b1f45 <sqlite3VdbeExec+0x4ee5>
  6b2003:	49 8b 74 2f 08       	mov    0x8(%r15,%rbp,1),%rsi
  6b2008:	4c 89 e7             	mov    %r12,%rdi
  6b200b:	e8 a0 85 fa ff       	call   65a5b0 <memcpy$plt>
  6b2010:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b2015:	49 63 44 2f 10       	movslq 0x10(%r15,%rbp,1),%rax
  6b201a:	49 01 c4             	add    %rax,%r12
  6b201d:	e9 23 ff ff ff       	jmp    6b1f45 <sqlite3VdbeExec+0x4ee5>
  6b2022:	81 fe ff 3f 00 00    	cmp    $0x3fff,%esi
  6b2028:	77 1a                	ja     6b2044 <sqlite3VdbeExec+0x4fe4>
  6b202a:	89 f0                	mov    %esi,%eax
  6b202c:	c1 e8 07             	shr    $0x7,%eax
  6b202f:	0c 80                	or     $0x80,%al
  6b2031:	41 88 45 00          	mov    %al,0x0(%r13)
  6b2035:	40 80 e6 7f          	and    $0x7f,%sil
  6b2039:	41 88 75 01          	mov    %sil,0x1(%r13)
  6b203d:	b8 02 00 00 00       	mov    $0x2,%eax
  6b2042:	eb 0a                	jmp    6b204e <sqlite3VdbeExec+0x4fee>
  6b2044:	4c 89 ef             	mov    %r13,%rdi
  6b2047:	e8 e4 00 fd ff       	call   682130 <putVarint64>
  6b204c:	48 98                	cltq
  6b204e:	49 01 c5             	add    %rax,%r13
  6b2051:	49 63 54 2f 10       	movslq 0x10(%r15,%rbp,1),%rdx
  6b2056:	48 85 d2             	test   %rdx,%rdx
  6b2059:	0f 84 e1 fe ff ff    	je     6b1f40 <sqlite3VdbeExec+0x4ee0>
  6b205f:	49 8b 74 2f 08       	mov    0x8(%r15,%rbp,1),%rsi
  6b2064:	4c 89 e7             	mov    %r12,%rdi
  6b2067:	e8 44 85 fa ff       	call   65a5b0 <memcpy$plt>
  6b206c:	49 63 44 2f 10       	movslq 0x10(%r15,%rbp,1),%rax
  6b2071:	49 01 c4             	add    %rax,%r12
  6b2074:	e9 c7 fe ff ff       	jmp    6b1f40 <sqlite3VdbeExec+0x4ee0>
  6b2079:	31 c0                	xor    %eax,%eax
  6b207b:	e9 93 02 00 00       	jmp    6b2313 <sqlite3VdbeExec+0x52b3>
  6b2080:	66 41 c7 47 14 01 00 	movw   $0x1,0x14(%r15)
  6b2087:	4c 89 7c 24 60       	mov    %r15,0x60(%rsp)
  6b208c:	4c 89 64 24 70       	mov    %r12,0x70(%rsp)
  6b2091:	4c 89 74 24 68       	mov    %r14,0x68(%rsp)
  6b2096:	49 8b 44 24 18       	mov    0x18(%r12),%rax
  6b209b:	0f b6 40 64          	movzbl 0x64(%rax),%eax
  6b209f:	88 84 24 88 00 00 00 	mov    %al,0x88(%rsp)
  6b20a6:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b20ab:	41 ff 56 28          	call   *0x28(%r14)
  6b20af:	8b 8c 24 84 00 00 00 	mov    0x84(%rsp),%ecx
  6b20b6:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b20ba:	4c 6b e0 38          	imul   $0x38,%rax,%r12
  6b20be:	48 89 c8             	mov    %rcx,%rax
  6b20c1:	4c 03 64 24 28       	add    0x28(%rsp),%r12
  6b20c6:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b20cb:	85 c0                	test   %eax,%eax
  6b20cd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b20d1:	0f 85 06 3a 00 00    	jne    6b5add <sqlite3VdbeExec+0x8a7d>
  6b20d7:	41 f6 44 24 14 02    	testb  $0x2,0x14(%r12)
  6b20dd:	75 0f                	jne    6b20ee <sqlite3VdbeExec+0x508e>
  6b20df:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b20e4:	41 88 44 24 16       	mov    %al,0x16(%r12)
  6b20e9:	e9 a6 30 00 00       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6b20ee:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b20f5:	00 00 
  6b20f7:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b20fc:	41 3a 44 24 16       	cmp    0x16(%r12),%al
  6b2101:	75 0e                	jne    6b2111 <sqlite3VdbeExec+0x50b1>
  6b2103:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2108:	49 83 c5 18          	add    $0x18,%r13
  6b210c:	e9 0f b2 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2111:	0f b6 f0             	movzbl %al,%esi
  6b2114:	4c 89 e7             	mov    %r12,%rdi
  6b2117:	e8 a4 d6 fc ff       	call   67f7c0 <sqlite3VdbeMemTranslate>
  6b211c:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2121:	49 83 c5 18          	add    $0x18,%r13
  6b2125:	e9 f6 b1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b212a:	c7 44 24 60 01 00 00 	movl   $0x1,0x60(%rsp)
  6b2131:	00 
  6b2132:	41 b8 01 00 00 00    	mov    $0x1,%r8d
  6b2138:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b213d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2142:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b2147:	e9 e9 04 00 00       	jmp    6b2635 <sqlite3VdbeExec+0x55d5>
  6b214c:	8b 44 24 38          	mov    0x38(%rsp),%eax
  6b2150:	f7 d0                	not    %eax
  6b2152:	41 03 84 24 e8 02 00 	add    0x2e8(%r12),%eax
  6b2159:	00 
  6b215a:	89 44 24 38          	mov    %eax,0x38(%rsp)
  6b215e:	41 8b 44 24 28       	mov    0x28(%r12),%eax
  6b2163:	41 83 ff 02          	cmp    $0x2,%r15d
  6b2167:	48 89 6c 24 50       	mov    %rbp,0x50(%rsp)
  6b216c:	75 76                	jne    6b21e4 <sqlite3VdbeExec+0x5184>
  6b216e:	41 8b 54 24 2c       	mov    0x2c(%r12),%edx
  6b2173:	83 e2 01             	and    $0x1,%edx
  6b2176:	85 c0                	test   %eax,%eax
  6b2178:	0f 8e 2f 1c 00 00    	jle    6b3dad <sqlite3VdbeExec+0x6d4d>
  6b217e:	44 89 fd             	mov    %r15d,%ebp
  6b2181:	89 94 24 b8 00 00 00 	mov    %edx,0xb8(%rsp)
  6b2188:	41 89 d5             	mov    %edx,%r13d
  6b218b:	41 83 f5 01          	xor    $0x1,%r13d
  6b218f:	41 be 08 00 00 00    	mov    $0x8,%r14d
  6b2195:	45 31 ff             	xor    %r15d,%r15d
  6b2198:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6b219d:	4a 8b 3c 30          	mov    (%rax,%r14,1),%rdi
  6b21a1:	be 04 02 00 00       	mov    $0x204,%esi
  6b21a6:	44 89 ea             	mov    %r13d,%edx
  6b21a9:	e8 b2 1c fe ff       	call   693e60 <sqlite3BtreeTripAllCursors>
  6b21ae:	85 c0                	test   %eax,%eax
  6b21b0:	0f 85 06 39 00 00    	jne    6b5abc <sqlite3VdbeExec+0x8a5c>
  6b21b6:	49 ff c7             	inc    %r15
  6b21b9:	49 63 44 24 28       	movslq 0x28(%r12),%rax
  6b21be:	49 83 c6 20          	add    $0x20,%r14
  6b21c2:	49 39 c7             	cmp    %rax,%r15
  6b21c5:	7c d1                	jl     6b2198 <sqlite3VdbeExec+0x5138>
  6b21c7:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b21ce:	00 00 
  6b21d0:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b21d5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b21da:	41 89 ef             	mov    %ebp,%r15d
  6b21dd:	8b 94 24 b8 00 00 00 	mov    0xb8(%rsp),%edx
  6b21e4:	85 c0                	test   %eax,%eax
  6b21e6:	0f 8e c1 1b 00 00    	jle    6b3dad <sqlite3VdbeExec+0x6d4d>
  6b21ec:	89 94 24 b8 00 00 00 	mov    %edx,0xb8(%rsp)
  6b21f3:	44 89 fd             	mov    %r15d,%ebp
  6b21f6:	41 be 08 00 00 00    	mov    $0x8,%r14d
  6b21fc:	45 31 ff             	xor    %r15d,%r15d
  6b21ff:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6b2204:	4a 8b 3c 30          	mov    (%rax,%r14,1),%rdi
  6b2208:	89 ee                	mov    %ebp,%esi
  6b220a:	8b 54 24 38          	mov    0x38(%rsp),%edx
  6b220e:	e8 8d 1f fe ff       	call   6941a0 <sqlite3BtreeSavepoint>
  6b2213:	85 c0                	test   %eax,%eax
  6b2215:	0f 85 d1 37 00 00    	jne    6b59ec <sqlite3VdbeExec+0x898c>
  6b221b:	49 ff c7             	inc    %r15
  6b221e:	49 63 44 24 28       	movslq 0x28(%r12),%rax
  6b2223:	49 83 c6 20          	add    $0x20,%r14
  6b2227:	49 39 c7             	cmp    %rax,%r15
  6b222a:	7c d3                	jl     6b21ff <sqlite3VdbeExec+0x519f>
  6b222c:	31 c0                	xor    %eax,%eax
  6b222e:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b2233:	41 89 ef             	mov    %ebp,%r15d
  6b2236:	8b 94 24 b8 00 00 00 	mov    0xb8(%rsp),%edx
  6b223d:	e9 70 1b 00 00       	jmp    6b3db2 <sqlite3VdbeExec+0x6d52>
  6b2242:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b2248:	e9 47 2f 00 00       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6b224d:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b2252:	48 8d 3c 11          	lea    (%rcx,%rdx,1),%rdi
  6b2256:	89 c6                	mov    %eax,%esi
  6b2258:	83 e6 fc             	and    $0xfffffffc,%esi
  6b225b:	89 c1                	mov    %eax,%ecx
  6b225d:	83 e1 03             	and    $0x3,%ecx
  6b2260:	49 89 c0             	mov    %rax,%r8
  6b2263:	49 89 f1             	mov    %rsi,%r9
  6b2266:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
  6b226d:	00 00 00 
  6b2270:	66 49 0f 6e c0       	movq   %r8,%xmm0
  6b2275:	66 0f 70 c0 44       	pshufd $0x44,%xmm0,%xmm0
  6b227a:	66 0f 6f 0d 6e 91 f9 	movdqa -0x66e92(%rip),%xmm1        # 64b3f0 <.LCPI830_2>
  6b2281:	ff 
  6b2282:	66 0f 6f d0          	movdqa %xmm0,%xmm2
  6b2286:	66 0f f4 d1          	pmuludq %xmm1,%xmm2
  6b228a:	66 0f 73 d0 20       	psrlq  $0x20,%xmm0
  6b228f:	66 0f f4 c1          	pmuludq %xmm1,%xmm0
  6b2293:	66 0f 73 f0 20       	psllq  $0x20,%xmm0
  6b2298:	66 48 0f 6e cf       	movq   %rdi,%xmm1
  6b229d:	66 0f 70 c9 44       	pshufd $0x44,%xmm1,%xmm1
  6b22a2:	66 0f d4 ca          	paddq  %xmm2,%xmm1
  6b22a6:	66 0f d4 c8          	paddq  %xmm0,%xmm1
  6b22aa:	66 0f 6f 05 1e 8f f9 	movdqa -0x670e2(%rip),%xmm0        # 64b1d0 <.LCPI830_3>
  6b22b1:	ff 
  6b22b2:	66 0f d4 c1          	paddq  %xmm1,%xmm0
  6b22b6:	66 0f d4 0d c2 8f f9 	paddq  -0x6703e(%rip),%xmm1        # 64b280 <.LCPI830_4>
  6b22bd:	ff 
  6b22be:	66 0f 70 c0 4e       	pshufd $0x4e,%xmm0,%xmm0
  6b22c3:	f3 43 0f 7f 44 c4 20 	movdqu %xmm0,0x20(%r12,%r8,8)
  6b22ca:	66 0f 70 c1 4e       	pshufd $0x4e,%xmm1,%xmm0
  6b22cf:	f3 43 0f 7f 44 c4 10 	movdqu %xmm0,0x10(%r12,%r8,8)
  6b22d6:	49 83 c0 fc          	add    $0xfffffffffffffffc,%r8
  6b22da:	49 83 c1 fc          	add    $0xfffffffffffffffc,%r9
  6b22de:	75 90                	jne    6b2270 <sqlite3VdbeExec+0x5210>
  6b22e0:	39 c6                	cmp    %eax,%esi
  6b22e2:	74 2f                	je     6b2313 <sqlite3VdbeExec+0x52b3>
  6b22e4:	48 6b f1 38          	imul   $0x38,%rcx,%rsi
  6b22e8:	48 01 d6             	add    %rdx,%rsi
  6b22eb:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b22f0:	48 01 f2             	add    %rsi,%rdx
  6b22f3:	48 83 c2 c8          	add    $0xffffffffffffffc8,%rdx
  6b22f7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
  6b22fe:	00 00 
  6b2300:	49 89 54 cc 28       	mov    %rdx,0x28(%r12,%rcx,8)
  6b2305:	48 83 c2 c8          	add    $0xffffffffffffffc8,%rdx
  6b2309:	48 83 f9 01          	cmp    $0x1,%rcx
  6b230d:	48 8d 49 ff          	lea    -0x1(%rcx),%rcx
  6b2311:	77 ed                	ja     6b2300 <sqlite3VdbeExec+0x52a0>
  6b2313:	b9 40 f2 ff ff       	mov    $0xfffff240,%ecx
  6b2318:	41 23 4d 14          	and    0x14(%r13),%ecx
  6b231c:	ff c1                	inc    %ecx
  6b231e:	66 41 89 4d 14       	mov    %cx,0x14(%r13)
  6b2323:	49 8b 4c 24 08       	mov    0x8(%r12),%rcx
  6b2328:	0f b7 f0             	movzwl %ax,%esi
  6b232b:	49 8d 54 24 30       	lea    0x30(%r12),%rdx
  6b2330:	4c 89 e7             	mov    %r12,%rdi
  6b2333:	ff 51 18             	call   *0x18(%rcx)
  6b2336:	41 83 7c 24 24 00    	cmpl   $0x0,0x24(%r12)
  6b233c:	0f 84 b3 22 00 00    	je     6b45f5 <sqlite3VdbeExec+0x7595>
  6b2342:	7e 26                	jle    6b236a <sqlite3VdbeExec+0x530a>
  6b2344:	4c 89 ef             	mov    %r13,%rdi
  6b2347:	e8 24 7b ff ff       	call   6a9e70 <sqlite3_value_text>
  6b234c:	48 8d 35 f2 3c fa ff 	lea    -0x5c30e(%rip),%rsi        # 656045 <.L.str.277>
  6b2353:	4c 89 f7             	mov    %r14,%rdi
  6b2356:	48 89 c2             	mov    %rax,%rdx
  6b2359:	31 c0                	xor    %eax,%eax
  6b235b:	e8 80 07 ff ff       	call   6a2ae0 <sqlite3VdbeError>
  6b2360:	41 8b 44 24 24       	mov    0x24(%r12),%eax
  6b2365:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b236a:	41 8b 54 24 20       	mov    0x20(%r12),%edx
  6b236f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2374:	41 8b 4d 04          	mov    0x4(%r13),%ecx
  6b2378:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b237d:	48 8b b4 24 20 01 00 	mov    0x120(%rsp),%rsi
  6b2384:	00 
  6b2385:	e8 c6 32 ff ff       	call   6a5650 <sqlite3VdbeDeleteAuxData>
  6b238a:	41 c7 44 24 24 00 00 	movl   $0x0,0x24(%r12)
  6b2391:	00 00 
  6b2393:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b2398:	e9 c8 16 00 00       	jmp    6b3a65 <sqlite3VdbeExec+0x6a05>
  6b239d:	4d 8b 67 08          	mov    0x8(%r15),%r12
  6b23a1:	41 8b 6c 24 58       	mov    0x58(%r12),%ebp
  6b23a6:	41 0f b7 44 24 5c    	movzwl 0x5c(%r12),%eax
  6b23ac:	41 8b 49 10          	mov    0x10(%r9),%ecx
  6b23b0:	e9 c2 23 00 00       	jmp    6b4777 <sqlite3VdbeExec+0x7717>
  6b23b5:	a8 01                	test   $0x1,%al
  6b23b7:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b23bb:	0f 85 f8 1b 00 00    	jne    6b3fb9 <sqlite3VdbeExec+0x6f59>
  6b23c1:	a8 08                	test   $0x8,%al
  6b23c3:	0f 85 eb 28 00 00    	jne    6b4cb4 <sqlite3VdbeExec+0x7c54>
  6b23c9:	a8 12                	test   $0x12,%al
  6b23cb:	0f 84 9b 34 00 00    	je     6b586c <sqlite3VdbeExec+0x880c>
  6b23d1:	e8 ca eb fe ff       	call   6a0fa0 <memRealValue>
  6b23d6:	e9 dd 28 00 00       	jmp    6b4cb8 <sqlite3VdbeExec+0x7c58>
  6b23db:	4c 89 54 24 50       	mov    %r10,0x50(%rsp)
  6b23e0:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b23e5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b23ea:	e9 4a 1c 00 00       	jmp    6b4039 <sqlite3VdbeExec+0x6fd9>
  6b23ef:	c7 84 24 b0 00 00 00 	movl   $0x1,0xb0(%rsp)
  6b23f6:	01 00 00 00 
  6b23fa:	a8 01                	test   $0x1,%al
  6b23fc:	0f 85 c7 1b 00 00    	jne    6b3fc9 <sqlite3VdbeExec+0x6f69>
  6b2402:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2407:	e9 5a 1c 00 00       	jmp    6b4066 <sqlite3VdbeExec+0x7006>
  6b240c:	83 e1 2e             	and    $0x2e,%ecx
  6b240f:	83 f9 02             	cmp    $0x2,%ecx
  6b2412:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2417:	75 18                	jne    6b2431 <sqlite3VdbeExec+0x53d1>
  6b2419:	4c 89 c7             	mov    %r8,%rdi
  6b241c:	31 f6                	xor    %esi,%esi
  6b241e:	e8 fd a9 ff ff       	call   6ace20 <applyNumericAffinity>
  6b2423:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
  6b2428:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b242d:	0f b7 6f 14          	movzwl 0x14(%rdi),%ebp
  6b2431:	89 e8                	mov    %ebp,%eax
  6b2433:	83 e0 2e             	and    $0x2e,%eax
  6b2436:	66 83 f8 02          	cmp    $0x2,%ax
  6b243a:	0f 85 f9 1b 00 00    	jne    6b4039 <sqlite3VdbeExec+0x6fd9>
  6b2440:	31 f6                	xor    %esi,%esi
  6b2442:	e8 d9 a9 ff ff       	call   6ace20 <applyNumericAffinity>
  6b2447:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
  6b244c:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b2451:	e9 e3 1b 00 00       	jmp    6b4039 <sqlite3VdbeExec+0x6fd9>
  6b2456:	31 c0                	xor    %eax,%eax
  6b2458:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b245c:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b2460:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b2464:	48 03 4c 24 28       	add    0x28(%rsp),%rcx
  6b2469:	48 63 71 10          	movslq 0x10(%rcx),%rsi
  6b246d:	48 c1 e6 03          	shl    $0x3,%rsi
  6b2471:	48 89 c2             	mov    %rax,%rdx
  6b2474:	48 09 f2             	or     %rsi,%rdx
  6b2477:	48 c1 ea 20          	shr    $0x20,%rdx
  6b247b:	74 07                	je     6b2484 <sqlite3VdbeExec+0x5424>
  6b247d:	31 d2                	xor    %edx,%edx
  6b247f:	48 f7 f6             	div    %rsi
  6b2482:	eb 04                	jmp    6b2488 <sqlite3VdbeExec+0x5428>
  6b2484:	31 d2                	xor    %edx,%edx
  6b2486:	f7 f6                	div    %esi
  6b2488:	48 8b 41 08          	mov    0x8(%rcx),%rax
  6b248c:	48 89 d1             	mov    %rdx,%rcx
  6b248f:	48 c1 e9 03          	shr    $0x3,%rcx
  6b2493:	0f b6 04 08          	movzbl (%rax,%rcx,1),%eax
  6b2497:	83 e2 07             	and    $0x7,%edx
  6b249a:	0f a3 d0             	bt     %edx,%eax
  6b249d:	73 10                	jae    6b24af <sqlite3VdbeExec+0x544f>
  6b249f:	41 ff 86 f0 00 00 00 	incl   0xf0(%r14)
  6b24a6:	49 83 c5 18          	add    $0x18,%r13
  6b24aa:	e9 71 ae ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b24af:	41 ff 86 f4 00 00 00 	incl   0xf4(%r14)
  6b24b6:	e9 b1 33 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b24bb:	49 83 7e 50 00       	cmpq   $0x0,0x50(%r14)
  6b24c0:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b24c4:	0f 85 4d ae ff ff    	jne    6ad317 <sqlite3VdbeExec+0x2b7>
  6b24ca:	49 83 bc 24 f8 02 00 	cmpq   $0x0,0x2f8(%r12)
  6b24d1:	00 00 
  6b24d3:	0f 85 3e ae ff ff    	jne    6ad317 <sqlite3VdbeExec+0x2b7>
  6b24d9:	e9 8e 33 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b24de:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b24e2:	49 83 c5 18          	add    $0x18,%r13
  6b24e6:	e9 35 ae ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b24eb:	49 bc ff ff ff ff ff 	movabs $0x7fffffffffffffff,%r12
  6b24f2:	ff ff 7f 
  6b24f5:	f2 0f 10 00          	movsd  (%rax),%xmm0
  6b24f9:	f2 0f 10 0d 2f 90 f9 	movsd  -0x66fd1(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6b2500:	ff 
  6b2501:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b2505:	0f 86 e7 1b 00 00    	jbe    6b40f2 <sqlite3VdbeExec+0x7092>
  6b250b:	49 ff c4             	inc    %r12
  6b250e:	e9 bd af ff ff       	jmp    6ad4d0 <sqlite3VdbeExec+0x470>
  6b2513:	48 b8 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rax
  6b251a:	ff ff 7f 
  6b251d:	f2 41 0f 10 45 00    	movsd  0x0(%r13),%xmm0
  6b2523:	f2 0f 10 0d 05 90 f9 	movsd  -0x66ffb(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6b252a:	ff 
  6b252b:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b252f:	0f 86 d5 1b 00 00    	jbe    6b410a <sqlite3VdbeExec+0x70aa>
  6b2535:	48 ff c0             	inc    %rax
  6b2538:	e9 a2 af ff ff       	jmp    6ad4df <sqlite3VdbeExec+0x47f>
  6b253d:	a8 08                	test   $0x8,%al
  6b253f:	0f 85 e2 1b 00 00    	jne    6b4127 <sqlite3VdbeExec+0x70c7>
  6b2545:	a8 12                	test   $0x12,%al
  6b2547:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b254b:	0f 84 f6 26 00 00    	je     6b4c47 <sqlite3VdbeExec+0x7be7>
  6b2551:	49 83 7c 24 08 00    	cmpq   $0x0,0x8(%r12)
  6b2557:	0f 84 ea 26 00 00    	je     6b4c47 <sqlite3VdbeExec+0x7be7>
  6b255d:	4c 89 e7             	mov    %r12,%rdi
  6b2560:	e8 eb e9 fe ff       	call   6a0f50 <memIntValue>
  6b2565:	41 0f b7 4c 24 14    	movzwl 0x14(%r12),%ecx
  6b256b:	e9 93 ba ff ff       	jmp    6ae003 <sqlite3VdbeExec+0xfa3>
  6b2570:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2574:	49 83 c5 18          	add    $0x18,%r13
  6b2578:	e9 a3 ad ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b257d:	0f b7 47 56          	movzwl 0x56(%rdi),%eax
  6b2581:	66 85 c0             	test   %ax,%ax
  6b2584:	0f 84 2b bb ff ff    	je     6ae0b5 <sqlite3VdbeExec+0x1055>
  6b258a:	48 8b 8f 88 00 00 00 	mov    0x88(%rdi),%rcx
  6b2591:	80 79 08 00          	cmpb   $0x0,0x8(%rcx)
  6b2595:	0f 84 1a bb ff ff    	je     6ae0b5 <sqlite3VdbeExec+0x1055>
  6b259b:	ff c8                	dec    %eax
  6b259d:	66 89 47 56          	mov    %ax,0x56(%rdi)
  6b25a1:	41 c7 44 24 18 00 00 	movl   $0x0,0x18(%r12)
  6b25a8:	00 00 
  6b25aa:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b25ae:	e9 54 c3 ff ff       	jmp    6ae907 <sqlite3VdbeExec+0x18a7>
  6b25b3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b25b7:	49 83 c5 18          	add    $0x18,%r13
  6b25bb:	e9 60 ad ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b25c0:	48 8b 46 10          	mov    0x10(%rsi),%rax
  6b25c4:	48 89 47 10          	mov    %rax,0x10(%rdi)
  6b25c8:	66 0f 10 06          	movupd (%rsi),%xmm0
  6b25cc:	66 0f 11 07          	movupd %xmm0,(%rdi)
  6b25d0:	f6 46 15 20          	testb  $0x20,0x15(%rsi)
  6b25d4:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b25d8:	0f 85 39 ad ff ff    	jne    6ad317 <sqlite3VdbeExec+0x2b7>
  6b25de:	b8 ff 8f ff ff       	mov    $0xffff8fff,%eax
  6b25e3:	23 47 14             	and    0x14(%rdi),%eax
  6b25e6:	0d 00 40 00 00       	or     $0x4000,%eax
  6b25eb:	66 89 47 14          	mov    %ax,0x14(%rdi)
  6b25ef:	49 83 c5 18          	add    $0x18,%r13
  6b25f3:	e9 28 ad ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b25f8:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b25ff:	00 
  6b2600:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2604:	49 6b fe 38          	imul   $0x38,%r14,%rdi
  6b2608:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6b260d:	f6 47 14 01          	testb  $0x1,0x14(%rdi)
  6b2611:	0f 85 54 17 00 00    	jne    6b3d6b <sqlite3VdbeExec+0x6d0b>
  6b2617:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b261e:	00 
  6b261f:	e8 ac e8 fe ff       	call   6a0ed0 <sqlite3VdbeIntValue>
  6b2624:	48 f7 d0             	not    %rax
  6b2627:	49 89 04 24          	mov    %rax,(%r12)
  6b262b:	e9 28 17 00 00       	jmp    6b3d58 <sqlite3VdbeExec+0x6cf8>
  6b2630:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2635:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b2639:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b263d:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b2642:	4c 8d 3c 01          	lea    (%rcx,%rax,1),%r15
  6b2646:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6b264b:	0f 84 0b 0d 00 00    	je     6b335c <sqlite3VdbeExec+0x62fc>
  6b2651:	4c 89 ff             	mov    %r15,%rdi
  6b2654:	4c 89 c6             	mov    %r8,%rsi
  6b2657:	e8 84 ef fe ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6b265c:	e9 05 0d 00 00       	jmp    6b3366 <sqlite3VdbeExec+0x6306>
  6b2661:	31 d2                	xor    %edx,%edx
  6b2663:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2667:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b266b:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b266f:	48 8b 0c c8          	mov    (%rax,%rcx,8),%rcx
  6b2673:	49 63 75 08          	movslq 0x8(%r13),%rsi
  6b2677:	48 8b 04 f0          	mov    (%rax,%rsi,8),%rax
  6b267b:	48 8b 79 28          	mov    0x28(%rcx),%rdi
  6b267f:	48 8b 70 28          	mov    0x28(%rax),%rsi
  6b2683:	e8 d8 7c fe ff       	call   69a360 <sqlite3BtreeTransferRow>
  6b2688:	85 c0                	test   %eax,%eax
  6b268a:	0f 84 87 ac ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b2690:	e9 22 34 00 00       	jmp    6b5ab7 <sqlite3VdbeExec+0x8a57>
  6b2695:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b269c:	00 
  6b269d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b26a1:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b26a5:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b26a9:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b26ad:	48 8b 48 10          	mov    0x10(%rax),%rcx
  6b26b1:	48 8d 51 01          	lea    0x1(%rcx),%rdx
  6b26b5:	48 89 50 10          	mov    %rdx,0x10(%rax)
  6b26b9:	49 89 0c 24          	mov    %rcx,(%r12)
  6b26bd:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b26c2:	49 83 c5 18          	add    $0x18,%r13
  6b26c6:	e9 55 ac ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b26cb:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b26d2:	00 
  6b26d3:	48 63 c5             	movslq %ebp,%rax
  6b26d6:	e9 86 0a 00 00       	jmp    6b3161 <sqlite3VdbeExec+0x6101>
  6b26db:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b26e2:	00 
  6b26e3:	49 8b 47 78          	mov    0x78(%r15),%rax
  6b26e7:	49 63 4e 04          	movslq 0x4(%r14),%rcx
  6b26eb:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b26ef:	4c 8b 78 28          	mov    0x28(%rax),%r15
  6b26f3:	4c 89 ff             	mov    %r15,%rdi
  6b26f6:	e8 d5 1d fe ff       	call   6944d0 <getCellInfo>
  6b26fb:	41 8b 57 40          	mov    0x40(%r15),%edx
  6b26ff:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b2704:	3b 90 88 00 00 00    	cmp    0x88(%rax),%edx
  6b270a:	0f 87 fe 3b 00 00    	ja     6b630e <sqlite3VdbeExec+0x92ae>
  6b2710:	41 0f b7 47 44       	movzwl 0x44(%r15),%eax
  6b2715:	49 8b 4f 38          	mov    0x38(%r15),%rcx
  6b2719:	49 8b b7 88 00 00 00 	mov    0x88(%r15),%rsi
  6b2720:	8b 76 58             	mov    0x58(%rsi),%esi
  6b2723:	29 ce                	sub    %ecx,%esi
  6b2725:	31 ff                	xor    %edi,%edi
  6b2727:	85 f6                	test   %esi,%esi
  6b2729:	0f 4f fe             	cmovg  %esi,%edi
  6b272c:	39 f0                	cmp    %esi,%eax
  6b272e:	0f 4e f8             	cmovle %eax,%edi
  6b2731:	49 89 4c 24 08       	mov    %rcx,0x8(%r12)
  6b2736:	39 fa                	cmp    %edi,%edx
  6b2738:	76 24                	jbe    6b275e <sqlite3VdbeExec+0x56fe>
  6b273a:	4c 89 ff             	mov    %r15,%rdi
  6b273d:	31 f6                	xor    %esi,%esi
  6b273f:	4c 89 e1             	mov    %r12,%rcx
  6b2742:	e8 f9 f1 fe ff       	call   6a1940 <sqlite3VdbeMemFromBtree>
  6b2747:	85 c0                	test   %eax,%eax
  6b2749:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b274e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2752:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2757:	74 20                	je     6b2779 <sqlite3VdbeExec+0x5719>
  6b2759:	e9 d4 3a 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b275e:	66 41 c7 44 24 14 10 	movw   $0x4010,0x14(%r12)
  6b2765:	40 
  6b2766:	41 89 54 24 10       	mov    %edx,0x10(%r12)
  6b276b:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b2770:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2774:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2779:	41 83 7d 0c 00       	cmpl   $0x0,0xc(%r13)
  6b277e:	0f 84 bb 0d 00 00    	je     6b353f <sqlite3VdbeExec+0x64df>
  6b2784:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b278b:	00 00 
  6b278d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2792:	49 83 c5 18          	add    $0x18,%r13
  6b2796:	e9 85 ab ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b279b:	4c 89 e9             	mov    %r13,%rcx
  6b279e:	4d 89 fd             	mov    %r15,%r13
  6b27a1:	4c 8d 79 18          	lea    0x18(%rcx),%r15
  6b27a5:	8b 69 04             	mov    0x4(%rcx),%ebp
  6b27a8:	49 8b 44 24 30       	mov    0x30(%r12),%rax
  6b27ad:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6b27b2:	0f b7 41 28          	movzwl 0x28(%rcx),%eax
  6b27b6:	66 89 44 24 7c       	mov    %ax,0x7c(%rsp)
  6b27bb:	c6 44 24 7e 00       	movb   $0x0,0x7e(%rsp)
  6b27c0:	48 63 41 24          	movslq 0x24(%rcx),%rax
  6b27c4:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b27c8:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6b27cd:	48 89 44 24 68       	mov    %rax,0x68(%rsp)
  6b27d2:	c7 84 24 e8 00 00 00 	movl   $0x0,0xe8(%rsp)
  6b27d9:	00 00 00 00 
  6b27dd:	ff c5                	inc    %ebp
  6b27df:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6b27e4:	48 8d 8c 24 e8 00 00 	lea    0xe8(%rsp),%rcx
  6b27eb:	00 
  6b27ec:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b27f1:	4c 89 e6             	mov    %r12,%rsi
  6b27f4:	e8 a7 6f ff ff       	call   6a97a0 <sqlite3VdbeIdxKeyCompare>
  6b27f9:	85 c0                	test   %eax,%eax
  6b27fb:	0f 85 f9 31 00 00    	jne    6b59fa <sqlite3VdbeExec+0x899a>
  6b2801:	83 bc 24 e8 00 00 00 	cmpl   $0x0,0xe8(%rsp)
  6b2808:	00 
  6b2809:	0f 8f ec 2c 00 00    	jg     6b54fb <sqlite3VdbeExec+0x849b>
  6b280f:	0f 89 f6 2c 00 00    	jns    6b550b <sqlite3VdbeExec+0x84ab>
  6b2815:	ff cd                	dec    %ebp
  6b2817:	85 ed                	test   %ebp,%ebp
  6b2819:	0f 8e 0d 2d 00 00    	jle    6b552c <sqlite3VdbeExec+0x84cc>
  6b281f:	41 c7 44 24 18 00 00 	movl   $0x0,0x18(%r12)
  6b2826:	00 00 
  6b2828:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
  6b282d:	e8 3e 2a fe ff       	call   695270 <sqlite3BtreeNext>
  6b2832:	85 c0                	test   %eax,%eax
  6b2834:	74 a9                	je     6b27df <sqlite3VdbeExec+0x577f>
  6b2836:	83 f8 65             	cmp    $0x65,%eax
  6b2839:	0f 84 d1 2c 00 00    	je     6b5510 <sqlite3VdbeExec+0x84b0>
  6b283f:	e9 b6 31 00 00       	jmp    6b59fa <sqlite3VdbeExec+0x899a>
  6b2844:	4c 8b 2c 24          	mov    (%rsp),%r13
  6b2848:	48 8b 40 38          	mov    0x38(%rax),%rax
  6b284c:	4c 8d 78 10          	lea    0x10(%rax),%r15
  6b2850:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b2855:	48 63 49 08          	movslq 0x8(%rcx),%rcx
  6b2859:	4c 6b e1 38          	imul   $0x38,%rcx,%r12
  6b285d:	4c 03 64 24 28       	add    0x28(%rsp),%r12
  6b2862:	8b 28                	mov    (%rax),%ebp
  6b2864:	41 39 6c 24 20       	cmp    %ebp,0x20(%r12)
  6b2869:	7d 28                	jge    6b2893 <sqlite3VdbeExec+0x5833>
  6b286b:	4c 89 e7             	mov    %r12,%rdi
  6b286e:	89 ee                	mov    %ebp,%esi
  6b2870:	31 d2                	xor    %edx,%edx
  6b2872:	e8 39 dd fe ff       	call   6a05b0 <sqlite3VdbeMemGrow>
  6b2877:	85 c0                	test   %eax,%eax
  6b2879:	0f 85 12 39 00 00    	jne    6b6191 <sqlite3VdbeExec+0x9131>
  6b287f:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b2884:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b2889:	41 23 44 24 14       	and    0x14(%r12),%eax
  6b288e:	83 c8 10             	or     $0x10,%eax
  6b2891:	eb 0e                	jmp    6b28a1 <sqlite3VdbeExec+0x5841>
  6b2893:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
  6b2898:	49 89 7c 24 08       	mov    %rdi,0x8(%r12)
  6b289d:	66 b8 10 00          	mov    $0x10,%ax
  6b28a1:	48 63 d5             	movslq %ebp,%rdx
  6b28a4:	41 89 6c 24 10       	mov    %ebp,0x10(%r12)
  6b28a9:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b28af:	4c 89 fe             	mov    %r15,%rsi
  6b28b2:	e8 f9 7c fa ff       	call   65a5b0 <memcpy$plt>
  6b28b7:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b28bb:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6b28c0:	48 63 4a 0c          	movslq 0xc(%rdx),%rcx
  6b28c4:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
  6b28c8:	c7 40 18 00 00 00 00 	movl   $0x0,0x18(%rax)
  6b28cf:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b28d6:	00 00 
  6b28d8:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b28dd:	4d 89 ef             	mov    %r13,%r15
  6b28e0:	49 89 d5             	mov    %rdx,%r13
  6b28e3:	49 83 c5 18          	add    $0x18,%r13
  6b28e7:	e9 34 aa ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b28ec:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b28f0:	49 83 c5 18          	add    $0x18,%r13
  6b28f4:	e9 27 aa ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b28f9:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b28fd:	49 83 c5 18          	add    $0x18,%r13
  6b2901:	e9 1a aa ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2906:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b290d:	00 
  6b290e:	4d 8b 6e 10          	mov    0x10(%r14),%r13
  6b2912:	4d 85 ed             	test   %r13,%r13
  6b2915:	74 0f                	je     6b2926 <sqlite3VdbeExec+0x58c6>
  6b2917:	4c 89 ef             	mov    %r13,%rdi
  6b291a:	e8 31 7a fa ff       	call   65a350 <strlen$plt>
  6b291f:	25 ff ff ff 3f       	and    $0x3fffffff,%eax
  6b2924:	eb 02                	jmp    6b2928 <sqlite3VdbeExec+0x58c8>
  6b2926:	31 c0                	xor    %eax,%eax
  6b2928:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b292d:	89 41 04             	mov    %eax,0x4(%rcx)
  6b2930:	80 7c 24 1f 01       	cmpb   $0x1,0x1f(%rsp)
  6b2935:	0f 84 ca 12 00 00    	je     6b3c05 <sqlite3VdbeExec+0x6ba5>
  6b293b:	4c 89 e7             	mov    %r12,%rdi
  6b293e:	4c 89 ee             	mov    %r13,%rsi
  6b2941:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  6b2948:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b294d:	45 31 c0             	xor    %r8d,%r8d
  6b2950:	e8 2b d7 fc ff       	call   680080 <sqlite3VdbeMemSetStr>
  6b2955:	85 c0                	test   %eax,%eax
  6b2957:	0f 85 b1 39 00 00    	jne    6b630e <sqlite3VdbeExec+0x92ae>
  6b295d:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b2963:	a8 02                	test   $0x2,%al
  6b2965:	0f 85 27 12 00 00    	jne    6b3b92 <sqlite3VdbeExec+0x6b32>
  6b296b:	0f b6 4c 24 1f       	movzbl 0x1f(%rsp),%ecx
  6b2970:	41 88 4c 24 16       	mov    %cl,0x16(%r12)
  6b2975:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b297a:	e9 3d 12 00 00       	jmp    6b3bbc <sqlite3VdbeExec+0x6b5c>
  6b297f:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b2986:	00 
  6b2987:	49 6b c7 38          	imul   $0x38,%r15,%rax
  6b298b:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b2990:	48 01 d0             	add    %rdx,%rax
  6b2993:	48 8b 00             	mov    (%rax),%rax
  6b2996:	48 85 c0             	test   %rax,%rax
  6b2999:	7e 31                	jle    6b29cc <sqlite3VdbeExec+0x596c>
  6b299b:	49 6b ce 38          	imul   $0x38,%r14,%rcx
  6b299f:	48 8b 0c 0a          	mov    (%rdx,%rcx,1),%rcx
  6b29a3:	31 d2                	xor    %edx,%edx
  6b29a5:	48 85 c9             	test   %rcx,%rcx
  6b29a8:	48 0f 4f d1          	cmovg  %rcx,%rdx
  6b29ac:	48 be ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rsi
  6b29b3:	ff ff 7f 
  6b29b6:	48 29 c6             	sub    %rax,%rsi
  6b29b9:	48 01 c2             	add    %rax,%rdx
  6b29bc:	48 39 f1             	cmp    %rsi,%rcx
  6b29bf:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
  6b29c6:	48 0f 4e c2          	cmovle %rdx,%rax
  6b29ca:	eb 07                	jmp    6b29d3 <sqlite3VdbeExec+0x5973>
  6b29cc:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
  6b29d3:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b29d8:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b29dc:	e9 80 07 00 00       	jmp    6b3161 <sqlite3VdbeExec+0x6101>
  6b29e1:	66 c7 45 4c 01 00    	movw   $0x1,0x4c(%rbp)
  6b29e7:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b29eb:	48 63 84 24 e8 00 00 	movslq 0xe8(%rsp),%rax
  6b29f2:	00 
  6b29f3:	48 85 c0             	test   %rax,%rax
  6b29f6:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b29fb:	74 3c                	je     6b2a39 <sqlite3VdbeExec+0x59d9>
  6b29fd:	83 7c 24 20 00       	cmpl   $0x0,0x20(%rsp)
  6b2a02:	0f 85 02 36 00 00    	jne    6b600a <sqlite3VdbeExec+0x8faa>
  6b2a08:	48 8b 4d 00          	mov    0x0(%rbp),%rcx
  6b2a0c:	48 f7 d8             	neg    %rax
  6b2a0f:	48 01 c8             	add    %rcx,%rax
  6b2a12:	48 ff c0             	inc    %rax
  6b2a15:	48 89 45 00          	mov    %rax,0x0(%rbp)
  6b2a19:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  6b2a1e:	4c 8d 05 6b 65 fc ff 	lea    -0x39a95(%rip),%r8        # 678f90 <sqlite3_free>
  6b2a25:	4c 89 e7             	mov    %r12,%rdi
  6b2a28:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  6b2a2f:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b2a34:	e8 47 d6 fc ff       	call   680080 <sqlite3VdbeMemSetStr>
  6b2a39:	f6 45 4c 02          	testb  $0x2,0x4c(%rbp)
  6b2a3d:	75 12                	jne    6b2a51 <sqlite3VdbeExec+0x59f1>
  6b2a3f:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b2a44:	88 45 4e             	mov    %al,0x4e(%rbp)
  6b2a47:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2a4c:	e9 36 e4 ff ff       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6b2a51:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b2a56:	3a 45 4e             	cmp    0x4e(%rbp),%al
  6b2a59:	74 0d                	je     6b2a68 <sqlite3VdbeExec+0x5a08>
  6b2a5b:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b2a60:	4c 89 e7             	mov    %r12,%rdi
  6b2a63:	e8 58 cd fc ff       	call   67f7c0 <sqlite3VdbeMemTranslate>
  6b2a68:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2a6d:	e9 15 e4 ff ff       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6b2a72:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b2a79:	00 
  6b2a7a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2a7e:	66 41 c7 44 24 14 08 	movw   $0x8,0x14(%r12)
  6b2a85:	00 
  6b2a86:	49 8b 45 10          	mov    0x10(%r13),%rax
  6b2a8a:	f2 0f 10 00          	movsd  (%rax),%xmm0
  6b2a8e:	f2 41 0f 11 04 24    	movsd  %xmm0,(%r12)
  6b2a94:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2a99:	49 83 c5 18          	add    $0x18,%r13
  6b2a9d:	e9 7e a8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2aa2:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b2aa9:	00 
  6b2aaa:	b0 01                	mov    $0x1,%al
  6b2aac:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2ab0:	49 8b 8e 08 01 00 00 	mov    0x108(%r14),%rcx
  6b2ab7:	49 63 55 04          	movslq 0x4(%r13),%rdx
  6b2abb:	48 8b 71 10          	mov    0x10(%rcx),%rsi
  6b2abf:	48 63 79 4c          	movslq 0x4c(%rcx),%rdi
  6b2ac3:	48 8d 3c 7f          	lea    (%rdi,%rdi,2),%rdi
  6b2ac7:	48 63 74 fe 04       	movslq 0x4(%rsi,%rdi,8),%rsi
  6b2acc:	48 01 d6             	add    %rdx,%rsi
  6b2acf:	48 6b f6 38          	imul   $0x38,%rsi,%rsi
  6b2ad3:	48 03 71 18          	add    0x18(%rcx),%rsi
  6b2ad7:	84 c0                	test   %al,%al
  6b2ad9:	74 40                	je     6b2b1b <sqlite3VdbeExec+0x5abb>
  6b2adb:	48 8b 46 10          	mov    0x10(%rsi),%rax
  6b2adf:	49 89 44 24 10       	mov    %rax,0x10(%r12)
  6b2ae4:	66 0f 10 06          	movupd (%rsi),%xmm0
  6b2ae8:	66 41 0f 11 04 24    	movupd %xmm0,(%r12)
  6b2aee:	f6 46 15 20          	testb  $0x20,0x15(%rsi)
  6b2af2:	0f 85 03 10 00 00    	jne    6b3afb <sqlite3VdbeExec+0x6a9b>
  6b2af8:	b8 ff 8f ff ff       	mov    $0xffff8fff,%eax
  6b2afd:	41 23 44 24 14       	and    0x14(%r12),%eax
  6b2b02:	0d 00 40 00 00       	or     $0x4000,%eax
  6b2b07:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b2b0d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2b12:	49 83 c5 18          	add    $0x18,%r13
  6b2b16:	e9 05 a8 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2b1b:	4c 89 e7             	mov    %r12,%rdi
  6b2b1e:	ba 00 40 00 00       	mov    $0x4000,%edx
  6b2b23:	e8 f8 ec fe ff       	call   6a1820 <vdbeClrCopy>
  6b2b28:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b2b2d:	49 83 c5 18          	add    $0x18,%r13
  6b2b31:	e9 ea a7 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2b36:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b2b3d:	00 
  6b2b3e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2b42:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b2b47:	48 8b 40 20          	mov    0x20(%rax),%rax
  6b2b4b:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b2b4f:	48 c1 e1 05          	shl    $0x5,%rcx
  6b2b53:	48 8b 4c 08 08       	mov    0x8(%rax,%rcx,1),%rcx
  6b2b58:	41 8b 45 0c          	mov    0xc(%r13),%eax
  6b2b5c:	85 c0                	test   %eax,%eax
  6b2b5e:	74 23                	je     6b2b83 <sqlite3VdbeExec+0x5b23>
  6b2b60:	48 8b 51 08          	mov    0x8(%rcx),%rdx
  6b2b64:	8b 72 40             	mov    0x40(%rdx),%esi
  6b2b67:	39 c6                	cmp    %eax,%esi
  6b2b69:	0f 47 c6             	cmova  %esi,%eax
  6b2b6c:	48 8b 09             	mov    (%rcx),%rcx
  6b2b6f:	48 89 4a 08          	mov    %rcx,0x8(%rdx)
  6b2b73:	48 8b 0a             	mov    (%rdx),%rcx
  6b2b76:	89 81 bc 00 00 00    	mov    %eax,0xbc(%rcx)
  6b2b7c:	89 c0                	mov    %eax,%eax
  6b2b7e:	e9 de 05 00 00       	jmp    6b3161 <sqlite3VdbeExec+0x6101>
  6b2b83:	48 8b 01             	mov    (%rcx),%rax
  6b2b86:	48 8b 49 08          	mov    0x8(%rcx),%rcx
  6b2b8a:	48 89 41 08          	mov    %rax,0x8(%rcx)
  6b2b8e:	48 8b 01             	mov    (%rcx),%rax
  6b2b91:	8b 80 bc 00 00 00    	mov    0xbc(%rax),%eax
  6b2b97:	89 c0                	mov    %eax,%eax
  6b2b99:	e9 c3 05 00 00       	jmp    6b3161 <sqlite3VdbeExec+0x6101>
  6b2b9e:	66 c7 47 14 04 00    	movw   $0x4,0x14(%rdi)
  6b2ba4:	41 8b 6e 0c          	mov    0xc(%r14),%ebp
  6b2ba8:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6b2bad:	49 63 4e 04          	movslq 0x4(%r14),%rcx
  6b2bb1:	48 c1 e1 05          	shl    $0x5,%rcx
  6b2bb5:	48 8b 4c 08 08       	mov    0x8(%rax,%rcx,1),%rcx
  6b2bba:	48 8b 41 08          	mov    0x8(%rcx),%rax
  6b2bbe:	4c 8b 28             	mov    (%rax),%r13
  6b2bc1:	45 0f b6 65 09       	movzbl 0x9(%r13),%r12d
  6b2bc6:	83 fd ff             	cmp    $0xffffffff,%ebp
  6b2bc9:	41 0f 44 ec          	cmove  %r12d,%ebp
  6b2bcd:	41 80 7d 15 02       	cmpb   $0x2,0x15(%r13)
  6b2bd2:	4d 89 fe             	mov    %r15,%r14
  6b2bd5:	77 11                	ja     6b2be8 <sqlite3VdbeExec+0x5b88>
  6b2bd7:	49 8b 45 50          	mov    0x50(%r13),%rax
  6b2bdb:	48 83 38 00          	cmpq   $0x0,(%rax)
  6b2bdf:	74 0a                	je     6b2beb <sqlite3VdbeExec+0x5b8b>
  6b2be1:	49 83 7d 60 00       	cmpq   $0x0,0x60(%r13)
  6b2be6:	7e 03                	jle    6b2beb <sqlite3VdbeExec+0x5b8b>
  6b2be8:	44 89 e5             	mov    %r12d,%ebp
  6b2beb:	48 8d 3d f2 7f b9 ff 	lea    -0x46800e(%rip),%rdi        # 24abe4 <sqlite3PagerFilename.zFake+0x4>
  6b2bf2:	41 80 7d 13 00       	cmpb   $0x0,0x13(%r13)
  6b2bf7:	75 14                	jne    6b2c0d <sqlite3VdbeExec+0x5bad>
  6b2bf9:	48 8d 05 90 cb 0d 00 	lea    0xdcb90(%rip),%rax        # 78f790 <memdb_vfs>
  6b2c00:	49 39 45 00          	cmp    %rax,0x0(%r13)
  6b2c04:	74 07                	je     6b2c0d <sqlite3VdbeExec+0x5bad>
  6b2c06:	49 8b bd d8 00 00 00 	mov    0xd8(%r13),%rdi
  6b2c0d:	83 fd 05             	cmp    $0x5,%ebp
  6b2c10:	0f 85 d3 25 00 00    	jne    6b51e9 <sqlite3VdbeExec+0x8189>
  6b2c16:	48 85 ff             	test   %rdi,%rdi
  6b2c19:	0f 84 d3 25 00 00    	je     6b51f2 <sqlite3VdbeExec+0x8192>
  6b2c1f:	49 89 cf             	mov    %rcx,%r15
  6b2c22:	e8 29 77 fa ff       	call   65a350 <strlen$plt>
  6b2c27:	48 c1 e0 22          	shl    $0x22,%rax
  6b2c2b:	0f 84 c1 25 00 00    	je     6b51f2 <sqlite3VdbeExec+0x8192>
  6b2c31:	41 80 7d 11 00       	cmpb   $0x0,0x11(%r13)
  6b2c36:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b2c3b:	0f 84 95 25 00 00    	je     6b51d6 <sqlite3VdbeExec+0x8176>
  6b2c41:	44 89 e5             	mov    %r12d,%ebp
  6b2c44:	e9 b1 25 00 00       	jmp    6b51fa <sqlite3VdbeExec+0x819a>
  6b2c49:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b2c50:	00 
  6b2c51:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2c55:	4d 8b 75 10          	mov    0x10(%r13),%r14
  6b2c59:	49 8b 46 50          	mov    0x50(%r14),%rax
  6b2c5d:	48 85 c0             	test   %rax,%rax
  6b2c60:	0f 84 f2 10 00 00    	je     6b3d58 <sqlite3VdbeExec+0x6cf8>
  6b2c66:	48 8b 78 10          	mov    0x10(%rax),%rdi
  6b2c6a:	4c 8b 0f             	mov    (%rdi),%r9
  6b2c6d:	ff 40 18             	incl   0x18(%rax)
  6b2c70:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b2c75:	48 8b 40 20          	mov    0x20(%rax),%rax
  6b2c79:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b2c7d:	48 c1 e1 05          	shl    $0x5,%rcx
  6b2c81:	48 8b 34 08          	mov    (%rax,%rcx,1),%rsi
  6b2c85:	49 8b 16             	mov    (%r14),%rdx
  6b2c88:	41 8b 4d 0c          	mov    0xc(%r13),%ecx
  6b2c8c:	4c 8d 44 24 60       	lea    0x60(%rsp),%r8
  6b2c91:	41 ff 91 c0 00 00 00 	call   *0xc0(%r9)
  6b2c98:	41 89 c7             	mov    %eax,%r15d
  6b2c9b:	49 8b 7e 50          	mov    0x50(%r14),%rdi
  6b2c9f:	e8 dc 73 00 00       	call   6ba080 <sqlite3VtabUnlock>
  6b2ca4:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
  6b2ca9:	45 85 ff             	test   %r15d,%r15d
  6b2cac:	0f 85 3a 33 00 00    	jne    6b5fec <sqlite3VdbeExec+0x8f8c>
  6b2cb2:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b2cb9:	00 00 
  6b2cbb:	48 85 f6             	test   %rsi,%rsi
  6b2cbe:	74 1b                	je     6b2cdb <sqlite3VdbeExec+0x5c7b>
  6b2cc0:	4c 8d 05 c9 62 fc ff 	lea    -0x39d37(%rip),%r8        # 678f90 <sqlite3_free>
  6b2cc7:	4c 89 e7             	mov    %r12,%rdi
  6b2cca:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  6b2cd1:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b2cd6:	e8 a5 d3 fc ff       	call   680080 <sqlite3VdbeMemSetStr>
  6b2cdb:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2cdf:	e9 74 10 00 00       	jmp    6b3d58 <sqlite3VdbeExec+0x6cf8>
  6b2ce4:	66 41 c7 45 14 04 00 	movw   $0x4,0x14(%r13)
  6b2ceb:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2cef:	66 41 c7 45 14 01 00 	movw   $0x1,0x14(%r13)
  6b2cf6:	41 8b 84 24 e0 00 00 	mov    0xe0(%r12),%eax
  6b2cfd:	00 
  6b2cfe:	ff c0                	inc    %eax
  6b2d00:	41 39 84 24 d4 00 00 	cmp    %eax,0xd4(%r12)
  6b2d07:	00 
  6b2d08:	0f 8f b2 30 00 00    	jg     6b5dc0 <sqlite3VdbeExec+0x8d60>
  6b2d0e:	4c 89 e0             	mov    %r12,%rax
  6b2d11:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
  6b2d16:	4c 63 62 0c          	movslq 0xc(%rdx),%r12
  6b2d1a:	c7 44 24 60 00 00 00 	movl   $0x0,0x60(%rsp)
  6b2d21:	00 
  6b2d22:	48 8b 40 20          	mov    0x20(%rax),%rax
  6b2d26:	4c 89 e1             	mov    %r12,%rcx
  6b2d29:	48 c1 e1 05          	shl    $0x5,%rcx
  6b2d2d:	48 8b 7c 08 08       	mov    0x8(%rax,%rcx,1),%rdi
  6b2d32:	8b 72 04             	mov    0x4(%rdx),%esi
  6b2d35:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6b2d3a:	e8 31 9b fe ff       	call   69c870 <sqlite3BtreeDropTable>
  6b2d3f:	66 41 c7 45 14 04 00 	movw   $0x4,0x14(%r13)
  6b2d46:	48 63 54 24 60       	movslq 0x60(%rsp),%rdx
  6b2d4b:	49 89 55 00          	mov    %rdx,0x0(%r13)
  6b2d4f:	85 c0                	test   %eax,%eax
  6b2d51:	0f 85 e2 2c 00 00    	jne    6b5a39 <sqlite3VdbeExec+0x89d9>
  6b2d57:	85 d2                	test   %edx,%edx
  6b2d59:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b2d5e:	0f 84 30 24 00 00    	je     6b5194 <sqlite3VdbeExec+0x8134>
  6b2d64:	41 8b 4d 04          	mov    0x4(%r13),%ecx
  6b2d68:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b2d6d:	44 89 e6             	mov    %r12d,%esi
  6b2d70:	e8 3b 4d 00 00       	call   6b7ab0 <sqlite3RootPageMoved>
  6b2d75:	41 fe c4             	inc    %r12b
  6b2d78:	4c 89 64 24 48       	mov    %r12,0x48(%rsp)
  6b2d7d:	e9 12 24 00 00       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6b2d82:	49 8b 45 28          	mov    0x28(%r13),%rax
  6b2d86:	48 8b 00             	mov    (%rax),%rax
  6b2d89:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b2d8e:	4c 8b 30             	mov    (%rax),%r14
  6b2d91:	66 0f ef c0          	pxor   %xmm0,%xmm0
  6b2d95:	48 8d 84 24 f8 00 00 	lea    0xf8(%rsp),%rax
  6b2d9c:	00 
  6b2d9d:	f3 0f 7f 40 10       	movdqu %xmm0,0x10(%rax)
  6b2da2:	f3 0f 7f 00          	movdqu %xmm0,(%rax)
  6b2da6:	48 c7 40 20 00 00 00 	movq   $0x0,0x20(%rax)
  6b2dad:	00 
  6b2dae:	4c 89 a4 24 e8 00 00 	mov    %r12,0xe8(%rsp)
  6b2db5:	00 
  6b2db6:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b2dbb:	88 84 24 10 01 00 00 	mov    %al,0x110(%rsp)
  6b2dc2:	48 c7 44 24 68 00 00 	movq   $0x0,0x68(%rsp)
  6b2dc9:	00 00 
  6b2dcb:	c7 44 24 64 00 00 00 	movl   $0x1000000,0x64(%rsp)
  6b2dd2:	01 
  6b2dd3:	48 8d 44 24 60       	lea    0x60(%rsp),%rax
  6b2dd8:	48 89 84 24 f0 00 00 	mov    %rax,0xf0(%rsp)
  6b2ddf:	00 
  6b2de0:	f6 42 02 01          	testb  $0x1,0x2(%rdx)
  6b2de4:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b2dea:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2dee:	0f 85 f0 13 00 00    	jne    6b41e4 <sqlite3VdbeExec+0x7184>
  6b2df4:	25 40 f2 ff ff       	and    $0xfffff240,%eax
  6b2df9:	ff c0                	inc    %eax
  6b2dfb:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b2e01:	e9 dc 25 00 00       	jmp    6b53e2 <sqlite3VdbeExec+0x8382>
  6b2e06:	31 c0                	xor    %eax,%eax
  6b2e08:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2e0c:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b2e10:	48 6b f1 38          	imul   $0x38,%rcx,%rsi
  6b2e14:	48 03 74 24 28       	add    0x28(%rsp),%rsi
  6b2e19:	48 63 4e 10          	movslq 0x10(%rsi),%rcx
  6b2e1d:	48 c1 e1 03          	shl    $0x3,%rcx
  6b2e21:	48 89 c2             	mov    %rax,%rdx
  6b2e24:	48 09 ca             	or     %rcx,%rdx
  6b2e27:	48 c1 ea 20          	shr    $0x20,%rdx
  6b2e2b:	74 07                	je     6b2e34 <sqlite3VdbeExec+0x5dd4>
  6b2e2d:	31 d2                	xor    %edx,%edx
  6b2e2f:	48 f7 f1             	div    %rcx
  6b2e32:	eb 04                	jmp    6b2e38 <sqlite3VdbeExec+0x5dd8>
  6b2e34:	31 d2                	xor    %edx,%edx
  6b2e36:	f7 f1                	div    %ecx
  6b2e38:	89 d1                	mov    %edx,%ecx
  6b2e3a:	80 e1 07             	and    $0x7,%cl
  6b2e3d:	b0 01                	mov    $0x1,%al
  6b2e3f:	d2 e0                	shl    %cl,%al
  6b2e41:	48 8b 4e 08          	mov    0x8(%rsi),%rcx
  6b2e45:	48 c1 ea 03          	shr    $0x3,%rdx
  6b2e49:	08 04 11             	or     %al,(%rcx,%rdx,1)
  6b2e4c:	49 83 c5 18          	add    $0x18,%r13
  6b2e50:	e9 cb a4 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2e55:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b2e59:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b2e5d:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b2e62:	0f b7 44 01 14       	movzwl 0x14(%rcx,%rax,1),%eax
  6b2e67:	83 e0 3f             	and    $0x3f,%eax
  6b2e6a:	48 8d 0d ff 7d b9 ff 	lea    -0x468201(%rip),%rcx        # 24ac70 <sqlite3_value_type.aType>
  6b2e71:	0f b6 0c 08          	movzbl (%rax,%rcx,1),%ecx
  6b2e75:	fe c9                	dec    %cl
  6b2e77:	b8 01 00 00 00       	mov    $0x1,%eax
  6b2e7c:	d3 e0                	shl    %cl,%eax
  6b2e7e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2e82:	e9 c6 18 00 00       	jmp    6b474d <sqlite3VdbeExec+0x76ed>
  6b2e87:	49 8b 44 24 08       	mov    0x8(%r12),%rax
  6b2e8c:	48 85 c0             	test   %rax,%rax
  6b2e8f:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2e93:	0f 84 7e a4 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b2e99:	41 8b 4d 08          	mov    0x8(%r13),%ecx
  6b2e9d:	ff c1                	inc    %ecx
  6b2e9f:	83 e1 03             	and    $0x3,%ecx
  6b2ea2:	8b 90 c8 00 00 00    	mov    0xc8(%rax),%edx
  6b2ea8:	83 e2 fc             	and    $0xfffffffc,%edx
  6b2eab:	0f b7 f1             	movzwl %cx,%esi
  6b2eae:	09 d6                	or     %edx,%esi
  6b2eb0:	66 89 b0 c8 00 00 00 	mov    %si,0xc8(%rax)
  6b2eb7:	48 8b 40 10          	mov    0x10(%rax),%rax
  6b2ebb:	48 85 c0             	test   %rax,%rax
  6b2ebe:	75 e2                	jne    6b2ea2 <sqlite3VdbeExec+0x5e42>
  6b2ec0:	e9 52 a4 ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6b2ec5:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b2ecc:	00 
  6b2ecd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2ed1:	48 8b 54 24 10       	mov    0x10(%rsp),%rdx
  6b2ed6:	48 8b 42 20          	mov    0x20(%rdx),%rax
  6b2eda:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b2ede:	48 c1 e1 05          	shl    $0x5,%rcx
  6b2ee2:	48 8b 44 08 08       	mov    0x8(%rax,%rcx,1),%rax
  6b2ee7:	48 8b 40 08          	mov    0x8(%rax),%rax
  6b2eeb:	8b 40 40             	mov    0x40(%rax),%eax
  6b2eee:	49 89 04 24          	mov    %rax,(%r12)
  6b2ef2:	49 89 d4             	mov    %rdx,%r12
  6b2ef5:	49 83 c5 18          	add    $0x18,%r13
  6b2ef9:	e9 22 a4 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b2efe:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b2f05:	00 
  6b2f06:	49 8b 45 78          	mov    0x78(%r13),%rax
  6b2f0a:	49 63 4e 04          	movslq 0x4(%r14),%rcx
  6b2f0e:	48 8b 2c c8          	mov    (%rax,%rcx,8),%rbp
  6b2f12:	f6 45 05 02          	testb  $0x2,0x5(%rbp)
  6b2f16:	0f 85 0f 1e 00 00    	jne    6b4d2b <sqlite3VdbeExec+0x7ccb>
  6b2f1c:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b2f20:	80 3f 00             	cmpb   $0x0,(%rdi)
  6b2f23:	75 0a                	jne    6b2f2f <sqlite3VdbeExec+0x5ecf>
  6b2f25:	f6 47 01 08          	testb  $0x8,0x1(%rdi)
  6b2f29:	0f 85 ba 1d 00 00    	jne    6b4ce9 <sqlite3VdbeExec+0x7c89>
  6b2f2f:	48 8d b4 24 e8 00 00 	lea    0xe8(%rsp),%rsi
  6b2f36:	00 
  6b2f37:	e8 b4 1f fe ff       	call   694ef0 <btreeLast>
  6b2f3c:	85 c0                	test   %eax,%eax
  6b2f3e:	0f 85 88 32 00 00    	jne    6b61cc <sqlite3VdbeExec+0x916c>
  6b2f44:	83 bc 24 e8 00 00 00 	cmpl   $0x0,0xe8(%rsp)
  6b2f4b:	00 
  6b2f4c:	0f 84 0d 18 00 00    	je     6b475f <sqlite3VdbeExec+0x76ff>
  6b2f52:	48 c7 44 24 60 01 00 	movq   $0x1,0x60(%rsp)
  6b2f59:	00 00 
  6b2f5b:	e9 c2 1d 00 00       	jmp    6b4d22 <sqlite3VdbeExec+0x7cc2>
  6b2f60:	a8 08                	test   $0x8,%al
  6b2f62:	0f 85 94 12 00 00    	jne    6b41fc <sqlite3VdbeExec+0x719c>
  6b2f68:	a8 12                	test   $0x12,%al
  6b2f6a:	0f 84 de 1c 00 00    	je     6b4c4e <sqlite3VdbeExec+0x7bee>
  6b2f70:	49 83 7c 24 08 00    	cmpq   $0x0,0x8(%r12)
  6b2f76:	0f 84 d2 1c 00 00    	je     6b4c4e <sqlite3VdbeExec+0x7bee>
  6b2f7c:	4c 89 e7             	mov    %r12,%rdi
  6b2f7f:	e8 cc df fe ff       	call   6a0f50 <memIntValue>
  6b2f84:	41 0f b7 4c 24 14    	movzwl 0x14(%r12),%ecx
  6b2f8a:	e9 91 cd ff ff       	jmp    6afd20 <sqlite3VdbeExec+0x2cc0>
  6b2f8f:	a8 08                	test   $0x8,%al
  6b2f91:	4d 89 fe             	mov    %r15,%r14
  6b2f94:	0f 85 8c 12 00 00    	jne    6b4226 <sqlite3VdbeExec+0x71c6>
  6b2f9a:	a8 12                	test   $0x12,%al
  6b2f9c:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b2fa0:	0f 84 af 1c 00 00    	je     6b4c55 <sqlite3VdbeExec+0x7bf5>
  6b2fa6:	49 83 7d 08 00       	cmpq   $0x0,0x8(%r13)
  6b2fab:	0f 84 a4 1c 00 00    	je     6b4c55 <sqlite3VdbeExec+0x7bf5>
  6b2fb1:	4c 89 ef             	mov    %r13,%rdi
  6b2fb4:	e8 97 df fe ff       	call   6a0f50 <memIntValue>
  6b2fb9:	41 0f b7 4d 14       	movzwl 0x14(%r13),%ecx
  6b2fbe:	e9 99 cd ff ff       	jmp    6afd5c <sqlite3VdbeExec+0x2cfc>
  6b2fc3:	49 8b 44 24 20       	mov    0x20(%r12),%rax
  6b2fc8:	4c 89 f9             	mov    %r15,%rcx
  6b2fcb:	48 c1 e1 05          	shl    $0x5,%rcx
  6b2fcf:	48 8b 7c 08 18       	mov    0x18(%rax,%rcx,1),%rdi
  6b2fd4:	e8 a7 4e 00 00       	call   6b7e80 <sqlite3SchemaClear>
  6b2fd9:	41 80 64 24 2c ef    	andb   $0xef,0x2c(%r12)
  6b2fdf:	41 0f b7 4d 02       	movzwl 0x2(%r13),%ecx
  6b2fe4:	4c 89 e7             	mov    %r12,%rdi
  6b2fe7:	44 89 fe             	mov    %r15d,%esi
  6b2fea:	48 8b 94 24 d0 00 00 	mov    0xd0(%rsp),%rdx
  6b2ff1:	00 
  6b2ff2:	e8 c9 51 00 00       	call   6b81c0 <sqlite3InitOne>
  6b2ff7:	41 80 4c 24 2c 01    	orb    $0x1,0x2c(%r12)
  6b2ffd:	41 80 a6 c8 00 00 00 	andb   $0xfc,0xc8(%r14)
  6b3004:	fc 
  6b3005:	85 c0                	test   %eax,%eax
  6b3007:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b300b:	0f 85 8f 2e 00 00    	jne    6b5ea0 <sqlite3VdbeExec+0x8e40>
  6b3011:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3018:	00 00 
  6b301a:	49 83 c5 18          	add    $0x18,%r13
  6b301e:	e9 fd a2 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3023:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b302a:	00 
  6b302b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b302f:	c7 44 24 60 00 00 00 	movl   $0x0,0x60(%rsp)
  6b3036:	00 
  6b3037:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b303c:	48 8b 40 20          	mov    0x20(%rax),%rax
  6b3040:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b3044:	48 c1 e1 05          	shl    $0x5,%rcx
  6b3048:	48 8b 7c 08 08       	mov    0x8(%rax,%rcx,1),%rdi
  6b304d:	41 8b 55 0c          	mov    0xc(%r13),%edx
  6b3051:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6b3056:	e8 d5 8f fe ff       	call   69c030 <sqlite3BtreeCreateTable>
  6b305b:	85 c0                	test   %eax,%eax
  6b305d:	0f 85 cf 31 00 00    	jne    6b6232 <sqlite3VdbeExec+0x91d2>
  6b3063:	8b 44 24 60          	mov    0x60(%rsp),%eax
  6b3067:	49 89 04 24          	mov    %rax,(%r12)
  6b306b:	e9 24 21 00 00       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6b3070:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b3077:	00 
  6b3078:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b307c:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b3080:	e9 dc 00 00 00       	jmp    6b3161 <sqlite3VdbeExec+0x6101>
  6b3085:	a8 01                	test   $0x1,%al
  6b3087:	0f 85 3f 12 00 00    	jne    6b42cc <sqlite3VdbeExec+0x726c>
  6b308d:	a8 08                	test   $0x8,%al
  6b308f:	0f 85 f5 1d 00 00    	jne    6b4e8a <sqlite3VdbeExec+0x7e2a>
  6b3095:	a8 12                	test   $0x12,%al
  6b3097:	0f 84 71 27 00 00    	je     6b580e <sqlite3VdbeExec+0x87ae>
  6b309d:	e8 fe de fe ff       	call   6a0fa0 <memRealValue>
  6b30a2:	e9 6b 27 00 00       	jmp    6b5812 <sqlite3VdbeExec+0x87b2>
  6b30a7:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b30ae:	00 
  6b30af:	49 8b 47 78          	mov    0x78(%r15),%rax
  6b30b3:	49 63 4e 04          	movslq 0x4(%r14),%rcx
  6b30b7:	4c 8b 2c c8          	mov    (%rax,%rcx,8),%r13
  6b30bb:	41 80 7d 02 00       	cmpb   $0x0,0x2(%r13)
  6b30c0:	74 22                	je     6b30e4 <sqlite3VdbeExec+0x6084>
  6b30c2:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b30c9:	00 
  6b30ca:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b30cf:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b30d4:	4d 89 fe             	mov    %r15,%r14
  6b30d7:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b30db:	49 83 c5 18          	add    $0x18,%r13
  6b30df:	e9 3c a2 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b30e4:	41 80 7d 03 00       	cmpb   $0x0,0x3(%r13)
  6b30e9:	0f 84 a4 17 00 00    	je     6b4893 <sqlite3VdbeExec+0x7833>
  6b30ef:	49 83 c5 48          	add    $0x48,%r13
  6b30f3:	4d 89 fe             	mov    %r15,%r14
  6b30f6:	e9 a3 28 00 00       	jmp    6b599e <sqlite3VdbeExec+0x893e>
  6b30fb:	48 c7 44 24 60 00 00 	movq   $0x0,0x60(%rsp)
  6b3102:	00 00 
  6b3104:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6b3109:	4c 89 e7             	mov    %r12,%rdi
  6b310c:	e8 ff 9b fe ff       	call   69cd10 <sqlite3BtreeCount>
  6b3111:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3118:	00 00 
  6b311a:	85 c0                	test   %eax,%eax
  6b311c:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3120:	0f 85 91 29 00 00    	jne    6b5ab7 <sqlite3VdbeExec+0x8a57>
  6b3126:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b312a:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b312e:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b3132:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b3136:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b313b:	0f 84 7a 02 00 00    	je     6b33bb <sqlite3VdbeExec+0x635b>
  6b3141:	4c 89 e7             	mov    %r12,%rdi
  6b3144:	e8 17 49 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b3149:	e9 75 02 00 00       	jmp    6b33c3 <sqlite3VdbeExec+0x6363>
  6b314e:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b3155:	00 
  6b3156:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b315a:	49 8b 45 10          	mov    0x10(%r13),%rax
  6b315e:	48 8b 00             	mov    (%rax),%rax
  6b3161:	49 89 04 24          	mov    %rax,(%r12)
  6b3165:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b316a:	49 83 c5 18          	add    $0x18,%r13
  6b316e:	e9 ad a1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3173:	49 8b 7e 28          	mov    0x28(%r14),%rdi
  6b3177:	4c 8b 27             	mov    (%rdi),%r12
  6b317a:	4d 8b 3c 24          	mov    (%r12),%r15
  6b317e:	41 ff 57 48          	call   *0x48(%r15)
  6b3182:	89 c5                	mov    %eax,%ebp
  6b3184:	4c 89 ef             	mov    %r13,%rdi
  6b3187:	4c 89 e6             	mov    %r12,%rsi
  6b318a:	e8 81 69 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6b318f:	48 89 e8             	mov    %rbp,%rax
  6b3192:	85 c0                	test   %eax,%eax
  6b3194:	0f 85 36 2e 00 00    	jne    6b5fd0 <sqlite3VdbeExec+0x8f70>
  6b319a:	49 8b 7e 28          	mov    0x28(%r14),%rdi
  6b319e:	41 ff 57 50          	call   *0x50(%r15)
  6b31a2:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b31a9:	00 00 
  6b31ab:	85 c0                	test   %eax,%eax
  6b31ad:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b31b2:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b31b7:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b31bc:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b31c0:	0f 85 c1 dc ff ff    	jne    6b0e87 <sqlite3VdbeExec+0x3e27>
  6b31c6:	e9 a7 dc ff ff       	jmp    6b0e72 <sqlite3VdbeExec+0x3e12>
  6b31cb:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b31d0:	48 63 40 04          	movslq 0x4(%rax),%rax
  6b31d4:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b31d8:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b31dd:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b31e1:	49 83 c5 e8          	add    $0xffffffffffffffe8,%r13
  6b31e5:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b31e9:	49 83 c5 18          	add    $0x18,%r13
  6b31ed:	e9 2e a1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b31f2:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b31f6:	49 83 c5 18          	add    $0x18,%r13
  6b31fa:	e9 21 a1 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b31ff:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b3206:	00 
  6b3207:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b320b:	49 8b 75 10          	mov    0x10(%r13),%rsi
  6b320f:	49 63 55 04          	movslq 0x4(%r13),%rdx
  6b3213:	4c 89 e7             	mov    %r12,%rdi
  6b3216:	48 85 f6             	test   %rsi,%rsi
  6b3219:	74 0c                	je     6b3227 <sqlite3VdbeExec+0x61c7>
  6b321b:	31 c9                	xor    %ecx,%ecx
  6b321d:	45 31 c0             	xor    %r8d,%r8d
  6b3220:	e8 5b ce fc ff       	call   680080 <sqlite3VdbeMemSetStr>
  6b3225:	eb 17                	jmp    6b323e <sqlite3VdbeExec+0x61de>
  6b3227:	89 d6                	mov    %edx,%esi
  6b3229:	e8 52 e3 fe ff       	call   6a1580 <sqlite3VdbeMemSetZeroBlob>
  6b322e:	4c 89 e7             	mov    %r12,%rdi
  6b3231:	e8 8a d7 fe ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6b3236:	85 c0                	test   %eax,%eax
  6b3238:	0f 85 30 29 00 00    	jne    6b5b6e <sqlite3VdbeExec+0x8b0e>
  6b323e:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b3243:	41 88 44 24 16       	mov    %al,0x16(%r12)
  6b3248:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b324d:	49 83 c5 18          	add    $0x18,%r13
  6b3251:	e9 ca a0 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3256:	a8 01                	test   $0x1,%al
  6b3258:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b325c:	0f 85 73 10 00 00    	jne    6b42d5 <sqlite3VdbeExec+0x7275>
  6b3262:	a8 08                	test   $0x8,%al
  6b3264:	0f 85 29 1c 00 00    	jne    6b4e93 <sqlite3VdbeExec+0x7e33>
  6b326a:	a8 12                	test   $0x12,%al
  6b326c:	0f 84 dd 25 00 00    	je     6b584f <sqlite3VdbeExec+0x87ef>
  6b3272:	e8 29 dd fe ff       	call   6a0fa0 <memRealValue>
  6b3277:	e9 d7 25 00 00       	jmp    6b5853 <sqlite3VdbeExec+0x87f3>
  6b327c:	8b 41 04             	mov    0x4(%rcx),%eax
  6b327f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3284:	41 3b 45 04          	cmp    0x4(%r13),%eax
  6b3288:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b328c:	0f 84 da 25 00 00    	je     6b586c <sqlite3VdbeExec+0x880c>
  6b3292:	41 89 45 04          	mov    %eax,0x4(%r13)
  6b3296:	49 83 c5 18          	add    $0x18,%r13
  6b329a:	e9 81 a0 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b329f:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b32a4:	83 b8 d8 00 00 00 00 	cmpl   $0x0,0xd8(%rax)
  6b32ab:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b32af:	0f 8f 54 2f 00 00    	jg     6b6209 <sqlite3VdbeExec+0x91a9>
  6b32b5:	4d 85 e4             	test   %r12,%r12
  6b32b8:	0f 84 1b 16 00 00    	je     6b48d9 <sqlite3VdbeExec+0x7879>
  6b32be:	4c 89 e7             	mov    %r12,%rdi
  6b32c1:	e8 8a 70 fa ff       	call   65a350 <strlen$plt>
  6b32c6:	49 89 c5             	mov    %rax,%r13
  6b32c9:	41 81 e5 ff ff ff 3f 	and    $0x3fffffff,%r13d
  6b32d0:	e9 07 16 00 00       	jmp    6b48dc <sqlite3VdbeExec+0x787c>
  6b32d5:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6b32dc:	00 00 00 00 
  6b32e0:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b32e5:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b32e9:	e9 7e 25 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b32ee:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b32f2:	49 83 c5 18          	add    $0x18,%r13
  6b32f6:	e9 25 a0 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b32fb:	48 01 d0             	add    %rdx,%rax
  6b32fe:	48 8b 48 30          	mov    0x30(%rax),%rcx
  6b3302:	48 89 8c 24 90 00 00 	mov    %rcx,0x90(%rsp)
  6b3309:	00 
  6b330a:	66 0f 10 00          	movupd (%rax),%xmm0
  6b330e:	66 0f 10 48 10       	movupd 0x10(%rax),%xmm1
  6b3313:	f3 0f 6f 50 20       	movdqu 0x20(%rax),%xmm2
  6b3318:	66 0f 7f 94 24 80 00 	movdqa %xmm2,0x80(%rsp)
  6b331f:	00 00 
  6b3321:	66 0f 29 4c 24 70    	movapd %xmm1,0x70(%rsp)
  6b3327:	66 0f 29 44 24 60    	movapd %xmm0,0x60(%rsp)
  6b332d:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6b3332:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b3337:	be 43 00 00 00       	mov    $0x43,%esi
  6b333c:	e8 4f 9c ff ff       	call   6acf90 <applyAffinity>
  6b3341:	0f b7 44 24 74       	movzwl 0x74(%rsp),%eax
  6b3346:	a8 04                	test   $0x4,%al
  6b3348:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b334c:	0f 84 1a 25 00 00    	je     6b586c <sqlite3VdbeExec+0x880c>
  6b3352:	4c 8b 64 24 60       	mov    0x60(%rsp),%r12
  6b3357:	e9 a5 db ff ff       	jmp    6b0f01 <sqlite3VdbeExec+0x3ea1>
  6b335c:	4d 89 07             	mov    %r8,(%r15)
  6b335f:	66 41 c7 47 14 04 00 	movw   $0x4,0x14(%r15)
  6b3366:	48 63 74 24 64       	movslq 0x64(%rsp),%rsi
  6b336b:	41 f6 47 4d 90       	testb  $0x90,0x4d(%r15)
  6b3370:	74 0b                	je     6b337d <sqlite3VdbeExec+0x631d>
  6b3372:	49 8d 7f 38          	lea    0x38(%r15),%rdi
  6b3376:	e8 65 e2 fe ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6b337b:	eb 0b                	jmp    6b3388 <sqlite3VdbeExec+0x6328>
  6b337d:	49 89 77 38          	mov    %rsi,0x38(%r15)
  6b3381:	66 41 c7 47 4c 04 00 	movw   $0x4,0x4c(%r15)
  6b3388:	48 63 74 24 68       	movslq 0x68(%rsp),%rsi
  6b338d:	41 f6 87 85 00 00 00 	testb  $0x90,0x85(%r15)
  6b3394:	90 
  6b3395:	74 11                	je     6b33a8 <sqlite3VdbeExec+0x6348>
  6b3397:	49 83 c7 70          	add    $0x70,%r15
  6b339b:	4c 89 ff             	mov    %r15,%rdi
  6b339e:	e8 3d e2 fe ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6b33a3:	e9 41 16 00 00       	jmp    6b49e9 <sqlite3VdbeExec+0x7989>
  6b33a8:	49 89 77 70          	mov    %rsi,0x70(%r15)
  6b33ac:	66 41 c7 87 84 00 00 	movw   $0x4,0x84(%r15)
  6b33b3:	00 04 00 
  6b33b6:	e9 2e 16 00 00       	jmp    6b49e9 <sqlite3VdbeExec+0x7989>
  6b33bb:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b33c2:	00 
  6b33c3:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
  6b33c8:	49 89 04 24          	mov    %rax,(%r12)
  6b33cc:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b33d1:	e9 b1 da ff ff       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6b33d6:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b33db:	e9 e2 1b 00 00       	jmp    6b4fc2 <sqlite3VdbeExec+0x7f62>
  6b33e0:	48 8b 7e 08          	mov    0x8(%rsi),%rdi
  6b33e4:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b33e9:	83 fa 01             	cmp    $0x1,%edx
  6b33ec:	77 11                	ja     6b33ff <sqlite3VdbeExec+0x639f>
  6b33ee:	85 d2                	test   %edx,%edx
  6b33f0:	0f 84 f9 0f 00 00    	je     6b43ef <sqlite3VdbeExec+0x738f>
  6b33f6:	31 d2                	xor    %edx,%edx
  6b33f8:	83 7f 40 00          	cmpl   $0x0,0x40(%rdi)
  6b33fc:	0f 95 c2             	setne  %dl
  6b33ff:	89 51 50             	mov    %edx,0x50(%rcx)
  6b3402:	c6 41 54 ff          	movb   $0xff,0x54(%rcx)
  6b3406:	48 89 81 80 00 00 00 	mov    %rax,0x80(%rcx)
  6b340d:	48 89 71 08          	mov    %rsi,0x8(%rcx)
  6b3411:	48 89 79 20          	mov    %rdi,0x20(%rcx)
  6b3415:	c6 41 01 00          	movb   $0x0,0x1(%rcx)
  6b3419:	48 8b 47 10          	mov    0x10(%rdi),%rax
  6b341d:	48 85 c0             	test   %rax,%rax
  6b3420:	0f 84 97 04 00 00    	je     6b38bd <sqlite3VdbeExec+0x685d>
  6b3426:	31 f6                	xor    %esi,%esi
  6b3428:	49 89 c0             	mov    %rax,%r8
  6b342b:	eb 0c                	jmp    6b3439 <sqlite3VdbeExec+0x63d9>
  6b342d:	0f 1f 00             	nopl   (%rax)
  6b3430:	4d 8b 40 28          	mov    0x28(%r8),%r8
  6b3434:	4d 85 c0             	test   %r8,%r8
  6b3437:	74 14                	je     6b344d <sqlite3VdbeExec+0x63ed>
  6b3439:	41 39 50 50          	cmp    %edx,0x50(%r8)
  6b343d:	75 f1                	jne    6b3430 <sqlite3VdbeExec+0x63d0>
  6b343f:	41 80 48 01 20       	orb    $0x20,0x1(%r8)
  6b3444:	c6 41 01 20          	movb   $0x20,0x1(%rcx)
  6b3448:	40 b6 20             	mov    $0x20,%sil
  6b344b:	eb e3                	jmp    6b3430 <sqlite3VdbeExec+0x63d0>
  6b344d:	40 80 ce 01          	or     $0x1,%sil
  6b3451:	e9 6a 04 00 00       	jmp    6b38c0 <sqlite3VdbeExec+0x6860>
  6b3456:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b345b:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3460:	49 89 ef             	mov    %rbp,%r15
  6b3463:	49 83 c5 18          	add    $0x18,%r13
  6b3467:	e9 b4 9e ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b346c:	4c 89 e7             	mov    %r12,%rdi
  6b346f:	be 04 00 00 00       	mov    $0x4,%esi
  6b3474:	e8 37 81 fe ff       	call   69b5b0 <sqlite3BtreeDelete>
  6b3479:	85 c0                	test   %eax,%eax
  6b347b:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3480:	0f 85 79 2d 00 00    	jne    6b61ff <sqlite3VdbeExec+0x919f>
  6b3486:	49 c7 46 18 00 00 00 	movq   $0x0,0x18(%r14)
  6b348d:	00 
  6b348e:	e9 ec 1d 00 00       	jmp    6b527f <sqlite3VdbeExec+0x821f>
  6b3493:	f2 0f 59 d0          	mulsd  %xmm0,%xmm2
  6b3497:	eb 1a                	jmp    6b34b3 <sqlite3VdbeExec+0x6453>
  6b3499:	66 0f 57 c9          	xorpd  %xmm1,%xmm1
  6b349d:	66 0f 2e d1          	ucomisd %xmm1,%xmm2
  6b34a1:	75 02                	jne    6b34a5 <sqlite3VdbeExec+0x6445>
  6b34a3:	7b 78                	jnp    6b351d <sqlite3VdbeExec+0x64bd>
  6b34a5:	f2 0f 5e c2          	divsd  %xmm2,%xmm0
  6b34a9:	eb 04                	jmp    6b34af <sqlite3VdbeExec+0x644f>
  6b34ab:	f2 0f 5c c2          	subsd  %xmm2,%xmm0
  6b34af:	66 0f 28 d0          	movapd %xmm0,%xmm2
  6b34b3:	66 0f 2e d2          	ucomisd %xmm2,%xmm2
  6b34b7:	7a 64                	jp     6b351d <sqlite3VdbeExec+0x64bd>
  6b34b9:	f2 41 0f 11 14 24    	movsd  %xmm2,(%r12)
  6b34bf:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b34c4:	41 23 44 24 14       	and    0x14(%r12),%eax
  6b34c9:	83 c8 08             	or     $0x8,%eax
  6b34cc:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b34d2:	e9 cf 24 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b34d7:	e8 f4 d9 fe ff       	call   6a0ed0 <sqlite3VdbeIntValue>
  6b34dc:	48 89 c5             	mov    %rax,%rbp
  6b34df:	4c 89 ef             	mov    %r13,%rdi
  6b34e2:	e8 e9 d9 fe ff       	call   6a0ed0 <sqlite3VdbeIntValue>
  6b34e7:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6b34ec:	48 85 ed             	test   %rbp,%rbp
  6b34ef:	74 2c                	je     6b351d <sqlite3VdbeExec+0x64bd>
  6b34f1:	48 83 fd ff          	cmp    $0xffffffffffffffff,%rbp
  6b34f5:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b34fa:	48 0f 45 cd          	cmovne %rbp,%rcx
  6b34fe:	48 89 c2             	mov    %rax,%rdx
  6b3501:	48 09 ca             	or     %rcx,%rdx
  6b3504:	48 c1 ea 20          	shr    $0x20,%rdx
  6b3508:	0f 84 97 19 00 00    	je     6b4ea5 <sqlite3VdbeExec+0x7e45>
  6b350e:	48 99                	cqto
  6b3510:	48 f7 f9             	idiv   %rcx
  6b3513:	0f 57 d2             	xorps  %xmm2,%xmm2
  6b3516:	f2 48 0f 2a d2       	cvtsi2sd %rdx,%xmm2
  6b351b:	eb 9c                	jmp    6b34b9 <sqlite3VdbeExec+0x6459>
  6b351d:	41 f6 44 24 15 90    	testb  $0x90,0x15(%r12)
  6b3523:	74 0d                	je     6b3532 <sqlite3VdbeExec+0x64d2>
  6b3525:	4c 89 e7             	mov    %r12,%rdi
  6b3528:	e8 13 e0 fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b352d:	e9 74 24 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b3532:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b3539:	00 
  6b353a:	e9 67 24 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b353f:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3546:	00 00 
  6b3548:	41 f6 44 24 15 40    	testb  $0x40,0x15(%r12)
  6b354e:	0f 85 42 0f 00 00    	jne    6b4496 <sqlite3VdbeExec+0x7436>
  6b3554:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3559:	49 83 c5 18          	add    $0x18,%r13
  6b355d:	e9 be 9d ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3562:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b3566:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b356a:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b356e:	4c 8d 3c 08          	lea    (%rax,%rcx,1),%r15
  6b3572:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b3577:	0f 84 58 14 00 00    	je     6b49d5 <sqlite3VdbeExec+0x7975>
  6b357d:	4c 89 ff             	mov    %r15,%rdi
  6b3580:	e8 db 44 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b3585:	e9 52 14 00 00       	jmp    6b49dc <sqlite3VdbeExec+0x797c>
  6b358a:	66 41 c7 45 14 04 00 	movw   $0x4,0x14(%r13)
  6b3591:	66 41 c7 45 14 01 00 	movw   $0x1,0x14(%r13)
  6b3598:	4c 89 ef             	mov    %r13,%rdi
  6b359b:	e8 50 d8 fe ff       	call   6a0df0 <vdbeMemClear>
  6b35a0:	48 8d 05 ff 04 fa ff 	lea    -0x5fb01(%rip),%rax        # 653aa6 <.L.str.302>
  6b35a7:	49 89 45 00          	mov    %rax,0x0(%r13)
  6b35ab:	4d 89 65 08          	mov    %r12,0x8(%r13)
  6b35af:	66 41 c7 45 14 01 1a 	movw   $0x1a01,0x14(%r13)
  6b35b6:	41 c6 45 17 70       	movb   $0x70,0x17(%r13)
  6b35bb:	48 8d 05 6e 75 ff ff 	lea    -0x8a92(%rip),%rax        # 6aab30 <sqlite3VdbeValueListFree>
  6b35c2:	49 89 45 30          	mov    %rax,0x30(%r13)
  6b35c6:	e9 2a 10 00 00       	jmp    6b45f5 <sqlite3VdbeExec+0x7595>
  6b35cb:	a8 12                	test   $0x12,%al
  6b35cd:	0f 84 2c 14 00 00    	je     6b49ff <sqlite3VdbeExec+0x799f>
  6b35d3:	49 89 ff             	mov    %rdi,%r15
  6b35d6:	e8 c5 d9 fe ff       	call   6a0fa0 <memRealValue>
  6b35db:	4c 89 ff             	mov    %r15,%rdi
  6b35de:	66 0f 28 d0          	movapd %xmm0,%xmm2
  6b35e2:	e9 82 de ff ff       	jmp    6b1469 <sqlite3VdbeExec+0x4409>
  6b35e7:	a8 12                	test   $0x12,%al
  6b35e9:	0f 84 19 14 00 00    	je     6b4a08 <sqlite3VdbeExec+0x79a8>
  6b35ef:	49 89 ff             	mov    %rdi,%r15
  6b35f2:	4c 89 ef             	mov    %r13,%rdi
  6b35f5:	f2 0f 11 54 24 38    	movsd  %xmm2,0x38(%rsp)
  6b35fb:	e8 a0 d9 fe ff       	call   6a0fa0 <memRealValue>
  6b3600:	f2 0f 10 54 24 38    	movsd  0x38(%rsp),%xmm2
  6b3606:	4c 89 ff             	mov    %r15,%rdi
  6b3609:	e9 7d de ff ff       	jmp    6b148b <sqlite3VdbeExec+0x442b>
  6b360e:	41 0f b6 45 0c       	movzbl 0xc(%r13),%eax
  6b3613:	49 8b 4e 18          	mov    0x18(%r14),%rcx
  6b3617:	88 41 70             	mov    %al,0x70(%rcx)
  6b361a:	41 83 7d 04 01       	cmpl   $0x1,0x4(%r13)
  6b361f:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b3624:	75 2d                	jne    6b3653 <sqlite3VdbeExec+0x65f3>
  6b3626:	49 8b 44 24 08       	mov    0x8(%r12),%rax
  6b362b:	48 85 c0             	test   %rax,%rax
  6b362e:	74 1b                	je     6b364b <sqlite3VdbeExec+0x65eb>
  6b3630:	8b 88 c8 00 00 00    	mov    0xc8(%rax),%ecx
  6b3636:	83 e1 fc             	and    $0xfffffffc,%ecx
  6b3639:	ff c1                	inc    %ecx
  6b363b:	66 89 88 c8 00 00 00 	mov    %cx,0xc8(%rax)
  6b3642:	48 8b 40 10          	mov    0x10(%rax),%rax
  6b3646:	48 85 c0             	test   %rax,%rax
  6b3649:	75 e5                	jne    6b3630 <sqlite3VdbeExec+0x65d0>
  6b364b:	41 80 a6 c8 00 00 00 	andb   $0xfc,0xc8(%r14)
  6b3652:	fc 
  6b3653:	4c 89 f8             	mov    %r15,%rax
  6b3656:	85 c0                	test   %eax,%eax
  6b3658:	0f 84 8b 13 00 00    	je     6b49e9 <sqlite3VdbeExec+0x7989>
  6b365e:	e9 56 28 00 00       	jmp    6b5eb9 <sqlite3VdbeExec+0x8e59>
  6b3663:	48 c7 07 00 00 00 00 	movq   $0x0,(%rdi)
  6b366a:	66 c7 47 14 04 00    	movw   $0x4,0x14(%rdi)
  6b3670:	49 83 c5 18          	add    $0x18,%r13
  6b3674:	e9 a7 9c ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3679:	48 8b 4d 18          	mov    0x18(%rbp),%rcx
  6b367d:	42 f6 04 39 02       	testb  $0x2,(%rcx,%r15,1)
  6b3682:	0f 85 59 10 00 00    	jne    6b46e1 <sqlite3VdbeExec+0x7681>
  6b3688:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b368d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3691:	e9 65 10 00 00       	jmp    6b46fb <sqlite3VdbeExec+0x769b>
  6b3696:	48 89 c1             	mov    %rax,%rcx
  6b3699:	48 8b 87 d0 01 00 00 	mov    0x1d0(%rdi),%rax
  6b36a0:	49 89 04 24          	mov    %rax,(%r12)
  6b36a4:	48 89 c8             	mov    %rcx,%rax
  6b36a7:	4c 89 a7 d0 01 00 00 	mov    %r12,0x1d0(%rdi)
  6b36ae:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b36b3:	e9 74 b4 ff ff       	jmp    6aeb2c <sqlite3VdbeExec+0x1acc>
  6b36b8:	49 c7 44 24 10 00 00 	movq   $0x0,0x10(%r12)
  6b36bf:	00 00 
  6b36c1:	41 c7 44 24 18 00 00 	movl   $0x0,0x18(%r12)
  6b36c8:	00 00 
  6b36ca:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b36cf:	41 8b 74 24 3c       	mov    0x3c(%r12),%esi
  6b36d4:	31 d2                	xor    %edx,%edx
  6b36d6:	e8 65 8e fe ff       	call   69c540 <sqlite3BtreeClearTable>
  6b36db:	85 c0                	test   %eax,%eax
  6b36dd:	0f 84 19 03 00 00    	je     6b39fc <sqlite3VdbeExec+0x699c>
  6b36e3:	e9 51 23 00 00       	jmp    6b5a39 <sqlite3VdbeExec+0x89d9>
  6b36e8:	4c 89 f7             	mov    %r14,%rdi
  6b36eb:	e8 b0 29 ff ff       	call   6a60a0 <freeCursorWithCache>
  6b36f0:	49 8b 46 78          	mov    0x78(%r14),%rax
  6b36f4:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b36f8:	48 c7 04 c8 00 00 00 	movq   $0x0,(%rax,%rcx,8)
  6b36ff:	00 
  6b3700:	49 83 c5 18          	add    $0x18,%r13
  6b3704:	e9 17 9c ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3709:	41 0f b6 4d 10       	movzbl 0x10(%r13),%ecx
  6b370e:	fe c9                	dec    %cl
  6b3710:	b8 01 00 00 00       	mov    $0x1,%eax
  6b3715:	d3 e0                	shl    %cl,%eax
  6b3717:	e9 31 10 00 00       	jmp    6b474d <sqlite3VdbeExec+0x76ed>
  6b371c:	48 8b 7c 24 28       	mov    0x28(%rsp),%rdi
  6b3721:	4c 8d 0c 17          	lea    (%rdi,%rdx,1),%r9
  6b3725:	49 83 c1 38          	add    $0x38,%r9
  6b3729:	89 cf                	mov    %ecx,%edi
  6b372b:	81 e7 fc ff ff 7f    	and    $0x7ffffffc,%edi
  6b3731:	66 0f 6f 05 b7 78 f9 	movdqa -0x68749(%rip),%xmm0        # 64aff0 <.LCPI1509_1>
  6b3738:	ff 
  6b3739:	45 31 d2             	xor    %r10d,%r10d
  6b373c:	0f 1f 40 00          	nopl   0x0(%rax)
  6b3740:	66 0f 6f 0d a8 7c f9 	movdqa -0x68358(%rip),%xmm1        # 64b3f0 <.LCPI830_2>
  6b3747:	ff 
  6b3748:	66 0f 6f d0          	movdqa %xmm0,%xmm2
  6b374c:	66 0f f4 d1          	pmuludq %xmm1,%xmm2
  6b3750:	66 0f 6f d8          	movdqa %xmm0,%xmm3
  6b3754:	66 0f 73 d3 20       	psrlq  $0x20,%xmm3
  6b3759:	66 0f f4 d9          	pmuludq %xmm1,%xmm3
  6b375d:	66 0f 73 f3 20       	psllq  $0x20,%xmm3
  6b3762:	66 49 0f 6e e1       	movq   %r9,%xmm4
  6b3767:	66 0f 70 e4 44       	pshufd $0x44,%xmm4,%xmm4
  6b376c:	66 0f d4 e2          	paddq  %xmm2,%xmm4
  6b3770:	66 0f d4 e3          	paddq  %xmm3,%xmm4
  6b3774:	66 0f d4 cc          	paddq  %xmm4,%xmm1
  6b3778:	66 0f d4 25 f0 7b f9 	paddq  -0x68410(%rip),%xmm4        # 64b370 <.LCPI830_8>
  6b377f:	ff 
  6b3780:	f3 43 0f 7f 0c d0    	movdqu %xmm1,(%r8,%r10,8)
  6b3786:	f3 43 0f 7f 64 d0 10 	movdqu %xmm4,0x10(%r8,%r10,8)
  6b378d:	49 83 c2 04          	add    $0x4,%r10
  6b3791:	66 0f d4 05 37 7c f9 	paddq  -0x683c9(%rip),%xmm0        # 64b3d0 <.LCPI1509_2>
  6b3798:	ff 
  6b3799:	4c 39 d7             	cmp    %r10,%rdi
  6b379c:	75 a2                	jne    6b3740 <sqlite3VdbeExec+0x66e0>
  6b379e:	39 f8                	cmp    %edi,%eax
  6b37a0:	74 2e                	je     6b37d0 <sqlite3VdbeExec+0x6770>
  6b37a2:	4c 6b cf 38          	imul   $0x38,%rdi,%r9
  6b37a6:	49 01 d1             	add    %rdx,%r9
  6b37a9:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b37ae:	4c 01 ca             	add    %r9,%rdx
  6b37b1:	48 83 c2 70          	add    $0x70,%rdx
  6b37b5:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  6b37bc:	00 00 00 00 
  6b37c0:	49 89 14 f8          	mov    %rdx,(%r8,%rdi,8)
  6b37c4:	48 ff c7             	inc    %rdi
  6b37c7:	48 83 c2 38          	add    $0x38,%rdx
  6b37cb:	48 39 f8             	cmp    %rdi,%rax
  6b37ce:	75 f0                	jne    6b37c0 <sqlite3VdbeExec+0x6760>
  6b37d0:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b37d5:	48 8b 50 10          	mov    0x10(%rax),%rdx
  6b37d9:	4c 89 e7             	mov    %r12,%rdi
  6b37dc:	ff 55 40             	call   *0x40(%rbp)
  6b37df:	41 89 c6             	mov    %eax,%r14d
  6b37e2:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
  6b37e7:	4c 89 ee             	mov    %r13,%rsi
  6b37ea:	e8 21 63 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6b37ef:	4c 89 f0             	mov    %r14,%rax
  6b37f2:	85 c0                	test   %eax,%eax
  6b37f4:	0f 85 8e 26 00 00    	jne    6b5e88 <sqlite3VdbeExec+0x8e28>
  6b37fa:	4c 89 e7             	mov    %r12,%rdi
  6b37fd:	ff 55 50             	call   *0x50(%rbp)
  6b3800:	41 c6 47 02 00       	movb   $0x0,0x2(%r15)
  6b3805:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b380c:	00 00 
  6b380e:	85 c0                	test   %eax,%eax
  6b3810:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3815:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3819:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b381e:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b3823:	0f 85 43 20 00 00    	jne    6b586c <sqlite3VdbeExec+0x880c>
  6b3829:	e9 e9 9a ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6b382e:	48 8b 07             	mov    (%rdi),%rax
  6b3831:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
  6b3835:	48 8b 39             	mov    (%rcx),%rdi
  6b3838:	8b b0 e8 02 00 00    	mov    0x2e8(%rax),%esi
  6b383e:	3b b7 80 00 00 00    	cmp    0x80(%rdi),%esi
  6b3844:	7e 10                	jle    6b3856 <sqlite3VdbeExec+0x67f6>
  6b3846:	80 7f 0a 00          	cmpb   $0x0,0xa(%rdi)
  6b384a:	74 0a                	je     6b3856 <sqlite3VdbeExec+0x67f6>
  6b384c:	e8 df a2 fd ff       	call   68db30 <pagerOpenSavepoint>
  6b3851:	e9 38 a2 ff ff       	jmp    6ada8e <sqlite3VdbeExec+0xa2e>
  6b3856:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b385b:	48 8b 68 10          	mov    0x10(%rax),%rbp
  6b385f:	49 89 6c 24 30       	mov    %rbp,0x30(%r12)
  6b3864:	48 85 ed             	test   %rbp,%rbp
  6b3867:	0f 84 28 01 00 00    	je     6b3995 <sqlite3VdbeExec+0x6935>
  6b386d:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b3872:	4c 89 e6             	mov    %r12,%rsi
  6b3875:	48 83 c6 3c          	add    $0x3c,%rsi
  6b3879:	0f b7 50 02          	movzwl 0x2(%rax),%edx
  6b387d:	83 ca 02             	or     $0x2,%edx
  6b3880:	e8 ab 87 fe ff       	call   69c030 <sqlite3BtreeCreateTable>
  6b3885:	85 c0                	test   %eax,%eax
  6b3887:	75 2d                	jne    6b38b6 <sqlite3VdbeExec+0x6856>
  6b3889:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b388e:	4d 8b 44 24 28       	mov    0x28(%r12),%r8
  6b3893:	41 8b 74 24 3c       	mov    0x3c(%r12),%esi
  6b3898:	80 7f 11 00          	cmpb   $0x0,0x11(%rdi)
  6b389c:	74 0b                	je     6b38a9 <sqlite3VdbeExec+0x6849>
  6b389e:	48 8b 07             	mov    (%rdi),%rax
  6b38a1:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
  6b38a5:	48 89 41 08          	mov    %rax,0x8(%rcx)
  6b38a9:	ba 04 00 00 00       	mov    $0x4,%edx
  6b38ae:	48 89 e9             	mov    %rbp,%rcx
  6b38b1:	e8 9a 09 fe ff       	call   694250 <btreeCursor>
  6b38b6:	31 c9                	xor    %ecx,%ecx
  6b38b8:	e9 0f 01 00 00       	jmp    6b39cc <sqlite3VdbeExec+0x696c>
  6b38bd:	40 b6 01             	mov    $0x1,%sil
  6b38c0:	c6 01 01             	movb   $0x1,(%rcx)
  6b38c3:	48 89 41 28          	mov    %rax,0x28(%rcx)
  6b38c7:	48 89 4f 10          	mov    %rcx,0x10(%rdi)
  6b38cb:	40 88 71 01          	mov    %sil,0x1(%rcx)
  6b38cf:	c6 41 02 00          	movb   $0x0,0x2(%rcx)
  6b38d3:	48 83 bf 88 00 00 00 	cmpq   $0x0,0x88(%rdi)
  6b38da:	00 
  6b38db:	74 12                	je     6b38ef <sqlite3VdbeExec+0x688f>
  6b38dd:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b38e4:	00 00 
  6b38e6:	49 83 c5 18          	add    $0x18,%r13
  6b38ea:	e9 31 9a ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b38ef:	e8 8c 1e 09 00       	call   745780 <allocateTempSpace>
  6b38f4:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b38f9:	49 83 c5 18          	add    $0x18,%r13
  6b38fd:	e9 1e 9a ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3902:	49 89 c6             	mov    %rax,%r14
  6b3905:	83 3d 1c 91 0d 00 00 	cmpl   $0x0,0xd911c(%rip)        # 78ca28 <sqlite3Config>
  6b390c:	48 8b 6c 24 30       	mov    0x30(%rsp),%rbp
  6b3911:	74 19                	je     6b392c <sqlite3VdbeExec+0x68cc>
  6b3913:	4c 89 e7             	mov    %r12,%rdi
  6b3916:	ff 15 44 91 0d 00    	call   *0xd9144(%rip)        # 78ca60 <sqlite3Config+0x38>
  6b391c:	48 98                	cltq
  6b391e:	48 29 05 03 c8 0d 00 	sub    %rax,0xdc803(%rip)        # 790128 <sqlite3Stat>
  6b3925:	48 ff 0d 44 c8 0d 00 	decq   0xdc844(%rip)        # 790170 <sqlite3Stat+0x48>
  6b392c:	4c 89 e7             	mov    %r12,%rdi
  6b392f:	ff 15 1b 91 0d 00    	call   *0xd911b(%rip)        # 78ca50 <sqlite3Config+0x28>
  6b3935:	4c 89 f0             	mov    %r14,%rax
  6b3938:	49 89 ee             	mov    %rbp,%r14
  6b393b:	e9 ec b1 ff ff       	jmp    6aeb2c <sqlite3VdbeExec+0x1acc>
  6b3940:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6b3947:	00 00 00 00 
  6b394b:	49 89 cf             	mov    %rcx,%r15
  6b394e:	49 83 c5 18          	add    $0x18,%r13
  6b3952:	e9 c9 99 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3957:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b395d:	49 83 c5 18          	add    $0x18,%r13
  6b3961:	e9 ba 99 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3966:	48 85 c0             	test   %rax,%rax
  6b3969:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b396d:	79 1e                	jns    6b398d <sqlite3VdbeExec+0x692d>
  6b396f:	48 b9 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rcx
  6b3976:	ff ff 7f 
  6b3979:	48 29 c1             	sub    %rax,%rcx
  6b397c:	48 83 c1 02          	add    $0x2,%rcx
  6b3980:	48 8d 56 01          	lea    0x1(%rsi),%rdx
  6b3984:	48 39 d1             	cmp    %rdx,%rcx
  6b3987:	0f 8f be da ff ff    	jg     6b144b <sqlite3VdbeExec+0x43eb>
  6b398d:	48 01 f0             	add    %rsi,%rax
  6b3990:	e9 3f 0c 00 00       	jmp    6b45d4 <sqlite3VdbeExec+0x7574>
  6b3995:	41 c7 44 24 3c 01 00 	movl   $0x1,0x3c(%r12)
  6b399c:	00 00 
  6b399e:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b39a3:	4d 8b 44 24 28       	mov    0x28(%r12),%r8
  6b39a8:	80 7f 11 00          	cmpb   $0x0,0x11(%rdi)
  6b39ac:	74 0b                	je     6b39b9 <sqlite3VdbeExec+0x6959>
  6b39ae:	48 8b 07             	mov    (%rdi),%rax
  6b39b1:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
  6b39b5:	48 89 41 08          	mov    %rax,0x8(%rcx)
  6b39b9:	be 01 00 00 00       	mov    $0x1,%esi
  6b39be:	ba 04 00 00 00       	mov    $0x4,%edx
  6b39c3:	31 c9                	xor    %ecx,%ecx
  6b39c5:	e8 86 08 fe ff       	call   694250 <btreeCursor>
  6b39ca:	b1 01                	mov    $0x1,%cl
  6b39cc:	41 88 4c 24 04       	mov    %cl,0x4(%r12)
  6b39d1:	48 89 c2             	mov    %rax,%rdx
  6b39d4:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b39d9:	66 83 78 02 08       	cmpw   $0x8,0x2(%rax)
  6b39de:	0f 95 c0             	setne  %al
  6b39e1:	41 0f b6 4c 24 05    	movzbl 0x5(%r12),%ecx
  6b39e7:	c0 e0 02             	shl    $0x2,%al
  6b39ea:	80 e1 fb             	and    $0xfb,%cl
  6b39ed:	08 c1                	or     %al,%cl
  6b39ef:	41 88 4c 24 05       	mov    %cl,0x5(%r12)
  6b39f4:	85 d2                	test   %edx,%edx
  6b39f6:	0f 85 1b 21 00 00    	jne    6b5b17 <sqlite3VdbeExec+0x8ab7>
  6b39fc:	41 c6 44 24 02 01    	movb   $0x1,0x2(%r12)
  6b3a02:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3a09:	00 00 
  6b3a0b:	e9 e5 0b 00 00       	jmp    6b45f5 <sqlite3VdbeExec+0x7595>
  6b3a10:	48 c7 07 01 00 00 00 	movq   $0x1,(%rdi)
  6b3a17:	66 c7 47 14 04 00    	movw   $0x4,0x14(%rdi)
  6b3a1d:	41 c6 44 24 29 00    	movb   $0x0,0x29(%r12)
  6b3a23:	4c 89 f0             	mov    %r14,%rax
  6b3a26:	4d 89 fe             	mov    %r15,%r14
  6b3a29:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3a2d:	49 8b 3c 24          	mov    (%r12),%rdi
  6b3a31:	f6 47 15 90          	testb  $0x90,0x15(%rdi)
  6b3a35:	75 06                	jne    6b3a3d <sqlite3VdbeExec+0x69dd>
  6b3a37:	83 7f 20 00          	cmpl   $0x0,0x20(%rdi)
  6b3a3b:	74 19                	je     6b3a56 <sqlite3VdbeExec+0x69f6>
  6b3a3d:	4d 89 f7             	mov    %r14,%r15
  6b3a40:	49 89 c6             	mov    %rax,%r14
  6b3a43:	e8 a8 d3 fe ff       	call   6a0df0 <vdbeMemClear>
  6b3a48:	4c 89 f0             	mov    %r14,%rax
  6b3a4b:	4d 89 fe             	mov    %r15,%r14
  6b3a4e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3a52:	49 8b 3c 24          	mov    (%r12),%rdi
  6b3a56:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b3a5c:	41 c7 44 24 24 00 00 	movl   $0x0,0x24(%r12)
  6b3a63:	00 00 
  6b3a65:	85 c0                	test   %eax,%eax
  6b3a67:	0f 84 27 17 00 00    	je     6b5194 <sqlite3VdbeExec+0x8134>
  6b3a6d:	e9 c0 27 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b3a72:	4c 89 f7             	mov    %r14,%rdi
  6b3a75:	48 89 ce             	mov    %rcx,%rsi
  6b3a78:	e8 13 8c ff ff       	call   6ac690 <sqlite3VdbeExpandSql>
  6b3a7d:	49 89 c4             	mov    %rax,%r12
  6b3a80:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b3a85:	48 8b b8 f8 00 00 00 	mov    0xf8(%rax),%rdi
  6b3a8c:	4c 89 e6             	mov    %r12,%rsi
  6b3a8f:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b3a94:	ff 90 f0 00 00 00    	call   *0xf0(%rax)
  6b3a9a:	4c 89 e7             	mov    %r12,%rdi
  6b3a9d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3aa2:	e8 e9 54 fc ff       	call   678f90 <sqlite3_free>
  6b3aa7:	e9 b9 0e 00 00       	jmp    6b4965 <sqlite3VdbeExec+0x7905>
  6b3aac:	31 c0                	xor    %eax,%eax
  6b3aae:	41 c6 44 24 03 00    	movb   $0x0,0x3(%r12)
  6b3ab4:	41 c7 44 24 18 00 00 	movl   $0x0,0x18(%r12)
  6b3abb:	00 00 
  6b3abd:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3ac2:	85 c0                	test   %eax,%eax
  6b3ac4:	0f 85 68 27 00 00    	jne    6b6232 <sqlite3VdbeExec+0x91d2>
  6b3aca:	8b 44 24 60          	mov    0x60(%rsp),%eax
  6b3ace:	41 88 44 24 02       	mov    %al,0x2(%r12)
  6b3ad3:	41 83 7d 08 00       	cmpl   $0x0,0x8(%r13)
  6b3ad8:	0f 9f c1             	setg   %cl
  6b3adb:	85 c0                	test   %eax,%eax
  6b3add:	0f 95 c0             	setne  %al
  6b3ae0:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3ae7:	00 00 
  6b3ae9:	84 c1                	test   %al,%cl
  6b3aeb:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3af0:	0f 84 21 98 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b3af6:	e9 71 1d 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b3afb:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3b00:	49 83 c5 18          	add    $0x18,%r13
  6b3b04:	e9 17 98 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3b09:	4c 89 f8             	mov    %r15,%rax
  6b3b0c:	f6 44 24 75 90       	testb  $0x90,0x75(%rsp)
  6b3b11:	75 0a                	jne    6b3b1d <sqlite3VdbeExec+0x6abd>
  6b3b13:	83 bc 24 80 00 00 00 	cmpl   $0x0,0x80(%rsp)
  6b3b1a:	00 
  6b3b1b:	74 16                	je     6b3b33 <sqlite3VdbeExec+0x6ad3>
  6b3b1d:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b3b22:	4d 89 f7             	mov    %r14,%r15
  6b3b25:	49 89 c6             	mov    %rax,%r14
  6b3b28:	e8 c3 d2 fe ff       	call   6a0df0 <vdbeMemClear>
  6b3b2d:	4c 89 f0             	mov    %r14,%rax
  6b3b30:	4d 89 fe             	mov    %r15,%r14
  6b3b33:	85 c0                	test   %eax,%eax
  6b3b35:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3b39:	0f 85 a5 28 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6b3b3f:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3b46:	00 00 
  6b3b48:	49 83 c5 18          	add    $0x18,%r13
  6b3b4c:	e9 cf 97 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3b51:	4c 89 e7             	mov    %r12,%rdi
  6b3b54:	e8 07 22 fe ff       	call   695d60 <sqlite3BtreeRowCountEst>
  6b3b59:	48 89 c7             	mov    %rax,%rdi
  6b3b5c:	e8 0f f0 fc ff       	call   682b70 <sqlite3LogEst>
  6b3b61:	48 0f bf d0          	movswq %ax,%rdx
  6b3b65:	49 63 4d 0c          	movslq 0xc(%r13),%rcx
  6b3b69:	48 39 ca             	cmp    %rcx,%rdx
  6b3b6c:	0f 8c 22 16 00 00    	jl     6b5194 <sqlite3VdbeExec+0x8134>
  6b3b72:	49 63 4d 10          	movslq 0x10(%r13),%rcx
  6b3b76:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3b7d:	00 00 
  6b3b7f:	48 39 ca             	cmp    %rcx,%rdx
  6b3b82:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3b87:	0f 8f 8a 97 ff ff    	jg     6ad317 <sqlite3VdbeExec+0x2b7>
  6b3b8d:	e9 da 1c 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b3b92:	0f b6 4c 24 1f       	movzbl 0x1f(%rsp),%ecx
  6b3b97:	41 3a 4c 24 16       	cmp    0x16(%r12),%cl
  6b3b9c:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3ba1:	74 19                	je     6b3bbc <sqlite3VdbeExec+0x6b5c>
  6b3ba3:	0f b6 f1             	movzbl %cl,%esi
  6b3ba6:	4c 89 e7             	mov    %r12,%rdi
  6b3ba9:	e8 12 bc fc ff       	call   67f7c0 <sqlite3VdbeMemTranslate>
  6b3bae:	85 c0                	test   %eax,%eax
  6b3bb0:	0f 85 86 27 00 00    	jne    6b633c <sqlite3VdbeExec+0x92dc>
  6b3bb6:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b3bbc:	41 c7 44 24 20 00 00 	movl   $0x0,0x20(%r12)
  6b3bc3:	00 00 
  6b3bc5:	0d 00 20 00 00       	or     $0x2000,%eax
  6b3bca:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b3bd0:	41 80 7d 01 fa       	cmpb   $0xfa,0x1(%r13)
  6b3bd5:	75 0e                	jne    6b3be5 <sqlite3VdbeExec+0x6b85>
  6b3bd7:	49 8b 75 10          	mov    0x10(%r13),%rsi
  6b3bdb:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b3be0:	e8 db 69 fc ff       	call   67a5c0 <sqlite3DbFree>
  6b3be5:	41 c6 45 01 fa       	movb   $0xfa,0x1(%r13)
  6b3bea:	49 8b 44 24 08       	mov    0x8(%r12),%rax
  6b3bef:	49 89 45 10          	mov    %rax,0x10(%r13)
  6b3bf3:	41 8b 44 24 10       	mov    0x10(%r12),%eax
  6b3bf8:	41 89 45 04          	mov    %eax,0x4(%r13)
  6b3bfc:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b3c03:	00 00 
  6b3c05:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
  6b3c0a:	3b 81 88 00 00 00    	cmp    0x88(%rcx),%eax
  6b3c10:	0f 8f f8 26 00 00    	jg     6b630e <sqlite3VdbeExec+0x92ae>
  6b3c16:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3c1b:	41 c6 45 00 49       	movb   $0x49,0x0(%r13)
  6b3c20:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b3c25:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3c29:	49 8b 46 68          	mov    0x68(%r14),%rax
  6b3c2d:	49 63 4d 08          	movslq 0x8(%r13),%rcx
  6b3c31:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b3c35:	4c 8d 24 08          	lea    (%rax,%rcx,1),%r12
  6b3c39:	f6 44 08 15 90       	testb  $0x90,0x15(%rax,%rcx,1)
  6b3c3e:	74 0a                	je     6b3c4a <sqlite3VdbeExec+0x6bea>
  6b3c40:	4c 89 e7             	mov    %r12,%rdi
  6b3c43:	e8 18 3e 09 00       	call   747a60 <out2PrereleaseWithClear>
  6b3c48:	eb 08                	jmp    6b3c52 <sqlite3VdbeExec+0x6bf2>
  6b3c4a:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b3c51:	00 
  6b3c52:	66 41 c7 44 24 14 02 	movw   $0x2202,0x14(%r12)
  6b3c59:	22 
  6b3c5a:	49 8b 45 10          	mov    0x10(%r13),%rax
  6b3c5e:	49 89 44 24 08       	mov    %rax,0x8(%r12)
  6b3c63:	41 8b 45 04          	mov    0x4(%r13),%eax
  6b3c67:	41 89 44 24 10       	mov    %eax,0x10(%r12)
  6b3c6c:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b3c71:	41 88 44 24 16       	mov    %al,0x16(%r12)
  6b3c76:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b3c7a:	48 85 c0             	test   %rax,%rax
  6b3c7d:	7e 2a                	jle    6b3ca9 <sqlite3VdbeExec+0x6c49>
  6b3c7f:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b3c83:	41 0f b7 4d 02       	movzwl 0x2(%r13),%ecx
  6b3c88:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
  6b3c8d:	48 39 0c 02          	cmp    %rcx,(%rdx,%rax,1)
  6b3c91:	75 24                	jne    6b3cb7 <sqlite3VdbeExec+0x6c57>
  6b3c93:	66 41 c7 44 24 14 10 	movw   $0x2210,0x14(%r12)
  6b3c9a:	22 
  6b3c9b:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3ca0:	49 83 c5 18          	add    $0x18,%r13
  6b3ca4:	e9 77 96 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3ca9:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3cae:	49 83 c5 18          	add    $0x18,%r13
  6b3cb2:	e9 69 96 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3cb7:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3cbc:	49 83 c5 18          	add    $0x18,%r13
  6b3cc0:	e9 5b 96 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3cc5:	48 8b 50 28          	mov    0x28(%rax),%rdx
  6b3cc9:	48 89 51 10          	mov    %rdx,0x10(%rcx)
  6b3ccd:	4c 89 e7             	mov    %r12,%rdi
  6b3cd0:	48 89 c6             	mov    %rax,%rsi
  6b3cd3:	e8 28 87 02 00       	call   6dc400 <sqlite3FreeIndex>
  6b3cd8:	41 80 4c 24 2c 01    	orb    $0x1,0x2c(%r12)
  6b3cde:	49 83 c5 18          	add    $0x18,%r13
  6b3ce2:	e9 39 96 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3ce7:	ff 48 2c             	decl   0x2c(%rax)
  6b3cea:	0f 84 a2 af ff ff    	je     6aec92 <sqlite3VdbeExec+0x1c32>
  6b3cf0:	41 80 4c 24 2c 01    	orb    $0x1,0x2c(%r12)
  6b3cf6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b3cfa:	49 83 c5 18          	add    $0x18,%r13
  6b3cfe:	e9 1d 96 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3d03:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b3d0a:	00 
  6b3d0b:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3d10:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3d15:	49 83 c5 18          	add    $0x18,%r13
  6b3d19:	e9 02 96 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3d1e:	48 63 40 08          	movslq 0x8(%rax),%rax
  6b3d22:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b3d26:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b3d2b:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b3d2f:	49 83 c5 e8          	add    $0xffffffffffffffe8,%r13
  6b3d33:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6b3d3a:	00 00 00 00 
  6b3d3e:	49 83 c5 18          	add    $0x18,%r13
  6b3d42:	e9 d9 95 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3d47:	85 ed                	test   %ebp,%ebp
  6b3d49:	78 20                	js     6b3d6b <sqlite3VdbeExec+0x6d0b>
  6b3d4b:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b3d50:	49 8b 36             	mov    (%r14),%rsi
  6b3d53:	e8 28 46 fd ff       	call   688380 <sqlite3RowSetInsert>
  6b3d58:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b3d5d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3d62:	49 83 c5 18          	add    $0x18,%r13
  6b3d66:	e9 b5 95 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3d6b:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b3d70:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3d75:	49 83 c5 18          	add    $0x18,%r13
  6b3d79:	e9 a2 95 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b3d7e:	31 c0                	xor    %eax,%eax
  6b3d80:	e9 5a 97 ff ff       	jmp    6ad4df <sqlite3VdbeExec+0x47f>
  6b3d85:	45 31 e4             	xor    %r12d,%r12d
  6b3d88:	e9 43 97 ff ff       	jmp    6ad4d0 <sqlite3VdbeExec+0x470>
  6b3d8d:	80 fa 69             	cmp    $0x69,%dl
  6b3d90:	0f 85 c6 0e 00 00    	jne    6b4c5c <sqlite3VdbeExec+0x7bfc>
  6b3d96:	49 d3 e4             	shl    %cl,%r12
  6b3d99:	e9 49 1a 00 00       	jmp    6b57e7 <sqlite3VdbeExec+0x8787>
  6b3d9e:	41 0f b7 45 10       	movzwl 0x10(%r13),%eax
  6b3da3:	66 41 89 47 06       	mov    %ax,0x6(%r15)
  6b3da8:	e9 3c 0c 00 00       	jmp    6b49e9 <sqlite3VdbeExec+0x7989>
  6b3dad:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b3db2:	85 d2                	test   %edx,%edx
  6b3db4:	74 45                	je     6b3dfb <sqlite3VdbeExec+0x6d9b>
  6b3db6:	44 89 fd             	mov    %r15d,%ebp
  6b3db9:	4d 89 f7             	mov    %r14,%r15
  6b3dbc:	49 89 c6             	mov    %rax,%r14
  6b3dbf:	49 8b 44 24 08       	mov    0x8(%r12),%rax
  6b3dc4:	48 85 c0             	test   %rax,%rax
  6b3dc7:	74 1b                	je     6b3de4 <sqlite3VdbeExec+0x6d84>
  6b3dc9:	8b 88 c8 00 00 00    	mov    0xc8(%rax),%ecx
  6b3dcf:	83 e1 fc             	and    $0xfffffffc,%ecx
  6b3dd2:	ff c1                	inc    %ecx
  6b3dd4:	66 89 88 c8 00 00 00 	mov    %cx,0xc8(%rax)
  6b3ddb:	48 8b 40 10          	mov    0x10(%rax),%rax
  6b3ddf:	48 85 c0             	test   %rax,%rax
  6b3de2:	75 e5                	jne    6b3dc9 <sqlite3VdbeExec+0x6d69>
  6b3de4:	4c 89 e7             	mov    %r12,%rdi
  6b3de7:	e8 04 bd fe ff       	call   69faf0 <sqlite3ResetAllSchemasOfConnection>
  6b3dec:	41 80 4c 24 2c 01    	orb    $0x1,0x2c(%r12)
  6b3df2:	4c 89 f0             	mov    %r14,%rax
  6b3df5:	4d 89 fe             	mov    %r15,%r14
  6b3df8:	41 89 ef             	mov    %ebp,%r15d
  6b3dfb:	85 c0                	test   %eax,%eax
  6b3dfd:	48 8b 6c 24 50       	mov    0x50(%rsp),%rbp
  6b3e02:	74 20                	je     6b3e24 <sqlite3VdbeExec+0x6dc4>
  6b3e04:	e9 b0 20 00 00       	jmp    6b5eb9 <sqlite3VdbeExec+0x8e59>
  6b3e09:	49 8b 84 24 d0 01 00 	mov    0x1d0(%r12),%rax
  6b3e10:	00 
  6b3e11:	48 89 06             	mov    %rax,(%rsi)
  6b3e14:	49 89 b4 24 d0 01 00 	mov    %rsi,0x1d0(%r12)
  6b3e1b:	00 
  6b3e1c:	41 ff 8c 24 e8 02 00 	decl   0x2e8(%r12)
  6b3e23:	00 
  6b3e24:	49 8b b4 24 d8 02 00 	mov    0x2d8(%r12),%rsi
  6b3e2b:	00 
  6b3e2c:	48 39 ee             	cmp    %rbp,%rsi
  6b3e2f:	0f 84 aa 00 00 00    	je     6b3edf <sqlite3VdbeExec+0x6e7f>
  6b3e35:	48 8b 46 18          	mov    0x18(%rsi),%rax
  6b3e39:	49 89 84 24 d8 02 00 	mov    %rax,0x2d8(%r12)
  6b3e40:	00 
  6b3e41:	49 3b b4 24 e8 01 00 	cmp    0x1e8(%r12),%rsi
  6b3e48:	00 
  6b3e49:	73 14                	jae    6b3e5f <sqlite3VdbeExec+0x6dff>
  6b3e4b:	49 3b b4 24 d8 01 00 	cmp    0x1d8(%r12),%rsi
  6b3e52:	00 
  6b3e53:	73 b4                	jae    6b3e09 <sqlite3VdbeExec+0x6da9>
  6b3e55:	49 3b b4 24 e0 01 00 	cmp    0x1e0(%r12),%rsi
  6b3e5c:	00 
  6b3e5d:	73 60                	jae    6b3ebf <sqlite3VdbeExec+0x6e5f>
  6b3e5f:	49 83 bc 24 00 03 00 	cmpq   $0x0,0x300(%r12)
  6b3e66:	00 00 
  6b3e68:	74 12                	je     6b3e7c <sqlite3VdbeExec+0x6e1c>
  6b3e6a:	4c 89 e7             	mov    %r12,%rdi
  6b3e6d:	e8 6e 66 fc ff       	call   67a4e0 <measureAllocationSize>
  6b3e72:	41 ff 8c 24 e8 02 00 	decl   0x2e8(%r12)
  6b3e79:	00 
  6b3e7a:	eb a8                	jmp    6b3e24 <sqlite3VdbeExec+0x6dc4>
  6b3e7c:	83 3d a5 8b 0d 00 00 	cmpl   $0x0,0xd8ba5(%rip)        # 78ca28 <sqlite3Config>
  6b3e83:	74 24                	je     6b3ea9 <sqlite3VdbeExec+0x6e49>
  6b3e85:	48 89 f7             	mov    %rsi,%rdi
  6b3e88:	49 89 f6             	mov    %rsi,%r14
  6b3e8b:	ff 15 cf 8b 0d 00    	call   *0xd8bcf(%rip)        # 78ca60 <sqlite3Config+0x38>
  6b3e91:	4c 89 f6             	mov    %r14,%rsi
  6b3e94:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b3e99:	48 98                	cltq
  6b3e9b:	48 29 05 86 c2 0d 00 	sub    %rax,0xdc286(%rip)        # 790128 <sqlite3Stat>
  6b3ea2:	48 ff 0d c7 c2 0d 00 	decq   0xdc2c7(%rip)        # 790170 <sqlite3Stat+0x48>
  6b3ea9:	48 89 f7             	mov    %rsi,%rdi
  6b3eac:	ff 15 9e 8b 0d 00    	call   *0xd8b9e(%rip)        # 78ca50 <sqlite3Config+0x28>
  6b3eb2:	41 ff 8c 24 e8 02 00 	decl   0x2e8(%r12)
  6b3eb9:	00 
  6b3eba:	e9 65 ff ff ff       	jmp    6b3e24 <sqlite3VdbeExec+0x6dc4>
  6b3ebf:	49 8b 84 24 c0 01 00 	mov    0x1c0(%r12),%rax
  6b3ec6:	00 
  6b3ec7:	48 89 06             	mov    %rax,(%rsi)
  6b3eca:	49 89 b4 24 c0 01 00 	mov    %rsi,0x1c0(%r12)
  6b3ed1:	00 
  6b3ed2:	41 ff 8c 24 e8 02 00 	decl   0x2e8(%r12)
  6b3ed9:	00 
  6b3eda:	e9 45 ff ff ff       	jmp    6b3e24 <sqlite3VdbeExec+0x6dc4>
  6b3edf:	41 83 ff 01          	cmp    $0x1,%r15d
  6b3ee3:	0f 85 90 00 00 00    	jne    6b3f79 <sqlite3VdbeExec+0x6f19>
  6b3ee9:	48 8b 45 18          	mov    0x18(%rbp),%rax
  6b3eed:	49 89 84 24 d8 02 00 	mov    %rax,0x2d8(%r12)
  6b3ef4:	00 
  6b3ef5:	4c 89 e7             	mov    %r12,%rdi
  6b3ef8:	48 89 ee             	mov    %rbp,%rsi
  6b3efb:	e8 c0 66 fc ff       	call   67a5c0 <sqlite3DbFree>
  6b3f00:	8b 4c 24 40          	mov    0x40(%rsp),%ecx
  6b3f04:	84 c9                	test   %cl,%cl
  6b3f06:	0f 85 80 00 00 00    	jne    6b3f8c <sqlite3VdbeExec+0x6f2c>
  6b3f0c:	41 ff 8c 24 e8 02 00 	decl   0x2e8(%r12)
  6b3f13:	00 
  6b3f14:	e9 82 00 00 00       	jmp    6b3f9b <sqlite3VdbeExec+0x6f3b>
  6b3f19:	66 0f 2e 0d 27 77 f9 	ucomisd -0x688d9(%rip),%xmm1        # 64b648 <.LCPI1090_1>
  6b3f20:	ff 
  6b3f21:	77 05                	ja     6b3f28 <sqlite3VdbeExec+0x6ec8>
  6b3f23:	f2 48 0f 2c c1       	cvttsd2si %xmm1,%rax
  6b3f28:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b3f2c:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b3f30:	75 02                	jne    6b3f34 <sqlite3VdbeExec+0x6ed4>
  6b3f32:	7b 38                	jnp    6b3f6c <sqlite3VdbeExec+0x6f0c>
  6b3f34:	48 b9 00 00 00 00 00 	movabs $0xfff8000000000000,%rcx
  6b3f3b:	00 f8 ff 
  6b3f3e:	48 01 c1             	add    %rax,%rcx
  6b3f41:	48 c1 e9 34          	shr    $0x34,%rcx
  6b3f45:	81 f9 ff 0f 00 00    	cmp    $0xfff,%ecx
  6b3f4b:	0f 82 dc 0f 00 00    	jb     6b4f2d <sqlite3VdbeExec+0x7ecd>
  6b3f51:	0f 57 c0             	xorps  %xmm0,%xmm0
  6b3f54:	f2 48 0f 2a c0       	cvtsi2sd %rax,%xmm0
  6b3f59:	66 48 0f 7e c9       	movq   %xmm1,%rcx
  6b3f5e:	66 48 0f 7e c2       	movq   %xmm0,%rdx
  6b3f63:	48 39 d1             	cmp    %rdx,%rcx
  6b3f66:	0f 85 c1 0f 00 00    	jne    6b4f2d <sqlite3VdbeExec+0x7ecd>
  6b3f6c:	49 89 04 24          	mov    %rax,(%r12)
  6b3f70:	66 b8 04 00          	mov    $0x4,%ax
  6b3f74:	e9 e5 0f 00 00       	jmp    6b4f5e <sqlite3VdbeExec+0x7efe>
  6b3f79:	66 0f 10 45 08       	movupd 0x8(%rbp),%xmm0
  6b3f7e:	66 41 0f 11 84 24 f0 	movupd %xmm0,0x2f0(%r12)
  6b3f85:	02 00 00 
  6b3f88:	8b 4c 24 40          	mov    0x40(%rsp),%ecx
  6b3f8c:	41 83 ff 02          	cmp    $0x2,%r15d
  6b3f90:	0f 95 c0             	setne  %al
  6b3f93:	84 c8                	test   %cl,%al
  6b3f95:	0f 85 41 0f 00 00    	jne    6b4edc <sqlite3VdbeExec+0x7e7c>
  6b3f9b:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
  6b3fa0:	44 89 fe             	mov    %r15d,%esi
  6b3fa3:	8b 54 24 38          	mov    0x38(%rsp),%edx
  6b3fa7:	e8 d4 29 00 00       	call   6b6980 <sqlite3VtabSavepoint>
  6b3fac:	85 c0                	test   %eax,%eax
  6b3fae:	0f 84 28 0f 00 00    	je     6b4edc <sqlite3VdbeExec+0x7e7c>
  6b3fb4:	e9 ab 20 00 00       	jmp    6b6064 <sqlite3VdbeExec+0x9004>
  6b3fb9:	41 83 7d 0c 00       	cmpl   $0x0,0xc(%r13)
  6b3fbe:	0f 85 a8 18 00 00    	jne    6b586c <sqlite3VdbeExec+0x880c>
  6b3fc4:	e9 4e 93 ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6b3fc9:	c7 84 24 b0 00 00 00 	movl   $0xffffffff,0xb0(%rsp)
  6b3fd0:	ff ff ff ff 
  6b3fd4:	48 8d 05 35 8a 0d 00 	lea    0xd8a35(%rip),%rax        # 78ca10 <sqlite3aLTb>
  6b3fdb:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3fe0:	e9 bb 00 00 00       	jmp    6b40a0 <sqlite3VdbeExec+0x7040>
  6b3fe5:	44 89 d1             	mov    %r10d,%ecx
  6b3fe8:	81 e1 d3 ff 00 00    	and    $0xffd3,%ecx
  6b3fee:	66 41 89 48 14       	mov    %cx,0x14(%r8)
  6b3ff3:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b3ff8:	a8 02                	test   $0x2,%al
  6b3ffa:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b3fff:	4c 89 54 24 50       	mov    %r10,0x50(%rsp)
  6b4004:	75 2f                	jne    6b4035 <sqlite3VdbeExec+0x6fd5>
  6b4006:	a8 2c                	test   $0x2c,%al
  6b4008:	74 2f                	je     6b4039 <sqlite3VdbeExec+0x6fd9>
  6b400a:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b400f:	ba 01 00 00 00       	mov    $0x1,%edx
  6b4014:	e8 77 ca fe ff       	call   6a0a90 <sqlite3VdbeMemStringify>
  6b4019:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
  6b401e:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b4023:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b4028:	23 47 14             	and    0x14(%rdi),%eax
  6b402b:	81 e5 bd 0d 00 00    	and    $0xdbd,%ebp
  6b4031:	09 c5                	or     %eax,%ebp
  6b4033:	eb 04                	jmp    6b4039 <sqlite3VdbeExec+0x6fd9>
  6b4035:	80 67 14 d3          	andb   $0xd3,0x14(%rdi)
  6b4039:	49 8b 55 10          	mov    0x10(%r13),%rdx
  6b403d:	4c 89 c6             	mov    %r8,%rsi
  6b4040:	e8 2b 44 ff ff       	call   6a8470 <sqlite3MemCompare>
  6b4045:	45 0f b6 7d 00       	movzbl 0x0(%r13),%r15d
  6b404a:	85 c0                	test   %eax,%eax
  6b404c:	78 21                	js     6b406f <sqlite3VdbeExec+0x700f>
  6b404e:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b4053:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
  6b4058:	4c 8b 54 24 50       	mov    0x50(%rsp),%r10
  6b405d:	74 2f                	je     6b408e <sqlite3VdbeExec+0x702e>
  6b405f:	89 84 24 b0 00 00 00 	mov    %eax,0xb0(%rsp)
  6b4066:	48 8d 05 b3 89 0d 00 	lea    0xd89b3(%rip),%rax        # 78ca20 <sqlite3aGTb>
  6b406d:	eb 31                	jmp    6b40a0 <sqlite3VdbeExec+0x7040>
  6b406f:	89 84 24 b0 00 00 00 	mov    %eax,0xb0(%rsp)
  6b4076:	48 8d 05 93 89 0d 00 	lea    0xd8993(%rip),%rax        # 78ca10 <sqlite3aLTb>
  6b407d:	4c 8b 44 24 38       	mov    0x38(%rsp),%r8
  6b4082:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
  6b4087:	4c 8b 54 24 50       	mov    0x50(%rsp),%r10
  6b408c:	eb 12                	jmp    6b40a0 <sqlite3VdbeExec+0x7040>
  6b408e:	c7 84 24 b0 00 00 00 	movl   $0x0,0xb0(%rsp)
  6b4095:	00 00 00 00 
  6b4099:	48 8d 05 78 89 0d 00 	lea    0xd8978(%rip),%rax        # 78ca18 <sqlite3aEQb>
  6b40a0:	48 8b 00             	mov    (%rax),%rax
  6b40a3:	41 0f b6 cf          	movzbl %r15b,%ecx
  6b40a7:	80 3c 08 00          	cmpb   $0x0,(%rax,%rcx,1)
  6b40ab:	66 89 6f 14          	mov    %bp,0x14(%rdi)
  6b40af:	66 45 89 50 14       	mov    %r10w,0x14(%r8)
  6b40b4:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b40b8:	0f 85 ae 17 00 00    	jne    6b586c <sqlite3VdbeExec+0x880c>
  6b40be:	e9 54 92 ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6b40c3:	66 41 c7 46 02 00 00 	movw   $0x0,0x2(%r14)
  6b40ca:	41 c7 46 18 00 00 00 	movl   $0x0,0x18(%r14)
  6b40d1:	00 
  6b40d2:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b40d9:	00 00 
  6b40db:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b40e0:	49 83 c5 18          	add    $0x18,%r13
  6b40e4:	e9 37 92 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b40e9:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6b40ed:	e9 0a 12 00 00       	jmp    6b52fc <sqlite3VdbeExec+0x829c>
  6b40f2:	66 0f 2e 05 4e 75 f9 	ucomisd -0x68ab2(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6b40f9:	ff 
  6b40fa:	0f 87 d0 93 ff ff    	ja     6ad4d0 <sqlite3VdbeExec+0x470>
  6b4100:	f2 4c 0f 2c e0       	cvttsd2si %xmm0,%r12
  6b4105:	e9 c6 93 ff ff       	jmp    6ad4d0 <sqlite3VdbeExec+0x470>
  6b410a:	66 0f 2e 05 36 75 f9 	ucomisd -0x68aca(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6b4111:	ff 
  6b4112:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b4117:	0f 87 c7 93 ff ff    	ja     6ad4e4 <sqlite3VdbeExec+0x484>
  6b411d:	f2 48 0f 2c c0       	cvttsd2si %xmm0,%rax
  6b4122:	e9 bd 93 ff ff       	jmp    6ad4e4 <sqlite3VdbeExec+0x484>
  6b4127:	48 b8 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rax
  6b412e:	ff ff 7f 
  6b4131:	f2 41 0f 10 04 24    	movsd  (%r12),%xmm0
  6b4137:	f2 0f 10 0d f1 73 f9 	movsd  -0x68c0f(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6b413e:	ff 
  6b413f:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b4143:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b4147:	0f 86 84 0b 00 00    	jbe    6b4cd1 <sqlite3VdbeExec+0x7c71>
  6b414d:	48 ff c0             	inc    %rax
  6b4150:	e9 ae 9e ff ff       	jmp    6ae003 <sqlite3VdbeExec+0xfa3>
  6b4155:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b415b:	a9 00 90 00 00       	test   $0x9000,%eax
  6b4160:	74 12                	je     6b4174 <sqlite3VdbeExec+0x7114>
  6b4162:	4c 89 e7             	mov    %r12,%rdi
  6b4165:	e8 d6 d3 fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b416a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b416f:	e9 fe cc ff ff       	jmp    6b0e72 <sqlite3VdbeExec+0x3e12>
  6b4174:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b417b:	00 
  6b417c:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b4181:	e9 ec cc ff ff       	jmp    6b0e72 <sqlite3VdbeExec+0x3e12>
  6b4186:	49 89 34 24          	mov    %rsi,(%r12)
  6b418a:	66 41 c7 44 24 14 04 	movw   $0x4,0x14(%r12)
  6b4191:	00 
  6b4192:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b4197:	49 83 c5 18          	add    $0x18,%r13
  6b419b:	e9 80 91 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b41a0:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b41a7:	00 
  6b41a8:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b41ad:	49 83 c5 18          	add    $0x18,%r13
  6b41b1:	e9 6a 91 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b41b6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b41ba:	49 83 c5 18          	add    $0x18,%r13
  6b41be:	e9 5d 91 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b41c3:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b41c9:	49 83 c5 18          	add    $0x18,%r13
  6b41cd:	e9 4e 91 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b41d2:	48 89 37             	mov    %rsi,(%rdi)
  6b41d5:	66 c7 47 14 04 00    	movw   $0x4,0x14(%rdi)
  6b41db:	49 83 c5 18          	add    $0x18,%r13
  6b41df:	e9 3c 91 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b41e4:	a9 00 90 00 00       	test   $0x9000,%eax
  6b41e9:	0f 84 db 11 00 00    	je     6b53ca <sqlite3VdbeExec+0x836a>
  6b41ef:	4c 89 e7             	mov    %r12,%rdi
  6b41f2:	e8 49 d3 fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b41f7:	e9 d6 11 00 00       	jmp    6b53d2 <sqlite3VdbeExec+0x8372>
  6b41fc:	48 b8 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rax
  6b4203:	ff ff 7f 
  6b4206:	f2 41 0f 10 04 24    	movsd  (%r12),%xmm0
  6b420c:	f2 0f 10 0d 1c 73 f9 	movsd  -0x68ce4(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6b4213:	ff 
  6b4214:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b4218:	0f 86 3c 0c 00 00    	jbe    6b4e5a <sqlite3VdbeExec+0x7dfa>
  6b421e:	48 ff c0             	inc    %rax
  6b4221:	e9 fa ba ff ff       	jmp    6afd20 <sqlite3VdbeExec+0x2cc0>
  6b4226:	48 b8 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rax
  6b422d:	ff ff 7f 
  6b4230:	f2 41 0f 10 45 00    	movsd  0x0(%r13),%xmm0
  6b4236:	f2 0f 10 0d f2 72 f9 	movsd  -0x68d0e(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6b423d:	ff 
  6b423e:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b4242:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b4246:	0f 86 26 0c 00 00    	jbe    6b4e72 <sqlite3VdbeExec+0x7e12>
  6b424c:	48 ff c0             	inc    %rax
  6b424f:	49 89 45 00          	mov    %rax,0x0(%r13)
  6b4253:	81 e1 40 f2 ff ff    	and    $0xfffff240,%ecx
  6b4259:	83 c9 04             	or     $0x4,%ecx
  6b425c:	66 41 89 4d 14       	mov    %cx,0x14(%r13)
  6b4261:	e9 8f 03 00 00       	jmp    6b45f5 <sqlite3VdbeExec+0x7595>
  6b4266:	49 01 46 50          	add    %rax,0x50(%r14)
  6b426a:	49 83 c5 18          	add    $0x18,%r13
  6b426e:	e9 ad 90 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b4273:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b4278:	48 89 ef             	mov    %rbp,%rdi
  6b427b:	31 d2                	xor    %edx,%edx
  6b427d:	e8 0e c8 fe ff       	call   6a0a90 <sqlite3VdbeMemStringify>
  6b4282:	85 c0                	test   %eax,%eax
  6b4284:	0f 85 b7 20 00 00    	jne    6b6341 <sqlite3VdbeExec+0x92e1>
  6b428a:	0f b7 45 14          	movzwl 0x14(%rbp),%eax
  6b428e:	83 e0 fd             	and    $0xfffffffd,%eax
  6b4291:	89 44 24 38          	mov    %eax,0x38(%rsp)
  6b4295:	4c 8b 7c 24 40       	mov    0x40(%rsp),%r15
  6b429a:	41 0f b7 77 14       	movzwl 0x14(%r15),%esi
  6b429f:	4c 8b 84 24 b8 00 00 	mov    0xb8(%rsp),%r8
  6b42a6:	00 
  6b42a7:	0f b7 c6             	movzwl %si,%eax
  6b42aa:	a8 12                	test   $0x12,%al
  6b42ac:	74 30                	je     6b42de <sqlite3VdbeExec+0x727e>
  6b42ae:	a9 00 04 00 00       	test   $0x400,%eax
  6b42b3:	74 50                	je     6b4305 <sqlite3VdbeExec+0x72a5>
  6b42b5:	4c 89 ff             	mov    %r15,%rdi
  6b42b8:	e8 03 c7 fe ff       	call   6a09c0 <sqlite3VdbeMemExpandBlob>
  6b42bd:	eb 2e                	jmp    6b42ed <sqlite3VdbeExec+0x728d>
  6b42bf:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b42c6:	00 
  6b42c7:	e9 70 d9 ff ff       	jmp    6b1c3c <sqlite3VdbeExec+0x4bdc>
  6b42cc:	41 8b 45 0c          	mov    0xc(%r13),%eax
  6b42d0:	e9 4e 15 00 00       	jmp    6b5823 <sqlite3VdbeExec+0x87c3>
  6b42d5:	41 8b 45 0c          	mov    0xc(%r13),%eax
  6b42d9:	e9 86 15 00 00       	jmp    6b5864 <sqlite3VdbeExec+0x8804>
  6b42de:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b42e3:	4c 89 ff             	mov    %r15,%rdi
  6b42e6:	31 d2                	xor    %edx,%edx
  6b42e8:	e8 a3 c7 fe ff       	call   6a0a90 <sqlite3VdbeMemStringify>
  6b42ed:	85 c0                	test   %eax,%eax
  6b42ef:	0f 85 4c 20 00 00    	jne    6b6341 <sqlite3VdbeExec+0x92e1>
  6b42f5:	41 0f b7 77 14       	movzwl 0x14(%r15),%esi
  6b42fa:	83 e6 fd             	and    $0xfffffffd,%esi
  6b42fd:	4c 8b 84 24 b8 00 00 	mov    0xb8(%rsp),%r8
  6b4304:	00 
  6b4305:	48 63 45 10          	movslq 0x10(%rbp),%rax
  6b4309:	4d 63 6f 10          	movslq 0x10(%r15),%r13
  6b430d:	49 01 c5             	add    %rax,%r13
  6b4310:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b4315:	44 3b a8 88 00 00 00 	cmp    0x88(%rax),%r13d
  6b431c:	0f 8f ec 1f 00 00    	jg     6b630e <sqlite3VdbeExec+0x92ae>
  6b4322:	89 b4 24 c8 00 00 00 	mov    %esi,0xc8(%rsp)
  6b4329:	41 8d 75 02          	lea    0x2(%r13),%esi
  6b432d:	31 d2                	xor    %edx,%edx
  6b432f:	4c 8b 74 24 50       	mov    0x50(%rsp),%r14
  6b4334:	45 39 f0             	cmp    %r14d,%r8d
  6b4337:	0f 94 c2             	sete   %dl
  6b433a:	4c 89 e7             	mov    %r12,%rdi
  6b433d:	4d 89 c7             	mov    %r8,%r15
  6b4340:	e8 6b c2 fe ff       	call   6a05b0 <sqlite3VdbeMemGrow>
  6b4345:	85 c0                	test   %eax,%eax
  6b4347:	0f 85 d3 1b 00 00    	jne    6b5f20 <sqlite3VdbeExec+0x8ec0>
  6b434d:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b4352:	41 23 44 24 14       	and    0x14(%r12),%eax
  6b4357:	83 c8 02             	or     $0x2,%eax
  6b435a:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b4360:	45 39 f7             	cmp    %r14d,%r15d
  6b4363:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b4368:	48 8b 44 24 40       	mov    0x40(%rsp),%rax
  6b436d:	74 27                	je     6b4396 <sqlite3VdbeExec+0x7336>
  6b436f:	49 8b 7c 24 08       	mov    0x8(%r12),%rdi
  6b4374:	48 8b 70 08          	mov    0x8(%rax),%rsi
  6b4378:	48 8b 44 24 40       	mov    0x40(%rsp),%rax
  6b437d:	48 63 50 10          	movslq 0x10(%rax),%rdx
  6b4381:	e8 2a 62 fa ff       	call   65a5b0 <memcpy$plt>
  6b4386:	48 8b 44 24 40       	mov    0x40(%rsp),%rax
  6b438b:	8b 8c 24 c8 00 00 00 	mov    0xc8(%rsp),%ecx
  6b4392:	66 89 48 14          	mov    %cx,0x14(%rax)
  6b4396:	48 63 78 10          	movslq 0x10(%rax),%rdi
  6b439a:	49 03 7c 24 08       	add    0x8(%r12),%rdi
  6b439f:	48 8b 75 08          	mov    0x8(%rbp),%rsi
  6b43a3:	48 63 55 10          	movslq 0x10(%rbp),%rdx
  6b43a7:	e8 04 62 fa ff       	call   65a5b0 <memcpy$plt>
  6b43ac:	4c 89 e8             	mov    %r13,%rax
  6b43af:	48 83 e0 fe          	and    $0xfffffffffffffffe,%rax
  6b43b3:	0f b6 54 24 1f       	movzbl 0x1f(%rsp),%edx
  6b43b8:	80 fa 02             	cmp    $0x2,%dl
  6b43bb:	49 0f 42 c5          	cmovb  %r13,%rax
  6b43bf:	8b 4c 24 38          	mov    0x38(%rsp),%ecx
  6b43c3:	66 89 4d 14          	mov    %cx,0x14(%rbp)
  6b43c7:	49 8b 4c 24 08       	mov    0x8(%r12),%rcx
  6b43cc:	c6 04 01 00          	movb   $0x0,(%rcx,%rax,1)
  6b43d0:	49 8b 4c 24 08       	mov    0x8(%r12),%rcx
  6b43d5:	c6 44 01 01 00       	movb   $0x0,0x1(%rcx,%rax,1)
  6b43da:	41 80 4c 24 15 02    	orb    $0x2,0x15(%r12)
  6b43e0:	41 89 44 24 10       	mov    %eax,0x10(%r12)
  6b43e5:	41 88 54 24 16       	mov    %dl,0x16(%r12)
  6b43ea:	e9 b7 15 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b43ef:	48 8d 35 8d 8a f9 ff 	lea    -0x67573(%rip),%rsi        # 64ce83 <.L.str.754>
  6b43f6:	48 8d 15 36 d2 f9 ff 	lea    -0x62dca(%rip),%rdx        # 651633 <.L.str.755>
  6b43fd:	4c 8d 05 83 a3 f9 ff 	lea    -0x65c7d(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b4404:	b8 0b 00 00 00       	mov    $0xb,%eax
  6b4409:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b440e:	bf 0b 00 00 00       	mov    $0xb,%edi
  6b4413:	b9 c0 26 01 00       	mov    $0x126c0,%ecx
  6b4418:	31 c0                	xor    %eax,%eax
  6b441a:	e8 61 ab fc ff       	call   67ef80 <sqlite3_log>
  6b441f:	49 83 c5 18          	add    $0x18,%r13
  6b4423:	e9 f8 8e ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b4428:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b442f:	00 00 
  6b4431:	41 80 3c 24 03       	cmpb   $0x3,(%r12)
  6b4436:	0f 85 17 04 00 00    	jne    6b4853 <sqlite3VdbeExec+0x77f3>
  6b443c:	49 63 4c 24 1c       	movslq 0x1c(%r12),%rcx
  6b4441:	48 85 c9             	test   %rcx,%rcx
  6b4444:	0f 8e 09 04 00 00    	jle    6b4853 <sqlite3VdbeExec+0x77f3>
  6b444a:	48 6b c9 38          	imul   $0x38,%rcx,%rcx
  6b444e:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
  6b4453:	8b 54 0e 10          	mov    0x10(%rsi,%rcx,1),%edx
  6b4457:	41 89 54 24 64       	mov    %edx,0x64(%r12)
  6b445c:	41 89 54 24 60       	mov    %edx,0x60(%r12)
  6b4461:	48 8b 7c 0e 08       	mov    0x8(%rsi,%rcx,1),%rdi
  6b4466:	49 89 7c 24 58       	mov    %rdi,0x58(%r12)
  6b446b:	e9 f6 01 00 00       	jmp    6b4666 <sqlite3VdbeExec+0x7606>
  6b4470:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b4477:	00 00 
  6b4479:	41 0f b7 44 24 42    	movzwl 0x42(%r12),%eax
  6b447f:	0f b7 c8             	movzwl %ax,%ecx
  6b4482:	39 cd                	cmp    %ecx,%ebp
  6b4484:	0f 83 cc 05 00 00    	jae    6b4a56 <sqlite3VdbeExec+0x79f6>
  6b448a:	89 e8                	mov    %ebp,%eax
  6b448c:	45 8b 4c 84 70       	mov    0x70(%r12,%rax,4),%r9d
  6b4491:	e9 44 11 00 00       	jmp    6b55da <sqlite3VdbeExec+0x857a>
  6b4496:	4c 89 e7             	mov    %r12,%rdi
  6b4499:	e8 92 b9 fc ff       	call   67fe30 <sqlite3VdbeMemMakeWriteable>
  6b449e:	85 c0                	test   %eax,%eax
  6b44a0:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b44a5:	0f 84 6c 8e ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b44ab:	e9 54 1e 00 00       	jmp    6b6304 <sqlite3VdbeExec+0x92a4>
  6b44b0:	89 f1                	mov    %esi,%ecx
  6b44b2:	81 e1 fc ff ff 7f    	and    $0x7ffffffc,%ecx
  6b44b8:	48 6b f9 38          	imul   $0x38,%rcx,%rdi
  6b44bc:	4c 01 c7             	add    %r8,%rdi
  6b44bf:	41 89 f1             	mov    %esi,%r9d
  6b44c2:	41 c1 e9 02          	shr    $0x2,%r9d
  6b44c6:	41 81 e1 ff ff ff 1f 	and    $0x1fffffff,%r9d
  6b44cd:	49 c1 e1 05          	shl    $0x5,%r9
  6b44d1:	45 31 d2             	xor    %r10d,%r10d
  6b44d4:	66 49 0f 6e c0       	movq   %r8,%xmm0
  6b44d9:	66 0f 70 c0 44       	pshufd $0x44,%xmm0,%xmm0
  6b44de:	66 0f 6f 0d 7a 6e f9 	movdqa -0x69186(%rip),%xmm1        # 64b360 <.LCPI830_5>
  6b44e5:	ff 
  6b44e6:	66 0f d4 c8          	paddq  %xmm0,%xmm1
  6b44ea:	66 0f d4 05 7e 6b f9 	paddq  -0x69482(%rip),%xmm0        # 64b070 <.LCPI830_6>
  6b44f1:	ff 
  6b44f2:	f3 42 0f 7f 0c 12    	movdqu %xmm1,(%rdx,%r10,1)
  6b44f8:	f3 42 0f 7f 44 12 10 	movdqu %xmm0,0x10(%rdx,%r10,1)
  6b44ff:	49 81 c0 e0 00 00 00 	add    $0xe0,%r8
  6b4506:	49 83 c2 20          	add    $0x20,%r10
  6b450a:	4d 39 d1             	cmp    %r10,%r9
  6b450d:	75 c5                	jne    6b44d4 <sqlite3VdbeExec+0x7474>
  6b450f:	39 f1                	cmp    %esi,%ecx
  6b4511:	74 10                	je     6b4523 <sqlite3VdbeExec+0x74c3>
  6b4513:	48 89 3c ca          	mov    %rdi,(%rdx,%rcx,8)
  6b4517:	48 83 c7 38          	add    $0x38,%rdi
  6b451b:	48 ff c1             	inc    %rcx
  6b451e:	48 39 ce             	cmp    %rcx,%rsi
  6b4521:	75 f0                	jne    6b4513 <sqlite3VdbeExec+0x74b3>
  6b4523:	41 0f b6 4d 02       	movzbl 0x2(%r13),%ecx
  6b4528:	4d 89 ee             	mov    %r13,%r14
  6b452b:	4c 8b 6c 24 10       	mov    0x10(%rsp),%r13
  6b4530:	41 88 4d 6c          	mov    %cl,0x6c(%r13)
  6b4534:	48 8d 4c 24 60       	lea    0x60(%rsp),%rcx
  6b4539:	4c 89 e7             	mov    %r12,%rdi
  6b453c:	ff d0                	call   *%rax
  6b453e:	89 c5                	mov    %eax,%ebp
  6b4540:	0f b6 44 24 20       	movzbl 0x20(%rsp),%eax
  6b4545:	41 88 45 6c          	mov    %al,0x6c(%r13)
  6b4549:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
  6b454e:	4c 89 e6             	mov    %r12,%rsi
  6b4551:	e8 ba 55 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6b4556:	85 ed                	test   %ebp,%ebp
  6b4558:	74 38                	je     6b4592 <sqlite3VdbeExec+0x7532>
  6b455a:	40 80 fd 13          	cmp    $0x13,%bpl
  6b455e:	4d 89 f5             	mov    %r14,%r13
  6b4561:	0f 85 24 1a 00 00    	jne    6b5f8b <sqlite3VdbeExec+0x8f2b>
  6b4567:	49 8b 45 10          	mov    0x10(%r13),%rax
  6b456b:	80 78 1c 00          	cmpb   $0x0,0x1c(%rax)
  6b456f:	0f 84 16 1a 00 00    	je     6b5f8b <sqlite3VdbeExec+0x8f2b>
  6b4575:	41 0f b7 45 02       	movzwl 0x2(%r13),%eax
  6b457a:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b4581:	00 00 
  6b4583:	83 f8 04             	cmp    $0x4,%eax
  6b4586:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b458b:	74 2f                	je     6b45bc <sqlite3VdbeExec+0x755c>
  6b458d:	e9 1a 1a 00 00       	jmp    6b5fac <sqlite3VdbeExec+0x8f4c>
  6b4592:	41 83 7e 04 00       	cmpl   $0x0,0x4(%r14)
  6b4597:	4d 89 f5             	mov    %r14,%r13
  6b459a:	74 0e                	je     6b45aa <sqlite3VdbeExec+0x754a>
  6b459c:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
  6b45a1:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
  6b45a6:	48 89 41 38          	mov    %rax,0x38(%rcx)
  6b45aa:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b45af:	49 ff 46 38          	incq   0x38(%r14)
  6b45b3:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b45ba:	00 00 
  6b45bc:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b45c1:	49 83 c5 18          	add    $0x18,%r13
  6b45c5:	e9 56 8d ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b45ca:	31 d2                	xor    %edx,%edx
  6b45cc:	f7 f1                	div    %ecx
  6b45ce:	89 d0                	mov    %edx,%eax
  6b45d0:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b45d4:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6b45d9:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
  6b45de:	49 89 04 24          	mov    %rax,(%r12)
  6b45e2:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b45e7:	41 23 44 24 14       	and    0x14(%r12),%eax
  6b45ec:	83 c8 04             	or     $0x4,%eax
  6b45ef:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b45f5:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b45fa:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b45ff:	49 83 c5 18          	add    $0x18,%r13
  6b4603:	e9 18 8d ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b4608:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b460f:	00 00 
  6b4611:	4c 89 ef             	mov    %r13,%rdi
  6b4614:	48 89 6c 24 38       	mov    %rbp,0x38(%rsp)
  6b4619:	4c 89 c5             	mov    %r8,%rbp
  6b461c:	e8 af fe fd ff       	call   6944d0 <getCellInfo>
  6b4621:	49 89 e8             	mov    %rbp,%r8
  6b4624:	48 8b 6c 24 38       	mov    0x38(%rsp),%rbp
  6b4629:	41 8b 45 40          	mov    0x40(%r13),%eax
  6b462d:	41 89 44 24 60       	mov    %eax,0x60(%r12)
  6b4632:	49 8b 7d 38          	mov    0x38(%r13),%rdi
  6b4636:	49 8b 85 88 00 00 00 	mov    0x88(%r13),%rax
  6b463d:	8b 40 58             	mov    0x58(%rax),%eax
  6b4640:	29 f8                	sub    %edi,%eax
  6b4642:	31 c9                	xor    %ecx,%ecx
  6b4644:	85 c0                	test   %eax,%eax
  6b4646:	0f 4f c8             	cmovg  %eax,%ecx
  6b4649:	41 0f b7 55 44       	movzwl 0x44(%r13),%edx
  6b464e:	39 c2                	cmp    %eax,%edx
  6b4650:	0f 4e ca             	cmovle %edx,%ecx
  6b4653:	41 89 4c 24 64       	mov    %ecx,0x64(%r12)
  6b4658:	49 89 7c 24 58       	mov    %rdi,0x58(%r12)
  6b465d:	41 8b 46 2c          	mov    0x2c(%r14),%eax
  6b4661:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b4666:	41 89 44 24 18       	mov    %eax,0x18(%r12)
  6b466b:	0f b6 07             	movzbl (%rdi),%eax
  6b466e:	41 89 00             	mov    %eax,(%r8)
  6b4671:	b9 01 00 00 00       	mov    $0x1,%ecx
  6b4676:	84 c0                	test   %al,%al
  6b4678:	79 1b                	jns    6b4695 <sqlite3VdbeExec+0x7635>
  6b467a:	4c 89 c6             	mov    %r8,%rsi
  6b467d:	48 89 6c 24 38       	mov    %rbp,0x38(%rsp)
  6b4682:	4c 89 c5             	mov    %r8,%rbp
  6b4685:	e8 f6 dd fc ff       	call   682480 <sqlite3GetVarint32>
  6b468a:	49 89 e8             	mov    %rbp,%r8
  6b468d:	48 8b 6c 24 38       	mov    0x38(%rsp),%rbp
  6b4692:	0f b6 c8             	movzbl %al,%ecx
  6b4695:	41 89 4c 24 38       	mov    %ecx,0x38(%r12)
  6b469a:	66 41 c7 44 24 42 00 	movw   $0x0,0x42(%r12)
  6b46a1:	00 
  6b46a2:	41 8b 10             	mov    (%r8),%edx
  6b46a5:	41 39 54 24 64       	cmp    %edx,0x64(%r12)
  6b46aa:	0f 83 d2 01 00 00    	jae    6b4882 <sqlite3VdbeExec+0x7822>
  6b46b0:	49 c7 44 24 58 00 00 	movq   $0x0,0x58(%r12)
  6b46b7:	00 00 
  6b46b9:	41 c7 44 24 64 00 00 	movl   $0x0,0x64(%r12)
  6b46c0:	00 00 
  6b46c2:	41 8b 00             	mov    (%r8),%eax
  6b46c5:	3d 03 80 01 00       	cmp    $0x18003,%eax
  6b46ca:	77 0b                	ja     6b46d7 <sqlite3VdbeExec+0x7677>
  6b46cc:	41 3b 44 24 60       	cmp    0x60(%r12),%eax
  6b46d1:	0f 86 30 11 00 00    	jbe    6b5807 <sqlite3VdbeExec+0x87a7>
  6b46d7:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b46dc:	e9 d1 11 00 00       	jmp    6b58b2 <sqlite3VdbeExec+0x8852>
  6b46e1:	41 f6 44 24 14 01    	testb  $0x1,0x14(%r12)
  6b46e7:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b46ec:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b46f0:	75 07                	jne    6b46f9 <sqlite3VdbeExec+0x7699>
  6b46f2:	41 f6 45 14 01       	testb  $0x1,0x14(%r13)
  6b46f7:	74 02                	je     6b46fb <sqlite3VdbeExec+0x769b>
  6b46f9:	f7 d8                	neg    %eax
  6b46fb:	89 c1                	mov    %eax,%ecx
  6b46fd:	f7 d9                	neg    %ecx
  6b46ff:	41 f6 c6 01          	test   $0x1,%r14b
  6b4703:	0f 44 c8             	cmove  %eax,%ecx
  6b4706:	89 8c 24 b0 00 00 00 	mov    %ecx,0xb0(%rsp)
  6b470d:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b4712:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b4717:	49 83 c5 18          	add    $0x18,%r13
  6b471b:	e9 00 8c ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b4720:	48 89 c1             	mov    %rax,%rcx
  6b4723:	48 8b 87 c0 01 00 00 	mov    0x1c0(%rdi),%rax
  6b472a:	49 89 04 24          	mov    %rax,(%r12)
  6b472e:	48 89 c8             	mov    %rcx,%rax
  6b4731:	4c 89 a7 c0 01 00 00 	mov    %r12,0x1c0(%rdi)
  6b4738:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b473d:	e9 ea a3 ff ff       	jmp    6aeb2c <sqlite3VdbeExec+0x1acc>
  6b4742:	48 8d 0d 79 65 b9 ff 	lea    -0x469a87(%rip),%rcx        # 24acc2 <sqlite3VdbeExec.aMask>
  6b4749:	0f b6 04 08          	movzbl (%rax,%rcx,1),%eax
  6b474d:	41 0f b7 4d 02       	movzwl 0x2(%r13),%ecx
  6b4752:	85 c8                	test   %ecx,%eax
  6b4754:	0f 85 12 11 00 00    	jne    6b586c <sqlite3VdbeExec+0x880c>
  6b475a:	e9 b8 8b ff ff       	jmp    6ad317 <sqlite3VdbeExec+0x2b7>
  6b475f:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b4763:	e9 8c 05 00 00       	jmp    6b4cf4 <sqlite3VdbeExec+0x7c94>
  6b4768:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b476e:	e9 f9 10 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b4773:	31 ed                	xor    %ebp,%ebp
  6b4775:	89 c8                	mov    %ecx,%eax
  6b4777:	41 ff 86 18 01 00 00 	incl   0x118(%r14)
  6b477e:	49 8b 96 08 01 00 00 	mov    0x108(%r14),%rdx
  6b4785:	49 89 54 24 08       	mov    %rdx,0x8(%r12)
  6b478a:	48 8b 74 24 10       	mov    0x10(%rsp),%rsi
  6b478f:	48 8b 56 38          	mov    0x38(%rsi),%rdx
  6b4793:	49 89 54 24 38       	mov    %rdx,0x38(%r12)
  6b4798:	49 8b 56 38          	mov    0x38(%r14),%rdx
  6b479c:	49 89 54 24 60       	mov    %rdx,0x60(%r12)
  6b47a1:	49 8b 16             	mov    (%r14),%rdx
  6b47a4:	48 8b 52 78          	mov    0x78(%rdx),%rdx
  6b47a8:	49 89 54 24 68       	mov    %rdx,0x68(%r12)
  6b47ad:	49 8b 96 28 01 00 00 	mov    0x128(%r14),%rdx
  6b47b4:	49 89 54 24 40       	mov    %rdx,0x40(%r12)
  6b47b9:	49 c7 86 28 01 00 00 	movq   $0x0,0x128(%r14)
  6b47c0:	00 00 00 00 
  6b47c4:	49 c7 46 38 00 00 00 	movq   $0x0,0x38(%r14)
  6b47cb:	00 
  6b47cc:	4d 89 a6 08 01 00 00 	mov    %r12,0x108(%r14)
  6b47d3:	49 8d 54 24 70       	lea    0x70(%r12),%rdx
  6b47d8:	48 89 54 24 28       	mov    %rdx,0x28(%rsp)
  6b47dd:	49 89 56 68          	mov    %rdx,0x68(%r14)
  6b47e1:	41 89 6e 24          	mov    %ebp,0x24(%r14)
  6b47e5:	0f b7 c0             	movzwl %ax,%eax
  6b47e8:	41 89 46 28          	mov    %eax,0x28(%r14)
  6b47ec:	48 63 c5             	movslq %ebp,%rax
  6b47ef:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b47f3:	4c 01 e0             	add    %r12,%rax
  6b47f6:	48 83 c0 70          	add    $0x70,%rax
  6b47fa:	49 89 46 78          	mov    %rax,0x78(%r14)
  6b47fe:	48 63 c9             	movslq %ecx,%rcx
  6b4801:	48 8d 3c c8          	lea    (%rax,%rcx,8),%rdi
  6b4805:	49 89 7c 24 28       	mov    %rdi,0x28(%r12)
  6b480a:	49 89 f4             	mov    %rsi,%r12
  6b480d:	41 8b 41 08          	mov    0x8(%r9),%eax
  6b4811:	8d 48 07             	lea    0x7(%rax),%ecx
  6b4814:	83 c0 0e             	add    $0xe,%eax
  6b4817:	85 c9                	test   %ecx,%ecx
  6b4819:	0f 49 c1             	cmovns %ecx,%eax
  6b481c:	c1 f8 03             	sar    $0x3,%eax
  6b481f:	48 63 d0             	movslq %eax,%rdx
  6b4822:	31 f6                	xor    %esi,%esi
  6b4824:	4d 89 cf             	mov    %r9,%r15
  6b4827:	e8 24 5d fa ff       	call   65a550 <memset$plt>
  6b482c:	49 8b 0f             	mov    (%r15),%rcx
  6b482f:	49 89 8e 88 00 00 00 	mov    %rcx,0x88(%r14)
  6b4836:	41 8b 47 08          	mov    0x8(%r15),%eax
  6b483a:	41 89 86 90 00 00 00 	mov    %eax,0x90(%r14)
  6b4841:	48 89 4c 24 58       	mov    %rcx,0x58(%rsp)
  6b4846:	4c 8d 69 e8          	lea    -0x18(%rcx),%r13
  6b484a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b484e:	e9 34 c6 ff ff       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6b4853:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b4857:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b485b:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b4860:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6b4864:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6b4869:	0f 84 ea 0c 00 00    	je     6b5559 <sqlite3VdbeExec+0x84f9>
  6b486f:	e8 cc cc fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b4874:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b4879:	49 83 c5 18          	add    $0x18,%r13
  6b487d:	e9 9e 8a ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b4882:	48 89 6c 24 38       	mov    %rbp,0x38(%rsp)
  6b4887:	49 8b 74 24 58       	mov    0x58(%r12),%rsi
  6b488c:	31 c0                	xor    %eax,%eax
  6b488e:	e9 46 02 00 00       	jmp    6b4ad9 <sqlite3VdbeExec+0x7a79>
  6b4893:	41 80 7d 00 02       	cmpb   $0x2,0x0(%r13)
  6b4898:	49 8b 7d 28          	mov    0x28(%r13),%rdi
  6b489c:	4d 89 fe             	mov    %r15,%r14
  6b489f:	0f 85 e3 0b 00 00    	jne    6b5488 <sqlite3VdbeExec+0x8428>
  6b48a5:	4c 8b 3f             	mov    (%rdi),%r15
  6b48a8:	49 8b 07             	mov    (%r15),%rax
  6b48ab:	4c 8d 6c 24 60       	lea    0x60(%rsp),%r13
  6b48b0:	4c 89 ee             	mov    %r13,%rsi
  6b48b3:	ff 50 60             	call   *0x60(%rax)
  6b48b6:	89 c5                	mov    %eax,%ebp
  6b48b8:	4c 89 f7             	mov    %r14,%rdi
  6b48bb:	4c 89 fe             	mov    %r15,%rsi
  6b48be:	e8 4d 52 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6b48c3:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b48ca:	00 00 
  6b48cc:	85 ed                	test   %ebp,%ebp
  6b48ce:	0f 84 ca 10 00 00    	je     6b599e <sqlite3VdbeExec+0x893e>
  6b48d4:	e9 e8 1a 00 00       	jmp    6b63c1 <sqlite3VdbeExec+0x9361>
  6b48d9:	45 31 ed             	xor    %r13d,%r13d
  6b48dc:	4c 8b 7c 24 10       	mov    0x10(%rsp),%r15
  6b48e1:	41 8b 97 e8 02 00 00 	mov    0x2e8(%r15),%edx
  6b48e8:	41 03 97 ec 02 00 00 	add    0x2ec(%r15),%edx
  6b48ef:	4c 89 ff             	mov    %r15,%rdi
  6b48f2:	31 f6                	xor    %esi,%esi
  6b48f4:	e8 87 20 00 00       	call   6b6980 <sqlite3VtabSavepoint>
  6b48f9:	85 c0                	test   %eax,%eax
  6b48fb:	0f 85 20 19 00 00    	jne    6b6221 <sqlite3VdbeExec+0x91c1>
  6b4901:	41 8d 75 21          	lea    0x21(%r13),%esi
  6b4905:	4c 89 ff             	mov    %r15,%rdi
  6b4908:	e8 43 5f fc ff       	call   67a850 <sqlite3DbMallocRawNN>
  6b490d:	48 85 c0             	test   %rax,%rax
  6b4910:	0f 84 c6 05 00 00    	je     6b4edc <sqlite3VdbeExec+0x7e7c>
  6b4916:	49 89 c7             	mov    %rax,%r15
  6b4919:	48 89 c7             	mov    %rax,%rdi
  6b491c:	48 83 c7 20          	add    $0x20,%rdi
  6b4920:	48 89 38             	mov    %rdi,(%rax)
  6b4923:	41 ff c5             	inc    %r13d
  6b4926:	4c 89 e6             	mov    %r12,%rsi
  6b4929:	4c 89 ea             	mov    %r13,%rdx
  6b492c:	e8 7f 5c fa ff       	call   65a5b0 <memcpy$plt>
  6b4931:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
  6b4936:	80 79 65 00          	cmpb   $0x0,0x65(%rcx)
  6b493a:	0f 84 76 05 00 00    	je     6b4eb6 <sqlite3VdbeExec+0x7e56>
  6b4940:	c6 41 65 00          	movb   $0x0,0x65(%rcx)
  6b4944:	c6 41 6d 01          	movb   $0x1,0x6d(%rcx)
  6b4948:	e9 6f 05 00 00       	jmp    6b4ebc <sqlite3VdbeExec+0x7e5c>
  6b494d:	49 8b b4 24 f8 00 00 	mov    0xf8(%r12),%rsi
  6b4954:	00 
  6b4955:	bf 01 00 00 00       	mov    $0x1,%edi
  6b495a:	4c 89 f2             	mov    %r14,%rdx
  6b495d:	41 ff 94 24 f0 00 00 	call   *0xf0(%r12)
  6b4964:	00 
  6b4965:	41 8b 45 04          	mov    0x4(%r13),%eax
  6b4969:	3b 05 61 82 0d 00    	cmp    0xd8261(%rip),%eax        # 78cbd0 <sqlite3Config+0x1a8>
  6b496f:	7c 52                	jl     6b49c3 <sqlite3VdbeExec+0x7963>
  6b4971:	41 80 7d 00 b8       	cmpb   $0xb8,0x0(%r13)
  6b4976:	0f 84 9b 89 ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b497c:	49 63 8e 90 00 00 00 	movslq 0x90(%r14),%rcx
  6b4983:	48 83 f9 02          	cmp    $0x2,%rcx
  6b4987:	7c 30                	jl     6b49b9 <sqlite3VdbeExec+0x7959>
  6b4989:	89 ce                	mov    %ecx,%esi
  6b498b:	49 8b 86 88 00 00 00 	mov    0x88(%r14),%rax
  6b4992:	48 ff c9             	dec    %rcx
  6b4995:	ba 01 00 00 00       	mov    $0x1,%edx
  6b499a:	83 fe 02             	cmp    $0x2,%esi
  6b499d:	75 72                	jne    6b4a11 <sqlite3VdbeExec+0x79b1>
  6b499f:	f6 c1 01             	test   $0x1,%cl
  6b49a2:	74 15                	je     6b49b9 <sqlite3VdbeExec+0x7959>
  6b49a4:	48 8d 0c 52          	lea    (%rdx,%rdx,2),%rcx
  6b49a8:	80 3c c8 0f          	cmpb   $0xf,(%rax,%rcx,8)
  6b49ac:	75 0b                	jne    6b49b9 <sqlite3VdbeExec+0x7959>
  6b49ae:	48 8d 04 c8          	lea    (%rax,%rcx,8),%rax
  6b49b2:	c7 40 04 00 00 00 00 	movl   $0x0,0x4(%rax)
  6b49b9:	41 c7 45 04 00 00 00 	movl   $0x0,0x4(%r13)
  6b49c0:	00 
  6b49c1:	31 c0                	xor    %eax,%eax
  6b49c3:	ff c0                	inc    %eax
  6b49c5:	41 89 45 04          	mov    %eax,0x4(%r13)
  6b49c9:	41 ff 86 ec 00 00 00 	incl   0xec(%r14)
  6b49d0:	e9 97 0e 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b49d5:	66 41 c7 47 14 04 00 	movw   $0x4,0x14(%r15)
  6b49dc:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b49e1:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
  6b49e6:	49 89 07             	mov    %rax,(%r15)
  6b49e9:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b49f0:	00 00 
  6b49f2:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b49f6:	49 83 c5 18          	add    $0x18,%r13
  6b49fa:	e9 21 89 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b49ff:	66 0f 57 d2          	xorpd  %xmm2,%xmm2
  6b4a03:	e9 61 ca ff ff       	jmp    6b1469 <sqlite3VdbeExec+0x4409>
  6b4a08:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b4a0c:	e9 7a ca ff ff       	jmp    6b148b <sqlite3VdbeExec+0x442b>
  6b4a11:	48 8d 70 34          	lea    0x34(%rax),%rsi
  6b4a15:	48 89 cf             	mov    %rcx,%rdi
  6b4a18:	48 83 e7 fe          	and    $0xfffffffffffffffe,%rdi
  6b4a1c:	48 f7 df             	neg    %rdi
  6b4a1f:	eb 1a                	jmp    6b4a3b <sqlite3VdbeExec+0x79db>
  6b4a21:	48 83 c6 30          	add    $0x30,%rsi
  6b4a25:	4c 8d 04 17          	lea    (%rdi,%rdx,1),%r8
  6b4a29:	49 83 c0 02          	add    $0x2,%r8
  6b4a2d:	48 83 c2 02          	add    $0x2,%rdx
  6b4a31:	49 83 f8 01          	cmp    $0x1,%r8
  6b4a35:	0f 84 64 ff ff ff    	je     6b499f <sqlite3VdbeExec+0x793f>
  6b4a3b:	80 7e e4 0f          	cmpb   $0xf,-0x1c(%rsi)
  6b4a3f:	75 07                	jne    6b4a48 <sqlite3VdbeExec+0x79e8>
  6b4a41:	c7 46 e8 00 00 00 00 	movl   $0x0,-0x18(%rsi)
  6b4a48:	80 7e fc 0f          	cmpb   $0xf,-0x4(%rsi)
  6b4a4c:	75 d3                	jne    6b4a21 <sqlite3VdbeExec+0x79c1>
  6b4a4e:	c7 06 00 00 00 00    	movl   $0x0,(%rsi)
  6b4a54:	eb cb                	jmp    6b4a21 <sqlite3VdbeExec+0x79c1>
  6b4a56:	41 8b 4c 24 38       	mov    0x38(%r12),%ecx
  6b4a5b:	41 8b 10             	mov    (%r8),%edx
  6b4a5e:	45 31 c9             	xor    %r9d,%r9d
  6b4a61:	39 d1                	cmp    %edx,%ecx
  6b4a63:	0f 83 66 0b 00 00    	jae    6b55cf <sqlite3VdbeExec+0x856f>
  6b4a69:	49 8b 74 24 58       	mov    0x58(%r12),%rsi
  6b4a6e:	48 85 f6             	test   %rsi,%rsi
  6b4a71:	48 89 6c 24 38       	mov    %rbp,0x38(%rsp)
  6b4a76:	75 61                	jne    6b4ad9 <sqlite3VdbeExec+0x7a79>
  6b4a78:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b4a7c:	66 0f 29 84 24 80 00 	movapd %xmm0,0x80(%rsp)
  6b4a83:	00 00 
  6b4a85:	66 0f 29 44 24 70    	movapd %xmm0,0x70(%rsp)
  6b4a8b:	66 0f 29 44 24 60    	movapd %xmm0,0x60(%rsp)
  6b4a91:	48 c7 84 24 90 00 00 	movq   $0x0,0x90(%rsp)
  6b4a98:	00 00 00 00 00 
  6b4a9d:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
  6b4aa2:	41 8b 30             	mov    (%r8),%esi
  6b4aa5:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
  6b4aaa:	4d 89 c7             	mov    %r8,%r15
  6b4aad:	e8 6e cf fe ff       	call   6a1a20 <sqlite3VdbeMemFromBtreeZeroOffset>
  6b4ab2:	85 c0                	test   %eax,%eax
  6b4ab4:	0f 85 42 15 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6b4aba:	48 8b 74 24 68       	mov    0x68(%rsp),%rsi
  6b4abf:	41 0f b7 44 24 42    	movzwl 0x42(%r12),%eax
  6b4ac5:	41 8b 4c 24 38       	mov    0x38(%r12),%ecx
  6b4aca:	41 8b 17             	mov    (%r15),%edx
  6b4acd:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b4ad4:	00 00 
  6b4ad6:	4d 89 f8             	mov    %r15,%r8
  6b4ad9:	0f b7 e8             	movzwl %ax,%ebp
  6b4adc:	45 8b 3c a8          	mov    (%r8,%rbp,4),%r15d
  6b4ae0:	41 89 cd             	mov    %ecx,%r13d
  6b4ae3:	49 01 f5             	add    %rsi,%r13
  6b4ae6:	89 d7                	mov    %edx,%edi
  6b4ae8:	48 89 b4 24 c8 00 00 	mov    %rsi,0xc8(%rsp)
  6b4aef:	00 
  6b4af0:	48 01 f7             	add    %rsi,%rdi
  6b4af3:	8b 74 24 38          	mov    0x38(%rsp),%esi
  6b4af7:	4c 89 44 24 40       	mov    %r8,0x40(%rsp)
  6b4afc:	48 89 7c 24 50       	mov    %rdi,0x50(%rsp)
  6b4b01:	48 89 b4 24 b8 00 00 	mov    %rsi,0xb8(%rsp)
  6b4b08:	00 
  6b4b09:	45 0f b6 4d 00       	movzbl 0x0(%r13),%r9d
  6b4b0e:	45 89 4c ac 70       	mov    %r9d,0x70(%r12,%rbp,4)
  6b4b13:	45 84 c9             	test   %r9b,%r9b
  6b4b16:	78 16                	js     6b4b2e <sqlite3VdbeExec+0x7ace>
  6b4b18:	49 ff c5             	inc    %r13
  6b4b1b:	44 89 c8             	mov    %r9d,%eax
  6b4b1e:	48 8d 0d cb 60 b9 ff 	lea    -0x469f35(%rip),%rcx        # 24abf0 <sqlite3SmallTypeSizes>
  6b4b25:	0f b6 04 08          	movzbl (%rax,%rcx,1),%eax
  6b4b29:	e9 a1 00 00 00       	jmp    6b4bcf <sqlite3VdbeExec+0x7b6f>
  6b4b2e:	41 0f b6 4d 01       	movzbl 0x1(%r13),%ecx
  6b4b33:	84 c9                	test   %cl,%cl
  6b4b35:	78 14                	js     6b4b4b <sqlite3VdbeExec+0x7aeb>
  6b4b37:	45 0f b6 4d 00       	movzbl 0x0(%r13),%r9d
  6b4b3c:	41 83 e1 7f          	and    $0x7f,%r9d
  6b4b40:	41 c1 e1 07          	shl    $0x7,%r9d
  6b4b44:	41 09 c9             	or     %ecx,%r9d
  6b4b47:	b0 02                	mov    $0x2,%al
  6b4b49:	eb 58                	jmp    6b4ba3 <sqlite3VdbeExec+0x7b43>
  6b4b4b:	41 0f be 45 02       	movsbl 0x2(%r13),%eax
  6b4b50:	85 c0                	test   %eax,%eax
  6b4b52:	78 1c                	js     6b4b70 <sqlite3VdbeExec+0x7b10>
  6b4b54:	41 0f b6 55 00       	movzbl 0x0(%r13),%edx
  6b4b59:	83 e2 7f             	and    $0x7f,%edx
  6b4b5c:	c1 e2 0e             	shl    $0xe,%edx
  6b4b5f:	83 e1 7f             	and    $0x7f,%ecx
  6b4b62:	c1 e1 07             	shl    $0x7,%ecx
  6b4b65:	09 d1                	or     %edx,%ecx
  6b4b67:	09 c1                	or     %eax,%ecx
  6b4b69:	b0 03                	mov    $0x3,%al
  6b4b6b:	41 89 c9             	mov    %ecx,%r9d
  6b4b6e:	eb 33                	jmp    6b4ba3 <sqlite3VdbeExec+0x7b43>
  6b4b70:	48 8d b4 24 e8 00 00 	lea    0xe8(%rsp),%rsi
  6b4b77:	00 
  6b4b78:	4c 89 ef             	mov    %r13,%rdi
  6b4b7b:	e8 a0 d7 fc ff       	call   682320 <sqlite3GetVarint>
  6b4b80:	4c 8b 8c 24 e8 00 00 	mov    0xe8(%rsp),%r9
  6b4b87:	00 
  6b4b88:	4c 89 c9             	mov    %r9,%rcx
  6b4b8b:	48 c1 e9 20          	shr    $0x20,%rcx
  6b4b8f:	75 5d                	jne    6b4bee <sqlite3VdbeExec+0x7b8e>
  6b4b91:	4c 8b 44 24 40       	mov    0x40(%rsp),%r8
  6b4b96:	48 8b 7c 24 50       	mov    0x50(%rsp),%rdi
  6b4b9b:	48 8b b4 24 b8 00 00 	mov    0xb8(%rsp),%rsi
  6b4ba2:	00 
  6b4ba3:	0f b6 c0             	movzbl %al,%eax
  6b4ba6:	49 01 c5             	add    %rax,%r13
  6b4ba9:	45 89 4c ac 70       	mov    %r9d,0x70(%r12,%rbp,4)
  6b4bae:	41 81 f9 80 00 00 00 	cmp    $0x80,%r9d
  6b4bb5:	72 08                	jb     6b4bbf <sqlite3VdbeExec+0x7b5f>
  6b4bb7:	41 8d 41 f4          	lea    -0xc(%r9),%eax
  6b4bbb:	d1 e8                	shr    $1,%eax
  6b4bbd:	eb 0e                	jmp    6b4bcd <sqlite3VdbeExec+0x7b6d>
  6b4bbf:	44 89 c8             	mov    %r9d,%eax
  6b4bc2:	48 8d 0d 27 60 b9 ff 	lea    -0x469fd9(%rip),%rcx        # 24abf0 <sqlite3SmallTypeSizes>
  6b4bc9:	0f b6 04 08          	movzbl (%rax,%rcx,1),%eax
  6b4bcd:	89 c0                	mov    %eax,%eax
  6b4bcf:	49 01 c7             	add    %rax,%r15
  6b4bd2:	48 8d 45 01          	lea    0x1(%rbp),%rax
  6b4bd6:	45 89 7c a8 04       	mov    %r15d,0x4(%r8,%rbp,4)
  6b4bdb:	48 39 f5             	cmp    %rsi,%rbp
  6b4bde:	73 37                	jae    6b4c17 <sqlite3VdbeExec+0x7bb7>
  6b4be0:	48 89 c5             	mov    %rax,%rbp
  6b4be3:	49 39 fd             	cmp    %rdi,%r13
  6b4be6:	0f 82 1d ff ff ff    	jb     6b4b09 <sqlite3VdbeExec+0x7aa9>
  6b4bec:	eb 29                	jmp    6b4c17 <sqlite3VdbeExec+0x7bb7>
  6b4bee:	0f b6 c0             	movzbl %al,%eax
  6b4bf1:	49 01 c5             	add    %rax,%r13
  6b4bf4:	41 c7 44 ac 70 ff ff 	movl   $0xffffffff,0x70(%r12,%rbp,4)
  6b4bfb:	ff ff 
  6b4bfd:	41 b9 ff ff ff ff    	mov    $0xffffffff,%r9d
  6b4c03:	4c 8b 44 24 40       	mov    0x40(%rsp),%r8
  6b4c08:	48 8b 7c 24 50       	mov    0x50(%rsp),%rdi
  6b4c0d:	48 8b b4 24 b8 00 00 	mov    0xb8(%rsp),%rsi
  6b4c14:	00 
  6b4c15:	eb a0                	jmp    6b4bb7 <sqlite3VdbeExec+0x7b57>
  6b4c17:	49 39 fd             	cmp    %rdi,%r13
  6b4c1a:	73 57                	jae    6b4c73 <sqlite3VdbeExec+0x7c13>
  6b4c1c:	41 8b 4c 24 60       	mov    0x60(%r12),%ecx
  6b4c21:	49 39 cf             	cmp    %rcx,%r15
  6b4c24:	48 8b 14 24          	mov    (%rsp),%rdx
  6b4c28:	48 8b 6c 24 38       	mov    0x38(%rsp),%rbp
  6b4c2d:	77 5d                	ja     6b4c8c <sqlite3VdbeExec+0x7c2c>
  6b4c2f:	e9 3e 09 00 00       	jmp    6b5572 <sqlite3VdbeExec+0x8512>
  6b4c34:	48 89 37             	mov    %rsi,(%rdi)
  6b4c37:	66 c7 47 14 04 00    	movw   $0x4,0x14(%rdi)
  6b4c3d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b4c42:	e9 40 c2 ff ff       	jmp    6b0e87 <sqlite3VdbeExec+0x3e27>
  6b4c47:	31 c0                	xor    %eax,%eax
  6b4c49:	e9 b5 93 ff ff       	jmp    6ae003 <sqlite3VdbeExec+0xfa3>
  6b4c4e:	31 c0                	xor    %eax,%eax
  6b4c50:	e9 cb b0 ff ff       	jmp    6afd20 <sqlite3VdbeExec+0x2cc0>
  6b4c55:	31 c0                	xor    %eax,%eax
  6b4c57:	e9 00 b1 ff ff       	jmp    6afd5c <sqlite3VdbeExec+0x2cfc>
  6b4c5c:	4c 89 e0             	mov    %r12,%rax
  6b4c5f:	48 d3 e8             	shr    %cl,%rax
  6b4c62:	4d 85 e4             	test   %r12,%r12
  6b4c65:	0f 88 6d 0b 00 00    	js     6b57d8 <sqlite3VdbeExec+0x8778>
  6b4c6b:	49 89 c4             	mov    %rax,%r12
  6b4c6e:	e9 74 0b 00 00       	jmp    6b57e7 <sqlite3VdbeExec+0x8787>
  6b4c73:	48 8b 14 24          	mov    (%rsp),%rdx
  6b4c77:	48 8b 6c 24 38       	mov    0x38(%rsp),%rbp
  6b4c7c:	77 0e                	ja     6b4c8c <sqlite3VdbeExec+0x7c2c>
  6b4c7e:	41 8b 4c 24 60       	mov    0x60(%r12),%ecx
  6b4c83:	49 39 cf             	cmp    %rcx,%r15
  6b4c86:	0f 84 e6 08 00 00    	je     6b5572 <sqlite3VdbeExec+0x8512>
  6b4c8c:	41 83 38 00          	cmpl   $0x0,(%r8)
  6b4c90:	0f 84 d7 08 00 00    	je     6b556d <sqlite3VdbeExec+0x850d>
  6b4c96:	49 83 7c 24 58 00    	cmpq   $0x0,0x58(%r12)
  6b4c9c:	0f 84 e8 0b 00 00    	je     6b588a <sqlite3VdbeExec+0x882a>
  6b4ca2:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b4ca7:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b4cac:	49 89 d7             	mov    %rdx,%r15
  6b4caf:	e9 fe 0b 00 00       	jmp    6b58b2 <sqlite3VdbeExec+0x8852>
  6b4cb4:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6b4cb8:	66 0f 57 c9          	xorpd  %xmm1,%xmm1
  6b4cbc:	66 0f 2e c1          	ucomisd %xmm1,%xmm0
  6b4cc0:	0f 85 51 86 ff ff    	jne    6ad317 <sqlite3VdbeExec+0x2b7>
  6b4cc6:	0f 8a 4b 86 ff ff    	jp     6ad317 <sqlite3VdbeExec+0x2b7>
  6b4ccc:	e9 9b 0b 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b4cd1:	66 0f 2e 05 6f 69 f9 	ucomisd -0x69691(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6b4cd8:	ff 
  6b4cd9:	0f 87 24 93 ff ff    	ja     6ae003 <sqlite3VdbeExec+0xfa3>
  6b4cdf:	f2 48 0f 2c c0       	cvttsd2si %xmm0,%rax
  6b4ce4:	e9 1a 93 ff ff       	jmp    6ae003 <sqlite3VdbeExec+0xfa3>
  6b4ce9:	c7 84 24 e8 00 00 00 	movl   $0x0,0xe8(%rsp)
  6b4cf0:	00 00 00 00 
  6b4cf4:	49 89 ff             	mov    %rdi,%r15
  6b4cf7:	e8 d4 f7 fd ff       	call   6944d0 <getCellInfo>
  6b4cfc:	49 8b 47 30          	mov    0x30(%r15),%rax
  6b4d00:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6b4d05:	48 b9 ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rcx
  6b4d0c:	ff ff 7f 
  6b4d0f:	48 39 c8             	cmp    %rcx,%rax
  6b4d12:	75 06                	jne    6b4d1a <sqlite3VdbeExec+0x7cba>
  6b4d14:	80 4d 05 02          	orb    $0x2,0x5(%rbp)
  6b4d18:	eb 08                	jmp    6b4d22 <sqlite3VdbeExec+0x7cc2>
  6b4d1a:	48 ff c0             	inc    %rax
  6b4d1d:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
  6b4d22:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b4d29:	00 00 
  6b4d2b:	49 63 46 0c          	movslq 0xc(%r14),%rax
  6b4d2f:	48 85 c0             	test   %rax,%rax
  6b4d32:	4d 89 ee             	mov    %r13,%r14
  6b4d35:	0f 84 79 00 00 00    	je     6b4db4 <sqlite3VdbeExec+0x7d54>
  6b4d3b:	49 8b 8e 08 01 00 00 	mov    0x108(%r14),%rcx
  6b4d42:	4c 8b 74 24 28       	mov    0x28(%rsp),%r14
  6b4d47:	48 85 c9             	test   %rcx,%rcx
  6b4d4a:	74 14                	je     6b4d60 <sqlite3VdbeExec+0x7d00>
  6b4d4c:	0f 1f 40 00          	nopl   0x0(%rax)
  6b4d50:	48 89 ca             	mov    %rcx,%rdx
  6b4d53:	48 8b 49 08          	mov    0x8(%rcx),%rcx
  6b4d57:	48 85 c9             	test   %rcx,%rcx
  6b4d5a:	75 f4                	jne    6b4d50 <sqlite3VdbeExec+0x7cf0>
  6b4d5c:	4c 8b 72 18          	mov    0x18(%rdx),%r14
  6b4d60:	4c 6b f8 38          	imul   $0x38,%rax,%r15
  6b4d64:	4f 8d 2c 3e          	lea    (%r14,%r15,1),%r13
  6b4d68:	4c 89 ef             	mov    %r13,%rdi
  6b4d6b:	e8 10 c3 fe ff       	call   6a1080 <sqlite3VdbeMemIntegerify>
  6b4d70:	4b 8b 0c 3e          	mov    (%r14,%r15,1),%rcx
  6b4d74:	b8 0d 00 00 00       	mov    $0xd,%eax
  6b4d79:	48 ba ff ff ff ff ff 	movabs $0x7fffffffffffffff,%rdx
  6b4d80:	ff ff 7f 
  6b4d83:	48 39 d1             	cmp    %rdx,%rcx
  6b4d86:	0f 84 97 12 00 00    	je     6b6023 <sqlite3VdbeExec+0x8fc3>
  6b4d8c:	f6 45 05 02          	testb  $0x2,0x5(%rbp)
  6b4d90:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b4d95:	0f 85 34 14 00 00    	jne    6b61cf <sqlite3VdbeExec+0x916f>
  6b4d9b:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
  6b4da0:	48 39 c8             	cmp    %rcx,%rax
  6b4da3:	7f 0b                	jg     6b4db0 <sqlite3VdbeExec+0x7d50>
  6b4da5:	48 ff c1             	inc    %rcx
  6b4da8:	48 89 4c 24 60       	mov    %rcx,0x60(%rsp)
  6b4dad:	48 89 c8             	mov    %rcx,%rax
  6b4db0:	49 89 45 00          	mov    %rax,0x0(%r13)
  6b4db4:	f6 45 05 02          	testb  $0x2,0x5(%rbp)
  6b4db8:	75 0a                	jne    6b4dc4 <sqlite3VdbeExec+0x7d64>
  6b4dba:	4c 8b 6c 24 60       	mov    0x60(%rsp),%r13
  6b4dbf:	e9 82 00 00 00       	jmp    6b4e46 <sqlite3VdbeExec+0x7de6>
  6b4dc4:	45 31 f6             	xor    %r14d,%r14d
  6b4dc7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
  6b4dce:	00 00 
  6b4dd0:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
  6b4dd5:	bf 08 00 00 00       	mov    $0x8,%edi
  6b4dda:	e8 d1 a3 fc ff       	call   67f1b0 <sqlite3_randomness>
  6b4ddf:	49 bd ff ff ff ff ff 	movabs $0x3fffffffffffffff,%r13
  6b4de6:	ff ff 3f 
  6b4de9:	4c 23 6c 24 60       	and    0x60(%rsp),%r13
  6b4dee:	49 ff c5             	inc    %r13
  6b4df1:	4c 89 6c 24 60       	mov    %r13,0x60(%rsp)
  6b4df6:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b4dfa:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b4e01:	00 00 
  6b4e03:	48 8d 8c 24 e8 00 00 	lea    0xe8(%rsp),%rcx
  6b4e0a:	00 
  6b4e0b:	4c 89 ee             	mov    %r13,%rsi
  6b4e0e:	31 d2                	xor    %edx,%edx
  6b4e10:	e8 5b 01 fe ff       	call   694f70 <sqlite3BtreeTableMoveto>
  6b4e15:	8b 8c 24 e8 00 00 00 	mov    0xe8(%rsp),%ecx
  6b4e1c:	85 c0                	test   %eax,%eax
  6b4e1e:	75 11                	jne    6b4e31 <sqlite3VdbeExec+0x7dd1>
  6b4e20:	85 c9                	test   %ecx,%ecx
  6b4e22:	75 0d                	jne    6b4e31 <sqlite3VdbeExec+0x7dd1>
  6b4e24:	41 83 fe 63          	cmp    $0x63,%r14d
  6b4e28:	41 8d 56 01          	lea    0x1(%r14),%edx
  6b4e2c:	41 89 d6             	mov    %edx,%r14d
  6b4e2f:	72 9f                	jb     6b4dd0 <sqlite3VdbeExec+0x7d70>
  6b4e31:	85 c0                	test   %eax,%eax
  6b4e33:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b4e38:	0f 85 77 13 00 00    	jne    6b61b5 <sqlite3VdbeExec+0x9155>
  6b4e3e:	85 c9                	test   %ecx,%ecx
  6b4e40:	0f 84 6f 13 00 00    	je     6b61b5 <sqlite3VdbeExec+0x9155>
  6b4e46:	c6 45 03 00          	movb   $0x0,0x3(%rbp)
  6b4e4a:	c7 45 18 00 00 00 00 	movl   $0x0,0x18(%rbp)
  6b4e51:	4d 89 2c 24          	mov    %r13,(%r12)
  6b4e55:	e9 4c 0b 00 00       	jmp    6b59a6 <sqlite3VdbeExec+0x8946>
  6b4e5a:	66 0f 2e 05 e6 67 f9 	ucomisd -0x6981a(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6b4e61:	ff 
  6b4e62:	0f 87 b8 ae ff ff    	ja     6afd20 <sqlite3VdbeExec+0x2cc0>
  6b4e68:	f2 48 0f 2c c0       	cvttsd2si %xmm0,%rax
  6b4e6d:	e9 ae ae ff ff       	jmp    6afd20 <sqlite3VdbeExec+0x2cc0>
  6b4e72:	66 0f 2e 05 ce 67 f9 	ucomisd -0x69832(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6b4e79:	ff 
  6b4e7a:	0f 87 dc ae ff ff    	ja     6afd5c <sqlite3VdbeExec+0x2cfc>
  6b4e80:	f2 48 0f 2c c0       	cvttsd2si %xmm0,%rax
  6b4e85:	e9 d2 ae ff ff       	jmp    6afd5c <sqlite3VdbeExec+0x2cfc>
  6b4e8a:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6b4e8e:	e9 7f 09 00 00       	jmp    6b5812 <sqlite3VdbeExec+0x87b2>
  6b4e93:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6b4e97:	e9 b7 09 00 00       	jmp    6b5853 <sqlite3VdbeExec+0x87f3>
  6b4e9c:	31 d2                	xor    %edx,%edx
  6b4e9e:	f7 f6                	div    %esi
  6b4ea0:	e9 2f f7 ff ff       	jmp    6b45d4 <sqlite3VdbeExec+0x7574>
  6b4ea5:	31 d2                	xor    %edx,%edx
  6b4ea7:	f7 f1                	div    %ecx
  6b4ea9:	0f 57 d2             	xorps  %xmm2,%xmm2
  6b4eac:	f2 48 0f 2a d2       	cvtsi2sd %rdx,%xmm2
  6b4eb1:	e9 03 e6 ff ff       	jmp    6b34b9 <sqlite3VdbeExec+0x6459>
  6b4eb6:	ff 81 e8 02 00 00    	incl   0x2e8(%rcx)
  6b4ebc:	48 8b 81 d8 02 00 00 	mov    0x2d8(%rcx),%rax
  6b4ec3:	49 89 47 18          	mov    %rax,0x18(%r15)
  6b4ec7:	4c 89 b9 d8 02 00 00 	mov    %r15,0x2d8(%rcx)
  6b4ece:	66 0f 10 81 f0 02 00 	movupd 0x2f0(%rcx),%xmm0
  6b4ed5:	00 
  6b4ed6:	66 41 0f 11 47 08    	movupd %xmm0,0x8(%r15)
  6b4edc:	41 80 be c7 00 00 00 	cmpb   $0x3,0xc7(%r14)
  6b4ee3:	03 
  6b4ee4:	0f 84 89 10 00 00    	je     6b5f73 <sqlite3VdbeExec+0x8f13>
  6b4eea:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b4ef1:	00 00 
  6b4ef3:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b4ef8:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b4efd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b4f01:	49 83 c5 18          	add    $0x18,%r13
  6b4f05:	e9 16 84 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b4f0a:	48 8b 7e 28          	mov    0x28(%rsi),%rdi
  6b4f0e:	48 8b 07             	mov    (%rdi),%rax
  6b4f11:	48 8b 08             	mov    (%rax),%rcx
  6b4f14:	ff 48 08             	decl   0x8(%rax)
  6b4f17:	ff 51 38             	call   *0x38(%rcx)
  6b4f1a:	e9 d1 e7 ff ff       	jmp    6b36f0 <sqlite3VdbeExec+0x6690>
  6b4f1f:	48 8b 7e 28          	mov    0x28(%rsi),%rdi
  6b4f23:	e8 28 f4 fd ff       	call   694350 <sqlite3BtreeCloseCursor>
  6b4f28:	e9 c3 e7 ff ff       	jmp    6b36f0 <sqlite3VdbeExec+0x6690>
  6b4f2d:	4c 89 ef             	mov    %r13,%rdi
  6b4f30:	4c 89 e6             	mov    %r12,%rsi
  6b4f33:	8b 54 24 40          	mov    0x40(%rsp),%edx
  6b4f37:	8b 4c 24 50          	mov    0x50(%rsp),%ecx
  6b4f3b:	f2 0f 11 4c 24 40    	movsd  %xmm1,0x40(%rsp)
  6b4f41:	e8 0a ce fc ff       	call   681d50 <sqlite3Atoi64>
  6b4f46:	f2 0f 10 4c 24 40    	movsd  0x40(%rsp),%xmm1
  6b4f4c:	85 c0                	test   %eax,%eax
  6b4f4e:	0f 84 a8 09 00 00    	je     6b58fc <sqlite3VdbeExec+0x889c>
  6b4f54:	f2 41 0f 11 0c 24    	movsd  %xmm1,(%r12)
  6b4f5a:	66 b8 08 00          	mov    $0x8,%ax
  6b4f5e:	41 8b 4c 24 14       	mov    0x14(%r12),%ecx
  6b4f63:	83 e1 fd             	and    $0xfffffffd,%ecx
  6b4f66:	0f b7 c0             	movzwl %ax,%eax
  6b4f69:	09 c8                	or     %ecx,%eax
  6b4f6b:	66 41 89 44 24 14    	mov    %ax,0x14(%r12)
  6b4f71:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b4f77:	8b 54 24 38          	mov    0x38(%rsp),%edx
  6b4f7b:	0f b7 c0             	movzwl %ax,%eax
  6b4f7e:	a8 24                	test   $0x24,%al
  6b4f80:	0f 84 ac 00 00 00    	je     6b5032 <sqlite3VdbeExec+0x7fd2>
  6b4f86:	4d 8b 2c 24          	mov    (%r12),%r13
  6b4f8a:	66 41 89 54 24 14    	mov    %dx,0x14(%r12)
  6b4f90:	a8 24                	test   $0x24,%al
  6b4f92:	0f 84 d2 00 00 00    	je     6b506a <sqlite3VdbeExec+0x800a>
  6b4f98:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b4f9c:	45 31 e4             	xor    %r12d,%r12d
  6b4f9f:	48 8d 8c 24 c4 00 00 	lea    0xc4(%rsp),%rcx
  6b4fa6:	00 
  6b4fa7:	4c 89 ee             	mov    %r13,%rsi
  6b4faa:	31 d2                	xor    %edx,%edx
  6b4fac:	e8 bf ff fd ff       	call   694f70 <sqlite3BtreeTableMoveto>
  6b4fb1:	4c 89 6d 48          	mov    %r13,0x48(%rbp)
  6b4fb5:	85 c0                	test   %eax,%eax
  6b4fb7:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b4fbc:	0f 85 3a 10 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6b4fc2:	8b 84 24 c4 00 00 00 	mov    0xc4(%rsp),%eax
  6b4fc9:	41 83 ff 17          	cmp    $0x17,%r15d
  6b4fcd:	72 1c                	jb     6b4feb <sqlite3VdbeExec+0x7f8b>
  6b4fcf:	85 c0                	test   %eax,%eax
  6b4fd1:	78 0d                	js     6b4fe0 <sqlite3VdbeExec+0x7f80>
  6b4fd3:	41 83 f7 18          	xor    $0x18,%r15d
  6b4fd7:	41 09 c7             	or     %eax,%r15d
  6b4fda:	0f 85 7b 01 00 00    	jne    6b515b <sqlite3VdbeExec+0x80fb>
  6b4fe0:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b4fe4:	e8 87 02 fe ff       	call   695270 <sqlite3BtreeNext>
  6b4fe9:	eb 2d                	jmp    6b5018 <sqlite3VdbeExec+0x7fb8>
  6b4feb:	85 c0                	test   %eax,%eax
  6b4fed:	7f 0d                	jg     6b4ffc <sqlite3VdbeExec+0x7f9c>
  6b4fef:	41 83 f7 15          	xor    $0x15,%r15d
  6b4ff3:	41 09 c7             	or     %eax,%r15d
  6b4ff6:	0f 85 6d 01 00 00    	jne    6b5169 <sqlite3VdbeExec+0x8109>
  6b4ffc:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b5000:	80 67 01 f1          	andb   $0xf1,0x1(%rdi)
  6b5004:	66 c7 47 46 00 00    	movw   $0x0,0x46(%rdi)
  6b500a:	80 3f 00             	cmpb   $0x0,(%rdi)
  6b500d:	0f 84 24 01 00 00    	je     6b5137 <sqlite3VdbeExec+0x80d7>
  6b5013:	e8 58 11 fe ff       	call   696170 <btreePrevious>
  6b5018:	85 c0                	test   %eax,%eax
  6b501a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b501e:	0f 84 65 01 00 00    	je     6b5189 <sqlite3VdbeExec+0x8129>
  6b5024:	83 f8 65             	cmp    $0x65,%eax
  6b5027:	0f 84 49 01 00 00    	je     6b5176 <sqlite3VdbeExec+0x8116>
  6b502d:	e9 00 12 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b5032:	a8 08                	test   $0x8,%al
  6b5034:	0f 85 86 00 00 00    	jne    6b50c0 <sqlite3VdbeExec+0x8060>
  6b503a:	a8 12                	test   $0x12,%al
  6b503c:	0f 84 1f 01 00 00    	je     6b5161 <sqlite3VdbeExec+0x8101>
  6b5042:	49 83 7c 24 08 00    	cmpq   $0x0,0x8(%r12)
  6b5048:	0f 84 13 01 00 00    	je     6b5161 <sqlite3VdbeExec+0x8101>
  6b504e:	4c 89 e7             	mov    %r12,%rdi
  6b5051:	41 89 d5             	mov    %edx,%r13d
  6b5054:	e8 f7 be fe ff       	call   6a0f50 <memIntValue>
  6b5059:	44 89 ea             	mov    %r13d,%edx
  6b505c:	49 89 c5             	mov    %rax,%r13
  6b505f:	41 0f b7 44 24 14    	movzwl 0x14(%r12),%eax
  6b5065:	e9 20 ff ff ff       	jmp    6b4f8a <sqlite3VdbeExec+0x7f2a>
  6b506a:	a8 08                	test   $0x8,%al
  6b506c:	75 7c                	jne    6b50ea <sqlite3VdbeExec+0x808a>
  6b506e:	41 80 ff 16          	cmp    $0x16,%r15b
  6b5072:	0f 87 33 01 00 00    	ja     6b51ab <sqlite3VdbeExec+0x814b>
  6b5078:	83 e0 01             	and    $0x1,%eax
  6b507b:	0f 85 2a 01 00 00    	jne    6b51ab <sqlite3VdbeExec+0x814b>
  6b5081:	48 8b 7d 28          	mov    0x28(%rbp),%rdi
  6b5085:	80 3f 00             	cmpb   $0x0,(%rdi)
  6b5088:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b508d:	75 0a                	jne    6b5099 <sqlite3VdbeExec+0x8039>
  6b508f:	f6 47 01 08          	testb  $0x8,0x1(%rdi)
  6b5093:	0f 85 0e 07 00 00    	jne    6b57a7 <sqlite3VdbeExec+0x8747>
  6b5099:	48 8d b4 24 c4 00 00 	lea    0xc4(%rsp),%rsi
  6b50a0:	00 
  6b50a1:	e8 4a fe fd ff       	call   694ef0 <btreeLast>
  6b50a6:	31 c9                	xor    %ecx,%ecx
  6b50a8:	85 c0                	test   %eax,%eax
  6b50aa:	0f 85 4c 0f 00 00    	jne    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6b50b0:	83 bc 24 c4 00 00 00 	cmpl   $0x0,0xc4(%rsp)
  6b50b7:	00 
  6b50b8:	41 89 cc             	mov    %ecx,%r12d
  6b50bb:	e9 b0 00 00 00       	jmp    6b5170 <sqlite3VdbeExec+0x8110>
  6b50c0:	49 bd ff ff ff ff ff 	movabs $0x7fffffffffffffff,%r13
  6b50c7:	ff ff 7f 
  6b50ca:	f2 41 0f 10 04 24    	movsd  (%r12),%xmm0
  6b50d0:	f2 0f 10 0d 58 64 f9 	movsd  -0x69ba8(%rip),%xmm1        # 64b530 <.LCPI1090_0>
  6b50d7:	ff 
  6b50d8:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b50dc:	0f 86 dc 00 00 00    	jbe    6b51be <sqlite3VdbeExec+0x815e>
  6b50e2:	49 ff c5             	inc    %r13
  6b50e5:	e9 a0 fe ff ff       	jmp    6b4f8a <sqlite3VdbeExec+0x7f2a>
  6b50ea:	f2 41 0f 10 04 24    	movsd  (%r12),%xmm0
  6b50f0:	66 0f 2e c0          	ucomisd %xmm0,%xmm0
  6b50f4:	0f 8a 4e 04 00 00    	jp     6b5548 <sqlite3VdbeExec+0x84e8>
  6b50fa:	f2 0f 10 0d 26 64 f9 	movsd  -0x69bda(%rip),%xmm1        # 64b528 <.LCPI710_0>
  6b5101:	ff 
  6b5102:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b5106:	0f 87 3c 04 00 00    	ja     6b5548 <sqlite3VdbeExec+0x84e8>
  6b510c:	b0 01                	mov    $0x1,%al
  6b510e:	66 0f 2e 05 aa 64 f9 	ucomisd -0x69b56(%rip),%xmm0        # 64b5c0 <.LCPI101_3>
  6b5115:	ff 
  6b5116:	73 0e                	jae    6b5126 <sqlite3VdbeExec+0x80c6>
  6b5118:	f2 48 0f 2c c8       	cvttsd2si %xmm0,%rcx
  6b511d:	49 39 cd             	cmp    %rcx,%r13
  6b5120:	0f 8d 1c 04 00 00    	jge    6b5542 <sqlite3VdbeExec+0x84e2>
  6b5126:	44 20 f8             	and    %r15b,%al
  6b5129:	0f b6 c0             	movzbl %al,%eax
  6b512c:	44 01 f8             	add    %r15d,%eax
  6b512f:	41 89 c7             	mov    %eax,%r15d
  6b5132:	e9 61 fe ff ff       	jmp    6b4f98 <sqlite3VdbeExec+0x7f38>
  6b5137:	0f b7 47 56          	movzwl 0x56(%rdi),%eax
  6b513b:	66 85 c0             	test   %ax,%ax
  6b513e:	0f 84 cf fe ff ff    	je     6b5013 <sqlite3VdbeExec+0x7fb3>
  6b5144:	48 8b 8f 88 00 00 00 	mov    0x88(%rdi),%rcx
  6b514b:	80 79 08 00          	cmpb   $0x0,0x8(%rcx)
  6b514f:	0f 84 be fe ff ff    	je     6b5013 <sqlite3VdbeExec+0x7fb3>
  6b5155:	ff c8                	dec    %eax
  6b5157:	66 89 47 56          	mov    %ax,0x56(%rdi)
  6b515b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b515f:	eb 28                	jmp    6b5189 <sqlite3VdbeExec+0x8129>
  6b5161:	45 31 ed             	xor    %r13d,%r13d
  6b5164:	e9 21 fe ff ff       	jmp    6b4f8a <sqlite3VdbeExec+0x7f2a>
  6b5169:	48 8b 45 28          	mov    0x28(%rbp),%rax
  6b516d:	80 38 00             	cmpb   $0x0,(%rax)
  6b5170:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5174:	74 13                	je     6b5189 <sqlite3VdbeExec+0x8129>
  6b5176:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b517d:	00 00 
  6b517f:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5184:	e9 e3 06 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b5189:	49 8d 45 18          	lea    0x18(%r13),%rax
  6b518d:	45 85 e4             	test   %r12d,%r12d
  6b5190:	4c 0f 45 e8          	cmovne %rax,%r13
  6b5194:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b519b:	00 00 
  6b519d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b51a2:	49 83 c5 18          	add    $0x18,%r13
  6b51a6:	e9 75 81 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b51ab:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b51b0:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b51b4:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b51b9:	e9 ae 06 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b51be:	66 0f 2e 05 82 64 f9 	ucomisd -0x69b7e(%rip),%xmm0        # 64b648 <.LCPI1090_1>
  6b51c5:	ff 
  6b51c6:	0f 87 be fd ff ff    	ja     6b4f8a <sqlite3VdbeExec+0x7f2a>
  6b51cc:	f2 4c 0f 2c e8       	cvttsd2si %xmm0,%r13
  6b51d1:	e9 b4 fd ff ff       	jmp    6b4f8a <sqlite3VdbeExec+0x7f2a>
  6b51d6:	4c 89 f9             	mov    %r15,%rcx
  6b51d9:	41 80 7d 08 00       	cmpb   $0x0,0x8(%r13)
  6b51de:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b51e3:	0f 84 1c 07 00 00    	je     6b5905 <sqlite3VdbeExec+0x88a5>
  6b51e9:	44 39 e5             	cmp    %r12d,%ebp
  6b51ec:	0f 85 a4 00 00 00    	jne    6b5296 <sqlite3VdbeExec+0x8236>
  6b51f2:	44 89 e5             	mov    %r12d,%ebp
  6b51f5:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b51fa:	49 89 c6             	mov    %rax,%r14
  6b51fd:	85 c0                	test   %eax,%eax
  6b51ff:	44 0f 44 e5          	cmove  %ebp,%r12d
  6b5203:	4c 89 ef             	mov    %r13,%rdi
  6b5206:	44 89 e6             	mov    %r12d,%esi
  6b5209:	e8 02 91 fd ff       	call   68e310 <sqlite3PagerSetJournalMode>
  6b520e:	48 8b 54 24 38       	mov    0x38(%rsp),%rdx
  6b5213:	66 c7 42 14 02 22    	movw   $0x2202,0x14(%rdx)
  6b5219:	83 f8 06             	cmp    $0x6,%eax
  6b521c:	75 0c                	jne    6b522a <sqlite3VdbeExec+0x81ca>
  6b521e:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx)
  6b5225:	00 
  6b5226:	31 c0                	xor    %eax,%eax
  6b5228:	eb 21                	jmp    6b524b <sqlite3VdbeExec+0x81eb>
  6b522a:	89 c0                	mov    %eax,%eax
  6b522c:	48 8d 0d 7d 39 0d 00 	lea    0xd397d(%rip),%rcx        # 788bb0 <sqlite3JournalModename.azModeName>
  6b5233:	48 8b 3c c1          	mov    (%rcx,%rax,8),%rdi
  6b5237:	48 89 7a 08          	mov    %rdi,0x8(%rdx)
  6b523b:	49 89 d4             	mov    %rdx,%r12
  6b523e:	e8 0d 51 fa ff       	call   65a350 <strlen$plt>
  6b5243:	4c 89 e2             	mov    %r12,%rdx
  6b5246:	25 ff ff ff 3f       	and    $0x3fffffff,%eax
  6b524b:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5250:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5254:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5259:	89 42 10             	mov    %eax,0x10(%rdx)
  6b525c:	c6 42 16 01          	movb   $0x1,0x16(%rdx)
  6b5260:	80 7c 24 1f 01       	cmpb   $0x1,0x1f(%rsp)
  6b5265:	74 0d                	je     6b5274 <sqlite3VdbeExec+0x8214>
  6b5267:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b526c:	48 89 d7             	mov    %rdx,%rdi
  6b526f:	e8 4c a5 fc ff       	call   67f7c0 <sqlite3VdbeMemTranslate>
  6b5274:	4c 89 f0             	mov    %r14,%rax
  6b5277:	85 c0                	test   %eax,%eax
  6b5279:	0f 85 80 0f 00 00    	jne    6b61ff <sqlite3VdbeExec+0x919f>
  6b527f:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b5286:	00 00 
  6b5288:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b528d:	49 83 c5 18          	add    $0x18,%r13
  6b5291:	e9 8a 80 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b5296:	41 83 fc 05          	cmp    $0x5,%r12d
  6b529a:	0f 94 c0             	sete   %al
  6b529d:	74 09                	je     6b52a8 <sqlite3VdbeExec+0x8248>
  6b529f:	83 fd 05             	cmp    $0x5,%ebp
  6b52a2:	0f 85 4d ff ff ff    	jne    6b51f5 <sqlite3VdbeExec+0x8195>
  6b52a8:	4c 8b 7c 24 10       	mov    0x10(%rsp),%r15
  6b52ad:	41 80 7f 65 00       	cmpb   $0x0,0x65(%r15)
  6b52b2:	0f 84 84 0f 00 00    	je     6b623c <sqlite3VdbeExec+0x91dc>
  6b52b8:	41 83 bf d4 00 00 00 	cmpl   $0x2,0xd4(%r15)
  6b52bf:	02 
  6b52c0:	0f 8d 76 0f 00 00    	jge    6b623c <sqlite3VdbeExec+0x91dc>
  6b52c6:	84 c0                	test   %al,%al
  6b52c8:	0f 84 ef 01 00 00    	je     6b54bd <sqlite3VdbeExec+0x845d>
  6b52ce:	49 89 cf             	mov    %rcx,%r15
  6b52d1:	4c 89 ef             	mov    %r13,%rdi
  6b52d4:	48 8b 74 24 10       	mov    0x10(%rsp),%rsi
  6b52d9:	e8 22 a5 fd ff       	call   68f800 <sqlite3PagerCloseWal>
  6b52de:	85 c0                	test   %eax,%eax
  6b52e0:	0f 85 14 ff ff ff    	jne    6b51fa <sqlite3VdbeExec+0x819a>
  6b52e6:	4c 89 ef             	mov    %r13,%rdi
  6b52e9:	89 ee                	mov    %ebp,%esi
  6b52eb:	e8 20 90 fd ff       	call   68e310 <sqlite3PagerSetJournalMode>
  6b52f0:	4c 89 f9             	mov    %r15,%rcx
  6b52f3:	e9 eb 01 00 00       	jmp    6b54e3 <sqlite3VdbeExec+0x8483>
  6b52f8:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b52fc:	66 0f 57 c9          	xorpd  %xmm1,%xmm1
  6b5300:	f2 0f c2 c8 04       	cmpneqsd %xmm0,%xmm1
  6b5305:	66 49 0f 7e cf       	movq   %xmm1,%r15
  6b530a:	41 83 e7 01          	and    $0x1,%r15d
  6b530e:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b5312:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b5316:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b531b:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
  6b531f:	0f b7 4c 01 14       	movzwl 0x14(%rcx,%rax,1),%ecx
  6b5324:	f6 c1 24             	test   $0x24,%cl
  6b5327:	74 0b                	je     6b5334 <sqlite3VdbeExec+0x82d4>
  6b5329:	31 c0                	xor    %eax,%eax
  6b532b:	48 83 3f 00          	cmpq   $0x0,(%rdi)
  6b532f:	0f 95 c0             	setne  %al
  6b5332:	eb 36                	jmp    6b536a <sqlite3VdbeExec+0x830a>
  6b5334:	b8 02 00 00 00       	mov    $0x2,%eax
  6b5339:	f6 c1 01             	test   $0x1,%cl
  6b533c:	75 2c                	jne    6b536a <sqlite3VdbeExec+0x830a>
  6b533e:	f6 c1 08             	test   $0x8,%cl
  6b5341:	75 0c                	jne    6b534f <sqlite3VdbeExec+0x82ef>
  6b5343:	f6 c1 12             	test   $0x12,%cl
  6b5346:	74 0d                	je     6b5355 <sqlite3VdbeExec+0x82f5>
  6b5348:	e8 53 bc fe ff       	call   6a0fa0 <memRealValue>
  6b534d:	eb 0a                	jmp    6b5359 <sqlite3VdbeExec+0x82f9>
  6b534f:	f2 0f 10 07          	movsd  (%rdi),%xmm0
  6b5353:	eb 04                	jmp    6b5359 <sqlite3VdbeExec+0x82f9>
  6b5355:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b5359:	66 0f 57 c9          	xorpd  %xmm1,%xmm1
  6b535d:	f2 0f c2 c8 04       	cmpneqsd %xmm0,%xmm1
  6b5362:	66 48 0f 7e c8       	movq   %xmm1,%rax
  6b5367:	83 e0 01             	and    $0x1,%eax
  6b536a:	4b 8d 0c 7f          	lea    (%r15,%r15,2),%rcx
  6b536e:	48 01 c1             	add    %rax,%rcx
  6b5371:	41 80 7d 00 2c       	cmpb   $0x2c,0x0(%r13)
  6b5376:	48 8d 05 33 59 b9 ff 	lea    -0x46a6cd(%rip),%rax        # 24acb0 <sqlite3VdbeExec.and_logic>
  6b537d:	48 8d 15 35 59 b9 ff 	lea    -0x46a6cb(%rip),%rdx        # 24acb9 <sqlite3VdbeExec.or_logic>
  6b5384:	48 0f 44 d0          	cmove  %rax,%rdx
  6b5388:	0f b6 0c 0a          	movzbl (%rdx,%rcx,1),%ecx
  6b538c:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b5390:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b5394:	48 03 44 24 28       	add    0x28(%rsp),%rax
  6b5399:	48 83 f9 02          	cmp    $0x2,%rcx
  6b539d:	75 0c                	jne    6b53ab <sqlite3VdbeExec+0x834b>
  6b539f:	b9 40 f2 ff ff       	mov    $0xfffff240,%ecx
  6b53a4:	23 48 14             	and    0x14(%rax),%ecx
  6b53a7:	ff c1                	inc    %ecx
  6b53a9:	eb 0e                	jmp    6b53b9 <sqlite3VdbeExec+0x8359>
  6b53ab:	48 89 08             	mov    %rcx,(%rax)
  6b53ae:	b9 40 f2 ff ff       	mov    $0xfffff240,%ecx
  6b53b3:	23 48 14             	and    0x14(%rax),%ecx
  6b53b6:	83 c9 04             	or     $0x4,%ecx
  6b53b9:	66 89 48 14          	mov    %cx,0x14(%rax)
  6b53bd:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b53c1:	49 83 c5 18          	add    $0x18,%r13
  6b53c5:	e9 56 7f ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b53ca:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b53d1:	00 
  6b53d2:	66 41 c7 44 24 14 01 	movw   $0x401,0x14(%r12)
  6b53d9:	04 
  6b53da:	41 c7 04 24 00 00 00 	movl   $0x0,(%r12)
  6b53e1:	00 
  6b53e2:	49 8b 7d 28          	mov    0x28(%r13),%rdi
  6b53e6:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b53eb:	41 8b 55 08          	mov    0x8(%r13),%edx
  6b53ef:	48 8d b4 24 e8 00 00 	lea    0xe8(%rsp),%rsi
  6b53f6:	00 
  6b53f7:	41 ff 56 58          	call   *0x58(%r14)
  6b53fb:	89 c5                	mov    %eax,%ebp
  6b53fd:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5402:	4c 89 f7             	mov    %r14,%rdi
  6b5405:	48 8b 74 24 20       	mov    0x20(%rsp),%rsi
  6b540a:	e8 01 47 ff ff       	call   6a9b10 <sqlite3VtabImportErrmsg>
  6b540f:	83 bc 24 0c 01 00 00 	cmpl   $0x0,0x10c(%rsp)
  6b5416:	00 
  6b5417:	7e 23                	jle    6b543c <sqlite3VdbeExec+0x83dc>
  6b5419:	4c 89 e7             	mov    %r12,%rdi
  6b541c:	e8 4f 4a ff ff       	call   6a9e70 <sqlite3_value_text>
  6b5421:	48 8d 35 1d 0c fa ff 	lea    -0x5f3e3(%rip),%rsi        # 656045 <.L.str.277>
  6b5428:	4c 89 f7             	mov    %r14,%rdi
  6b542b:	48 89 c2             	mov    %rax,%rdx
  6b542e:	31 c0                	xor    %eax,%eax
  6b5430:	e8 ab d6 fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5435:	8b ac 24 0c 01 00 00 	mov    0x10c(%rsp),%ebp
  6b543c:	41 f6 44 24 14 02    	testb  $0x2,0x14(%r12)
  6b5442:	75 0c                	jne    6b5450 <sqlite3VdbeExec+0x83f0>
  6b5444:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b5449:	41 88 44 24 16       	mov    %al,0x16(%r12)
  6b544e:	eb 19                	jmp    6b5469 <sqlite3VdbeExec+0x8409>
  6b5450:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b5455:	41 3a 44 24 16       	cmp    0x16(%r12),%al
  6b545a:	74 0d                	je     6b5469 <sqlite3VdbeExec+0x8409>
  6b545c:	0f b6 74 24 1f       	movzbl 0x1f(%rsp),%esi
  6b5461:	4c 89 e7             	mov    %r12,%rdi
  6b5464:	e8 57 a3 fc ff       	call   67f7c0 <sqlite3VdbeMemTranslate>
  6b5469:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b5470:	00 00 
  6b5472:	85 ed                	test   %ebp,%ebp
  6b5474:	0f 85 b6 0d 00 00    	jne    6b6230 <sqlite3VdbeExec+0x91d0>
  6b547a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b547f:	49 83 c5 18          	add    $0x18,%r13
  6b5483:	e9 98 7e ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b5488:	80 3f 00             	cmpb   $0x0,(%rdi)
  6b548b:	0f 84 f8 04 00 00    	je     6b5989 <sqlite3VdbeExec+0x8929>
  6b5491:	4c 89 ef             	mov    %r13,%rdi
  6b5494:	e8 47 2a ff ff       	call   6a7ee0 <sqlite3VdbeHandleMovedCursor>
  6b5499:	85 c0                	test   %eax,%eax
  6b549b:	0f 85 88 0e 00 00    	jne    6b6329 <sqlite3VdbeExec+0x92c9>
  6b54a1:	41 80 7d 02 00       	cmpb   $0x0,0x2(%r13)
  6b54a6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b54aa:	0f 84 d5 04 00 00    	je     6b5985 <sqlite3VdbeExec+0x8925>
  6b54b0:	66 41 c7 44 24 14 01 	movw   $0x1,0x14(%r12)
  6b54b7:	00 
  6b54b8:	e9 45 e5 ff ff       	jmp    6b3a02 <sqlite3VdbeExec+0x69a2>
  6b54bd:	41 83 fc 04          	cmp    $0x4,%r12d
  6b54c1:	75 13                	jne    6b54d6 <sqlite3VdbeExec+0x8476>
  6b54c3:	4c 89 ef             	mov    %r13,%rdi
  6b54c6:	be 02 00 00 00       	mov    $0x2,%esi
  6b54cb:	49 89 cf             	mov    %rcx,%r15
  6b54ce:	e8 3d 8e fd ff       	call   68e310 <sqlite3PagerSetJournalMode>
  6b54d3:	4c 89 f9             	mov    %r15,%rcx
  6b54d6:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b54db:	85 c0                	test   %eax,%eax
  6b54dd:	0f 85 17 fd ff ff    	jne    6b51fa <sqlite3VdbeExec+0x819a>
  6b54e3:	31 f6                	xor    %esi,%esi
  6b54e5:	83 fd 05             	cmp    $0x5,%ebp
  6b54e8:	40 0f 94 c6          	sete   %sil
  6b54ec:	ff c6                	inc    %esi
  6b54ee:	48 89 cf             	mov    %rcx,%rdi
  6b54f1:	e8 4a 96 fe ff       	call   69eb40 <sqlite3BtreeSetVersion>
  6b54f6:	e9 ff fc ff ff       	jmp    6b51fa <sqlite3VdbeExec+0x819a>
  6b54fb:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
  6b5500:	66 83 78 02 00       	cmpw   $0x0,0x2(%rax)
  6b5505:	4c 0f 45 f8          	cmovne %rax,%r15
  6b5509:	eb 05                	jmp    6b5510 <sqlite3VdbeExec+0x84b0>
  6b550b:	4c 8b 7c 24 08       	mov    0x8(%rsp),%r15
  6b5510:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b5517:	00 00 
  6b5519:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b551e:	4c 89 e8             	mov    %r13,%rax
  6b5521:	4d 89 fd             	mov    %r15,%r13
  6b5524:	49 89 c7             	mov    %rax,%r15
  6b5527:	e9 40 03 00 00       	jmp    6b586c <sqlite3VdbeExec+0x880c>
  6b552c:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b5533:	00 00 
  6b5535:	4d 89 ef             	mov    %r13,%r15
  6b5538:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b553d:	e9 6a 94 ff ff       	jmp    6ae9ac <sqlite3VdbeExec+0x194c>
  6b5542:	0f 8e 91 03 00 00    	jle    6b58d9 <sqlite3VdbeExec+0x8879>
  6b5548:	44 89 f8             	mov    %r15d,%eax
  6b554b:	83 e0 01             	and    $0x1,%eax
  6b554e:	41 01 c7             	add    %eax,%r15d
  6b5551:	41 ff cf             	dec    %r15d
  6b5554:	e9 3f fa ff ff       	jmp    6b4f98 <sqlite3VdbeExec+0x7f38>
  6b5559:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b555f:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5564:	49 83 c5 18          	add    $0x18,%r13
  6b5568:	e9 b3 7d ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b556d:	31 c0                	xor    %eax,%eax
  6b556f:	49 89 fd             	mov    %rdi,%r13
  6b5572:	66 41 89 44 24 42    	mov    %ax,0x42(%r12)
  6b5578:	44 2b ac 24 c8 00 00 	sub    0xc8(%rsp),%r13d
  6b557f:	00 
  6b5580:	45 89 6c 24 38       	mov    %r13d,0x38(%r12)
  6b5585:	49 83 7c 24 58 00    	cmpq   $0x0,0x58(%r12)
  6b558b:	74 0a                	je     6b5597 <sqlite3VdbeExec+0x8537>
  6b558d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5592:	49 89 d7             	mov    %rdx,%r15
  6b5595:	eb 38                	jmp    6b55cf <sqlite3VdbeExec+0x856f>
  6b5597:	f6 44 24 75 90       	testb  $0x90,0x75(%rsp)
  6b559c:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b55a1:	49 89 d7             	mov    %rdx,%r15
  6b55a4:	75 0a                	jne    6b55b0 <sqlite3VdbeExec+0x8550>
  6b55a6:	83 bc 24 80 00 00 00 	cmpl   $0x0,0x80(%rsp)
  6b55ad:	00 
  6b55ae:	74 1f                	je     6b55cf <sqlite3VdbeExec+0x856f>
  6b55b0:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b55b5:	4c 89 4c 24 50       	mov    %r9,0x50(%rsp)
  6b55ba:	e8 31 b8 fe ff       	call   6a0df0 <vdbeMemClear>
  6b55bf:	4c 8b 4c 24 50       	mov    0x50(%rsp),%r9
  6b55c4:	4c 8b 44 24 40       	mov    0x40(%rsp),%r8
  6b55c9:	41 0f b7 44 24 42    	movzwl 0x42(%r12),%eax
  6b55cf:	0f b7 c0             	movzwl %ax,%eax
  6b55d2:	39 c5                	cmp    %eax,%ebp
  6b55d4:	0f 83 fe 00 00 00    	jae    6b56d8 <sqlite3VdbeExec+0x8678>
  6b55da:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b55de:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b55e2:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b55e7:	4c 8d 14 01          	lea    (%rcx,%rax,1),%r10
  6b55eb:	f6 44 01 15 90       	testb  $0x90,0x15(%rcx,%rax,1)
  6b55f0:	74 28                	je     6b561a <sqlite3VdbeExec+0x85ba>
  6b55f2:	4c 89 d7             	mov    %r10,%rdi
  6b55f5:	48 89 6c 24 38       	mov    %rbp,0x38(%rsp)
  6b55fa:	4c 89 c5             	mov    %r8,%rbp
  6b55fd:	4d 89 cd             	mov    %r9,%r13
  6b5600:	4c 89 54 24 40       	mov    %r10,0x40(%rsp)
  6b5605:	e8 36 bf fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b560a:	4c 8b 54 24 40       	mov    0x40(%rsp),%r10
  6b560f:	4d 89 e9             	mov    %r13,%r9
  6b5612:	49 89 e8             	mov    %rbp,%r8
  6b5615:	48 8b 6c 24 38       	mov    0x38(%rsp),%rbp
  6b561a:	41 8b 4c 24 64       	mov    0x64(%r12),%ecx
  6b561f:	89 e8                	mov    %ebp,%eax
  6b5621:	41 3b 4c 80 04       	cmp    0x4(%r8,%rax,4),%ecx
  6b5626:	0f 83 98 00 00 00    	jae    6b56c4 <sqlite3VdbeExec+0x8664>
  6b562c:	0f b6 4c 24 1f       	movzbl 0x1f(%rsp),%ecx
  6b5631:	41 88 4a 16          	mov    %cl,0x16(%r10)
  6b5635:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
  6b563a:	0f b7 49 02          	movzwl 0x2(%rcx),%ecx
  6b563e:	81 e1 c0 00 00 00    	and    $0xc0,%ecx
  6b5644:	74 2b                	je     6b5671 <sqlite3VdbeExec+0x8611>
  6b5646:	81 f9 80 00 00 00    	cmp    $0x80,%ecx
  6b564c:	0f 84 ba 00 00 00    	je     6b570c <sqlite3VdbeExec+0x86ac>
  6b5652:	41 83 f9 0c          	cmp    $0xc,%r9d
  6b5656:	72 1f                	jb     6b5677 <sqlite3VdbeExec+0x8617>
  6b5658:	41 f6 c1 01          	test   $0x1,%r9b
  6b565c:	0f 84 aa 00 00 00    	je     6b570c <sqlite3VdbeExec+0x86ac>
  6b5662:	0f b7 c9             	movzwl %cx,%ecx
  6b5665:	81 f9 c0 00 00 00    	cmp    $0xc0,%ecx
  6b566b:	0f 84 9b 00 00 00    	je     6b570c <sqlite3VdbeExec+0x86ac>
  6b5671:	41 83 f9 7f          	cmp    $0x7f,%r9d
  6b5675:	77 14                	ja     6b568b <sqlite3VdbeExec+0x862b>
  6b5677:	44 89 c9             	mov    %r9d,%ecx
  6b567a:	48 8d 15 6f 55 b9 ff 	lea    -0x46aa91(%rip),%rdx        # 24abf0 <sqlite3SmallTypeSizes>
  6b5681:	80 3c 11 00          	cmpb   $0x0,(%rcx,%rdx,1)
  6b5685:	0f 84 81 00 00 00    	je     6b570c <sqlite3VdbeExec+0x86ac>
  6b568b:	41 8b 0c 80          	mov    (%r8,%rax,4),%ecx
  6b568f:	45 8b 46 2c          	mov    0x2c(%r14),%r8d
  6b5693:	48 83 ec 08          	sub    $0x8,%rsp
  6b5697:	4c 89 e7             	mov    %r12,%rdi
  6b569a:	89 ee                	mov    %ebp,%esi
  6b569c:	44 89 ca             	mov    %r9d,%edx
  6b569f:	4c 8b 8c 24 e0 00 00 	mov    0xe0(%rsp),%r9
  6b56a6:	00 
  6b56a7:	41 52                	push   %r10
  6b56a9:	e8 32 10 00 00       	call   6b66e0 <vdbeColumnFromOverflow>
  6b56ae:	48 83 c4 10          	add    $0x10,%rsp
  6b56b2:	85 c0                	test   %eax,%eax
  6b56b4:	0f 85 32 0c 00 00    	jne    6b62ec <sqlite3VdbeExec+0x928c>
  6b56ba:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
  6b56bf:	e9 ed ac ff ff       	jmp    6b03b1 <sqlite3VdbeExec+0x3351>
  6b56c4:	41 8b 0c 80          	mov    (%r8,%rax,4),%ecx
  6b56c8:	49 03 4c 24 58       	add    0x58(%r12),%rcx
  6b56cd:	41 83 f9 0b          	cmp    $0xb,%r9d
  6b56d1:	77 50                	ja     6b5723 <sqlite3VdbeExec+0x86c3>
  6b56d3:	48 89 cf             	mov    %rcx,%rdi
  6b56d6:	eb 3b                	jmp    6b5713 <sqlite3VdbeExec+0x86b3>
  6b56d8:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b56dc:	48 6b f8 38          	imul   $0x38,%rax,%rdi
  6b56e0:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6b56e5:	41 80 7d 01 f6       	cmpb   $0xf6,0x1(%r13)
  6b56ea:	0f 85 9a 00 00 00    	jne    6b578a <sqlite3VdbeExec+0x872a>
  6b56f0:	49 8b 75 10          	mov    0x10(%r13),%rsi
  6b56f4:	ba 00 20 00 00       	mov    $0x2000,%edx
  6b56f9:	e8 f2 c0 fe ff       	call   6a17f0 <sqlite3VdbeMemShallowCopy>
  6b56fe:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5703:	49 83 c5 18          	add    $0x18,%r13
  6b5707:	e9 14 7c ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b570c:	48 8d 3d 0d 52 b9 ff 	lea    -0x46adf3(%rip),%rdi        # 24a920 <sqlite3CtypeMap>
  6b5713:	44 89 ce             	mov    %r9d,%esi
  6b5716:	4c 89 d2             	mov    %r10,%rdx
  6b5719:	e8 52 28 ff ff       	call   6a7f70 <sqlite3VdbeSerialGet>
  6b571e:	e9 8e ac ff ff       	jmp    6b03b1 <sqlite3VdbeExec+0x3351>
  6b5723:	45 8d 61 f4          	lea    -0xc(%r9),%r12d
  6b5727:	41 d1 ec             	shr    $1,%r12d
  6b572a:	45 89 62 10          	mov    %r12d,0x10(%r10)
  6b572e:	0f b6 44 24 1f       	movzbl 0x1f(%rsp),%eax
  6b5733:	41 88 42 16          	mov    %al,0x16(%r10)
  6b5737:	41 8d 74 24 02       	lea    0x2(%r12),%esi
  6b573c:	41 39 72 20          	cmp    %esi,0x20(%r10)
  6b5740:	0f 8d eb 01 00 00    	jge    6b5931 <sqlite3VdbeExec+0x88d1>
  6b5746:	48 89 4c 24 38       	mov    %rcx,0x38(%rsp)
  6b574b:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
  6b5750:	44 3b a0 88 00 00 00 	cmp    0x88(%rax),%r12d
  6b5757:	0f 8f b1 0b 00 00    	jg     6b630e <sqlite3VdbeExec+0x92ae>
  6b575d:	4d 89 cd             	mov    %r9,%r13
  6b5760:	66 41 c7 42 14 01 00 	movw   $0x1,0x14(%r10)
  6b5767:	4c 89 d7             	mov    %r10,%rdi
  6b576a:	31 d2                	xor    %edx,%edx
  6b576c:	4c 89 d5             	mov    %r10,%rbp
  6b576f:	e8 3c ae fe ff       	call   6a05b0 <sqlite3VdbeMemGrow>
  6b5774:	85 c0                	test   %eax,%eax
  6b5776:	0f 85 52 02 00 00    	jne    6b59ce <sqlite3VdbeExec+0x896e>
  6b577c:	48 8b 7d 08          	mov    0x8(%rbp),%rdi
  6b5780:	48 8b 4c 24 38       	mov    0x38(%rsp),%rcx
  6b5785:	e9 b5 01 00 00       	jmp    6b593f <sqlite3VdbeExec+0x88df>
  6b578a:	f6 47 15 90          	testb  $0x90,0x15(%rdi)
  6b578e:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5793:	0f 84 dd 01 00 00    	je     6b5976 <sqlite3VdbeExec+0x8916>
  6b5799:	e8 a2 bd fe ff       	call   6a1540 <vdbeMemClearExternAndSetNull>
  6b579e:	49 83 c5 18          	add    $0x18,%r13
  6b57a2:	e9 79 7b ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b57a7:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b57ab:	e9 e4 f9 ff ff       	jmp    6b5194 <sqlite3VdbeExec+0x8134>
  6b57b0:	4c 89 44 24 40       	mov    %r8,0x40(%rsp)
  6b57b5:	4c 89 e7             	mov    %r12,%rdi
  6b57b8:	e8 b3 26 ff ff       	call   6a7e70 <sqlite3VdbeFinishMoveto>
  6b57bd:	85 c0                	test   %eax,%eax
  6b57bf:	0f 85 74 02 00 00    	jne    6b5a39 <sqlite3VdbeExec+0x89d9>
  6b57c5:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b57cc:	00 00 
  6b57ce:	4c 8b 44 24 40       	mov    0x40(%rsp),%r8
  6b57d3:	e9 39 ee ff ff       	jmp    6b4611 <sqlite3VdbeExec+0x75b1>
  6b57d8:	f6 d9                	neg    %cl
  6b57da:	49 c7 c4 ff ff ff ff 	mov    $0xffffffffffffffff,%r12
  6b57e1:	49 d3 e4             	shl    %cl,%r12
  6b57e4:	49 09 c4             	or     %rax,%r12
  6b57e7:	4c 89 27             	mov    %r12,(%rdi)
  6b57ea:	b8 40 f2 ff ff       	mov    $0xfffff240,%eax
  6b57ef:	23 47 14             	and    0x14(%rdi),%eax
  6b57f2:	83 c8 04             	or     $0x4,%eax
  6b57f5:	66 89 47 14          	mov    %ax,0x14(%rdi)
  6b57f9:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b57fe:	49 83 c5 18          	add    $0x18,%r13
  6b5802:	e9 19 7b ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b5807:	31 c0                	xor    %eax,%eax
  6b5809:	e9 71 ec ff ff       	jmp    6b447f <sqlite3VdbeExec+0x741f>
  6b580e:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b5812:	66 0f 57 c9          	xorpd  %xmm1,%xmm1
  6b5816:	f2 0f c2 c8 04       	cmpneqsd %xmm0,%xmm1
  6b581b:	66 48 0f 7e c8       	movq   %xmm1,%rax
  6b5820:	83 e0 01             	and    $0x1,%eax
  6b5823:	49 6b ff 38          	imul   $0x38,%r15,%rdi
  6b5827:	41 33 45 10          	xor    0x10(%r13),%eax
  6b582b:	48 03 7c 24 28       	add    0x28(%rsp),%rdi
  6b5830:	48 63 f0             	movslq %eax,%rsi
  6b5833:	f6 47 15 90          	testb  $0x90,0x15(%rdi)
  6b5837:	0f 84 14 b3 ff ff    	je     6b0b51 <sqlite3VdbeExec+0x3af1>
  6b583d:	e8 9e bd fe ff       	call   6a15e0 <vdbeReleaseAndSetInt64>
  6b5842:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5846:	49 83 c5 18          	add    $0x18,%r13
  6b584a:	e9 d1 7a ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b584f:	66 0f 57 c0          	xorpd  %xmm0,%xmm0
  6b5853:	66 0f 57 c9          	xorpd  %xmm1,%xmm1
  6b5857:	f2 0f c2 c8 04       	cmpneqsd %xmm0,%xmm1
  6b585c:	66 48 0f 7e c8       	movq   %xmm1,%rax
  6b5861:	83 e0 01             	and    $0x1,%eax
  6b5864:	85 c0                	test   %eax,%eax
  6b5866:	0f 84 ab 7a ff ff    	je     6ad317 <sqlite3VdbeExec+0x2b7>
  6b586c:	49 63 45 08          	movslq 0x8(%r13),%rax
  6b5870:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b5874:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b5879:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b587d:	49 83 c5 e8          	add    $0xffffffffffffffe8,%r13
  6b5881:	49 83 c5 18          	add    $0x18,%r13
  6b5885:	e9 96 7a ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b588a:	f6 44 24 75 90       	testb  $0x90,0x75(%rsp)
  6b588f:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5894:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5899:	49 89 d7             	mov    %rdx,%r15
  6b589c:	75 0a                	jne    6b58a8 <sqlite3VdbeExec+0x8848>
  6b589e:	83 bc 24 80 00 00 00 	cmpl   $0x0,0x80(%rsp)
  6b58a5:	00 
  6b58a6:	74 0a                	je     6b58b2 <sqlite3VdbeExec+0x8852>
  6b58a8:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
  6b58ad:	e8 3e b5 fe ff       	call   6a0df0 <vdbeMemClear>
  6b58b2:	48 8b 4c 24 58       	mov    0x58(%rsp),%rcx
  6b58b7:	48 63 41 0c          	movslq 0xc(%rcx),%rax
  6b58bb:	48 85 c0             	test   %rax,%rax
  6b58be:	0f 8e dc 09 00 00    	jle    6b62a0 <sqlite3VdbeExec+0x9240>
  6b58c4:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  6b58c8:	4c 8d 2c c1          	lea    (%rcx,%rax,8),%r13
  6b58cc:	49 83 c5 e8          	add    $0xffffffffffffffe8,%r13
  6b58d0:	49 83 c5 18          	add    $0x18,%r13
  6b58d4:	e9 47 7a ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b58d9:	0f 57 c9             	xorps  %xmm1,%xmm1
  6b58dc:	f2 49 0f 2a cd       	cvtsi2sd %r13,%xmm1
  6b58e1:	66 0f 2e c1          	ucomisd %xmm1,%xmm0
  6b58e5:	0f 87 3b f8 ff ff    	ja     6b5126 <sqlite3VdbeExec+0x80c6>
  6b58eb:	66 0f 2e c8          	ucomisd %xmm0,%xmm1
  6b58ef:	0f 87 53 fc ff ff    	ja     6b5548 <sqlite3VdbeExec+0x84e8>
  6b58f5:	31 c0                	xor    %eax,%eax
  6b58f7:	e9 2a f8 ff ff       	jmp    6b5126 <sqlite3VdbeExec+0x80c6>
  6b58fc:	66 b8 04 00          	mov    $0x4,%ax
  6b5900:	e9 59 f6 ff ff       	jmp    6b4f5e <sqlite3VdbeExec+0x7efe>
  6b5905:	49 8b 45 48          	mov    0x48(%r13),%rax
  6b5909:	48 8b 00             	mov    (%rax),%rax
  6b590c:	83 38 02             	cmpl   $0x2,(%rax)
  6b590f:	0f 8c dd f8 ff ff    	jl     6b51f2 <sqlite3VdbeExec+0x8192>
  6b5915:	48 83 78 68 00       	cmpq   $0x0,0x68(%rax)
  6b591a:	0f 84 d2 f8 ff ff    	je     6b51f2 <sqlite3VdbeExec+0x8192>
  6b5920:	41 80 fc 05          	cmp    $0x5,%r12b
  6b5924:	0f 84 c8 f8 ff ff    	je     6b51f2 <sqlite3VdbeExec+0x8192>
  6b592a:	31 c0                	xor    %eax,%eax
  6b592c:	e9 77 f9 ff ff       	jmp    6b52a8 <sqlite3VdbeExec+0x8248>
  6b5931:	4d 89 cd             	mov    %r9,%r13
  6b5934:	49 8b 7a 28          	mov    0x28(%r10),%rdi
  6b5938:	49 89 7a 08          	mov    %rdi,0x8(%r10)
  6b593c:	4c 89 d5             	mov    %r10,%rbp
  6b593f:	48 89 ce             	mov    %rcx,%rsi
  6b5942:	4c 89 e2             	mov    %r12,%rdx
  6b5945:	e8 66 4c fa ff       	call   65a5b0 <memcpy$plt>
  6b594a:	48 8b 45 08          	mov    0x8(%rbp),%rax
  6b594e:	42 c6 04 20 00       	movb   $0x0,(%rax,%r12,1)
  6b5953:	48 8b 45 08          	mov    0x8(%rbp),%rax
  6b5957:	42 c6 44 20 01 00    	movb   $0x0,0x1(%rax,%r12,1)
  6b595d:	41 83 e5 01          	and    $0x1,%r13d
  6b5961:	48 8d 05 a4 5b f9 ff 	lea    -0x6a45c(%rip),%rax        # 64b50c <sqlite3VdbeExec.aFlag>
  6b5968:	42 0f b7 04 68       	movzwl (%rax,%r13,2),%eax
  6b596d:	66 89 45 14          	mov    %ax,0x14(%rbp)
  6b5971:	e9 3b aa ff ff       	jmp    6b03b1 <sqlite3VdbeExec+0x3351>
  6b5976:	66 c7 47 14 01 00    	movw   $0x1,0x14(%rdi)
  6b597c:	49 83 c5 18          	add    $0x18,%r13
  6b5980:	e9 9b 79 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b5985:	49 8b 7d 28          	mov    0x28(%r13),%rdi
  6b5989:	49 89 fd             	mov    %rdi,%r13
  6b598c:	e8 3f eb fd ff       	call   6944d0 <getCellInfo>
  6b5991:	49 83 c5 30          	add    $0x30,%r13
  6b5995:	48 c7 44 24 20 00 00 	movq   $0x0,0x20(%rsp)
  6b599c:	00 00 
  6b599e:	49 8b 45 00          	mov    0x0(%r13),%rax
  6b59a2:	49 89 04 24          	mov    %rax,(%r12)
  6b59a6:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b59ab:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b59af:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b59b4:	49 83 c5 18          	add    $0x18,%r13
  6b59b8:	e9 63 79 ff ff       	jmp    6ad320 <sqlite3VdbeExec+0x2c0>
  6b59bd:	48 c7 c3 ff ff ff ff 	mov    $0xffffffffffffffff,%rbx
  6b59c4:	b8 09 00 00 00       	mov    $0x9,%eax
  6b59c9:	e9 16 0a 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b59ce:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b59d3:	eb 08                	jmp    6b59dd <sqlite3VdbeExec+0x897d>
  6b59d5:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b59da:	4d 89 ef             	mov    %r13,%r15
  6b59dd:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b59e2:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b59e7:	e9 b8 77 ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b59ec:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b59f1:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b59f5:	e9 ea 09 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b59fa:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b59ff:	eb 2b                	jmp    6b5a2c <sqlite3VdbeExec+0x89cc>
  6b5a01:	48 8d 35 7b 74 f9 ff 	lea    -0x68b85(%rip),%rsi        # 64ce83 <.L.str.754>
  6b5a08:	48 8d 15 24 bc f9 ff 	lea    -0x643dc(%rip),%rdx        # 651633 <.L.str.755>
  6b5a0f:	4c 8d 05 71 8d f9 ff 	lea    -0x6728f(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b5a16:	bf 0b 00 00 00       	mov    $0xb,%edi
  6b5a1b:	b9 dc 87 01 00       	mov    $0x187dc,%ecx
  6b5a20:	31 c0                	xor    %eax,%eax
  6b5a22:	e8 59 95 fc ff       	call   67ef80 <sqlite3_log>
  6b5a27:	b8 0b 00 00 00       	mov    $0xb,%eax
  6b5a2c:	4d 89 ef             	mov    %r13,%r15
  6b5a2f:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5a34:	e9 ab 09 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5a39:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5a3e:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5a43:	e9 9c 09 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5a48:	83 e0 3f             	and    $0x3f,%eax
  6b5a4b:	48 8d 15 1e 52 b9 ff 	lea    -0x46ade2(%rip),%rdx        # 24ac70 <sqlite3_value_type.aType>
  6b5a52:	0f b6 04 10          	movzbl (%rax,%rdx,1),%eax
  6b5a56:	48 8d 15 03 37 0d 00 	lea    0xd3703(%rip),%rdx        # 789160 <vdbeMemTypeName.azTypes>
  6b5a5d:	48 8b 54 c2 f8       	mov    -0x8(%rdx,%rax,8),%rdx
  6b5a62:	48 8d 05 87 71 0d 00 	lea    0xd7187(%rip),%rax        # 78cbf0 <sqlite3StdType>
  6b5a69:	48 8b 4c c8 f8       	mov    -0x8(%rax,%rcx,8),%rcx
  6b5a6e:	4d 8b 07             	mov    (%r15),%r8
  6b5a71:	4d 8b 4d f2          	mov    -0xe(%r13),%r9
  6b5a75:	48 8d 35 df c0 f9 ff 	lea    -0x63f21(%rip),%rsi        # 651b5b <.L.str.285>
  6b5a7c:	4c 89 f7             	mov    %r14,%rdi
  6b5a7f:	31 c0                	xor    %eax,%eax
  6b5a81:	e8 5a d0 fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5a86:	b8 13 0c 00 00       	mov    $0xc13,%eax
  6b5a8b:	e9 99 08 00 00       	jmp    6b6329 <sqlite3VdbeExec+0x92c9>
  6b5a90:	b8 04 02 00 00       	mov    $0x204,%eax
  6b5a95:	e9 cf 05 00 00       	jmp    6b6069 <sqlite3VdbeExec+0x9009>
  6b5a9a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5a9f:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5aa3:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b5aa8:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5aad:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5ab2:	e9 ed 76 ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b5ab7:	e9 28 09 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5abc:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5ac1:	e9 68 08 00 00       	jmp    6b632e <sqlite3VdbeExec+0x92ce>
  6b5ac6:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5acb:	49 89 cc             	mov    %rcx,%r12
  6b5ace:	e9 3f 04 00 00       	jmp    6b5f12 <sqlite3VdbeExec+0x8eb2>
  6b5ad3:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5ad8:	e9 35 04 00 00       	jmp    6b5f12 <sqlite3VdbeExec+0x8eb2>
  6b5add:	4d 89 f7             	mov    %r14,%r15
  6b5ae0:	49 89 c6             	mov    %rax,%r14
  6b5ae3:	4d 85 e4             	test   %r12,%r12
  6b5ae6:	0f 84 ec 03 00 00    	je     6b5ed8 <sqlite3VdbeExec+0x8e78>
  6b5aec:	41 8b 44 24 14       	mov    0x14(%r12),%eax
  6b5af1:	89 c1                	mov    %eax,%ecx
  6b5af3:	f7 d1                	not    %ecx
  6b5af5:	f7 c1 02 02 00 00    	test   $0x202,%ecx
  6b5afb:	0f 85 c1 03 00 00    	jne    6b5ec2 <sqlite3VdbeExec+0x8e62>
  6b5b01:	41 80 7c 24 16 01    	cmpb   $0x1,0x16(%r12)
  6b5b07:	0f 85 b5 03 00 00    	jne    6b5ec2 <sqlite3VdbeExec+0x8e62>
  6b5b0d:	49 8b 54 24 08       	mov    0x8(%r12),%rdx
  6b5b12:	e9 c3 03 00 00       	jmp    6b5eda <sqlite3VdbeExec+0x8e7a>
  6b5b17:	49 8b 7d 00          	mov    0x0(%r13),%rdi
  6b5b1b:	4d 89 fc             	mov    %r15,%r12
  6b5b1e:	4d 89 f7             	mov    %r14,%r15
  6b5b21:	49 89 d6             	mov    %rdx,%r14
  6b5b24:	e8 d7 c5 fd ff       	call   692100 <sqlite3BtreeClose>
  6b5b29:	49 8b 47 78          	mov    0x78(%r15),%rax
  6b5b2d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5b32:	49 63 4d 04          	movslq 0x4(%r13),%rcx
  6b5b36:	48 c7 04 c8 00 00 00 	movq   $0x0,(%rax,%rcx,8)
  6b5b3d:	00 
  6b5b3e:	4c 89 f0             	mov    %r14,%rax
  6b5b41:	4d 89 fe             	mov    %r15,%r14
  6b5b44:	4d 89 e7             	mov    %r12,%r15
  6b5b47:	e9 e6 06 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b5b4c:	41 89 46 34          	mov    %eax,0x34(%r14)
  6b5b50:	41 0f b6 4d 08       	movzbl 0x8(%r13),%ecx
  6b5b55:	41 88 8e c4 00 00 00 	mov    %cl,0xc4(%r14)
  6b5b5c:	85 c0                	test   %eax,%eax
  6b5b5e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5b62:	75 2d                	jne    6b5b91 <sqlite3VdbeExec+0x8b31>
  6b5b64:	e9 89 05 00 00       	jmp    6b60f2 <sqlite3VdbeExec+0x9092>
  6b5b69:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5b6e:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5b73:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b5b78:	e9 27 76 ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b5b7d:	41 89 46 34          	mov    %eax,0x34(%r14)
  6b5b81:	41 0f b6 45 08       	movzbl 0x8(%r13),%eax
  6b5b86:	41 88 86 c4 00 00 00 	mov    %al,0xc4(%r14)
  6b5b8d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5b91:	49 63 45 0c          	movslq 0xc(%r13),%rax
  6b5b95:	48 85 c0             	test   %rax,%rax
  6b5b98:	7e 0b                	jle    6b5ba5 <sqlite3VdbeExec+0x8b45>
  6b5b9a:	41 80 7d 01 00       	cmpb   $0x0,0x1(%r13)
  6b5b9f:	0f 84 f1 04 00 00    	je     6b6096 <sqlite3VdbeExec+0x9036>
  6b5ba5:	41 0f b7 45 02       	movzwl 0x2(%r13),%eax
  6b5baa:	48 85 c0             	test   %rax,%rax
  6b5bad:	0f 84 e6 03 00 00    	je     6b5f99 <sqlite3VdbeExec+0x8f39>
  6b5bb3:	b9 ff ff ff ff       	mov    $0xffffffff,%ecx
  6b5bb8:	01 c8                	add    %ecx,%eax
  6b5bba:	48 8d 0d 1f 2f 0d 00 	lea    0xd2f1f(%rip),%rcx        # 788ae0 <sqlite3VdbeExec.azType>
  6b5bc1:	48 8b 14 c1          	mov    (%rcx,%rax,8),%rdx
  6b5bc5:	48 8d 35 b0 f6 f9 ff 	lea    -0x60950(%rip),%rsi        # 65527c <.L.str.282>
  6b5bcc:	4c 89 f7             	mov    %r14,%rdi
  6b5bcf:	31 c0                	xor    %eax,%eax
  6b5bd1:	e8 0a cf fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5bd6:	49 8b 4d 10          	mov    0x10(%r13),%rcx
  6b5bda:	48 85 c9             	test   %rcx,%rcx
  6b5bdd:	0f 84 dd 04 00 00    	je     6b60c0 <sqlite3VdbeExec+0x9060>
  6b5be3:	4c 8b bc 24 d0 00 00 	mov    0xd0(%rsp),%r15
  6b5bea:	00 
  6b5beb:	49 8b 17             	mov    (%r15),%rdx
  6b5bee:	48 8d 35 22 93 f9 ff 	lea    -0x66cde(%rip),%rsi        # 64ef17 <.L.str.283>
  6b5bf5:	4c 89 e7             	mov    %r12,%rdi
  6b5bf8:	31 c0                	xor    %eax,%eax
  6b5bfa:	e8 21 90 fc ff       	call   67ec20 <sqlite3MPrintf>
  6b5bff:	49 89 07             	mov    %rax,(%r15)
  6b5c02:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5c06:	e9 b5 04 00 00       	jmp    6b60c0 <sqlite3VdbeExec+0x9060>
  6b5c0b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5c0f:	e9 1e 06 00 00       	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b5c14:	48 8d 35 e3 c2 f9 ff 	lea    -0x63d1d(%rip),%rsi        # 651efe <.L.str.287>
  6b5c1b:	4c 89 f7             	mov    %r14,%rdi
  6b5c1e:	4c 89 e2             	mov    %r12,%rdx
  6b5c21:	31 c0                	xor    %eax,%eax
  6b5c23:	e8 b8 ce fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5c28:	b8 01 00 00 00       	mov    $0x1,%eax
  6b5c2d:	e9 ca 03 00 00       	jmp    6b5ffc <sqlite3VdbeExec+0x8f9c>
  6b5c32:	4c 8b a4 24 d0 00 00 	mov    0xd0(%rsp),%r12
  6b5c39:	00 
  6b5c3a:	49 8b 34 24          	mov    (%r12),%rsi
  6b5c3e:	4c 8b 7c 24 10       	mov    0x10(%rsp),%r15
  6b5c43:	4c 89 ff             	mov    %r15,%rdi
  6b5c46:	e8 75 49 fc ff       	call   67a5c0 <sqlite3DbFree>
  6b5c4b:	48 8d 35 33 70 f9 ff 	lea    -0x68fcd(%rip),%rsi        # 64cc85 <.L.str.293>
  6b5c52:	4c 89 ff             	mov    %r15,%rdi
  6b5c55:	e8 36 50 fc ff       	call   67ac90 <sqlite3DbStrDup>
  6b5c5a:	49 89 04 24          	mov    %rax,(%r12)
  6b5c5e:	49 8b 47 20          	mov    0x20(%r15),%rax
  6b5c62:	49 63 76 04          	movslq 0x4(%r14),%rsi
  6b5c66:	48 89 f1             	mov    %rsi,%rcx
  6b5c69:	48 c1 e1 05          	shl    $0x5,%rcx
  6b5c6d:	48 8b 44 08 18       	mov    0x18(%rax,%rcx,1),%rax
  6b5c72:	39 28                	cmp    %ebp,(%rax)
  6b5c74:	74 08                	je     6b5c7e <sqlite3VdbeExec+0x8c1e>
  6b5c76:	4c 89 ff             	mov    %r15,%rdi
  6b5c79:	e8 02 0e 00 00       	call   6b6a80 <sqlite3ResetOneSchema>
  6b5c7e:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5c83:	41 8b 86 c8 00 00 00 	mov    0xc8(%r14),%eax
  6b5c8a:	83 e0 ec             	and    $0xffffffec,%eax
  6b5c8d:	ff c0                	inc    %eax
  6b5c8f:	66 41 89 86 c8 00 00 	mov    %ax,0xc8(%r14)
  6b5c96:	00 
  6b5c97:	b8 11 00 00 00       	mov    $0x11,%eax
  6b5c9c:	e9 c3 03 00 00       	jmp    6b6064 <sqlite3VdbeExec+0x9004>
  6b5ca1:	48 8d 35 db 71 f9 ff 	lea    -0x68e25(%rip),%rsi        # 64ce83 <.L.str.754>
  6b5ca8:	48 8d 15 84 b9 f9 ff 	lea    -0x6467c(%rip),%rdx        # 651633 <.L.str.755>
  6b5caf:	4c 8d 05 d1 8a f9 ff 	lea    -0x6752f(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b5cb6:	bf 0b 00 00 00       	mov    $0xb,%edi
  6b5cbb:	b9 cb 82 01 00       	mov    $0x182cb,%ecx
  6b5cc0:	31 c0                	xor    %eax,%eax
  6b5cc2:	e8 b9 92 fc ff       	call   67ef80 <sqlite3_log>
  6b5cc7:	b8 0b 00 00 00       	mov    $0xb,%eax
  6b5ccc:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5cd1:	e9 0e 07 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5cd6:	4d 89 f7             	mov    %r14,%r15
  6b5cd9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5cde:	45 8b 75 04          	mov    0x4(%r13),%r14d
  6b5ce2:	41 8b 45 08          	mov    0x8(%r13),%eax
  6b5ce6:	41 0f b6 4c 24 65    	movzbl 0x65(%r12),%ecx
  6b5cec:	41 39 ce             	cmp    %ecx,%r14d
  6b5cef:	0f 85 33 04 00 00    	jne    6b6128 <sqlite3VdbeExec+0x90c8>
  6b5cf5:	85 c0                	test   %eax,%eax
  6b5cf7:	48 8d 05 46 72 f9 ff 	lea    -0x68dba(%rip),%rax        # 64cf44 <.L.str.292>
  6b5cfe:	48 8d 0d 0d 72 f9 ff 	lea    -0x68df3(%rip),%rcx        # 64cf12 <.L.str.291>
  6b5d05:	48 0f 44 c8          	cmove  %rax,%rcx
  6b5d09:	45 85 f6             	test   %r14d,%r14d
  6b5d0c:	48 8d 35 4d a9 f9 ff 	lea    -0x656b3(%rip),%rsi        # 650660 <.L.str.290>
  6b5d13:	48 0f 45 f1          	cmovne %rcx,%rsi
  6b5d17:	41 be 01 00 00 00    	mov    $0x1,%r14d
  6b5d1d:	4c 89 ff             	mov    %r15,%rdi
  6b5d20:	31 c0                	xor    %eax,%eax
  6b5d22:	e8 b9 cd fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5d27:	48 8b 0c 24          	mov    (%rsp),%rcx
  6b5d2b:	4c 89 f0             	mov    %r14,%rax
  6b5d2e:	e9 77 04 00 00       	jmp    6b61aa <sqlite3VdbeExec+0x914a>
  6b5d33:	41 8b 46 2c          	mov    0x2c(%r14),%eax
  6b5d37:	83 c8 01             	or     $0x1,%eax
  6b5d3a:	83 c0 02             	add    $0x2,%eax
  6b5d3d:	41 89 46 2c          	mov    %eax,0x2c(%r14)
  6b5d41:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5d46:	49 63 45 04          	movslq 0x4(%r13),%rax
  6b5d4a:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b5d4e:	48 8b 4c 24 28       	mov    0x28(%rsp),%rcx
  6b5d53:	48 01 c1             	add    %rax,%rcx
  6b5d56:	49 89 8e a0 00 00 00 	mov    %rcx,0xa0(%r14)
  6b5d5d:	41 80 7c 24 67 00    	cmpb   $0x0,0x67(%r12)
  6b5d63:	0f 85 a9 01 00 00    	jne    6b5f12 <sqlite3VdbeExec+0x8eb2>
  6b5d69:	41 f6 44 24 6e 04    	testb  $0x4,0x6e(%r12)
  6b5d6f:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5d73:	74 1a                	je     6b5d8f <sqlite3VdbeExec+0x8d2f>
  6b5d75:	49 8b b4 24 f8 00 00 	mov    0xf8(%r12),%rsi
  6b5d7c:	00 
  6b5d7d:	bf 04 00 00 00       	mov    $0x4,%edi
  6b5d82:	4c 89 f2             	mov    %r14,%rdx
  6b5d85:	31 c9                	xor    %ecx,%ecx
  6b5d87:	41 ff 94 24 f0 00 00 	call   *0xf0(%r12)
  6b5d8e:	00 
  6b5d8f:	4c 89 e8             	mov    %r13,%rax
  6b5d92:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6b5d97:	48 c1 e8 03          	shr    $0x3,%rax
  6b5d9b:	69 c0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%eax
  6b5da1:	ff c0                	inc    %eax
  6b5da3:	41 89 46 30          	mov    %eax,0x30(%r14)
  6b5da7:	bd 64 00 00 00       	mov    $0x64,%ebp
  6b5dac:	e9 df 08 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b5db1:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b5db6:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5dbb:	e9 57 01 00 00       	jmp    6b5f17 <sqlite3VdbeExec+0x8eb7>
  6b5dc0:	41 c6 86 c4 00 00 00 	movb   $0x2,0xc4(%r14)
  6b5dc7:	02 
  6b5dc8:	b8 06 00 00 00       	mov    $0x6,%eax
  6b5dcd:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5dd2:	e9 0d 06 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5dd7:	48 8d 35 67 02 fa ff 	lea    -0x5fd99(%rip),%rsi        # 656045 <.L.str.277>
  6b5dde:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
  6b5de3:	49 89 ff             	mov    %rdi,%r15
  6b5de6:	4c 89 e2             	mov    %r12,%rdx
  6b5de9:	49 89 c6             	mov    %rax,%r14
  6b5dec:	31 c0                	xor    %eax,%eax
  6b5dee:	e8 ed cc fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5df3:	4c 89 e7             	mov    %r12,%rdi
  6b5df6:	e8 95 31 fc ff       	call   678f90 <sqlite3_free>
  6b5dfb:	4c 89 f0             	mov    %r14,%rax
  6b5dfe:	83 f8 07             	cmp    $0x7,%eax
  6b5e01:	4c 89 e9             	mov    %r13,%rcx
  6b5e04:	0f 85 96 03 00 00    	jne    6b61a0 <sqlite3VdbeExec+0x9140>
  6b5e0a:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5e0f:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b5e14:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5e19:	4d 89 fe             	mov    %r15,%r14
  6b5e1c:	49 89 cf             	mov    %rcx,%r15
  6b5e1f:	e9 80 73 ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b5e24:	83 f8 0c             	cmp    $0xc,%eax
  6b5e27:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b5e2c:	0f 84 b2 05 00 00    	je     6b63e4 <sqlite3VdbeExec+0x9384>
  6b5e32:	e9 99 05 00 00       	jmp    6b63d0 <sqlite3VdbeExec+0x9370>
  6b5e37:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5e3c:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5e40:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5e45:	e9 9a 05 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5e4a:	41 c7 46 34 13 03 00 	movl   $0x313,0x34(%r14)
  6b5e51:	00 
  6b5e52:	41 c6 86 c4 00 00 00 	movb   $0x2,0xc4(%r14)
  6b5e59:	02 
  6b5e5a:	48 8d 35 d3 13 fa ff 	lea    -0x5ec2d(%rip),%rsi        # 657234 <.L.str.264>
  6b5e61:	4c 89 f7             	mov    %r14,%rdi
  6b5e64:	31 c0                	xor    %eax,%eax
  6b5e66:	e8 75 cc fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5e6b:	41 80 be c6 00 00 00 	cmpb   $0x0,0xc6(%r14)
  6b5e72:	00 
  6b5e73:	b8 01 00 00 00       	mov    $0x1,%eax
  6b5e78:	b9 13 03 00 00       	mov    $0x313,%ecx
  6b5e7d:	0f 49 c8             	cmovns %eax,%ecx
  6b5e80:	48 89 c8             	mov    %rcx,%rax
  6b5e83:	e9 e1 01 00 00       	jmp    6b6069 <sqlite3VdbeExec+0x9009>
  6b5e88:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5e8d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5e91:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5e96:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5e9b:	e9 44 05 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5ea0:	4c 89 e7             	mov    %r12,%rdi
  6b5ea3:	4d 89 f7             	mov    %r14,%r15
  6b5ea6:	49 89 c6             	mov    %rax,%r14
  6b5ea9:	e8 42 9c fe ff       	call   69faf0 <sqlite3ResetAllSchemasOfConnection>
  6b5eae:	4c 89 f0             	mov    %r14,%rax
  6b5eb1:	4d 89 fe             	mov    %r15,%r14
  6b5eb4:	83 f8 07             	cmp    $0x7,%eax
  6b5eb7:	74 59                	je     6b5f12 <sqlite3VdbeExec+0x8eb2>
  6b5eb9:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5ebd:	e9 22 05 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5ec2:	a8 01                	test   $0x1,%al
  6b5ec4:	75 12                	jne    6b5ed8 <sqlite3VdbeExec+0x8e78>
  6b5ec6:	4c 89 e7             	mov    %r12,%rdi
  6b5ec9:	be 01 00 00 00       	mov    $0x1,%esi
  6b5ece:	e8 cd bb fe ff       	call   6a1aa0 <valueToText>
  6b5ed3:	48 89 c2             	mov    %rax,%rdx
  6b5ed6:	eb 02                	jmp    6b5eda <sqlite3VdbeExec+0x8e7a>
  6b5ed8:	31 d2                	xor    %edx,%edx
  6b5eda:	48 8d 35 64 01 fa ff 	lea    -0x5fe9c(%rip),%rsi        # 656045 <.L.str.277>
  6b5ee1:	4c 89 ff             	mov    %r15,%rdi
  6b5ee4:	31 c0                	xor    %eax,%eax
  6b5ee6:	e8 f5 cb fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5eeb:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5ef0:	4c 89 f0             	mov    %r14,%rax
  6b5ef3:	4d 89 fe             	mov    %r15,%r14
  6b5ef6:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5efa:	e9 e5 04 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5eff:	48 8b 7c 24 60       	mov    0x60(%rsp),%rdi
  6b5f04:	41 ff 56 38          	call   *0x38(%r14)
  6b5f08:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5f0d:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5f12:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b5f17:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5f1b:	e9 84 72 ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b5f20:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5f25:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5f2a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5f2e:	e9 aa fa ff ff       	jmp    6b59dd <sqlite3VdbeExec+0x897d>
  6b5f33:	3c 06                	cmp    $0x6,%al
  6b5f35:	0f 85 a9 04 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5f3b:	49 8b 55 10          	mov    0x10(%r13),%rdx
  6b5f3f:	48 8d 35 4b ff f9 ff 	lea    -0x600b5(%rip),%rsi        # 655e91 <.L.str.301>
  6b5f46:	4c 89 f7             	mov    %r14,%rdi
  6b5f49:	4d 89 f7             	mov    %r14,%r15
  6b5f4c:	49 89 c6             	mov    %rax,%r14
  6b5f4f:	31 c0                	xor    %eax,%eax
  6b5f51:	e8 8a cb fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5f56:	eb 98                	jmp    6b5ef0 <sqlite3VdbeExec+0x8e90>
  6b5f58:	48 8d 35 d8 f9 f9 ff 	lea    -0x60628(%rip),%rsi        # 655937 <.L.str.297>
  6b5f5f:	4c 89 f7             	mov    %r14,%rdi
  6b5f62:	31 c0                	xor    %eax,%eax
  6b5f64:	e8 77 cb fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b5f69:	b8 01 00 00 00       	mov    $0x1,%eax
  6b5f6e:	e9 71 04 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b5f73:	bd 65 00 00 00       	mov    $0x65,%ebp
  6b5f78:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b5f7d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b5f82:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b5f86:	e9 05 07 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b5f8b:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5f90:	49 ff 46 38          	incq   0x38(%r14)
  6b5f94:	e9 97 02 00 00       	jmp    6b6230 <sqlite3VdbeExec+0x91d0>
  6b5f99:	49 8b 55 10          	mov    0x10(%r13),%rdx
  6b5f9d:	48 8d 35 a1 00 fa ff 	lea    -0x5ff5f(%rip),%rsi        # 656045 <.L.str.277>
  6b5fa4:	4c 89 f7             	mov    %r14,%rdi
  6b5fa7:	e9 0d 01 00 00       	jmp    6b60b9 <sqlite3VdbeExec+0x9059>
  6b5fac:	83 f8 05             	cmp    $0x5,%eax
  6b5faf:	0f 85 74 02 00 00    	jne    6b6229 <sqlite3VdbeExec+0x91c9>
  6b5fb5:	b0 02                	mov    $0x2,%al
  6b5fb7:	41 88 86 c4 00 00 00 	mov    %al,0xc4(%r14)
  6b5fbe:	e9 6d 02 00 00       	jmp    6b6230 <sqlite3VdbeExec+0x91d0>
  6b5fc3:	49 89 fc             	mov    %rdi,%r12
  6b5fc6:	e8 25 9b fe ff       	call   69faf0 <sqlite3ResetAllSchemasOfConnection>
  6b5fcb:	e9 42 ff ff ff       	jmp    6b5f12 <sqlite3VdbeExec+0x8eb2>
  6b5fd0:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5fd5:	e9 8a 00 00 00       	jmp    6b6064 <sqlite3VdbeExec+0x9004>
  6b5fda:	31 c9                	xor    %ecx,%ecx
  6b5fdc:	a9 00 00 10 00       	test   $0x100000,%eax
  6b5fe1:	0f 94 c1             	sete   %cl
  6b5fe4:	8d 04 49             	lea    (%rcx,%rcx,2),%eax
  6b5fe7:	83 c0 08             	add    $0x8,%eax
  6b5fea:	eb 78                	jmp    6b6064 <sqlite3VdbeExec+0x9004>
  6b5fec:	48 89 f7             	mov    %rsi,%rdi
  6b5fef:	e8 9c 2f fc ff       	call   678f90 <sqlite3_free>
  6b5ff4:	4c 89 f8             	mov    %r15,%rax
  6b5ff7:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b5ffc:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b6001:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b6005:	e9 da 03 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b600a:	48 8b 7c 24 60       	mov    0x60(%rsp),%rdi
  6b600f:	e8 7c 2f fc ff       	call   678f90 <sqlite3_free>
  6b6014:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b6019:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b601e:	e9 c1 03 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b6023:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b6028:	e9 a2 01 00 00       	jmp    6b61cf <sqlite3VdbeExec+0x916f>
  6b602d:	3c 05                	cmp    $0x5,%al
  6b602f:	75 33                	jne    6b6064 <sqlite3VdbeExec+0x9004>
  6b6031:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b6036:	48 89 c1             	mov    %rax,%rcx
  6b6039:	4c 89 e8             	mov    %r13,%rax
  6b603c:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6b6041:	48 c1 e8 03          	shr    $0x3,%rax
  6b6045:	69 c0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%eax
  6b604b:	41 89 46 30          	mov    %eax,0x30(%r14)
  6b604f:	48 89 cd             	mov    %rcx,%rbp
  6b6052:	41 89 4e 34          	mov    %ecx,0x34(%r14)
  6b6056:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b605b:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b605f:	e9 2c 06 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b6064:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b6069:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b606e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b6072:	e9 6d 03 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b6077:	48 8d 35 1b 9e f9 ff 	lea    -0x661e5(%rip),%rsi        # 64fe99 <.L.str.288>
  6b607e:	4c 89 f7             	mov    %r14,%rdi
  6b6081:	31 c0                	xor    %eax,%eax
  6b6083:	e8 58 ca fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b6088:	b8 05 00 00 00       	mov    $0x5,%eax
  6b608d:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b6091:	e9 4e 03 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b6096:	48 6b c0 38          	imul   $0x38,%rax,%rax
  6b609a:	48 8b 7c 24 28       	mov    0x28(%rsp),%rdi
  6b609f:	48 01 c7             	add    %rax,%rdi
  6b60a2:	be 01 00 00 00       	mov    $0x1,%esi
  6b60a7:	e8 c4 b9 fe ff       	call   6a1a70 <sqlite3ValueText>
  6b60ac:	48 8d 35 92 ff f9 ff 	lea    -0x6006e(%rip),%rsi        # 656045 <.L.str.277>
  6b60b3:	4c 89 f7             	mov    %r14,%rdi
  6b60b6:	48 89 c2             	mov    %rax,%rdx
  6b60b9:	31 c0                	xor    %eax,%eax
  6b60bb:	e8 20 ca fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b60c0:	4c 89 e8             	mov    %r13,%rax
  6b60c3:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6b60c8:	48 c1 e8 03          	shr    $0x3,%rax
  6b60cc:	69 d0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%edx
  6b60d2:	41 8b 7d 04          	mov    0x4(%r13),%edi
  6b60d6:	4d 8b 86 a8 00 00 00 	mov    0xa8(%r14),%r8
  6b60dd:	49 8b 8e f8 00 00 00 	mov    0xf8(%r14),%rcx
  6b60e4:	48 8d 35 1c e1 f9 ff 	lea    -0x61ee4(%rip),%rsi        # 654207 <.L.str.284>
  6b60eb:	31 c0                	xor    %eax,%eax
  6b60ed:	e8 8e 8e fc ff       	call   67ef80 <sqlite3_log>
  6b60f2:	4c 89 f7             	mov    %r14,%rdi
  6b60f5:	e8 36 09 ff ff       	call   6a6a30 <sqlite3VdbeHalt>
  6b60fa:	83 f8 05             	cmp    $0x5,%eax
  6b60fd:	75 12                	jne    6b6111 <sqlite3VdbeExec+0x90b1>
  6b60ff:	41 c7 46 34 05 00 00 	movl   $0x5,0x34(%r14)
  6b6106:	00 
  6b6107:	bd 05 00 00 00       	mov    $0x5,%ebp
  6b610c:	e9 7f 05 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b6111:	41 83 7e 34 00       	cmpl   $0x0,0x34(%r14)
  6b6116:	b8 65 00 00 00       	mov    $0x65,%eax
  6b611b:	bd 01 00 00 00       	mov    $0x1,%ebp
  6b6120:	0f 44 e8             	cmove  %eax,%ebp
  6b6123:	e9 68 05 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b6128:	85 c0                	test   %eax,%eax
  6b612a:	0f 84 46 01 00 00    	je     6b6276 <sqlite3VdbeExec+0x9216>
  6b6130:	4c 89 e7             	mov    %r12,%rdi
  6b6133:	be 04 02 00 00       	mov    $0x204,%esi
  6b6138:	e8 c3 0f ff ff       	call   6a7100 <sqlite3RollbackAll>
  6b613d:	b0 01                	mov    $0x1,%al
  6b613f:	41 88 44 24 65       	mov    %al,0x65(%r12)
  6b6144:	4c 89 ff             	mov    %r15,%rdi
  6b6147:	e8 e4 08 ff ff       	call   6a6a30 <sqlite3VdbeHalt>
  6b614c:	83 f8 05             	cmp    $0x5,%eax
  6b614f:	0f 85 7b 01 00 00    	jne    6b62d0 <sqlite3VdbeExec+0x9270>
  6b6155:	4c 89 e8             	mov    %r13,%rax
  6b6158:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6b615d:	48 c1 e8 03          	shr    $0x3,%rax
  6b6161:	69 c0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%eax
  6b6167:	41 89 47 30          	mov    %eax,0x30(%r15)
  6b616b:	b0 01                	mov    $0x1,%al
  6b616d:	44 28 f0             	sub    %r14b,%al
  6b6170:	41 88 44 24 65       	mov    %al,0x65(%r12)
  6b6175:	41 c7 47 34 05 00 00 	movl   $0x5,0x34(%r15)
  6b617c:	00 
  6b617d:	bd 05 00 00 00       	mov    $0x5,%ebp
  6b6182:	e9 e1 01 00 00       	jmp    6b6368 <sqlite3VdbeExec+0x9308>
  6b6187:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
  6b618c:	e9 53 02 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b6191:	b8 07 00 00 00       	mov    $0x7,%eax
  6b6196:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b619b:	e9 8c f8 ff ff       	jmp    6b5a2c <sqlite3VdbeExec+0x89cc>
  6b61a0:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b61a5:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b61aa:	4d 89 fe             	mov    %r15,%r14
  6b61ad:	49 89 cf             	mov    %rcx,%r15
  6b61b0:	e9 2f 02 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b61b5:	85 c0                	test   %eax,%eax
  6b61b7:	b9 0d 00 00 00       	mov    $0xd,%ecx
  6b61bc:	0f 45 c8             	cmovne %eax,%ecx
  6b61bf:	89 c8                	mov    %ecx,%eax
  6b61c1:	eb 0c                	jmp    6b61cf <sqlite3VdbeExec+0x916f>
  6b61c3:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b61c7:	e9 11 f8 ff ff       	jmp    6b59dd <sqlite3VdbeExec+0x897d>
  6b61cc:	4d 89 ee             	mov    %r13,%r14
  6b61cf:	e9 55 01 00 00       	jmp    6b6329 <sqlite3VdbeExec+0x92c9>
  6b61d4:	48 8d 35 a8 6c f9 ff 	lea    -0x69358(%rip),%rsi        # 64ce83 <.L.str.754>
  6b61db:	48 8d 15 a3 df f9 ff 	lea    -0x6205d(%rip),%rdx        # 654185 <.L.str.294>
  6b61e2:	4c 8d 05 9e 85 f9 ff 	lea    -0x67a62(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b61e9:	bf 0b 03 00 00       	mov    $0x30b,%edi
  6b61ee:	b9 0f 87 01 00       	mov    $0x1870f,%ecx
  6b61f3:	31 c0                	xor    %eax,%eax
  6b61f5:	e8 86 8d fc ff       	call   67ef80 <sqlite3_log>
  6b61fa:	b8 0b 03 00 00       	mov    $0x30b,%eax
  6b61ff:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b6204:	e9 db 01 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b6209:	48 8d 35 37 6c f9 ff 	lea    -0x693c9(%rip),%rsi        # 64ce47 <.L.str.286>
  6b6210:	4c 89 f7             	mov    %r14,%rdi
  6b6213:	31 c0                	xor    %eax,%eax
  6b6215:	e8 c6 c8 fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b621a:	b8 05 00 00 00       	mov    $0x5,%eax
  6b621f:	eb 11                	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b6221:	4d 89 fc             	mov    %r15,%r12
  6b6224:	e9 40 fe ff ff       	jmp    6b6069 <sqlite3VdbeExec+0x9009>
  6b6229:	41 88 86 c4 00 00 00 	mov    %al,0xc4(%r14)
  6b6230:	89 e8                	mov    %ebp,%eax
  6b6232:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b6237:	e9 a8 01 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b623c:	83 fd 05             	cmp    $0x5,%ebp
  6b623f:	48 8d 05 d9 05 fa ff 	lea    -0x5fa27(%rip),%rax        # 65681f <.L.str.299>
  6b6246:	48 8d 15 c5 0d fa ff 	lea    -0x5f23b(%rip),%rdx        # 657012 <.L.str.300>
  6b624d:	48 0f 44 d0          	cmove  %rax,%rdx
  6b6251:	48 8d 35 51 b3 f9 ff 	lea    -0x64caf(%rip),%rsi        # 6515a9 <.L.str.298>
  6b6258:	4c 89 f7             	mov    %r14,%rdi
  6b625b:	31 c0                	xor    %eax,%eax
  6b625d:	e8 7e c8 fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b6262:	b8 01 00 00 00       	mov    $0x1,%eax
  6b6267:	4d 89 fc             	mov    %r15,%r12
  6b626a:	e9 bf 00 00 00       	jmp    6b632e <sqlite3VdbeExec+0x92ce>
  6b626f:	b8 14 00 00 00       	mov    $0x14,%eax
  6b6274:	eb bc                	jmp    6b6232 <sqlite3VdbeExec+0x91d2>
  6b6276:	45 85 f6             	test   %r14d,%r14d
  6b6279:	0f 84 d5 00 00 00    	je     6b6354 <sqlite3VdbeExec+0x92f4>
  6b627f:	41 83 bc 24 d8 00 00 	cmpl   $0x0,0xd8(%r12)
  6b6286:	00 00 
  6b6288:	0f 8e c6 00 00 00    	jle    6b6354 <sqlite3VdbeExec+0x92f4>
  6b628e:	41 be 05 00 00 00    	mov    $0x5,%r14d
  6b6294:	48 8d 35 d5 a0 f9 ff 	lea    -0x65f2b(%rip),%rsi        # 650370 <.L.str.289>
  6b629b:	e9 7d fa ff ff       	jmp    6b5d1d <sqlite3VdbeExec+0x8cbd>
  6b62a0:	48 8d 35 dc 6b f9 ff 	lea    -0x69424(%rip),%rsi        # 64ce83 <.L.str.754>
  6b62a7:	48 8d 15 85 b3 f9 ff 	lea    -0x64c7b(%rip),%rdx        # 651633 <.L.str.755>
  6b62ae:	4c 8d 05 d2 84 f9 ff 	lea    -0x67b2e(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b62b5:	bf 0b 00 00 00       	mov    $0xb,%edi
  6b62ba:	b9 f7 79 01 00       	mov    $0x179f7,%ecx
  6b62bf:	31 c0                	xor    %eax,%eax
  6b62c1:	e8 ba 8c fc ff       	call   67ef80 <sqlite3_log>
  6b62c6:	b8 0b 00 00 00       	mov    $0xb,%eax
  6b62cb:	e9 14 01 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b62d0:	4c 89 e7             	mov    %r12,%rdi
  6b62d3:	e8 08 10 ff ff       	call   6a72e0 <sqlite3CloseSavepoints>
  6b62d8:	41 83 7f 34 00       	cmpl   $0x0,0x34(%r15)
  6b62dd:	b8 65 00 00 00       	mov    $0x65,%eax
  6b62e2:	bd 01 00 00 00       	mov    $0x1,%ebp
  6b62e7:	0f 44 e8             	cmove  %eax,%ebp
  6b62ea:	eb 7c                	jmp    6b6368 <sqlite3VdbeExec+0x9308>
  6b62ec:	83 f8 12             	cmp    $0x12,%eax
  6b62ef:	74 1d                	je     6b630e <sqlite3VdbeExec+0x92ae>
  6b62f1:	83 f8 07             	cmp    $0x7,%eax
  6b62f4:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b62f9:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b62fe:	0f 85 e0 00 00 00    	jne    6b63e4 <sqlite3VdbeExec+0x9384>
  6b6304:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b6309:	e9 96 6e ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b630e:	48 8d 35 83 68 f9 ff 	lea    -0x6977d(%rip),%rsi        # 64cb98 <.L.str.269>
  6b6315:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b631a:	4c 89 f7             	mov    %r14,%rdi
  6b631d:	31 c0                	xor    %eax,%eax
  6b631f:	e8 bc c7 fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b6324:	b8 12 00 00 00       	mov    $0x12,%eax
  6b6329:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b632e:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b6332:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b6337:	e9 a8 00 00 00       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b633c:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b6341:	4c 8b 64 24 10       	mov    0x10(%rsp),%r12
  6b6346:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b634a:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b634f:	e9 50 6e ff ff       	jmp    6ad1a4 <sqlite3VdbeExec+0x144>
  6b6354:	4c 89 ff             	mov    %r15,%rdi
  6b6357:	be 01 00 00 00       	mov    $0x1,%esi
  6b635c:	e8 6f 06 ff ff       	call   6a69d0 <sqlite3VdbeCheckFk>
  6b6361:	85 c0                	test   %eax,%eax
  6b6363:	74 63                	je     6b63c8 <sqlite3VdbeExec+0x9368>
  6b6365:	48 89 c5             	mov    %rax,%rbp
  6b6368:	48 8b 04 24          	mov    (%rsp),%rax
  6b636c:	4d 89 fe             	mov    %r15,%r14
  6b636f:	49 89 c7             	mov    %rax,%r15
  6b6372:	e9 19 03 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b6377:	48 89 c5             	mov    %rax,%rbp
  6b637a:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b637e:	e9 0d 03 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b6383:	4c 89 e8             	mov    %r13,%rax
  6b6386:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6b638b:	48 c1 e8 03          	shr    $0x3,%rax
  6b638f:	69 c0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%eax
  6b6395:	41 89 46 30          	mov    %eax,0x30(%r14)
  6b6399:	41 c6 44 24 65 00    	movb   $0x0,0x65(%r12)
  6b639f:	41 c7 46 34 05 00 00 	movl   $0x5,0x34(%r14)
  6b63a6:	00 
  6b63a7:	bd 05 00 00 00       	mov    $0x5,%ebp
  6b63ac:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b63b0:	e9 db 02 00 00       	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b63b5:	41 c6 44 24 65 00    	movb   $0x0,0x65(%r12)
  6b63bb:	4c 8b 3c 24          	mov    (%rsp),%r15
  6b63bf:	eb 23                	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b63c1:	89 e8                	mov    %ebp,%eax
  6b63c3:	e9 61 ff ff ff       	jmp    6b6329 <sqlite3VdbeExec+0x92c9>
  6b63c8:	44 89 f0             	mov    %r14d,%eax
  6b63cb:	e9 6f fd ff ff       	jmp    6b613f <sqlite3VdbeExec+0x90df>
  6b63d0:	89 e8                	mov    %ebp,%eax
  6b63d2:	48 81 c4 28 01 00 00 	add    $0x128,%rsp
  6b63d9:	5b                   	pop    %rbx
  6b63da:	41 5c                	pop    %r12
  6b63dc:	41 5d                	pop    %r13
  6b63de:	41 5e                	pop    %r14
  6b63e0:	41 5f                	pop    %r15
  6b63e2:	5d                   	pop    %rbp
  6b63e3:	c3                   	ret
  6b63e4:	48 89 1c 24          	mov    %rbx,(%rsp)
  6b63e8:	4c 89 f3             	mov    %r14,%rbx
  6b63eb:	bd 07 00 00 00       	mov    $0x7,%ebp
  6b63f0:	41 80 7c 24 67 00    	cmpb   $0x0,0x67(%r12)
  6b63f6:	75 34                	jne    6b642c <sqlite3VdbeExec+0x93cc>
  6b63f8:	89 c5                	mov    %eax,%ebp
  6b63fa:	3d 0a 21 00 00       	cmp    $0x210a,%eax
  6b63ff:	75 2b                	jne    6b642c <sqlite3VdbeExec+0x93cc>
  6b6401:	48 8d 35 7b 6a f9 ff 	lea    -0x69585(%rip),%rsi        # 64ce83 <.L.str.754>
  6b6408:	48 8d 15 24 b2 f9 ff 	lea    -0x64ddc(%rip),%rdx        # 651633 <.L.str.755>
  6b640f:	4c 8d 05 71 83 f9 ff 	lea    -0x67c8f(%rip),%r8        # 64e787 <.L.str.792+0x14>
  6b6416:	bd 0b 00 00 00       	mov    $0xb,%ebp
  6b641b:	bf 0b 00 00 00       	mov    $0xb,%edi
  6b6420:	b9 0b 91 01 00       	mov    $0x1910b,%ecx
  6b6425:	31 c0                	xor    %eax,%eax
  6b6427:	e8 54 8b fc ff       	call   67ef80 <sqlite3_log>
  6b642c:	48 83 bb a8 00 00 00 	cmpq   $0x0,0xa8(%rbx)
  6b6433:	00 
  6b6434:	75 6c                	jne    6b64a2 <sqlite3VdbeExec+0x9442>
  6b6436:	81 fd 0a 0c 00 00    	cmp    $0xc0a,%ebp
  6b643c:	74 64                	je     6b64a2 <sqlite3VdbeExec+0x9442>
  6b643e:	48 8d 15 8b c7 f9 ff 	lea    -0x63875(%rip),%rdx        # 652bd0 <.L.str.733>
  6b6445:	81 fd 04 02 00 00    	cmp    $0x204,%ebp
  6b644b:	74 44                	je     6b6491 <sqlite3VdbeExec+0x9431>
  6b644d:	83 fd 65             	cmp    $0x65,%ebp
  6b6450:	74 0e                	je     6b6460 <sqlite3VdbeExec+0x9400>
  6b6452:	83 fd 64             	cmp    $0x64,%ebp
  6b6455:	75 12                	jne    6b6469 <sqlite3VdbeExec+0x9409>
  6b6457:	48 8d 15 69 c5 f9 ff 	lea    -0x63a97(%rip),%rdx        # 6529c7 <.L.str.734>
  6b645e:	eb 31                	jmp    6b6491 <sqlite3VdbeExec+0x9431>
  6b6460:	48 8d 15 3a ac f9 ff 	lea    -0x653c6(%rip),%rdx        # 6510a1 <.L.str.735>
  6b6467:	eb 28                	jmp    6b6491 <sqlite3VdbeExec+0x9431>
  6b6469:	40 0f b6 c5          	movzbl %bpl,%eax
  6b646d:	48 8d 15 ce d4 f9 ff 	lea    -0x62b32(%rip),%rdx        # 653942 <.L.str.732>
  6b6474:	83 f8 1c             	cmp    $0x1c,%eax
  6b6477:	77 18                	ja     6b6491 <sqlite3VdbeExec+0x9431>
  6b6479:	b9 04 00 41 01       	mov    $0x1410004,%ecx
  6b647e:	48 0f a3 c1          	bt     %rax,%rcx
  6b6482:	72 0d                	jb     6b6491 <sqlite3VdbeExec+0x9431>
  6b6484:	89 c0                	mov    %eax,%eax
  6b6486:	48 8d 0d a3 28 0d 00 	lea    0xd28a3(%rip),%rcx        # 788d30 <sqlite3ErrStr.aMsg>
  6b648d:	48 8b 14 c1          	mov    (%rcx,%rax,8),%rdx
  6b6491:	48 8d 35 ad fb f9 ff 	lea    -0x60453(%rip),%rsi        # 656045 <.L.str.277>
  6b6498:	48 89 df             	mov    %rbx,%rdi
  6b649b:	31 c0                	xor    %eax,%eax
  6b649d:	e8 3e c6 fe ff       	call   6a2ae0 <sqlite3VdbeError>
  6b64a2:	81 fd 0a 0c 00 00    	cmp    $0xc0a,%ebp
  6b64a8:	0f 94 c0             	sete   %al
  6b64ab:	89 6b 34             	mov    %ebp,0x34(%rbx)
  6b64ae:	89 e9                	mov    %ebp,%ecx
  6b64b0:	81 e1 fb 00 00 00    	and    $0xfb,%ecx
  6b64b6:	83 f9 0a             	cmp    $0xa,%ecx
  6b64b9:	0f 95 c1             	setne  %cl
  6b64bc:	08 c1                	or     %al,%cl
  6b64be:	75 1d                	jne    6b64dd <sqlite3VdbeExec+0x947d>
  6b64c0:	49 8b 3c 24          	mov    (%r12),%rdi
  6b64c4:	48 8b 8f 80 00 00 00 	mov    0x80(%rdi),%rcx
  6b64cb:	31 c0                	xor    %eax,%eax
  6b64cd:	48 85 c9             	test   %rcx,%rcx
  6b64d0:	74 06                	je     6b64d8 <sqlite3VdbeExec+0x9478>
  6b64d2:	31 f6                	xor    %esi,%esi
  6b64d4:	31 d2                	xor    %edx,%edx
  6b64d6:	ff d1                	call   *%rcx
  6b64d8:	41 89 44 24 5c       	mov    %eax,0x5c(%r12)
  6b64dd:	4c 89 e8             	mov    %r13,%rax
  6b64e0:	48 2b 44 24 58       	sub    0x58(%rsp),%rax
  6b64e5:	48 c1 e8 03          	shr    $0x3,%rax
  6b64e9:	69 d0 ab aa aa aa    	imul   $0xaaaaaaab,%eax,%edx
  6b64ef:	4c 8b 83 a8 00 00 00 	mov    0xa8(%rbx),%r8
  6b64f6:	48 8b 8b f8 00 00 00 	mov    0xf8(%rbx),%rcx
  6b64fd:	48 8d 35 98 ae f9 ff 	lea    -0x65168(%rip),%rsi        # 65139c <.L.str.304>
  6b6504:	89 ef                	mov    %ebp,%edi
  6b6506:	31 c0                	xor    %eax,%eax
  6b6508:	e8 73 8a fc ff       	call   67ef80 <sqlite3_log>
  6b650d:	80 bb c7 00 00 00 02 	cmpb   $0x2,0xc7(%rbx)
  6b6514:	75 08                	jne    6b651e <sqlite3VdbeExec+0x94be>
  6b6516:	48 89 df             	mov    %rbx,%rdi
  6b6519:	e8 12 05 ff ff       	call   6a6a30 <sqlite3VdbeHalt>
  6b651e:	83 fd 0b             	cmp    $0xb,%ebp
  6b6521:	0f 84 a1 00 00 00    	je     6b65c8 <sqlite3VdbeExec+0x9568>
  6b6527:	81 fd 0a 0c 00 00    	cmp    $0xc0a,%ebp
  6b652d:	49 89 de             	mov    %rbx,%r14
  6b6530:	48 8b 1c 24          	mov    (%rsp),%rbx
  6b6534:	0f 85 a3 00 00 00    	jne    6b65dd <sqlite3VdbeExec+0x957d>
  6b653a:	41 80 7c 24 67 00    	cmpb   $0x0,0x67(%r12)
  6b6540:	0f 85 97 00 00 00    	jne    6b65dd <sqlite3VdbeExec+0x957d>
  6b6546:	41 80 7c 24 68 00    	cmpb   $0x0,0x68(%r12)
  6b654c:	0f 85 8b 00 00 00    	jne    6b65dd <sqlite3VdbeExec+0x957d>
  6b6552:	41 c6 44 24 67 01    	movb   $0x1,0x67(%r12)
  6b6558:	41 83 bc 24 dc 00 00 	cmpl   $0x0,0xdc(%r12)
  6b655f:	00 00 
  6b6561:	7e 0c                	jle    6b656f <sqlite3VdbeExec+0x950f>
  6b6563:	41 c7 84 24 90 01 00 	movl   $0x1,0x190(%r12)
  6b656a:	00 01 00 00 00 
  6b656f:	41 ff 84 24 98 01 00 	incl   0x198(%r12)
  6b6576:	00 
  6b6577:	66 41 c7 84 24 9c 01 	movw   $0x0,0x19c(%r12)
  6b657e:	00 00 00 00 
  6b6582:	49 8b bc 24 58 01 00 	mov    0x158(%r12),%rdi
  6b6589:	00 
  6b658a:	48 85 ff             	test   %rdi,%rdi
  6b658d:	74 4e                	je     6b65dd <sqlite3VdbeExec+0x957d>
  6b658f:	48 8d 35 85 7c f9 ff 	lea    -0x6837b(%rip),%rsi        # 64e21b <.L.str.16>
  6b6596:	31 c0                	xor    %eax,%eax
  6b6598:	e8 93 49 fc ff       	call   67af30 <sqlite3ErrorMsg>
  6b659d:	49 8b 84 24 58 01 00 	mov    0x158(%r12),%rax
  6b65a4:	00 
  6b65a5:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
  6b65ac:	00 00 00 00 
  6b65b0:	c7 40 18 07 00 00 00 	movl   $0x7,0x18(%rax)
  6b65b7:	48 8b 80 08 01 00 00 	mov    0x108(%rax),%rax
  6b65be:	48 85 c0             	test   %rax,%rax
  6b65c1:	74 1a                	je     6b65dd <sqlite3VdbeExec+0x957d>
  6b65c3:	ff 40 34             	incl   0x34(%rax)
  6b65c6:	eb e8                	jmp    6b65b0 <sqlite3VdbeExec+0x9550>
  6b65c8:	41 80 7c 24 65 00    	cmpb   $0x0,0x65(%r12)
  6b65ce:	49 89 de             	mov    %rbx,%r14
  6b65d1:	48 8b 1c 24          	mov    (%rsp),%rbx
  6b65d5:	75 06                	jne    6b65dd <sqlite3VdbeExec+0x957d>
  6b65d7:	41 80 4c 24 34 02    	orb    $0x2,0x34(%r12)
  6b65dd:	bd 01 00 00 00       	mov    $0x1,%ebp
  6b65e2:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
  6b65e7:	84 c0                	test   %al,%al
  6b65e9:	74 7f                	je     6b666a <sqlite3VdbeExec+0x960a>
  6b65eb:	0f b6 c0             	movzbl %al,%eax
  6b65ee:	b9 ff ff ff ff       	mov    $0xffffffff,%ecx
  6b65f3:	01 c8                	add    %ecx,%eax
  6b65f5:	49 8b 4c 24 20       	mov    0x20(%r12),%rcx
  6b65fa:	48 c1 e0 05          	shl    $0x5,%rax
  6b65fe:	48 8b 44 01 18       	mov    0x18(%rcx,%rax,1),%rax
  6b6603:	66 83 48 72 08       	orw    $0x8,0x72(%rax)
  6b6608:	48 8b 41 38          	mov    0x38(%rcx),%rax
  6b660c:	66 83 48 72 08       	orw    $0x8,0x72(%rax)
  6b6611:	41 80 64 24 2c ef    	andb   $0xef,0x2c(%r12)
  6b6617:	41 83 7c 24 48 00    	cmpl   $0x0,0x48(%r12)
  6b661d:	75 71                	jne    6b6690 <sqlite3VdbeExec+0x9630>
  6b661f:	41 8b 44 24 28       	mov    0x28(%r12),%eax
  6b6624:	85 c0                	test   %eax,%eax
  6b6626:	7e 68                	jle    6b6690 <sqlite3VdbeExec+0x9630>
  6b6628:	4c 89 6c 24 08       	mov    %r13,0x8(%rsp)
  6b662d:	4d 89 fd             	mov    %r15,%r13
  6b6630:	41 be 18 00 00 00    	mov    $0x18,%r14d
  6b6636:	45 31 ff             	xor    %r15d,%r15d
  6b6639:	eb 14                	jmp    6b664f <sqlite3VdbeExec+0x95ef>
  6b663b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  6b6640:	49 ff c7             	inc    %r15
  6b6643:	48 63 c8             	movslq %eax,%rcx
  6b6646:	49 83 c6 20          	add    $0x20,%r14
  6b664a:	49 39 cf             	cmp    %rcx,%r15
  6b664d:	7d 26                	jge    6b6675 <sqlite3VdbeExec+0x9615>
  6b664f:	49 8b 4c 24 20       	mov    0x20(%r12),%rcx
  6b6654:	4a 8b 3c 31          	mov    (%rcx,%r14,1),%rdi
  6b6658:	f6 47 72 08          	testb  $0x8,0x72(%rdi)
  6b665c:	74 e2                	je     6b6640 <sqlite3VdbeExec+0x95e0>
  6b665e:	e8 1d 18 00 00       	call   6b7e80 <sqlite3SchemaClear>
  6b6663:	41 8b 44 24 28       	mov    0x28(%r12),%eax
  6b6668:	eb d6                	jmp    6b6640 <sqlite3VdbeExec+0x95e0>
  6b666a:	48 c7 44 24 48 00 00 	movq   $0x0,0x48(%rsp)
  6b6671:	00 00 
  6b6673:	eb 1b                	jmp    6b6690 <sqlite3VdbeExec+0x9630>
  6b6675:	4c 8b 74 24 30       	mov    0x30(%rsp),%r14
  6b667a:	4d 89 ef             	mov    %r13,%r15
  6b667d:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
  6b6682:	66 66 66 66 66 2e 0f 	data16 data16 data16 data16 cs nopw 0x0(%rax,%rax,1)
  6b6689:	1f 84 00 00 00 00 00 
  6b6690:	49 39 df             	cmp    %rbx,%r15
  6b6693:	72 37                	jb     6b66cc <sqlite3VdbeExec+0x966c>
  6b6695:	49 8b 84 24 08 02 00 	mov    0x208(%r12),%rax
  6b669c:	00 
  6b669d:	48 85 c0             	test   %rax,%rax
  6b66a0:	74 2a                	je     6b66cc <sqlite3VdbeExec+0x966c>
  6b66a2:	41 8b 8c 24 18 02 00 	mov    0x218(%r12),%ecx
  6b66a9:	00 
  6b66aa:	48 01 cb             	add    %rcx,%rbx
  6b66ad:	49 8b bc 24 10 02 00 	mov    0x210(%r12),%rdi
  6b66b4:	00 
  6b66b5:	ff d0                	call   *%rax
  6b66b7:	85 c0                	test   %eax,%eax
  6b66b9:	74 d5                	je     6b6690 <sqlite3VdbeExec+0x9630>
  6b66bb:	b8 09 00 00 00       	mov    $0x9,%eax
  6b66c0:	48 c7 c3 ff ff ff ff 	mov    $0xffffffffffffffff,%rbx
  6b66c7:	e9 18 fd ff ff       	jmp    6b63e4 <sqlite3VdbeExec+0x9384>
  6b66cc:	45 01 be e4 00 00 00 	add    %r15d,0xe4(%r14)
  6b66d3:	e9 f8 fc ff ff       	jmp    6b63d0 <sqlite3VdbeExec+0x9370>
  6b66d8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
  6b66df:	00 

00000000006b66e0 <vdbeColumnFromOverflow>:
