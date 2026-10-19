# Source authority, locale state and strict interchange bytes

## Exact scope and prerequisites

This required packet supplies `AX-PR-T05-01-T05.01.a/b/c` and
`AX-PR-T05-01-T05.03.a/b/c`, including each original `-DELAYED` task.
Families A, B, C, D, F, I and J apply differently: authority classification is
assessed by the source ledger and analysis; decoder/state contracts additionally
require implementation. Prerequisite artifacts are the learner's PR-T05-01
authority notebook, T02 pointer/type verdicts, T03 live-span bounds argument
and the T05.02 separate-compilation evidence. The original fixture and all
parent criteria remain authoritative; do not substitute a heading for a task.

Use hosted ISO C17 with eight-bit bytes and actual libc conversion facilities.
Only separate named probes use C23 or GNU17. This lab permits mbrtowc to study
its contract; the strict UTF-8 decoder must be built from integer/octet
operations, without a decoder library. It intentionally precedes the later
T67 owning Unicode abstraction and introduces no reverse prerequisite edge.
Python codecs and Unicode normalization are host-side independent oracles,
not target dependencies. Run only original local programs and synthetic text.

## Authority records and changed evidence

For each identity in `authority_cases.json`, record URL, revision/edition,
retrieval date, actual passage, wording class, exact claim, missing evidence
and the relevant compiler/header/runtime observation. The inspected sources
are bounded as follows; all wider readings remain unclaimed.

|Source|Authority and inspected portion|Claim limit|
|---|---|---|
|N1570|C11 committee draft;7.18p1-4 and7.29.6.3.2p1-4|Macro and conversion wording; not published C17|
|N3096|April2023 working draft;6.4.1p1-3,6.7.1p2-6,notes/footnotes and examples throughp20|Draft keyword/constexpr wording; not published C23|
|DR400 original and converted issue0400|Historical report/proposed wording versus current issue status fixed in C17|Issue history; not the entire published corrective edition or current libc behavior|
|GCC15.2 manual,cleanup subsection|Implementation contract for automatic scope cleanup|A compiler extension, not an ISO C ownership guarantee|
|ISO9899:2024 catalog,identity/general information|Edition5,publication2024-10 and catalog status|Publication metadata, not inspection of the758-page standard|
|WG14N325,Introduction andclauses1-4.4|Scanned1994 revised addendum draft for DAM balloting|Actual amendment-draft header/spelling passage, not the published1995 amendment|

For T05.01.a, explain what each row can establish and refuse conclusions that
cross its authority boundary. Its delayed fixture supplies a **synthetic**
issue status change. Implement `ledger.append_revision(history,incoming)` as
an append-only result with exact consecutive revisions starting1, statuses
open/resolved and nonempty applicability/source strings. It returns a deep
copy; never mutate the previous report or alias either caller input. Reject
bad shape, extra fields, bool-as-revision, stale/skipped revision or missing
source with ValueError before publication. Memory exhaustion may raise
MemoryError without changing inputs. Cost is O(history size) time/storage.
Retain hashes of the before/after artifacts. Actual DR400 historical summary
and current fixed-in-C17 metadata are separately observed; a synthetic event
must not be represented as a WG14 decision.

For T05.01.b, classify the actual constexpr constraint, footnote, note and
example separately. N3096p5 constrains qualifiers, initialization and exact
representability; p6 adds initializer restrictions. Derive why volatile int,
unsigned initialized by -1 and a missing initializer are refused. Positive
42LL-to-int and -1U-to-unsigned cases keep the rejection cause specific.
The delayed apparent contradiction is `constexpr int K=47` rejected in C17:
check document revision, mode and supported capability before alleging a
contradiction. Run the same declared C23 mode with a positive enum-extent use.

For T05.01.c, record preprocessing definitions, translation and actual execution
of bool with/without stdbool.h in C17 and declared C23 mode. Record the selected
implementation's macro/nonmacro result; an absent macro alone does not prove
keyword status. GNU17 cleanup separately records normal and early return order
21. Correct prototype, automatic storage and normally returning callbacks are
premises. No longjmp, cancellation, exception or ownership-safety claim follows.

## Amendment and locale tasks

For T05.03.a, make a facility/context table for iso646, wchar and wctype.
The actual C17 program uses `and`, `not`, wchar_t, mbrtowc and iswalpha; these
probes establish only the selected configuration. For its delayed task,
inspect N325clause4.2 and4.4: revise the previously uninspected amendment-draft
claim with its exact identity and passage. The public WG14 scan was rendered
because text extraction was empty. The published amendment and all unread
clauses remain gaps; neither a contents entry nor current header establishes
historical final wording. Do not erase the old reference-gap record.

For T05.03.b, compare ASCII A and C3 A9 in locale C and installed C.UTF-8.
Use explicit independent state objects, record actual locale names, wchar
width, byte counts, initial-state checks, errors and incomplete results.
Unavailable locale is a missing experiment, never an equivalent pass.
The delayed experiment interleaves C3 then A9 in one state and E2 82 then AC
in another; each continuation consumes only its own remaining bytes.

