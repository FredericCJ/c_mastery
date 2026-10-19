# T68.09 technical analysis and sealed transfer

Context authority is stronger than a counter value: callers must not retain
tokens from a later epoch and then reuse the old authority. Runtime visible
context checks cannot prove the absence of other live tokens. Staging allocation
of contexts permits transactional failure even after several recursive calls.
Each call reserves a distinct context, including empty calls. MAX is never
issued; MAX-1 may be issued once, leaving an exhausted sentinel.

Repetition binds a selected list index, not a syntax context. A nested CALL
preserves that index and allocates a fresh syntax context; independent nested
repetition binds the other list. Zip requires equal lengths; truncation loses
input. Reject rebinding an already active list because this two-flat-list model
does not represent Rust's nested fragment tree. Explicit ESCAPE chooses the
immediate parent's context, while ordinary arguments preserve their full token.

Token budget alone cannot bound zero-output recursion. Depth bounds frames and
step budget bounds invocation/instruction work, including empty invocations and
zero-length REPEAT instructions. All failure paths discard the staged output
and authority. The independent oracle uses explicit pending actions rather than
the C call stack, exposing ordering/budget mistakes separately.

Reverse environment resolution compares symbol/context pairs. The correct
caller token resolves11; retagging or comparing only the symbol selects99. These
are original local semantic defects, not executable attack chains. Uniform
context assignment is intentionally different from Rust mixed-site local/item
lookup, as the actual Rust positive and negative translations demonstrate.

Controls1..6 retag nested arguments, truncate unequal repetitions, admit one
excess depth, publish contexts on failure, bypass escape permission and ignore
context in lookup. Controls7/8 compile actual retained MOD17/18. Failures are
ordinary assertions, not out-of-bounds access or sanitizer substitutes.

Sealed case: use an empty first repeated body that consumes a context, followed
by an independent-list nested repeat containing CALL and ESCAPE. Require the
candidate to predict parent versus caller contexts and exact failure at a
step budget just below completion. Change initial authority to MAX-2 and require
unchanged output despite one completed invocation. Swap argument list lengths
to distinguish zip failure from a valid Cartesian product. All operations are
implemented in the author support; the precise unseen program is separately
held. Preserve original R1-R4 and record unaided effort. Finite tests and
author-team source review do not establish independent learner mastery.
