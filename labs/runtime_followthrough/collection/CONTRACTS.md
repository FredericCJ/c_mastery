# T46.07/08: identity, phase storage and precise collection

This required successor supplies executable work for the unchanged criterion IDs
`AX-PR-T46-07-T46.07.{a,b,c}` and `AX-PR-T46-08-T46.08.{a,b,c}` and their
`-DELAYED` variants. Original project criteria and accepted receipts remain
historical. The separate successor evidence records which work was executed.

Prerequisites are the CVM1 value/image contract, fixed pair heap, T13 graph
reachability and T74 ORGraph/ORVM root registration with T13 generational pools.
First construct the original fixed heap and stop-the-world collector from their
permitted primitives. Then import those **learner-owned** components for this
composition; implementing collection by calling an external GC library is not
permitted. The author campaign names its assessor providers explicitly and is
not evidence of learner integration.

Use C17. C23 does not make a copied C structure an owning reference, add tracing,
or make generation counters unbounded; no C23-specific facility is needed here.
The native profiles are hosted only for test I/O. The core is freestanding:
caller storage, fixed arrays, no allocation, locks, libc, HAL or runtime archives.
The optional M4 builder requires explicit predecessor files, compiler headers,
original byte/division helpers and a teaching output driver. Headers supply
declarations; their libraries must not be linked. Inspect every object's imports,
the linker map, final undefined-symbol closure, disassembly and stack records.
PPC64 ELFv2 big endian is a separate executed profile, not all-ABI evidence.

## Representation and invariants

The original pair handle contains slot and full32-bit generation. Allocation
increments generation; reclamation leaves it intact. A free slot at generation
`UINT32_MAX` is retired. Reset starts another lifetime and can repeat the same
slot/generation. `VgRef` therefore also carries the address of an immovable,
live `Vg` controller and its nonwrapping epoch. Zero-initialize the controller
before its first reset; never copy a live controller. Native code can forge these
fields, so this is a checked ownership contract, not authentication or memory
isolation. Destroying the controller invalidates all references regardless of
numeric fields. All arguments and output storage must be live and disjoint
from the controller; operations are serialized and its underlying VM is stopped.
Do not mutate internal state except for clearly labeled valid boundary fixtures.

`Vg` owns the original VM, actual ORVM registration pool, two-client lease
directory, four weak slots and an ORGraph. There are16 heap slots and16 root
registrations. A lease names controller, epoch, ticket and client1 or2. A ticket
is an actual generational-pool identity; releasing one client's lease cannot
release another's. References and leases are different: copying a reference
does not acquire a root. Reset refuses any live lease or active mark cycle.
`vg_reset(g, arena)` increments epoch and initializes an empty heap; it does not
transfer survivors. Refuse epoch exhaustion rather than wrapping.

`vg_pair` checks both operands, returns a new reference only on success, and
refuses allocation while a mark cycle is active. In arena mode occupied slots
are never reclaimed. In collection mode, full-heap allocation temporarily roots
**both** arguments, completes a collection, then retries allocation. Collection
may reclaim garbage even if the eventual allocation still refuses because all
remaining free identities are retired. Do not promise full-heap rollback for
that case. The allocation counter also refuses exhaustion. Other output
parameters remain unchanged on refusal; scratch marks/cost counters are not a
transactional snapshot. Both temporary roots must be restored on every return.

`vg_begin` validates used pair fields and the root representation, copies a
graph and starts marking. Its root sources are the VM operand stack, explicit
roots, **initialized** frame slots, registrations and active allocation
temporaries. It deliberately validates even unreachable used pair fields; this
is stricter than the original collector. A C local is invisible unless one of
these mechanisms exposes it. `vg_step` scans at most the requested number of
objects. Zero budget may report DONE when no gray object exists. READY means
the current gray set is empty; it does not disable barriers before commit.

White means unseen, gray means reached but not scanned, black means scanned.
While marking, a store from a black object to a white object must shade the
target. Adding a root after the last step must also shade it. `vg_store` and
lease changes reopen MARK; the underlying ORGraph barrier remains armed even
through READY. No allocation is permitted during the incremental cycle. Root
removal can leave floating garbage until the next cycle; that is allowed.
These are serialized mutation boundaries, not a concurrent collector.

`vg_commit` requires complete marking. Weak processing receives the already
proved black set without re-traversing strong edges; otherwise it could hide a
missing write barrier. Clear dead weak references before freeing cells.
Weak acquisition first obtains a strong lease, then publishes the reference,
without an allocation or yield between those steps. No ephemeron/finalizer
semantics are promised by this adapter. They remain in the T74 work.

The original collector's work units are root examinations, two fields for each
reachable pair and16 sweep cells. A failed mark pass must never sweep; a retry
clears all marks. The incremental adapter instead records validation inspections,
object scans and commits separately. ORGraph searches a fixed array for gray
objects, so at variable heap capacity H its scan search can take O(H²), despite
at most H object scans. Neither unit count establishes a wall-clock deadline.
Guest instruction fuel does not pay for all collector or native callback work.

