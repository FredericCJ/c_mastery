# Withheld ownership and lifecycle analysis

Require the ten exact ordinary/changed pairs in the public specification. The
callback descendant obligation is assessed separately, not inferred from these
resource counts. The active/mask mismatch is a valid C object but invalid domain
state; admission must reject before trying to use any resource.

The resource ledger's lost acknowledgment is deliberately BACKEND with ownership
already acquired. The operation key remains queryable; replay while held does
not acquire twice. This bounded idempotency is not crash persistence or perpetual
deduplication. Each event includes resource,key,acquire/release. Require exactly
one release per successful acquire and no release for pre-acquisition failure.
Mixed cleanup closes in reverse order and retains the first cause even while
continuing independent cleanup. A failed consuming close is not retried; a
failed retaining close is. An alternative policy is acceptable only when its
contract explicitly changes and its caller migrates accordingly.

Raw pointer/struct copies do not perform a logical move. The caller-owned
AcShared control remains live even after its payload is released, allowing a
weak operation to reject without reading freed control. Without that separate
lifetime premise a strong-count test is itself unsafe. The two-node cycle has
one internal reference per node after outside drops, so neither reaches zero.
Removing one edge and dropping its owned reference releases both. Reject an
answer that calls ordinary reference counting a cycle collector or claims that
the tiny recursive example establishes bounded stack use for arbitrary graphs.

Generation retirement is finite capacity: each of two slots permits three
lifetimes. Old token checks occur after each new allocation, which rejects a
mutant that checks only live status. The lease alternative checks at most127
ticks of modular elapsed time and admits token ages0..3. Its proof assumes the
client obeys maximum retention, and a full-cycle timestamp reuse is a required
counterexample to overclaiming. A longer generation counter reduces practical
frequency but does not logically prove absence of wrap.

The FBuffer producer is imported rather than cloned. Empty and rejected appends
do not change its generation; successful nonempty append does. The changed
value revision is independent: a structural FView remains usable after an
in-place value update, while a saved value snapshot rejects. A reset invalidates
the structural view. The live flag guards logical destruction only while the
control storage still exists. SQLite comparisons must distinguish its source
contract from this teaching policy; never execute a stale SQLite pointer to
obtain an expected result.

Constructor failure records FAILED/mask0 after releasing its prefix. A snapshot
of the published state is assessed fieldwise; comparing padding bytes is not
logical equality. A PARTIAL object may be stopped, but an uninitialized storage
region is not a NEW object until constructed as such. Matrix rows are NEW:
init; PARTIAL/ACTIVE/FAILED:stop; STOPPED:stop,destroy,restart; DESTROYED:none.
The optional resource changes ACTIVE's required mask to1, not its successful
state tag. Epoch exhaustion is rejected before creating an un-stoppable session.
An external handle table solves access to a state tag only; it does not keep a
returned object alive without a separately defined borrow protocol.

At least7days later present the original changed tasks and a fresh resource
whose close reports failure after consuming its handle. Require a revised
cleanup ledger and a first-error trace, then change the lease retention bound
and ask for a concrete stale-identity history. Also require the optional-resource
matrix and a restart timeline invalidating prior views. Full credit requires
explicit assumptions, independently derived counterexamples and working mapped
artifacts, not a reproduced pass count or a claim that C enforces ownership.
