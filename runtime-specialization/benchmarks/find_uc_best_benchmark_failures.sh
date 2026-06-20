#!/usr/bin/env bash
# Run AllBenchmarks one benchmark at a time under the UC best-pipeline JSON and
# record which benchmarks pass or fail. Intended to isolate per-benchmark
# SIGSEGVs without aborting the whole benchmark binary.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
    cat <<'EOF'
Usage: find_uc_best_benchmark_failures.sh <all_benchmarks_binary> [options]

Options:
  --config-json PATH        UC per-kernel pipeline JSON
                            (default: benchmarks/reports/thesis-figures/rq4/best_uc_pipelines.json)
  --jobs N                  GNU parallel job count (default: 8)
  --timeout SEC             Per-benchmark timeout in seconds (default: 120)
  --benchmark-filter REGEX  Optional benchmark filter passed to --benchmark_list_tests
  --output-dir DIR          Output directory
                            (default: benchmarks/reports/<timestamp>-uc-best-benchmark-failures)
  --help                    Show this help

Artifacts written under the output directory:
  benchmark_names.txt       Full benchmark list
  results.tsv               One row per benchmark
  passed_benchmarks.txt     Benchmarks that returned 0
  failed_benchmarks.txt     Benchmarks that returned non-zero
  sigsegv_benchmarks.txt    Benchmarks that returned 139
  timeout_benchmarks.txt    Benchmarks that returned 124
  logs/                     Per-benchmark stdout/stderr logs
EOF
}

if [[ $# -lt 1 ]]; then
    usage >&2
    exit 1
fi

BINARY=""
CONFIG_JSON="$SCRIPT_DIR/reports/thesis-figures/rq4/best_uc_pipelines.json"
JOBS=8
TIMEOUT_SEC=120
BENCHMARK_FILTER=""
OUTPUT_DIR=""

if [[ $# -gt 0 ]]; then
    BINARY="$1"
    shift
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        --config-json)
            CONFIG_JSON="$2"
            shift 2
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --timeout)
            TIMEOUT_SEC="$2"
            shift 2
            ;;
        --benchmark-filter)
            BENCHMARK_FILTER="$2"
            shift 2
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

if [[ ! -x "$BINARY" ]]; then
    echo "Error: benchmark binary is not executable: $BINARY" >&2
    exit 1
fi

if [[ ! -f "$CONFIG_JSON" ]]; then
    echo "Error: missing config JSON: $CONFIG_JSON" >&2
    exit 1
fi

if ! command -v parallel >/dev/null 2>&1; then
    echo "Error: GNU parallel is required but was not found in PATH." >&2
    exit 1
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
if [[ -z "$OUTPUT_DIR" ]]; then
    OUTPUT_DIR="$SCRIPT_DIR/reports/${STAMP}-uc-best-benchmark-failures"
fi

mkdir -p "$OUTPUT_DIR/logs"

BENCHMARK_LIST="$OUTPUT_DIR/benchmark_names.txt"
RESULTS_TSV="$OUTPUT_DIR/results.tsv"
PASSED_TXT="$OUTPUT_DIR/passed_benchmarks.txt"
FAILED_TXT="$OUTPUT_DIR/failed_benchmarks.txt"
SIGSEGV_TXT="$OUTPUT_DIR/sigsegv_benchmarks.txt"
TIMEOUT_TXT="$OUTPUT_DIR/timeout_benchmarks.txt"
WORKER_SCRIPT="$OUTPUT_DIR/parallel_worker.sh"

LIST_CMD=("$BINARY" "--benchmark_list_tests")
if [[ -n "$BENCHMARK_FILTER" ]]; then
    LIST_CMD+=("--benchmark_filter=$BENCHMARK_FILTER")
fi

printf "Listing benchmarks...\n"
"${LIST_CMD[@]}" | awk 'NF && $0 != "Benchmark" { print }' > "$BENCHMARK_LIST"

BENCH_COUNT="$(wc -l < "$BENCHMARK_LIST" | tr -d ' ')"
if [[ "$BENCH_COUNT" == "0" ]]; then
    echo "Error: no benchmarks matched the requested filter." >&2
    exit 1
fi

cat > "$WORKER_SCRIPT" <<'EOF'
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
EOF

chmod +x "$WORKER_SCRIPT"

printf "status\trc\thash\tlog_path\tbenchmark_name\n" > "$RESULTS_TSV"

printf "Binary:  %s\n" "$BINARY"
printf "Config:  %s\n" "$CONFIG_JSON"
printf "Jobs:    %s\n" "$JOBS"
printf "Timeout: %ss\n" "$TIMEOUT_SEC"
if [[ -n "$BENCHMARK_FILTER" ]]; then
    printf "Filter:  %s\n" "$BENCHMARK_FILTER"
else
    printf "Filter:  <all benchmarks>\n"
fi
printf "Count:   %s\n" "$BENCH_COUNT"
printf "Output:  %s\n\n" "$OUTPUT_DIR"

parallel \
    --jobs "$JOBS" \
    --keep-order \
    --line-buffer \
    "$WORKER_SCRIPT" "$BINARY" "$CONFIG_JSON" "$TIMEOUT_SEC" "$OUTPUT_DIR/logs" :::: "$BENCHMARK_LIST" \
    | tee -a "$RESULTS_TSV" >/dev/null

tail -n +2 "$RESULTS_TSV" | awk -F '\t' '$1 == "pass" { print $5 }' > "$PASSED_TXT"
tail -n +2 "$RESULTS_TSV" | awk -F '\t' '$1 != "pass" { print $5 }' > "$FAILED_TXT"
tail -n +2 "$RESULTS_TSV" | awk -F '\t' '$1 == "sigsegv" { print $5 }' > "$SIGSEGV_TXT"
tail -n +2 "$RESULTS_TSV" | awk -F '\t' '$1 == "timeout" { print $5 }' > "$TIMEOUT_TXT"

PASS_COUNT="$(wc -l < "$PASSED_TXT" | tr -d ' ')"
FAIL_COUNT="$(wc -l < "$FAILED_TXT" | tr -d ' ')"
SIGSEGV_COUNT="$(wc -l < "$SIGSEGV_TXT" | tr -d ' ')"
TIMEOUT_COUNT="$(wc -l < "$TIMEOUT_TXT" | tr -d ' ')"

printf "\nSummary:\n"
printf "  Passed:   %s\n" "$PASS_COUNT"
printf "  Failed:   %s\n" "$FAIL_COUNT"
printf "  SIGSEGV:  %s\n" "$SIGSEGV_COUNT"
printf "  Timeout:  %s\n" "$TIMEOUT_COUNT"
printf "\nArtifacts:\n"
printf "  %s\n" "$RESULTS_TSV"
printf "  %s\n" "$FAILED_TXT"
printf "  %s\n" "$SIGSEGV_TXT"
printf "  %s\n" "$TIMEOUT_TXT"
