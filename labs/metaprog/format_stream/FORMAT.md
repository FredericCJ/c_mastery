# Required T68.08 grammar, atomic messages and partial writer progress

Apply original X-T68-008 R1-R4 and X-T68-008-DELAYED. Complete X-T68-007;
retain its semantic reasoning, but this allocation-free mechanism imports no
owner or fault-adapter ABI. Implement src/format_stream.c and the retained
src/metaprog.c formatting operations. The actual retained mp_format converts
decimal numbers; do not substitute printf, a format library or a parser library
for the studied mechanisms. CMake selects learner sources by default. Private
references are separate. Python is a host-side independent oracle.

Use hosted ISO C17, exact uint32, eight-bit octets and ASCII execution characters
(translation assertion). Neither native struct layout nor size is a wire ABI.
All objects are live, aligned, initialized and disjoint, with actual extents;
calls are externally serialized. Controls may not move during a call. A program
must be produced by successful mfp_parse and remain unmodified; its public
representation is not an unforgeable certificate. Message bytes up to length
are initialized. A stream begins fully zeroed. Inputs, outputs, sink backing and
callback-accessible storage are disjoint. These lifetime/alias premises are
caller discipline, not guarantees provided by _Generic.

## Grammar and typed arguments

The format is a byte extent of length0..32, not a zero-terminated string.
Ordinary bytes, including zero and percent, are literals. Two opening braces
encode one literal opening brace; two closing braces encode one closing brace.
The only placeholders are {u0}..{u3} for decimal uint32 and {s0}..{s3} for bounded
byte spans. Every other unmatched brace, type, index or incomplete placeholder
returns FORMAT. Reusing one index with different kinds returns TYPE. Parsing
validates left to right and publishes the entire program only on success;
refusal leaves output unchanged. Null output, length>32 or null nonempty input
returns INVALID. The program owns copied literal bytes and typed descriptors;
it retains no input pointer. Null+0 is admitted without null pointer arithmetic.

MFP_ARG selects a typed factory for uint32 or MfpSpan and evaluates its argument
once. It does not parse the grammar at translation time. Unsupported categories
require a diagnostic in the tested strict translations. Explicitly forged tags,
casts, invalid pointers or wrong extents defeat stronger static safety claims.
MfpArg is a fully initialized plain struct, not a union; kind1 is NUMBER and
kind2 BYTES. A byte span is0..16 bytes, null only at length0. Embedded zero is
content. All supplied arguments, including unused ones, have valid kinds and
valid byte spans; otherwise TYPE/INVALID respectively. Argument count is0..4.
Every required index must exist and match its placeholder kind or return TYPE.
Unused valid arguments are allowed. Scalar range follows exact uint32.

Compare the inspected C++ rolling draft basic_format_string constructor's
constant-expression compatibility requirement with actual C++20-mode compiler
experiments. A checked literal with incompatible argument type is rejected;
std::vformat with the same incompatible runtime grammar throws format_error.
Do not claim the rolling draft's newer dynamic-format-string overload existed
in the published C++20 text. The C factory supplies narrower category selection;
runtime mfp_parse/mfp_prepare implement the stated grammar and compatibility.

## Complete message and transactional sink

mfp_prepare first validates arguments, then stages at most128 owned bytes.
Literal descriptors contribute one byte; a numeric placeholder invokes the
actual retained mp_format; a span contributes its exact bytes. The source bound
permits at most8 placeholders, each at most16 bytes, so128 is sufficient for
parse-produced programs (literal substitutions only reduce that maximum).
Still check every append. Publish MfpMessage only after complete success;
required null controls or impossible descriptor bounds return INVALID without
publication. No allocator, callback or format interpreter is involved here.

mfp_render validates sink used<=capacity and backing (null only capacity0),
prepares the complete message, then checks capacity-used and appends it. FULL
preserves every sink byte and used. It does not publish a partial prefix.
On success only appended bytes and used change; there is no added terminator.
Example format `{{{u0}}}:{s1}:%n` and arguments42,span(A) yields exactly
`{42}:A:%n` (9bytes). {u0} with UINT32_MAX produces4294967295;9free bytes fail,
10succeed. {s0} with bytes00,25,6e emits exactly those three bytes.

The retained zero-terminated text operation has a different caller precondition:
input must really terminate in live storage, disjoint from output. Its bounded
scan does not validate arbitrary pointers or turn unterminated input into a
portable test. Test capacities0..11 and guard-filled suffixes. Decimal zero is
one byte, and UINT32_MAX requires10digits. No library conversion replaces the
division/remainder derivation.

## Partial writer state and irreversible effects

MfpStream owns a copied MfpMessage, offset, state, uncertainty flag and terminal
failure. start validates message length<=128 and known stream state, refuses
ACTIVE with BUSY unchanged, and otherwise copies a fresh message with offset0,
ACTIVE, uncertainty0. Thus it may reuse EMPTY or terminal storage; no resources
or callbacks are retained. Changing the original message after start has no
effect. Reusing a terminal stream begins a new logical output transaction; the
caller must decide whether that is safe for its external protocol.

step performs at most one synchronous writer call with the unaccepted suffix.
The writer returns an accepted prefix length and WRITE_OK, WRITE_AGAIN or
WRITE_ERROR. It consumes/copies the accepted prefix before returning, retains
no pointer, returns normally, and cannot reenter, destroy, move or mutate the
stream or participating objects. Its context remains live through the call;
null context is allowed if the writer supports it. Concurrent cancellation is
not admitted. A required null writer on nonempty ACTIVE input returns INVALID
unchanged. An empty active message becomes DONE without a callback.

