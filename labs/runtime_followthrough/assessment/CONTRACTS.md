# T46.12 — cumulative evidence and asynchronous persistence

Use PR-T46-12's original criterion IDs and minimum-seven-day rules. Prerequisites:
your actual CVM1 compiler/core, T46.02 verifier, T46.03 continuation, T46.09 budget
analysis, T46.10 service adapter and T47 transactional store. This is C17 hosted
POSIX work; pthreads and file I/O are explicit dependencies, not freestanding
imports. The VM/compiler algorithms remain your artifacts. The original accepted
providers are used only in author-side execution, identified as such.

## T46.12.a: identify every boundary, then replay it

The supplied source is `(fn main () (host 1 41))`. Freeze its exact byte span,
compiler source/headers/build command and binary, compiler output, verified CVM1
bytes, VM build/binary, selected host adapter, actual command transcript and
status/value/fuel/host-call result. `chain_driver.c` builds in separate compiler
and VM modes; the VM mode does not link the hosted compiler. The narrow demonstration
adapter1 adds1 and adapter2 adds2, admitting only service1 and input<=INT32_MAX-2.
The same19 wire bytes have different results under these two embedding contracts.

`provenance.py` records and validates a JSON chain. Validation checks file/build
identities, recompiles the exact source through the pinned trusted compiler,
compares bytes, then executes that result through the pinned VM and compares the
recorded result. A hash alone cannot establish a derivation: insert the image for
source42 while claiming source41, even with a correct hash for the inserted file.
The replay must reject the mixed claim. Explain the remaining trusted compiler,
OS and capture assumptions; neither source identity nor a verifier verdict proves
the compiler correct or the guest behavior safe under arbitrary native callbacks.

Required deliverables: before-run predicted19-byte diagram, exact build/chain
records, stale-artifact experiment, independently derived result/fuel table, and
reproduction instructions. `run.py` requires explicit --cvm,--compiler,--execution,
--verifier,--services,--store,--adapter providers and --build; --source defaults
to the intentionally unfinished async learner. Use a new build directory for each
chain; creation refuses existing output images. Every predecessor is an actual
source file rather than a name-only integration promise.

Delayed transfer asks a second reader to reproduce the selected chain from a
clean learner export. Supply a complete package now: this contract, source,
public drivers/checker, predecessor headers/specifications and explicit provider
requirements. The reader records their identity, date, actual providers, commands,
result and any changed environmental assumptions. Author export/replay checks
verify that the procedure is executable; they are not second-reader evidence.

## T46.12.b: prove what each diagnostic distinguishes

Construct a valid/invalid pair for verifier operand height; a no-collection versus
allocation-triggered-collection pair with both C-temporary operands live; and a
sufficient versus insufficient fuel pair for a terminating image. State the exact
property, smallest trigger under the fixed capacities, output preservation and
expected first refusal. The assessor compiles incorrect providers separately and
requires the distinguishing semantic assertion. A compilation error, timeout or
sanitizer crash is not a killed semantic mutant. Keep the original provider intact.

The changed boundary combines an accepted HOST image with an expired native loan.
A stable C slot changes logical owner/epoch between validation and the callback.
The unchecked callback reads the new synthetic owner's value; a checked callback
rejects the expired loan and preserves result. All C objects stay allocated so
the demonstration has defined accesses. This demonstrates logical lifetime and
authority failure, not an observed C use-after-free or arbitrary code execution.
Derive why bytecode verification cannot know the host context's lifetime. A
language lifetime system can enforce some safe borrow scopes, but a raw foreign
pointer/protocol still needs a valid boundary; ISO C does not enforce this loan.

Required artifacts: minimal bytecode/state pairs, mutant-to-property mapping,
causal repair analysis, source-bound execution and remaining untested mutations.
The supplied private diagnosis is withheld. Learners must construct their own
pairs and explanation, not merely replay the assessor executable.

## T46.12.c: changed capacity and a real async store continuation

Prepare the delayed assessment without administering it early. The ordinary
changed-capacity package is explicitly named **CVM1-H8**: keep wire grammar and
instructions but reduce heap16 to8. The supplied exact-source header port preserves
the original file and records both hashes. Derive new Cvm layout/footprint, arena
exhaustion, collector sweep cost and pair-temporary assumptions before compiling.
Two roots containing distinct pairs cost2 root visits+4 field visits+sweepH:
H=16 requires22 units, H=8 requires14. One fewer unit must refuse before sweeping,
retaining every object; the exact budget frees all but the two reachable objects.
A source/image accepted under H16 can fail under H8, so image identity alone does
not bind resource semantics. Do not silently call H8 the baseline.

