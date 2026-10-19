# Current T24 signal successor evidence

This is an author-side execution record, not a learner assessment or whole-T24
completion claim. `signals_validation.json` records136 successful commands and
12 ordinary/changed observations for the six T24.01–02 leaves, with actual source
and retained artifact hashes. It ran on the recorded WSL Linux host with GCC15.2,
Clang21.1.8 and glibc; the JSON contains exact versions and argv vectors.

```sh
python3 tools/run_events_followthrough.py --module signals \
  --report reviews/v5/expansion/legacy_events/signals_validation.json
python3 tools/check_events_followthrough.py \
  --report reviews/v5/expansion/legacy_events/signals_validation.json
python3 tools/close_events_followthrough.py \
  --report reviews/v5/expansion/legacy_events/signals_validation.json
python3 tools/check_events_exports.py
```

The campaign passed GCC/Clang O0, O2/NDEBUG and ASan/UBSan public tests and all13
process/thread modes per profile, four acquisition cuts per profile, five
compiled semantic mutants, a compiling learner-stub rejection, receiver TSan
and two generated handler audits. The independent bounds are160,801 deadline
pairs and9,841 counted-queue histories per profile. The real blocking cases use
owned-child syscall enrollment, signal acknowledgements and child gates; no
enrollment assertion rests on a guessed sleep interval.

The first campaign stopped after61 commands because an actual generated handler
imported `__stack_chk_fail`. That failed receipt is retained under `runs/`. The
selected original teaching-port translation now explicitly disables that implicit
dependency with `-fno-stack-protector`; the final object audits find only the
declared write/errno handler calls and no atomic helper. This does not change
host policy or establish that arbitrary mitigation runtimes are async-safe.

The source/evidence checker passed14 falsification controls, including missing
changed tasks, skipped modes, stale sources, missing objects and substituting a
crash for a semantic rejection. The actual PR-T24-01 CLI export passed with
unchanged original criteria/seven-day intervals, public scaffold identity and
assessor exclusion. A transient unmatched parenthesis in the new export producer
was fixed before that successful export; no execution was inferred from it.

`research/v5/expansion/legacy_events/support.json` attaches12 exact dispositions.
Twenty-eight other original T24 leaf criteria still require implementation beyond
the two previously supported terminal criteria. Published-C17 source access,
external review, independent/delayed learner evidence and physical extrapolation
remain separate. The unsafe handler is rigorous analysis, not a fabricated UB
output, privilege escalation or arbitrary-code-execution demonstration.
