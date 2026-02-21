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

// TODO refactor the error handling: create a special "ClangRuntimeSpecializationError" for this project.
namespace clangRuntimeSpecializer {

  class ClangRuntimeSpecializer {
  public:
    static ClangRuntimeSpecializer* init();
    
    //TODO extract the non generic logic into the cpp file. So still perform the generic arg serialization 
    // in this header, but then call a method implemented in the cpp file.
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

      // Ensure the arity matches before creating the call.
      if (sizeof...(Args) != TargetFunc->arg_size()) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Mismatch between provided arguments and target function parameters.");
      }


      // TODO reuse a clean copy of the llvm module. Possibly perform llvm::CloneModule(*Module) and then add the specialization wrapper to the copied module only.
      //   Currently we get conflicts, so specialization fails.
      static uint64_t SpecializationCount = 0;
      std::string SpecializationWrapperName = "specialized_wrapper_" + std::to_string(++SpecializationCount);
      
      // Create a new function that takes no arguments and, inside, calls the target with serialized constants.
      llvm::IRBuilder<> Builder{Context};
      llvm::FunctionType* FTy = llvm::FunctionType::get(TargetFunc->getReturnType(), false);
      llvm::Function* NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, SpecializationWrapperName, *Module);

      llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", NewFunc);
      Builder.SetInsertPoint(Entry);

      // Serialize the runtime arguments to IR constants and create a call to the target function with them.
      // We pass the TargetFunc's arguments to guide serialization.
      auto ArgValues = serializeArgumentsToIR(Builder, TargetFunc->arg_begin(), std::forward<Args>(args)...);


      // Validate serialized args
      bool allSerialized = true;
      for (auto* val : ArgValues) {
        if (!val) { allSerialized = false; break; }
      }

      if (!allSerialized) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Not all arguments could be serialized to IR constants.");
      }

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

    // Recursively serialize a value of a given LLVM type from a memory location.
    llvm::Value* serializeValueToIR(llvm::IRBuilder<>& builder, llvm::Type* type, const void* valuePtr) {
      if (type->isIntegerTy()) {
        unsigned BitWidth = type->getIntegerBitWidth();
        if (BitWidth <= 64) {
          uint64_t Val = 0;
          std::memcpy(&Val, valuePtr, (BitWidth + 7) / 8);
          return llvm::ConstantInt::get(type, Val);
        }
        // TODO: support > 64 bit integers if needed.
      } else if (type->isFloatTy()) {
        float Val;
        std::memcpy(&Val, valuePtr, sizeof(float));
        return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(Val));
      } else if (type->isDoubleTy()) {
        double Val;
        std::memcpy(&Val, valuePtr, sizeof(double));
        return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(Val));
      } else if (type->isPointerTy()) {
        uintptr_t Val;
        std::memcpy(&Val, valuePtr, sizeof(uintptr_t));
        llvm::Type* Ty = llvm::Type::getInt64Ty(builder.getContext());
        llvm::Constant* IntVal = llvm::ConstantInt::get(Ty, static_cast<uint64_t>(Val));
        return llvm::ConstantExpr::getIntToPtr(IntVal, type);
      } else if (type->isStructTy()) {
        llvm::StructType* STy = llvm::cast<llvm::StructType>(type);
        const llvm::DataLayout& DL = Module->getDataLayout();
        const llvm::StructLayout* SL = DL.getStructLayout(STy);
        
        std::vector<llvm::Constant*> Elements;
        for (unsigned i = 0; i < STy->getNumElements(); ++i) {
          llvm::Type* ElemTy = STy->getElementType(i);
          uint64_t Offset = SL->getElementOffset(i);
          const void* ElemPtr = static_cast<const char*>(valuePtr) + Offset;
          
          llvm::Value* ElemVal = serializeValueToIR(builder, ElemTy, ElemPtr);
          if (auto* C = llvm::dyn_cast_or_null<llvm::Constant>(ElemVal)) {
            Elements.push_back(C);
          } else {
            return nullptr;
          }
        }
        return llvm::ConstantStruct::get(STy, Elements);
      } else if (type->isArrayTy()) {
        llvm::ArrayType* ATy = llvm::cast<llvm::ArrayType>(type);
        llvm::Type* ElemTy = ATy->getElementType();
        const llvm::DataLayout& DL = Module->getDataLayout();
        uint64_t ElemSize = DL.getTypeAllocSize(ElemTy);
        
        std::vector<llvm::Constant*> Elements;
        for (uint64_t i = 0; i < ATy->getNumElements(); ++i) {
          const void* ElemPtr = static_cast<const char*>(valuePtr) + (i * ElemSize);
          llvm::Value* ElemVal = serializeValueToIR(builder, ElemTy, ElemPtr);
          if (auto* C = llvm::dyn_cast_or_null<llvm::Constant>(ElemVal)) {
            Elements.push_back(C);
          } else {
            return nullptr;
          }
        }
        return llvm::ConstantArray::get(ATy, Elements);
      }
      
      return nullptr;
    }

    template <class T>
    llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& builder, llvm::Argument* irArg, T&& value) {
      using Decayed = std::decay_t<T>;
      llvm::Type* expectedType = irArg ? irArg->getType() : nullptr;
      if (!expectedType) return nullptr;

      // If it's a pointer in IR, we just pass the address or the pointer value.
      if (expectedType->isPointerTy()) {
        if constexpr (std::is_pointer_v<Decayed>) {
          return serializeValueToIR(builder, expectedType, &value);
        } else {
          // It's a class/struct passed by reference or 'this'
          const Decayed* ptr = &value;
          return serializeValueToIR(builder, expectedType, &ptr);
        }
      }

      // If it's a struct/class passed by value in IR
      if (expectedType->isStructTy() || expectedType->isArrayTy() || expectedType->isIntegerTy() || expectedType->isFloatingPointTy()) {
        return serializeValueToIR(builder, expectedType, &value);
      }

      return nullptr;
    }

    template <class... Args>
    std::vector<llvm::Value*> serializeArgumentsToIR(llvm::IRBuilder<>& builder, llvm::Function::arg_iterator irArgIt, Args&&... args) {
      std::vector<llvm::Value*> result;
      (result.push_back(serializeArgumentToIR(builder, &(*(irArgIt++)), std::forward<Args>(args))), ...);
      return result;
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