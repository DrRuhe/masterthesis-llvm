#pragma once

#include <cstdarg>
#include <cstring>
#include <memory>
#include <string>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#define CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME "call_specialized_func_name"

// TODO refactor the error handling: create a special "ClangRuntimeSpecializationError" for this project.
namespace clangRuntimeSpecializer {

  class ClangRuntimeSpecializerError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };

  class ClangRuntimeSpecializerArgSerializationError : public ClangRuntimeSpecializerError {
  public:
    using ClangRuntimeSpecializerError::ClangRuntimeSpecializerError;
  };

  class ClangRuntimeSpecializerDumpedIRError : public ClangRuntimeSpecializerError {
  public:
    using ClangRuntimeSpecializerError::ClangRuntimeSpecializerError;
  };

  class ClangRuntimeSpecializerChangesBehaviorError : public ClangRuntimeSpecializerError {
  public:
    using ClangRuntimeSpecializerError::ClangRuntimeSpecializerError;
  };

  template <typename T, typename = void>
  struct HasEqualityOperator : std::false_type {};

  template <typename T>
  struct HasEqualityOperator<T, std::void_t<decltype(std::declval<const T&>() == std::declval<const T&>())>> : std::true_type {};

  template <typename... Args>
  constexpr bool allComparable() {
    return (HasEqualityOperator<std::decay_t<Args>>::value && ...);
  }

  class ClangRuntimeSpecializer {
  public:
    struct WriteBack {
        llvm::Value* Source; // Changed from GlobalVariable* GV
        void* OriginalPtr;
        uint64_t Size;
    };

    enum class LogLevel {
      None,
      Error,
      Info,
      Debug
    };

    static void setLogLevel(LogLevel Level);
    static LogLevel getLogLevel();

    struct InstructionCounts {
      uint64_t Total;
      uint64_t Loads;
      uint64_t Stores;
      uint64_t Calls;
      uint64_t Arith;
      uint64_t Cmp;
      uint64_t Branches;
      uint64_t Returns;
      uint64_t Other;
    };

    static void resetCounters();
    static InstructionCounts getCurrentCounters();
    static void printCounters();
    static void printComparisonTable(const char* funcName, const InstructionCounts& Before, const InstructionCounts& After);

    static void log(LogLevel Level, const char* FuncName, const llvm::Twine Message);
    static void log(LogLevel Level, const char* FuncName, const char* Message);

    template <typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback>>>
    static void log(LogLevel Level, const char* FuncName, Callback&& CB) {
      if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
          log(Level, FuncName, CB());
      }
    }

    template <typename T>
    static std::string printLLVM(T Val) {
      std::string S;
      llvm::raw_string_ostream OS(S);
      if (Val) {
        Val->print(OS);
      } else {
        OS << "nullptr";
      }
      return S;
    }
