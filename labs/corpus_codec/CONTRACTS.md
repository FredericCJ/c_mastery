# Bounded compression contract

This required T15.15 refinement teaches two distinct mechanisms and their
composition. RLE is a useful predecessor but supplies neither a prefix-code tree
nor a backwards match dictionary. Complete the public stubs; the private
reference lives outside the learner package. All component functions use C17,
8-bit bytes and optional exact-width uint8_t/uint16_t/uint32_t types. The hosted
driver may use stdio. The component uses no allocation, libc byte operations,
compression library or implicit runtime helper. No operating system or hardware
compression facility implements the assessed mechanism.

## Representation and contracts

The entire raw message is at most128 bytes. Each Huffman input symbol is an
integer0..15, with at most512 symbols. The codebook has16 lengths0..15 and16
unsigned codewords. A zero length means absent. Build lengths by repeatedly
joining two least-weight roots; initial leaf numbers are symbol values, internal
nodes follow in creation order, and equal weights choose the lower node number.
Each nonzero uint16_t frequency is admitted; total weight fits uint32_t. Empty
frequencies produce all-zero lengths. A singleton receives one bit, an explicit
format convention rather than the unrestricted zero-bit singleton optimum.

Derive canonical codewords from lengths: sort by (length,symbol), left-shift the
next code when length grows, then increment. Reject lengths above15 and
oversubscribed prefix space. Incomplete trees are permitted; a bit sequence with
no leaf is an error. Zero-weight leaves receive no code. `cc_book` must publish
nothing if validation fails. Derive the equivalent prefix-slot recurrence and
explain why a length-only decoder cannot know whether the encoder used optimal
frequencies.

The original Huffman frame is two little-endian octets for decoded symbol count,
then16 length octets, then codewords packed most-significant bit first within
each octet. The last unused low bits are zero. Decode exactly the announced
count, reject excess whole octets, nonzero padding, truncated codes and missing
leaves. An empty frame has an18-byte header and no payload. This grammar is an
original teaching format. RFC1951's DEFLATE format has different headers, byte
packing and length/distance alphabets; this exercise does not claim compliance.

The LZ grammar has no header: literal is `[0,value]`; match is `[1,length,distance]`,
where length3..16 and backwards distance1..32. A match cannot precede the produced
prefix. Its copy proceeds forwards one byte at a time, so length may exceed
distance. Literal x, literal y, match(5,2) decodes to xyxyxyx. A snapshot copy
does not implement this recurrence. Reject other tags, truncated tokens and any
decoded result above128 bytes. Empty token input is the empty message.

Greedy encoding chooses the longest legal match, breaking ties by the smallest
distance, and emits a literal if no length>=3 exists. It may inspect the complete
immutable input, including bytes that a decoder generates by overlap. At most
256 token bytes are needed: a literal costs2 bytes per input byte, a match costs3
for at least3. Token buffers and codebooks are caller-owned; borrowed input is
never retained. Span objects, counters and output must be live and disjoint.
Only a zero-length span admits NULL. Aliasing and lifetime preconditions cannot
be established by comparing arbitrary pointers in ISO C.

Composition splits each token byte into high nibble then low nibble and Huffman
encodes that sequence. Expansion reverses those stages, rejects an odd nibble
count, and validates LZ before publishing. There is no compression marker or
fallback storage format. Incompressible input may grow, including the18-byte
header. No finite lossless encoding can shorten every input.

Status CC_OK publishes output and `used`. CC_INVALID means an input/domain/format
violation; CC_SPACE means insufficient output capacity. Every failure preserves
the complete output span and counter. Internal bounded scratch permits that
transactional guarantee. Admission priority is specified by the functions:
domain/header checks precede capacity; hdecode checks count capacity before
payload validity; lzdecode validates the whole bounded stream before capacity.
Callers must not treat a capacity error as validation of attacker-controlled bytes.

## Derivations, failure and costs

Prove the greedy Huffman exchange step: among positive-weight symbols an optimal
binary prefix tree has two lowest weights as deepest siblings after exchanges;
contracting them reduces cost by their combined weight. Apply induction for the
unrestricted16-symbol tree, whose depth is at most15. The proof says nothing
about a separately imposed smaller maximum depth; clipping lengths is invalid.

Derive O(A^2) minimum scans for A<=16, O(N*W*L) match search with W<=32,L<=16,
and O(S*15*A) simple codeword decoding. Compute actual worst-case scratch and
call-stack sums from compiled frame reports: summing isolated function maxima
without their call graph can overstate or understate simultaneous stack use.
Record object/text bytes and imported symbols. Timing compares greedy and
optimal token parsers only under the same accepted inputs and output contract;
their output byte counts can differ. Host timing is not board latency.

Attack inputs are synthetic files passed only to this original local component.
Capabilities include every header, tag and payload octet. The properties are
bounded writes/work, failure atomicity and honest validation status. A repeated
distance1 stream requests expansion beyond128 bytes: the harmless success
condition for the repaired decoder is CC_INVALID with sentinel output intact.
An intentionally faulty control accepting padding or oversubscribed books
demonstrates a validation defect, not arbitrary code execution. Altering a valid
literal and recompressing is undetectable: compression provides no authenticity.

## Required transfer

After at least seven days, implement `cc_lzoptimal` and `cc_compress_optimal` from
the unchanged interface. The new goal is minimum LZ token bytes, not longest
current match. Each input position is a vertex; literal and match edges advance
by their decoded lengths and cost2 or3. Derive a shortest-path recurrence, choose
literal on equal cost and otherwise the first improving match in ascending
distance/length order. The supplied independent oracle uses forward relaxation;
a backward DP implementation must reach equal minimal cost. Exhibit a concrete
input where greedy loses. Then show why minimizing token bytes does not prove
minimum final Huffman frame bytes: token histograms and tree-description overhead
change the objective. Preserve a measured counterexample or explicitly report
the explored bound if none is found; do not assert an unobserved result.

Complete the separate assessor interview on changed resource limits: propose a
length-limited prefix-code algorithm or reject the smaller limit with a precise
status before publication. Merely truncating codewords is a failing answer.
This latter item is analysis of an altered contract, not a claim that a
length-limited encoder has been implemented by these APIs.

## Verification and evidence

Public C tests check exact empty/singleton/all-symbol cases, every output capacity
for a concrete128-byte message, failure preservation, overlap and malformed
codebooks. Private verification uses exhaustive Kraft-feasible length vectors
for small positive alphabets, independently constructed bitstrings and forward
shortest paths. Enumerate binary messages through length10, then high-entropy,
all-equal and periodic128-byte vectors. Check every cut of selected valid frames,
trailing bytes, invalid tags/distances and bounded expansion requests.

Four compiling controls omit the prefix-space check, accept nonzero padding,
corrupt overlapped copies, and disable optimal-parser matches. Each must fail a
specific CC_CHECK without a sanitizer crash. A compiling learner stub must fail
the public contract. The normal candidate must pass GCC/Clang debug, optimized
and ASan/UBSan profiles. Audit the actual freestanding object/image separately.
Export only this public scaffold and fixtures, then test it against an explicit
external provider. Author execution, independent delayed work and physical
observations are separate evidence dimensions.
