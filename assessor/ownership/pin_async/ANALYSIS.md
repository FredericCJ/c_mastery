# X-T66-008: authority is later than notification

The retained pin is a checked C protocol, not Rust's language/library ownership boundary. A raw copy of a self-referential struct retains the old interior pointer; compare it while both objects are live. Thread7 is only a model value. The host adapter's pthread_equal check is stronger for this execution environment but does not validate arbitrary memory or authenticate a process.

## R1 and R2

The product state must agree: pending implies one FAsync operation, while a notification only records a result. Cancel and timeout preserve both pending and the borrowed address. Reserve one future release before starting, otherwise an operation admitted at releases=MAX can never clean up. The final generation MAX is usable once; refusal after that must not obstruct terminal cleanup or an idle rebind. State/output publication is staged for the pure core, while external backend effects require different recovery arguments.

The real worker may write after the owner has requested timeout. It records notification before calling the foreign receiver, then records done after the receiver returns. The owner's join is the final actual termination fact. Do not clear started or release the buffer on notified/done. Failed pthread_create has no worker: restoring the old core is valid. A failed pthread_join does not justify reclamation. Under the valid-object and no-cancellation premises pthread mutex/condition errors are excluded; no general corrupted-pthread recovery is claimed.

## R3

The independent event oracle models full state and unchanged output, not a copy of the C representation. It has37449words including rejected events, but finite no-wrap traces do not replace explicit MAX tests. OPA controls1-4 remove distinct core obligations; retained OW controls8/18 remove pending/affinity checks. Actual thread tests coordinate the receiver so the notification-before-return interval is observed deterministically, without assuming a scheduling distribution. TSan observations remain evidence of observed executions only.

## R4 and delayed device transfer

Publish pending before Pd enable. Configure refusal cannot have begun transfer, so owner rollback is safe under the provider contract. Once enabled, timeout cannot roll back external writes. The actual Pd state machine's matching-token/EN/FIFO/TC checks and finalize establish its modeled authority. The adapter must retain ownership until those calls complete; OPA5 and PD5 separately break those obligations. Error results conservatively distinguish completed bytes from possibly written bytes. A retained finalized result prevents a retry from issuing a second finalize if later owner publication unexpectedly refuses.

The native model's post-timeout array writes are real C writes, not physical DMA observations. The selected F439 ranges are numeric validation inputs, never converted to native pointers. The stable scope object owns actual arrays. Physical reuse requires the separately specified evidence plan and remains not run. Reject any submission calling this a physical DMA test, claiming EN=0 alone proves quiescence, or treating possibly-written extent as a safe retry prefix.

## Assessment and cost review

Require the dependency source identities: actual FAsync producer, actual external Pd source and exact public import copies. A missing provider must refuse configuration; an unfinished provider must compile but fail the named semantic anchor. Record host versus target dependencies. Require all thirteen cost dimensions in PIN.md, including the difference between constant bounded control operations and unbounded scheduling/join/device wait. The actual size, stack/object and seven timing samples are descriptive for the tested toolchains, not universal ABI/performance bounds.

The delayed exercise is not complete as learner mastery because the author supplied a model. Keep independent learner and physical evidence dimensions unresolved until they are separately performed. A bounded author-team source inspection, if later recorded, is not an independent human assessment.
