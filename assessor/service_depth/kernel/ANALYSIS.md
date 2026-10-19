# Service adapter assessment and delayed variants

Use the learner-facing adapters/CONTRACTS.md and original X-T56-002..005 rubrics.
These solutions are assessor material; public guest probes are deliberately
observable. Supplying these reference files as learner imports is not an
independent integration attempt. Award each original objective separately.

## Loader and mappings: SD-KERNEL-LOAD-COW

The acquisition list is an ownership log, not a copy of the free-list head.
Taking a frame removes one free frame; publishing its PTE adds one owned mapping
reference. Before publication, every taken frame belongs to this transaction.
Rollback zeros its bytes, clears its reference and returns it exactly once,
then restores the unowned task descriptor. Static table slots do not add frame
allocations. A failed load must preserve preexisting images and task/thread
counts at every actual allocation index. The leak mutant compiles but fails the
allocator/reference snapshot equality. Returning an error after partial thread
publication is a false repair.

Private clone rejects non-private mapping kinds and saturated references before
the retained no-failure publication phase. The one-live-thread restriction is a
policy prerequisite, not an ISO C guarantee. Hardware invalidation requires
remote acknowledgement. On concurrent faults, checking the PTE again under
the lock is necessary: the observed trap can predate another CPU's repair.
The test holds the child's shared backing alive while two parent CPUs fault.
Simply joining the child before the race would destroy this premise.

V1 replaces the private-writable premise with immutable sharing. The same frame
has two references and no permitted write. The assessor mutant marks it COW:
the child then writes a private copy, demonstrating that isolation and immutability
are different properties. V2's lock/recheck approach allocates only after taking
the lock, so no speculative losing allocation exists. A valid alternative may
allocate before locking, but must demonstrate its loser cleanup explicitly.
Costs: O(mapped pages) clone/rollback,32 stack pages plus admitted image pages,
up to512 transaction entries, O(page size) fault copy, bounded synchronous
shootdown. Inspect compiler stack reports; no emulator maximum is a WCET proof.

## Capabilities and partial progress: SD-KERNEL-CHANNEL

User tokens identify kernel rows. Rights, process ownership and incarnation
remain in protected storage. The rights mutant removes this check and admits
a receive through a send-only capability; the harmless result is the synthetic
byte, not privilege escalation. A blocked send owns copied bytes. Removing
the waiter before cancellation wake prevents later publication of cancelled99.
The cancellation mutant leaves enrollment active and fails the exact check.

A receive reserves the admitted head and preflights the entire actual output
span before consuming it. The blocked receiver stores no user address and
returns RETRY so the caller resubmits. Alternatives pinning address-space pages
are valid only with separately demonstrated pin/unmap/exit ownership. This
target has no unmap syscall; do not imply a general pinning proof.

V1 gives bytes, consumed offset, reservation owner and end-of-message marker
distinct meanings. The expected trace is [1,2], rejected copy with no progress,
[3,4], [5]+END. The partial mutant never advances the offset; the second successful
read returns [1,2] and fails the suffix check. Empty messages still end exactly
once. V2 removes a dying sender's pending copy and then reuses the queue slot;
the old incarnation wake is stale and cannot publish into the new queue.
Reason about a cancellation that loses to completion: reporting success in that
case would falsely imply rollback. Fixed capacities bound memory and admission;
cleanup/wake scans are O(thread limit), not an unbounded wait-free guarantee.

## File identities and versions: SD-KERNEL-FILES

There are three identities: process-owned handle, shared open description, and
backing version. Open creates an independent offset; dup and cloned handles
share the description. Copy rejection precedes offset publication. The offset
mutant advances before validating and is rejected without relying on a crash.
V1 adds a process owner and reference for each inherited handle. Its mutant
omits this step; the child's first read is stale. The parent's close must not
remove the child's separate ownership, and the last owner releases the description.

V2 snapshots retain an old backing version, while live descriptions follow the
current root. Mutating [1,2,3,4,5] to [9,8] leaves a snapshot's five bytes intact;
a live reader already at offset4 sees EOF. Seeking to0 then reads [9,8]. Both
versions pinned means FULL with no publication. Closing the old snapshot permits
an empty replacement. The snapshot mutant silently creates a live description
and fails the original-byte check. A copy-on-open snapshot is another valid
design if its allocation/failure/copy costs are measured under an equivalent
contract. Borrowing a freed version is never equivalent.

## Transactions and torn media: SD-DISK-RECEIPT

Derive results before execution: first +7 is7; identical retry remains7;
conflicting delta is rejected; the next client's +3 gives10; -2,+1,+1 yields
8,9,10. The evicted first ID stays stale through the durable high-water mark.
Receipt retention is per client, and the second client's original result stays10.
The output sentinel99 must survive rejection and an uncertain commit.

The effect, payload-bound receipt and high-water/cursor share one store commit.
The reapply mutant skips durable duplicate lookup; split-receipt commits effect
before its identity; payload-binding mutant accepts the same ID with another
delta. Each is compiled and executed and rejected by the intended assertion.
An in-memory ID table cannot repair any of these across owner restart/reboot.

V1's two clients/nonconsecutive IDs/three-receipt window is implemented by the
same durable protocol. IDs and epochs never wrap into an authorization grant.
V2 imports the actual store journal and compares two declared media models:
185 five-mode cuts, and18,176 sector-mask/page-prefix cases per compiler. All
expected recovered effects are independently restricted to0 or7; retry is7,
never14. A finite enumeration is not all combinations of old/new sectors across
all pages. The protocol argument depends on successful flush ordering, bounded
record validation, and the admitted corruption model. With arbitrary malicious
corruption an adversary can replace a record and recompute its unkeyed CRC; a
checksum is not authentication and cannot prove the stronger property. Require
that counterexample in the learner's analysis instead of awarding an impossible
guarantee from a green fuzz/test run.

The VM experiment interrupts before/after all write/flush calls and before the
reply, then boots a new VM with the same synthetic disk. QEMU process exit is
not a host power cut; cached writes can already be visible. Keep model results,
guest observations and physically measured device contracts separate.

## Graduated assessment and release protocol

H1 asks for the ownership/publication diagram without code. H2 supplies one
violated invariant, not the repair. H3 identifies the last safe publication
point. Record which hints were used. Correct nominal output without reasoning
is at most level1. Level2 requires the actual imported component fingerprints,
all named mechanisms and variants, discrimination of the faulty implementations,
generated-image closure and the corresponding original objective's reasoning.

After at least seven days, independently assign the original V1/V2 labels from
each X exercise. Freeze the candidate's prerequisites and withhold this analysis
and reference implementations. Require the variant's own invariant/cost changes,
tests with independent expectations, a fresh build and an unaided diagnosis.
Assess each original rubric criterion; do not turn the delay into a restriction
on today's author implementation. Preserve assistance, attempts and raw failures.
Level3 is an independent learner result; no author campaign supplies it.
