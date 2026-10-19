# First-principles error correction, revision1

Required exercise X-T15-008 refines PR-T15-10/T15.15.a,b,c. The historical elective
project identity/hours remain unchanged; this extension is required. The active
language is C17. Profile C component uses shifts, masks, loops and caller-owned
storage only, with hosted test/oracle tools separated from the target dependency
closure. `uint8_t`/`uint16_t` availability and eight-bit octets are profile
requirements, not universal ISO type promises. No ECC or bit-count library.

## Representation and operations

Support extended even-parity Hamming(8,4) and(16,11). Positions are numbered1..n,
stored in bit(position-1). Check positions below n are1,2,4[,8]; fill remaining
positions with ascending least-significant payload bits. The final position n
is overall parity. The wire convention is original and explicit; numerical bit
positions are not host-memory endianness. Native struct bytes are never the wire.

`ec_encode` admits n8/data0..15 or n16/data0..2047, then publishes one uint16.
`ec_decode` computes syndrome plus overall parity. Under the premise of at most
two flipped bits: zero flips return EC_OK; one flip returns EC_CORRECTED with
the exact original data/codeword and one-based corrected position; two distinct
flips return EC_DETECTED without publishing. Ordinary success has position0.
Input high bits outside the selected word width or invalid n/null output return
EC_INVALID. Every refused output is unchanged; the output object must be live.
No dynamic allocation, callback, partial I/O or recovery resource is involved.

`ec_erasures` receives a mask identifying unreliable positions. Erased bit values
are ignored. Admit errors0or1 and2*errors+popcount(mask)<4. Search candidate
codewords agreeing within that known-bit error budget. Publish only one candidate;
no candidate means EC_DETECTED, multiple means EC_AMBIGUOUS, output unchanged.
With the correct codebook and admitted bound, multiple candidates cannot occur:
prove this with minimum distance. EC_OK means no erasure and no repair; otherwise
success is EC_CORRECTED and position0 because this API reports no unique location.
The caller must justify error/erasure bounds; the decoder cannot certify them.

Interleave exactly four valid-width words into4*n bytes, each byte0or1. Wire
offset4*bit+lane holds that lane's bit. Deinterleave validates every input byte
before publishing four words. Input/output extents are live and disjoint. Output
storage has capacity64bytes/four words; the unused tail for n8 remains unchanged.
Invalid width/byte values refuse unchanged. This is an original frame mapping,
not a physical coding/modulation/framing standard.

## Derivation, limits and adversarial behavior

Derive parity masks and the single-flip syndrome rather than memorizing a table.
Show injectivity, linearity and minimum distance4. From the distance argument
derive single correction/double detection and the2t+s<4 unique-recovery bound.
Explain why adding overall parity separates the single and double cases; flipping
two bits is not two successive single-error decodes.

Without interleaving, four consecutive transmitted bits can affect one word.
With the declared four-lane mapping, every contiguous burst of length<=4 touches
each lane at most once, including column boundaries. Verify every start/length
in both frame sizes. Larger bursts or separate bursts may hit a lane twice;
interleaving adds buffering/delay and requires preserved frame alignment. It
does not eliminate errors, repair lost insertions/deletions, or model a radio.

Security target: only these original local arrays, synthetic payloads, no device
or network target. Attacker may flip or replace all codeword bits, knows the
encoding, and has no secret to recover. Demonstrate harmless success by encoding
unauthorized payload9 and observing a valid decoding. Also show zero word XOR7
miscorrects to a different payload, and XOR135 is already another valid word.
These are property counterexamples, not arbitrary execution. Repair an overstated
claim by defining error correction separately from message authority. Application
range checks can reject9 under an explicit zero-only policy but cannot stop a
different authorized-range forgery. Authentication, identity and replay policy
require a separately specified trusted mechanism; adding parity cannot provide
them. Regression tests retain both valid encodings and the above counterexamples.

## Costs and deliverables

D1: hand-derived parity equations, distance and recovery proofs, exact five API
contracts, ownership/admission/state-publication argument. D2: encoder, syndrome
decoder, bounded error/erasure search and interleaver from permitted primitives.
D3: exhaustive0/1/2-bit flips for all16 and2048 payloads, unchanged refusal outputs,
independent small nearest-codeword oracle, changed-code codebook comparison,
compiled mutations, sanitizers and strict target dependency audit. D4: changed
16/11 profile plus erasure/burst/attacker and delayed transfer dossier.

Report O(n) encoding/decoding, O(2^k*n) enumerative erasure search and O(4n)
interleave work; all storage bounds are fixed. Distinguish code rate4/8 versus
11/16, parity redundancy, per-frame latency and decoder work. Do not rank the
profiles on an unstated noise/error distribution. Benchmarks must compare equal
error guarantees and separate encoding, decoding and channel simulation costs.

## Verification and assessment

The public CMake harness compiles unfinished stubs; all four selected tests must
fail until implemented. The assessor runner uses six GCC/Clang C17 debug,
optimized and sanitizer configurations. It checks592 plus280576 bounded flip
cases per configuration, all small received words against a separately searched
parity-equation codebook and all2048 changed-profile codewords. The erasure suite
checks every three-erasure location on zero and every payload with a rotating
one-erasure/one-error case; the proof transfers these finite observations, not a
false assertion that every erasure/input combination was executed. Burst tests
cover every admitted contiguous start/length in each selected frame.

Six compiled mutations omit overall parity, incorrectly repair double errors,
enlarge the erasure acceptance radius, ignore erasure locations, change the
interleave layout, and discard the phase. Ordinary code, each compiling mutant
and the compiling learner stub have separate expected verdicts. Cortex-M4 GCC
and Clang O0/O3 object/link maps and undefined symbols are audited; no board or
target execution, channel error rate or physical fault injection is implied.

R1 representation/derivation; R2 bounded decoder/publication; R3 erasures and
independent oracle; R4 burst composition/costs; R5 higher-error/attacker limits.
For each row:0 violates its contract;1 reproduces ordinary results but lacks the
specific mechanism argument;2 derives, implements and falsifies that mechanism;
3 independently transfers it under the delayed change. Passing tests is author
evidence, not learner assessment or absolute mastery.

Hints: identify powers-of-two positions; write each position's binary index;
separate total parity from syndrome; compare codeword distances before proposing
a recovery policy. Assessor implementation and solutions are published separately.

V1: change to16/11 while preserving wire semantics, then combine one known
erasure with one unknown error and justify the bound. V2 after>=7days: rotate the
four-lane permutation each frame, rederive the burst bound and diagnose loss of
frame phase; provide a counterexample to treating unknown phase as harmless.
Implement `ec_interleave_phase`/`ec_deinterleave_phase`: wire lane is
(logical_lane+phase)%4, phase0..3; phase>=4 refuses unchanged. The ordinary calls
are phase0. The public campaign exercises every four-bit burst/start at every
phase and demonstrates valid decoding under a wrong phase with wrong ownership.
Source support and exact validation status are registered separately.
