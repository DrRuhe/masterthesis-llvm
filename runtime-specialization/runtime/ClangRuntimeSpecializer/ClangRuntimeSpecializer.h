#pragma once

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <memory>
#include <vector>
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
        throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Could not find function: ") + funcName + " (It might be optimized out already by dead-code-elimination?)");
      }




      // Try to find the call site of call_specialized in the IR to get more precise type information.
      llvm::CallBase* CallSite = nullptr;
      for (auto &F : *Module) {
        for (auto &BB : F) {
          for (auto &I : BB) {
            if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I)) {
              if (auto *Callee = CB->getCalledFunction()) {
                std::string CalleeName = Callee->getName().str();
                // Check if it's a call to call_specialized or its implementation
                if (CalleeName.find("call_specialized") != std::string::npos) {
                  // The first argument to call_specialized (after 'this' if it's a member) is the funcName.
                  // For call_specialized_impl(const char* funcName, ...), it's the first arg.
                  // We need to be careful about which arg it is.
                  unsigned FuncNameArgIdx = 0;
                  // If it's a member function of ClangRuntimeSpecializer, the first IR arg is 'this'.
                  if (CalleeName.find("ClangRuntimeSpecializer") != std::string::npos && 
                      CalleeName.find("call_specialized") != std::string::npos) {
                      FuncNameArgIdx = 1;
                  }
                  
                  if (CB->arg_size() > FuncNameArgIdx) {
                    auto *Arg0 = CB->getArgOperand(FuncNameArgIdx);
                    if (auto *CE = llvm::dyn_cast<llvm::ConstantExpr>(Arg0)) {
                       if (CE->getOpcode() == llvm::Instruction::GetElementPtr) {
                         Arg0 = CE->getOperand(0);
                       }
                    }
                    if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(Arg0)) {
                      if (GV->hasInitializer()) {
                        if (auto *CDS = llvm::dyn_cast<llvm::ConstantDataSequential>(GV->getInitializer())) {
                          if (CDS->isString() && CDS->getAsString().starts_with(funcName)) {
                            CallSite = CB;
                            break;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if (CallSite) break;
        }
        if (CallSite) break;
      }

      if (CallSite) {
        std::fprintf(stderr, "[ClangRuntimeSpecializer] Found IR call site for: %s\n", funcName);
        // Extract the actual types from the IR call site.
        // We skip the first 2 arguments of call_specialized (this and funcName).
        // For call_specialized_impl, we skip 1 or 2 depending on whether it's a member.
        // TODO this can be cleaned up: let the macro call create a UID, which we can use to cleanly retrieve the callsite. In the macro definition also
        unsigned SkipArgs = (CallSite->getCalledFunction()->getName().find("ClangRuntimeSpecializer") != std::string::npos) ? 2 : 1;
        
        std::vector<llvm::Type*> PreciseTypes;
        for (unsigned i = SkipArgs; i < CallSite->arg_size(); ++i) {
            PreciseTypes.push_back(CallSite->getArgOperand(i)->getType());
        }

        if (PreciseTypes.size() == sizeof...(Args)) {
             for (unsigned i = 0; i < PreciseTypes.size(); ++i) {
                 llvm::Value* ArgOp = CallSite->getArgOperand(i + SkipArgs);
                 if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(ArgOp)) {
                     ArgOp = LI->getPointerOperand();
                 }
                 if (auto *Alloca = llvm::dyn_cast<llvm::AllocaInst>(ArgOp)) {
                     llvm::Type* AllocatedType = Alloca->getAllocatedType();
                     std::fprintf(stderr, "[ClangRuntimeSpecializer] Arg %u is alloca of type: ", i);
                     AllocatedType->print(llvm::errs());
                     llvm::errs() << "\n";
                 }
             }
        }
      }

      // Ensure the arity matches before creating the call.
      if (sizeof...(Args) != TargetFunc->arg_size()) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Mismatch between provided arguments and target function parameters.");
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
      if (CallSite) {
          unsigned SkipArgs = (CallSite->getCalledFunction()->getName().find("ClangRuntimeSpecializer") != std::string::npos) ? 2 : 1;
          std::vector<llvm::Type*> PreciseTypes;
          for (unsigned i = 0; i < sizeof...(Args); ++i) {
              llvm::Value* ArgOp = CallSite->getArgOperand(i + SkipArgs);
              if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(ArgOp)) {
                  ArgOp = LI->getPointerOperand();
              }
              if (auto *Alloca = llvm::dyn_cast<llvm::AllocaInst>(ArgOp)) {
                  PreciseTypes.push_back(Alloca->getAllocatedType());
              } else {
                  PreciseTypes.push_back(ArgOp->getType());
              }
          }
          ArgValues = serializeArgumentsToIRWithPreciseTypes(Builder, TargetFuncInNewModule->arg_begin(), PreciseTypes, std::forward<Args>(args)...);
      } else {
          // Fallback to old method if call site not found.
          // Note: we need to re-implement or keep serializeArgumentsToIR for this.
          // Let's just use empty PreciseTypes.
          std::vector<llvm::Type*> EmptyPreciseTypes(sizeof...(Args), nullptr);
          ArgValues = serializeArgumentsToIRWithPreciseTypes(Builder, TargetFuncInNewModule->arg_begin(), EmptyPreciseTypes, std::forward<Args>(args)...);
      }


      // Validate serialized args
      bool allSerialized = true;
      for (auto* val : ArgValues) {
        if (!val) { allSerialized = false; break; }
      }

      if (!allSerialized) {
        throw std::runtime_error("[ClangRuntimeSpecializer] Not all arguments could be serialized to IR constants.");
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
    uint64_t GlobalSpecializationCount = 0;
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
    std::vector<llvm::Value*> serializeArgumentsToIRWithPreciseTypes(llvm::IRBuilder<>& builder, llvm::Function::arg_iterator irArgIt, const std::vector<llvm::Type*>& preciseTypes, Args&&... args) {
      std::vector<llvm::Value*> result;
      unsigned i = 0;
      (result.push_back(serializeArgumentToIRWithPreciseType(builder, &(*(irArgIt++)), preciseTypes[i++], std::forward<Args>(args))), ...);
      return result;
    }

    template <class T>
    llvm::Value* serializeArgumentToIRWithPreciseType(llvm::IRBuilder<>& builder, llvm::Argument* irArg, llvm::Type* preciseType, T&& value) {
      using Decayed = std::decay_t<T>;
      llvm::Type* expectedType = irArg ? irArg->getType() : nullptr;
      if (!expectedType) return nullptr;

      if (expectedType->isPointerTy()) {
        if constexpr (std::is_pointer_v<Decayed>) {
          return serializeValueToIR(builder, expectedType, &value);
        } else {
          // It's a class/struct passed by reference or 'this'
          const Decayed* ptr = &value;
          
          // If we have a precise struct type, we can serialize the struct as a constant
          // and then take its address. This allows the JIT to see the fields.
          if (preciseType && preciseType->isStructTy()) {
              llvm::Value* structVal = serializeValueToIR(builder, preciseType, ptr);
              if (auto *structConst = llvm::dyn_cast_or_null<llvm::Constant>(structVal)) {
                  // Create a global variable for this constant struct so we can take its address.
                  // (JIT will optimize this away anyway if it's inlined).
                  auto *GV = new llvm::GlobalVariable(*Module, preciseType, true, 
                                                      llvm::GlobalValue::InternalLinkage, structConst, "specialized_instance");
                  return builder.CreateBitCast(GV, expectedType);
              }
          }
          
          return serializeValueToIR(builder, expectedType, &ptr);
        }
      }

      return serializeValueToIR(builder, expectedType, &value);
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

  template <class Fn, class... Args>
  void specialize_and_compare_impl(const char* funcName, Fn f, Args... args) {
    auto* RS = ClangRuntimeSpecializer::init();
    if (!RS) {
        throw std::runtime_error("[ClangRuntimeSpecializer] could not init!");
    }

    // Copy arguments for both calls
    auto args_orig = std::make_tuple(args...);
    auto args_spec = std::make_tuple(args...);

    using R = decltype(f(args...));

    if constexpr (std::is_void_v<R>) {
        // Call original
        std::apply(f, args_orig);
        // Call specialized
        std::apply([&](auto&&... call_args) {
            RS->template call_specialized<void>(funcName, std::forward<decltype(call_args)>(call_args)...);
        }, args_spec);

        // Compare modified arguments (if they were passed by reference/pointer)
        if (args_orig != args_spec) {
            std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: arguments differ after execution of %s\n", funcName);
            std::abort();
        }
    } else {
        // Call original
        R res_orig = std::apply(f, args_orig);
        // Call specialized
        R res_spec = std::apply([&](auto&&... call_args) -> R {
            return RS->template call_specialized<R>(funcName, std::forward<decltype(call_args)>(call_args)...);
        }, args_spec);

        // Compare return values
        if (res_orig != res_spec) {
            std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: return values differ for %s\n", funcName);
            std::abort();
        }

        // Compare modified arguments
        if (args_orig != args_spec) {
            std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: arguments differ after execution of %s\n", funcName);
            std::abort();
        }
    }
  }

#define CLANG_RUNTIME_SPECIALIZE_AND_COMPARE(fn, ...) \
    clangRuntimeSpecializer::specialize_and_compare_impl(#fn, fn, ##__VA_ARGS__)

} // namespace clangRuntimeSpecializer