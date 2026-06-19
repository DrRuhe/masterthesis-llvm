#!/usr/bin/env python3
"""Export the best UC pipeline per kernel as shared JSON.

The source study is the iter-3 optimizer run. For this study, `jit_overhead`
must be reconstructed from `cpu_time` because the raw `real_time` field is 0 for
manual-time UC JIT benchmarks.

By default the script writes two artifacts:
- `<output>` as the amortized winner (`U / S`), used by the thesis and benchmarks.
- `<output_stem>_first_call.json` as the first-call winner (`U / (S + J)`).
It also writes `<output_stem>_amortized.json` as an explicit copy of the
amortized artifact.
"""

from __future__ import annotations

import argparse
import json
from collections import defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Literal

import duckdb


EXPECTED_KERNELS = (
    "apply_row_delta",
    "batch_delta",
    "box_filter",
    "column_scan",
    "count_matching_rows",
    "edge_detection",
    "email_match",
    "generic_sort",
    "grouped_count",
    "grouped_minmax",
    "grouped_sum",
    "multi_agg_delta",
    "multi_key_sort",
    "multi_pattern_match",
    "multi_predicate",
    "separable_gaussian",
    "struct_sort",
    "url_match",
)

UC_LABELS = {
    "uc1_sql": "UC1",
    "uc2_conv": "UC2",
    "uc7_dfa": "UC7",
    "uc8_ivm": "UC8",
    "uc12_groupby": "UC12",
    "uc14_sort": "UC14",
}


@dataclass(frozen=True)
class KernelCandidate:
    kernel: str
    group: str
    size: str
    abstraction: str
    pipeline_id: int
    fixpoint_max: int
    unroll_max: int
    large_module_max: int
    early_prune: bool
    o3_final: bool
    p1_inline_threshold: int | None
    p1_max_module_growth: float | None
    p2_min_func_size: int | None
    p2_max_clones: int | None
    p2_func_spec_iters: int | None
    p2_force_spec: bool | None
    p2_spec_on_addr: bool | None
    p2_spec_literal: bool | None
    first_call_speedup: float
    exec_speedup: float
    jit_ms: float
    spec_ms: float
    unspec_ms: float
    trial_id: int


MetricName = Literal["amortized", "first_call"]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Export the best UC pipeline per kernel as shared JSON.",
    )
    parser.add_argument(
        "--db",
        default="benchmarks/benchmarks.duckdb",
        metavar="PATH",
        help="DuckDB database path.",
    )
    parser.add_argument(
        "--study-name",
        default="uc_optim_iter3_20260601",
        metavar="STR",
        help="Optimization study to export.",
    )
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq4/best_uc_pipelines.json",
        metavar="PATH",
        help="Primary output JSON path.",
    )
    parser.add_argument(
        "--metric",
        choices=("amortized", "first_call", "both"),
        default="both",
        help=(
            "Selection objective to export. 'both' writes the amortized artifact "
            "to --output, plus explicit amortized/first-call siblings."
        ),
    )
    return parser.parse_args()


