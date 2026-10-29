# Repository working agreement

Read `curriculum/edition.json`, `planning/authoring_contract.md`, relevant canonical
contracts and the current revision ledger before changing curriculum or evidence.
The authorized native-Ubuntu revision continues across tool and skill changes.

- Keep C17, bounded C23 comparisons and the strict freestanding dependency policy.
  Separate language, compiler, ABI, operating-system and hardware claims.
- Preserve unrelated work, historical IDs and accepted receipts. Change canonical
  sources and generators rather than patching generated output alone.
- `curriculum/v5/expansion/work_inventory.json` and its canonical disposition
  sources remain authoritative. OpenSpec, Spec Kit and Beads reference those IDs;
  tool task status cannot close curriculum evidence gates.
- Distinguish implementation, executed author checks, protocol competence, native
  hardware observation and independent learner assessment. Structural coverage,
  installed tools and authored rubrics do not prove learner mastery.
- Run focused regressions and source-bound final checks. Report unavailable and
  stale evidence honestly. Public validation never substitutes for the stronger
  author-evidence or release acceptance profile.
- Learner artifacts exclude assessor implementations, withheld transfer tasks,
  private source captures and author solution analysis.
- Continue authorized, reversible work without routine permission questions.
  Do not make unsolicited commits, tags, releases, pushes or publications. Do not
  enable automatic commit/push hooks or change privileged PMU/system policy.

## Installed author tools

Use `python3 tools/author-tool TOOL ...`; pinned versions and restoration are in
`tools/author_tools.lock.json` and `planning/author_tools.md`.

- **Graphify:** local code navigation at `build/author-tools/graphify-out/`.
  Query relationships when useful and confirm actual source before edits. Its
  derived code-only index may be stale; it is not a curriculum coverage map or
  evidence receipt. Do not upload code to an LLM provider or enable a remote graph
  service without an explicit request.
- **OpenSpec:** change proposals/scenarios under `openspec/`. Refer to canonical
  obligation IDs; no competing completion authority or compulsory reapproval of
  already authorized implementation.
- **Spec Kit:** templates and constitution under `.specify/`; use for detailed
  specifications or cross-artifact review when useful. Existing contracts and
  the approved revision take precedence over template defaults.
- **Beads:** local operational coordination under `.beads/`. Use canonical work
  references; task closure does not replace the obligation ledger. The wrapper
  disables telemetry and automatic pushes. Initialization runs outside Git because
  upstream init otherwise attempts a commit.
- **Superpowers:** project skills in `.agents/skills/` for planning, debugging,
  review and verification. Apply relevant skills within the approved scope.
  Existing user authorization takes precedence over generic approval, worktree
  and commit workflow steps.

No Git hooks or remote graph services were enabled by this setup.
