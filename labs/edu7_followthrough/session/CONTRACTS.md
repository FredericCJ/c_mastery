# EDU7 distributed session and upper commit, revision1

This required additive contract implements the unchanged PR-T40-06 ordinary
criteria AX-PR-T40-06-T40.06.a/b/c and each original seven-day V1. The parent,
legacy assessment shared contract, actual E7Session contract and the explicit
EtTx/EtRx transport contract apply. No historical baseline evidence is replaced.
Learner files are session.h, learner.c, tests.c and explicit prerequisite stubs;
replace stubs with the independently completed prerequisite artifacts.

## Prerequisites and inspected source

Import actual E7Session prepare/commit/token/resume, EtTx/EtRx, original L6
record codec and synthetic L7 application. These implementations are separately
identified in each build; a component name is not sufficient. Inspect the exact
original L5 fields and named producer passages registered in
research/v5/expansion/legacy_edu7_session/resources.json. This packet claims no
fresh inspection of an entire protocol textbook or published ISO edition.
Prepare a diagram distinguishing sender transport completion, receiver session
acceptance, committed application state and a durable external effect.

## Environment and limits

C17, strict profile E, fixed caller storage, no allocator or protocol library.
Native host verification may use stdio, Python and sanitizers. Cortex-M4 executes
actual compiled C with explicit original byte helpers and no unresolved target
symbols. Host tools never become target dependencies. Six native profiles,
actual i386 and four Thumb profiles are bounded author evidence. Neither host
nor MPS2 execution proves the electrical two-segment topology. ISO C cannot
validate a lower adapter's claimed acceptance, remote peer identity or lifetime.
The API uses no C23 feature; its C11/C17 unsigned and lifetime premises are the
same for these mechanisms. Exact final ISO-text access is not implied.

## Problem and concrete inputs

Build two distinct EsPeer objects A17 and B51, each with its own E7Session,
EtTx and EtRx. There is no shared permission variable. Negotiate session7,
L5 epoch1, schema1/unit1/initial holder17 through actual L4 fragments and ACKs.
Queue TOKEN A→B, deliver it, lose its final ACK, then attempt DATA at both peers.
Queue B's reply while A still awaits matching completion: A must retain complete
incoming data without publishing it until that uncertainty is resolved.

SET request9/value2500 at A checkpoint1 commits once at B. Transfer the token;
B's response uses B checkpoint1 and is a different directional receipt. Repeat
the original SET under a later L4 transfer ID: replay the recorded response with
no new effect. Change its bytes and inject malformed L6 at A checkpoint2. Refuse
without advancing the committed checkpoint. Repeat with insufficient response
capacity and prove every byte of state/output remains unchanged.

Reset only quiescent transport through the actual profile2 greater-incarnation
API while retaining L5 RAM. RESUME checkpoint1 succeeds; stale L5 epoch and a
rebooted peer without receipts refuse. CLOSE retains epoch retirement; a greater
L5 epoch starts empty receipts. Finally execute the supplied intentionally flawed
external-pulse adapter: the synthetic pulse survives a reset after application
work and before receipt commit. Explain why replay produces two pulses.

## Necessity and representation

One coordinator cannot demonstrate two-peer lost-completion safety. The original
EduSession commits before upper acceptance and owns one checkpoint; it cannot
establish the directional and transactional requirements. This successor uses
the actual E7Session transaction rather than renaming that original state.

EsPeer stores identity, phase, independent incoming receipts, copied transport
state, one outbound-kind/transfer binding, local sent checkpoint/copied last outbound bytes, pending ERROR identity and synthetic
application state. Incoming receipt[0] concerns A DATA; receipt[1] concerns B DATA.
At B the former is used; at A the latter is used. The unused opposite receipt
must remain unchanged. sent_checkpoint records acknowledged outbound L4 delivery;
it is explicitly not evidence that the remote application committed an effect.
A valid transport ACK can coexist with an L5/L6 rejection.