def fetch_candidates(
    con: duckdb.DuckDBPyConnection, study_name: str
) -> list[KernelCandidate]:
    rows = con.execute(
        """
        WITH phase_rows AS (
            SELECT
                otp.study_name,
                otp.trial_id,
                otp.params_json,
                p."group",
                p.kernel,
                p.kv_s,
                p.kv_a,
                p.phase,
                CASE
                    WHEN p.phase = 'jit_overhead'
                    THEN p.cpu_time * CASE p.time_unit
                        WHEN 'ns' THEN 1.0
                        WHEN 'us' THEN 1e3
                        WHEN 'ms' THEN 1e6
                        WHEN 's'  THEN 1e9
                    END
                    ELSE p.real_time * CASE p.time_unit
                        WHEN 'ns' THEN 1.0
                        WHEN 'us' THEN 1e3
                        WHEN 'ms' THEN 1e6
                        WHEN 's'  THEN 1e9
                    END
                END AS phase_ns
            FROM optim_trial_params otp
            JOIN v_parsed p ON p.run_id = otp.run_id
            WHERE otp.study_name = ?
              AND NOT otp.used_timeout_fallback
              AND p.run_type = 'iteration'
              AND p.kv_a = 'low'
              AND p.phase IN ('jit_overhead', 'specialized_exec')
        ),
        jit AS (
            SELECT
                study_name, trial_id, params_json, "group", kernel, kv_s, kv_a,
                MAX(phase_ns) AS t_jit_ns
            FROM phase_rows
            WHERE phase = 'jit_overhead'
            GROUP BY study_name, trial_id, params_json, "group", kernel, kv_s, kv_a
        ),
        spec AS (
            SELECT
                study_name, trial_id, "group", kernel, kv_s, kv_a,
                MAX(phase_ns) AS t_spec_ns
            FROM phase_rows
            WHERE phase = 'specialized_exec'
            GROUP BY study_name, trial_id, "group", kernel, kv_s, kv_a
        )
        SELECT
            kernel,
            "group",
            kv_s AS size,
            kv_a AS abstraction,
            CAST(json_extract_string(params_json, '$.pipeline') AS INTEGER) AS pipeline_id,
            CAST(json_extract_string(params_json, '$.fixpoint_max') AS INTEGER) AS fixpoint_max,
            CAST(json_extract_string(params_json, '$.unroll_max') AS INTEGER) AS unroll_max,
            COALESCE(
                CAST(json_extract_string(params_json, '$.p0_large_module_max') AS INTEGER),
                CAST(json_extract_string(params_json, '$.p1_large_module_max') AS INTEGER),
                CAST(json_extract_string(params_json, '$.p2_large_module_max') AS INTEGER),
                CAST(json_extract_string(params_json, '$.large_module_max') AS INTEGER)
            ) AS large_module_max,
            CAST(json_extract_string(params_json, '$.early_prune') AS INTEGER) <> 0 AS early_prune,
            CAST(json_extract_string(params_json, '$.o3_final') AS INTEGER) <> 0 AS o3_final,
            CAST(json_extract_string(params_json, '$.p1_inline_threshold') AS INTEGER) AS p1_inline_threshold,
            CAST(json_extract_string(params_json, '$.p1_max_module_growth') AS DOUBLE) AS p1_max_module_growth,
            CAST(json_extract_string(params_json, '$.p2_min_func_size') AS INTEGER) AS p2_min_func_size,
            CAST(json_extract_string(params_json, '$.p2_max_clones') AS INTEGER) AS p2_max_clones,
            CAST(json_extract_string(params_json, '$.p2_func_spec_iters') AS INTEGER) AS p2_func_spec_iters,
            CAST(json_extract_string(params_json, '$.p2_force_spec') AS INTEGER) AS p2_force_spec,
            CAST(json_extract_string(params_json, '$.p2_spec_on_addr') AS INTEGER) AS p2_spec_on_addr,
            CAST(json_extract_string(params_json, '$.p2_spec_literal') AS INTEGER) AS p2_spec_literal,
            first_call_speedup,
            exec_speedup,
            t_jit_ns / 1e6 AS jit_ms,
            t_spec_ns / 1e6 AS spec_ms,
            unspec_ns / 1e6 AS unspec_ms,
            trial_id
        FROM (
            SELECT
                jit.trial_id,
                jit.params_json,
                jit."group",
                jit.kernel,
                jit.kv_s,
                jit.kv_a,
                jit.t_jit_ns,
                spec.t_spec_ns,
                u.unspec_ns,
                u.unspec_ns / (jit.t_jit_ns + spec.t_spec_ns) AS first_call_speedup,
                u.unspec_ns / spec.t_spec_ns AS exec_speedup
            FROM jit
            JOIN spec
              ON spec.study_name = jit.study_name
             AND spec.trial_id = jit.trial_id
             AND spec."group" = jit."group"
             AND spec.kernel = jit.kernel
             AND spec.kv_s = jit.kv_s
             AND spec.kv_a = jit.kv_a
            JOIN unspec_baselines u
              ON u.study_name = jit.study_name
             AND u.kernel = jit.kernel
        )
        ORDER BY kernel, trial_id
        """,
        [study_name],
    ).fetchall()

    return [KernelCandidate(*row) for row in rows]


