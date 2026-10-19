# PR-T37-04 technical analysis (withheld)

T37.06.a: source availability is an edge, not a mnemonic label. The load's
MEM6 value enters consumer EX7. The full trace reproduces I4 held in ID5/6,
I5 held in IF5/6, and an empty EX6. The two-cycle load occupies MEM6/7 and
delays the consumer to EX8. The LOAD mutant incorrectly admits a value while
the producer is still in EX or early MEM. Independent certificates reject
this even if the final register values would coincidentally agree.

T37.06.b: the default retires six instructions by13, but fetches eight distinct
instances. Both I6 and the first I7 fetch are discarded at8; the correct I7
refetch begins9. The frozen s_pipeline recurrence has no wrong-path IDs; it
is valid only as a compatible retired-stream cycle cross-check, not a full
stage certificate. Its four-instruction total is9/14. Read-before-write adds
one cycle at each of three nonforwarded dependencies, yielding17. An ID branch
without ID forwarding waits for r4 WB9 and yields14 total/one flush, illustrating
why an earlier nominal resolution stage can lose an operand-bypass advantage.

A unified memory port need not add a new stall to the original dependent
stream because IF is already held when LOAD occupies MEM. The initial pilot
incorrectly required every shared-port configuration to have a distinct port
stall; that test premise was corrected using the actual overlap rule. The
independent stream exposes the otherwise hidden port conflict and ends at9.
Do not present removal of that invalid premise as relaxed correctness.

T37.06.c: dependence is a source-value relation. Hazard is the chosen timing
mechanism's inability to deliver the value when needed. A C data race requires
different threads, conflicting accesses with a non-atomic access and absence
of happens-before. The compile-only original counter example establishes an
analysis task, not an executed race or exploit. Atomic relaxed increment
repairs that operation's conflict but says nothing about unrelated publication.

The changed out-of-order model binds each source to an earlier producer tag,
computes results in private per-instruction slots, and retires in order.
Independent work bypasses a waiting consumer. WAR/WAW naming does not redirect
an already-bound source; true RAW completion still constrains issue. The TAG
mutant loses this relation and fails the independent sequential final-value
comparison. Load-port occupancy and retirement order are checked separately
from final values, so a lucky numerical result cannot validate the schedule.

All scheduler outputs are built locally and published once. Full input
admission precedes indexing. There is no allocation or concurrent mutation.
128-cycle/64-instance limits bound trace storage; they do not establish a
physical worst-case execution time. Actual native compiler frame/code sizes,
emulated C execution and modeled cycle counts are kept distinct.

The old provider's complete translation unit exposed an unrelated optimized
maybe-uninitialized warning in s_reduce. The successor extracts the exact
unchanged s_pipeline body with source/body hashes rather than editing the
frozen family or suppressing diagnostics. The code and provenance are verified
against the original body on every package guard. Only this function is reused.
