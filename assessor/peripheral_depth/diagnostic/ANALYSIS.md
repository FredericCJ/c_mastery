# Assessor analysis — retain outside learner exports

This packet assesses the original T28.12.a/b/c tasks and unchanged delayed
variants. Do not give a score3 for executing the supplied reference. The learner
must explain its invariants and solve the changed transport/board problem after
the original minimum interval; the author can validate those fixtures today.

## 12.a

The host adapter must reject MSG_TRUNC **before** treating the retained36 bytes as
a complete datagram. A37-byte message with a valid36-byte prefix demonstrates the
failure without undefined behavior. The parser cannot detect truncation if the
transport silently discards the37th byte. Waiting for readability belongs in the
host driver; its initial pilot exposed the incorrect assumption that send implies
immediate receive readiness. One bounded poll supplies the needed precondition.

The Ethernet length field excludes padding, whereas an EtherType parser cannot
infer upper-protocol length. A minimum60-byte frame containing a6-byte diagnostic
payload must not pass46 padded bytes into the strict message decoder. Descriptor
acquisition creates a borrow; both success and application rejection release it.
Correct FCS subtraction handles splits at61,62,63, which can yield zero exposed
second length. A returned ED_Borrow never licenses indexing beyond its lengths.
The actual ED producer decides admissible descriptor flags; this adapter does
not independently reinvent or weaken those conditions. Phase2 transmit BUSY is a
retained allocation state, not transaction rollback.

Delayed USB counterexamples are separate: SET_ADDRESS before successful status
changes the address too soon; a new SETUP makes an earlier status token stale;
CLEAR_FEATURE must reset the toggle even when HALT was already clear; duplicate
DATA packets after a lost ACK must not dispatch again; a final full packet without
the chosen protocol's ZLP leaves a larger host read pending; cancel return alone
cannot free a request still owned by the provider. The original model's five
compiling mutants target these distinct mechanisms. Do not count a compile error
as a semantic rejection.

The generic Linux gadget documentation describes completion as deferred. The
inspected selected dummy controller has a small-IN FIFO-copy path that directly
gives back from `queue`. The guest therefore sets ownership before submission,
never holds its own callback lock across the call, and records inline completion.
Buffer return is not host consumption: the FIFO copy can outlive that request.
For OUT cancellation, dequeue may complete later on another controller; requeue
is a separate command conditioned on callback-returned ownership. Disable is the
separate quiescence operation used before freeing requests. Host URB cancellation
retains its struct and payload until a matching reap. A finite guest trace says
which path occurred, not that every controller behaves the same way.

Alternative implementations may retain input borrows rather than copying only
if they amend the caller lifetime contract and prove publication/quiescence for
every error and reset edge. C's type system does not supply affine ownership,
borrow checking or exhaustive state transitions. Dynamic checks reject stale
tokens; compiler diagnostics catch some incompatible calls; runtime stop evidence
and programmer discipline supply the remaining boundaries.

## 12.b

The representation is a byte protocol, not `sizeof(DGMessage)` or a packed struct
overlay. Read16BE uses integer promotion within range; byte order is explicit.
The cursor checks bounds by subtraction before pointer formation. The staged
output means a failed decode cannot leak partial publication. The sequence policy
uses mathematical order on1..65535, not modular “newer” comparison. Skipped
sequences are legal; acceptance of65535 retires this incarnation. Any proposed
wrap policy must carry an independent incarnation or a bounded-lifetime argument.

For a stream, the outer length is essential. A valid frame followed by a second
frame may arrive in one read; the API reports the first consumed prefix, retains
one ready frame and leaves the suffix for the caller. Rejecting an inner replay
still consumes its known outer frame. An invalid outer length loses framing, so
the chosen fail-stop rule poisons the stream rather than guessing a boundary.
EOF after one length byte is truncation; EOF with no buffered bytes is ordinary
end. The decoder's all-sequence oracle uses Python arbitrary integers and network
`struct.pack`, not the C shift loop or its own round-trip as sole expected output.

## 12.c

Require separate evidence for descriptor ownership, PHY/link, protocol parsing
and application effects. Correct golden bytes survive a PHY replacement, but
register identity, straps, clock source/routing, MDIO divider assumptions, link
mode and electrical measurements require reassessment. An emulated register bit
does not prove an external pin transitioned. A software stack inclusion says
nothing about its selected configuration or the target's received bytes.

The board experiment must identify fixture source identities, reset conditions,
capture location, frame/FCS policy, instrument bandwidth/sample rate, timebase
correlation, uncertainty, load, link renegotiation/loss and cleanup. Require
readback and a known negative control per claimed layer. PHY replacement is an
implemented assessment variant even though nobody has yet attached the board.

Grade each original criterion independently. An ordinary byte round-trip cannot
substitute for USB enumeration/ownership or the changed-PHY dossier. Wrong-layer
claims, a missing required variant or a false ISO/ABI/device equivalence fails
that criterion despite green tests elsewhere. External review and physical or
independent learner evidence remain their own dimensions.
