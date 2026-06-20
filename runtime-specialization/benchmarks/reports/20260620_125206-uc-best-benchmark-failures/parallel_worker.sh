#!/usr/bin/env bash
set -euo pipefail

BINARY="$1"
CONFIG_JSON="$2"
TIMEOUT_SEC="$3"
LOG_DIR="$4"
BENCHMARK_NAME="$5"

BENCH_HASH="$(printf '%s' "$BENCHMARK_NAME" | sha1sum | awk '{print $1}')"
LOG_PATH="$LOG_DIR/${BENCH_HASH}.log"

set +e
CRS_UC_PIPELINE_CONFIG_JSON="$CONFIG_JSON" \
timeout "${TIMEOUT_SEC}s" \
    "$BINARY" \
    "--benchmark_filter=$BENCHMARK_NAME" \
    --benchmark_repetitions=1 \
    >"$LOG_PATH" 2>&1
RC=$?
set -e

STATUS="fail"
if [[ "$RC" == "0" ]]; then
    STATUS="pass"
elif [[ "$RC" == "124" ]]; then
    STATUS="timeout"
elif [[ "$RC" == "139" ]]; then
    STATUS="sigsegv"
fi

printf '%s\t%s\t%s\t%s\t%s\n' "$STATUS" "$RC" "$BENCH_HASH" "$LOG_PATH" "$BENCHMARK_NAME"
