# PR-T37-03 technical analysis (withheld)

T37.01.a: the retained clock commits exactly eleven original instructions.
Input r1=0 before the first branch changes its target to4; after the branch,
that committed target cannot change. The API rejects an input during a live
load request. These are explicit serialized boundaries, not a hidden
asynchronous update within the old-state evaluation.

T37.01.b: the toy has byte registers and instruction indices. Actual imported
CMP primitives have32-bit operands and native flags; their equality result
agrees with byte SUB zero for admitted0..255 operands. It does not license
claiming the other flags, encoding, register width or timing are the same.
Z/ZF means zero result in the inspected manuals. The new toy policy updates
it on SUB only. The ZERO mutant proves a diagnostic distinction, and HALT
preservation tests prevent accidental update by unrelated transitions.

T37.01.c: classify actual command evidence by what was observed. A linked
image demonstrates link closure; a QEMU pass demonstrates selected emulated
execution. Neither is physical observation. The provided fourteen-edge board
scenario is intentionally synthetic and labeled so. Reinspect the supposed
mapping between GPIO edges and model commits, clock/divider/probe configuration,
interrupts, latency and binary identity. Preserve the bounded arithmetic result.

T37.05.a: run the actual original sc_clock on a local copy. An LD proposal owns
the captured byte and control while pending, but has no committed reg write.
Ready alone publishes once; cancel takes precedence over ready and clears
owned work. After publication, no-pending is a stable response, not rollback.
The first issue tick plus one ready tick makes12 ticks for11 commits. Stalls
increment only ticks. Reject counter exhaustion before any output change.

T37.05.b: operand mux values and proposed enables are combinational evidence.
The trace's committed bit separates them from effective architecture updates.
Budget4+7 reaches the same final state. An invalid third instruction retains
two earlier commits; incrementing the live PC before validation violates
clock atomicity even when an ordinary trace later happens to match.

T37.05.c: the speculation slot owns a branch snapshot and one candidate.
Neither candidate stores nor candidate errors are published at predict time.
Resolve commits the branch exactly once. A wrong prediction clears all scratch
even if it contained a store or invalid instruction. A correct fault reports
partial branch progress; the selected invalid successor remains uncommitted.
A correct valid successor stays owned until separate retirement or discard.
The full origin/current-state comparisons detect legitimate intervening input;
they are not a capability against hostile structure mutation or illicit owner
reinitialization. Exclusive ownership remains a precondition.

The SQUASH mutant copies the wrong-path candidate into committed state and
fails the exact comparison with the independently clocked branch snapshot.
CANCEL reverses the simultaneous-signal precedence. INPUT silently drops the
external event. The two original producer mutants remain actual compiled
controls and are not replaced with synthetic failure exits. All assert markers
are verified; failing an earlier unrelated assertion is insufficient evidence.

The independent Python model uses tuple/control tables and arithmetic, while
the real C producer drives actual state and the native/Thumb assembly supplies
the selected ISA comparison. The same author wrote these checks; this is not
independent examiner evidence. The original delayed tasks remain learner work
despite their author implementation and finite campaign.
