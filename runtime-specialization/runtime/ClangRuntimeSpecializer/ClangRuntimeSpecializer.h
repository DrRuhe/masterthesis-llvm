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
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace clangRuntimeSpecializer {

  class ClangRuntimeSpecializer {
  public:
    static ClangRuntimeSpecializer* init();

    template <class MemFn, class Obj, class... Args>
    decltype(auto) call_specialized(const char* funcName, MemFn mf, Obj&& obj, Args&&... args) {
      // Any failure during specialization should be reported and we should fall back to calling the function normally.
      try {
        if (funcName == nullptr) {
          throw std::runtime_error("[ClangRuntimeSpecializer] funcName was null! ");
        }
        if (Module == nullptr) {
          throw std::runtime_error("[ClangRuntimeSpecializer] Module was null! ");
        }
        std::string FuncNameStr(funcName);
        if (!FuncNameStr.empty() && FuncNameStr.front() == '&') {
          FuncNameStr.erase(0, 1);
        }
        llvm::Function *TargetFunc = Module->getFunction(FuncNameStr);

        if (TargetFunc == nullptr) {
          throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Could not find function: ") + funcName);
        }

        // Create a new function that takes no arguments and, inside, calls the target with serialized constants.
        llvm::IRBuilder<> Builder{Context};
        llvm::FunctionType* FTy = llvm::FunctionType::get(TargetFunc->getReturnType(), false);
        llvm::Function* NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, "specialized_wrapper", *Module);

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

        // TODO this is currently faulty, as a call to a member function is going to have the additional instance argument.
        // Ensure the arity matches before creating the call.
        if (ArgValues.size() != TargetFunc->arg_size()) {
          throw std::runtime_error("[ClangRuntimeSpecializer] Mismatch between provided arguments and target function parameters.");
        }

        auto *CallInst = Builder.CreateCall(TargetFunc->getFunctionType(), TargetFunc, ArgValues);
        if (TargetFunc->getReturnType()->isVoidTy()) {
          Builder.CreateRetVoid();
        } else {
          Builder.CreateRet(CallInst);
        }

        std::fprintf(stderr, "[ClangRuntimeSpecializer] Specializing call to: %s\n", funcName);
        // Print the new function to stderr as requested.
        NewFunc->print(llvm::errs());
        llvm::errs() << "\n";
      } catch (const std::exception& e) {
        std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed: %s\n", e.what());
      } catch (...) {
        std::fprintf(stderr, "[ClangRuntimeSpecializer] Specialization failed with an unknown error.\n");
      }

      // Fallback: call the original member function normally.
      auto invoke = [&]() -> decltype(auto) {
        return (std::forward<Obj>(obj).*mf)(std::forward<Args>(args)...);
      };
      if constexpr (std::is_void_v<decltype(invoke())>) {
        invoke();
        return;
      } else {
        return invoke();
      }
    }

    ~ClangRuntimeSpecializer();
  private:

    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> Module;
    explicit ClangRuntimeSpecializer();

    template <class T>
    llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& builder, T&& value) {
      using Decayed = std::decay_t<T>;
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
    if (auto* RS = ClangRuntimeSpecializer::init()) {
      return RS->call_specialized(funcName, mf, std::forward<Obj>(obj), std::forward<Args>(args)...);
    }

    auto invoke = [&]() -> decltype(auto) {
      return (std::forward<Obj>(obj).*mf)(std::forward<Args>(args)...);
    };

    if constexpr (std::is_void_v<decltype(invoke())>) {
      invoke();
      return;
    } else {
      return invoke();
    }
  }

// TODO allow specializing of member functions and non member functions.
#define call_specialized(fn, obj, ...) call_specialized_impl(#fn, fn, obj, ##__VA_ARGS__)

} // namespace clangRuntimeSpecializer