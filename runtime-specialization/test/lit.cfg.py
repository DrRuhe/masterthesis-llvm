import os
import lit.formats

import lit.llvm

config.name = "RuntimeSpecializer"
config.test_format = lit.formats.ShTest(True)
config.suffixes = [".ll", ".cpp"]

print("Loaded lit.cfg.py")

# Initialisiert u.a. lit.llvm.llvm_config (global im Modul!)
lit.llvm.initialize(lit_config, config)

# Ab hier llvm_config korrekt beziehen:
llvm_config = lit.llvm.llvm_config

# Tools aus dem Build-Tree bevorzugen.
llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)


#
# # Standard-Substitutions
llvm_shlib_dir = os.path.normpath(config.llvm_shlib_dir)
config.substitutions.append(("%llvmshlibdir", llvm_shlib_dir))
config.substitutions.append(("%shlibext", config.llvm_plugin_ext))



# TODO there should be nothing at:
#  config.llvm_obj_root = "/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug"
#  config.llvm_src_root = "/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm"

# TODO:
# /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/test/call-specialized.cpp:13:10: fatal error: 'ClangRuntimeSpecializer.h' file not found
#    13 | #include "ClangRuntimeSpecializer.h"
#       |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~
# 1 error generated.

llvm_config.use_clang(
    additional_flags=[
        config.additional_clang_flags,
        "-Wl,--export-dynamic ",
        f"{llvm_shlib_dir}/libClangRuntimeSpecializer{config.llvm_plugin_ext} ",
        f"-Wl,-rpath,{llvm_shlib_dir} "
        f"-I{config.llvm_src_include} ",
        f"-I{config.llvm_obj_include} ",
        f"-B{config.llvm_tools_dir} "
        f"-I{config.runtime_specializer_src_root} "
        f"-I{config.runtime_specializer_include_dir} "
    ]
)
llvm_config.use_llvm_tool("opt")
#
# # Clang/Clang++ aus dem Build-Tree auflösen (nicht System clang).
# config.substitutions.append((
#     "%clang",
#     llvm_config.use_llvm_tool("clang", search_paths=[
#         config.llvm_tools_dir
#     ], required=True,additional_flags=""),
# ))
# config.substitutions.append((
#     "%clangxx",
#     llvm_config.use_llvm_tool("clang++", search_paths=[
#         config.llvm_tools_dir
#     ], required=True),
# ))