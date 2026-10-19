# T66.02 cleanup and recovery analysis

An ordinary scope is a reversed-list predicate filter. Removal happens whether
the callback runs or is skipped; count is unprocessed entries. Guarded rejection
inside finish preserves that count. In the jump adapter, the Context belongs to
the caller of er_run, not the function containing setjmp, so its changed fields
are available after the jump. No cleanup callback raises. Convert the returned
error before calling ow_finish; each observer must see the converted status.
External increments are intentionally not rolled back by resource cleanup.

Publication assigns the new owner and consumes staging before old-release
callbacks. Their read-only observer sees97 and empty staging. This ordering does
not create synchronization with another thread. The reference tests compose the
actual retained typed allocator/constructor/destructor/scope and error boundary.

Fallible recovery changes the contract. RETAINED means the action did not release;
the callback and every earlier dependency remain owned. RELEASED_ERROR removes
the entry exactly like RELEASED and increments only a bounded diagnostic count.
It confers no retry permission. Sealing registration on first recovery bounds
diagnostics by the initial eight entries and prevents new obligations from being
inserted around a partially completed stack. Later begin starts a fresh finite
generation only after the previous owner is inactive. Generation exhaustion never
prevents the last live stack from draining.

The model tests all finite retain/error masks through four entries and an explicit
eight-entry boundary. The actual callback adapter retains/releases real typed
regions, not only counters. Mutants are rejected by defined state/trace assertions;
the source does not deliberately double-free native heap storage. The original
language comparison is bounded: automatic Drop handles ordinary scope ownership,
but a retryable fallible close requires an explicit result and retained owner.

Ask for a backend whose outcome is uncertain, not merely retained: the three-state
contract is insufficient without another authority/reconciliation state. Also ask
why an unbounded retry loop cannot imply progress, why BUSY during a rejected
reentrant call differs from an admitted pending-recovery result, and why a copied
ticket is not an independent owner. These are mandatory reasoning distinctions;
passing finite histories does not certify arbitrary callbacks or learner mastery.
