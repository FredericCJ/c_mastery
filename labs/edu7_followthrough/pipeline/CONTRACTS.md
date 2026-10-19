# EDU7 three-node pipeline and physical evidence boundary, revision1

This required additive packet serves unchanged PR-T40-08 criteria
AX-PR-T40-08-T40.08.a/b/c and all original seven-day V1 tasks. Apply the parent,
legacy assessment shared contract and actual foundation, physical, transport
and session contracts explicitly. It replaces no accepted baseline identity,
criterion, historical hour estimate or acceptance receipt.

## Prerequisites, learning objectives and inspected references

Supply completed learner artifacts EfFramer/EfRoute, PuDriver/PuRx, R5Delays,
EpTx/EpMaster/EpSlave, EtTx/EtRx, E7Session, EsPeer and original L6/L7 codecs and
application. Each build names actual source paths and contracts. The source
register research/v5/expansion/legacy_edu7_pipeline/resources.json identifies
inspected original T40 L1–L7 fields, PR08 tasks and actual producer code. Existing
TI/ST inspected scopes are inherited through the physical producer; this packet
does not claim additional unseen book or standards passages.

Explain how a successful SET, a received frame, a routed datagram, a retransmitted
fragment and an application receipt differ. Derive a full reverse trace that
keeps node/port/request identity while changing local link/route responsibility.
Assess an all-green CPU run without physical captures honestly.

## Environment and permitted dependencies

C17, profile E, fixed caller storage, no allocation or preimplemented networking
library. Native GCC/Clang verification may use stdio and sanitizers; Python is a
host oracle, never target code. Four Cortex-M4 GCC/Clang O0/O3 builds execute the
actual three endpoint/relay controls against four original UART register models.
Separate non-model F439 builds retain the actual selected GPIO/UART board adapter
and audit final symbols, assembly, map and explicit helper closure. They are not
run on deployed or physical hardware. Target byte helpers are supplied explicitly;
no implicit runtime or division provider is silently admitted. C11/C17 rules used
here agree; there is no C23-only feature or claim of final-text inspection.

ISO C governs storage, arithmetic and callbacks. UART TC, RCC/GPIO and selected
pins are vendor/board contracts. Model timing and the chosen10us scheduler are
experiment assumptions, not guarantees of actual clocks or analog wiring.

## Concrete problem and supplied artifacts

Implement pipeline.h/learner.c and board_learner.c. Create A17, R34 and B51 as
three separate controls. A is the left-segment master; R.left is its slave.
R.right is the right-segment master; B is its slave. Four independent UART/DE
owners carry actual encoded bytes. R owns no initialized EsPeer, E7Session or
application state. Every forwarding decision uses the actual copied EfRoute
producer; every endpoint datagram uses actual EtTx/EtRx and EsPeer.

Starting plant1800/setpoint2200, route SET42=-4000, exact cached replay, invalid
SET43=12501 and GET44. Transfer the token for each reverse reply; do not bypass
L5 to simplify the test. Expect one accepted SET effect, retained setpoint-4000,
ERROR43 with-4000 and REPLY44 with immutable plant1800. Simulate a refused response
attempt while B lacks the token; retry of the committed request returns the
cached result without repeating SET. Then grant B authority and emit its reply.

Capture the actual transmitted request and reverse response bytes on both
segments. Explain node17/51, ports7/8, transfer IDs, per-direction checkpoint1,
request42, hop4→3, local addresses and recomputed CRC. Independent oracle.py
unstuffs the bytes and checks CRC/fields/TLV values without calling C encoders.

Inject reverse-left disconnect after the receiver commits, a370us input pause
at the17th encoded byte of a DATA frame, one corrupted byte, unknown route and
hop1, malformed TLV containing an escaped delimiter, incompatible OPEN, nonholder
DATA, held upper output and peer reboot. The unchanged physical fixture remains
pending physical; CPU evidence cannot fill its missing captures.

## Necessity, representation and integration invariants

