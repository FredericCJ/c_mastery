# Configuration and symbols: equality survives movement

A configuration table turns bytes into stable meaning. Start with linear lookup
and explicit ownership. Add hashing only after measuring the query mix and
stating which bytes define equality. A hash collision is not equality; a changed
address is not a changed symbol. Tombstone deletion must preserve probe reachability.

PR-T14-04 and X-T14-002 establish finite probing and full-table termination.
Later X-T74-002 owns binary symbols, including embedded zero bytes. Rehashing
adds a failure-atomicity obligation: allocate or reserve the replacement, migrate
without losing the old lookup behavior, then publish one complete replacement.
If allocation fails, the old table and borrowed-key promises must still hold.

Derive expected lookup work only under an explicit key/hash assumption. Construct
adversarial keys separately; measure locality at the same load factor and with the
same equality cost. A good random-key mean is not a worst-case bound. Amortized
rehash work requires its own argument and does not bound one update's latency.

Reuse symbols in the compiler and managed runtime. When collection moves objects,
revisit whether the table stores object addresses, stable handles or owned keys.
A weak interning table adds liveness and reclamation obligations beyond hashing.
The dispatch consumer adds callback reentrancy and type-erasure contracts.

Transfer asks for identity shared across a reload while old readers finish.
Choose ownership and migration semantics before choosing the dictionary. Reject
an alternative with a concrete failed lifetime or collision trace and explain
what evidence would make that alternative acceptable under a weaker contract.