- A valid accepted count is0..remaining. WRITE_OK requires positive progress.
- Advance offset by accepted even on WRITE_ERROR. ERROR always becomes FAILED
  with IO, including when it accepted the last byte. Its external prefix remains.
- AGAIN with a remaining suffix returns AGAIN; OK with a suffix returns MORE.
  Either non-error status accepting the entire suffix becomes DONE/OK.
- An oversized count, unknown status or OK with zero progress is a protocol
  violation. Become FAILED/PROTOCOL with uncertain1, preserving the last known
  offset. The external output may have changed; do not retry or claim rollback.
- DONE returns OK without a writer call; FAILED returns its stored failure;
  STOPPED returns CANCELLED. Terminal calls are idempotent.

cancel on ACTIVE becomes STOPPED/CANCELLED, preserving accepted offset. DONE
stays OK; FAILED retains its failure; STOPPED stays CANCELLED; EMPTY is INVALID.
Cancellation prevents later calls but cannot retract bytes already accepted.
It requires no callback in flight and gives no asynchronous quiescence guarantee.
For hello with accepted lengths2,2,1, offsets are2,4,5 and external output is
exactly hello. An ERROR after accepting2 retains offset2 and fails permanently.
Restarting from offset0 would duplicate the prefix; hiding that by relabeling
the streaming API transactional is a contract failure.

## Required deliverables and independent changed conditions

- D1/R1: derive grammar/argument distinction and both publication boundaries;
  classify every caller premise, runtime refusal and implementation diagnostic.
- D2/R2: prove digit/message bounds, prefix invariant and terminal behavior;
  compare actual C++ checked-literal and runtime-format behavior under the
  inspected source/version limits. Give counterexamples to lifetime/type myths.
- D3/R3: run the retained formatting anchor and all four successor anchors in
  six profiles, eight compiled semantic controls and four compiling stubs;
  compare independent parser/substitution/prefix models and every capacity0..11.
- D4/R4: implement this parsed DSL and copied partial stream; inject backpressure,
  error after progress, cancellation after a prefix and invalid callback reports.
  Execute the real local nonblocking pipe adapter, retaining call/offset evidence.
  Complete the thirteen cost rows and a separate unaided delayed assessment.

Tools: run_metaprog_format_stream.py, verify_metaprog_format_stream.py,
run_metaprog_format_investigation.py, check_metaprog_format_stream.py --self-test.
The oracle uses a regex-token grammar, Python decimal strings, concatenation
and immutable prefix states; it imports no C expected result. Preserve exact
commands, source/tool identity and first divergences. Controls must compile and
fail MPO_CHECK, with no sanitizer-based substitute for the intended assertion.
Public test success alone earns at most level1. Level2 requires each specific
R1-R4 obligation; level3 requires an independently attempted delayed case with
assistance recorded. No finite suite proves complete mastery.

Graduated hints: (1) stage parsed descriptors and complete bytes separately;
(2) distinguish accepted count from callback status; (3) a dishonest report
makes subsequent offset recovery unsafe. Private technical analysis and the
sealed changed schedule remain under assessor/.

## Costs and enforcement

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual program/message/stream sizes and initialized state encodings; no wire struct dumps. |
| Reachable storage | Copied32-byte source,32 descriptors,4 type requirements,128-byte message; no retained caller borrow. |
| Lifetime | Parse/start copies; callback borrow ends synchronously; terminal reuse starts a new transaction. |
| Mutation/aliasing | Live disjoint controls/backing; externally serialized callbacks; unchanged failure snapshots. |
| Allocation | None in the mechanism; C++ comparison, host oracle and OS pipe are separate host dependencies. |
| Movement/copying | Source/message/control copies are bounded; sink copies once after admission; stream offers remaining suffix. |
| Destruction | No resource destructor; terminal control discard is constant work, while external pipe cleanup closes both owned descriptors. |
| Dispatch | Direct typed factories and retained formatter; one indirect writer call per admitted nonempty step; inspect actual objects. |
| Concurrency | No thread-safe or async reclamation claim; cancellation only between synchronous calls. |
| Failure | Sink failure is unchanged; partial external output survives IO/cancel; dishonest callbacks poison recovery. |
| Compilation | Strict C category negatives with positive controls, actual C++20-mode literal/runtime comparison, clean/unchanged builds. |
| Binary/stack | Six-profile executable/object/stack logs, compiler copy helpers and repeated timing samples; no freestanding inference. |
| Boundary | ASCII grammar/raw bytes, uint32 decimal; POSIX pipe adapter is separate; no general Unicode/locale/printf compatibility. |

Parsing is O(format length); preparation O(descriptors+rendered bytes), all
bounded by32/128. Each step is O(1) adapter work plus the writer's declared work;
successful offsets monotonically increase at most128 but arbitrarily many
AGAIN-with-zero responses are allowed, so no unconditional termination/time
bound follows. Seven10000-prepare CPU-time samples use identical checked
contracts and zero separate warmup; preserve resolution/noise and operation
counts. Never omit validation to manufacture a faster comparison.

Security scope: only the supplied original local formatter, scripted writer
and owned anonymous pipe, with synthetic bytes. Attacker-controlled format or
argument tags can trigger runtime refusal; percent-n remains data, not a native
format operation. Invalid callback reports demonstrate a protocol violation,
not arbitrary code execution. Repair the invariant and rerun its named control.
There are no external services, credentials, privilege changes or board writes.
Actual source-bound author validation, independent human review, learner
assessment, exact published C17/C23 access and physical observations remain
distinct. No POSIX behavior is attributed to ISO C.
