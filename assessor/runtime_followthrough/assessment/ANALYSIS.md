# Private cumulative VM assessment

## 12a

Independent wire derivation:434d is wrong magic; correct ASCII CVM1 is43564d31,
then01 function,0000 entry,0800 length,00 arity,00 locals,01 INT,29000000 operand41,
1601 HOST1,11 RET. Complete hex43564d31010000080000000129000000160111(19bytes).
Three dispatches consume fuel3; one callback consumes host1; adapter1 returns42,
adapter2 returns43 with identical bytes. The private literal oracle does not call
the compiler or codec to compute these bytes. Reject mixed source/image even if
both individual hashes are correct: recompile the claimed source and compare.
Seven controls cover mixed image, mixed source, absent source pin, stale compiler,
forged result with matching forged transcript, wrong adapter and false verifier
transcript. Trust actual capture and compiler/build inputs explicitly; a malicious
producer can forge an unsigned report, and replay is not a proof of compilation.

Score0 wrong chain or stale acceptance;1 hashes without derivation;2 independent
19-byte/result derivation and all boundary records;3 an actual second reader's
clean-export reproduction plus explanation of changed tool/environment limits.
The author has executed export/replay, not acted as an independent second reader.
The separate DELAYED.md packet contains the procedure and blank evidence fields.

## 12b

Verifier pair INT0 RET versus RET alone distinguishes relative result height1
from0 without executing the invalid stream. The exact-source control changes
only h!=1 to permit0; it must compile and fail the verifier assertion. Root pair
fills15versus16 baseline slots: the extra allocation triggers collection only
at16. Both unregistered C-temporary references are passed to cvm_pair; the original
roots them before collecting. The omission control leaves a stale generation
and is detected by checked cvm_field, not an out-of-bounds access. Capacity15/16
is the minimal allocation trigger for this exact H16 mechanism, not a claim of
globally minimal source text. Fuel1versus2 for INT7 RET is terminating even in the
no-fuel control; expected insufficient case preserves91 and consumes all fuel.
Original provider remains byte-for-byte unchanged; controls are generated copies
or the original explicit test macro. No compile failure counts as a semantic kill.

Accepted HOST1 image cannot protect a native context loan. Original slot(epoch1,
live,value42) is reused(epoch2,live,value77) while the C allocation stays live.
Checked callback rejects the expired loan with HOST_ERROR/result91; deliberately
flawed callback returns77. This is a defined logical ownership violation. Do not
call it measured C use-after-free, arbitrary execution or privilege escalation.
Repair is use-time validation and an actual owner discipline, not another bytecode
verification pass. A legitimate alternative transfers/copies the value before
release, provided the service's semantics and lifetime contract permit that.

Score0 mechanism confused or UB golden output;1 mutant merely fails compilation;
2 each minimal pair and actual semantic rejection with repair;3 independent
accepted-verifier/host-lifetime composition and a stated limit for untested mutants.

## 12c

For H16/H8 the native object representation changes while wire grammar does not.
Two distinct rooted pairs have2root+4field visits, plus16/8sweep units. Budget21/13
stops before sweep with all slots still used and gc_budget15/7;22/14succeeds,
leaving2 objects. H8 arena fills8 then refuses ninth; H16 fills16 then refuses
seventeenth. Revisit all allocation/fuel/performance claims dependent on heap size.
No target-profile equivalence is inferred from a hosted build.

Va copies a request/config but borrows store/session lifetimes. Worker exclusively
operates store; main exclusively operates Vs. Mutex synchronizes done/result and
cancelled. Poll joins after observing done, then invokes exactly one matching
vs_complete; actual join is necessary even if done was already published just
before the worker returns. Borrow ends after join, not after cancel/notification.
Create failure cannot leave a half-taken request pending forever: complete failure
and release the unstarted borrow. Hosted --wrap injection exercises actual EAGAIN
handling without changing host limits. It is a test-linker facility, not ISO C.

Actual tmpfile I/O gates final commit flush. Four combinations of cancellation and
reply loss all leave durable-model value15 visible on reopen, but only ordinary
success resumes RET to15. Other cases retain91 and become HOST_ERROR. RetryingID1
returns saved15 without another flush. The precise claim is observed OS file I/O
plus controlled acknowledgment loss, not a physical power-failure guarantee.
Negative async controls report completion while blocked, ignore cancellation,
and alter completion epoch. All are compiled and semantically rejected. Harness
joins a lying specimen before reporting its failure to contain its worker; this
cleanup is not evidence that the specimen obeyed the ownership contract.

Score0 frees on cancel or double-applies increment;1 serialized mock/flags without
actual worker and file I/O;2 actual quiescence, creation failure and reconciliation
plus changed-capacity accounting;3 independent delayed implementation/dossier,
including counterexample to immediate cancellation and arbitrary exactly-once.
Alternative event-loop I/O is valid if it demonstrates real pending operations,
explicit buffer/store ownership and a provider-defined quiescence boundary.

All provider hashes, exact commands and bounded observations live in current
campaign/export/guard records. Parallel authorship is not external review. Keep
at least seven days before learner administration; preparing/validating the
variant today supplies no elapsed learner-performance evidence.
