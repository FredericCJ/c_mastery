# Required incremental and owned UTF-8: X-T67-004

This successor supplies T67-04-INVESTIGATION and T67-04-TRANSFER support,
including X-T67-004-DELAYED. Complete the original ad_text constructor and
X-T67-001 owning-sum successor before composition; retain bytes.cursor and
bytes.checked reasoning from the named prerequisite artifacts. Build your
actual adt.c and owning_sum.c with this module. The assessor campaign imports
those real providers. No Unicode validation, streaming-codec or ownership
library implements the C mechanism. Python/Rust libraries are host comparisons.

Use hosted ISO C17 with CHAR_BIT==8, uint32_t and the repository GCC/Clang
debug/release/ASan+UBSan profiles. The stream core is fixed storage; the owner
adapter permits the declared clone/drop backend, exercised with actual malloc
and free. Neither host build nor sanitizer execution is strict target linkage
or physical evidence. Read the registered RFC3629 sections3/4 and Rust checked
conversion passages; published C17/C23 text remains inaccessible.

## Exact stream state and operations

AduStream stores at most three copied pending octets, their count, accepted-byte
and completed-scalar counts, and OPEN/FINISHED/CANCELLED phase. It never retains
a pointer into a caller chunk. adu_init resets any live stream to empty OPEN;
it owns no resource or outstanding borrow. All other calls require an initialized,
live, exclusively accessed stream; public fields must not be forged. Control,
input and summary storage are disjoint. NULL input is admitted only for size0.

adu_feed accepts a complete chunk transactionally. Success adds its byte count
and the number of newly completed scalars; a valid incomplete final sequence
remains copied in pending. The total accepted-byte limit is65535, including
pending bytes. ADU_INVALID rejects null stream or nonempty null input; ADU_STATE
rejects non-OPEN phase; ADU_RANGE rejects a chunk exceeding the remaining limit.
ADU_UTF8 rejects malformed bytes or a pending prefix that cannot be extended
to a valid scalar. Every refusal leaves all stream fields unchanged. No prefix
of a refused chunk is consumed. The caller may retry a corrected chunk, including
the prior valid pending prefix. Failure is not an automatic reset or error latch.

Example: feed E2 82, mutate/free the caller chunk, then feed AC. The stream still
completes one scalar from three bytes. If a later chunk41 FF is refused, neither
its41 nor FF is committed. Retrying41 42 adds exactly two bytes/two scalars.
An incomplete lead by itself can succeed; ED A0 is immediately refused because
no following byte could make that prefix valid. This early refusal is an explicit
policy, distinct from a codec that delays the same error until a third byte.

adu_finish requires no pending bytes. On success it publishes{bytes,scalars} and
sets FINISHED; repeated finish is idempotent. A pending prefix returns ADU_UTF8
without changing stream or output. Cancelled finish returns ADU_STATE. Null
stream/output returns ADU_INVALID. A summary otherwise names a live writable
object, disjoint from control. There is no byte emission or user callback.

adu_cancel clears all pending bytes and both counts, sets CANCELLED, and is
idempotent. It also admits an already FINISHED stream. Feed/finish then refuse
until explicit init. NULL cancellation returns ADU_INVALID. Cancellation here
is a synchronous local state transition; it has no asynchronous quiescence or
external I/O meaning. At the length boundary a pending lead may consume the
last byte of capacity and make completion impossible; finalization refuses and
cancellation remains available. A limit refusal never silently truncates text.

## Copied-owner composition and lifetime

AduOwner composes the actual AdoValue TEXT/EMPTY owner and a scalar count.
Initialize only unowned owner storage; do not duplicate owners by assignment.
adu_owner_set validates the entire input through actual ad_text before invoking
the declared clone callback through actual ado_text. Invalid encoding returns
ADU_UTF8 without allocation. More than65535 bytes returns ADU_RANGE; invalid
pointer/clone/drop admission returns ADU_INVALID. A clone failure returns
ADU_NOMEM and leaves the old owner, scalar count and old borrowed views valid.
The inherited clone contract cleans any partial allocation before failure.

Successful replacement independently copies the bytes before dropping the old
owner and publishes the new scalar count. Input may borrow the old owned bytes;
input must remain unchanged during the call. Clone/drop callbacks must obey the
owning-sum contract, not reenter or mutate any part of this wrapper, not retain
temporary input borrows, and keep their context alive through final destruction.
An empty successful TEXT still owns its declared drop obligation. The supplied
callback represents its zero-byte payload with NULL and drops it exactly once.

adu_owner_view returns a borrowed immutable ADText. An initialized/dropped EMPTY
owner yields NULL/0/0. A successful replacement or drop invalidates every prior
borrow, so never read or even compare a freed pointer afterward. Owner drop is
idempotent. The owner is a copying owner, not a hidden dynamic borrow checker:
its public fields and const-qualified view can be subverted by a violating caller.
External original-buffer mutation cannot change the independent copy, but direct
mutation of owner storage breaks the validated-owner precondition.

## Required reasoning and experimental artifacts

