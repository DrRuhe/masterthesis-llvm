#pragma once

#include <type_traits>
#include <utility>

namespace clangRuntimeSpecializer {

  namespace detail {
    // Implementiert in der .cpp (damit wir dort Weak-Refs + Logging haben).
    void maybe_log_irdump();
  }

  // call_specialized stellt bereit:
  template <class MemFn, class Obj, class... Args>
  decltype(auto) call_specialized(MemFn mf, Obj&& obj, Args&&... args) {
    detail::maybe_log_irdump();

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