#define CRS_LOG(Level, Msg) clangRuntimeSpecializer::ClangRuntimeSpecializer::log(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Level, __FUNCTION__, Msg)


    struct Options {
      bool EnableInstructionInstrumentation = false;
      bool KeepDebugInfo = false;
      bool PrintFixpointIterations = false;
    };

    static ClangRuntimeSpecializer* init();
    ClangRuntimeSpecializer* enableInstructionInstrumentation() {
      CurrentOptions.EnableInstructionInstrumentation = true;
      return this;
    }




    bool isInstructionInstrumentationEnabled() const {
      return CurrentOptions.EnableInstructionInstrumentation;
    }

    ClangRuntimeSpecializer* setKeepDebugInfo(bool Keep) {
      CurrentOptions.KeepDebugInfo = Keep;
      return this;
    }

    bool shouldKeepDebugInfo() const {
        return CurrentOptions.KeepDebugInfo;
    }

    ClangRuntimeSpecializer* printFixpointIterations() {
        CurrentOptions.PrintFixpointIterations = true;
        return this;
    }

      bool printsFixpointIterations() const {
        return CurrentOptions.PrintFixpointIterations;
    }

    Options& getOptions() { return CurrentOptions; }
    const Options& getOptions() const { return CurrentOptions; }


    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R callSpecialized(ARGS&&... Args) {
      return callImpl<funcName, R, false, true>(__FUNCTION__, std::forward<ARGS>(Args)...);
    }

    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R call_instrumented(ARGS&&... Args) {
      return callImpl<funcName, R, true, true>(__FUNCTION__, std::forward<ARGS>(Args)...);
    }

    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R call_instrumented_baseline(ARGS&&... Args) {
      return callImpl<funcName, R, true, false>(__FUNCTION__, std::forward<ARGS>(Args)...);
    }

    ~ClangRuntimeSpecializer();

  private:

    template <const char* funcName, class R, bool Instrument,  bool Optimize = true, class... ARGS>
    R callImpl(const char* CallerName, ARGS&&... Args) {
      checkInitialization(funcName);
      llvm::Function *TargetFunc = getTargetFunction(funcName);
      llvm::CallBase* CallSite = findCallSpecializedFunctionInModule(CallerName, funcName);
      validateArgs(TargetFunc, CallSite, sizeof...(ARGS));

      if constexpr (Instrument)
      {
          log(LogLevel::Info, CallerName, (llvm::Twine("Instrumenting call to: ") + funcName).str());
      }
      else
      {
          log(LogLevel::Info, CallerName, (llvm::Twine("Specializing call to: ") + funcName).str());
      }
      // TODO reuse a clean copy of the llvm module. Possibly perform llvm::CloneModule(*Module) and then add the specialization wrapper to the copied module only.
      //   Currently we get conflicts, so specialization fails.
      std::string UniqueWrapperName = createUniqueWrapperName() + (Optimize ? "" : "_no_opt");
      auto NewModule = llvm::CloneModule(*Module);
      auto* TargetFuncInNewModule = NewModule->getFunction(TargetFunc->getName());
      encourageInlining(TargetFuncInNewModule);

      std::vector<llvm::Constant*> ArgConstants;
      std::vector<WriteBack> WriteBacks;

      auto SerializeArgs = [&](auto&&... ArgsInner) {
          unsigned I = 0;
          ([&] {
              llvm::Argument* IrArg = TargetFunc->getArg(I);
              ArgConstants.push_back(serializeArgumentToIR(*NewModule, IrArg, std::forward<decltype(ArgsInner)>(ArgsInner), WriteBacks));
              I++;
          }(), ...);
      };
      SerializeArgs(std::forward<ARGS>(Args)...);

      for (llvm::Value* Arg : ArgConstants)
      {
          log(LogLevel::Debug, CallerName, (llvm::Twine("Arg Serialized to: ") + printLLVM(Arg)).str());
      }

      buildWrapperIR(*NewModule, UniqueWrapperName, TargetFuncInNewModule, ArgConstants, WriteBacks, Instrument,  Optimize);

      prepareModuleForJIT(*NewModule, UniqueWrapperName);

      auto TSM = llvm::orc::ThreadSafeModule(std::move(NewModule),
                                             llvm::orc::ThreadSafeContext(std::make_unique<llvm::LLVMContext>()));

      // Export serialized argument addresses to the JIT if they are external (for baseline runs).
      {
          auto &JD = JIT->getMainJITDylib();
          llvm::orc::SymbolMap Symbols;
          for (const auto& WB : WriteBacks) {
              if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(WB.Source)) {
                  if (GV->hasExternalLinkage()) {
                      Symbols[JIT->mangleAndIntern(GV->getName())] = { llvm::orc::ExecutorAddr::fromPtr(WB.OriginalPtr), llvm::JITSymbolFlags::Exported };
                  }
              }
          }
          if (!Symbols.empty()) {
              cantFail(JD.define(llvm::orc::absoluteSymbols(Symbols)));
          }
      }

      uintptr_t Addr = addModuleAndLookup(std::move(TSM), UniqueWrapperName);

        auto SpecializedFnPtr = reinterpret_cast<R(*)()>(Addr);
        if constexpr (std::is_void_v<R>) {
            SpecializedFnPtr();
            return;
        } else {
            return SpecializedFnPtr();
        }
    }

    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> Module;
    std::unique_ptr<llvm::orc::LLJIT> JIT;
    uint64_t GlobalSpecializationCount = 0;
    Options CurrentOptions;
    std::vector<std::unique_ptr<char[]>> SerializationBuffers;

    void checkInitialization(const char* funcName) const;
    llvm::Function* getTargetFunction(const char* funcName) const;
    void validateArgs(llvm::Function* TargetFunc, llvm::CallBase* CallSite, size_t NumArgs) const;
    std::string createUniqueWrapperName();
    void prepareModuleForJIT(llvm::Module& M, const std::string& WrapperName);
    uintptr_t addModuleAndLookup(llvm::orc::ThreadSafeModule TSM, const std::string& WrapperName);
    void encourageInlining(llvm::Function* F);
    llvm::Function* buildWrapperIR(llvm::Module& M, const std::string& WrapperName, llvm::Function* TargetFunc,
                                   llvm::ArrayRef<llvm::Constant*> SpecializedArgs, llvm::ArrayRef<WriteBack> WriteBacks,
                                   bool ForceInstrument, bool Optimize = true);

    explicit ClangRuntimeSpecializer();

    llvm::CallBase* findCallSpecializedFunctionInModule(const char* FunctionName, const char* UID) const;

    // Helper to identify concrete type of a polymorphic object from its vtable pointer
    llvm::StructType* identifyPolymorphicType(llvm::Module& M, const void* ObjectPtr);

    // Recursively serialize a value of a given LLVM type from a memory location.
    llvm::Constant* serializeValueToIR(llvm::Module& M, llvm::Type* Type, const void* ValuePtr);

    template <class T>
    llvm::Constant* serializeArgumentToIR(llvm::Module& M, llvm::Argument* IrArg, T&& Value, std::vector<WriteBack>& WriteBacks) {
      using Decayed = std::decay_t<T>;
      llvm::Type* ExpectedType = IrArg ? IrArg->getType() : nullptr;
      if (!ExpectedType) {
          throw ClangRuntimeSpecializerArgSerializationError("expectedType was null during serialization.");
      }

      CRS_LOG(Debug,[&] {
          return (llvm::Twine("Inferred type for serialized argument: ") + printLLVM(ExpectedType)).str();
      });

      using ElementType = std::conditional_t<std::is_pointer_v<Decayed>, std::remove_pointer_t<Decayed>, Decayed>;

      // Handle polymorphic types by reconstructing them with their concrete type
      if constexpr (std::is_polymorphic_v<ElementType>) {
          const void* ObjectPtr = nullptr;
          if constexpr (std::is_pointer_v<Decayed>) {
              ObjectPtr = Value;
          } else {
              ObjectPtr = &Value;
          }

          if (!ObjectPtr) {
              // Null pointer - serialize as null
              return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ExpectedType));
          }

          CRS_LOG(Debug,[&] {
              return "Serializing polymorphic type - identifying concrete type";
          });

          // Identify the concrete type from the vtable pointer
          llvm::StructType* ConcreteType = identifyPolymorphicType(M, ObjectPtr);

          if (!ConcreteType) {
              CRS_LOG(Debug,[&] {
                  return "Could not identify concrete type - falling back to opaque pointer";
              });
              // Fall back to pointer serialization
              return serializeValueToIR(M, ExpectedType, &Value);
          }

          CRS_LOG(Debug,[&] {
              return (llvm::Twine("Serializing polymorphic object with concrete type: ") + printLLVM(ConcreteType)).str();
          });

          // Serialize the object with its concrete type (recursively handles polymorphic members)
          llvm::Constant* SerializedObject = serializeValueToIR(M, ConcreteType, ObjectPtr);

          if (!SerializedObject) {
              throw ClangRuntimeSpecializerArgSerializationError("Failed to serialize polymorphic object");
          }

          // Create a global variable with the serialized object
          auto *GV = new llvm::GlobalVariable(M, ConcreteType, true, // isConstant = true
                                              llvm::GlobalValue::InternalLinkage,
                                              SerializedObject, "__specialization_global_polymorphic_object");

          // Return a pointer to the global, bitcast if necessary
          if (ExpectedType->isPointerTy()) {
              return llvm::ConstantExpr::getBitCast(GV, ExpectedType);
          } else {
              return GV;
          }
      }

      if constexpr ((std::is_class_v<ElementType> || std::is_union_v<ElementType>) && !std::is_polymorphic_v<ElementType>) {
          struct DumpContext {
              ClangRuntimeSpecializer* Self;
              llvm::Module& Module;
              llvm::Type* ExpectedType;
              llvm::Type* PreciseType = nullptr;
              
              struct Frame {
                  llvm::Type* Type;
                  std::vector<llvm::Constant*> Elements;
                  unsigned NextElemIdx = 0;
              };
              std::vector<Frame> Stack;
              llvm::Constant* Result = nullptr;
              bool LastWasFieldHeader = false;

              DumpContext(ClangRuntimeSpecializer* ClangRuntimeSpecializer, llvm::Module& M, llvm::Type* Type)
                  : Self(ClangRuntimeSpecializer), Module(M), ExpectedType(Type) {}

              void handleTypeName(const char* Name) {
                  if (PreciseType) return;
                  std::string N = Name;
                  // Strip "struct " or "class " prefix if present in the dump name
                  if (N.compare(0, 7, "struct ") == 0) N = N.substr(7);
                  else if (N.compare(0, 6, "class ") == 0) N = N.substr(6);

                  auto& Ctx = Module.getContext();
                  if (auto* StructTy = llvm::StructType::getTypeByName(Ctx, "struct." + N)) PreciseType = StructTy;
                  else if (auto* ClassTy = llvm::StructType::getTypeByName(Ctx, "class." + N)) PreciseType = ClassTy;
                  else if (auto* Ty = llvm::StructType::getTypeByName(Ctx, N)) PreciseType = Ty;
                  
                  if (!PreciseType) {
                      if (ExpectedType->isStructTy()) PreciseType = ExpectedType;
                  }

                  if (PreciseType) {
                      CRS_LOG(Debug,[&] {
                          return (llvm::Twine("Discovered precise type: ") + printLLVM(PreciseType) + " for " + Name).str();
                      });
                      if (Stack.empty()) {
                          Stack.push_back({PreciseType, {}, 0});
                      }
                  }
              }

              void pushFrame() {
                  if (Stack.empty()) return;
                  auto& Top = Stack.back();
                  skipPadding(Top);
                  llvm::Type* NextTy = nullptr;
                  if (auto* STy = llvm::dyn_cast<llvm::StructType>(Top.Type)) {
                      if (Top.NextElemIdx < STy->getNumElements())
                          NextTy = STy->getElementType(Top.NextElemIdx);
                  } else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(Top.Type)) {
                      NextTy = ATy->getElementType();
                  }
                  if (NextTy) Stack.push_back({NextTy, {}, 0});
              }

              void popFrame() {
                  if (Stack.empty()) return;
                  Frame F = std::move(Stack.back());
                  Stack.pop_back();

                  llvm::Constant* C = nullptr;
                  if (auto* STy = llvm::dyn_cast<llvm::StructType>(F.Type)) {
                      while (F.Elements.size() < STy->getNumElements()) {
                          F.Elements.push_back(llvm::UndefValue::get(STy->getElementType(F.Elements.size())));
                      }
                      C = llvm::ConstantStruct::get(STy, F.Elements);
                  } else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(F.Type)) {
                      C = llvm::ConstantArray::get(ATy, F.Elements);
                  }

                  if (Stack.empty()) Result = C;
                  else {
                      Stack.back().Elements.push_back(C);
                      ++Stack.back().NextElemIdx;
                  }
              }

              void skipPadding(Frame& F) {
                  if (auto* STy = llvm::dyn_cast<llvm::StructType>(F.Type)) {
                      while (F.NextElemIdx < STy->getNumElements()) {
                          llvm::Type* ETy = STy->getElementType(F.NextElemIdx);
                          // Heuristic: padding is often anonymous [N x i8]
                          if (ETy->isArrayTy() && ETy->getArrayElementType()->isIntegerTy(8)) {
                              F.Elements.push_back(llvm::UndefValue::get(ETy));
                              ++F.NextElemIdx;
                          } else break;
                      }
                  }
              }

              void addConstant(llvm::Constant* C) {
                  if (Stack.empty()) { Result = C; return; }
                  auto& Top = Stack.back();
                  skipPadding(Top);
                  Top.Elements.push_back(C);
                  Top.NextElemIdx++;
              }
          };

          DumpContext Ctx{this, M, ExpectedType};
          auto Callback = [](void* Context, const char* Fmt, ...) -> int {
              auto* C = static_cast<DumpContext*>(Context);
              va_list Args;
              va_start(Args, Fmt);
              if (std::strcmp(Fmt, "%s") == 0) {
                  C->handleTypeName(va_arg(Args, char*));
              } else if (std::strcmp(Fmt, " {\n") == 0) {
                  if (C->LastWasFieldHeader) C->pushFrame();
              } else if (std::strcmp(Fmt, "}\n") == 0 || std::strcmp(Fmt, "%s}\n") == 0) {
                  C->popFrame();
              } else if (std::strstr(Fmt, "=")) {
                  va_arg(Args, char*); // indent
                  va_arg(Args, char*); // type
                  va_arg(Args, char*); // name
                  int Specifiers = 0;
                  for (const char* P = Fmt; *P; ++P) if (*P == '%') Specifiers++;
                  
                  if (Specifiers >= 4) {
                      C->LastWasFieldHeader = false;
                      // Primitive or pointer leaf
                      if (C->Stack.empty()) { va_end(Args); return 0; }
                      auto& Top = C->Stack.back();
                      C->skipPadding(Top);
                      if (Top.NextElemIdx >= (llvm::isa<llvm::StructType>(Top.Type) ? llvm::cast<llvm::StructType>(Top.Type)->getNumElements() : 0xFFFFFFFF)) {
                          va_end(Args); return 0;
                      }
                      llvm::Type* ETy = nullptr;
                      if (auto* STy = llvm::dyn_cast<llvm::StructType>(Top.Type)) ETy = STy->getElementType(Top.NextElemIdx);
                      else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(Top.Type)) ETy = ATy->getElementType();

                      if (ETy) {
                          if (std::strstr(Fmt, "%d") || std::strstr(Fmt, "%u") || std::strstr(Fmt, "%x")) {
                              if (ETy->isIntegerTy(64)) C->addConstant(llvm::ConstantInt::get(ETy, va_arg(Args, long long)));
                              else C->addConstant(llvm::ConstantInt::get(ETy, va_arg(Args, int)));
                          } else if (std::strstr(Fmt, "%f")) {
                              C->addConstant(llvm::ConstantFP::get(ETy, va_arg(Args, double)));
                          } else if (std::strstr(Fmt, "%p") || std::strstr(Fmt, "%.32s")) {
                              void* Ptr = va_arg(Args, void*);
                              if (ETy->isPointerTy()) {
                                  uintptr_t Val = reinterpret_cast<uintptr_t>(Ptr);
                                  C->addConstant(llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(llvm::Type::getInt64Ty(C->Module.getContext()), Val), ETy));
                              } else if (ETy->isArrayTy()) {
                                  // Nested array - use fallback
                                  C->addConstant(C->Self->serializeValueToIR(C->Module, ETy, Ptr));
                              }
                          }
                      }
                  } else {
                      C->LastWasFieldHeader = true;
                  }
              }
              va_end(Args);
              return 0;
          };

          if constexpr (std::is_pointer_v<Decayed>) {
              if (!Value) return serializeValueToIR(M, ExpectedType, &Value);
              __builtin_dump_struct(Value, Callback, &Ctx);
          } else {
              __builtin_dump_struct(&Value, Callback, &Ctx);
          }

          if (Ctx.Result) {
              if (ExpectedType->isPointerTy()) {
                  // Determine if we need write-back.
                  // If the original was a pointer/reference, we should write back modifications to it.
                  // We only do this if it's not a byval argument.
                  bool IsByVal = IrArg && IrArg->hasByValAttr();
                  bool ShouldWriteBack = !IsByVal;

                  auto *GV = new llvm::GlobalVariable(M, Ctx.Result->getType(), true, // isConstant = true
                                                      llvm::GlobalValue::InternalLinkage, Ctx.Result, "__specialization_global_specialized_instance");
                  
                  if (ShouldWriteBack) {
                      void* Ptr = nullptr;
                      if constexpr (std::is_pointer_v<Decayed>) {
                          Ptr = reinterpret_cast<void*>(Value);
                      } else {
                          Ptr = const_cast<void*>(static_cast<const void*>(&Value));
                      }
                      if (Ptr) {
                          WriteBacks.push_back({GV, Ptr, M.getDataLayout().getTypeStoreSize(Ctx.Result->getType())});
                      }
                  }
                  return llvm::ConstantExpr::getBitCast(GV, ExpectedType);
              }
              return Ctx.Result;
          }
      }

      return serializeValueToIR(M, ExpectedType, &Value);
    }
  };

  template <const char* funcName, class MemFn, class OBJ, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeMethodOrFallback(MemFn MF, OBJ&& Obj, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return (std::forward<OBJ>(Obj).*MF)(std::forward<ARGS>(Args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(Invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<funcName,void>(std::forward<OBJ>(Obj), std::forward<ARGS>(Args)...);
          return;
        } else {
          return RS->callSpecialized<funcName,R>( std::forward<OBJ>(Obj), std::forward<ARGS>(Args)...);
        }
      }
    } catch (const std::exception& E) {
      CRS_LOG(Error, (llvm::Twine("Specialization failed: ") + E.what()).str());
    } catch (...) {
      CRS_LOG(Error, "Specialization failed with an unknown error.");
    }

    if constexpr (std::is_void_v<decltype(Invoke())>) {
      Invoke();
      return;
    } else {
      return Invoke();
    }
  }

  template <const char* funcName, class Fn, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeFunctionOrFallback(Fn F, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return F(std::forward<ARGS>(Args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(Invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<funcName, void>(std::forward<ARGS>(Args)...);
          return;
        } else {
          return RS->callSpecialized<funcName, R>(std::forward<ARGS>(Args)...);
        }

      }
    } catch (const std::exception& E) {
      CRS_LOG(Error, (llvm::Twine("Specialization failed: ") + E.what()).str());
    } catch (...) {
      CRS_LOG(Error, "Specialization failed with an unknown error.");
    }

    if constexpr (std::is_void_v<decltype(Invoke())>) {
      Invoke();
      return;
    } else {
      return Invoke();
    }
  }

  template <const char* funcName, class Fn, class... ARGS>
  __attribute__((always_inline))
  void assertSpecializedFunctionIsEquivalent( Fn F, ARGS... Args) {
    static_assert(allComparable<ARGS...>(),
        "All arguments passed to assertSpecializedFunctionIsEquivalent must support the equality operator (==). "
        "This is required to ensure that the specialized function behavior matches the original when arguments are modified.");

    auto* RS = ClangRuntimeSpecializer::init();

    // Copy arguments for both calls
    auto ArgsOrig = std::make_tuple(Args...);
    auto ArgsSpec = std::make_tuple(Args...);

    using R = decltype(F(Args...));

    if constexpr (std::is_void_v<R>) {
        // Call original
        std::apply(F, ArgsOrig);
        // Call specialized
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<funcName, void>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);

        // Compare modified arguments (if they were passed by reference/pointer)
        if constexpr (allComparable<ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: arguments differ after execution of ") + funcName).str());
            }
        }
    } else {
        // Call original
        R ResOrig = std::apply(F, ArgsOrig);
        // Call specialized
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return RS->callSpecialized<funcName, R>( std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);

        // Compare return values
        if constexpr (HasEqualityOperator<R>::value) {
            if (ResOrig != ResSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: return values differ for ") + funcName).str());
            }
        }

        // Compare modified arguments
        if constexpr (allComparable<ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: arguments differ after execution of ") + funcName).str());
            }
        }
    }
    CRS_LOG(Info, (llvm::Twine("Successfully specialized ") + funcName + "! No differences could be observed.").str());
  }


  template <const char* funcName, class MemFn, class OBJ, class... ARGS>
  __attribute__((always_inline))
  void assertSpecializedMethodIsEquivalent(MemFn Mf, OBJ Obj, ARGS... Args) {
    static_assert(allComparable<ARGS...>(),
        "All arguments passed to assertSpecializedMethodIsEquivalent must support the equality operator (==). "
        "This is required to ensure that the specialized method behavior matches the original when arguments are modified.");

    auto* RS = ClangRuntimeSpecializer::init();

    // Copy object + arguments for both calls
    auto ArgsOrig = std::forward_as_tuple(Obj, Args...);
    auto ArgsSpec = std::make_tuple(Obj, Args...);


    using R = decltype(std::invoke(Mf, Obj, Args...));

    if constexpr (std::is_void_v<R>) {
        // Call original
        std::apply([&](auto&& ObjArg, auto&&... CallArgs) {
            std::invoke(Mf, std::forward<decltype(ObjArg)>(ObjArg), std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsOrig);

        // Call specialized
        std::apply([&](auto&& ObjArg, auto&&... CallArgs) {
            RS->callSpecialized<funcName, void>(
              std::forward<decltype(ObjArg)>(ObjArg),
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);

        if constexpr (HasEqualityOperator<OBJ>::value && allComparable<ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: object/args differ after execution of ") + funcName).str());
            }
        }
    } else {
        // Call original
        R ResOrig = std::apply([&](auto&& ObjArg, auto&&... CallArgs) -> R {
            return std::invoke(Mf, std::forward<decltype(ObjArg)>(ObjArg), std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsOrig);

        // Call specialized
        R ResSpec = std::apply([&](auto&& ObjArg, auto&&... CallArgs) -> R {
            return RS->callSpecialized<funcName, R>(
              std::forward<decltype(ObjArg)>(ObjArg),
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);

        if constexpr (HasEqualityOperator<R>::value) {
            if (ResOrig != ResSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: return values differ for ") + funcName).str());
            }
        }

        if constexpr (HasEqualityOperator<OBJ>::value && allComparable<ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: object/args differ after execution of ") + funcName).str());
            }
        }
    }
    CRS_LOG(Info, (llvm::Twine("Successfully specialized ") + funcName + "! No differences could be observed.").str());
  }

  template <const char* funcName, class Fn, class... ARGS>
  __attribute__((always_inline))
  void compareFunctionInstructionCounts(Fn F, ARGS... Args) {
    auto* RS = ClangRuntimeSpecializer::init()->enableInstructionInstrumentation();

    // Copy arguments for both calls to ensure same initial state
    auto ArgsOrig = std::make_tuple(Args...);
    auto ArgsSpec = std::make_tuple(Args...);

    using R = decltype(F(Args...));

    ClangRuntimeSpecializer::InstructionCounts Before, After;

    // Call baseline instrumented
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented_baseline<funcName, void>(std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    } else {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented_baseline<funcName, R>(std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    }
    Before = RS->getCurrentCounters();

    // Call specialized instrumented
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented<funcName, void>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    } else {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented<funcName, R>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    }
    After = RS->getCurrentCounters();

    ClangRuntimeSpecializer::printComparisonTable(funcName, Before, After);
  }

} // namespace clangRuntimeSpecializer
