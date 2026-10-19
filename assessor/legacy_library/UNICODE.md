# Assessor analysis: original T12.05–06 and withheld variants

The two-pass implementation makes a whole call transactional without a heap or
an output-sized stack array. Copy at most the fixed state, walk without publishing,
then replay from the original state. This relies on stable disjoint input and a
valid API state; it is not concurrency-safe against an unsynchronized writer.
Only successful preflight guarantees that replay has sufficient capacity and no
encoding rejection. Complexity remains linear even when recovery retries a byte:
each retry first discards a nonempty retained prefix, so it cannot retry forever.

Second-byte restrictions prevent impossible prefixes immediately, rather than
retaining ED A0 until a future byte. The recovery rule emits one replacement for
ED and reprocesses A0 and80 as two separate invalid starters. For F0 9F 41, emit
one replacement for F0 9F then41. For a call containing valid A followed by a bad
prefix, strict output must remain unchanged despite the earlier valid scalar.
At capacity exhaustion after a retained prefix completes, preserve that prefix
so a retry can succeed. Mutants separately remove range checks, swallow the
offending byte, and publish failed state. They must compile and fail the intended
UNICODE-CHECK assertion, not merely crash or be rejected at translation.

The independent Python codec covers every Unicode scalar encoding in UTF-8 and
UTF-16LE, all65536 two-byte sequences in strict/replacement modes, and4096 seeded
multichunk inputs with capacities0..4. It uses codec state snapshots to model
whole-call rollback and independently checks pending bytes, result counts and
untouched guards. This is bounded history testing, not a proof that the two
implementations have no correlated assumptions. The RFC derivation supplies the
separate semantic argument; visible C examples keep key counterexamples inspectable.
Python can defer ED A0 as an incomplete prefix; our contract rejects it immediately.
The oracle therefore constructs the set of extendible prefixes from Python's
complete scalar encoder and flushes an impossible deferred suffix. This avoids
copying the C range algorithm. A second discovered distinction is error priority:
strict malformed input wins over insufficient capacity, so validation preflight
uses the full scalar count before comparing output capacity. Both counterexamples
remain in explicit C regressions and the retained failed experiment receipts.

Withhold the following original seven-day packages until the ordinary task is
submitted. T12.05a: trace a signed FF narrowing versus fgetc's int256-domain and
EOF sentinel; reject "casting EOF to unsigned char recovers EOF". T12.05b: supply
65 CC81 and require separate byte/scalar/grapheme/display contracts; no property
database or renderer has been implemented by wchar_t. T12.05c: give independent
locale needs to two threads and require object ownership, restore-before-free,
and real conversion/classification evidence without process-global mutation.
T12.06a: interleave every split of F0 9F 98 80 with Euro, using separate state.
T12.06b: change to the exact replacement rule, require input-consumption and
capacity rollback counterexamples, and finish an incomplete stream explicitly.
T12.06c: provide the actual COFF wchar2 manifest and require the original UTF-16
adapter, pair derivation, wire endianness choice and explicit library-execution
gap. Do not grant Windows runtime credit from a COFF object alone.

Accept other representations if they prove the same grammar, resource and
transaction contracts. A one-pass approach needs enough private staging or a
revised explicitly approved publication contract; writing then decrementing the
length does not undo visible output. A valid UTF-8 byte sequence need not be a
safe identifier or match another parser's normalization. Assess each original
criterion independently; the full scalar oracle gives no credit for the locale
ownership explanation or delayed independent performance.
