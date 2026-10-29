# PR-T37-04: Dynamic stage instances and changed scheduling assumptions

This required extension preserves the three original T37.06 criteria and V1
tasks. It also implements X-T37-001's original write/read-order V1 and
structural-memory-port V2. Use the original project and legacy shared contract.
Exact inspected sources are registered in
`research/v5/expansion/legacy_architecture_pipeline/resources.json`; private
analysis is withheld under `assessor/architecture_pipeline/ANALYSIS.md`.

## Prerequisites and environment

Bring your completed `s_pipeline` retired-stream scheduler and its recorded
source identity. Read its exact existing contract: stalls there are inserted
before equivalent IF issue, and wrong-path instances do not exist in that
representation. Reuse it only for compatible retired-work/cycle comparisons.
Understand RAW, old/next-state updates, output admission and immutable programs.

The new core uses ISO C17 scalar arrays/structs and type headers. No pipeline,
scheduling or graph library may implement the mechanism. Host GCC/Clang and
ASan/UBSan, Python certificate checks, disassembly and symbol tools are separate
verification dependencies. The dedicated MPS2 Cortex-M4 image executes this
C model; it does not expose the Cortex-M4 physical pipeline. Actual target
helper closure and host function-frame estimates are different evidence.
N1570 is inspected C11 draft evidence for data-race terminology, not final
C17/C23 edition inspection. No real instruction stream or external system is
attacked or modified.

## Problem and representation

Implement both functions in `pipeline.h`. `struct ap_schedule` records each cycle's
IF/ID/EX/MEM/WB occupants by fetched-instance ID, with a separate program-PC
mapping, branch-resolution cycle, flush cycle and retirement sequence. An
instruction fetched on the wrong path and its later correct refetch have
different IDs. All indices must be checked before access. An empty slot is−1.
Unused output fields are initialized deterministically; rejection preserves
the entire caller output. Inputs and outputs are live, immutable/disjoint for
the call; forged pointers and falsely claimed backing extents are outside the
contract. No input pointer is retained and no allocation occurs.

The admitted program has0..16 records. Registers0..7 have constant r0.
Operations are ALU, LOAD, BRANCH. Nonbranches have target/taken0. Branch dst0,
taken0/1 and a strictly forward target inside the program are required even
when not taken. `taken` is an explicit selected control outcome, not a
computed real-processor prediction. Backward loops, exceptions, stores and
uninspected opcodes are outside this finite stage model. Invalid input returns
`AP_INVALID`; exhausting the128-cycle/64-instance storage limit returns
`AP_LIMIT` without publishing a partial certificate.

Use the original seven-record dependency chain: I1 ALU→r1; I2 ALU fromr1→r2;
I3 LOAD fromr2→r3; I4 ALU fromr3→r4; I5 taken branch readingr4, targetI7; I6
wrong-path ALU; I7 target ALU. The original default schedule must match every
stage cell in `fixtures/algorithms/PR-T37-04.json`, not merely its final cycle.

## Stage rules and changed configurations

Cycles begin with the displayed stage occupants. WB retires at that cycle.
ALU results are available after the first EX cycle; load data after the final
MEM cycle. Forwarding supplies the next cycle's EX, never same-cycle MEM→EX.
When forwarding is off, operands are read in the final ID cycle after producer
WB under write-first order, or a later cycle under read-before-write order.
Writes to r0 create no dependency. The nearest older unsquashed producer owns
the value; a register-name match alone does not establish readiness.

Each stage holds at most one instance. ALU/branch MEM takes one cycle; load
MEM takes the configured one or two cycles. A blocked MEM holds EX and then
younger stages as necessary. A blocked ID retains its instance while EX can
contain a bubble. The trace records occupancy, not a second retirement of a
stalled instruction. WB never stalls in this model.

Default branches resolve at the end of their first EX cycle and squash current
younger IF/ID instances. Target refetch can begin next cycle. The changed ID
model has **no forwarding into ID**: its branch waits for register-file
availability under the selected write/read order, resolves in ID, and squashes
only IF. Branches still flow through later stages once. Earlier resolution can
therefore introduce extra operand waiting; it is not an unconditional speedup.

A unified port lets data LOAD MEM and a new instruction fetch share only one
port; data has priority. Holding an already fetched IF instance consumes no
new memory access. A fetch vacancy during LOAD MEM remains empty. This can
overlap an existing data stall and add no new cycle. Do not add two independent
penalties when they describe the same blocked interval.

`struct ap_config` explicitly selects forwarding0/1, load MEM duration1/2,
write-first0/1, ID branch resolution0/1 and unified port0/1. All32 combinations
must honor their declared rules. Default: six retire by13, I3 MEM6→I4 EX7,
two instances flush at8, target refetch9. First four retire by9 with forwarding,
14 without, and17 without forwarding under read-before-write order. Two-cycle
load MEM extends the seven-record default to14. ID branch resolution with
write-first and no ID forwarding resolves at9 and also ends at14, with one
flushed fetch. A separate four-instruction independent stream exposes a shared
port conflict: one lost fetch opportunity, last retirement9 instead of8.

## Out-of-order changed semantics

