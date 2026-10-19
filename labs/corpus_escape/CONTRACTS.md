# Text escape pair and dialect composition, version 1

This complete shared contract supplies the API, failure ordering, ownership,
cost and security premises for X-T15-022. It refines the existing template
exercise with the missing inverse and an actual incompatible-consumer adapter.

## Representation and language

Use C17 with ordinary eight-bit character octets, explicit size_t lengths and
EsText:384 bytes plus a length. The meaningful prefix is not a C string.
Input plain text is at most64 octets: printable ASCII32..126, LF and TAB.
NUL, CR and all high octets are rejected. No allocation, recursion, VLA,
string/conversion/escaping library or external parser implements this core.
Profile C permits compiler byte-copy helpers; a hosted executable is not a
strict freestanding final-image audit. Host Python models and tests are separate.

`es_encode` replaces every `& < > " '` respectively by
`&amp; &lt; &gt; &quot; &apos;`. Other admitted octets are copied.
`es_decode` accepts a concatenation of those five complete, case-sensitive
entities and admitted plain octets other than those five literal special
characters. It decodes once into at most64 octets. Unknown, numeric, incomplete
or differently cased references are rejected. This canonical subset is stricter
than XML: it is not an XML processor, HTML sanitizer or Unicode codec.

`es_recode_html` accepts the same fragment language except that apostrophe is
spelled exactly `&#39;`, not `&apos;`; it decodes that dialect once and encodes
the result into the named dialect. Both input forms have a384-octet limit.
It validates bytes, not trusted provenance. This is designed for the actual
`tc_template_render(...,html_text=1,...)` output for one model value, not arbitrary
HTML, template literals, attributes, scripts, URLs or style contexts.

## Operations, invariants and failure

All three signatures take `(input,length,capacity,output)`. Capacity0..384 is a
caller-imposed bound on the successful output prefix, not the physical EsText
allocation size. Live input and output objects must be disjoint, aligned and
uncorrupted; caller operations are serialized. NULL input is permitted only
with length0, and no input pointer arithmetic is then required. Output is
nonnull. There is no arbitrary pointer-validity detection or overlap promise.

Validate output/input control premises and capacity first: violations return
ES_INVALID. Oversized declared input returns ES_RANGE without reading input.
Scan left to right; a malformed character/entity returns ES_SYNTAX; decoding
a65th valid plain character returns ES_RANGE immediately. Only after complete
semantic validation compare the output size against capacity: ES_CAPACITY.
Every failure preserves the entire output object's bytes, including padding.
On success publish a length and initialized byte array; bytes beyond length
are zero, but padding is not a promised serialization. No resources are acquired
or retained; input lifetime need only cover the call. Staging is an intentional
cost of unchanged-output failure semantics.

Prove the invariant that the staged prefix equals the transformation of exactly
the consumed input prefix. An entity replacement is appended as data and is
never scanned again. Derive `decode(encode(s))=s` on the admitted domain and
give counterexamples to idempotent encoding, arbitrary-HTML round trips and
unrestricted repeated decoding. A fixed five-entry comparison table has constant
maximum lookup work. Time is O(n), encode expansion at most6n, core storage
O(384); recoding temporarily holds a decoded stage and encoded stage. Measure
actual sizeof/stack separately from these abstract storage bounds.

## Concrete composition and changed conditions

Use the real X-T15-016 template and X-T15-019 event parser artifacts. Compile
`{a}`, supply a single field value and render HTML-text mode. An apostrophe
produces `&#39;`: both this named decoder and the original events parser reject
it. The recoder makes `&apos;`, after which `<a>` plus the encoded fragment plus
`</a>` produces exactly one START, one END and one DOCUMENT with unchanged
character data. A literal input `&#39;` first becomes `&amp;#39;`; recoding must
preserve that literal spelling after one decode. Similarly escaped `<a/>` must
produce characters, not a nested START event.

Run every two-part input split and both actual EV_TEXT16 and EV_TEXT1 builds.
Callback data is borrowed only during the callback; the composition test copies
it. No event parser or template source is modified. `composition.c` is a public
verifier, not a replacement implementation. CMake supports explicit public
dependency roots and completed implementation source paths; isolated workspaces
must pass them. Core learner builds need no private provider. Composition uses
previously completed learner artifacts; no library implements the new pair.

## Security and acceptance

Only original local components, synthetic ASCII and copied event ledgers are
targets. Attacker capability is choosing admitted text, malformed references,
capacity and chunk boundaries. The property is preservation of data versus
syntax and failure atomicity. A harmless failure is an extra element event,
wrong character data or changed output on rejection. None establishes browser
script execution or arbitrary code execution. Repair escaping/once-only parsing
and retain the counterexample. No browser, external endpoint, credentials or
host security changes are involved. Escaping for one context does not authorize
use in another; caller context selection remains a discipline obligation.

Require exact deterministic tests, independent table-language comparisons,
two causal mutants (unescaped ampersand and repeated decoding), compiling learner
stub rejection and actual composition. The assessor must separately grade the
invariant, failure model, context limits and delayed changed-condition reasoning.
Finite author tests are not independent learner mastery or physical evidence.
