// RUN: clang -emit-llvm -S -O0 %s -o - | opt -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializer%shlibext -passes='runtime-specializer','runtime-specializeable-ir-finalizer' -S | FileCheck --dump-input=always %s

//CHECK: @RuntimeSpecializeableIR_ptr = constant ptr @RuntimeSpecializeableIR_data
//CHECK: @RuntimeSpecializeableIR_len = constant i64 [[LEN:[1-9][0-9]*]]
//CHECK: @RuntimeSpecializeableIR_data = internal unnamed_addr constant

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

  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %result, ptr @.str, ptr @.str.1, i32 [[#]], ptr null)
  [[clang::annotate("specialize")]]
  int result = instance.getMod2();

  // Aufruf über den speziellen Member-Pointer-Operator
  return result;
}