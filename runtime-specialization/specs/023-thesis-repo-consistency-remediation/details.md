# Remediation Strategy

## Decision Rule

Prefer correcting thesis prose over changing code when:

- the code behavior is already validated by tests or benchmark artifacts,
- the mismatch is stale wording, overstatement, or outdated evaluation
  interpretation,
- changing the code would reopen finished evaluation work without a clear thesis
  benefit.

Prefer proposing code work when:

- the mismatch reveals a real implementation defect,
- the defect invalidates an evaluation artifact or benchmark conclusion,
- the thesis would otherwise need to defend behavior that the repo should
  realistically fix.

## Audit Buckets

### Bucket A: Implementation-description drift

These are thesis statements that no longer match the code:

- P0 described as default although code defaults to P2
- annotation-free wording broader than plugin-rewritten reality
- embedded IR described as if it were the full original TU IR
- lambda specialization described as if it deep-snapshotted closure state
- lookup described as deterministic despite collision overwrite behavior

Expected handling:
- thesis edits first
- optional code follow-up only for collision handling if the user wants stronger
  implementation guarantees

### Bucket B: Runtime-scope understatement

These are safety and deployment claims that are too weak or too optimistic:

- effective single-threaded requirement is broader than concurrent-argument
  mutation alone
- portability text is softer than the unconditional host-wide settings in code

Expected handling:
- thesis edits first
- no code change required unless the user wants to harden portability or
  concurrency behavior

### Bucket C: Evaluation-evidence drift

These are claims where the prose and artifact base are out of sync:

- UC default-pipeline headline numbers mix oracle-style and deployable claims
- SQLite/TPC-H default-pipeline rationale leans on invalidated wrong-blob
  evidence
- binary-size prose appears to use a different basis than the recorded RQ2
  measurement
- size-scaling prose does not point to one settled artifact
- supported-architectures section still contains a live TODO

Expected handling:
- thesis edits plus possible artifact/provenance cleanup
- only propose code changes where artifact invalidity stems from a real repo bug

## Likely Code-Fix Candidate

The strongest candidate for non-prose follow-up is blob-collision handling for
function-to-blob lookup. The current runtime logs a warning and overwrites the
earlier mapping with the later registration. That behavior is important because
it already affected SQLite benchmark interpretation.

Possible implementations to evaluate later:

- reject collisions eagerly during `init()` and fail fast,
- store multiple candidates and require an exact TU identity discriminator,
- improve IR-dumping/rewriting so declarations do not register as specialization
  targets when a definition lives in another blob.

## Verification Philosophy

The remediation work should verify claims by comparing the exact thesis section
against:

- current runtime code for API/behavior claims,
- current benchmark report artifacts for evaluation claims,
- `specs/TODO.md` when the thesis uses a result tracked as completed or blocked.

The goal is not perfect global consistency across the entire thesis in one pass.
The goal is to eliminate the contradictions surfaced by this audit and leave a
clean, executable repair path for the remaining work.
