#pragma once

#include <optional>
#include <type_traits>
#include <utility>
#include <memory>

//#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace clangRuntimeSpecializer {

  class ClangRuntimeSpecializer {
  public:
    static ClangRuntimeSpecializer* init();

    template <class MemFn, class Obj, class... Args>
    decltype(auto) call_specialized(MemFn mf, Obj&& obj, Args&&... args) {
      auto invoke = [&]() -> decltype(auto) {
        return (std::forward<Obj>(obj).*mf)(std::forward<Args>(args)...);
      };

      // TODO: Use parsed module + serialized args to specialize the call.
      (void)serializeArgumentsToIR(Builder, std::forward<Args>(args)...);

      if constexpr (std::is_void_v<decltype(invoke())>) {
        invoke();
        return;
      } else {
        return invoke();
      }
    }

    ~ClangRuntimeSpecializer();
  private:
    struct Impl;

    explicit ClangRuntimeSpecializer(std::unique_ptr<llvm::Module> mod);

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
        // TODO: Add serialization for pointers, aggregates, and user-defined types.
        (void)builder;
        (void)value;
        return nullptr;
      }
    }

    template <class... Args>
    void serializeArgumentsToIR(llvm::IRBuilder<>& builder, Args&&... args) {
      (void)std::initializer_list<int>{
          (serializeArgumentToIR(builder, std::forward<Args>(args)), 0)...};
    }

    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> Module;
    llvm::IRBuilder<> Builder{Context};
  };

  // call_specialized stellt bereit:
  template <class MemFn, class Obj, class... Args>
  decltype(auto) call_specialized(MemFn mf, Obj&& obj, Args&&... args) {
    if (auto* RS = ClangRuntimeSpecializer::init()) {
      return RS->call_specialized(mf, std::forward<Obj>(obj), std::forward<Args>(args)...);
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