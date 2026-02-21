#pragma once

#include <cassert>
#include <optional>
#include <type_traits>
#include <utility>
#include <memory>
#include <vector>
#include <typeinfo>
#include <stdexcept>
#include <cstdio>

//#include "llvm/IR/Constants.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace clangRuntimeSpecializer {

  class ClangRuntimeSpecializer {
  public:
    static ClangRuntimeSpecializer* init();

    template <class R, class... Args>
    R call_specialized(const char* funcName, Args&&... args) {
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
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Could not find function: ") + funcName);
      }

      // TODO reuse a clean copy of the llvm module. POssibly perform llvm::CloneModule(*Module) and then add the specialization wrapper to the copied module only.
      static uint64_t SpecializationCount = 0;
      std::string SpecializationWrapperName = "specialized_wrapper_" + std::to_string(++SpecializationCount);

      // Create a new function that takes no arguments and, inside, calls the target with serialized constants.
      llvm::IRBuilder<> Builder{Context};
      llvm::FunctionType* FTy = llvm::FunctionType::get(TargetFunc->getReturnType(), false);
      llvm::Function* NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, SpecializationWrapperName, *Module);

      llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", NewFunc);
      Builder.SetInsertPoint(Entry);

      // Serialize the runtime arguments to IR constants and create a call to the target function with them.
      auto ArgValues = serializeArgumentsToIR(Builder, std::forward<Args>(args)...);

      // Validate serialized args
      bool allSerialized = true;
      for (auto* val : ArgValues) {
        if (!val) { allSerialized = false; break; }
      }

      if (!allSerialized) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Not all arguments could be serialized to IR constants.");
      }

      // Ensure the arity matches before creating the call.
      if (ArgValues.size() != TargetFunc->arg_size()) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Mismatch between provided arguments and target function parameters.");
      }

      if (TargetFunc->hasFnAttribute(llvm::Attribute::NoInline)) throw std::runtime_error("[ClangRuntimeSpecializer] The specialized function has a no inline attribute!");

      // Encourage inlining for the callee in the JIT pipeline.
      TargetFunc->removeFnAttr(llvm::Attribute::NoInline);
      TargetFunc->removeFnAttr(llvm::Attribute::OptimizeNone);
      TargetFunc->addFnAttr(llvm::Attribute::AlwaysInline);

      auto *CallInst = Builder.CreateCall(TargetFunc->getFunctionType(), TargetFunc, ArgValues);
      CallInst->setAttributes(TargetFunc->getAttributes());
      CallInst->addFnAttr(llvm::Attribute::AlwaysInline);

      if (TargetFunc->getReturnType()->isVoidTy()) {
        Builder.CreateRetVoid();
      } else {
        Builder.CreateRet(CallInst);
      }

      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specializing call to: %s\n", funcName);
      // Print the new function to stderr as requested.
      NewFunc->print(llvm::errs());
      llvm::errs() << "\n";

      // Clone the module and add it to the JIT.
      // We need to move the Module into a ThreadSafeModule, but we want to keep it in the Specializer too.
      // So we clone it.
      auto NewModule = llvm::CloneModule(*Module);
      
      // Every function except the specialized wrapper should have available_externally linkage
      // if it has a definition. This allows the JIT inliner to see the bodies but won't
      // produce a definition in the resulting object file, as we want to use the host's version
      // if it's not inlined.
      for (auto &F : *NewModule) {
        if (F.getName() == SpecializationWrapperName) {
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

      auto SpecializedFn = JIT->lookup(SpecializationWrapperName);
      if (!SpecializedFn) {
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Failed to lookup wrapper: ") + llvm::toString(SpecializedFn.takeError()));
      }

      auto SpecializedFnPtr = SpecializedFn->template toPtr<R()>();

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
    explicit ClangRuntimeSpecializer();

    template <class T>
    llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& builder, T&& value) {
      using Decayed = std::decay_t<T>;
      // TODO implement proper serialization logic for all sorts of types.
      if constexpr (std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>) {
        llvm::Type* Ty = llvm::Type::getIntNTy(builder.getContext(),
                                              static_cast<unsigned>(sizeof(Decayed) * 8));
        return llvm::ConstantInt::get(Ty, static_cast<std::uint64_t>(value));
      } else if constexpr (std::is_floating_point_v<Decayed>) {
        if constexpr (std::is_same_v<Decayed, float>) {
          return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(value));
        } else {
          return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(static_cast<double>(value)));
        }
      } else if constexpr (std::is_pointer_v<Decayed>) {
        llvm::Type* Ty = llvm::Type::getInt64Ty(builder.getContext());
        llvm::Constant* IntVal = llvm::ConstantInt::get(Ty, reinterpret_cast<std::uintptr_t>(value));
        return llvm::ConstantExpr::getIntToPtr(IntVal, llvm::PointerType::getUnqual(builder.getContext()));
      } else {
        std::fprintf(stderr, "[ClangRuntimeSpecializer] Warning: Cannot serialize argument of type %s to IR.\n",
                     typeid(T).name());
        return nullptr;
      }
    }

    template <class... Args>
    std::vector<llvm::Value*> serializeArgumentsToIR(llvm::IRBuilder<>& builder, Args&&... args) {
      return {serializeArgumentToIR(builder, std::forward<Args>(args))...};
    }
  };

  // call_specialized_impl stellt bereit:
  template <class MemFn, class Obj, class... Args>
  decltype(auto) call_specialized_impl(const char* funcName, MemFn mf, Obj&& obj, Args&&... args) {
    auto invoke = [&]() -> decltype(auto) {
      return (std::forward<Obj>(obj).*mf)(std::forward<Args>(args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(invoke());
        if constexpr (std::is_void_v<R>) {
          RS->template call_specialized<void>(funcName, std::forward<Obj>(obj), std::forward<Args>(args)...);
          return;
        } else {
          return RS->template call_specialized<R>(funcName, std::forward<Obj>(obj), std::forward<Args>(args)...);
        }
      }
    } catch (const std::exception& e) {
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed: %s\n", e.what());
    } catch (...) {
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed with an unknown error.\n");
    }

    if constexpr (std::is_void_v<decltype(invoke())>) {
      invoke();
      return;
    } else {
      return invoke();
    }
  }

  template <class Fn, class... Args>
  decltype(auto) call_specialized_free_impl(const char* funcName, Fn f, Args&&... args) {
    auto invoke = [&]() -> decltype(auto) {
      return f(std::forward<Args>(args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(invoke());
        if constexpr (std::is_void_v<R>) {
          RS->template call_specialized<void>(funcName, std::forward<Args>(args)...);
          return;
        } else {
          return RS->template call_specialized<R>(funcName, std::forward<Args>(args)...);
        }

      }
    } catch (const std::exception& e) {
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed: %s\n", e.what());
    } catch (...) {
      std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed with an unknown error.\n");
    }

    if constexpr (std::is_void_v<decltype(invoke())>) {
      invoke();
      return;
    } else {
      return invoke();
    }
  }

#define call_specialized(fn, obj, ...) call_specialized_impl(#fn, fn, obj, ##__VA_ARGS__)
#define call_specialized_free(fn, ...) call_specialized_free_impl(#fn, fn, ##__VA_ARGS__)

} // namespace clangRuntimeSpecializer