`LcLocale` owns its mbstate_t and poison flag. Initialize exclusive live control
with `lc_locale_init`; keep process locale fixed until all such states are
reset. No concurrent setlocale or shared-state access is admitted. The actual
mbrtowc call is synchronous and retains no source pointer. `lc_locale_one`
requires nonnull state, data, wchar output and size output with disjoint live
storage. On complete conversion it publishes the wchar and **raw mbrtowc
return count**, including zero for the null character. Zero is not a generic
byte-consumption count. Incomplete status retains conversion progress but
preserves both outputs. Encoding error poisons the control because the library
state is unspecified; further calls return POISONED until explicit reset.
Invalid admission preserves state/outputs. This wrapper cannot make locale
conversion equivalent to strict UTF-8 or make wchar_t universally Unicode.

## Strict octet stream and changed Unicode assumptions

`LcUtf` has one partial scalar, its minimum, remaining continuation count and
accepted byte/scalar counters. Initialize before use, keep control exclusive
and unmodified except through these operations, and keep control/data/output
storage disjoint. No borrowed input survives a call. The total stream limit
is64 octets; `NULL,0` is valid. A nonzero null span is INVALID; too many bytes
is RANGE. `lc_utf_feed` stages the whole chunk: malformed input or bounds
refusal preserves all fields, including previously accepted partial progress.
Valid incomplete prefixes may be retained; prefixes with no legal completion
are rejected immediately. `lc_utf_finish` publishes a scalar count only for a
complete stream; INCOMPLETE preserves the output. Repeated finish is allowed,
and does not prevent further feed. Reset deliberately discards pending state.

Implement the RFC3629 section4 octet grammar with checked finite arithmetic.
Count null as a scalar. Reject overlong encodings, surrogate values, values
above10FFFF and invalid continuations. Derive prefix feasibility and why the
accumulator and shifts fit uint32_t. Feed costs O(chunk length), storage O(1).
For T05.03.c the exact original sequences41,C0 80,ED A0 80,F4 90 80 80 must
yield valid/invalid/invalid/invalid independently of the process locale.
The delayed C3 A9 versus65 CC81 comparison yields one versus two scalars;
the independent host oracle confirms canonical equivalence after NFC.
Scalar validity/count does not implement normalization or grapheme segmentation.
No full normalizer or universal displayed-character count is claimed.

## Deliverables, verification and assessment

D1 is the exact authority table, wording classification and predicted mode,
locale and grammar outcomes. D2 is the append-only ledger, strict stream and
conversion-state wrapper, with contracts and complexity arguments. D3 is the
actual compiler/header/runtime matrix, intended diagnostics, independent
oracle, source/tool identities and unchanged-failure evidence. D4 contains
each named changed-condition task, repeated after at least seven days without
assessor solutions. Keep the six authority and six locale/Unicode IDs separate.

Compile the public unfinished context.c with its adjacent CMake file. All four
anchors compile and must fail semantically until implemented: strict,partial,
locale,combining. The four assessor mutations omit scalar constraints, discard
partial UTF state, discard partial locale state, or confuse bytes with scalars.
Reject them for LP_CHECK assertions after successful build, not sanitizer or
compile failures. GCC/Clang debug/release/address+undefined-sanitizer profiles
run the actual locale behavior and213637 independent byte/split cases. The
oracle covers all0/1/2-byte strings and splits plus longer scalar boundaries,
truncations and malformed/composed examples; it is not exhaustive over64bytes.
Record native sizes, stack and object dependencies separately from libc costs.

The companion runs bool modes with macro dumps, two valid constexpr cases,
three intended constraint failures, the wrong-mode failure and GNU cleanup at
O0/O3 with both compilers. Preserve rejected logs. Run ledger reference and
unfinished learner controls; reject six malformed updates while preserving
the initial history. A compiler's C23 flag is not whole-C23 conformance.

Score each exact ID:0 for wrong authority, type, state or output;1 for copying
fixture output without its premises;2 for the complete ordinary task plus
independent falsification and intended evidence;3 for an independently derived
changed-condition solution with honest limits. Source-ledger interpretation
requires assessor review beyond the executable schema tests. Hints: identify
the authority before drawing a rule; split conversion state by stream; treat
error state as unusable; derive legal scalar intervals before counting bytes.

## References and validation limits

Use the exact source records in the accompanying research register. Primary
URLs: [N1570](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf),
[N3096](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf),
[N325](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n325.pdf),
[DR400](https://www.open-std.org/jtc1/sc22/wg14/www/docs/dr_400.htm),
[issue0400](https://www.open-std.org/jtc1/sc22/wg14/issues/c11c17/issue0400.html),
[GCC15.2](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Common-Variable-Attributes.html),
[ISO catalog](https://www.iso.org/standard/82075.html),
[RFC3629](https://www.rfc-editor.org/rfc/rfc3629),
[Unicode normalization section1.2](https://www.unicode.org/reports/tr15/#Norm_Forms).
Only named passages were read. Raw primary bytes are pinned where download
succeeded; the ISO catalog was read through the web tool but direct download
returned403, and is not represented by an invented raw-file hash.

Source-bound reports distinguish authored, compiled and actually executed
support. Human-independent review, delayed learner assessment, other locales,
other wchar representations and physical execution remain unclaimed. The
historical amendment-draft gap is narrowed; exact published C17/C23 and the
published1995 amendment remain inaccessible. Finite passing tests do not prove
absolute mastery or correctness under invalid control corruption.
