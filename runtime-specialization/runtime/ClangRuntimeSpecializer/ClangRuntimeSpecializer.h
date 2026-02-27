#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdarg>
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
  struct has_equality_operator : std::false_type {};

  template <typename T>
  struct has_equality_operator<T, std::void_t<decltype(std::declval<const T&>() == std::declval<const T&>())>> : std::true_type {};

  template <typename... Args>
  constexpr bool all_comparable() {
    return (has_equality_operator<std::decay_t<Args>>::value && ...);
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

      // Ensure the number of args between the callsite in the IR, the function to specialize and number of passed args are compatible.
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

    llvm::CallBase* findCallSpecializedFunctionInModule(const char* functionName, const char* UID) const
    {
      auto readCStringFromGlobal = [&](llvm::GlobalVariable *GV) -> std::string {
        if (!GV) return {};
        if (auto *CDA = llvm::dyn_cast<llvm::ConstantDataArray>(GV->getInitializer())) {
          if (CDA->isCString()) return CDA->getAsCString().str();
        }
        return {};
      };

      auto getAnnotationValueForFunction = [&](const llvm::Function &F, llvm::StringRef Key) -> std::optional<std::string> {
        llvm::GlobalVariable *AnnGV = Module->getGlobalVariable("llvm.global.annotations");
        if (!AnnGV || !AnnGV->hasInitializer()) return std::nullopt;
        auto *CA = llvm::dyn_cast<llvm::ConstantArray>(AnnGV->getInitializer());
        if (!CA) return std::nullopt;
        for (unsigned i = 0; i < CA->getNumOperands(); ++i) {
          auto *Elt = llvm::dyn_cast<llvm::ConstantStruct>(CA->getOperand(i));
          if (!Elt || Elt->getNumOperands() < 4) continue;
          // 0: ptr to annotated global (function), 1: ptr to anno string, 4: extra args (optional)
          llvm::Value *Op0 = Elt->getOperand(0);
          llvm::Value *Op1 = Elt->getOperand(1);
          llvm::Value *Op4 = (Elt->getNumOperands() >= 5) ? Elt->getOperand(4) : nullptr;

          if (auto *Op0C = llvm::dyn_cast<llvm::Constant>(Op0)) {
            if (auto *Target = Op0C->stripPointerCasts()) {
              if (Target == &F) {
                // Extract key
                std::string KeyStr;
                if (auto *Op1C = llvm::dyn_cast<llvm::Constant>(Op1)) {
                  if (auto *KeyGV = llvm::dyn_cast<llvm::GlobalVariable>(Op1C->stripPointerCasts())) {
                    KeyStr = readCStringFromGlobal(KeyGV);
                  }
                }

                if (KeyStr == Key) {
                  // Try to extract first argument string from args struct if present
                  if (Op4) {
                    if (auto *Op4C = llvm::dyn_cast<llvm::Constant>(Op4)) {
                      if (auto *ArgsGV = llvm::dyn_cast<llvm::GlobalVariable>(Op4C->stripPointerCasts())) {
                        if (auto *ArgsInit = llvm::dyn_cast<llvm::ConstantStruct>(ArgsGV->getInitializer())) {
                          if (ArgsInit->getNumOperands() >= 1) {
                            if (auto *Arg0C = llvm::dyn_cast<llvm::Constant>(ArgsInit->getOperand(0))) {
                              if (auto *StrGV = llvm::dyn_cast<llvm::GlobalVariable>(Arg0C->stripPointerCasts())) {
                                std::string V = readCStringFromGlobal(StrGV);
                                if (!V.empty()) return V;
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                  // No value available
                  return std::string();
                }
              }
            }
          }
        }
        return std::nullopt;
      };

      for (auto &F : *Module) {
        std::string FName = F.getName().str();
        if (FName.find(functionName) != std::string::npos) {
          auto AnnoVal = getAnnotationValueForFunction(F, CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME);
          if (!AnnoVal)
          {
            continue;
          }

          if (*AnnoVal == UID) {
            std::fprintf(stderr, "[ClangRuntimeSpecializer] Found Function %s responsible for specializing %s \n",FName.c_str(),UID);
            // Found a function that represents our call_specialized instantiation. Now find calls to it
            for (auto &U : F.uses()) {
              if (auto *CB = llvm::dyn_cast<llvm::CallBase>(U.getUser())) {
                if (CB->getCalledFunction() == &F) {
                  // if (auto *EnclosingF = CB->getFunction()) {
                  //   std::fprintf(stderr, "[ClangRuntimeSpecializer] Callback used in function: %s\n", EnclosingF->getName().data());
                  //   EnclosingF->print(llvm::errs());
                  //   llvm::errs() << "\n";
                  // }
                  return CB;
                }
              }
            }
          }
        }
      }

      throw std::runtime_error(std::string("[ClangRuntimeSpecializer] Could not find callsite for UID: ") + UID);
    }

    template<typename T>
    static std::string getTypeNameViaDumpStruct(const T& value) {
      if constexpr (std::is_class_v<T> || std::is_union_v<T>) {
        std::string name;
        auto callback = [](void* ctx, const char* fmt, ...) -> int {
          std::string* namePtr = static_cast<std::string*>(ctx);
          if (!namePtr->empty()) return 0;
          
          va_list args;
          va_start(args, fmt);
          if (std::strcmp(fmt, "%s") == 0) {
            const char* n = va_arg(args, const char*);
            if (n) *namePtr = n;
          }
          va_end(args);
          return 0;
        };
        __builtin_dump_struct(&value, callback, (void*)&name);
        return name;
      }
      return "";
    }

    // Recursively serialize a value of a given LLVM type from a memory location.
    llvm::Value* serializeValueToIR(llvm::IRBuilder<>& builder, llvm::Type* type, const void* valuePtr) {
      if (type->isIntegerTy()) {
        unsigned BitWidth = type->getIntegerBitWidth();
        if (BitWidth <= 64) {
          uint64_t Val = 0;
          std::memcpy(&Val, valuePtr, (BitWidth + 7) / 8);
          return llvm::ConstantInt::get(type, Val);
        }
        throw std::runtime_error("[ClangRuntimeSpecializer] Integers > 64 bits are not supported yet.");
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
            throw std::runtime_error("[ClangRuntimeSpecializer] Failed to serialize struct element " + std::to_string(i));
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
             throw std::runtime_error("[ClangRuntimeSpecializer] Failed to serialize array element " + std::to_string(i));
          }
        }
        return llvm::ConstantArray::get(ATy, Elements);
      }
      
      throw std::runtime_error("[ClangRuntimeSpecializer] Unsupported type for serialization: " + std::to_string(type->getTypeID()));
    }

    template <class T>
    llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& builder, llvm::Argument* irArg, T&& value) {
      using Decayed = std::decay_t<T>;

      llvm::Type* expectedType = irArg ? irArg->getType() : nullptr;
      if (!expectedType) {
          throw std::runtime_error("[ClangRuntimeSpecializer] expectedType was null during serialization.");
      }

      auto findPreciseType = [&](const auto& val) -> llvm::Type* {
          using ValType = std::decay_t<decltype(val)>;
          if constexpr (std::is_class_v<ValType> || std::is_union_v<ValType>) {
              std::string name = getTypeNameViaDumpStruct(val);
              if (!name.empty()) {
                  if (auto* Ty = llvm::StructType::getTypeByName(Module->getContext(), "struct." + name)) return Ty;
                  if (auto* Ty = llvm::StructType::getTypeByName(Module->getContext(), "class." + name)) return Ty;
                  if (auto* Ty = llvm::StructType::getTypeByName(Module->getContext(), name)) return Ty;
              }
          }
          return nullptr;
      };

      llvm::Type* preciseType = nullptr;
      if (irArg && irArg->hasByValAttr()) {
          preciseType = irArg->getParamByValType();
      } else {
          preciseType = findPreciseType(value);
      }

      if (expectedType->isPointerTy()) {
        if constexpr (std::is_pointer_v<Decayed>) {
          if (!preciseType && value != nullptr) {
              preciseType = findPreciseType(*value);
          }
          
          if (!preciseType) {
              throw std::runtime_error("[ClangRuntimeSpecializer] Could not infer precise type for pointer argument.");
          }

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
          
          if (!preciseType) {
              throw std::runtime_error("[ClangRuntimeSpecializer] Could not infer precise type for pointer argument (passed by reference).");
          }

          return serializeValueToIR(builder, expectedType, &ptr);
        }
      }

      llvm::Value* val = serializeValueToIR(builder, expectedType, &value);
      if (!val) {
          throw std::runtime_error("[ClangRuntimeSpecializer] Failed to serialize argument of type " + std::to_string(expectedType->getTypeID()));
      }
      return val;
    }
  };

  // call_specialized_impl stellt bereit:
  template <const char* funcName, class MemFn, class Obj, class... Args>
  __attribute__((always_inline))
  decltype(auto) specializeMethodOrFallback(MemFn mf, Obj&& obj, Args&&... args) {
    auto invoke = [&]() -> decltype(auto) {
      return (std::forward<Obj>(obj).*mf)(std::forward<Args>(args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<funcName,void>(std::forward<Obj>(obj), std::forward<Args>(args)...);
          return;
        } else {
          return RS->callSpecialized<funcName,R>( std::forward<Obj>(obj), std::forward<Args>(args)...);
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
    static_assert(all_comparable<ARGS...>(),
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
        if constexpr (all_comparable<ARGS...>()) {
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
        if constexpr (has_equality_operator<R>::value) {
            if (ResOrig != ResSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: return values differ for %s\n", funcName);
                std::abort();
            }
        }

        // Compare modified arguments
        if constexpr (all_comparable<ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: arguments differ after execution of %s\n", funcName);
                std::abort();
            }
        }
    }
    std::fprintf(stderr, "[ClangRuntimeSpecializer] Successfully specialized %s! No differences could be observed. \n", funcName);
  }


  template <const char* funcName, class MemFn, class Obj, class... ARGS>
  __attribute__((always_inline))
  void assertSpecializedMethodIsEquivalent(MemFn mf, Obj obj, ARGS... Args) {
    static_assert(all_comparable<Obj, ARGS...>(),
        "The object and all arguments passed to assertSpecializedMethodIsEquivalent must support the equality operator (==). "
        "This is required to ensure that the specialized method behavior matches the original when the object or arguments are modified.");

    auto* RS = ClangRuntimeSpecializer::init();
    if (!RS) {
        throw std::runtime_error("[ClangRuntimeSpecializer] could not init!");
    }

    // Copy object + arguments for both calls
    auto ArgsOrig = std::forward_as_tuple(obj, Args...);
    auto ArgsSpec = std::make_tuple(obj, Args...);


    using R = decltype(std::invoke(mf, obj, Args...));

    if constexpr (std::is_void_v<R>) {
        // Call original
        std::apply([&](auto&& ObjArg, auto&&... CallArgs) {
            std::invoke(mf, std::forward<decltype(ObjArg)>(ObjArg), std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsOrig);

        // Call specialized
        std::apply([&](auto&& ObjArg, auto&&... CallArgs) {
            RS->callSpecialized<funcName, void>(
              std::forward<decltype(ObjArg)>(ObjArg),
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);

        if constexpr (all_comparable<Obj, ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: object/args differ after execution of %s\n", funcName);
                std::abort();
            }
        }
    } else {
        // Call original
        R ResOrig = std::apply([&](auto&& ObjArg, auto&&... CallArgs) -> R {
            return std::invoke(mf, std::forward<decltype(ObjArg)>(ObjArg), std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsOrig);

        // Call specialized
        R ResSpec = std::apply([&](auto&& ObjArg, auto&&... CallArgs) -> R {
            return RS->callSpecialized<funcName, R>(
              std::forward<decltype(ObjArg)>(ObjArg),
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);

        if constexpr (has_equality_operator<R>::value) {
            if (ResOrig != ResSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: return values differ for %s\n", funcName);
                std::abort();
            }
        }

        if constexpr (all_comparable<Obj, ARGS...>()) {
            if (ArgsOrig != ArgsSpec) {
                std::fprintf(stderr, "[ClangRuntimeSpecializer] Comparison failed: object/args differ after execution of %s\n", funcName);
                std::abort();
            }
        }
    }
    std::fprintf(stderr, "[ClangRuntimeSpecializer] Successfully specialized %s! No differences could be observed. \n", funcName);
  }

} // namespace clangRuntimeSpecializer