def make_options(entry: KernelCandidate) -> dict[str, Any]:
    options: dict[str, Any] = {
        "optimization_pipeline": entry.pipeline_id,
        "max_fixpoint_iterations": entry.fixpoint_max,
        "loop_unroll_count": entry.unroll_max,
        "large_module_instr_threshold": entry.large_module_max,
        "enable_early_prune": entry.early_prune,
        "enable_o3_final": entry.o3_final,
    }
    if entry.pipeline_id == 1:
        options["p1_inline_threshold"] = entry.p1_inline_threshold
        options["p1_max_module_growth"] = entry.p1_max_module_growth
    if entry.pipeline_id == 2:
        options["p2_min_func_size"] = entry.p2_min_func_size
        options["p2_max_clones"] = entry.p2_max_clones
        options["p2_func_spec_iters"] = entry.p2_func_spec_iters
        options["p2_force_spec"] = bool(entry.p2_force_spec)
        options["p2_spec_on_addr"] = bool(entry.p2_spec_on_addr)
        options["p2_spec_literal"] = bool(entry.p2_spec_literal)
    return options


def make_env(entry: KernelCandidate) -> dict[str, str]:
    env = {
        "CRS_DEFAULT_PIPELINE": str(entry.pipeline_id),
        "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": str(entry.fixpoint_max),
        "CRS_DEFAULT_LOOP_UNROLL_COUNT": str(entry.unroll_max),
        "CRS_DEFAULT_EARLY_PRUNE": "1" if entry.early_prune else "0",
        "CRS_DEFAULT_O3_FINAL": "1" if entry.o3_final else "0",
    }
    if entry.pipeline_id == 0:
        env["CRS_DEFAULT_P0_LARGE_MODULE_INSTR_THRESHOLD"] = str(entry.large_module_max)
    elif entry.pipeline_id == 1:
        env["CRS_DEFAULT_P1_LARGE_MODULE_INSTR_THRESHOLD"] = str(entry.large_module_max)
        env["CRS_DEFAULT_P1_INLINE_THRESHOLD"] = str(entry.p1_inline_threshold)
        env["CRS_DEFAULT_P1_MAX_MODULE_GROWTH"] = str(entry.p1_max_module_growth)
    elif entry.pipeline_id == 2:
        env["CRS_DEFAULT_P2_LARGE_MODULE_INSTR_THRESHOLD"] = str(entry.large_module_max)
        env["CRS_P2_MIN_FUNC_SIZE"] = str(entry.p2_min_func_size)
        env["CRS_P2_MAX_CLONES"] = str(entry.p2_max_clones)
        env["CRS_P2_FUNC_SPEC_ITERS"] = str(entry.p2_func_spec_iters)
        env["CRS_P2_FORCE_SPEC"] = "1" if entry.p2_force_spec else "0"
        env["CRS_P2_SPEC_ON_ADDR"] = "1" if entry.p2_spec_on_addr else "0"
        env["CRS_P2_SPEC_LITERAL"] = "1" if entry.p2_spec_literal else "0"
    return env


def pipeline_label(pipeline_id: int) -> str:
    return {
        0: "P0",
        1: "P1",
        2: "P2",
    }[pipeline_id]


def key_settings(entry: KernelCandidate) -> str:
    parts = [
        f"fixpoint={entry.fixpoint_max}",
        f"unroll={entry.unroll_max}",
        f"large_module={entry.large_module_max}",
        "prune on" if entry.early_prune else "prune off",
        "O3 on" if entry.o3_final else "O3 off",
    ]
    if entry.pipeline_id == 1:
        parts.append(f"inline_threshold={entry.p1_inline_threshold}")
        parts.append(f"max_growth={entry.p1_max_module_growth:g}")
    if entry.pipeline_id == 2:
        parts.append(f"min_func={entry.p2_min_func_size}")
        parts.append(f"max_clones={entry.p2_max_clones}")
        parts.append(f"iters={entry.p2_func_spec_iters}")
        parts.append("force on" if entry.p2_force_spec else "force off")
        parts.append("addr on" if entry.p2_spec_on_addr else "addr off")
        parts.append("literal on" if entry.p2_spec_literal else "literal off")
    return ", ".join(parts)


def metric_value(entry: KernelCandidate, metric: MetricName) -> float:
    if metric == "amortized":
        return entry.exec_speedup
    return entry.first_call_speedup


def metric_name_for_json(metric: MetricName) -> str:
    return {
        "amortized": "amortized_speedup_u_over_s",
        "first_call": "first_call_speedup_u_over_s_plus_j",
    }[metric]


