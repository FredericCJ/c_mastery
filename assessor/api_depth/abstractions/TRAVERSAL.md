# Private traversal assessment

Bind ordinary/changed observations to T19.07.a/b/c separately. Public tests expose
the small fixture; independently ask for a different tree shape, an invalidation
between two pumps, and a callback that starts a nested walk. Do not give this
analysis to learners or export reference implementations.

07.a: the decisive distinction is logical owner lifetime, structural version and
value version. A live control plus mismatched structural version yields STALE
before read, including at a would-be end. Updating an existing value preserves
the view and changes the later observation. A dead control cannot be dereferenced
to discover its version. `AT_MUTANT_LIFETIME` ignores logical death and must fail;
`AT_MUTANT_STOP` changes strict > into >= and must fail the equal-threshold case.
The expected prefix length for the public length/threshold grid is
min(length,floor(threshold/2)); its sum is k(k+1). This arithmetic oracle is
independent of cursor implementation.

07.b: pull and push agree on visits and stop boundary, not on who owns control.
REMOVE is rejected without modifying the owner. After successful cancel, clearing
the retained callback and changing terminal state proves quiescence only because
all calls are serialized and cancellation during a callback is rejected. The
`AT_MUTANT_CANCEL` return-code-only implementation must be rejected by a later
pump that still invokes the visitor. A correct concurrent variant needs a
separate lifetime and ordering argument, not just an atomic active flag.

07.c: the continuation frame must distinguish enter, left returned, right returned
and leave. Advancing the parent's stage before pushing a child prevents repeating
the child after return. Depth measures active nodes, not total visits. Depth0
with a nonempty root emits nothing/FULL; empty root is END at any depth. Compare
the independently written preorder prefixes for32 bounds and the explicit
invalid-index/cycle cases. `.su` sizes and actual structures support a bounded
cost dossier; they do not prove arbitrary recursion fits a target stack.

Accept different valid representations if all output preservation, callback,
ownership and bound contracts remain equivalent. Fail a passing trace justified
by a false claim that generation numbers make dangling pointers safe, or that
requesting cancellation universally joins callbacks. Native/sanitizer passes
cover the recorded finite cases; inspection plus changed-condition reasoning is
required for the highest score. External review and seven-day attempts remain
unperformed unless separately recorded.