A one-way helper calling each layer once does not demonstrate full ownership or
recovery. EwNode owns one or two EwPort objects, one copied relay slot, an endpoint
session only on A/B, one upper event slot and bounded counters/trace snapshots.
Each port owns one copied pending DATA frame and its actual master/slave driver.
The relay preserves end-to-end payload and nodes, decrements hop once, rebuilds
link fields and encodes a fresh CRC. It never reads L5/L6 application fields.
Profile1 is original protocol4. Profile2 explicitly stages protocol0x84 to the
original route codec, forwards its opaque epoch-bearing payload and restores
0x84; this is the registered changed-incarnation profile, not silent compatibility.

All controls are zero-initialized, live, aligned and single-owner. Inputs/output
and control storage are disjoint. Callback contexts outlive the port and do not
reenter. Initialization may configure quiescent hardware before refusal; it is
not a transactional hardware rollback. A copied queue acceptance releases the
caller borrow; BUSY preserves the queued object. Do not overwrite exposed state
except read-only diagnostic snapshots and explicitly identified fault injection
in the original laboratory targets.

Fixed work: queue≤40 payload bytes; frame≤96 encoded bytes; one received UART
sample and bounded protocol operations per tick; relay copy≤40; session≤80.
No recursion, allocator or unbounded retry loop exists. Retained target stack
frame sums bound serialized call paths under the stated no-concurrent-interrupt
profile. Exposed counters are finite diagnostics, not external truth.

## Operations, scheduling and failure behavior

Time is nonwrapping uint32 microseconds and nondecreasing per node, leaving60000us
headroom; imported transport uses floor(now/1000) milliseconds. The master begins
at most one DATA then POLL per turn, respecting the prior grant's25ms minimum.
It waits for the completed response or its separate first/end deadlines. The
slave sends one DATA or EMPTY only after a validated POLL and200us guard; its
own pre-enable200us policy remains distinct. Local echo never becomes peer data.
The actual PuRx acquires status then data; fault status follows its byte.

A slave DATA parser admits at most one master DATA before the following POLL.
The master parser requires the actual grant. CRC/length/canonical-framing rejects
never produce a network publication. Relay BUSY/dropped datagrams are losses,
not permission to acknowledge unaccepted transport. Endpoints produce actual
transport ACKs; a full local frame slot may lose an ACK, so the unchanged sender
retries. Link-copy acceptance starts the transport timer; it is not final TC,
physical receipt, session acceptance or application commit.

One upper event slot is deliberately bounded. While the caller retains it, the
next complete transport SDU remains held and advertises zero credit. A further
message must probe rather than overwrite either object. Consuming the event
permits the next staged upper commit. A known nonholder DATA with no pending
token is rejected/released at L5, preventing indefinite complete-buffer retention.
A DATA arriving during genuine TOKEN_PENDING remains held until matching
completion resolves that authority; these conditions are not interchangeable.

The two-segment model transfers each octet only after87us of modeled shifting
and leaves10us scheduling granularity explicit. It observes all four actual
model DE outputs and detects same-segment overlap. A disconnect drops received
bytes without changing actual sender-ready or TC observations. Recovery therefore
uses timeout/retry and retained receipt, not a fabricated device-stop signal.
The fresh byte split pauses TXE between encoded bytes; no UART bit is stretched
or fake physical measurement claimed. Corruption is an actual altered wire byte,
so upper behavior follows the original CRC boundary.

## Quiescence, reset and changed assumptions

The .b-V1 resets transport between committed request and reverse response.
Stop admitting application work; ew_pause refuses owned frame/route/event or
transport work. It then stops new master turns while existing byte/grant activity
drains. ew_drained requires every local line/grant owner idle. All three nodes
must be paused and all four modeled shifters/DEs drained before reuse. This relies
on the closed teaching topology having no external sender; local idle alone
cannot prove an arbitrary bus quiescent.

ew_rebind requires profile2, a paused/drained endpoint and greater incarnation.
It composes actual EtTx/EtRx reset while preserving L5 receipts and application.
Resume then uses the same L5 identity/checkpoint under new L4 IDs. After a separately
modeled peer reboot, a fresh valid L4 epoch cannot invent L5 RAM receipts; RESUME
is refused. Never zero a live owner to evade drain or generation exhaustion.
The retained session/token and physical capture limitations remain applicable.

## Selected board adapter and physical acceptance

