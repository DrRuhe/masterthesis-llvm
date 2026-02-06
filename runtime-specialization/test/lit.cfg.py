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

# Standard-Substitutions
llvm_shlib_dir = os.path.normpath(config.llvm_shlib_dir)
config.substitutions.append(("%llvmshlibdir", llvm_shlib_dir))
config.substitutions.append(("%shlibext", config.llvm_plugin_ext))

# Clang/Clang++ aus dem Build-Tree auflösen (nicht System clang).
config.substitutions.append((
    "%clang",
    llvm_config.use_llvm_tool("clang", search_paths=[config.llvm_tools_dir], required=True),
))
config.substitutions.append((
    "%clangxx",
    llvm_config.use_llvm_tool("clang++", search_paths=[config.llvm_tools_dir], required=True),
))

import os

if os.environ.get("LIT_DEBUG", "0") == "1":
    lit_config.note(f"test_source_root = {config.test_source_root}")
    lit_config.note(f"test_exec_root   = {config.test_exec_root}")
    lit_config.note(f"llvm_tools_dir    = {getattr(config, 'llvm_tools_dir', None)}")
    lit_config.note(f"llvm_shlib_dir    = {getattr(config, 'llvm_shlib_dir', None)}")