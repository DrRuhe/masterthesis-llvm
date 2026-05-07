#!/usr/bin/env python3
"""Re-runs the report with identical arguments.
Output goes to a NEW timestamped directory — this report is not overwritten."""
import subprocess, sys

SCRIPT = '/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/benchmarks/reporting/combined.py'
ARGS   = ['--run-id', '0e134f5b-6f55-47af-9622-af1424252055']

if __name__ == "__main__":
    sys.exit(subprocess.call([sys.executable, SCRIPT, *ARGS]))
