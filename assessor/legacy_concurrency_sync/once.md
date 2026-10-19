# T51.11 assessor analysis

The reference composes actual CMX wait enrollment with actual MBOnce permanent
publication. It never substitutes a synthetic queue for CMX or redefines the
producer's CMX_RETRY status. The semantic bridge is `settle`: only an actual
CMX RETRY terminal result becomes CDO_FAILED when the acquired MB state is
FAILED. A selected TIMEOUT/CLOSED remains selected. Cooperative initializer
cancellation cleans up before publishing failure and waking the cohort.

`cdo_enter` admits one slot per participant, validates task generation and
administrative epoch, and assigns a per-operation serial. The serial matters
even without reset: a completed READY ticket must not consume a subsequent
READY operation. The owner/table/pool remain live through all registrations;
checks of a stale handle do not justify accessing freed storage. Reset first
freezes admission and requires drained slots, then releases registrations,
then advances epoch. Epoch/attempt/serial exhaustion refuses new work while
cleanup remains possible. Raw producer state cannot be changed behind the
adapter. A producer-state mismatch after an election is a broken integration
contract, not a safely retryable operation.

The independently derived table is 1,18,35,52,69,86,103,120: count8, checksum484.
Failure at every one of 11 construction cuts keeps the reader sentinel999,
clears all table fields, and drops all three cells once. Allocation failures
1/2/3 drop respectively0/1/2 cells; quiescent epoch2 retry adds three fresh
cells and one successful initialization. Three native contenders enroll both
waiters before a terminal transition. The actual mutex protects the whole
CMX/adapter state; MB atomic publication is not a license to call the adapter
without that exclusion. Thirty rounds are a finite contention experiment,
not a proof of every schedule, progress bound or weak-memory execution.

Mutant1 publishes MB READY after entry0; the serialized preflight rejects a
partial table before introducing threads. Mutant2 resets FAILED automatically;
permanent error/attempt count fails. Mutant3 omits CMX completion and leaves
waiters/ownership stuck. Mutant4 omits one drop and violates resource balance.
Mutant5 resets while registrations remain. Every mutant must compile and fail
its intended assertion; crashes, hangs and sanitizer errors are not accepted
surrogates. The core and both imported learner producer stubs must also fail
semantically. Target observations use actual imported sources and explicitly
audited memory helpers, not host libraries hidden behind the freestanding API.

The C hosted callback returns normally after setting application_status=-7:
there is one attempt because call_once does not interpret that status. The
C++ local static and std::call_once throw on their first active attempt, then
retry and publish once. Both synthetic external effect counters reach2.
RAII frees the constructed temporary but does not undo those effects. A false
retry expectation for ISO C and false permanent-failure expectation for C++
are executable counterexamples. Recursive C++ local-static initialization is
undefined under the inspected N4950 clause and is analyzed, not executed.
C11/N1570 and C23/N3096 were inspected; exact C17/N2176 was inaccessible.
Compiling the comparison as C17 is not proof that the C17 text was inspected.
Both TSan profiles pass the native course adapter but fail the language
comparisons: C11 thread creation has a fatal diagnostic and the C++
exception/call_once path times out. The six debug/optimized/ASan comparison
profiles pass. Preserve the failed first attempt; do not suppress diagnostics,
claim those profiles were validated, or attribute the C++ timeout to a proved
root cause.

Assess original `.a` through policies/retry/limits, `.b` through publication,
all failure cuts and allocation rollback, and `.c` through native contenders
and both language comparisons. The delayed `.a` administrative generation,
`.b` allocation/retry and `.c` language contract variants have current author
references; unaided delayed performance and independent review remain open.
The X-T51-002 synthetic journal variant is not supplied by this adapter.