The F439 adapter composes actual USART3/PD8/PD9/PD10 and USART2/PA2/PA3/PA1 code.
Only R owns both; endpoints use port0. Initialization first obtains immutable
callbacks from a fresh, never-submitted quiescent port, then transfers those
callbacks to the node's fresh owners. No callback borrows the temporary control.
The caller supplies a validated stable PuClock and monotonic elapsed-time source
from its earlier startup/time artifacts; this adapter does not infer them from
a requested baud. Observe PA1/PA2 Ethernet conflicts and the physical producer's
pin/termination/supply constraints. BOARD_CAPTURE.md specifies required raw data.

The software predicate tests completeness of a seven-item physical evidence
checklist; it does not authenticate evidence. Current executed tests pass mask0
and require PENDING. A hypothetical complete mask is never a physical receipt.
The source-bound completion record keeps physical_observation not-run. Only
separately obtained genuine voltage/polarity, termination, bit timing, DE guard,
relay, overlap and disconnect captures can change that dimension.

## Deliverables and acceptance procedures

D1: complete layer/namespace trace with ownership and time origins, copied-versus-
held storage ledger, failure responsibility and fixed resource costs. D2: node/
port/board implementation with actual prerequisite identities and target closure.
D3: ordinary and injected traces, independent CRC/TLV oracle, compiled mutants,
whole-state/effect witnesses, first-divergence explanations and source-pinned logs.
D4: original delayed .a response refusal, .b retained-session transport reset and
.c fresh split/lost-ACK/malformed case after at least seven days. D5: a separately
labeled physical evidence packet; missing data must remain missing.

Run exchange, identities, faults and adversarial. Require exact application values,
all four captured wires, correct route hop/link rewrite, no modeled overlap or
unplanned overrun, genuine zero-credit pause/recovery, no second SET effect and
no application publication after malformed/token/reboot failures. Compare first
incorrect state and precise layer, not just a process exit. Reject compiling
stubs and independent mutants for duplicate effect, wrong plant value, lost hop,
wrong local destination, premature namespace reset, nonholder retention and
synthetic physical acceptance. Target code must link without unresolved helpers;
source-bound board-link inspection is not silicon execution.

## Rubric, hints and meaningful variants

Criterion.a level0 conflates plant/setpoint or repeats SET; level1 reports one
happy reply; level2 demonstrates exact ordinary/error/replay semantics through
all layers; level3 diagnoses response refusal after commit and independently
recovers its cached result. Criterion.b level0 labels every identity as the same
sequence number; level1 names headers; level2 reconstructs actual four-wire
forward/reverse fields and CRCs; level3 drains/rebinds only transport and derives
permitted retained-session response identities. Criterion.c level0 marks synthetic
evidence physical; level1 lists faults; level2 identifies each first failing
boundary and its bounded observable; level3 independently reconstructs the new
split/lost-ACK/malformed campaign and obtains genuinely authorized board evidence.

Hint1: write down who still owns bytes after each apparent success. Hint2: follow
one request42 across two distinct link frames and the reverse reply. Hint3: retain
the session receipt while resetting only proven-quiescent transport; physical
absence is a separate evidence issue. Preserve unaided work before using hints.
The original AX-PR-T40-08-T40.08.a/b/c-V1 IDs and seven-day minimum are unchanged.
Additional transfer to arbitrary networks, ISR concurrency, unbounded routing,
authenticated control or persistent recovery requires new assumptions and proof.

## Security scope and validation status

Executable targets are only the original model and separately authorized teaching
boards. Inputs are synthetic bytes, local drops/corruption and known timing traces.
No production bus, operational vehicle, third-party system, credential or host
security setting is touched. Harmless successes for flawed controls are a repeated
local effect, wrong typed response, bad route or falsely satisfied local gate.
Repair, diagnose and regress each; CRC/session IDs provide no authentication.
No crash or sanitizer result proves arbitrary code execution.

The campaign distinguishes authored, executable, actually executed, independently
assessed, externally reviewed and physically observed work. Finite passing tests
are bounded evidence, not absolute mastery. Full-source/physical/external learner
obligations and exact original parent reconciliations remain visible separately.