`vg_transfer` moves one reachable graph between different stopped arena
controllers. Both remain live; output and all destination semantic fields are
failure-atomic. First discover reachable identities, allocate destination
placeholders, then rewrite edges using a forwarding map. This preserves cycles
and aliasing and uses destination generations. Do not transplant source slot
numbers. Keep unrelated destination objects. The source is unchanged even on
success, so its owner chooses when to reset it. Capacity, retired identities and
allocation-counter exhaustion can refuse the operation. For R reachable cells,
fault cuts1..3R+1 follow each allocation, each field rewrite and the last point
before publication; cut0 disables injection, larger cuts are invalid. A staged
whole destination VM costs `sizeof(Cvm)` temporary storage; array discovery plus
bounded allocation scans is O(HR), with O(H) forwarding/queue storage.

## Required construction and investigations

For each row submit the original criterion's D1–D4, code, initial prediction,
state trace, source-bound argument and commands. Read the actual interfaces;
do not infer contracts from matching function names.

| Objective | Concrete investigation and observable obligations |
|---|---|
| T46.07.a | Allocate/reclaim/reuse slot0; compare old/new generations and rejected old access. Set a free valid slot to the penultimate generation, allocate once and reclaim: it must never be reused. Reset and demonstrate the old raw-handle collision; implement controller epoch checking. Refusal must preserve the output. |
| T46.07.b | Fill16 slots with unrooted scalar pairs. Report occupied16/reachable0/reclaimable16 separately. Arena allocation17 refuses; a stopped raw-VM switch to collection mode permits reclamation. Build phase transfer for a two-node cyclic graph with duplicate edges into an arena containing a retained object and retired hole. Exercise every fault cut and insufficient capacity. |
| T46.07.c | Return a pair to native code, then allocate at a full heap without registering that result. Observe the stale guest handle. Repeat with a root and balance it on failure. Define reset and arena-to-collection ownership explicitly. Implement independently scoped leases for two clients, duplicate/cross-client release refusal and pool-generation retirement. |
| T46.08.a | Derive the exact root set and graph closure with cycles, aliases, operand/explicit/frame roots and uninitialized local slots. Bound the original collector's worklist by heap objects, not edge count. Implement separate weak clearing and upgrade-to-strong semantics. Compare an independently constructed graph model. |
| T46.08.b | Fill the heap, retain two children only in C locals and allocate a parent. Both must survive. Start with seven explicit roots so the second temporary-root push fails; verify restoration to seven and no parent. Add a helper allocation between evaluating left and right operands; root left before entering the helper. |
| T46.08.c | Use a fixture with three root examinations and four reachable pairs: derive its exact work requirement. Cut the budget at every value below completion, compare occupancy/generations, then retry. Add incremental stores and roots between the final mark step and commit; construct the black-to-white counterexample and repair it. |

`tests.c` supplies concrete initial states and assertions. The graph fixture is
bounded; the assessor has a separate oracle and withheld mutations. Add your own
disconnected cycle, empty graph, all-live heap, last registration, scalar transfer,
full destination and rejected forged/stale-state cases. Do not simply print the
expected PASS markers. Provide typed-state comparison; ISO C does not prescribe
portable structure-padding bytes.

Run `python run.py --cvm YOUR_CVM --objects YOUR_OBJECT_RUNTIME
--registrations YOUR_ORVM --pool YOUR_POOL --build OUTPUT` from this directory
(join the command onto one line). The default learner file intentionally refuses
operations. All four providers are mandatory. Target invocations are in
`target_build.py --help`; no missing provider may silently select assessor code.
Its checked `port_registration.py` transformation is the explicit versioned
enum-signedness correction R-VG-ORVM-ENUM-2026-01 for the retained ORVM source:
it changes only one exact tag-range guard, records both hashes and leaves the
provider untouched. A changed input must be inspected, not blindly rewritten.

## Hints, assessment and evidence limits

Hint1: label each identity's scope and list all roots before drawing heap edges.
Hint2: stop immediately before an allocation or sweep and ask which reference
can still be found. Hint3: write one invariant for each color transition; then
try adding an edge after all original roots have turned black. For phase
transfer, allocate all destinations before installing any cyclic edge.

The criterion rubric remains0–3. Level0 has wrong contracts or unsafe publication;
level1 reproduces public cases without causal reasoning; level2 derives the
ordinary invariant, diagnoses a compiling faulty implementation and supplies
independent evidence; level3 implements the changed mechanism, revisits ownership
and cost, and transfers it without copying the assessor. A false claim that C
checks lifetime, that weak means strong, that a crash proves code execution or
that tests prove absolute safety fails the associated objective. Record each
objective separately; success in allocation does not assess barrier reasoning.

The six existing delayed IDs retain their original tasks and minimum seven-day
separation. The assessor administers them separately, withholding analyses and
implementation. Author execution of the prepared variants is not a learner
attempt or a seven-day result. See the objective map and current evidence report
for exact validation state; authored code alone is not accepted evidence.

Security boundary: only these original local teaching heaps and synthetic values
may be manipulated. The harmless success condition for a flawed collector is
loss of a required guest reference or retained unreachable cell; neither is
native arbitrary code execution or privilege escalation. No external targets,
host settings or deployed equipment are involved. Repair root publication,
barriers and epochs, rerun the causal counterexample and retain the regression.

Primary narrative: Nystrom, *Crafting Interpreters*, “Garbage Collection,” root
classification, less-obvious roots, tricolor invariant, weak-reference timing and
C-temporary hazard. The exact inspected passages and local providers are listed
in `research/v5/expansion/legacy_vm_collection/resources.json`. This adaptation
uses a fixed caller-owned educational heap, not Lox's complete implementation.
