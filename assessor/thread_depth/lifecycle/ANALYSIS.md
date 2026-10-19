# T25.01 lifecycle technical analysis

This is assessor material. No worker lifetime is proved by observing a sum.
The actual publication/last-user edges below justify the selected finite cases.

## T25.01.a and exact delayed retained-heap variant

The retained baseline pl_pool owns stack Work[4] until every successfully
created thread joins; its successful-prefix invariant excludes the failed
handle and preserves earlier obligations. Its terminal125 on unexpected join
failure prevents returning borrowed stack storage but is not recovery. The
successor TLPool replaces that storage with a retained allocation and persists
the joined prefix. Workers read immutable distinct inputs and write distinct
results. The controller reads each result only after join. Repeated collect
after completion must not call pthread_join again; disposal checks all created
handles, not requested count.

The adapter injects EBUSY before the actual join at each ordinal. It is a
failure of the teaching adapter with known nonconsumption, not a promise that
an arbitrary libc EINVAL/ESRCH preserves a valid thread identity. The controller
retains the allocation and pending obligation; if its real-world provenance is
lost, retention/quarantine is safer than inventing success. The tests require
five callback invocations for four joins with one rejected attempt. An error
that updates the output would falsely publish a partial success record and is
caught independently of the sum mutant. Allocation failure has no workers;
partial creation has only an acquired prefix. No finite test guarantees
scheduler service or prevents an indefinitely running worker.

## T25.01.b and creator-cancellation variant

Detach manages implementation thread resources, not the creator's stack or
external input. The forbidden lifetime trace has no defined C value oracle.
The repair changes ownership explicitly: before creating the detached thread,
copy the bytes and establish two logical references protected by the monitor
mutex. A failed create leaves no worker and releases unpublished storage.
While the worker waits at the gate, normal creator return or creator cancellation
must leave exactly its worker reference. The public test overwrites the old
bytes only after joining the creator; sum10 therefore observes the copy rather
than accidentally extending a borrow.

Cancellation is disabled through allocation, attribute setup, creation and
cleanup installation. It is re-enabled only with the monitor mutex owned;
both normal pop and cancellation therefore use an unconditional unlock handler.
A canceled condition wait reacquires the mutex
before cleanup. Cleanup first unlocks, then drops the creator reference using
the common release operation. Both paths use that operation exactly once.
Normal completion can release worker-first or creator-first. Free is protected
by the same mutex; the controller waits for free with that mutex and never
destroys a condition variable containing a waiter. After last release unlocks,
the worker performs no more application-object access, so this establishes the
needed object quiescence even though its detached runtime epilogue can remain.

The monitor's lifetime belongs to the test controller, which is neither
canceled nor detached. Moving it into the creator frame would reintroduce the
original defect. Likewise, a user TLS destructor retaining it would invalidate
this chosen last-use boundary. The first optimized Clang campaign exposed a
blocked cleanup with a mutable automatic `locked` flag changed after cleanup
installation. The selected libc cleanup machinery can use a nonlocal return;
relying on such a changed nonvolatile local is invalid. Removing the flag and
making the handler's lock ownership invariant unconditional repairs the
protocol rather than adding sleeps or disabling optimization. The failed
campaign is retained separately; a later passing run does not erase it.
Asynchronous cancellation is outside this
implementation; ordinary mutex/refcount operations are not generally
async-cancel-safe. A cancel request return is not completion: the test joins
the creator and separately waits for worker object retirement.

## T25.01.c interface diagnostics and bounded edition analysis

Separate translation units establish the installed declarations with _Generic
and retain actual symbol linkage. C start returns int and join writes int;
POSIX start returns void* and join writes void*. Direct pthread error status
must not be confused with errno. Macro availability, header availability,
linkage and execution are different observations. No second C worker is built.
The bad-signature files fail a specific named static assertion. A cast can
compile and still violate the target start-function contract; type spelling or
handle size does not transport an identity between unrelated APIs.

N1570 7.26.1p2 and N3096 7.28.1p2 contain the optional-header condition; the
latter draft's create/join clauses retain integer start/join results. N3096
also records a version-specific thread-local spelling change, which is not a
reason to infer that every runtime offers all facilities. These two draft
observations are the bounded comparison actually inspected. Exact published
C17/C23 text remains unavailable; the delayed criterion requires that source
before its exact-edition claim is closed. Compiler C23 mode is not that source.

## Tool interpretation

ASan/UBSan and ThreadSanitizer are separate builds. The original race_control
intentionally has conflicting unsynchronized accesses: it is executed only in
the disposable detector sensitivity process. Its integer result is never an
oracle. A supported detector must report the intended shared-variable race;
runtime initialization failures or process timeout do not count. Clean runs
of the actual lifetime target remain evidence about those instrumented
histories, not a proof for all schedules, dependencies or implementations.
