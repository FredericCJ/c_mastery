# PR-T51-08 assessor reasoning

Assessor only. The adapter imports the actual retained CMX event implementation;
the occurrence sequence is original first-principles code with no event library.

T51.08.a: OR is idempotent. Three sets of bit4 before consumption leave only bit4,
whereas the actual CMX notification count holds three. The sequence gives each
observer a cumulative difference. `(7,MAX-1)` to `(8,0)` differs by two; omitting
epoch advancement causes apparent reversal and invalid admission. Keeping the
epoch but returning one hides the missed occurrence. `missed=delta-1` is an
observation count, not payload recovery or exactly-once processing. Terminal
counter exhaustion refuses rather than creating a silent old identity.

T51.08.b: actual CMX chooses the oldest currently eligible waiter, then clears its
observed mask and reevaluates. An unsatisfied older all-waiter does not block a
younger any-waiter. With both initially waiting and set3, all3 first consumes both
bits and leaves any1 pending; any1 first consumes1 and leaves all3 pending on2.
Observers see only states that still exist when their turn is evaluated. Reversal
is an intentional changed policy, not equivalent broadcast. Immediate and pending
results have one owner and must be collected before a new task operation begins.

T51.08.c: the 224-case oracle covers the three-bit immediate relation and the
12,544-case oracle covers exactly two initially blocked waiters and one set. The
independent oracle enumerates bit membership and sequentially subtracts consumed
observations; it does not call CMX or trust implementation output. Neither count
covers arbitrary-width concurrent executions. Explicit bit31 execution adds the
width boundary without a signed shift. Timeout-before-set keeps the terminal
timeout while retaining the new level for the next request. Nonconsuming STOP
reaches both predicates; close provides a separate all-waiter terminal path.

Reject explanations that repeated sets count occurrences, every overlapping
waiter sees one immutable snapshot, a ready task implies its result was collected,
or a host semantic checksum proves native/IRQ safety. The eight controls isolate
predicate, unrelated-bit preservation, observer semantics, FIFO order, sequence
epoch, delta multiplicity, high-bit width and close wakeup. Keep public scaffolds
unsolved. No independent human assessment, learner performance or physical target
claim follows from the author campaigns.
