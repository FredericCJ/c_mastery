# Private identity and ownership assessment

09.a: allocation order is slot0 generations1,2,3, then slot1 and slot2. There are
nine identities per epoch; epoch itself supplies only three such batches. A stale
capability may outlive an active borrow, so quiescence alone cannot justify wrapping
all identity bits. Require the candidate to give that counterexample. Wrap and
ignored-quiescence semantic controls must compile and then fail public assertions.
Accept another bounded representation if it retains all original contracts.

09.b: after root removal, strong cycle counts are1,1; ordinary strong counting
has no reachability information. Edge capabilities remain in the test so deliberate
cycle breaking can release them. With a weak back-edge, dropping the first root
destroys its payload but not control until the edge owner drops; no promotion from
zero is allowed. The mutex linearizes promote/drop. If promotion increments1 to2
first, a concurrent drop leaves1; if drop first makes0, promotion returns DEAD.
The control mutex cannot safely be acquired through an unowned dangling pointer.
The resurrection mutant must fail a deterministic dead-promotion test, not rely
on winning a scheduler race. Require balanced five sequential and289 threaded
control/payload allocations. Explain the max3 weak count includes its implicit owner.

09.c: owning two valid strong references proves retention, not synchronization
between payload increments. Reject any answer that treats a correct stress total
as proof or assigns a required total to the racy specimen. The local TSan report
is a bounded diagnostic witness; the language-level conflicting actions/no-order
argument is decisive. The mutex repair has20000 increments; read only after joins.
For immutable publication the reader's strong reference is reserved *before*
publication and a separate root survives until join. Atomic pointer load then
retain is not used and does not become safe merely because refcounts are atomic.

Delayed assessor variants keep the original minimum seven days. Change the slot
identity width independently of active regions; race promotion against the final
release with an explicit weak owner; remove the retained root from a publication
proposal and demand a reclamation protocol before execution. External review and
independent learner performance have not been supplied by this author campaign.
