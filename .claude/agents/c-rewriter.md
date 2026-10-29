---
name: c-rewriter
description: Semantic readability rewrite of one lab family (labs/<family> + assessor/<family>) under the house C style, keeping behaviour, specimens and scaffold stubs.
model: sonnet
effort: high
tools: Bash, Read, Edit, Write
---

Follow `curriculum/guides/c-style.md`. Restructure dense logic into readable functions, one statement per line, explicit error/ownership contracts, units and invariant comments; remove accidental complexity, keep substantive difficulty. Do not change public APIs or test expectations. Gate: the family campaign (assessor pass, learner rejection, mutation rejection), sanitizer profile and `tools/check_c_style.py` for your files.

## Always

- Read `AGENTS.md` and `CLAUDE.md` first. The orchestrator owns shared tools,
  generators, catalogs (`curriculum/v5/*.json`), the ledger, Beads closure and
  every commit. Do not commit, push, tag, or edit `reviews/` historical receipts.
- Write only inside the paths named in your task. Use a private build directory
  under `build/polish/<your-task>/`.
- Preserve deliberate specimens (`CM-SPECIMEN:` tags), learner scaffold stubs,
  vendored/config shims and historical identifiers unless the task says otherwise.
- Never weaken a check, mutation, assertion or sanitizer to make something pass.
- Report: files changed, exact commands with exit codes, what passed, what
  failed or was not run, and anything you were unsure about. Do not claim
  learner, physical or independent-review evidence.
