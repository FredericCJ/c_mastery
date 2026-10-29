# Required T68.04 typed logging transfer

This contract supplements X-T68-004 R1-R4 and X-T68-004-DELAYED. Complete
X-T68-003 and retain its single-publication/effect-accounting argument. The
actual original `mp_variadic` remains a separately tested numeric producer;
it is not a logging implementation. Implement `src/logging.c` and
`src/logging_macros.h`. Public declarations, tests, and this contract are
trusted assessment inputs. The compiling stub deliberately lacks the typed
selector and record publication. Do not copy the private assessor sources.

## Representation, operations and ownership

Use ISO C17 with extensions disabled, hosted GCC/Clang and the declared
six-profile campaign. `struct mpl_sink` owns four inline `struct mpl_record` slots and a count
in 0..4. Initialize fresh storage to zero. There is no heap, operating-system
I/O, format-string interpreter, concurrent publication or global logger.
Records contain zero to four typed values: number (uint32) or binary bytes
of length 0..8, including embedded zero. Each record owns its copied bytes.
Source metadata is absent or a copied, terminated filename of 0..255 bytes
and a nonzero unsigned line. Empty filename is permitted; no path meaning is
inferred. The exact representation is in `include/logging.h`.

The sink is live, exclusively owned during calls, unmodified except through
these operations and remains in scope until its reader finishes. Inputs,
borrowed byte extents, filename and control storage are disjoint from the
sink. Each nonempty extent is readable for its declared length; a filename
is readable through its first terminator or through 256 bytes, whichever
comes first. Null bytes with length zero are valid. Invalid native pointers
are not recoverable inputs. The metadata pointer and fields are borrowed
only during the call; no pointer is retained in a record. All concurrent
access needs external serialization; copying bytes supplies no synchronization.

`mpl_number` and `mpl_bytes` create descriptors. The number constructor takes
uint32_t, so ordinary C implicit numeric conversion still applies: passing
3.5 is not rejected as a type error. `mpl_log1` through `mpl_log4` instead take
`struct mpl_field` parameters: passing a raw integer or a different structure violates
a function-call constraint and must be diagnosed. A forged field tag is
checked at runtime. These are different enforcement boundaries.
The exact literal may additionally trigger a compiler conversion warning;
with warnings treated as errors, that policy can reject otherwise permitted
C. The separate probe records both policies and a nonconstant numeric input.

`mpl_write(sink,site,fields,count)` validates control and argument shape first:
null sink, sink count above four, field count above four, or nonzero field
count with null fields returns INVALID. A full valid sink then returns FULL
without reading site or field contents. Otherwise, nonnull site requires
nonnull filename and nonzero line; a filename without a terminator in the
first 256 bytes returns RANGE. Fields are validated in order: unknown kind
is INVALID; byte length above eight is RANGE; nonempty null bytes is INVALID.
Unused number-descriptor members and byte-descriptor number are ignored.
The entire record is staged before assignment and count increments last.
Every refusal leaves all sink bytes unchanged. Successful zero-field logging
still appends one record. A `{NULL,0}` site passed to a typed wrapper means
absent metadata; other wrapper sites obey the nonnull-site validation above.

## Required macro derivation

Provide `MPL_LOG(sink,field1,...,field4)` and `MPL_LOG_AT` for exactly 1..4
fields, and distinct `MPL_LOG0(sink)` / `MPL_LOG_AT0(sink)` operations.
The metadata forms capture the caller's `__FILE__` and `__LINE__`; ordinary
forms attach none. Each sink and field expression is evaluated once. Their
relative evaluation order is unspecified; no left-to-right API promise is
made. In particular, do not supply unsequenced conflicting scalar operations.
Use distinct counters for separate argument expressions. Expressions may
have effects even when the eventual call returns FULL or INVALID; sink
rollback does not roll back those effects.

Derive the selector position for each supported arity, then inspect the
expanded tokens. Include a trailing sentinel so the helper's variadic tail
is not empty in C17. Only commas inside matching parentheses are protected;
braces alone do not protect the commas of a compound literal. The expression
`((struct mpl_field){MPL_NUMBER,19,NULL,0})` is one field, whereas the form without
the outer parentheses is a diagnostic specimen. Macro argument expansion
and rescan are preprocessing, not evaluation. A compound-literal site passed
through another macro likewise needs the outer parentheses.

