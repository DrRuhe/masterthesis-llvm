# API Contract: Lambda Specialization API

**Namespace**: `clangRuntimeSpecializer`  
**Header**: `ClangRuntimeSpecializer.h`

---

## `SpecializedLambda<R, Args...>`

```cpp
template <class R, class... Args>
class SpecializedLambda {
public:
  // Default-constructed → falsy; calling operator() throws.
  SpecializedLambda() noexcept = default;

  // Move-only; copy is deleted.
  SpecializedLambda(SpecializedLambda&&) noexcept;
  SpecializedLambda& operator=(SpecializedLambda&&) noexcept;
  SpecializedLambda(const SpecializedLambda&) = delete;
  SpecializedLambda& operator=(const SpecializedLambda&) = delete;

  // Destructor releases the JITDylib (frees JIT-compiled machine code).
  ~SpecializedLambda();

  // Calls the JIT-compiled function with the given arguments.
  // Throws ClangRuntimeSpecializerError if the object is falsy (not initialized).
  R operator()(Args... args) const;

  // Alias for operator().
  R call(Args... args) const;

  // Returns true if the object holds a valid specialization.
  explicit operator bool() const noexcept;
};
```

**Guarantees**:
- `operator()` is not thread-safe (single-threaded usage only, per constitution).
- Destruction is noexcept; JITDylib teardown errors are swallowed (matching `SpecializedFunction<R>`).

---

## `specializeLambda<R>(funcName, lambda)`

```cpp
// Without explicit options — uses CurrentOptions of the singleton.
template <class R, class Lambda>
__attribute__((noinline))
auto specializeLambda(const char* funcName, Lambda& lambda)
    -> SpecializedLambda<R, explicit_arg_types_of_Lambda...>;

// With explicit options — per-call pipeline configuration.
template <class R, class Lambda>
__attribute__((noinline))
auto specializeLambda(const char* funcName, Lambda& lambda,
                      const ClangRuntimeSpecializer::Options& opts)
    -> SpecializedLambda<R, explicit_arg_types_of_Lambda...>;
```

**Preconditions**:
- `ClangRuntimeSpecializer::init()` must have been called (same as `specializeOnly`).
- `funcName` must name a function present in a registered IR blob.
- The kernel function's first parameter must accept a pointer to `Lambda`'s closure struct.
- The kernel's remaining parameters must match `Lambda::operator()`'s parameter types in order.
- The lambda object must remain alive for the lifetime of the returned `SpecializedLambda`.

**Postconditions**:
- Returns a truthy `SpecializedLambda<R, ExplicitArgTypes...>` on success.
- Returns a falsy (default-constructed) `SpecializedLambda` if the JIT times out
  (`opts.JITTimeoutSeconds > 0` and timeout elapsed).
- Throws `ClangRuntimeSpecializerDumpedIRError` if `funcName` is not found in any blob.
- Throws `ClangRuntimeSpecializerArgSerializationError` if the lambda's closure type
  cannot be serialized (e.g., a non-trivially-copyable non-pointer member).

**Type deduction**:

`ExplicitArgTypes...` is deduced from `Lambda::operator()` using `detail::LambdaTraits`.
Generic lambdas (`auto` parameters) are not supported; a static_assert fires.

---

## `assertSpecializedLambdaIsEquivalent<R>(funcName, lambda, inputs)`

```cpp
template <class R, class Lambda, class InputRange>
void assertSpecializedLambdaIsEquivalent(
    const char* funcName,
    Lambda& lambda,
    const InputRange& inputs);
```

`InputRange` is any range of `std::tuple<ExplicitArgTypes...>` elements.

**Behaviour**:
- Calls `specializeLambda<R>(funcName, lambda)` once.
- For each element `args` in `inputs`: calls `std::apply(lambda, args)` and
  `std::apply(spec, args)` and compares results with `==`.
- Throws `ClangRuntimeSpecializerChangesBehaviorError` on the first mismatch.
- Logs `INFO: assertSpecializedLambdaIsEquivalent passed for: {funcName}` on success.

**Constraints**: `R` must satisfy `HasEqualityOperator<R>` (or be `void`).

---

## Smoke Test Interface (FileCheck patterns)

Each smoke test file follows the pattern established by existing tests:

```cpp
// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
```

Minimum FileCheck patterns per test:

| Test | Required EXE: patterns |
|------|----------------------|
| `specialized-lambda-basic.cpp` | `INFO: Specializing call to: {funcName}`, result correctness assertion |
| `specialized-lambda-void.cpp` | `INFO: Specializing call to: {funcName}`, side-effect correctness |
| `specialized-lambda-no-captures.cpp` | `INFO: Specializing call to: {funcName}`, result correctness |
| `specialized-lambda-raii.cpp` | No crash output; loop completion message |
