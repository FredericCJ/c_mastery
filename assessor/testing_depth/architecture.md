# Assessor analysis: PR-T32-01

The independent list uses append and front deletion; its expected values do
not call the C diagnostic or repeat ring arithmetic. Enumeration includes all
length-seven words on four operations, so all shorter prefixes occur; this
does not prove arbitrary histories. The separate repeated-fill/drain suite
forces wraps at capacities1..8. Linked chain partition checks are diagnostic
only. The same count2 contract compares a physical3-slot spare ring with a
physical2-slot counted ring and dense array; changing visible capacity would
invalidate the comparison.

Failed push and empty output preservation require both state and storage
snapshots. The full-push mutation intentionally writes a byte without changing
the return code, and the empty mutation clobbers output. Missing count decrement
violates both public behavior and the structural invariant. Partial batch is a
different API: a FULL result with accepted2 is a committed prefix, not rollback.
Heap allocation by the hosted harness is independent of the core call graph.

Decoder validates before output publication. The flow owns both staged values
after begin; modifying the original packet is harmless. It advances the
accepted offset exactly once, counts attempts including zero progress, and
never invokes a callback after a terminal result. Prefix+ERROR retains accepted
progress but forbids further sends. Invalid reports set uncertainty because
the component cannot infer external effects. The offset and timeout mutations
are rejected by exact state/byte/call assertions, not crashes or tool failures.

POSIX MSG_NOSIGNAL changes signal delivery, not the EPIPE result. The one-byte
adapter is an intentionally capped syscall; the separate saturation run is
the actual kernel short-return observation. Closing both ends and matching
all locally accepted/drained bytes makes the real workflow inspectable. A
successful send still does not establish peer processing or durable delivery.

Plain/trap-only component symbol sets must be empty on the selected native
target. Instrumented objects introduce asan/ubsan or gcov entries; the link
maps attribute those to verification, while malloc/free belong to tests.c.
The coverage invocation creates data only in its private temporary build tree.
A no-library link is inspected, never executed as a valid startup image.

Require learners to explain why byte-identical script and socket outputs have
different environmental authority. Logical tests cannot determine UART baud,
ADC error or unavailable emulator device behavior. Do not award level3 from
the supplied oracle or reference run; retain actual independent delayed work.
