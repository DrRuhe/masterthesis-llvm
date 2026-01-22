import os
import lit.formats
from lit.llvm import llvm_config

config.name = 'RuntimeSpecializer'
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
config.suffixes = ['.ll', '.cpp']

# 1. Initialisiere Standard-LLVM-Pfade und Substitutionen
import lit.llvm
lit.llvm.initialize(lit_config, config)

# 2. Füge das LLVM-Tool-Verzeichnis zum PATH hinzu
llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)

# 3. Normalisiere den shlib-Pfad (entfernt das unnötige ./)
llvm_shlib_dir = os.path.normpath(config.llvm_shlib_dir)

# 4. Stelle sicher, dass Standard-Variablen für Pfade verfügbar sind
config.substitutions.append(("%llvmshlibdir", llvm_shlib_dir))
config.substitutions.append(("%shlibext", config.llvm_plugin_ext))