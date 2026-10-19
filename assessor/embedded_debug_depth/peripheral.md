# PR-T31-03 technical analysis

The configuration model deliberately maps wrong mode and wrong route to the
same software signature. A correct diagnosis reports this ambiguity. Changing
several switches until the LED moves is not a minimal causal intervention.
No predicate on a register readback can prove which connector carries an edge.

The correlation map is an affine map with an externally supplied scale/origin.
Integer arithmetic is deliberately exact for this model. Refuse negative tick
offsets and each finite overflow; preserve the old result. Interval overlap
only tests compatibility. Provenance cannot be promoted from MODEL/PREDICTED
to MEASURED by successful arithmetic. Analog/timebase bounds remain premises.

The frozen PuDriver intentionally discards a pre-write TC observation. Its last
buffer read releases memory while line ownership persists. PsSpi's DR read is
a side effect, and its final BSY check follows receive acknowledgement. The
master completion predicate must not inherit the slave-specific erratum rule.
For the selected slave policy, only a known last item or a justified final-frame
bound can replace BSY. Old BSY-only teaching is a falsified assumption, not
evidence that the actual master driver must be weakened.

PD cancellation is a selected software outcome. Whether it wins depends on
admitted adapter order: cancel while running→CANCELLED, poll reaches ready
first→OK. These are not assertions about which electrical event happened first.
PD_IO can supersede a cancellation when an actual transfer error is observed;
the exercised clean cancellation case does not erase that producer rule.
Quiescence precedes finalization and output publication. A stale snapshot is
ignored without changing generation2; caller use of old token is STALE. The
cached test's speculative clean refill is a hardware-model event, not an
authorized CPU payload read while DMA owns it. Removing final invalidation
exposes99 in a byte whose DMA backing value is1. Removing pre-clean loses the
untransferred tail; the current campaign tests that invariant positively, while
the post-invalidation omission is the separately compiled control.

The watchdog model explicitly orders reset before a same-tick CPU feed. With
firstseven halted ticks, DMA length6 and budget3, unfrozen reset occurs3 after
three beats; frozen DMA finishes6, observed8, with five later feeds. This small
model demonstrates why a debugger can hide a reset failure without stopping
DMA. It does not claim every F439 debug/peripheral interaction is specified by
the modeled DMA progress rule. A ready physical plan must inspect relevant
freeze bits and acquire actual clocks/signals before asserting equivalence.

Assess every original criterion and its variant independently. Physical
observation is still absent; author models can make that work ready but cannot
complete its observation dimension. No source-copy, oracle agreement or finite
mutation set establishes independent mastery or all executions.
