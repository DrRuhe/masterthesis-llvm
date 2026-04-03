#!/usr/bin/env bash
# asm-compare.sh — Diff original ELF assembly vs JIT-specialized assembly
#
# Usage: asm-compare.sh <binary> <orig_func_name> <bench_filter> [output_dir]
#   binary          Path to the benchmark executable
#   orig_func_name  The C function being specialized, e.g. mypow_bench
#   bench_filter    The benchmark to run for specialization, e.g. BM_mypow_specialized_exec
#   output_dir      Defaults to /tmp/asm-compare-<PID>
#
# Example:
#   asm-compare.sh ./SpecializerBenchmark mypow_bench BM_mypow_specialized_exec

set -euo pipefail

if [[ $# -lt 3 ]]; then
    echo "Usage: $0 <binary> <orig_func_name> <bench_filter> [output_dir]" >&2
    exit 1
fi

BINARY="$1"
ORIG_FUNC="$2"
BENCH_FILTER="$3"
OUTDIR="${4:-/tmp/asm-compare-$$}"

mkdir -p "$OUTDIR" "$OUTDIR/jit"

# Step 1: Extract original function ASM from ELF
OBJDUMP="$(command -v llvm-objdump || command -v objdump)"
echo "Using objdump: $OBJDUMP"
"$OBJDUMP" -d --no-show-raw-insn "$BINARY" \
    | awk "/^[[:xdigit:]]+ <${ORIG_FUNC}>/ {f=1}
           /^[[:xdigit:]]+ <[^>]+>/ { if (! /${ORIG_FUNC}/) f=0 }
           f" \
    > "$OUTDIR/orig_raw.asm"

if [[ ! -s "$OUTDIR/orig_raw.asm" ]]; then
    echo "Warning: could not extract '$ORIG_FUNC' from ELF. Check symbol name." >&2
fi

# Step 2: Trigger JIT compilation and capture specialized ASM
CRS_ASM_DUMP_DIR="$OUTDIR/jit" \
    "$BINARY" --benchmark_filter="$BENCH_FILTER" --benchmark_min_time=1x \
    > /dev/null 2>&1 || true

SPEC_ASM="$OUTDIR/jit/${ORIG_FUNC}__specialized.asm"
if [[ ! -f "$SPEC_ASM" ]]; then
    echo "Warning: specialized ASM file not found at $SPEC_ASM" >&2
    echo "  Make sure CRS_ASM_DUMP_DIR is respected and the benchmark ran." >&2
fi

# Step 3: Normalize both (strip address column, collapse whitespace, remove blanks)
normalize() {
    local INPUT="$1"
    local OUTPUT="$2"
    sed 's/^[[:space:]]*[0-9a-f]*:[[:space:]]*//' "$INPUT" \
        | sed 's/[[:space:]]\+/ /g'                         \
        | sed '/^[[:space:]]*$/d'                            \
        > "$OUTPUT"
}

normalize "$OUTDIR/orig_raw.asm" "$OUTDIR/orig.asm"
if [[ -f "$SPEC_ASM" ]]; then
    normalize "$SPEC_ASM" "$OUTDIR/spec.asm"
else
    touch "$OUTDIR/spec.asm"
fi

# Step 4: Show results
echo "=== Original: $ORIG_FUNC ==="
cat "$OUTDIR/orig.asm"
echo ""
echo "=== Specialized: $BENCH_FILTER ==="
cat "$OUTDIR/spec.asm"
echo ""
echo "=== Diff (- original  + specialized) ==="
diff --unified "$OUTDIR/orig.asm" "$OUTDIR/spec.asm" || true

echo ""
echo "Output files: $OUTDIR/"
echo "  orig.asm  — normalized original"
echo "  spec.asm  — normalized specialized"

# Side-by-side on wide terminals:
# sdiff -w 120 "$OUTDIR/orig.asm" "$OUTDIR/spec.asm"
