add_subdirectory(comptime)
add_subdirectory(runtime)

# Test Konfiguration
if (LLVM_INCLUDE_TESTS)
    # LLVM Funktionen für LIT laden

    # Pfade für das Plugin-Verzeichnis setzen
    set(SHLIBDIR "${LLVM_LIBRARY_OUTPUT_INTDIR}")

    configure_lit_site_cfg(
            ${CMAKE_CURRENT_SOURCE_DIR}/test/lit.site.cfg.py.in
            ${CMAKE_CURRENT_BINARY_DIR}/test/lit.site.cfg.py
            MAIN_CONFIG
            ${CMAKE_CURRENT_SOURCE_DIR}/test/lit.cfg.py
    )

    add_lit_testsuite(check-smoke-runtime-specializer "Running RuntimeSpecializer smoketests"
            ${CMAKE_CURRENT_BINARY_DIR}/test/WIP
            ${CMAKE_CURRENT_BINARY_DIR}/test/smoke
            DEPENDS LLVMRuntimeSpecializationComptimePlugin ClangRuntimeSpecializer opt FileCheck clang
            ARGS -vsa --no-progress-bar
    )


    add_lit_testsuite(check-wip-runtime-specializer "Running RuntimeSpecializer WIP tests"
            ${CMAKE_CURRENT_BINARY_DIR}/test/WIP
            DEPENDS LLVMRuntimeSpecializationComptimePlugin ClangRuntimeSpecializer opt FileCheck clang
            ARGS -vsa --no-progress-bar
    )

    add_lit_testsuite(check-all-runtime-specializer "Running RuntimeSpecializer all tests"
            ${CMAKE_CURRENT_BINARY_DIR}/test
            DEPENDS LLVMRuntimeSpecializationComptimePlugin ClangRuntimeSpecializer opt FileCheck clang
            ARGS -vsa --no-progress-bar
    )
endif()