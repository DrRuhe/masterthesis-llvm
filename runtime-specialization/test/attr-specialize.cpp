// RUN: clang -emit-llvm -S -O0 %s -o - | opt -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializer%shlibext -passes='runtime-specializer' -S | FileCheck %s




class A {
  int value;

public:
  A(int val) : value(val) {}

  int getMod2() const {
    return value % 2;
  }
};

int main(int argc, char** argv) {
  A instance(argc);

  //CHECK: @.str = private unnamed_addr constant [11 x i8] c"specialize\00", section "llvm.metadata"
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %result, ptr @.str, ptr @.str.1, i32 [[#]], ptr null)
  [[clang::annotate("specialize")]]
  int result = instance.getMod2();

  // Aufruf über den speziellen Member-Pointer-Operator
  return result;
}
