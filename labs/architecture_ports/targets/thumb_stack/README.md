# AP5: Thumb public-call alignment and stack admission

Required PR-T58-01 transfer; objectives T58.01.a/b/c; families A/B/C/D/F/H/I/J.
The six-deliverable, levels0–3, evidence, containment and seven-day independent
transfer rules in `curriculum/v5/contracts/b06.md` apply. Bring the learner's
PR-T58-01 integer stack model, linker map and original RV64/AArch64 guest evidence.
Read R-B06-FOLLOWUP-AAPCS32, exact registered stack constraints.

## Problem and invariants

Implement `unsigned ts_probe(unsigned requested, unsigned *observed)` in Thumb-1
assembly for Cortex-M0. `start.c` supplies a live output on the caller stack;
`link.ld` supplies a separate128-byte stack preceded by16 guard bytes. Choose an
eight-byte aligned candidate at or below `test_top-requested`; reject requests
beyond128 and candidates below `test_low` before modifying SP or output. On
success switch to the candidate, call a separately visible leaf that checks the
actual public-call alignment, write one word at SP if the reservation is nonzero,
publish the observed SP and restore the caller's SP and preserved registers.
The zero reservation performs no stack-memory write. No allocation or target
library is permitted. Ordinary C is C17; address casts are specific to the
selected32-bit target. The assembly/ABI rules are not ISO C stack guarantees.

The public harness checks all0..128 requests, guard integrity, caller SP and
three rejected requests129,136,UINT_MAX with unchanged output. AAPCS32 requires
four-byte alignment generally and eight at public interfaces; neither pointer
size nor the other ports' sixteen-byte requirement supplies this derivation.
Do not use a canary check after an unchecked reservation as admission logic.
All addresses and linker intervals must be derived without arithmetic wrap.
Maskable interrupts are disabled for this synthetic probe. Exact exhaustion has
no exception-frame or nested non-leaf headroom; NMI/fault recovery at that point
is outside this contract. The called alignment leaf does not allocate a frame.

## Deliverables, verification and assessment

D1/D2 provide interval/alignment arithmetic and source; D3 contains actual QEMU
microbit commands, ELF/map, disassembly, output and undefined-symbol audit. D4
shows rejection of four-byte alignment and exclusion-of-exact-fit mutants, each
by a semantic assertion, plus the unsolved scaffold rejection. D5 accounts for
128 reserved bytes,16 guard bytes,24-byte caller save frame and zero-byte leaf
frame; D6 explains interrupt headroom and the distinction between guard contents,
admission and memory protection. Run `python3 tools/run_b06_thumb_stack.py` for
separate assessor controls. The author receipt is `thumb_stack_validation.json`.

Assess `.a` by the bounds/alignment proof; `.b` by all boundary executions and
preservation; `.c` by diagnosing both mutants and a new ABI derivation. Level1
means one working call; level2 requires every stated invariant and negative
control; level3 requires unaided AP5-DELAYED after seven days: enable a named
exception-entry model, reserve its actual frame/alignment headroom, and justify
the new admission limit. Merely changing128 to another constant is insufficient.
Hints: draw the descending interval; account for rounding before committing SP;
keep the caller's saved state on its own stack. Separate technical analysis is
`assessor/architecture_ports/targets/STACK_ANALYSIS.md`. Only original guest
memory is touched; no board flashing or host settings changes. Emulator author
execution is not physical validation or an independent learner assessment.
