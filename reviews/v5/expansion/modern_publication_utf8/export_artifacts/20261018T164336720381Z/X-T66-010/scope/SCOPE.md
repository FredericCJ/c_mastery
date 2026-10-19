# Conditional cleanup, jump conversion and retained recovery ownership

Required X-T66-002/R1-R4 and X-T66-002-DELAYED after X-T66-001. Import the actual
early T65 er_run/er_raise result boundary from PR-T65-06 and its errors.result
provider PR-T65-04, plus current ow_defer/ow_finish/ow_publish and ow_allocator.
This is the early T65-to-T66 edge. It does not require the later T65 owning-scope
follow-through in return, and does not create a prerequisite cycle. The learner
explicitly supplies a completed errors workspace. C17, GCC/Clang, hosted Linux;
no extensions, asynchronous cancellation or freestanding claim.

## Ordinary cleanup and result conversion

The existing eight-entry OwScope retains ALWAYS, ON_ERROR and ON_SUCCESS callbacks.
Registration is ordered; finish pops every entry, invokes only selected callbacks
in reverse order and empties even skipped entries. Repeated empty finish is inert.
The supplied bounded diagnostic callback attempts registration and finish during
finish: both reject without mutation. This checks the stated guard, not arbitrary
reentrancy safety or permission to bypass the callback contract.

OcsPlan admits0..8policies, a fail_before value0(disabled) or1..count, and an
ER_OK..ER_CONTRACT outcome. ocs_run prevalidates all fields plus unsigned external
counter headroom for count effects. Invalid input preserves report and counter.
For each admitted registration step, increment the external effect counter, then
register one callback with ID1..count. An injected failure immediately before step k
raises ER_IO through the actual active er_frame after k-1registrations/effects.
Otherwise the specified nonzero outcome is raised after all registrations.
The normal case returns the registered count. All contexts modified by the body
live outside the actual setjmp-containing er_run invocation.

After er_run returns, record original status and convert any error to ER_CONTRACT.
Only then call ow_finish with the original success/failure outcome. Each callback
records its ID and the already converted status it observed. This converts an
error at a boundary before final cleanup; it does not let cleanup itself longjmp.
Publish the complete report after cleanup. The effect counter retains successful
increments on error: a resource cleanup protocol is not a semantic undo log.
For ALWAYS1,SUCCESS2,ERROR3, error trace is3,1 and success trace2,1.

Actual ow_publish is separately exercised with six real typed allocations:
three current and three staged. Publication consumes staged ownership before
releasing the old three. Every permitted old-release observer sees the new value97
and an empty staged owner. Matching allocator callbacks release exact bases.
This is externally serialized publication, not atomic publication to readers.

## Changed contract: a close action may retain ownership

OcsRecovery is a fixed-address, noncopied, initially zero control with up to eight
owned OcsEntry records, a finite generation, active/finishing/sealed flags and an
accumulated diagnostic count. Begin requires an empty inactive owner and returns
an owner-pointer/generation ticket. It increments without wrapping; UINT32_MAX
is issued once and still permits final cleanup, then future begin is RETIRED.
Old tickets reject before effects. The control must remain live even for stale
ticket checks; a generation does not validate a dangling native pointer.

Push transfers one unique resource identity to a callback/context entry on success.
Null closer, duplicate live identity, capacity8/9 or invalid ticket refusal leaves
ownership with the caller. Identity0 is valid. All callback contexts remain live
until their resource is released; numeric IDs are not authentication. The test
adapter owns actual int32_t regions obtained through unchanged ow_allocator.

The first recover seals registration and invokes the top closer. Its exact result:

- RELEASED: ownership ended; pop and continue toward earlier dependencies.
- RETAINED: no release occurred; stop with the top entry and all earlier entries
  retained. Report recovery_required, pending count and accumulated close errors.
- RELEASED_ERROR: ownership ended despite a diagnostic error; increment the error
  count, pop and continue. Never retry that identity.

The callback must report one of these exact states. An uncertain release cannot
be silently represented as RETAINED; a backend with uncertainty needs an additional
authority protocol. These are original teaching adapter semantics, not a blanket
claim about every operating system close API. Because registration is sealed and
released entries are never retried, close_errors is bounded by eight. Failed
attempts may occur indefinitely; no liveness bound is claimed for a retaining
backend. Each recover call performs at most the remaining count of callbacks.

