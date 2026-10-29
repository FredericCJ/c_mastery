# T19 callbacks, suppression and terminal ownership

PR-T19-01 T19.03.a/b/c ordinary and original changed tasks use families A/B/C/D/F/G/J.
Bring the T18 callback ownership dossier and actual `struct api_registry` component. Use
C17 hosted GCC/Clang O0/O2 and ASan/UBSan. Implement `registry.c`; keep assessor
bodies private. This is original local synthetic software, with no external
service, deployed system, credentials or host-security changes.

The existing registry has four slots, entry snapshots, generation handles,
deferred removal, no recursive dispatch and destruction that suppresses remaining
scheduled callbacks. Build an adapter over that actual provider. Do not silently
change it to obtain different expectations. Each adapter registration stores
function, context, release hook, provider handle and logical-enabled flag. Calls
are serialized, callback/drop functions return normally, and registration creates
a lifetime obligation. No C type proves that context remains alive.

First register the same increment callback with two counters0 and10. One dispatch
must yield1 and11. A global hidden counter is the wrong abstraction even if a
one-callback example passes. A context pointing to a returned stack frame is
invalid independently of the function's lifetime; analyze the timeline without
dereferencing it. For owned contexts, successful registration transfers one
release obligation; failed registration preserves ownership with the caller.
The release hook must not free registry storage. Only the final receiver may.

Use two explicit policies. Snapshot policy preserves the provider: A removing B
during a dispatch does not stop B's already-snapshotted invocation, but B is
dropped after dispatch. Immediate-suppression policy marks B logically disabled
and still calls provider remove; the wrapper then skips B while keeping its
context until the provider drop. The latter changes which callbacks run, not
the storage reclamation boundary. In either policy A removing itself yields A,B
then only B on the next dispatch. A adding C produces A,B first and A,B,C next.
Recursive dispatch returns BUSY without an inner invocation. Fifth registration
returns FULL and transfers no ownership. Handle generations are still the
provider's generations; never replace them with slot number alone.

For destruction, A requests destroy twice while B remains scheduled. Both
requests return PENDING; A's context remains alive until its return. B is
suppressed, both owned contexts drop once, and one terminal receiver runs after
the provider has returned from dispatch and all drops. Implement a receiver that
actually frees a heap-allocated adapter registry. The adapter must save return
status and callback/context before calling it and perform no subsequent registry
access. This transfers final control-storage ownership, unlike the original
provider's contract. Repeated destroy is valid on retained closed storage; any
call through the pointer after the terminal receiver freed it is invalid.

Deliver policy traces and a timeline for function/context/control-storage
lifetimes, implementation, two-context and allocation/release ledger, all ten
specified removal/addition/reentry/destruction histories, capacity tests and
terminal-free sanitizer results. Check exact A/B/C sequences, not only counts.
Compare mutated provider premature-remove and skipped-drop specimens: they must
compile and be rejected for the intended contract violation. Never count a
compiler failure or arbitrary crash as evidence for a semantic oracle.

For each leaf, score0 incorrect;1 nominal output without lifetime explanation;
2 ordinary traces and ownership proof plus failure controls;3 independently
derives the changed ownership/suppression/terminal-free protocol. After at least
seven days administer the original owned-context, immediate-suppression and
terminal-free variants with assessor solutions withheld. Actual learner evidence
remains pending. Hints: separate logical eligibility from physical lifetime;
write the last access to registry storage; keep the terminal invocation outside
the provider's dispatch frame.

Sources and edition limits are in the T19 source register. Original contracts,
actual provider code and exact changed tasks govern these observations. A
serialized trace is not proof of concurrent cancellation, weak-memory ordering
or wait-free progress; those require the later synchronization curriculum.
