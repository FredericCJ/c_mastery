# PR-T37-03: Commit boundaries, pending loads and speculative ownership

This required additive contract retains the six T37.01/T37.05 leaf criteria
and their original `AX-PR-T37-03-...-V1` tasks. It also implements the author
mechanisms of X-T37-002, R1–R6, and the exact original V1/V2 variants. It does
not rewrite any original task or satisfy independent seven-day performance.
Use it with the parent and legacy shared contracts. Source passages are in
`research/v5/expansion/legacy_architecture_commit/resources.json`; technical
analysis remains private in `assessor/architecture_commit/ANALYSIS.md`.

## Prerequisites and environment

Bring the actual completed `sc_clock`/`sc_run` producer and the selected
`aa_isa_compare` assembly from PR-T37-01. The original four-register/16-byte
machine contract in `labs/systems_certificates/CONTRACTS.md` remains authoritative.
Understand old-state reads, unsigned modulo256 arithmetic, absolute instruction
indices and per-clock versus whole-run transactions before adding a pending
operation. The predecessor is actually linked, not replaced with a similar API.

Use ISO C17 with eight-bit bytes and 32-bit counters, GCC/Clang host profiles,
SysV x86-64 and the dedicated MPS2 Cortex-M4 emulator. The new core uses scalar
C, actual typed state and the existing clock function; it contains no library
interpreter, speculative engine, allocator or thread primitive. Host Python and
sanitizers are verification dependencies. Audit target map, program headers
and final undefined symbols. GNU startup/assembly is a separate target layer.
Exact final C17/C23 remains a source gap; no unsupported edition equivalence is
claimed. A toy clock is a model transition, not a physical cycle.

## Problem and retained ordinary behavior

Run the unchanged seven-instruction LI/LI/SUB/BNZ/ST/LD/HALT program and emit
all eleven transitions. Preserve stable HALT, source/destination alias9−3=6,
invalid LD16 rejection, budget4+7 continuation and a decode error after two
commits. The last case retains the two successful commits but must not alter
state/control for the failed third clock. Demonstrate both original mutations:
reading a prematurely zeroed destination and changing PC before validation.

Implement `commit.h` in the learner scaffold. `struct at_machine` owns a committed
`struct sc_machine`, one pending load snapshot/control, zero-status bit and model tick
count. `struct at_trace` carries the proposed control wires, read operands, whether a
commit actually occurred, phase and tick count. A proposed reg-write bit is
not an effective architectural write unless `committed` is1. No pointer into
caller program storage is retained; the immutable program's lifetime still
must span the call. All object/control/output storage is live and disjoint.

Internal state is established by `at_init` and subsequently changed only by
these operations under one serialized owner. Forging a pending snapshot or
reinitializing a machine while a speculation object owns its origin is outside
the contract. Structural admission does not make arbitrary corrupted internal
state safe. External events use the explicit input operation.

## Operations, invariants and failure semantics

`at_init(out,state,zero)` accepts pc0..15, halted0/1, zero0/1 and copies the
initial committed state into a fresh owner. It clears pending ownership/ticks.
`at_input(machine,reg,value)` accepts reg0..3/value0..255 only while idle and
not halted. It changes that register between instruction commits and leaves
the zero bit unchanged. An event before BNZ is visible to that branch; an
event afterward affects only later instructions. No input arrives inside a
transition or changes the captured value of a pending load.

`at_begin` evaluates the actual original clock on a local state copy. Invalid
instruction/admission preserves machine and trace. HALT already committed
returns `AT_DONE` unchanged. A successful non-LD commits state once and adds
one tick. A successful LD adds an issue tick, stores its complete proposal and
control, and returns `AT_PENDING` with architectural state unchanged. The load
captures the selected memory byte at issue. `at_input` and a second begin are
rejected while this request owns its snapshot.

`at_poll(ready,cancel)` accepts Boolean signals. With a live pending request,
each admitted call adds one tick. Neither signal means a stall: state remains
uncommitted and owned. Cancellation wins when both are1; it clears every
pending snapshot/control and returns `AT_CANCELLED` without changing committed
state. Ready alone publishes the proposal exactly once and clears ownership.
After completion/cancellation, poll returns `AT_NO_PENDING` and changes
nothing; late cancellation cannot roll back a commit. Tick or commit-count
exhaustion is rejected before wrap. The eleven-commit fixture takes twelve
ticks with an immediately ready load after its issue; stalls add ticks only.

The documented ISA-visible zero relation is result==0. This toy extension
updates `zero` on SUB only and preserves it on every other opcode. Real
instructions have their own flag-update rules. Compare the relation for all
65,536 byte operand pairs with the actual native/Thumb CMP producer, whose
equality flag is equivalent for these operands. This does not equate toy byte
register width, instruction-index PC or timing with either real architecture.

`struct at_spec` is one caller-owned speculative slot, initialized to all zero.
`at_predict` accepts an idle nonhalted machine currently at BNZ and a predicted
in-program next index. It computes the branch proposal and one successor in
private snapshots. A bad speculative instruction is retained as a deferred
fault; no architectural state or memory changes. Prediction never retains a
program pointer. No prediction is accepted into an occupied slot.

