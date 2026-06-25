#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Rule:
    pattern: re.Pattern[str]
    message: str


RULES = (
    Rule(re.compile(r"\bspecializable variables?\b"), "use `runtime-invariant values` or `specialization constants` depending on context"),
    Rule(re.compile(r"\bspecializable values?\b"), "use `values treated as specialization constants` or a more precise term"),
    Rule(re.compile(r"\bdumped IR\b"), "use `embedded IR` or `embedded LLVM bitcode`"),
    Rule(re.compile(r"\bsteady-state\b"), "replace with `amortized`, `long-run`, or `remaining runtime work` depending on context"),
    Rule(re.compile(r"(?<!runtime )(?<!runtime-)specializers?\b"), "use `runtime specializer` or `CRS` for the whole system"),
)


def should_skip_line(line: str) -> bool:
    stripped = line.strip()
    return stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*")


def default_thesis_path() -> Path:
    repo_root = Path(__file__).resolve().parents[3]
    return repo_root / "docs" / "thesis.typ"


def main() -> int:
    parser = argparse.ArgumentParser(description="Check thesis terminology against the canonical terminology policy.")
    parser.add_argument("path", nargs="?", type=Path, default=default_thesis_path())
    args = parser.parse_args()

    text = args.path.read_text(encoding="utf-8")
    findings: list[str] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        if should_skip_line(line):
            continue
        for rule in RULES:
            match = rule.pattern.search(line)
            if not match:
                continue
            findings.append(
                f"{args.path}:{line_number}: `{match.group(0)}` -> {rule.message}\n"
                f"  {line.strip()}"
            )

    if not findings:
        print(f"Terminology check passed: {args.path}")
        return 0

    print("Terminology check failed:\n")
    print("\n\n".join(findings))
    return 1


if __name__ == "__main__":
    sys.exit(main())
