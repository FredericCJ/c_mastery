# PR-T25-02: withheld mechanism analysis

These are author reference arguments, not an independent review or a learner
solution receipt. Use the exact original criterion and delayed IDs. The public
contract defines allowed traces; successful finite tests do not close arbitrary
scheduling, cancellation, recovery or mastery claims.

## T25.03.a — complete invariant and unlocked snapshot

The split worker reaches(40,50); every access is mutex-protected but the total
is90 between transactions. The repaired worker keeps the lock until(40,60), so
the controller's trylock fails during the hook and the eventual total is100.
Check the whole causal ordering, not just100 at exit. The snapshot uses a single
SC order: accepting equal even sequence values means no intervening writer
sequence transition, provided the counter never wraps. Atomic payloads make
the speculative reads legal; an ordinary-payload seqlock would still have a C
data race. Reject exhaustion. Multiple writers would need serialization or a
different protocol; lock-free atomic operations do not prove reader progress.
The changed packet is the actual concurrent writer/reader test plus that proof.

## T25.03.b — cycle, callbacks and reused identities

Both workers hold their first mutex before either trylock. Both observe busy,
which records the would-wait edges without a hung process. A self-callback edge
adds a one-node cycle. For the repair, strict increasing distinct keys preclude
a finite cycle because its last edge would need to decrease. Storage reuse is
permitted only after join and stale-borrow invalidation; the new key differs.
Do not call pointer ordering a portable key. The relation mutant is rejected
before starting potentially blocking repaired acquisition, so its rejection
is a semantic assertion rather than a test timeout.

## T25.03.c — initialization prefix and admission-root lifetime

Fault ordinal k initializes/destroys exactly k-1 real synchronization objects.
Destroy is never a notification mechanism. The root serializes new references
and final decrement; operation admissions independently prevent core teardown.
At retirement, all reference holders have left, all admitted operations have
released, and queued contents are empty. The still-live root rejects later
calls without consulting destroyed mutexes. Final root destruction additionally
needs external quiescence; a refcount cannot authorize dereferencing its own
freed storage. The changed packet deliberately retains an extra reference
across close, observes retirement busy, then releases it and retires.
Zero references rejects all new operations, and the last reference cannot be
dropped while queued values remain; otherwise drain ownership would disappear.

## T25.04.a — ring and exactly one consumer

The old capacity-three history yields popped1,2 and contents3,4; the fifth push
cannot commit. A zero deadline checks the full predicate without an arbitrary
sleep. Close drains3,4. With two empty-queue consumers, a single21 plus close
has exactly one successful pop and one unchanged-output closed result. Neither
notification count nor scheduling order identifies the successful consumer.
The changed packet checks cardinality and each output, not thread identity.

## T25.04.b — notifications, empty state and theft

Broadcast before enrollment leaves no stored notification, but a queued17 is
visible to the first later consumer without any wait. Empty broadcast cannot
authorize a dequeue. The incorrect single-check branch is modeled and the
mutant exits its predicate loop without reading an uninitialized cell.
For theft, a test hook exposes a larger safe scheduling gap: the first waiter
has returned with its lock, then releases/reacquires with cancellation masked.
The controller consumes31 before the original consumer resumes. This is an
actual competing consumer history with the same predicate obligation as a
competitor winning the mutex after notification; it is not a claim to control
the libc scheduler's exact return instruction. A second wait and unchanged77
on expiration distinguish the while implementation.

## T25.04.c — absolute waiting budget and deferred cancellation

Four actual wait entries receive the same monotonic absolute time. Three noise
broadcasts do not extend it. Predicate availability wins over waiting timeout;
empty close wins over the empty timed-out branch at the locked decision. Sixteen
actual close/expiry competitions accept only the two allowed statuses and never
modify output. Cancellation occurs at an enrolled empty wait; joining yields
the canceled sentinel, active becomes0, and the core mutex is available for a
new push/pop. Cleanup does not use a mutable automatic lock-state flag. A
caller using asynchronous cancellation in this queue violates its contract;
the separate permit experiment does not broaden it.

