# Research: Lambda Specialization API

**Phase 0 output for plan.md**  
**Date**: 2026-05-10

---

## 1. Lambda Type Traits in C++17

**Decision**: Use member-pointer deduction on `&Lambda::operator()` to extract return type and argument list.

```cpp
namespace detail {
  template <class F> struct LambdaTraits;

  template <class L, class R, class... Args>
  struct LambdaTraits<R (L::*)(Args...) const> {
    using RetType  = R;
    using ArgTypes = std::tuple<Args...>;       // explicit args
    using FnPtrType = R(*)(Args...);
  };

  template <class L, class R, class... Args>
  struct LambdaTraits<R (L::*)(Args...)> {      // mutable lambda
    using RetType  = R;
    using ArgTypes = std::tuple<Args...>;
    using FnPtrType = R(*)(Args...);
  };
} // namespace detail

template <class Lambda>
using LambdaExplicitArgs = typename detail::LambdaTraits<
    decltype(&Lambda::operator())>::ArgTypes;
```

**Rationale**: Standard C++17; works for any concrete non-generic lambda.  Generic lambdas (`auto` params) have a templated `operator()` and cannot be resolved this way — correctly excluded by the spec.

**Alternatives considered**: `std::function` deduction guides — rejected because they require the lambda to be wrapped, adding overhead and losing the zero-overhead abstraction goal.

---

## 2. LLVM IR Type Mapping for Variadic Arg Lists

**Decision**: Add a `serializeTypeToLLVM<T>(LLVMContext&)` helper alongside the existing `serializeArgumentToIR`.  This companion maps a C++ type `T` to its LLVM `Type*` without needing a runtime value.

```cpp
template <class T>
static llvm::Type* serializeTypeToLLVM(llvm::LLVMContext& Ctx) {
  using D = std::decay_t<T>;
  if constexpr (std::is_integral_v<D> && !std::is_same_v<D, bool>)
    return llvm::Type::getIntNTy(Ctx, sizeof(D) * 8);
  else if constexpr (std::is_same_v<D, bool>)
    return llvm::Type::getInt1Ty(Ctx);
  else if constexpr (std::is_same_v<D, float>)
    return llvm::Type::getFloatTy(Ctx);
  else if constexpr (std::is_floating_point_v<D>)
    return llvm::Type::getDoubleTy(Ctx);
  else // pointer or class → opaque pointer
    return llvm::PointerType::getUnqual(Ctx);
}

// Variadic expansion: tuple<int, bool> → {Int32Ty, Int1Ty}
template <class Tuple, size_t... I>
std::vector<llvm::Type*> tupleToLLVMTypes(llvm::LLVMContext& Ctx,
                                          std::index_sequence<I...>) {
  return { serializeTypeToLLVM<std::tuple_element_t<I, Tuple>>(Ctx)... };
}
```

**Rationale**: Separating type-to-LLVM-type mapping from value-to-constant mapping avoids passing dummy values and keeps each concern in one place.

---

## 3. JIT Wrapper Function Shape for Partial Specialization

**Decision**: `specializeLambdaImpl` creates a wrapper function whose signature is
`R(ExplicitArgTypes...)` (non-zero parameter count), with the lambda's closure address
baked in as a pointer constant for the kernel's first parameter.

### Current (specializeOnlyImpl) wrapper shape

```llvm
define R @specialized_wrapper_N() {
entry:
  ; all original args become IR constants
  %result = call R @kernel(i64 baked_a0, i32 baked_a1, ...)
  ret R %result
}
```

### New (specializeLambdaImpl) wrapper shape

```llvm
define R @specialized_lambda_wrapper_N(i32 %x, i1 %y) {
entry:
  ; lambda closure address is baked in as a pointer constant
  ; explicit args (x, y) are live parameters forwarded unchanged
  %result = call R @kernel(ptr inttoptr(i64 {lambda_addr} to ptr), i32 %x, i1 %y)
  ret R %result
}
```

