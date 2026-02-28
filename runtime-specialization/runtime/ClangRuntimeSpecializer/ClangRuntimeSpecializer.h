#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>
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
#include "llvm/Transforms/Utils/Cloning.h"

#define CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME "call_specialized_func_name"

// TODO refactor the error handling: create a special "ClangRuntimeSpecializationError" for this project.
namespace clangRuntimeSpecializer {

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
    static ClangRuntimeSpecializer* init();
    
    //TODO extract the non generic logic into the cpp file. So still perform the generic arg serialization 
    // in this header, but then call a method implemented in the cpp file.
    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R callSpecialized(ARGS&&... Args) {
      if (funcName == nullptr) {
        throw std::runtime_error("[ClangRuntimeSpecializer] funcName was null! ");
      }
      if (Module == nullptr) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Module was null! ");
      }
      if (!JIT) {
        throw std::runtime_error("[ClangRuntimeSpecializer] JIT was not initialized!");
      }

      std::string FuncNameStr(funcName);

      if (!FuncNameStr.empty() && FuncNameStr.front() == '&') {
        FuncNameStr.erase(0, 1);
      }
      llvm::Function *TargetFunc = Module->getFunction(FuncNameStr);
      if (TargetFunc == nullptr) {
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Could not find function: ") + funcName + " (It might be optimized out already by dead-code-elimination?)");
      }

      // Try to find the call site of call_specialized in the IR to get more precise type information.
      llvm::CallBase* CallSite = findCallSpecializedFunctionInModule(__FUNCTION__,funcName);

      // Ensure the number of args between the callsite in the IR, the function to specialize and the number of passed args are compatible.
      size_t NumArgs = sizeof...(ARGS);
      
      // Determine how many leading arguments to skip at the callsite:
      // - Always skip the implicit 'this' pointer (1)
      // - Additionally skip the explicit 'funcName' runtime argument if present (legacy path)
      unsigned CallArgCount = CallSite->arg_size();

      unsigned SkipArgs = 1;
      if (CallArgCount != SkipArgs + static_cast<unsigned>(NumArgs))
      {
        std::fprintf(stderr,
          "[ClangRuntimeSpecializer] Unexpected callsite arg count. callArgCount=%u, numArgs=%zu (expected %u)\n",
          CallArgCount, NumArgs, static_cast<unsigned>(NumArgs) + 1);
        throw std::runtime_error("[ClangRuntimeSpecializer] Unexpected callsite arg count");
      }


