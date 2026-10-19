# Sequence/context assessor analysis

The public tree does not pretend a Unicode scalar fits RPNode.value. Its byte
is an item-table token; the imported deterministic traversal supplies token
order, and an explicit encoder supplies scalar bytes. Unknown deletes retain
identity either outside the item table or as placeholder items. Mode0's extra
queue is extra capacity, not a free optimization. Canonical snapshots sort
identity records and reconstruct topology, so arrival-dependent item indices
are not serialized semantics. Hidden ancestors remain traversable.

Compaction is a coordinated epoch transition, not local garbage collection.
All three equal admitted histories and a genuinely drained closed transport
allow replacing them with one visible chain. The unchanged-visible-order proof
must account for descendants of hidden nodes. Old epoch messages are refused;
restoring an old snapshot does not establish current authority. A hash check
cannot prove distributed quiescence or authenticate a peer.

The OT protocol deliberately restricts generated contexts to committed server
prefixes and permits only one outstanding operation per site. Thus each suffix
commit already uses the coordinate system reached after the prior suffix
commits. Rebasing the request one commit at a time maintains a common context.
This is stronger than merely comparing a componentwise vector and skipping
some arbitrary local log entries. It avoids claiming a general decentralized
transformation theorem. No speculative client state is displayed.

Delete-over-delete of the same token becomes no-op but still advances the
operation vector. A client receiving another site's commit must keep its own
pending original request. The server retains original requests for exact
identity/retry checking and transformed commits for rebase/delivery. The
three-operation axb/abx demonstration isolates the distinction: B's original
position0 belongs to empty state, while the retained position1 belongs to the
state after A. Applying a correct pairwise formula to the wrong one violates
the chosen context semantics. Reversing a tie-break to patch one example is
not a general repair.

The Python oracle uses tagged token deletion targets and an explicit insertion
gap marker, then compares complete server/client state. Its fixed histories and
named controls falsify particular errors, not every schedule or intention rule.
Ten controls isolate early-delete/scalar/barrier/epoch/vector/rebase/delivery/
replay/no-op/pending errors. Preserve normal semantic rejection and current
source pins. Public passes alone earn at most level1; the delayed changed
context and complete invariants must be assessed independently.