OPEN and OPEN_ACK are exact10-byte records: type, BE16 session, BE16 epoch,
checkpoint0, schema1, unit1, holder17. DATA is the same7-byte header plus1..80
L6 bytes. TOKEN has checkpoint0 and one new-holder byte. RESUME has no payload
and names the sending direction's checkpoint. CLOSE has checkpoint0/no payload.
A refused message with a valid nonzero identity records a bounded ERROR tuple.
An explicit es_emit type7 emits its exact seven-byte ERROR control, even while
closed; arbitrary unrelated error identities refuse. A matching inbound ERROR
sets sticky uncertainty, never new token permission. The L5 wire is unchanged. Transport profile2 uses
its explicitly different protocol0x84 epoch envelope; profile1 retains protocol4
and its no-reuse limitations. L4 incarnation and L5 epoch are distinct.

## Ownership, operations and invariants

All controls begin zeroed and remain live. One serialized nonreentrant owner
mutates a peer; inputs/outputs/control are disjoint and valid for the call.
Snapshots are read-only. Fixed input bytes are copied. Do not zero live state to
escape retirement or introduce concurrent ISR access without a new contract.
Each operation is bounded by120 transport bytes,80 session bytes and fixed
state-copy cost. No recursive callback, unbounded allocation or retry loop occurs.

es_emit stages the whole peer before lower begin. Admission failure leaves it
unchanged. A begins OPEN only while closed; B prepares OPEN_ACK only after valid
negotiation. A becomes READY on valid OPEN_ACK; B becomes READY after its matching
ACK completes. DATA needs READY, no uncertainty, no outbound operation, local
holder and no TOKEN_PENDING. Outbound checkpoint is next positive16-bit value, or an exact copied replay of
the last acknowledged outbound payload/checkpoint. Changed bytes reject; a
replay is a new L4 transfer but no new L5 checkpoint.
Checkpoint/epoch/transfer counters exhaust rather than wrap; a new namespace
requires explicit quiescence and negotiation, not a cast.

es_offer/es_admit distinguish copied staging from accepted lower transmission.
es_receive produces only the actual transport ACK and retains a complete SDU.
es_deliver separately validates L5/L6 and copies an EsEvent. FULL or BUSY retains
all peer/output bytes and the complete SDU. A malformed/stale message is consumed
without changing application/session state or output; its valid identity may
replace the pending bounded ERROR tuple. Successful delivery consumes
the held SDU; cached replay returns ES_DUPLICATE. The event's response is copied
and remains independent of subsequent receive storage reuse.

For new DATA, stage the actual E7Session prepare, decode L6, run the actual
synthetic application against a private setpoint, encode its reply, check response
capacity, then commit and publish in one serialized operation. Malformed input
never commits. An application-level invalid SET may commit an ERROR receipt,
with no successful SET effect. A receives only RESPONSE/ERROR verbs; B receives
only GET/SET. Responses do not recursively invoke the application.

## Token failure and recovery

A TOKEN sender invokes actual token_begin before transmission and retains the
outbound transfer/incarnation binding. The receiver changes its own holder once
only after completed reassembly and validated TOKEN. A duplicate lower fragment
gets its retained ACK and never delivers TOKEN again. The sender changes its own
holder only after an actual matching final transport ACK. Old IDs/incarnations
are rejected by the imported transport; a public epoch is not authentication.

The invariant is at most one permitted endpoint, not guaranteed progress. After
lost ACK the receiver may hold permission while the sender remains pending.
The sender may buffer the peer's later DATA but cannot publish it prematurely.
Replaying TOKEN obtains a duplicate lower ACK; matching completion resolves the
sender's holder and the queued DATA can proceed. A timeout or cancellation cannot
assign authority back. es_uncertain requires exact incarnation/transfer, sets a
sticky uncertainty flag and retains pending state. Even later completion does
not make that peer locally permitted without an explicit recovery protocol.
Close refuses pending token/request. Recovery from an uncertain retired transfer
requires externally established lower/peer quiescence and fresh negotiation;
this packet does not pretend a forced local reset proves those premises.

