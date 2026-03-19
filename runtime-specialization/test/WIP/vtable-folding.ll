; RUN: opt -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes=vtable-constant-folding -print-changed=cdiff -S %s | FileCheck %s

%class.Derived = type { %class.Base }
%class.Base = type { ptr }

@_ZTV7Derived = linkonce_odr dso_local unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @derived_val] }, align 8

define i32 @derived_val(ptr %this) {
  ret i32 2
}

define i32 @test_alloca() {
entry:
  %d = alloca %class.Derived, align 8
  ; Store the vtable pointer to the alloca (offset 0)
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Derived, i32 0, i32 0, i32 2), ptr %d, align 8
  
  ; Load the vtable pointer back
  ; CHECK-LABEL: define i32 @test_alloca() {
  ; CHECK: %d = alloca %class.Derived
  ; CHECK-NOT: %vtable = load ptr, ptr %d
  %vtable = load ptr, ptr %d, align 8
  
  ; Use the vtable pointer
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 0
  %0 = load ptr, ptr %vfn, align 8
  ; CHECK: %0 = load ptr, ptr %vfn
  ; CHECK: %call = call i32 %0(ptr %d)
  %call = call i32 %0(ptr %d)
  ret i32 %call
}

@internal_obj = internal global %class.Base { ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Derived, i32 0, i32 0, i32 2) }, align 8

define i32 @test_global() {
entry:
  ; CHECK-LABEL: define i32 @test_global() {
  ; CHECK-NOT: %vtable = load ptr, ptr @internal_obj
  %vtable = load ptr, ptr @internal_obj, align 8
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 0
  %0 = load ptr, ptr %vfn, align 8
  ; CHECK: %0 = load ptr, ptr %vfn
  ; CHECK: %call = call i32 %0(ptr @internal_obj)
  %call = call i32 %0(ptr @internal_obj)
  ret i32 %call
}

