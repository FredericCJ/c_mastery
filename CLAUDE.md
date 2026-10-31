@AGENTS.md

# Claude Code working notes for C Mastery

`AGENTS.md` (imported above) is the repository working agreement and takes
precedence. This file adds Claude-specific operating rules for the polish
revision recorded in `research/v5/expansion/polish_revision/obligations.json`
(checkpoint: `planning/polish_revision.json`).

**Resuming (for example on a new machine): read
`planning/handoff/CONTINUATION_PROMPT.md` first.** It holds the setup
checklist, the state at handoff, the working procedures and the next steps.

## Orchestration and model choice

The main session is the orchestrator. It owns the plan, the POLISH-* ledger,
shared tools, generators, catalogs, Beads closure, every acceptance decision and
every commit. Work is delegated in waves of at most ten agents with disjoint
write sets (one `labs/<family>` plus `assessor/<family>` per worker).

Always dispatch the cheapest model able to do the task. Price order per million
tokens (input/output): Haiku 5.5 $0.10/$0.50, Sonnet 5.5 $2/$10, Opus 5.5 $4/$20,
Fable 5.1 $10/$50. Start at the tier of the role in `.claude/agents/`; escalate
one tier only after two failed verifications, and log task, model, effort and
outcome in the Beads issue. Fable 5.1 is a logged last resort.

| Role (`.claude/agents/`) | Model | Effort | Use for |
| --- | --- | --- | --- |
| inventory-scout | haiku | low | read-only inventories, receipt and log triage |
| c-mechanic | haiku | medium | prepared rename/format/brace tooling on one family |
| campaign-runner | haiku | medium | running producers/checkers, classifying failures |
| board-operator | haiku | medium | reviewed NUCLEO-F439ZI flash/capture scripts |
| c-rewriter | sonnet | high | semantic readability rewrite of one family |
| curriculum-editor | sonnet | high | canonical briefs, strand pages, security labs, packets |
| reviewer | sonnet | high | independent review of ordinary changes |
| reviewer-deep | opus | high | security, concurrency, PMU and firmware review |
| systems-engineer | opus | xhigh | PMU/loss accounting, native runner, dossiers, F439 design |
| pedagogy-architect | opus | xhigh | episodes, encounter bindings, theory rubrics |

Give a worker the code and the contract, not your conclusion. A worker's report
is a claim to verify, not evidence.

## Commits

Local commits are authorized for this revision only, on branch
`polish/revision`, one per verified batch (mechanical rename, formatting-only,
brace insertion and each semantic batch separately). Never push, tag, release,
or enable git hooks. End commit messages with the attribution line required by
the session.

## C house style

New and rewritten C follows `curriculum/guides/c-style.md` (BARR-C:2018 layout,
SEI CERT C conversions, MISRA C:2023 subset in freestanding/firmware code,
Linux kernel style only for genuine kernel modules). Layout is enforced by the
pinned clang-format 21.1.8 at `build/author-tools/python/bin/clang-format` using
the repository `.clang-format`; never use a different clang-format version.
MISRA documents in `/home/fred/library/MISRA/` are licensed: cite rule numbers
and locators only, never copy their text into the repository.

## Repository author tools

- Graphify: `python3 tools/author-tool graphify query '<symbol>' --budget 600`
  for impact analysis; confirm in actual source. Re-extract after mechanical
  batches. Do not install its git or PreToolUse hooks.
- Beads: `python3 tools/author-tool bd ready --json`; also exposed as the local
  `beads` MCP server (`tools/mcp_beads.py`). Issues reference POLISH-* IDs;
  closing an issue never closes a curriculum obligation.
- OpenSpec (`/opsx:*`) and Spec Kit (`speckit-*` skills): proposals and specs
  cite canonical obligation IDs and never become a competing completion ledger.
- Superpowers plugin: TDD, systematic debugging and verification skills apply;
  existing user authorization overrides their generic approval/worktree steps.
- clangd-lsp and pyright-lsp plugins use pinned servers exposed by
  `python3 tools/install_author_tools.py --user-lsp-shims`.

## Hardware

`perf_event_paranoid` was set to -1 by the user; record it in every hardware
receipt and never change it. The NUCLEO-F439ZI (ST-LINK V2J39M27) may be used
for original curriculum images only: no option-byte (RDP/WRP/BOR) writes, no
ST-LINK firmware updates, bounded runs, one holder at a time.

## Never

Push or tag; edit `reviews/` historical receipts; weaken checks, mutations or
sanitizers; claim learner, physical or independent-review evidence that was not
produced; upload code to remote services.
