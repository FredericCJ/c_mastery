# P3 exact protocol and changed-condition assessment

This successor binds the four original criteria of `PR-P3-01` and five of
`PR-P3-02`, including their nine unchanged `-DELAYED` IDs. Read their canonical
records under `curriculum/v5/expansion/legacy_assessments/` and the existing
`labs/checkpoint_depth/CONTRACTS.md` and `transfer/CONTRACTS.md`. Those shared
contracts supply the fourteen required assessment fields. The task-specific
obligations below are required, not optional topics or historical evidence edits.

## Prerequisites and environment

Use C17, octets, the exact-width types and the original first-principles
`struct cp_parser`, `struct cp_queue`, `cp_crc32` and codec. The portable core has no allocation,
I/O or parser/CRC library dependency. Host verification may use Python/zlib,
GCC/Clang, sanitizers and a separate byte-stream driver. All controls and extents
are live, aligned, disjoint, initialized and accessed by one serialized owner.
Do not replace wire encoding with a structure overlay or infer pointer lifetime
from an integer check. Host runtime dependencies do not enter the portable core.

Later durable transfer explicitly requires the accepted receipt/store producer
and its write/flush/reopen contract. Later authentication requires a provisioned
synthetic key, trusted key identity/epoch, replay policy and the inspected
OpenSSL3 primitive contract. The existing hosted profile explicitly permits
that primitive for protocol composition; CRC itself remains first-principles.
Neither later dependency is a prerequisite for the first framing attempt.

## Exact representations and publication

For `T04.06.b`, the original frame has sync A5, version1, sequence258 encoded
least-significant octet first, length3, payload00 41 FF and four CRC octets,
for13 bytes. Use the supplied fixture, offset source/output buffers and an
independent swapped-host representation. Explain why conversion from numeric
value, rather than native memory order, preserves the wire. The changed task
allows that independent model; it does not require pretending a native run is
an executed big-endian target.

For `T15.12.b`, original v1 accepts length0..32 and total10+length, including
sequence0 and65535. Unsupported version, oversized length and truncated CRC
publish no message. Duplicate/gap policy belongs to a consumer. The v2 transfer
adds flags and an optional timestamp, includes every extension byte in the CRC,
and makes the unchanged v1 decoder reject v2 after the version octet.

**Current clarification of the earlier transfer wording “Keep v1 unchanged”:**
the baseline `struct cp_parser` keeps its32-byte limit and wire layout. The experimental
`struct cpx_parser` source applies its configured16/64 payload limit to both versions;
that is a changed-admission experiment, not identical acceptance to baseline v1.
When testing compatibility, explicitly use the original v1 parser or a32-byte
admission limit and record the difference. No claim of backwards-compatible
v1 payloads above32 follows from unchanged field layout. This versioned
clarification preserves the earlier source and receipts.

For `T15.12.c`, derive reflected IEEE CRC32 with polynomialEDB88320 and initial/
final XORFFFFFFFF. Check `123456789` and the independent zlib fixture, a corrupted
payload and a changed payload whose attacker recomputes CRC. In the latter case
the framing check succeeds: it is not authentication. The later authenticated
packet binds domain, key ID, epoch, sequence, length and payload before replay
state/output publication. Preserve wrong-key, header/payload tampering, replay,
capacity refusal, primitive failure and maximum-sequence tests. Reset of the
volatile replay ledger remains an explicit counterexample to durable replay
protection.

For `T18.01.b`, cover empty/max/partial/malformed/unsupported/bad-CRC and output
capacity boundaries. Sentinel values show refused encoding and `peek` leave
outputs unchanged. The changed in-place encoder permits only the specified
payload-at-buffer-start overlap and shifts it backward after validating the
whole request; ordinary encode keeps its disjoint contract. Prove each source
read remains live before its destination overwrites it.

## Streaming, refusal and faults

For `T15.13.b`, run every split of the13-byte frame, then all4096 compositions,
and refuse a completed message. A ready parser consumes zero new bytes until
admission succeeds. Pause across concatenated frames and count candidate,
pending-message and queue storage separately; bounded storage means upstream
backpressure, not unlimited ingestion during refusal.

For `T15.13.c`, the bad42-byte candidate contains a valid empty frame at offset6
and is followed by the known frame. Restart at rejected-sync+1 to recover the
nested frame. Count shifts, candidate inspections and CRC work under repeated
A5 input. Configuring maximum payload changes candidate capacity and the
O(NB) rescan bound, B=maximum wire length. A fixed runtime admission limit does
not shrink an already compiled C array.