While finishing, reentrant push/recover rejects BUSY unchanged. The supplied
diagnostic closer tests that rejection only. On admitted recovery, BUSY instead
means the report was published and an owned top entry remains; OK means all entries
were consumed and the control became inactive. A terminal duplicate ticket is
STALE and preserves output. New begin resets the diagnostic count but never
changes the generation of a live recovery obligation. No destruction or reinit
may discard an active owner merely to silence a retained-close error.

For resources0,1,2 with2retained on its first close and then released with error:
first recovery calls only2 and keeps all three typed regions. Second calls2,1,0,
releases each exactly once, returns error count1 and leaves no live region.
An external effect already published before this cleanup stays published. The
old void callback is not reinterpreted as a fallible callback or called twice.

All control/input/output/context storage is live, nonoverlapping and externally
serialized. Callbacks are synchronous, do not exit nonlocally, retain no borrowed
control pointers past their permitted lifetime and do not mutate the owner.
The two explicit rejection probes are the only admitted reentrant calls. No
thread-safe ownership, exception propagation or automatic destruction is promised.

## Required evidence and assessment

D1 distinguishes resource ownership, semantic rollback and irreversible external
effects; derive selection order and retained-top dependency safety. D2 implements
scope.h, imports actual providers and records every transfer/refusal. D3 runs the
independent reversed-list oracle:2735policy/failure histories through depth4 and
341retention/error-mask histories through depth4. Capacity8/9, duplicates, stale
tickets, final generation, publication observers and rejection probes are explicit
C anchors. D4 seals the fallible contract before implementing recovery, then
attempts the independent changed case after at least seven days.

Six anchors cover policy, jump conversion, capacity/reentry, publication, retained
cleanup and identity exhaustion. Eight compiled controls delay conversion until
after cleanup, drop a retained entry, retry an already released error, accept a
stale generation, deactivate a pending owner, advance a live recovery generation,
or compile actual OW_MUTANT2/12 for unconditional/FIFO cleanup. Only exit1 and the
named OW_CHECK assertion count as semantic rejection. Learner stubs compile and
fail the selected anchors; a sanitizer abort is not a passing negative control.

R1 assesses effects and protocol distinctions; R2 exact existing API composition,
publication and ownership; R3 complete policy enumeration, failure cuts and traces;
R4 actual typed-jump conversion and separately implemented fallible recovery.
Nominal reproduction earns1, complete justified bounded evidence2, independent
changed-condition reasoning3. Lost ownership, retried released identity, reversed
dependency violation or an unsupported longjmp/safety claim is a hard failure.
Hints: filter a reversed list; record ownership before effects; separate converted
status from cleanup policy; retain the failed top entry; distinguish diagnostic
failure from still-owned resource. Assessor analysis is separately packaged.

## Cost review (thirteen dimensions)

1. Representation: eight callback/context/identity entries plus finite state;
   record actual sizeof separately from any wire layout.
2. Work: O(n) policy finish and up to remaining n callbacks per recover attempt.
3. Space: bounded stack/context and caller-owned recovery record; no recursive unwind.
4. Allocation: core allocates none; publication/recovery tests use actual typed
   OwAllocator, with release poison and live counts as independent observations.
5. Traffic: copy admitted entries, pop once, publish report after terminal decision;
   a retained entry remains in place across attempts.
6. Dispatch: actual er_run/ow_finish plus cleanup and fallible close callbacks.
7. Code size: retain optimized object/disassembly and provider symbol observations.
8. Stack: emitted native .su records distinguish adapter context from er_frame.
9. Timing: seven10000-run error/conditional-cleanup samples; checksum490000 and
  280000retained external increments, not a physical latency guarantee.
10. Failure: invalid admission preserves outputs; admitted RETAINED publishes a
   recovery report; RELEASED_ERROR records a diagnostic with no retry authority.
11. Scheduling: serialized owner; blocking, async callback and cancellation models
   require another contract and are not inferred from these tests.
12. Builds: actual completed errors import, clean/incremental observations and
   exact C17 compiler/source closure, not a copied assessor dependency.
13. Portability: unsigned epoch never wraps; exact-width support and actual library
   setjmp implementation remain environment premises. Published C17 access remains open.

Only original local resources and synthetic effects are used. No external target,
host setting changes or physical claims. Author campaigns do not establish
independent learner mastery or an independent human assessment.
