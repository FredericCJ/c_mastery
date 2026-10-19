# Split failure, operation undo and savepoint analysis

In the ascending seven-key tree, committed0..34 plus insertion35 causes a leaf
split, an internal split and root allocation. Refusing each admission in turn
tests partial progress at distinct depths. The retained store aborts the entire
transaction, so a prior staged key0 replacement disappears and committed0 is
visible again. The new wrapper copies the entire typed engine workspace before
mutation, so the same refusal restores staged99 and leaves the transaction active.
No disk byte is restored: active mutations have no backend IO or external owner
acquisition. The allocation hook is a bounded verifier admission point, not an
actual heap allocator whose resources a struct assignment could magically free.

A savepoint uses the same bounded representation but has a longer lifetime.
It must preserve `saved_root/saved_used`, all node scratch and state, not merely
the current root and allocation bitmap. Exact owner+serial prevents an old token
from restoring a newer saved state. Rollback/release consume the point, commit
invalidates it on every outcome, and MAX serial forbids new admission without
preventing cleanup. A native public struct is cooperative ownership; copying or
reinitializing its address with retained tokens violates the contract.

Once commit invokes IO, restoring old memory is not rollback. The selected
first-write failure leaves the old committed map recoverable; the selected final
flush failure leaves visible new commit bytes and recovery installs the new map.
Both invalidate saved undo authority. Neither observation establishes what a
physical device persisted. Native image snapshots detect the intentionally leaked
dirty home write before commit, while the correct adapter has zero IO calls for
active put/delete and keeps committed image bytes unchanged.

The729word oracle models map, active flag, one saved map, finite issuance count
within its small trace, and generation. It is independent of the C tree/undo
representation. Separate controls cover partial allocation, final serial and
unknown commit outcome. Source-bound six-profile results remain author evidence;
other T47 variants and external learner/human/physical evidence stay separate.