`at_resolve` first compares complete committed state with the saved origin;
changed state returns `AT_STALE` without publication. It commits the branch
once. A wrong prediction returns `AT_SQUASHED`, discarding even a speculative
store or fault. A correct invalid successor returns `AT_FAULT` after the branch
commit, with the bad successor still uncommitted. A correct valid successor
becomes owned retirement work (`active=2`). `at_retire` compares current state
with that saved branch state and, if unchanged, commits the successor on a
separate tick; it then clears the slot. A stale retirement changes nothing.
`at_discard` explicitly clears owned work without touching the machine.
These APIs make branch progress on `AT_FAULT` explicit; they do not promise a
whole-resolve transaction. No host memory store is hidden in speculative work.

## Required reasoning and necessity

Explain the representation and each ownership transfer: idle→load-pending→
commit/cancel, and empty→predicted→resolved→retired/discarded. Draw the operand
mux, ALU, proposed write enables and PC selection separately from stored state.
Derive why destination/source alias reads the old value, why cancellation
ordering changes the result, and why a wrong-path store cannot escape scratch
state. Define the architectural commit point for every API and distinguish
an input event from an instruction commit. All queues/snapshots are fixed
size, operation cost is O(1) under these bounds and no heap resource exists.
Measure actual stack/code size separately from that asymptotic argument.

Specify preconditions/postconditions, complete failure outputs, invariant
preservation and overflow headroom. Refute: “a wire changed, so the register
changed”; “successful validation makes prediction architecturally real”; and
“eleven commits measure eleven hardware cycles.” ISO C governs the model's
arithmetic/objects; the API governs cancellation and speculation; manuals
govern selected ISA flags; physical timing would need distinct observation.

## Deliverables and verification

D1 is a six-leaf state/control/ownership derivation with the three evidence
layers. D2 is the original additive implementation composing the actual two
predecessors. D3 is a source-bound ordinary and changed-condition trace,
independent tuple-based oracle, named mutation diagnoses, helper audit and
resource analysis. D4 is each original delayed task below, with a fresh
independent program after seven days. D5 is the evidence classification dossier.

Run selectors `baseline`, `clocked`, `input`, `status`, `load`, `speculation`.
Require six GCC/Clang debug/optimized/sanitized host profiles, both emulator
optimization profiles, actual native instruction disassembly, final dependency
closure and learner-only export compiling then failing named assertions.
The independent oracle checks twelve ordinary trace/progress rows, twelve
clocked events, the ordered-input counterexample,767 sampled status records
with all65,536 pairs tested in C, eight load histories and36 prediction/fault/
store combinations. The six defined mutants must compile and fail their
intended semantic assertion rather than an unrelated early failure.

Inspect `evidence.json`: source, disassembly, host execution, target link,
emulator execution and the hypothetical board scenario are different evidence
classes. The last packet is explicitly original synthetic training data, not
a captured waveform. A contradictory asserted physical interval prompts clock,
instrumentation, interrupt, pipeline and memory-latency investigation; it does
not invalidate a correctly bounded modulo256 transition derivation.

## Original changed-condition tasks

* `AX-PR-T37-03-T37.01.a-V1` and `X-T37-002-V1` (original local ID `V1`): inject
  r1=0 after the first SUB and before BNZ. The branch now falls through to4;
  injecting after BNZ leaves that already committed target unchanged. Derive
  a fresh event placement and preserve its explicit precedence.
* `AX-PR-T37-03-T37.01.b-V1`: use the inspected Z/ZF definition, implement the
  stated SUB-only update policy and falsify an inverted/nonzero interpretation.
* `AX-PR-T37-03-T37.01.c-V1`: analyze the explicitly hypothetical contradictory
  fourteen-edge interval using the evidence packet and named assumptions;
  retain the valid eleven-commit model result. Actual board data is separate.
* `AX-PR-T37-03-T37.05.a-V1` and `X-T37-002-V2` (original local ID `V2`): add
  stalls, cancel before ready, simultaneous ready/cancel and cancel after
  publication. Compare twelve-plus-stall ticks with eleven commits.
* `AX-PR-T37-03-T37.05.b-V1`: inject an invalid third instruction after two
  commits; retain partial run progress and unchanged failed-clock state/control.
* `AX-PR-T37-03-T37.05.c-V1`: predict a successor that would store to memory,
  squash it on branch mismatch, defer wrong-path faults, and detect changed
  origin/retirement state. Explain who owns each scratch snapshot until cleared.

## Rubric, hints and containment

Assess the original six rubric criteria separately. Level0 breaks a state or
ownership rule; level1 repeats a supplied trace; level2 derives the specific
commit boundary and verifies ordinary/boundary behavior plus the named defect;
level3 independently handles the exact changed condition, creates a new
counterexample and identifies surviving limits. The X-T37-002 six criteria
use the same objective-specific evidence, not a blanket pass from one test.

Graduated hints: first mark every stored field and its sole update point;
then run the pure predecessor on a copy; finally keep speculative side effects
in separately owned scratch until the branch and origin checks succeed. For
load cancellation, write the simultaneous-ready/cancel row before coding.

The only targets are original toy state and dedicated emulator images with
synthetic instruction arrays. Adversarial inputs are bounded integer indices,
opcodes and event schedules, not forged native pointers. A harmless violation
is a wrong synthetic register/store or forbidden commit. No third-party
program, real data, security setting or physical device is touched. No crash
or undefined behavior is offered as proof of exploitation.

## Validation and limitations

Content is authored. Actual author execution is only the current source-bound
`legacy_architecture_commit/validation.json` and real CLI `exports.json`.
The core is an original bounded deterministic model; it is not a commercial
processor, a physical pipeline or a concurrency memory model. Passing finite
checks cannot establish absolute mastery. External assessment, independent
learner performance after the required delay, exact final C17/C23 source text
and physical observations remain unclaimed.
