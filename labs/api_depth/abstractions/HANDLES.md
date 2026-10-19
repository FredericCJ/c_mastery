# T19 identity, shared lifetime and access safety

Required T19.09.a/b/c packet, PR-T19-04; retain original AX IDs and seven-day
variants. Families A/B/C/D/F/G/J. Ordinary prerequisites are T18 ownership and
T19 stream/handle contracts. Concurrent changed tasks additionally require
T25/T51 happens-before, mutex and join reasoning; revisit this packet after
those prerequisites rather than adding a cycle to the introductory module graph.
Use C17 hosted Linux pthreads, GCC/Clang O0/O2 and ASan/UBSan, plus Clang TSan
for the explicitly racy specimen. No target freestanding claim is made for a
pthread/heap implementation. Host threads and synthetic bytes are the entire lab.

## Three finite slots cannot supply infinite permanent identity

`AhPool` owns three slots, each generation1..3, occupied and retired flags, and
an unsigned payload. `AhHandle` is (index,generation,epoch), never a raw payload
pointer. Initialize once with epoch1. Allocate the first free nonretired slot;
failure preserves output. Read/release validate index and both identities before
access. Release clears occupancy, advances generation when below3, and otherwise
permanently retires the slot within its epoch. Stale reads preserve output. Test
all nine possible allocations, invalid index99, simultaneous capacity3 and every
old handle after each release. Copying a handle does not retain a slot.

The changed protocol permits reuse only with no occupied slots and no registered
borrowers. `ah_enter/leave` count at most three serialized borrow regions; callers
must register before retaining/accessing owner storage and must stop admissions
around reuse. Reuse increments an independent epoch and resets slot generations.
Epoch itself is bounded1..3 and refuses wrap. Check27 distinct identities, stale
handles from all prior epochs, three-borrower capacity and rejection while borrowed.
This is a bounded safe reuse example, not infinite identity. Resetting epoch to1
would require proving that *all* old handles have been destroyed, including copied
or serialized handles; a zero in-flight count alone does not prove that. The pool
control must remain alive during any read. Atomic counters alone would not make
this serialized region/admission protocol concurrent.

## Strong references retain payload; weak references retain control

Implement opaque heap `AhShared` with mutex, strong count, weak/control count,
payload pointer and borrowed statistics ledger. Construct transactionally: failed
allocation or mutex initialization releases partial resources and preserves output.
The ledger outlives all controls. Strong count is1..3 while payload exists. Weak
count is explicit weak owners plus one implicit control owner whenever strong is
nonzero, also bounded3. Every retain/promotion rejects before overflow. Caller
capabilities are linear by contract: retain before sharing, drop exactly once,
never operate on an unowned/dangling control. C does not enforce these rules.

Last strong release destroys payload exactly once and releases the implicit weak
owner. The last remaining weak owner destroys the mutex/control. A weak promotion
holds the mutex, checks strong>0 and capacity, then increments; it cannot resurrect
a dead payload. The promoting thread already owns a weak reference, keeping control
alive while it acquires the lock. Each concurrently active owner contributes to
the counts; zero owners implies no legitimate waiter. This proves final mutex
destruction is quiescent under the ownership contract, not against forged aliases.

Represent two nodes with distinct root and edge ownership obligations. Two strong
cycle edges leave counts1,1 after roots are dropped; a graph cycle is not collected
by ordinary reference counting. Explicitly break edge obligations in the assessor
fixture so the test itself does not leak. Replace one edge with a weak edge and
derive the destruction order. Test strong/weak maxima, promotion at capacity,
promotion while alive and after death, and256 actual races with last-strong-drop.
Both racing outcomes are valid: promotion first retains the payload, or last drop
first makes promotion fail. No particular success fraction is an acceptance rule.

## Ownership does not order conflicting payload accesses

Two worker threads each receive a retained strong owner. Their unsynchronized
`++*ah_payload()` accesses conflict on a non-atomic object without a happens-before
edge. The program has a C data race, regardless of observed final value. Run this
original deliberately defective specimen only under the named local TSan profile;
require the intended race report, not a crash or a fabricated golden count.
The repair `ah_add` holds a separate payload access contract using the object's
mutex, checks unsigned addition before mutation, and yields exactly20000 after
two10000-operation workers join. The reference mutex happens also to manage
lifetime; that implementation choice is not a general property of shared ownership.

Changed publication: construct an immutable payload, reserve a worker strong
owner, then release-store its pointer. The worker acquire-loads it, reads immutable
payload and drops that reserved owner. Keep a root through join. Repeat32 values.
The publication edge orders initialization; reference accounting retains lifetime;
immutability removes later writes. An arbitrary atomic pointer load followed by
retain is unsafe if another thread can reclaim between them. Supply that
counterexample and the missing hazard/epoch/lock protocol before proposing reuse.
Changing counts to atomic does not repair unrelated payload races.

Deliver ownership graphs, all failure-preservation traces, bounded identity proof,
race/happens-before analysis, synchronized and immutable variants, heap balance and
tool/source receipts. Hints: separate identity from occupancy; include the implicit
weak owner; identify exactly which release/acquire or mutex edge orders payload.
Per leaf0 wrong,1 nominal behavior,2 complete ordinary argument and discriminating
evidence,3 independent changed assumptions. Native and sanitizer evidence is
bounded author validation; review, learner attempts and delayed transfer remain
separate. N1570 5.1.2.4 is an inspected C11 committee draft, not published C17;
the source register also identifies the inspected public Modern C third edition.
