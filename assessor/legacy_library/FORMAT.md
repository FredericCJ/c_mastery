# T12.09 assessor reasoning and withheld transfer

The prefix invariant is written=min(n,c-1) for c>0 and0 otherwise, with n still
an int until its negative case has been excluded. A count equal to capacity is
truncation: no room remains for all bytes and NUL. Expect (required,written,
terminated,complete)=(6,0,0,0),(6,0,1,0),(6,3,1,0),(6,5,1,0),(6,6,1,1)
at capacities0,1,4,6,7. Guards beyond capacity remain unchanged. Negative-return
injection must preserve the metadata; prefix mode promises no output rollback.

For the delayed transactional variant, use separate failure injection at query
and render, not only insufficient destination space. Render can write a prefix
and then fail. The reference keeps that effect in staging. Compare counts across
both calls and check the terminator before committing. A growing/changing input
between calls makes the sizing argument invalid; callback purity is an explicit
precondition and the count checks catch only some violations. Two independent
results can have equal lengths and different content, so equal counts do not
prove snapshot consistency. Valid alternatives include a caller-supplied staging
buffer with an equally explicit ownership/capacity contract.

Width/precision expected bytes come from padding arithmetic, not a second call
to the formatter. The UTF-8 prefix E2 82 is intentional: it is a valid narrow-byte
formatting observation and an invalid complete scalar encoding. The typed policy
fixes actual varargs pairs; its tag is a runtime convention, not a C type-level
proof. Compile the original wrong pairs only, with exact intended format
diagnostics. Do not promote a compiler warning's absence to ISO validity.

Three compiled mutants target independent defects: converting a negative result
before rejection; treating equality as complete; rendering directly into public
storage before failure is known. Require assertion failure with the matching
test, not a compile error or crash. Keep the learner stub's expected failure
distinct. For delayed assessment withhold this file, reference source and raw
reference outputs. Retain each original AX-...T12.09.a/b/c-DELAYED identity and
its seven-day rule; author execution is not the learner's attempt.
