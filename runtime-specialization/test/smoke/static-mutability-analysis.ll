; RUN: opt -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes=static-mutability-analysis -print-changed=cdiff -S %s | FileCheck %s

%struct.Data = type { i32, i32, i32 }

; Test 1: Simple read-only field
define i32 @test_read_only(ptr %d) {
entry:
; CHECK: %val = load i32, ptr %d, align 4, !invariant.load !0
  %val = load i32, ptr %d, align 4
  ret i32 %val
}

; Test 2: One field is mutated, other is read-only: only the readonly field is annotated.
define i32 @test_mixed(ptr %d, i32 %x) {
entry:
  %gep_read = getelementptr inbounds %struct.Data, ptr %d, i32 0, i32 0
; CHECK: %val = load i32, ptr %gep_read, align 4, !invariant.load !0
  %val = load i32, ptr %gep_read, align 4



  %gep_mut = getelementptr inbounds %struct.Data, ptr %d, i32 0, i32 1
; CHECK-NOT: %val_mut = load i32, ptr %gep_mut, align 4, !invariant.load !0
  %val_mut = load i32, ptr %gep_mut, align 4
  %sum = add i32 %val_mut, %x
  store i32 %sum, ptr %gep_mut, align 4
  ret i32 %val
}

; Test 3: Mutated field should NOT have !invariant.load
define i32 @test_mutated(ptr %d, i32 %x) {
entry:
  %gep_mut = getelementptr inbounds %struct.Data, ptr %d, i32 0, i32 1
  store i32 %x, ptr %gep_mut, align 4
; CHECK: %val = load i32, ptr %gep_mut, align 4
; CHECK-NOT: !invariant.load
  %val = load i32, ptr %gep_mut, align 4
  ret i32 %val
}

; Test 4: Nested structures
%struct.Outer = type { %struct.Data, i32 }

define i32 @test_nested(ptr %o, i32 %x) {
entry:
  %gep_inner_read = getelementptr inbounds %struct.Outer, ptr %o, i32 0, i32 0, i32 0
; CHECK: %val = load i32, ptr %gep_inner_read, align 4, !invariant.load !0
  %val = load i32, ptr %gep_inner_read, align 4
  
  %gep_outer_mut = getelementptr inbounds %struct.Outer, ptr %o, i32 0, i32 1
  store i32 %x, ptr %gep_outer_mut, align 4
  ret i32 %val
}

; Test 5: Call to function that modifies the field
declare void @bar(ptr %d, i32 %new_val)

define i32 @test_call_modifies(ptr %d, i32 %new_val) {
entry:
  %gep0 = getelementptr inbounds %struct.Data, ptr %d, i32 0, i32 0
  ; CHECK-NOT: %old = load i32, ptr %gep0, align 4, !invariant.load !0
  %old = load i32, ptr %gep0, align 4
  
  %gep1 = getelementptr inbounds %struct.Data, ptr %d, i32 0, i32 1
  ; This load is NOT invariant because %d escapes to @bar
  ; CHECK: %val1 = load i32, ptr %gep1, align 4
  %val1 = load i32, ptr %gep1, align 4
  
  call void @bar(ptr %d, i32 %new_val)
  
  ; CHECK-NOT: %old_after = load i32, ptr %gep0, align 4, !invariant.load !0
  %old_after = load i32, ptr %gep0, align 4
  
  %sum = add i32 %old, %val1
  %sum2 = add i32 %sum, %old_after
  ret i32 %sum2
}

; CHECK: !0 = !{}