For `T21.05.b`, complete validation before queue publication. Refusal preserves
the ready message and any consumer sequence state. Once capacity is available,
admit once, then invalidate the pending state; repeat delivery cannot duplicate
it. The later actual receipt/store adapter keeps queued, held, written, durable
and acknowledged states distinct. Retain the same operation on uncertain
write/flush failure, reopen and retry without applying its effect twice. Its
two-image device and74 finite cuts are models, not physical power interruption.

For `T32.06.a`, reset the complete parser/queue per input, cap input at256 bytes
and use deterministic random seeds and refusal schedules. Compare accepted
messages with the separate whole-buffer/zlib oracle. Record operation bounds,
the actual tested corpus and sanitizer/coverage limitations. Consumer refusal
decisions are explicit fuzz input; they must not make replay nondeterministic.

For `T32.06.c`, retain the original seed, violated property, reduction transcript
and repaired regression. The original skip-candidate defect loses nested
synthetic sequence7 while exiting normally. One-byte-deletion minimality is
different from globally shortest input. Your minimizer receipt must contain
`passed`, `single_byte_deletion_minimal`, `original_hex` and `minimized_hex`.
`check_replay.py` accepts that receipt and an independently implemented oracle;
it does not supply a private reference implementation or an assessor answer.

Implement `cpa_admit` from `learner.c`. It wraps the actual fixed `struct cp_queue` with
an explicit admission capacity1 or2. Refuse invalid capacity/count; preserve
ready state on full; otherwise invoke the actual producer delivery operation.
Do not mutate the admission limit while messages are owned. This changes the
allowed occupancy, not the two-element physical representation. Initial queues
contain harmless sequence900..(900+capacity-1), causing the unchanged reduced
input to encounter genuine full-queue refusal before the consumer resumes.
Replay original and minimized inputs at chunks1,7,256, both capacities, and
80 initial refusal turns. The output oracle includes the prefills and the
original valid messages. Each replay starts with fresh state. Require observed
refusal, occupancy never above the chosen capacity, preserved pending output,
normal child exit, and the same missing-sequence7 property in the faulty parser.

The driver bound is `8*(input_bytes+CP_WIRE+pause+capacity+1)` outer turns.
After the finite pause, each turn drains an item or consumes/rejects input or
finishes; at most256 input bytes and42 retained candidate bytes can generate
that bounded work. Internal shifts/CRC are separately bounded by candidate
size, so the outer bound is not an O(1) claim about each turn.

## Deliverables, verification and assessor rubric

Keep the original D1 sealed reasoning, D2 first-principles implementation and
exact prerequisite dossiers, D3 reproducible positive/negative evidence and
D4 independently performed changed task after at least seven days. Submit all
nine objective mappings, not just a single “protocol tests pass” statement.
Retain exact compiler flags, source hashes, oracle disagreements, refused
publications and changed storage/work calculations.

The existing codec/stream, configurable protocol, authenticated packet and
durable-sink campaigns remain valid only at their pinned scopes. The new
replay campaign independently compiles original/repaired/skip/drop/scaffold
cases under GCC/Clang O0/O3/ASan+UBSan and retains each actual replay packet.
Wrong-value mutations must fail the intended assertion; no crash, timeout or
compile rejection may stand in for the missing-message property.

Level1 shows a successful encode/decode. Level2 must explain each of the nine
mechanisms above, preserve the exact contracts, expose the relevant mutant
and distinguish CRC/authentication, refusal/failure and write/durability.
Level3 requires the exact changed assumptions and the original delayed
independent assessment. Author runs do not confer either learner independence
or absolute mastery. Assessor solutions remain in their existing private
producer directories and `assessor/checkpoint_followthrough/protocol_analysis.md`.

Hint1: track input consumption, candidate bytes, ready message and queue as
different owners. Hint2: refuse admission before clearing ready state. Hint3:
when minimizing, preserve the specific wrong-output predicate while changing
one assumption at a time; do not accept a crash as a smaller equivalent witness.

## Security containment and remaining evidence

The authentication target is the original local teaching packet with synthetic
public test keys. The attacker controls packet bytes and replay order, not the
configured trusted key. Harmless success is an unauthorized synthetic sequence
or state update in the flawed target. The repair authenticates the entire
header/payload and commits the replay ledger only after all checks succeed;
rerun the same mutations and refusal/failure regressions. No external target,
real credential, network service or production device is involved. Encryption,
key provisioning, durable revocation and universal side-channel security are
outside this bounded protocol contract and remain separately assessable.

Current author-validation receipts distinguish retained from newly executed
work. Final ISO-text access, independent review, elapsed learner attempts and
physical target observations remain separate. Original baseline identities,
descriptive hours and historical receipts are preserved.