Five fields must select a named arity-error declaration and fail translation.
An omitted/empty field in the nonempty form must fail; use the explicit zero
operation. No universal arbitrary-arity or empty-token detector is claimed.
Do not use GNU comma deletion or `__VA_OPT__` in the portable implementation.
The separate C2x/C++20 probes observe those compiler modes and N3033 proposal
behavior; they do not certify the inaccessible final C23/C17 publications.

## Concrete tasks and evidence

1. D1/R1: derive each 1..4 expansion, implement typed wrappers, and show exact
   once-only counters for all fields and the sink, including FULL. Preserve
   original seed7 arity0..4 sums 0,7,15,24,34 and modulo-wrap behavior.
2. D2/R2: classify omitted versus empty variadic tokens, protected comma and
   implicit numeric conversion. Supply preprocessor output, typed/arity
   diagnostic controls and separately executed C2x/C++20 comparisons.
3. D3/R3: implement complete staging/copy validation. Exercise full, invalid
   last field, maximum binary extent, null+zero, filename255/256, exact caller
   line, overwritten input storage and harmless literal `%n` data. Compare
   all independent oracle results; reject the compiled semantic mutants.
4. D4/R4: compose optional metadata and binary fields under the unchanged
   lifetime/publication rules. Run equal-record direct/macro measurements,
   compiler/stack/helper and clean/unchanged build observations. Explain
   precisely what changed from numeric dispatch and what C still cannot
   enforce. The delayed assessor variant uses metadata changes, binary
   partial states and refused calls with observable argument effects, not
   merely another numeric constant.

Pass requires each relevant state/output/effect predicate and source-bound
check, plus a written argument for every admitted input. Nominal successful
append alone is at most rubric level1. Level2 meets the exact contract and
diagnoses minimized counterexamples; level3 independently transfers it to
the held changed assumptions after the required delay. No author campaign
stands in for that learner assessment. Finite oracle cases cannot prove
all pointer validity or all macro token streams.

Hints: first expand a one-field invocation completely; then place the helper
sentinel. Next write an ordinary typed function with staging before wrapping
it. Finally list every expression evaluated before a refusal and distinguish
those effects from the sink bytes. Separate assessor reasoning is private.

## Thirteen cost and enforcement dimensions

| Dimension | Required evidence and interpretation |
|---|---|
| Representation | Four inline records, four inline eight-byte values/record, filename256; actual sizeof values in metrics; ABI padding is not wire format. |
| Operation/invariants | count<=4, count increments only after complete assignment; full refusal is O(1), admitted work O(filename bytes + fields + byte payload), bounded by this contract. |
| Ownership/lifetime | Input descriptors borrow; record copies own inline bytes; no destructor or allocator is needed. Caller validity/disjointness and external serialization are premises. |
| Failure | Shape/control before full, then site and ordered fields; no sink mutation on refusal, argument effects retained. |
| Enforcement | Struct prototype constraints get diagnostics; tag/length checked at runtime; lifetime and sequencing still programmer obligations; numeric constructors permit C conversions. |
| Dispatch | Preprocessing chooses a typed function; no runtime field-count dispatch in that macro. The common writer still validates runtime descriptors. |
| Allocation/traffic | No dynamic allocation; staged record plus final record copy and descriptor copies; quantify inline storage and generated memcpy/memset separately. |
| Stack | Stage and wrapper field-array storage, measured compiler stack reports at each profile; not a universal worst-case stack proof. |
| Code/text | O0/O3 function symbol sizes/disassembly plus preprocessing bytes/tokens; repeated macro expansion can increase caller text. |
| Timing | Seven repeated macro/direct samples each write the same four number values and observe checksum40000 over10000 resets; no metadata or I/O difference in this comparison. No universal speed ranking. |
| Build | Measured clean and unchanged Ninja builds plus strict diagnostic specimens; extensions are separate translation units. |
| Composition | Original numeric selector and new logger remain separate artifacts; optional metadata, borrowed binary fields and refusal effects have explicit adapters. |
| Portability/limits | C17 host profiles only; C2x/C++20 proposal comparisons are separate. No target timing, concurrency, published C17/C23 inspection, human independence or learner mastery is inferred. |

## Laboratory scope and status

Only original local teaching programs and synthetic bytes are used. There is
no external target or exploit claim. `%n` remains copied data because this
mechanism has no format interpreter; this is not a general sanitization claim.
Filename metadata can contain local paths, so the teaching sink stays local.
Actual validation is recorded in matching main/diagnostic reports. Independent
learner attempts, non-author assessment and exact-edition source gaps remain
separate from implemented author support.