`ap_ooo` is an explicitly different straight-line model, accepting ALU/LOAD
only, eight initial byte registers with r0=0 and16 memory bytes. ALU computes
(left+right) modulo256; LOAD reads memory[(left+right) modulo16]. At decode,
each source binds to the most recent earlier producer version, or initial
register value. Results live in per-instruction slots; architectural writes
occur only on in-order retirement, so WAR/WAW names do not corrupt operands.

Issue at most one oldest-ready instruction per cycle. ALU latency1; load
latency1..4. Consumers issue only after producer completion, not in the same
cycle. The single nonpipelined load port stays occupied through completion;
independent ALUs can issue while it is busy. Retire at most one instruction per
cycle, in order, no earlier than the cycle after completion. Record issue,
completion, retirement, producer tags and values. Admit the entire input first;
no output publishes until all work retires. Empty input returns initial state.

With load latency3, LOAD I0 issues1; dependent ALU I1 waits; independent I2
issues2; I1 issues4. The original assumption that a blocked consumer holds all
younger issue no longer applies. True RAW remains; version binding removes
false name dependencies. This is neither a commercial out-of-order simulator
nor a branch-speculation or weak-memory model. Those mechanisms remain separate.

## Reasoning, deliverables and assessment

D1 is a per-leaf stage and availability proof with ownership of each fetched
instance and operand version. D2 is the original model and compatible adapter
to the actual predecessor. D3 is full source-bound traces, independent
certificate checks, failure preservation and the six semantic mutation
diagnoses. D4 is every exact delayed task below using a new program after
seven days. D5 explains the evidence layers and resource costs.

Derive stage order, capacity, held-versus-empty invariants, no retirement after
flush, exact refetch identity, data-ready edges, register-file ordering and
output transaction. For out-of-order work, prove tags always refer backward,
completion precedes use, the load port does not overlap and in-order retirement
matches a sequential value oracle. No hidden allocation/ownership transfer is
permitted. In-order work is O(cycles) under fixed five-stage bounds plus output
initialization; renamed scheduling is O(cycles*n), fixed n≤16. Output storage,
compiler function frames and whole-call-stack bounds are separate quantities.
Inspect actual `-fstack-usage` and object-size output; a native frame estimate
is not a target high-water measurement.
Distinguish `static` from `dynamic,bounded` compiler reports: bounded call-site
stack adjustments do not imply unbounded allocation. Require a finite reported
upper bound below8192 bytes for each new core function, and reject unbounded
dynamic reports. Add callees and interrupt frames separately when deriving a
whole-stack budget; this per-function check alone does not establish one.

Run `schedule`, `matrix`, `boundaries`, `ooo`. Compare the full original table,
all32 configuration certificates plus the isolated port counterexample,
empty/invalid/register-zero states, and16 out-of-order workloads covering
independent bypass, true RAW, WAR/WAW versions, r0 and load-port contention.
Use a separate certificate representation and sequential value oracle. Require
GCC/Clang debug/optimized/sanitized profiles, dedicated emulator execution,
actual dependency audit, compiling learner rejection and six intended mutant
failures: early load availability, escaped squash, wrong register-file order,
ignored shared port, lost version tag and the retained producer's load-delay
defect. A compiler error or unrelated assertion is not a successful mutation.

The atomicity, boundary, changed-assumption and independent-assessment facets
are assessed separately for each original leaf. Level0 violates a rule;
level1 repeats outputs; level2 derives the rule and falsifies its defect;
level3 transfers independently with a new counterexample and precise limits.
Finite certificate checks do not prove absolute mastery or a real CPU model.

## Exact variants, hints and terminology

`AX-PR-T37-04-T37.06.a-V1` changes the load MEM duration to two cycles and
requires a new stage/availability proof. `AX-PR-T37-04-T37.06.b-V1` resolves in
ID under the stated no-ID-forwarding rule and derives the new stalls/flushes.
`AX-PR-T37-04-T37.06.c-V1` implements the separate renamed out-of-order model
and explains which in-order proof premises fail. X-T37-001's original local
`V1` changes register-file order; its `V2` introduces the shared memory port.
Author implementation of these mechanisms is not independent delayed evidence.

Hints: first mark the producer's final availability cycle; next follow the
same fetched instance through held slots; finally separate logical instruction
PC from dynamic fetch identity. For out-of-order operands, annotate the exact
producer before simulating issue. For shared-port costs, inspect overlap with
existing stalls before adding any cycle count.

`distinctions.json` and compile-only `race_examples.c` separate three cases:
RAW satisfied without a stall, a timing hazard repaired by a stall, and two
unordered conflicting C threads. The last is a C data race and undefined
behavior under inspected N1570§5.1.2.4p25; it is not executed. The atomic-counter
repair is also compile/analysis evidence only and promises no publication of
unrelated data. Do not call ordinary sequential RAW a C data race.

## Security, validation and limits

Only original synthetic descriptors, certificates and dedicated emulator images
are used. Deliberate defects violate a toy schedule/retirement property; no
speculative-execution exploit or third-party target is involved. A crash is
not credited as a valid diagnostic result.

Current author execution lives in the source-bound
`legacy_architecture_pipeline/validation.json` and learner `exports.json`.
Preserve prior receipts and report failed attempts honestly. External review,
independent learner performance, final C17/C23 source access and physical
pipeline/cycle observations remain unclaimed.
