# T12.05–06: bytes, scalar values and conversion state

Required original PR-T12-02 criteria T12.05a–c and T12.06a–c, families A/B/C/D/F/G/I/J.
Prerequisites: byte spans, integer promotions, tagged results, ownership and
T12.04's distinction between a counted field and a NUL-terminated string.
Complete unicode.c from the original octet grammar. No codec library may implement
this component. Hosted C17 library calls are permitted only in the separately
identified comparison notebook. The core requires CHAR_BIT=8, uint32_t and uint16_t;
its retained prefix is at most three octets. C23 comparisons do not change this
declared C17 profile. Host Python is an independent oracle, never a target dependency.

Inspect R-LLIB-RFC3629 sections3–4/10 and R-LLIB-N1570-CONVERSION named passages.
The latter is a C11 draft, not the published C17 standard. The locale comparison
uses the explicitly separate POSIX.1-2024 contracts in R-LLIB-POSIX2024-LOCALE.

The encoding grammar admits ASCII; C2..DF followed by one continuation; E0..EF
followed by two; F0..F4 followed by three. The second byte restricts E0 to A0..BF,
ED to80..9F, F0 to90..BF and F4 to80..8F. Derive these restrictions from the
shortest representation, surrogate exclusion and maximum U+10FFFF. A generic
"10xxxxxx continuation" check alone admits forbidden encodings. Every emitted
value must be a Unicode scalar. U+0000 is an ordinary scalar in a counted span.

Initialize each lu_state to zero; only the API may subsequently mutate it. Input,
output, state and result are live, disjoint caller-owned objects. Input stays
stable throughout a call. The API borrows them synchronously, retains only copied
prefix octets, allocates nothing and does not share state between streams. ISO C
does not enforce these ownership/disjointness promises. A corrupted arbitrary
state is outside this API's admitted domain, not a validated serialized state.

`lu_decode` consumes the entire supplied chunk on OK or INCOMPLETE, emits all
completed scalars and retains only a potentially valid suffix. Its consumed count
belongs to this call, even when the scalar started earlier. On INVALID, FULL or
ARGUMENT, all caller output, metadata and state remain unchanged. FULL is distinct
from an incomplete prefix. Zero capacity may retain an incomplete prefix but
cannot emit any scalar. Null input/output is admitted only with zero respective
extent, without pointer arithmetic. Output capacity is measured in uint32_t
elements, not bytes. Use a preflight pass and then an infallible replay, or prove
an alternative with the same rollback guarantee. Time is O(n), with at most two
visits per octet per pass under recovery; persistent storage is constant.

The changed-condition replacement policy is exact: an invalid starter consumes
one octet and emits U+FFFD. A valid retained prefix followed by a prohibited byte
emits one replacement for that prefix and reprocesses the offending byte. Thus
E2 41 becomes FFFD,0041, while ED A0 80 becomes three replacements. No new ASCII
starter is swallowed. Incomplete suffixes remain pending until `lu_finish`;
strict finish rejects, replacement finish emits one replacement if capacity
permits. Repeated finish on an empty state emits nothing. This policy is not a
promise to implement every Unicode replacement convention or normalization.

`lu_encode` emits the exact1–4 octet interchange encoding. `lu_utf16` emits one
16-bit code unit or the derived surrogate pair for a supplementary scalar.
Both reject surrogate inputs and values above U+10FFFF and preserve output/count
on invalid input or short capacity. Strict input validation takes priority over
output capacity: a chunk with both malformed bytes and too many scalars returns
INVALID. Derive U+1F600 as D83D DE00 after subtracting
0x10000; specify byte order separately when serializing these code units. A native
wchar_t object is neither this wire encoding nor a portable serialized unit.

Predict E2 then82 AC, every split of F0 9F 98 80, two interleaved streams, C0 AF,
ED A0 80, F0 9F, embedded NUL and a valid prefix followed by an ASCII starter.
Run unicode_tests.c with your component under GCC/Clang O0/O2 and a hosted
sanitizer profile. Add boundaries7F/80,7FF/800,D7FF/E000,FFFF/10000,10FFFF and
capacities that fail after a previously completed scalar. Preserve byte guards.
Deliver state-transition derivations, implementations, independent oracle traces,
two minimized semantic defects, and object/import reports for the strict target.

For the library notebook, record installed locales and establish C and an actual
UTF-8 locale before predicting results. Trace mbrtowc with persistent mbstate_t:
an incomplete call returns(size_t)-2 without a scalar; a later successful return
counts bytes in the later buffer. EILSEQ leaves conversion state unspecified:
abandon it and initialize a new state for a fresh stream. NUL returns0. Compare
wcrtomb's locale-selected bytes to the fixed interchange encoder; record actual
wchar_t width and byte order. Cross-build the16-bit platform manifest without
claiming its library was executed. Explain two-code-unit supplementary characters.

Classify all256 byte values after conversion to unsigned char, and preserve an
int-valued fgetc result until testing EOF. On the selected signed-byte Linux
profile, narrowing FF first collides with EOF; never pass negative non-EOF values
to ctype as a supposed defined experiment. Byte-wise isspace does not classify
U+20AC. The sequence65 CC81 has three bytes and two scalars; grapheme segmentation,
normalization and terminal column width each require a distinct versioned policy.
Do not infer them from wchar_t or this decoder.

Keep the protocol grammar ASCII through explicit octet comparisons. Compare this
with two actual threads using owned POSIX locale objects: per-operation *_l
classification and uselocale-selected conversion, restoring the previous locale
before freeing either object. Do not race process-global setlocale. Report an
unavailable locale as missing evidence; do not silently run a weaker comparison.

Rubric: level0 admits invalid scalars or violates lifetime/rollback; level1
reproduces Euro only; level2 derives all boundary/status/consumption rules and
distinguishes ISO, POSIX and observed ABI facts; level3 independently implements
the original changed condition after at least7days and explains which claims must
be revisited for another encoding, locale or wchar_t convention. Hints: first
derive the first-byte interval; next tighten the second byte; finally distinguish
retained progress from public output and preflight both before publishing.

Security boundary: original local decoder, synthetic octets, no network targets
or real identifiers. An attacker controls input chunks and malformed encodings;
the harmless success condition is an admitted forbidden scalar or lost delimiter
in a compiled semantic mutant. That proves a parser property violation, not code
execution. Repair admission/consumption and retain the counterexample. Bounds
checks do not establish normalization safety or agreement with a downstream parser.

Status: required authored package with separate source-bound author campaign.
Finite checks do not prove all histories; independent learner/review and physical
observations are separate. Assessor implementations/oracles stay outside exports.
