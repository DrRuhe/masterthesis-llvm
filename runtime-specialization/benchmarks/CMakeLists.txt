set(LLVM_LINK_COMPONENTS)

set(CLANG_EXE "$<TARGET_FILE:clang>")
set(PLUGIN_LIB "$<TARGET_FILE:LLVMRuntimeSpecializationComptimePlugin>")
set(OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/SpecializerBenchmark.o")

separate_arguments(CXX_FLAGS_LIST NATIVE_COMMAND "${CMAKE_CXX_FLAGS}")

add_custom_command(
    OUTPUT ${OBJ_FILE}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/SpecializerBenchmark.cpp
            -o ${OBJ_FILE}
    DEPENDS SpecializerBenchmark.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling SpecializerBenchmark with project clang and plugin"
)

set_source_files_properties(SpecializerBenchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)
add_benchmark(SpecializerBenchmark ${OBJ_FILE} SpecializerBenchmark.cpp)

target_link_libraries(SpecializerBenchmark PRIVATE
    ClangRuntimeSpecializer
)

# Ensure symbols are exported for the JIT
set_target_properties(SpecializerBenchmark PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
  target_link_options(SpecializerBenchmark PRIVATE "-Wl,--export-dynamic")
endif()
