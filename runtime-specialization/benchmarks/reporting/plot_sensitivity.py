#!/usr/bin/env python3
"""plot_sensitivity.py — OAT parameter sensitivity plots."""
import argparse
import sys
import json
import math
from pathlib import Path
from collections import defaultdict
import duckdb
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path
from report_utils import make_report_dir

KNOWN_PARAMS = [
    "fixpoint_max",
    "unroll_max",
    "large_module_max",
    "early_prune",
    "o3_final",
]


def geomean(vals):
    if not vals:
        return float("nan")
    log_sum = sum(math.log(max(v, 1.0)) for v in vals)
    return math.exp(log_sum / len(vals))


def parse_config_name(config_name):
    """Return (param, value_str) from a sens_{param}_{value} config name.

    Uses the known param list so that param names containing underscores are
    handled correctly (e.g. 'fixpoint_max' vs a hypothetical 'fix_point_max').
    """
    prefix = "sens_"
    if not config_name.startswith(prefix):
        return None, None
    rest = config_name[len(prefix):]
    for param in KNOWN_PARAMS:
        if rest.startswith(param + "_"):
            value_str = rest[len(param) + 1:]
            return param, value_str
    # Fallback: last token is value, everything else is param name
    parts = rest.rsplit("_", 1)
    if len(parts) == 2:
        return parts[0], parts[1]
    return None, None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--db", default=None)
    parser.add_argument("--study-name", required=True)
    parser.add_argument("--importance-json", default=None)
    parser.add_argument("--output-dir", default=None)
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    if args.output_dir:
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    else:
        output_dir = make_report_dir(__file__, parser, args)

    importance = {}
    if args.importance_json:
        with open(args.importance_json) as fh:
            importance = json.load(fh)

    rows = con.execute(
        """SELECT config_name, kernel, med_jit_ns, med_spec_ns
           FROM v_ablation_medians
           WHERE study_name=? AND config_name LIKE 'sens_%'
             AND med_jit_ns IS NOT NULL AND med_spec_ns IS NOT NULL
           ORDER BY config_name, kernel""",
        [args.study_name],
    ).fetchall()
    con.close()

    if not rows:
        print(f"No sensitivity data found for study '{args.study_name}'", file=sys.stderr)
        sys.exit(1)

    # param -> value_str -> [combined_cost_ns]
    param_data = defaultdict(lambda: defaultdict(list))
    for config_name, _kernel, jit_ns, spec_ns in rows:
        param, value_str = parse_config_name(config_name)
        if param is None:
            continue
        param_data[param][value_str].append(jit_ns + spec_ns)

    if not param_data:
        print("No parseable sens_* config names found in study", file=sys.stderr)
        sys.exit(1)

    fig, ax = plt.subplots(figsize=(10, 6))

    for param in sorted(param_data):
        val_costs = param_data[param]
        try:
            sorted_items = sorted(val_costs.items(), key=lambda kv: float(kv[0]))
        except ValueError:
            sorted_items = sorted(val_costs.items())

        x_vals = [kv[0] for kv in sorted_items]
        y_costs = [geomean(kv[1]) for kv in sorted_items]

        min_cost = min(y_costs) if y_costs else 1.0
        y_norm = [c / min_cost if min_cost > 0 else float("nan") for c in y_costs]

        imp = importance.get(param)
        label = f"{param} (imp={imp:.3f})" if imp is not None else param
        ax.plot(range(len(x_vals)), y_norm, "o-", label=label)

    ax.axhline(y=1.0, color="black", linestyle=":", linewidth=0.8)
    ax.set_xticks([])
    ax.set_xlabel("Parameter value (ordinal position in sweep)")
    ax.set_ylabel("Normalized geomean combined cost (JIT + exec)")
    ax.set_title(f"Parameter sensitivity\n(study: {args.study_name})")
    ax.legend(bbox_to_anchor=(1.05, 1), loc="upper left", fontsize=9)

    out_path = output_dir / f"sensitivity_{args.study_name}.png"
    plt.tight_layout()
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out_path}")


if __name__ == "__main__":
    main()
