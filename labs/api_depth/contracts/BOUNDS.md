# Required T18 contract construction: values, results and enforcement

This successor implements the ordinary and original changed tasks for
AX-PR-T18-01-T18.01.a/c, T18.02.a/b, T18.07.a/b/c and T18.09.a/b/c.
The .01b failed-state and .02c registry tasks continue in OWNERSHIP.md.
Families A/B/C/D/F/G/I/J apply. Preserve the original minimum7days for delayed
assessment. Prerequisites are T03 lifetime/arithmetic, the T12 library contracts,
the learner's bounded-view dossier, and accepted api_append/f_append artifacts.
Use C17, profileC for the component, hosted diagnostics for the tests. Implement
bounds.c against bounds.h; substitute learner versions of the two prerequisite
components after the author reference integration has passed. Assessor sources
are not learner artifacts. No library copies or pointer-order shortcuts are
allowed inside the component.

Start from valid C objects that violate domain facts. AcView admits length<=cap
and a nonnull data pointer whenever cap>0. NULL,0,0 is valid, while NULL,0,1 is
not. Admission inspects these scalar facts without dereferencing data. It does
not validate the lifetime, extent or concurrency of a nonnull pointer. The
caller must provide disjoint live output objects and actual readable/writable
extents on admitted calls. In particular, a fabricated pointer is not a useful
executable negative fixture. AcWait interprets every negative int as infinite,
zero as poll, and1..60000 as milliseconds. Larger values are INVALID. The output
is unchanged on rejection; conversion to unsigned follows classification.

The changed deadline has an explicit model: an8-bit modular millisecond counter,
a start sample0..255, a finite duration1..127, or a separate POLL/INFINITE tag
with duration0. Observation must occur within127 actual ticks of start. Compute
elapsed modulo256, reject residues128..255 and compare accepted elapsed with
duration. Justify why the external elapsed-time premise is necessary: an
observation256 ticks later can have the same counter value as start. No algorithm
using only these samples can detect that violation. Poll is immediately expired;
infinite is not; both retain the observation-window contract in this bounded
adapter. Derive the relation using unbounded absolute time, then enumerate
counter wrap and duration boundaries. Status rejection preserves the expired
output. This is an application counter contract, not an ISO clock definition.

For representation, report CHAR_BIT, sizeof(size_t), pointer size and alignment;
uint8_t is optional in portable C. This selected adapter statically requires
8-bit host bytes. Separately model a target storage unit containing16 bits using
uint_least16_t values. Convert each numerical unit into two little-endian octets,
using checked capacity in octets and no type-punning. Capacity check n<=cap/2
precedes writes; empty input performs no pointer arithmetic. If the least-width
type is wider than16 bits, reject values above65535 before publishing. The model
does not change host CHAR_BIT or constitute execution on a16-bit-byte machine.
Explain which counts become units, octets or C bytes under that changed target,
including why directly writing the model array's host representation is wrong.

Use the actual api_append baseline for the ordinary append: length<=cap<=32,
capacity checked by n<=cap-length after admission, failure leaves every logical
field/data element unchanged, and any source overlap with the whole buffer
object is forbidden. Its defined byte-address equality scan also rejects aliases
into metadata. Do not subtract or order unrelated pointers. The independent
fixture enumerates18,513 valid length/capacity/count combinations and predicts
the suffix from source indices, not by calling the implementation again.

The changed AcSlice supplies a live unique owner identity plus offset/count in
that owner's32-byte data array. Distinct owner objects must have disjoint storage;
same-owner nonempty input is rejected. Check offset<=32 and count<=32-offset
before making a pointer. Under that stronger caller contract, overlap admission
is O(1); the copy remains O(n). The baseline equality scan is O(n*sizeof buffer).
Do not claim constant-time admission while calling the baseline scan internally.
The separately versioned overlap-allowing operation imports actual f_append on
FBuffer, including its staging and generation-exhaustion rules. Test a source
inside the future destination suffix; copying forward without staging is not a
general proof for arbitrary overlap. Empty append preserves its generation.

Lookup outputs are a product of status and value, not a sentinel. Key0 finds
integer0 with a nullable pointer; key1 finds7 and a borrowed live pointer; key2
is missing; larger keys represent a backend failure. Status-plus-output preserves
the previous output on failure. Tagged form initializes an inert payload on
failure, whose fields must not be mistaken for a found value. Compare actual
object sizes and call-site obligations. A proposed zero sentinel removes one
integer value from the accepted domain; a null-pointer sentinel removes the
found-null case. That is a contract migration, not an equivalent encoding of
the original domain. State how an old caller would lose information.

Implement atomic ASCII copying and a separate progress operation. ASCII means
numerical octets0..127 here, not locale-dependent text. Atomic copy first checks
capacity, then every input octet, then copies and publishes length. Its two
versions either preserve written on error or set written=0 before further
validation; both preserve output data on failure. The progress form instead
returns status,consumed,produced. It validates the next octet before checking
remaining output capacity, commits a valid prefix and stops at the first invalid
octet or full destination. For [65,66,255,67] with cap4 expect INVALID,2,2 and
exactly [65,66] committed. For [65,66] with cap1 expect FULL,1,1. Successful empty
input has OK,0,0. All source/output spans must be disjoint. These two guarantees
cannot share a success/error-only result without losing partial progress.

For enforcement classify the octet layout assumption as compile-time, external
lengths as runtime admission, an internal length invariant as an assertion plus
declared fault policy, and finite observed cases as tests. ac_checked_add increments
its evaluation counter on every admitted call before validating state; callers
guarantee counter headroom. An existing value above capacity is INTERNAL; a valid
state with insufficient remaining capacity is FULL. Both preserve value. A
three-octet external packet carries current length,capacity,amount; the parser
rejects length>capacity or capacity>32 as INVALID before constructing trusted
state. The same relation moved across a trust boundary needs a different
enforcement site, not a disabled assertion.

Build NDEBUG both ways. Required arithmetic/validation must still execute once.
The instrumentation macro may intentionally evaluate its optional argument zero
times when disabled; test that separately from required evaluation. Run three
original service processes: recoverable capacity, malformed external request,
and impossible internal state. Only the last terminates that service with the
declared exit70; no host process or system is stopped. Preserve the first cause
instead of relabeling every failure a timeout.

Submit the contract/layer tables, source, independent expected-state derivations,
exact commands, compiled semantic failures and changed-condition arguments.
Twelve compiler/profile/instrumentation combinations must pass. The wrap,
sentinel and assertion-side-effect mutants must compile and fail their intended
checks; an unsolved scaffold must also fail. The Python absolute-time oracle
checks65,536 cases per compiler independently of modular subtraction. Explain
its bounded premise and why it cannot validate a caller that retains a deadline
for an entire counter cycle. Capacity/overlap/partial-input cases are original
synthetic programs only; no foreign memory or service is accessed.

Rubric:0 wrong domain or failure effects;1 supplied cases reproduced without
proof;2 correct implementations plus independent boundary/cost/lifetime arguments
for each named objective;3 independently handles its exact changed task after
the original delay. Hints: identify the first possible write; write units on
every count; distinguish the flag/tag from its payload; state the maximum age
before proving modular comparison. Current author receipts qualify these bounds,
not every possible caller, independent learner performance or physical targets.
