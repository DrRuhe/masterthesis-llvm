#pragma once

#include <optional>
#include <type_traits>
#include <utility>
#include <memory>
#include <vector>
#include <typeinfo>

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

      llvm::IRBuilder<> Builder{Context};

      auto ArgValues = serializeArgumentsToIR(Builder, std::forward<Args>(args)...);

      // Check if all arguments were successfully serialized
      bool allSerialized = true;
      for (auto* val : ArgValues) {
        if (!val) {
          allSerialized = false;
          break;
        }
      }

      if (allSerialized && Module) {
        // Map the member function pointer `mf` to an actual LLVM function in `Module`.
        // This is non-trivial as it requires name mangling or some registry.
        // For now, let's use the `funcName` if provided.

        // Create a new function that takes no arguments.
        llvm::FunctionType* FTy = llvm::FunctionType::get(Builder.getVoidTy(), false);
        llvm::Function* NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, "specialized_wrapper", *Module);

        llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", NewFunc);
        Builder.SetInsertPoint(Entry);

        // TODO: Handle the 'this' pointer and call the original function.
        if (funcName) {
           std::fprintf(stderr, "[ClangRuntimeSpecializer] Specializing call to: %s\n", funcName);
           // We might need to find the function in the module.
           // Note: The `funcName` here is from `__PRETTY_FUNCTION__`, which might not match mangled names.
           // However, it can be useful for debugging or as a hint.
        }

        // Print the new function to stderr as requested.
        NewFunc->print(llvm::errs());
        llvm::errs() << "\n";
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

  // call_specialized stellt bereit:
  template <class MemFn, class Obj, class... Args>
  decltype(auto) call_specialized(MemFn mf, Obj&& obj, Args&&... args) {
    //TODO this func Name will currently be "call_specialized", not the name of the actually specialized function. This should be solvable by some macro that the user uses instead, so that
    const char* funcName = __PRETTY_FUNCTION__;
#ifdef __clang__
    // Optional: could use __builtin_FUNCTION() or similar if available/needed,
    // but __PRETTY_FUNCTION__ is already quite good in Clang.
#endif
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

} // namespace clangRuntimeSpecializer