def metric_label(metric: MetricName) -> str:
    return {
        "amortized": "$U / S$",
        "first_call": "$U / (S + J)$",
    }[metric]


def select_winners(
    candidates: list[KernelCandidate], metric: MetricName
) -> list[KernelCandidate]:
    by_kernel: dict[str, list[KernelCandidate]] = defaultdict(list)
    for candidate in candidates:
        by_kernel[candidate.kernel].append(candidate)

    winners: list[KernelCandidate] = []
    for kernel in EXPECTED_KERNELS:
        kernel_candidates = by_kernel.get(kernel, [])
        if not kernel_candidates:
            raise RuntimeError(f"Missing candidates for kernel '{kernel}'")
        winners.append(
            min(
                kernel_candidates,
                key=lambda entry: (-metric_value(entry, metric), entry.trial_id),
            )
        )
    return winners


def make_document(
    entries: list[KernelCandidate], study_name: str, metric: MetricName
) -> dict[str, Any]:
    found = {entry.kernel for entry in entries}
    missing = sorted(set(EXPECTED_KERNELS) - found)
    extra = sorted(found - set(EXPECTED_KERNELS))
    if missing or extra:
        raise RuntimeError(
            f"Unexpected kernel coverage for {study_name}: missing={missing}, extra={extra}"
        )

    source_sizes = sorted({entry.size for entry in entries})
    source_abstractions = sorted({entry.abstraction for entry in entries})

    return {
        "source_study": study_name,
        "metric": metric_name_for_json(metric),
        "metric_label": metric_label(metric),
        "timing_policy": {
            "jit_overhead": "cpu_time",
            "specialized_exec": "real_time",
            "unspecialized": "real_time",
        },
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "expected_kernel_count": len(EXPECTED_KERNELS),
        "expected_kernels": list(EXPECTED_KERNELS),
        "source_sizes": source_sizes,
        "source_abstractions": source_abstractions,
        "entries": [
            {
                "kernel": entry.kernel,
                "group": entry.group,
                "uc_label": UC_LABELS[entry.group],
                "size": entry.size,
                "abstraction": entry.abstraction,
                "pipeline": pipeline_label(entry.pipeline_id),
                "pipeline_id": entry.pipeline_id,
                "selected_speedup": round(metric_value(entry, metric), 4),
                "first_call_speedup": round(entry.first_call_speedup, 4),
                "exec_speedup": round(entry.exec_speedup, 4),
                "jit_ms": round(entry.jit_ms, 3),
                "spec_ms": round(entry.spec_ms, 3),
                "unspec_ms": round(entry.unspec_ms, 3),
                "trial_id": entry.trial_id,
                "key_settings": key_settings(entry),
                "options": make_options(entry),
                "env": make_env(entry),
            }
            for entry in sorted(entries, key=lambda it: (UC_LABELS[it.group], it.kernel))
        ],
    }


def output_targets(base_output: Path, metric_mode: str) -> dict[MetricName, list[Path]]:
    stem = base_output.stem
    suffix = base_output.suffix
    parent = base_output.parent

    if metric_mode == "amortized":
        return {"amortized": [base_output]}
    if metric_mode == "first_call":
        return {"first_call": [base_output]}

    return {
        "amortized": [
            base_output,
            parent / f"{stem}_amortized{suffix}",
        ],
        "first_call": [
            parent / f"{stem}_first_call{suffix}",
        ],
    }


def main() -> None:
    args = parse_args()
    db_path = Path(args.db)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    con = duckdb.connect(str(db_path), read_only=True)
    try:
        candidates = fetch_candidates(con, args.study_name)
    finally:
        con.close()

    if not candidates:
        raise RuntimeError(f"No candidates found for study {args.study_name}")

    kernels_found = {candidate.kernel for candidate in candidates}
    if len(kernels_found) != len(EXPECTED_KERNELS):
        raise RuntimeError(
            f"Expected candidates for {len(EXPECTED_KERNELS)} kernels, got {len(kernels_found)}"
        )

    written_paths: list[Path] = []
    for metric, paths in output_targets(output_path, args.metric).items():
        document = make_document(select_winners(candidates, metric), args.study_name, metric)
        payload = json.dumps(document, indent=2) + "\n"
        for path in paths:
            path.write_text(payload, encoding="utf-8")
            written_paths.append(path)

    for path in written_paths:
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