D1 derives the accepted scalar language and separates byte validity, scalar
count, normalization, grapheme boundaries and application authorization. D2
states the pending-prefix invariant, transaction boundary, finite counts and
borrow/owner lifetimes. D3 supplies actual original+successor implementations,
independent comparison, all split/finalize/cancel boundaries and normal semantic
mutant rejections. D4 combines corrected chunks and copying ownership under
input mutation, failed cloning and delayed reuse; retain predictions before run.

Use the section4 byte grammar to recognize possible prefixes and the actual
ad_text provider to validate completed multibyte sequences. A four-byte sequence
needs only three retained bytes: its final byte arrives as an input value and
can be combined into temporary four-byte storage. Explain why staging a stream
copy gives whole-chunk rollback. A counter of complete scalars must not count
an incomplete lead or report byte length as scalar length.

The Python oracle enumerates every empty/one/two-byte string, all its two-chunk
split positions, every partition of named valid/invalid three/four-byte boundary
prefixes, all second-byte values after E0/ED/F0/F4, and seeded short strings:
233265 cases. It uses strict complete/incremental codecs and a separately
generated prefix set from encoding all1112064 Unicode scalars. The17652 possible
prefixes include empty. That extra set matters: an actual pilot found Python
retaining ED A0 although this API refuses it immediately. Retain that distinction;
do not label a different error-timing policy a C decoder defect. Each row checks
the original complete constructor, every feed status, final phase/count/pending
bytes and output sentinels, not merely eventual success.

Four anchors separate chunk rollback, finalization/cancellation, copied ownership
and the retained constructor. Eight compiled defects discard pending input,
accept incomplete finalization, preserve pending bytes on cancellation, publish
a valid prefix of a rejected chunk, use byte count as scalar count, skip owner
validation, admit original overlong forms or admit original surrogates. Every
control must compile and fail its normal ADO_CHECK assertion; crashes and
sanitizer reports are not intended semantic outcomes. Learner stubs must compile
and fail all four anchors. Fault injection covers allocation failure before and
after temporary allocation, with exact live/drop accounting.

The safe C alias experiment mutates a still-live original buffer and revalidates
it; no dangling pointer is dereferenced. The Rust comparison actually compiles
and runs last-use mutation and an owned-copy control, and requires E0506 when
mutation precedes the last use of an immutable str borrow. The available Rust1.93
compiler differs from the rolling documentation version; record both. It does
not enforce C ownership and its String allocator does not implement this C
callback failure protocol. No unsafe Rust bypass is presented as safe borrowing.

## Assessment, hints and changed conditions

Apply R1 to encoding-versus-lifetime claims; R2 to complete prefix, count and
alias reasoning; R3 to independent original+successor falsification; R4 to the
implemented stream/cancel/owner transfer. Level1 is public-example completion;
level2 requires the full named arguments, exact failure effects and tests;
level3 requires an unaided changed chunking/ownership attempt after the original
seven-day delay. Record assistance. Neither exhaustive small strings nor this
independent codec comparison establishes absolute mastery.

Hints: first separate complete scalars from valid incomplete prefixes; then
stage the entire chunk's state; finally copy before releasing old ownership.
The separate private ANALYSIS.md gives the proof and assessor changed-condition
case. A meaningful variation changes whole-chunk rollback to consumed-prefix
reporting: specify consumed input and pending ownership before coding. Another
changes the input producer to reuse a receive buffer immediately after each
successful feed. Test cancellation with one/two/three pending bytes and explicit
restart; do not reuse a FINISHED/CANCELLED control as though it remained OPEN.

## Costs, containment and validation limits

Thirteen dimensions: stream metadata and three retained octets are fixed-size;
owner reachability adds one payload and callback context; stream lifetime has no
input borrow; owner views borrow until successful replace/drop; clone allocates
O(n) bytes and drop follows its backend; staged stream copies are O(1) while text
copies O(n); dispatch is direct except clone/drop; shared controls need external
exclusion; refusals preserve prior state/owner; actual compiler/object work is
retained; stack/binary observations depend on profile; ISO types/runtime checks/
caller discipline enforce different claims; integration imports actual ad_text
and owning_sum. Feed is O(chunk length), finish/cancel O(1), owner validation+
copy O(n). Retry rescans a refused chunk; no progress is silently promised.
Record seven10000-stream samples, each eight one-byte feeds, checksum30000,
sizes/stack/helpers and optimization. No physical throughput or universal
zero-overhead conclusion follows from native host samples.

Security target/environment: the original local decoder and owner with synthetic
octets, no external input service or real credentials. An adversary controls
chunk bytes and split positions, not native pointer validity. Defects can admit
overlong/surrogate encodings, publish a rejected prefix or retain cancelled data.
Expose them with C0 80, ED A0 80,41 FF and a cancelled three-byte prefix; harmless
success is an incorrect status/count/state or invalid owned copy. Such a result
is not arbitrary code execution or demonstrated privilege escalation. Repair
strict language admission, transactional publication and explicit ownership;
rerun matching regressions and independent comparisons. Residual limitations
include higher-level text policies, direct caller invariant violations and
asynchronous cancellation. Exact execution lives in matching source-bound
reports; physical execution, non-author review and learner assessment remain
unobserved. Published-edition access gaps remain open.