**Implementation**:
```cpp
// Build explicit-arg parameter list from ExplicitArgTypes...
std::vector<llvm::Type*> ParamTys = tupleToLLVMTypes<ArgTuple>(Ctx, idx_seq);
llvm::FunctionType* WrapperTy = llvm::FunctionType::get(RetTy, ParamTys, false);
llvm::Function* WrapperFn = llvm::Function::Create(
    WrapperTy, llvm::Function::ExternalLinkage, UniqueName, *NewModule);
llvm::BasicBlock* BB = llvm::BasicBlock::Create(Ctx, "entry", WrapperFn);
llvm::IRBuilder<> B(BB);

// Serialize lambda closure as pointer constant (baked)
std::vector<llvm::Value*> CallArgs;
CallArgs.push_back(serializeArgumentToIR(B, lambda)); // lambda is a class → serialized as &lambda

// Forward the wrapper's live parameters unchanged
for (auto& Arg : WrapperFn->args())
  CallArgs.push_back(&Arg);

auto* CI = B.CreateCall(TargetFunc->getFunctionType(), TargetFunc, CallArgs);
CI->addFnAttr(llvm::Attribute::AlwaysInline);
if (RetTy->isVoidTy()) B.CreateRetVoid(); else B.CreateRet(CI);
```

**Rationale**: Reuses every component of `specializeOnlyImpl` except the wrapper `FunctionType`
and how live parameters are forwarded.  No new JIT infrastructure required.

---

## 4. SpecializedLambda<R, Args...> Type Design

**Decision**: Mirror `SpecializedFunction<R>` exactly, generalising the stored function pointer from `R(*)()` to `R(*)(Args...)`.

Key changes vs `SpecializedFunction<R>`:
- `R(*FnPtr)(Args...)` instead of `R(*FnPtr)()`
- `operator()(Args... args) const { return FnPtr(args...); }`
- Constructor takes `R(*fp)(Args...)` (same RAII handshake with JITDylib & ExecutionSession)
- Default-constructed / falsy → `operator()(Args...)` throws `ClangRuntimeSpecializerError`

**Rationale**: Identical ownership and cleanup semantics to `SpecializedFunction<R>`; users get predictable lifetime management without learning a new pattern.

---

## 5. assertSpecializedLambdaIsEquivalent Helper

**Decision**: Thin wrapper over `assertSpecializedIsEquivalent`; iterates over a range of input tuples rather than a single pair of tuples.

```cpp
template <class R, class Lambda, class InputRange>
void assertSpecializedLambdaIsEquivalent(
    const char* funcName, Lambda& lambda, const InputRange& inputs) {
  auto spec = specializeLambda<R>(funcName, lambda);
  for (auto& [args] : inputs) {
    R expected = std::apply(lambda,  args);
    R actual   = std::apply(spec,    args);
    if (expected != actual)
      throw ClangRuntimeSpecializerChangesBehaviorError(...);
  }
  CRS_LOG(Info, "assertSpecializedLambdaIsEquivalent passed for: " + funcName);
}
```

**Rationale**: Re-uses the same `SpecializedLambda` across all test inputs (no re-JIT per input), so the assertion is fast and checks real call-path consistency.

---

## 6. Test Fixture Pattern

**Decision**: Each smoke test is a single `.cpp` file (compiled with IRDumpingPass) that defines both the kernel and the test body.  Since the kernel is in the same TU as the test harness, the TU-separation constraint must be relaxed for tests — or the kernel can be defined with `extern "C"` and linked from a separate `.cpp`.

**Resolution**: For smoke tests, define the kernel in the **same file** using `extern "C"` with `__asm__` to give it a stable mangled name (matching the pattern in `pure-specialized-function-is-equivalent.cpp`).  This keeps each test self-contained and avoids multi-file lit complexity.

The lambda in the test captures a local variable whose address is stable for the test duration, matching the assumption in the spec.

**Rationale**: Existing smoke tests (e.g., `pure-specialized-function-is-equivalent.cpp`) already use this single-file pattern with stable names; extending it is consistent and low-friction.

---

## 7. Resolved Unknowns

| Unknown | Resolution |
|---------|-----------|
| How to extract lambda explicit arg types | `LambdaTraits<decltype(&Lambda::operator())>` |
| How to map C++ arg types to LLVM types without values | New `serializeTypeToLLVM<T>` template |
| How to construct a non-zero-arg JIT wrapper | Parameterize `FunctionType` with `ExplicitArgTypes`; forward wrapper args in IR call |
| How `SpecializedLambda` stores the function pointer | `R(*)(Args...)` variadic function pointer, same RAII as `SpecializedFunction<R>` |
| Test file structure | Single-file lit test with `extern "C"` + `__asm__` kernel |
| Generic lambda support | Out of scope — excluded by spec assumption |
