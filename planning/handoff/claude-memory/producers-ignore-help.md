---
name: producers-ignore-help
description: "Many c_mastery tools/run_*.py producers ignore argv, so `--help` runs them for real and rewrites tracked receipts"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1d36b06-322f-41b4-8846-6fa7820cb9b2
  modified: 2026-10-30T17:30:19.010Z
---

Many `tools/run_*.py` producers have no argparse. Invoking one with `--help`
starts a full run that rewrites tracked receipts (for example
`research/v5/expansion/legacy_*/validation.json`) and creates `reviews/`
output in whatever tree it runs in.

**Why:** on 2026-10-31, `python3 tools/run_algorithm_heap.py --help` in the
main checkout rewrote two tracked validation.json files. They had to be
restored with `git checkout`.

**How to apply:** to learn a producer's arguments, read its source (`grep -n
"add_argument\|argv" tools/run_X.py`). Never execute it to read its usage.
Run producers only in frozen or commit-view worktrees, never in the main
checkout. Afterwards, check `git status -- reviews research`. Related:
[[host-resource-limits]], [[golfed-python-edits]].
