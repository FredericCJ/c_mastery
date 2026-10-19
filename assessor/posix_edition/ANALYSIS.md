# Assessor-only POSIX edition reasoning and transfer

For the close model, the independent expected table is the concise oracle.
Issue7 leaves ownership unresolved for the selected EINTR/EIO outcomes. Linux
close retires the descriptor for those outcomes. Issue8 permits EINTR only with
restart semantics and retains the name; EINPROGRESS retires the name while the
untrackable close operation continues. A diagnostic does not imply retained
ownership. EBADF invalidates the caller's ownership premise, rather than proving
that this invocation freed a valid resource. Invalid flags can still close.

Translate into MC-style fallible cleanup only after checking the platform and
exclusive-owner preconditions. Retired/error and retained/error are different
results. The old unspecified case cannot be mapped soundly to either without
additional platform authority. An adapter must refuse unsupported profiles
before acquiring a resource or carry a separate unresolved state with an explicit
containment policy. Quiescence of users and completion of asynchronous I/O remain
separate from descriptor retirement; a close-only retry is not a new I/O permit.

Plausible errors: retry every error, erase ownership for every error, infer
Linux conformance from a C17 compiler mode, read errno after a successful call,
or equate the retired handle with durable data. The three close mutants compile
and fail the table. An all-reject stub fails semantically too.

Read admission: first exclude impossible ownership tuples. The initialized
self-writer case is not a promise of EDEADLK. Other writer ownership blocks.
Without waiting writers, acquire. With no scheduling option, preserve the
implementation-defined choice. In the selected scheduling population, an
existing reader may reacquire despite a waiting equal/higher-priority writer;
a new reader may not. The two read mutants respectively erase this recursion
exception and turn a permitted implementation choice into a universal preference.
These changes are not equivalent alternative implementations of the model.

For maximum read count, refusal may be EAGAIN; there is no portable numeric
maximum to insert into the learner model. A signal does not turn a wait into
EINTR. Wrong answers that propose a portable execution oracle for uninitialized
locks fail the semantic objective even if one runtime returns EINVAL.

## Withheld delayed package

Wait at least7days after the ordinary attempt; record actual dates rather than
manufacturing the interval. Supply a cleanup API that returns only `int error`,
and a destructor that automatically retries nonzero results. Require a revised
representation and migration plan supporting both retained EINTR and retired
EINPROGRESS. Expected result: an independent retirement field or tagged outcome,
single-owner state transition, and no retry after retirement. A boolean error
alone cannot encode the necessary distinction. Add a caller cancellation between
the native result and wrapper publication; require cancellation masking or an
owner journal at that boundary, not a claim that this pure classifier solves it.

For read admission, replace the homogeneous scheduling population with mixed
policies. The correct response identifies the model boundary and obtains the
applicable authority before choosing an answer. Also provide three successful
recursive read acquisitions and only two unlocks: one acquisition remains owned;
the writer can stay blocked. Ask whether a passed finite trace establishes
starvation freedom (no). Score ownership, authority selection and progress
reasoning separately. External reviewer identity and learner artifacts remain
unpopulated until actual independent work exists.
