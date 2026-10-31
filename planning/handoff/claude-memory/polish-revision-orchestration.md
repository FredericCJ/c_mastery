---
name: polish-revision-orchestration
description: "How the user wants large c_mastery work run - cheapest-capable subagents, repo author tools, local commits per batch, board use"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1d36b06-322f-41b4-8846-6fa7820cb9b2
  modified: 2026-10-30T17:40:00.324Z
---

For the c_mastery polish revision (started 2026-10-29) the user asked to break work across multiple agents under a central orchestrator and always dispatch the cheapest model able to do each task (Haiku 5.5 < Sonnet 5.5 < Opus 5.5 < Fable 5.1). Escalate one tier only after failed verification. Use the repo author tools (Graphify, OpenSpec, Spec Kit, Superpowers, Beads). Local commits per verified batch on branch `polish/revision` are authorized for this revision (never push/tag/release).

**Why:** cost control on a very large campaign, and reviewable separation of mechanical vs semantic changes.

**How to apply:** keep acceptance/integration in the orchestrator; give workers disjoint write sets. The NUCLEO-F439ZI board is connected and the user wants it used for real physical work, within the safety rules (no option-byte or ST-LINK firmware writes). perf_event_paranoid=-1 was set by the user. On 2026-10-31 the user said "I grant you full mandate to perform any action on the NUCLEO board". Board steps go through tools/f439_board.py, keep the repository's own limits (no option-byte or ST-LINK writes), and run after its safety reviews. The same day the user said they would run "unattended": defer anything that needs their password until they return, and keep working otherwise. Related: [[c-house-style]], [[producers-ignore-help]].