      if (NumArgs != TargetFunc->arg_size())
      {
        std::fprintf(
          stderr,
          "[ClangRuntimeSpecializer] The number of args are incompatible! numArgs: %zu, TargetFunc->arg_size(): %zu\n",
          NumArgs, TargetFunc->arg_size());
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] The number of args are incompatible! "));
      }

      // TODO reuse a clean copy of the llvm module. Possibly perform llvm::CloneModule(*Module) and then add the specialization wrapper to the copied module only.
      //   Currently we get conflicts, so specialization fails.
      std::string UniqueWrapperName = "specialized_wrapper_" + std::to_string(++GlobalSpecializationCount) + "_" + std::to_string(reinterpret_cast<uintptr_t>(this));

      // Clone the module first to avoid polluting the main Module with multiple wrappers.
      auto NewModule = llvm::CloneModule(*Module);
      
      // Create a new function that takes no arguments and, inside, calls the target with serialized constants.
      llvm::IRBuilder<> Builder{Context};
      llvm::FunctionType* FTy = llvm::FunctionType::get(TargetFunc->getReturnType(), false);
      llvm::Function* NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, UniqueWrapperName, *NewModule);

      llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", NewFunc);
      Builder.SetInsertPoint(Entry);

      // Serialize the runtime arguments to IR constants and create a call to the target function with them.
      // We pass the TargetFunc's arguments to guide serialization.
      auto* TargetFuncInNewModule = NewModule->getFunction(TargetFunc->getName());
      
      std::vector<llvm::Value*> ArgValues;

      //llvm::errs() << "[ClangRuntimeSpecializer] Function to specialize:\n";
      //TargetFunc->print(llvm::errs());
      //TODO iterate through TargetFunc->uses()


      auto SerializeArgs = [&](auto&&... ArgsInner) {
          unsigned I = 0;
          ([&] {
              llvm::Argument* IrArg = TargetFunc->getArg(I);
              ArgValues.push_back(serializeArgumentToIR(Builder, IrArg, std::forward<decltype(ArgsInner)>(ArgsInner)));
              I++;
          }(), ...);
      };
      SerializeArgs(std::forward<ARGS>(Args)...);


      for (llvm::Value* Arg : ArgValues)
      {
          std::fprintf(stderr, "[ClangRuntimeSpecializer] Arg Serialized to: ");
          Arg->print(llvm::errs());
          std::fprintf(stderr, "\n");
      }

      // Encourage inlining for the callee in the JIT pipeline.
      TargetFuncInNewModule->removeFnAttr(llvm::Attribute::NoInline);
      TargetFuncInNewModule->removeFnAttr(llvm::Attribute::OptimizeNone);
      TargetFuncInNewModule->addFnAttr(llvm::Attribute::AlwaysInline);

      auto *CallInst = Builder.CreateCall(TargetFuncInNewModule->getFunctionType(), TargetFuncInNewModule, ArgValues);
      CallInst->setAttributes(TargetFuncInNewModule->getAttributes());
      CallInst->addFnAttr(llvm::Attribute::AlwaysInline);

      if (TargetFuncInNewModule->getReturnType()->isVoidTy()) {
        Builder.CreateRetVoid();
      } else {
        Builder.CreateRet(CallInst);
      }

      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specializing call to: %s\n", funcName);
      // Print the new function to stderr as requested.
      NewFunc->print(llvm::errs());
      llvm::errs() << "\n";

      // Every function except the specialized wrapper should have available_externally linkage
      // if it has a definition. This allows the JIT inliner to see the bodies but won't
      // produce a definition in the resulting object file, as we want to use the host's version
      // if it's not inlined.
      for (auto &F : *NewModule) {
        if (F.getName() == UniqueWrapperName) {
           F.setLinkage(llvm::GlobalValue::ExternalLinkage);
           continue;
        }
        if (!F.isDeclaration()) {
           F.setLinkage(llvm::GlobalValue::AvailableExternallyLinkage);
        }
      }
      
      // Also convert global variables to available_externally or declarations.
      // Special care for constant strings and other internal globals.
      for (auto &G : NewModule->globals()) {
        if (!G.isDeclaration()) {
          // If it's a constant string or similar internal, we might want to keep it
          // as private/internal if we can't find it in the host.
          // However, available_externally for globals usually works if they are
          // indeed available. For JIT, internal globals might NOT be available.
          if (G.hasInternalLinkage() || G.hasPrivateLinkage()) {
            continue; // Keep internal/private globals as is.
          }
          G.setLinkage(llvm::GlobalValue::AvailableExternallyLinkage);
        }
      }

      auto TSM = llvm::orc::ThreadSafeModule(std::move(NewModule),
                                             llvm::orc::ThreadSafeContext(std::make_unique<llvm::LLVMContext>()));

      if (auto Err = JIT->addIRModule(std::move(TSM))) {
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Failed to add module to JIT: ") + llvm::toString(std::move(Err)));
      }

      auto SpecializedFn = JIT->lookup(UniqueWrapperName);
      if (!SpecializedFn) {
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Failed to lookup wrapper: ") + llvm::toString(SpecializedFn.takeError()));
      }

      auto SpecializedFnPtr = SpecializedFn->toPtr<R()>();

      if constexpr (std::is_void_v<R>) {
        SpecializedFnPtr();
        return;
      } else {
        return SpecializedFnPtr();
      }
    }

    ~ClangRuntimeSpecializer();
  private:

    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> Module;
    std::unique_ptr<llvm::orc::LLJIT> JIT;
    uint64_t GlobalSpecializationCount = 0;
    explicit ClangRuntimeSpecializer();

    llvm::CallBase* findCallSpecializedFunctionInModule(const char* FunctionName, const char* UID) const;

    // Recursively serialize a value of a given LLVM type from a memory location.
    llvm::Value* serializeValueToIR(llvm::IRBuilder<>& Builder, llvm::Type* Type, const void* ValuePtr);

    template <class T>
    llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& Builder, llvm::Argument* IrArg, T&& Value) {
      using Decayed = std::decay_t<T>;
      llvm::Type* ExpectedType = IrArg ? IrArg->getType() : nullptr;
      if (!ExpectedType) {
          throw std::runtime_error("[ClangRuntimeSpecializer] expectedType was null during serialization.");
      }

      using ElementType = std::conditional_t<std::is_pointer_v<Decayed>, std::remove_pointer_t<Decayed>, Decayed>;
      
      if constexpr (std::is_class_v<ElementType> || std::is_union_v<ElementType>) {
          struct DumpContext {
              ClangRuntimeSpecializer* Self;
              llvm::IRBuilder<>& Builder;
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

              DumpContext(ClangRuntimeSpecializer* ClangRuntimeSpecializer, llvm::IRBuilder<>& IrBuilder, llvm::Type* Type)
                  : Self(ClangRuntimeSpecializer), Builder(IrBuilder), ExpectedType(Type) {}

              void handleTypeName(const char* Name) {
                  if (PreciseType) return;
                  std::string N = Name;
                  // Strip "struct " or "class " prefix if present in the dump name
                  if (N.compare(0, 7, "struct ") == 0) N = N.substr(7);
                  else if (N.compare(0, 6, "class ") == 0) N = N.substr(6);

                  auto& Ctx = Self->Module->getContext();
                  if (auto* StructTy = llvm::StructType::getTypeByName(Ctx, "struct." + N)) PreciseType = StructTy;
                  else if (auto* ClassTy = llvm::StructType::getTypeByName(Ctx, "class." + N)) PreciseType = ClassTy;
                  else if (auto* Ty = llvm::StructType::getTypeByName(Ctx, N)) PreciseType = Ty;
                  
                  if (!PreciseType) {
                      if (ExpectedType->isStructTy()) PreciseType = ExpectedType;
                      else if (ExpectedType->isPointerTy()) {
                          // Try to find if there's a byval type or similar.
                          // For now, if we can't find it by name, we might be in trouble
                          // if the expectedType is just i8*.
                      }
                  }

                  if (PreciseType && Stack.empty()) {
                      Stack.push_back({PreciseType, {}, 0});
                  }
              }

              void pushFrame() {
                  if (Stack.empty()) return;
                  auto& Top = Stack.back();
                  skipPadding(Top);
                  llvm::Type* NextTy = nullptr;
                  if (auto* STy = llvm::dyn_cast<llvm::StructType>(Top.type)) {
                      if (Top.nextElemIdx < STy->getNumElements())
                          NextTy = STy->getElementType(Top.nextElemIdx);
                  } else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(Top.type)) {
                      NextTy = ATy->getElementType();
                  }
                  if (NextTy) Stack.push_back({NextTy, {}, 0});
              }

              void popFrame() {
                  if (Stack.empty()) return;
                  Frame F = std::move(Stack.back());
                  Stack.pop_back();

                  llvm::Constant* C = nullptr;
                  if (auto* STy = llvm::dyn_cast<llvm::StructType>(F.type)) {
                      while (F.elements.size() < STy->getNumElements()) {
                          F.elements.push_back(llvm::UndefValue::get(STy->getElementType(F.elements.size())));
                      }
                      C = llvm::ConstantStruct::get(STy, F.elements);
                  } else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(F.type)) {
                      C = llvm::ConstantArray::get(ATy, F.elements);
                  }

                  if (Stack.empty()) Result = C;
                  else {
                      Stack.back().elements.push_back(C);
                      ++Stack.back().nextElemIdx;
                  }
              }

              void skipPadding(Frame& F) {
                  if (auto* STy = llvm::dyn_cast<llvm::StructType>(F.type)) {
                      while (F.nextElemIdx < STy->getNumElements()) {
                          llvm::Type* ETy = STy->getElementType(F.nextElemIdx);
                          // Heuristic: padding is often anonymous [N x i8]
                          if (ETy->isArrayTy() && ETy->getArrayElementType()->isIntegerTy(8)) {
                              F.elements.push_back(llvm::UndefValue::get(ETy));
                              ++F.nextElemIdx;
                          } else break;
                      }
                  }
              }

              void addConstant(llvm::Constant* C) {
                  if (Stack.empty()) { Result = C; return; }
                  auto& Top = Stack.back();
                  skipPadding(Top);
                  Top.elements.push_back(C);
                  Top.nextElemIdx++;
              }
          };

          DumpContext Ctx{this, Builder, ExpectedType};
          auto Callback = [](void* Context, const char* Fmt, ...) -> int {
              auto* C = static_cast<DumpContext*>(Context);
              va_list Args;
              va_start(Args, Fmt);
              if (std::strcmp(Fmt, "%s") == 0) {
                  C->handleTypeName(va_arg(Args, char*));
              } else if (std::strcmp(Fmt, " {\n") == 0) {
                  if (C->lastWasFieldHeader) C->pushFrame();
              } else if (std::strcmp(Fmt, "}\n") == 0 || std::strcmp(Fmt, "%s}\n") == 0) {
                  C->popFrame();
              } else if (std::strstr(Fmt, "=")) {
                  va_arg(Args, char*); // indent
                  va_arg(Args, char*); // type
                  va_arg(Args, char*); // name
                  int Specifiers = 0;
                  for (const char* P = Fmt; *P; ++P) if (*P == '%') Specifiers++;
                  
                  if (Specifiers >= 4) {
                      C->lastWasFieldHeader = false;
                      // Primitive or pointer leaf
                      if (C->stack.empty()) { va_end(Args); return 0; }
                      auto& Top = C->stack.back();
                      C->skipPadding(Top);
                      if (Top.nextElemIdx >= (llvm::isa<llvm::StructType>(Top.type) ? llvm::cast<llvm::StructType>(Top.type)->getNumElements() : 0xFFFFFFFF)) {
                          va_end(Args); return 0;
                      }
                      llvm::Type* ETy = nullptr;
                      if (auto* STy = llvm::dyn_cast<llvm::StructType>(Top.type)) ETy = STy->getElementType(Top.nextElemIdx);
                      else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(Top.type)) ETy = ATy->getElementType();

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
                                  C->addConstant(llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(llvm::Type::getInt64Ty(C->builder.getContext()), Val), ETy));
                              } else if (ETy->isArrayTy()) {
                                  // Nested array - use fallback
                                  C->addConstant(llvm::cast<llvm::Constant>(C->self->serializeValueToIR(C->builder, ETy, Ptr)));
                              }
                          }
                      }
                  } else {
                      C->lastWasFieldHeader = true;
                  }
              }
              va_end(Args);
              return 0;
          };

          if constexpr (std::is_pointer_v<Decayed>) {
              if (!Value) return serializeValueToIR(Builder, ExpectedType, &Value);
              __builtin_dump_struct(Value, Callback, &Ctx);
          } else {
              __builtin_dump_struct(&Value, Callback, &Ctx);
          }

          if (Ctx.result) {
              if (ExpectedType->isPointerTy()) {
                  auto *GV = new llvm::GlobalVariable(*Module, Ctx.result->getType(), true,
                                                      llvm::GlobalValue::InternalLinkage, Ctx.result, "specialized_instance");
                  return Builder.CreateBitCast(GV, ExpectedType);
              }
              return Ctx.result;
          }
      }

      return serializeValueToIR(Builder, ExpectedType, &Value);
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
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed: %s\n", E.what());
    } catch (...) {
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed with an unknown error.\n");
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
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed: %s\n", E.what());
    } catch (...) {
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed with an unknown error.\n");
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
    if (!RS) {
        throw std::runtime_error("[ClangRuntimeSpecializer] could not init!");
    }

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
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: arguments differ after execution of %s\n", funcName);
                std::abort();
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
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: return values differ for %s\n", funcName);
                std::abort();
            }
        }

        // Compare modified arguments
        if constexpr (allComparable<ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: arguments differ after execution of %s\n", funcName);
                std::abort();
            }
        }
    }
    std::fprintf(stderr, "[ClangRuntimeSpecializer] Successfully specialized %s! No differences could be observed. \n", funcName);
  }


  template <const char* funcName, class MemFn, class OBJ, class... ARGS>
  __attribute__((always_inline))
  void assertSpecializedMethodIsEquivalent(MemFn Mf, OBJ Obj, ARGS... Args) {
    static_assert(allComparable<OBJ, ARGS...>(),
        "The object and all arguments passed to assertSpecializedMethodIsEquivalent must support the equality operator (==). "
        "This is required to ensure that the specialized method behavior matches the original when the object or arguments are modified.");

    auto* RS = ClangRuntimeSpecializer::init();
    if (!RS) {
        throw std::runtime_error("[ClangRuntimeSpecializer] could not init!");
    }

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

        if constexpr (allComparable<OBJ, ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: object/args differ after execution of %s\n", funcName);
                std::abort();
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
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: return values differ for %s\n", funcName);
                std::abort();
            }
        }

        if constexpr (allComparable<OBJ, ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: object/args differ after execution of %s\n", funcName);
                std::abort();
            }
        }
    }
    std::fprintf(stderr, "[ClangRuntimeSpecializer] Successfully specialized %s! No differences could be observed. \n", funcName);
  }

} // namespace clangRuntimeSpecializer