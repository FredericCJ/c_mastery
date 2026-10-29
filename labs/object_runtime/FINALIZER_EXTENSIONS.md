# Finalizer execution and ephemeron value graphs

This required X-T74-009 extension implements the existing `struct or_weak` collector through
`object_runtime_finalizers.h`. It is a single-threaded C17 model with synthetic
resource bits. No operating-system resource or third-party runtime is modified.

## State, ownership and admission

The stationary owner contains sixteen object identities, four ephemeron pairs,
four weak references, sixteen pending finalizers and sixteen cleanup entries.
Slots have generations 1 through 3; reclamation increments the generation and 4
retires the slot. A handle is a checked identity, not an owning root. `{0,0}` is
the null edge. Only `root` controls a Boolean external root; copying a handle or
setting the same root twice does not create additional ownership. The client must
coordinate all aliases before removing a root. Callbacks and their context are
borrowed until reclamation or explicit close. No callback context may depend on
an unrooted native address into another object.

Initialize fresh storage with `orf_init`; reinitializing a live heap loses its
obligations and is forbidden. All output objects must be valid and disjoint from
the heap, each other and callback context. Clients may inspect state for evidence
but may change it only through these APIs. Arbitrary state corruption, forged
function pointers and concurrently accessing this owner are outside the contract.

`new` admits one rooted object, with optional finalizer and optional synthetic
resource; a resource requires a finalizer. Injection `fail=1` or exhausted live
capacity/retired identities returns `OR_FULL`, leaving heap and output unchanged.
Only Boolean resource/failure arguments are accepted. New allocations reset all
per-identity tracing/finalization state. There are at most 48 admitted identities
over the owner's lifetime, so the 16-bit callback/resource counters cannot wrap.
`edge` has fields 0 and 1. `pair` and `weak` have indexes 0 through 3. Pairs must
contain either two live handles or two null handles. Invalid inputs leave state
unchanged; closed owners return `OR_STATE` after basic pointer/argument checks.

## Collection, callbacks and cleanup

Collection stages the actual `or_weak_collect` kernel before publication. Ordinary
strong closure and conditional ephemeron closure reach a fixed point together;
a value graph can expose the key of a later pair. A value-to-key cycle without an
external key path cannot make itself reachable. Weak references are cleared from
ordinary reachability **before** queued finalizers retain their objects. No
pending finalizer or pending cleanup may cross another collection. Successful
reclamation retires the exact old identity, clears its fields and invalidates all
old handles. The fixed arrays do not allocate and have no growing queue failure.

`run_one` chooses the lowest pending slot and marks it finalized before invoking
its callback. Its return `OR_OK` means one callback completed; the separate
callback-status output may report failure. Callback graph/root/allocation changes
remain committed even on failure. Callbacks may allocate, change roots/edges or
close resources. They must return normally: `longjmp`, process exit and asynchronous
cancellation cannot satisfy this cleanup contract. They must not mutate the heap
outside the API or invalidate the borrowed owner/context. Recursive collection,
finalizer draining, cleanup and close return `OR_STATE` with outputs unchanged.
An empty queue returns `OR_DONE` and preserves both output values.

After a callback returns, any resource it still owns enters the cleanup bitmap,
including after callback failure or resurrection. `cleanup(budget)` closes at most
that many entries in slot order and reports their count; zero budget is valid.
It returns `OR_DONE` exactly when no cleanup entry remains. `close_resource` is an
explicit exactly-once operation; a second close returns `OR_STATE`. A resurrected
object stays alive but its finalizer never runs again, and it cannot retain the
resource merely by resurrecting itself. Allocation inside a failing callback
can leave a newly rooted object: the callback/client must account for that root.

There is no prompt-finalization promise. Explicit `close`, after all client uses
and callbacks have ended, closes every remaining resource even if no collection
or finalizer has run, abandons outstanding callbacks, and invalidates the owner.
This is the deterministic ownership escape path. It is not a promise to invoke
every finalizer. No API can make an uncooperative non-returning callback complete.

## Required experiments and independent model

Build the learner implementation and produce D1: operation/state/invariant table,
the two closures, the 48-identity bound and a Lua/CPython comparison. D2 implements
all header operations using the original collector and creates failing,
allocating, resurrecting and explicit-resource-closing callbacks. D3 records
original and repaired traces, unchanged rejection state, full queues, generation
retirement, cleanup budgets, callback allocation failure and shutdown without GC.
D4 includes an independent set-based oracle, compiler/sanitizer evidence and the
13-dimensional cost review. Preserve callback return values separately from the
driver's operation status in every trace.

The supplied driver enumerates 64 strong graphs, 16 root sets and 25 pairs of
conditional targets: 25,600 expected closures. The independent Python model uses
sets and repeated reachability expansion rather than C bitmaps. The delayed
variant X-T74-009-DELAYED is the reverse-ordered relation `3 => 4`, `1 => 2` with
strong edge `2 -> 3`, then deletion of root 1 and addition of `4 -> 1`. The first
collection keeps all four; the second reclaims all four. A single ephemeron pass
and treating values as unconditional roots both fail this sequence.

The four compiled controls remove callback exclusion, skip cleanup on callback
failure, root ephemeron values unconditionally, and stop conditional closure after
one pass. Each must compile and fail a named semantic assertion. A compiler error,
process signal or sanitizer diagnostic is not the intended mutant rejection.

Hints: first separate ordinary reachability from temporary finalizer retention;
then mark finalization before calling user code; finally decide which obligations
remain when the callback fails. Assess R1 by deriving both sets, R2 by the callback
state machine and source comparison, R3 by independent closure and mutation
evidence, and R4 by bounded cleanup, callback allocation and the value-graph
transfer. A passing supplied test alone earns no independent transfer credit.

## Language policy comparison and evidence limits

The inspected Lua 5.4 manual permits finalization to be registered again and
describes weak clearing around resurrection. This model deliberately allows one
finalization event per identity. PEP 442 describes CPython's once-only cyclic
finalization and a reachability recheck after finalizers. Our explicit subsequent
collection provides a different schedule; it is not CPython's cycle-disposal
algorithm. Neither text makes this synthetic bitmap a real resource manager.

Sources: [Lua 5.4 sections 2.5.3–2.5.4](https://www.lua.org/manual/5.4/manual.html#2.5.3)
and [PEP 442, description and predictability](https://peps.python.org/pep-0442/).
The PEP is an inspected historical design proposal, not inspection of current
CPython execution. C17 enforces none of the root/finalizer protocol; checked API
branches enforce local transitions and programmer discipline supplies valid
storage, callback termination, quiescence and exclusive access. Finite host tests
and bounded enumeration do not prove arbitrary-program memory safety or mastery.
