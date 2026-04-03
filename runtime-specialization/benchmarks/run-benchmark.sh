#!/usr/bin/env bash
# run-benchmark.sh — Run a benchmark binary with CPU scaling disabled.
#
# Usage: run-benchmark.sh <benchmark_binary> [benchmark_args...]
#
# Temporarily sets the CPU governor to "performance" to eliminate timing noise
# from frequency scaling.  Requires sudo (for cpupower or sysfs writes).

set -euo pipefail

[[ $# -ge 1 ]] || { echo "Usage: $0 <benchmark_binary> [args...]" >&2; exit 1; }

BINARY="$(realpath "$1")"
shift

# ── CPU scaling helpers ───────────────────────────────────────────────────────

GOV_DIR=/sys/devices/system/cpu/cpu0/cpufreq

set_governor() {          # <governor>
    local gov="$1"
    if command -v cpupower >/dev/null 2>&1; then
        sudo cpupower frequency-set -g "$gov" > /dev/null
    else
        for f in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
            echo "$gov" | sudo tee "$f" > /dev/null
        done
    fi
}

if [[ ! -f "$GOV_DIR/scaling_governor" ]]; then
    echo "Warning: CPU frequency scaling not available — running benchmark as-is." >&2
    exec "$BINARY" "$@"
fi

ORIGINAL_GOV="$(cat "$GOV_DIR/scaling_governor")"

if [[ "$ORIGINAL_GOV" == "performance" ]]; then
    echo "CPU governor already set to: performance" >&2
    exec "$BINARY" "$@"
fi

cleanup() {
    echo "Restoring CPU governor to: $ORIGINAL_GOV" >&2
    set_governor "$ORIGINAL_GOV" || true
}
trap cleanup EXIT

echo "Setting CPU governor to: performance (was: $ORIGINAL_GOV)" >&2
set_governor performance

"$BINARY" "$@"
