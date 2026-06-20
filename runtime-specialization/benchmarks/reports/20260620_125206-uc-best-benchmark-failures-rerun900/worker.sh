#!/usr/bin/env bash
set -euo pipefail
BENCHMARK_NAME="$1"
BENCH_HASH="$(printf '%s' "$BENCHMARK_NAME" | sha1sum | awk '{print $1}')"
LOG_PATH="$OUT/logs/${BENCH_HASH}.log"
set +e
CRS_UC_PIPELINE_CONFIG_JSON="$CONFIG" timeout 900s "$BINARY" "--benchmark_filter=$BENCHMARK_NAME" --benchmark_repetitions=1 >"$LOG_PATH" 2>&1
RC=$?
set -e
STATUS=fail
if [[ "$RC" == 0 ]]; then
  STATUS=pass
elif [[ "$RC" == 124 ]]; then
  STATUS=timeout
elif [[ "$RC" == 139 ]]; then
  STATUS=sigsegv
fi
printf '%s\t%s\t%s\t%s\t%s\n' "$STATUS" "$RC" "$BENCH_HASH" "$LOG_PATH" "$BENCHMARK_NAME"
