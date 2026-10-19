# PR-T19-04 stream successor analysis

The completed offset is the sole retransmission boundary. Return zero must
stop this pump, whereas retry consumes budget without moving it. Error after
earlier accepted writes cannot restore the receiver; preserving offset is the
basic recovery guarantee. Start stages a complete frame before committing and
increments generation without wrap. Both supplied mutations violate an
observable property while all storage remains live: duplicated bytes and a
post-cancel callback. Neither demonstrates arbitrary code execution.

For delayed partial-error transport, a signed count alone is insufficient to
carry both progress and diagnostic. Require a result struct with accepted count
and disposition, commit the accepted prefix once, then retain/terminate according
to disposition. Do not infer that retrying a terminal syscall is universally safe.
Caller/transport side effects and lifetime remain explicit external premises.
