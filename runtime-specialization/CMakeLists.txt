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

    add_lit_testsuite(check-runtime-specializer "Running RuntimeSpecializer tests"
            ${CMAKE_CURRENT_BINARY_DIR}/test
            DEPENDS LLVMRuntimeSpecializationComptimePlugin opt FileCheck clang
    )
endif()