# Permanent failure, quiescent retry and initialization publication

This additive contract implements the original `PR-T51-11` criteria and their
three delayed variants. The retained project, fixture and acceptance evidence
remain unchanged. The implementation language is C17. Import completed
`cmx.h/c` and `bridge.h/c` explicitly; `once.c` is an unsolved learner scaffold.
The hosted ISO C and C++ comparisons are verification programs, not permitted
dependencies of the once implementation. Assessor solutions are not supplied.

## Inputs, representation and operations

Implement `once.h` for three registered participants, actual CMX task slots
0, 1 and 2. `struct cd_once` owns a permanent-policy `struct mb_once`, a CMX once wait object,
three result slots, three allocator-owned cells, and `struct cdo_table`. The table has
three fields: eight entries, count and checksum. The required workload writes
entry `i` as `17*i+1`, then count 8, then checksum 484. Every entry, count and
checksum is a distinct construction step. The checksum uses unsigned modular
addition; this workload does not overflow. Derive the independent expected
sum as `8 + 17*(0+1+2+3+4+5+6+7)`.

The allocator returns three distinct, live, correctly aligned `struct cdo_cell` objects
or null. The core must not call malloc/free. Accumulate each entry into cell
`i%3`; failure drops all acquired cells in reverse acquisition order, exactly
once. The callbacks cannot reenter, retain borrowed control storage, throw,
longjmp, or fail during drop. A null allocator context is allowed. Successful
construction retains its three cells until quiescent destruction. Pool-backed
host/target tests and actual malloc/free native tests exercise different
allocation environments under the same ownership contract.

`cdo_init` requires fresh or externally quiescent live storage, nonnull CMX,
allocator and drop function. Do not copy initialized atomic control objects.
`cdo_register` admits a READY/RUNNING task once and binds its current task
generation. A handle identifies owner, task and administrative epoch; a ticket
also binds an operation serial. An old completed ticket cannot collect a later
READY read by the same participant. Pointer validity remains a caller
precondition: stale identity checks do not make freed owner storage safe.

`cdo_enter` returns `CDO_INIT` to the elected initializer, `CMX_BLOCKED` with a
ticket to an enrolled waiter, or `CMX_OK` with a ticket for published data.
Permanent failure returns `CDO_FAILED` without admission or a new ticket.
Recursive initializer entry returns `CMX_DEADLOCK`. A participant has at most
one outstanding operation. Other rejected admissions leave the output ticket
unchanged. The CMX and MB producer state must be modified only through this
adapter; divergence is a contract failure requiring stop and inspection.

The initializer calls `cdo_allocate`, eight ordered `cdo_write` operations,
two `cdo_seal` operations, and `cdo_finish(...,1)`. A premature successful finish
is invalid. Failure or cooperative owner cancellation clears all unpublished
state and drops resources before publishing FAILED and waking waiters.
`cdo_read` and a successful `cdo_result` copy a complete table only after READY;
all other results leave the output unchanged. `cdo_result` consumes a terminal
ticket once. `cdo_cancel` of the initializer selects failure; a pending waiter
uses actual CMX cancellation. An already selected timeout/cancellation remains
that result even if the initializer subsequently succeeds. Completion already
selected wins over a later cancellation.

## Exclusion, publication and lifetime

Every core API operation, including `cdo_read`, requires external serialization
with CMX progress and other adapter calls. Do not overwrite a participating
task's result with another raw CMX operation while its ticket is outstanding.
Tasks and the allocator context remain live until results are collected and
registrations released. No asynchronous owner termination or arbitrary task
restart is supported. The actual MB release publication and acquire reader
observation are retained, but the whole adapter is not a lock-free algorithm.

The native driver uses one pthread mutex and condition variable around actual
CMX transitions and result checks. The initializer waits until both real
contenders enroll, then publishes success or fails after entry 3. Every wait
rechecks its predicate under the same mutex. Thirty rounds rotate thread
creation order; each mode creates 90 threads. A delayed reader checks the
stable outcome after joins. Unexpected pthread creation, lock, wait or clock
failure is an experiment failure requiring stop/retain/inspect; the driver
does not claim recovery from those failures or asynchronous pthread cancel.

Failure is permanent within an epoch. Administrative retry is a separate
protocol: collect every result, `cdo_prepare_reset`, release every registration,
then `cdo_reset`. Reset increments the epoch before new registration and retains
the cumulative attempt count. Reset of READY is forbidden. Old handles/tickets
are refused after the epoch changes. Generation, attempt and ticket counters
retire before wrap; close/release/destroy remain possible at exhausted limits.
`cdo_close` requires drained results, freezes admission, and allows release;
`cdo_destroy` additionally requires no registrations and drops retained cells.
Reusing owner storage still requires external quiescence of every old pointer.

## Required reasoning and policy comparison

Explain these three distinct mechanisms using the supplied `policies` case:

* The retained fixture's A-start/B-wait/A-fail/C-enter trace expects permanent
  failure and one attempt. The corrected adapter provides that contract.
* Actual `cmx_once_enter/finish` uses `CMX_RETRY` as initializer permission and
  as the wake result after failure; failure returns its raw object to EMPTY.