## Resume and irreversible effects

Reset only transport when both transmit and receive controls are quiescent,
using the actual greater-incarnation APIs. Retained L5 bytes, checkpoint, response
and identity must match. The sender may know only L4 delivery; its RESUME can be
refused if the receiver rejected upper data. Rebooted RAM cannot invent receipts.
The flawed cut_effect teaching adapter uses actual prepare/decode/application,
then increments a synthetic external pulse and resets before commit. Repeating
it gives two pulses for one intended operation. This is a local state-model
counterexample, not real device I/O. A durable repair requires an atomic effect/
receipt transaction or a durable idempotent command identity plus recovery;
a RAM assignment, checksum, retry or token is insufficient.

## Deliverables, verification and rubric

D1: exact wire/ownership diagrams, per-direction invariant, staged-publication
proof, failure taxonomy and bounded costs. D2: implementation, prerequisite
identities and exact lost-ACK/cancellation/duplicate/malformed/resume traces.
D3: independent expected TLV bytes, compiled negative controls, sanitizer and
changed-data-model results, final Thumb helper/map/stack audit. D4: the original
changed-condition artifact independently attempted after at least seven days,
including its failed unaided prediction if any. D5: a separate list of physical,
external review, durable-protocol and learner observations still absent.

Run dialogue/receipts/resume. Compare whole state/output, effect counters and
actual packet bytes; a zero exit alone is insufficient. Diagnose mutants that
release token early, accept incompatible negotiation, commit before upper
validation, ignore capacity, restore authority on cancellation, alias directional
receipts, accept stale epoch, invent reboot receipts or repeat duplicate effects.
The independent model enumerates finite token transitions and proves only that
model's invariant. Retain actual logs/tool identities; do not label an author's
run an independent learner assessment.

Criterion.a level0 grants both peers or conflates ACK with negotiation; level1
matches one happy exchange; level2 derives and demonstrates two-control lost-ACK
safety and exact transfer binding; level3 independently handles uncertain cancel
without restoring authority. Criterion.b level0 commits malformed input; level1
counts an ordinary SET; level2 demonstrates distinct directional receipts,
transactional capacity and cached duplicate output; level3 transfers prepare/
commit to a new response-capacity/failure schedule. Criterion.c level0 promises
durable exactly-once from RAM; level1 names reset; level2 distinguishes real
retained transport reset, stale identity, reboot and close/reopen; level3 exposes
the pulse/receipt cut and derives an adequate durable recovery contract.

## Graduated hints and changed conditions

Hint1: draw two peer objects and mark permission separately from successful
transport. Hint2: leave the sender pending when the receiver accepts TOKEN but
its ACK disappears. Hint3: stage application and receipt together; compare a
new request with an exact cached replay before changing any state. Record the
unaided attempt before consulting successive hints; assessor analysis is separate.

Original .a-V1 cancels an uncertain TOKEN; .b-V1 changes prepare/validate/commit
and cached-response capacity; .c-V1 cuts after external effect before durable
receipt. IDs and seven-day minimum are unchanged. Additional transfer to
concurrent owners, authenticated negotiation, persistent recovery or a different
schema requires explicit new contracts and fresh proof/measurements.

## Security scope and validation

Only original synthetic peers, forged local records and an original flawed
pulse model are executable targets. Attacker capability is arbitrary bytes or
replay in this isolated unauthenticated teaching network. Harmless outcomes are
an extra local pulse, premature authority or synthetic duplicate effect. No
external service, real credential, production bus or deployed device is used.
The repair is staged validation, exact state/identity binding and conservative
uncertainty; cryptographic authentication and durable commit remain absent.
No crash or sanitizer result is equated with code execution.

Source authored, executable, actually executed, externally reviewed, independently
assessed and physically observed are distinct statuses in the current receipt.
Finite passing tests do not prove absolute mastery or universal distributed
liveness. PR08 still requires the full two-segment routed exchange.
