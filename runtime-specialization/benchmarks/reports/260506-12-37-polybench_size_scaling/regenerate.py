#!/usr/bin/env python3
"""Re-runs the report with identical arguments.
Output goes to a NEW timestamped directory — this report is not overwritten."""
import subprocess, sys

SCRIPT = '/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/benchmarks/reporting/polybench_size_scaling.py'
ARGS   = ['--db', '/tmp/benchmarks_copy.duckdb']

if __name__ == "__main__":
    sys.exit(subprocess.call([sys.executable, SCRIPT, *ARGS]))