* Actual `struct mb_once` selects permanent or retry policy at initialization.
  `MB_INIT` grants initialization. `MB_RETRY` denotes compare-exchange
  contention, not the CMX initializer role. This experiment does not force a
  compare-exchange contention execution; it distinguishes the API meanings.

For each state transition derive the owner, resource and result invariants.
Show why waking before publishing, publication after only one entry, resetting
FAILED to EMPTY automatically, releasing one cell twice, and discarding an
uncollected result are distinct defects. Give a pre/postcondition for every
operation and an accounting equation `allocations - drops = live resources`.
All core loops have fixed bounds (3 participants, 3 cells, 8 entries); storage
is fixed plus three cells. Waiting time and external allocation cost are not
bounded by those operation counts. Record `sizeof`/alignment and generated
code for each tested ABI before making byte-cost claims.

## Language edition and changed-condition comparison

Read the exact passages registered in
`research/v5/expansion/legacy_concurrency_once/resources.json`. C11 draft N1570
7.26.2.1 supplies exactly-once callback execution and completion synchronization,
with a void result. The tested C17 hosted implementation returns normally after
recording an application error: a second call does not retry it. That is a
runtime observation, not inspection of exact C17 wording. N2176 was encrypted
and could not be inspected; retain that gap. N3096 C23 additionally declares
the once facility in stdlib.h; optional threads.h availability and installed
library support must be checked separately. This campaign does not claim a
C23 stdlib.h implementation test.

N4950 C++23 `[stmt.dcl]` specifies retry after an exception during local-static
initialization, concurrent wait, and undefined recursive reentry. Do not run
that undefined recursion as if it were a required diagnostic. Its
`[thread.once.callonce]` orders active calls and distinguishes exceptional from
returning completion. Neither facility provides this course's administrative
reset or permanent application-error protocol. `once_compare.cpp` throws once,
then uses three real threads: both local static and std::call_once make two
attempts and one returning initialization. A constructed temporary releases
its allocation during unwinding; a synthetic external counter effect remains
and repeats. Language cleanup does not roll back external side effects.

## Deliverables and verification

Submit the implementation, state graph, ownership ledger, trace annotations,
generated-code/link audit, tool versions and commands, raw observations, and
the policy/edition comparison. Run `run_once.py` with explicit completed
`--execution-source`, `--execution-include`, `--bridge-source`,
`--bridge-include`, `--once-source` and a named `--case`. The six cases cover
publication, all 11 failure cuts, three allocation failures plus administrative
retry, selected timeout/cancel, producer policy differences, and counter limits.
`run_once_native.py` requires the same explicit imports and `success` or
`failure`. Its serialized preflight must reject an intentionally flawed
publication implementation before threads start; do not intentionally run a
data race to obtain a diagnostic. Hosted comparison programs may use system
threads and once facilities solely as comparison oracles.

The M4 harness executes actual CMX and MB imports with logical tasks on owned
QEMU `mps2-an386`. Compile C17 freestanding Cortex-M4 soft-float with no default
libraries. Audit undefined symbols, disassembly and linker map, including
compiler-generated helpers. Only explicitly linked course memory helpers are
allowed. This is target ABI/emulator evidence, not a context-switch, physical
board, interrupt latency or hardware-memory-order observation. Semihosting is
a test transport. No host security changes or flashing are authorized.

Require strict GCC/Clang debug, optimized and ASan/UBSan observations; native
adapter verification also uses GCC/Clang TSan. The first comparison campaign
actually attempted both TSan profiles: C11 thread creation produced fatal
sanitizer diagnostics; the C++ exception/call_once path timed out. Those failed
observations are retained separately, not converted to passes. C11 project
issue 1603 documents a matching limitation; the precise C++ runtime cause
remains unresolved. Language comparisons therefore have six validated
profiles and two explicitly unvalidated TSan profiles. A build failure,
timeout, crash, sanitizer report or generic failure message is not the expected
semantic rejection. Match each named control's intended assertion. Passing
these finite tests cannot establish absolute correctness or learner mastery.

## Assessment, hints and delayed transfer

The original criterion IDs and minimum seven-day delayed variants remain in
the canonical project supplement. Grade `.a` on exact CMX/MB/permanent-policy
trace and new-generation reset, `.b` on complete publication/cleanup and every
allocation failure before retry, and `.c` on real contenders, delayed readers
and the ISO C/C++ non-equivalence counterexamples. Level 0: fabricated/missing
evidence or violated invariant. Level 1: ordinary success only. Level 2: all
specified boundaries and controls pass with correct reasoning and limits.
Level 3: an unaided delayed changed-condition solution with a fresh failure
argument and complete evidence. Author reference execution is not a learner's
independent attempt or independent human review.

Hints: first enumerate EMPTY/RUNNING/READY/FAILED separately from CMX wait
states; then add a result slot that survives wakeup; then make reset an
admission-free drain protocol. For cleanup, write the ownership ledger before
coding rollback. For language comparison, count attempts, successful
publications and external effects separately. Assessor implementation and
analysis are withheld. The related X-T51-002 synthetic journal transfer is a
distinct remaining scope until an actual idempotent/transactional adapter is
implemented and executed; this once campaign does not silently close it.