## T25.05.a — two phases and explicit break

Generation0 publishes cells1,2,3. Generation1 publishes total6. A barrier is
not a stored permit counter: one thread consuming three semaphore permits
would not establish three arrivals. A read/write lock excludes conflicting
access, but can let the aggregator read before every producer has written.
Once completes one initialization episode; a second phase would pass without
waiting for new arrivals. With expected3 and two observed actual arrivals,
the supervisor breaks; both callers and future callers receive cancellation.
No native cancellation is used inside this custom barrier. Generation overflow
breaks the object rather than aliasing an earlier phase. Unannounced vanished
participants require another failure-detection protocol, left outside this
explicit-departure task.

## T25.05.b — starvation without a safety violation

Reader-preference lasso: one reader active, a second enters, the old one exits;
rename identities and repeat while the writer stays queued. Permit lasso:
holder releases, newcomer wins, old waiter is not selected. Writer-preference
lasso: one writer leaves with another queued, the next wins before the waiting
reader. Each returns to an equivalent state with the selected victim still
waiting. An induction on this cycle establishes arbitrarily many overtakes
under the explicit unfair policy. Checking1000 loops is regression evidence
for the encoded policy, not a measurement of infinite host starvation.

## T25.05.c — selected invariant and asynchronous cancellation

The table's choices are mechanisms, not interchangeable API names. A mutex
alone can serialize initialization yet run it repeatedly without a once-state;
a once flag cannot count three simultaneous owners; a permit can be consumed
repeatedly by one participant instead of rendezvousing; a barrier does not
prevent a later writer from modifying data while a reader uses it. Require
these distinct rejected traces. The once callback returns no status: publish
initialization failure explicitly or design another once-policy object.

Three ownership bits bound active permits. Release validates that the caller
still owns a bit; stale tokens after reuse are excluded, not detected by an
unversioned mask. Cancellation is disabled through acquisition and cleanup
installation. The only enabled-async region executes a pure compute loop and
the allowed cancellation-state transition. Cancellation cleanup disables it
before touching the mutex. The actual test joins a canceled thread, observes
exactly one returned permit, then admits three and rejects a fourth again.
Increasing initial capacity would conceal a leak and violate the original
bound. General arbitrary asynchronous interruption of mutex/allocator internals
cannot be repaired by this technique. Disassembly must contain no injected
sanitizer/runtime call in the compute region other than the permitted state
change; this local instrumentation exclusion is recorded, not hidden.
The single canceling controller observes readiness only after that state-change
call has returned. The selected Clang sanitizer symbol is the explicit
interceptor trampoline; its source forwards to the real operation and checks
the old-state output. No arbitrary cancellation during interceptor internals
or portable sanitizer async-safety guarantee is inferred.
Clang's `no_sanitize("thread")` still instruments atomics; the additional
`disable_sanitizer_instrumentation` attribute is necessary for this isolated
compute function. The final disassembly must reject those atomic runtime calls.

## Assessor checks and research limitations

For each of the nine objective-specific original rubrics, level2 requires its
matching forced history and causal explanation above; passing a different
case is insufficient. Level3 requires the original seven-day changed task,
fresh independent reasoning and the altered-assumption limits. Reference
campaigns bind all18 ordinary/delayed author packages, but no learner dates,
reviewer independence or physical work are manufactured.

The ordinary algorithms use fixed bounded state; mutex contention, condition
waits, snapshot retries and scheduling have no finite latency guarantee.
Initialization rollback counts only objects successfully initialized. The
new source evidence contains exact POSIX.1-2017 reprint passages plus selected
Linux/GNU notes, not final C17/C23 or POSIX.1-2024 authority. Broader T25 memory
ordering, CAS reclamation and performance investigations remain separate work.
