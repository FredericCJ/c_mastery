# Streaming nested events, revision 1

This is the complete shared learner contract for X-T15-019. It is an original
bounded language derived from selected XML structure rules and SAX event
semantics. It is **not an XML processor** or a conforming SAX binding.

## Language and representation

Use ISO C17, exact uint32_t and uint64_t, ordinary character octets and explicit
size_t lengths. No parser, string conversion, allocator, DOM, container or memory
library implements the target mechanism. Python Expat is a host verification
tool only. The core has no allocation, recursion, VLA, file access or network I/O.

The byte grammar is `document := ws* element ws*`,
`element := '<' name '/>' | '<' name '>' content '</' same-name '>'`, and
`content := (text-byte | fixed-entity | element)*`.
`name := [A-Za-z][A-Za-z0-9_-]{0,11}` and ws is space, LF or TAB.
Text admits those three whitespace bytes and printable ASCII32..126 except
literal '<' and '&'. Fixed entities are exactly `&amp; &lt; &gt; &quot; &apos;`.
Their replacement character is text, never parsed again as markup. A raw '>'
or ']]>' is admitted here; XML has an additional ']]>' rule which this language
does not implement. CR, high octets, attributes, numeric references, XML
declarations, comments, namespaces, DTDs and external entities are rejected.
Names are case-sensitive. There is exactly one root, and close names must match.

The parser stores at most EV_DEPTH8 names of EV_NAME12 octets, one partial name,
one partial fixed entity, one EV_TEXT text chunk and one owned pending event.
Runtime depth_limit1..8 is enforced before another element is admitted.
EV_TEXT16 is ordinary; EV_TEXT1 is a separately compiled changed condition.
Event data has no terminating zero; only `[0,length)` is readable. START/END
carry a name, CHARACTERS carries nonempty decoded text, DOCUMENT has length0.
START and END depth is the element depth (root1); text is current depth;
DOCUMENT depth0. Empty tags produce both START and END in that order.

## Lifetime and execution

Controls, caller input and output objects are live, correctly aligned, disjoint,
uncorrupted and externally serialized. Controls are noncopied. ev_init requires
fresh storage or an inactive terminal parser with no outstanding callback; it
does not detect reinitialization of an active object. Valid initialization
changes no external owner. Invalid pointer/limit returns INVALID unchanged.
The queue and sum initializers require fresh or empty/quiescent storage.
No pointer validity or arbitrary control-corruption defense is promised.

The callback is synchronous and returns normally. Its event pointer borrows
parser storage only until return. It may copy the event's scalar fields and
valid data bytes; it must not retain the pointer, mutate the parser, destroy any
live control, or call ev_init. Reentry through feed/finish/cancel is explicitly
detected as BUSY and changes no state or output. Concurrent calls are outside
this API; ISO C does not supply a lock or automatic lifetime safety.

## Prefix and backpressure

ev_feed consumes exactly the returned prefix; a consumed byte is already
represented in parser state, an accepted event, or the owned pending event.
The caller may overwrite/reuse that prefix after return. An unconsumed suffix
must be resubmitted exactly once, without copying already-consumed bytes.
NULL plus length0 is legal and drains a pending event without pointer arithmetic.
Invalid admission and BUSY preserve `consumed`; admitted terminal calls set it0.
Syntax/limit failures include the offending consumed byte in the prefix.

ACCEPT means the callback committed the whole event. WAIT means it committed
nothing; the parser retains the identical event and sequence for retry. No
later event or input byte is admitted before that event is accepted. External
effects performed before WAIT violate the callback contract and are not undone.
REJECT or an invalid callback result latches CALLBACK; already accepted events
and external effects remain visible. There is no whole-document rollback.

Parsed structural state may advance before the matching event is accepted;
pending delivery is therefore part of the invariant. Each new event gets the
next nonwrapping uint32 sequence. UINT32_MAX may be issued once; a subsequent
event latches LIMIT without wrapping. The callback never sees a duplicate
accepted sequence. Multiple calls for a WAIT event are expected.

Text flushes at EV_TEXT bytes or before the next '<'. Call boundaries do not
force text flushing. Consumers must concatenate text segments at the same
logical position; neither a single call nor a single TEXT event equals a field.
After fatal syntax/limit/callback error no further callbacks occur. Cancel
discards unaccepted pending state, latches CANCELLED and makes no callback.
Already terminal cancel returns that terminal outcome unchanged. Finish drains
pending events, then requires DATA state, a closed root and depth0. Otherwise
it latches SYNTAX, including empty input or an unfinished entity/tag/root.
Successful finish offers one DOCUMENT event; WAIT retries it, ACCEPT yields
DONE. Later finish/feed return DONE without another event. These rules do not
convert a partial accepted event prefix into a successfully completed document.

## Separate consumer and copied queue

EvQueue owns two copied events; offer returns WAIT without mutation when full.
Pop copies the oldest event, removes it, and preserves output when empty
(PAUSED). Event/control/output objects must be disjoint. The callback must be
paired with a live queue; evq_offer is the concrete nonblocking handler. Draining
the queue and retrying the parser supplies backpressure without a borrowed
input buffer or retained callback pointer.

EvSum is independent of the parser: it consumes validated event sequences for
`<sum>` containing `<g>` groups and `<n>` decimal leaves. Groups contain groups
or leaves. Only whitespace is allowed outside n; n has one or more digits and
no child elements. Leading zeros are allowed. Each leaf fits uint32; the total
fits uint64. It checks sequence, depth, names, checked multiply/add and complete
document state. A refused event preserves the entire consumer; its caller
chooses to stop or repair the producer. Missing, duplicate or reordered events
are errors, not implicit retries. Only `done && depth==0` authorizes publishing
the final sum as a complete-document result. Queue drain alone is insufficient.

The test's direct total-boundary injection is a white-box arithmetic check,
not a reachable ordinary transcript. With the present uint32 event sequence,
each completed leaf requires at least three events and contributes at most
UINT32_MAX. That bounds every reachable total below UINT64_MAX. The explicit
total-overflow check is conservative support for changed event-counter models;
it does not establish that the current public API can reach that failure.

## Costs, failure and security

For N input octets, K events, bounded name length L and depth D, scanning is
O(N + K*L), with bounded O(D*L + EV_TEXT) parser storage. Repeated WAIT attempts
add callback work; no progress guarantee exists without consumer acceptance.
EvSum uses O(D) state and O(text length + name length) per event. The copied
queue adds two events; a DOM would additionally retain document-sized content.
Actual sizeof/target section/frame outputs are measured separately from these
models. Function frame size is not whole-system stack or physical latency.

Only original local synthetic strings, controls and queues are targets.
An input sender can supply wrong nesting, excessive depth/name length, entities,
truncation and numeric overflow; a deliberately flawed local callback can
violate WAIT's no-commit rule. Success is a wrong synthetic event/sum, duplicated
delivery or bounded refusal, never a claim of arbitrary execution. Source
mutants target missing name match, lost pending event, text-fragment reset and
missing consumer sequence advance. Repair the particular invariant and retain
its positive/negative controls. No external entity resolution exists; that
does not make arbitrary XML safe or supported. Resource limits bound admitted
state, while CPU use under repeated pauses and application side effects remain
explicit caller responsibilities.

## Validation boundary

Six C17 host profiles, independent Expat event comparisons and compiled semantic
controls are finite author evidence. Strict Cortex-M4 object/link audits check
the actual helper closure at both text bounds, without executing a board.
Published ISO C17/C23 source access, physical timing and independent learner or
human assessment are not inferred. The primary reference scope is recorded in
the exercise; only exact inspected clauses support comparison claims.