The separate changed condition composes the actual `Vs` continuation with an
asynchronous real file-backed store. Implement `async_store.h` in `learner.c`.
`Va` is an immovable, single-owner control object containing a mutex, one joinable
worker, copied `struct vs_request` and `Vh` configuration, borrowed session/store pointers,
result/status, and initialized/active/done/cancelled/joined state. Initialize only
uninitialized storage; destroy before reuse. The owner serializes all public calls.
The worker never reads or writes the session: it copies request/config before
start, performs vh_host on its exclusively borrowed store, then publishes status
and result under the mutex. Both the store and its I/O context/descriptor stay
live until successful join. Direct external store access while active is forbidden.

Submit accepts only a pending untaken request. Failure to create a thread completes
that request with HOST_ERROR and retains no worker borrow. While I/O is pending,
poll returnsVA_BUSY and destroy/re-submit refuse. A separate VM remains runnable.
When done is observed, poll joins before completing exactly the captured epoch and
serial. A second poll/cancel after completion refuses. Do not free a context on a
cancellation request or on a notification alone. The mutex publishes result data;
join also establishes that no worker access remains. POSIX specifies successful
join as observing thread termination; it is not an ISO C operation. [POSIX join](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_join.html)

Cancellation requests suppression of the guest result. It may prevent an operation
not yet begun; once running it does not interrupt arbitrary kernel I/O or roll back
the transaction. A cancel received before poll publication maps even a completed
commit to HOST_ERROR after join. Retain the operation ID and recover/reconcile via
the actual T46.10 receipt. A join error must retain ownership; do not treat an
unproved quiescence result as permission to reclaim. This adapter offers no hard
wall-time deadline for fsync/pread/pwrite/join. Adding one needs another provider
contract; a counter or cancellation flag cannot manufacture it.

`async_tests.c` uses an owned tmpfile, bounded page IDs and full-page transfer loops.
It gates the third flush of a real commit while the owner checks pending state,
refused destruction/restart and another VM's progress. Releasing the gate permits
fsync; the lost-reply variant deliberately returns failure after that successful
operation. Exercise all four cancel/no-cancel × lost/no-lost reply combinations.
All reopen to value15; retrying logical incrementID1,+5 performs no new flush.
These are actual file operations with controlled reply loss, not physical crash
or durability measurements. Short transfers/EINTR are handled by the page adapter;
fsync failure poisons the store per its contract. [POSIX pwrite](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pwrite.html), [POSIX fsync](https://pubs.opengroup.org/onlinepubs/9799919799/functions/fsync.html)

Required deliverables: continuation/worker/descriptor ownership diagram; exact
state transition table; changed-capacity derivation; async implementation; thread-
creation failure, cancellation, stale/duplicate completion and actual I/O tests;
resource costs including one OS thread/mutex and bounded copied records; and
failure/reconciliation analysis. Scheduler/thread-stack/syscall costs are hosted
costs, separate from VM fuel and fixed C struct sizes. Compare a language-native
future: storing a continuation does not establish cancellation, quiescence or an
exactly-once external effect. Identify programmer discipline versus runtime checks.

## Hints, assessment and containment

Hints progress from (1) list every artifact and borrowed object; (2) separate a
hash match from source derivation; (3) count GC roots/fields/sweep independently;
(4) mark the worker's last possible access before deciding who may free the store;
(5) distinguish a committed effect from a delivered completion. Full analysis,
mutants and worked answers belong only in the assessor package.

For each criterion, superficial output matching earns at most1/3. Passing requires
correct contracts/causal explanation and discriminating evidence(2); independent
changed-condition derivation and transfer earns3 when actually assessed. An
unattempted or contradicted argument fails even if tests pass. The private delayed
package specifies withholding and evidence capture. All security demonstrations
use original VM/provider programs and synthetic values; the harmless success is
a stale loan revealing another synthetic slot value77. Repair validates epoch and
liveness at use. Native memory isolation, malicious C callers, real credentials,
third-party services, privilege escalation and hardware flashing are outside scope.

Status is six-dimensional in the attached disposition record. Author tests, even
with ASan/UBSan/TSan and finite negative controls, do not prove absence of races or
absolute mastery. External review, second reader, actual delayed learner attempt
and physical observation stay pending until separately observed.
