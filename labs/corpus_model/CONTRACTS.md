# Persistent signal semantics and resumable read/eval/print

This is a bounded, original C17 companion. It does not implement Fowler's reader
application, Scheme, or a general reactive framework. Its runtime mechanism uses
fixed typed arrays and checked arithmetic. Stdio and Python belong to hosted
drivers/oracles. No heap allocation, external parser, graph library or scripting
runtime implements the model. The separately accepted original X-T15-012
`pd_lex`, `pd_pratt`, and `pd_evaluate` are actual dependencies for queries.

## Domain objects and transactions

A `CmModel` has sixteen stable slot IDs, a uint32 revision, and nodes of kind
absent, input, or linear. An input owns a signed32 constant in `bias`. A linear
node owns two dependency IDs, two signed32 weights and a signed32 bias. Its value
is **((value[a] * w0) + (value[b] * w1)) + bias**. Each product and each addition
must individually fit signed32; cancellation after an overflowing intermediate
does not rescue a transaction. Dependencies remain required even with zero
weight. Each linear node's cached value equals this calculation over the current
definitions. The persistent domain graph contains no tokens, AST nodes, source
offsets, borrowed text pointers, or expression operators.

`cm_init` creates an empty revision0 model. `cm_apply` replaces 0..16 distinct
slot definitions as one transaction. Kind must be input1 or linear2. Input
dependency/weight fields must be zero; unused fields of absent slots must be
zero. IDs are 0..15. Linear references may point forward to definitions in the
same batch. A missing slot, directed cycle, duplicate edited ID, invalid
definition, overflowing calculation, or exhausted revision refuses the entire
batch. Destination bytes remain unchanged. Successful nonempty batches increment
revision exactly once, even when replacing a definition with the same value;
empty batches do not increment. There is no deletion operation. `cm_valid`
checks canonical fields, reference/cycle validity and all cached values.

Evaluation has bounded progress: initially mark inputs/absent slots done; scan
linear nodes whose dependencies are done. Each successful resolution removes
one unresolved node. A complete scan with unresolved nodes but no progress
establishes a cycle, after missing references have been excluded. Worst case is
16 scans over16 slots. Staging a model makes refusal publication independent of
the scan order. This is an explicit topological runtime calculation, not the
transient AST's source order. A serialized single owner manages every live model
and session. Public fields are inspectable for experiments but must not be
changed except via initialization/API (the revision-exhaustion test is an
explicit valid-state fixture). All spans/control objects/output pointers are
live, correctly typed, sufficiently sized and disjoint; callbacks do not reenter
or mutate the session. No atomicity across threads is promised.

`cm_get` validates the model and returns one cached value. All numeric output
parameters remain untouched on failure. Status values are listed in model.h.
No arbitrary malformed object pointer or lifetime validation is possible in C.

## Two concrete configuration syntaxes

`cm_load` accepts at most1024 bytes and sixteen nonempty newline-separated
records, with an optional final newline. It reads an explicit byte span: a zero
byte is ordinary invalid input, not a terminator. Empty lines are ignored; a
line containing only whitespace is invalid. Each frontend emits temporary
`CmEdit` records, then calls the same atomic `cm_apply`. No record is published
while later text remains unparsed. An empty configuration preserves revision.

Line syntax (one or more space/tab/CR separators; optional leading/trailing
space/tab/CR):

```
linear 2 0 3 1 -2 5
input 1 7
input 0 -3
```

CSV syntax (literal commas, no whitespace, quoting or header):

```
L,2,0,3,1,-2,5
I,1,7
I,0,-3
```

Both define the same graph and produce slot2=-18 at revision1. A direct array
containing the corresponding three edits produces the same semantic fields.
Decimal fields permit optional `-` and at least one digit; no plus sign,
prefixes, trailing fields or out-of-range signed32 integers. IDs must additionally
be 0..15. Duplicate records are conflicts, not last-write-wins. Forward references
and record order are deliberately different from evaluation order. Changing or
destroying the configuration buffer after return cannot change the model.

## Expression queries reuse the accepted parser

`cm_eval` accepts at most256 input bytes. `$` followed by decimal ID names the
current node value. Resolve each reference and expand it into a parenthesized
canonical expression, then call the **actual** accepted lexer, right-power Pratt
parser and checked evaluator. The expanded form must also fit256 bytes and the
provider's64-token/64-node limits. Source diagnostics here are status codes;
the provider's internal offsets refer to expansion text and are deliberately
not misreported as original source positions. Configuration diagnostics do not
claim full source-location tracking either; X-T15-012 retains that obligation.

Grouping matters: input0=-3 makes `$0^2` equal9, whereas bare textual `-3^2`
would give-9. INT32_MIN expands as `(-2147483647-1)`, since the accepted lexer
cannot represent the positive token2147483648. Literal negative minimum text is
therefore not silently granted a lexer extension. The query AST is transient
and discarded after evaluation; the model persists. Operators and precedence
remain the accepted provider contract: checked `+`, `-`, `*`, unary minus and
right-associative `^`; exponent0..31 and0^0=1. Parser range errors map to CM_RANGE,
other parser/domain errors to CM_SYNTAX. Missing model references map to
CM_MISSING. A query never modifies the model.

## Persistent commands and output authority

Initialize `CmSession` once. Complete-line `cm_step` supports the line-syntax
definitions above, `set ID VALUE` (existing input only), `get ID`, `eval EXPR`,
and `exit`. A line is at most256 bytes, without its terminating newline.
Successful definition/set replies `OK\n`; get/eval replies `VALUE decimal\n`;
failed commands reply `ERR status\n` and preserve the model. Exit replies
`BYE\n` and closes admission. Empty complete lines are successful empty batches.
Unexpected trailing text is refused. `cm_step` rejects CM_BUSY if a partial
line or reply already exists. Stream APIs manage their own partial-line buffer.
Embedded LF in a direct complete-line call is a syntax error; multi-record
configuration belongs to cm_load, not a disguised multi-command step.

The command commits **before** publication of its reply. Reply bytes are owned
by the session, separate from input storage. `cm_drain` calls the supplied writer
with the remaining suffix. The callback returns the number of prefix bytes it
accepted; zero means temporary failure/no progress. A positive short count
advances the offset and returns CM_PENDING; zero returns CM_IO without losing
the pending reply; accepting the entire suffix clears it. Returning more than
offered is an invalid callback contract (CM_INVALID, offset unchanged); external
side effects of a lying callback cannot be rolled back. Exactly-once here means
each admitted command executes once within this live session, **not** exactly-once
delivery across process failure, dishonest sinks or durable restarts. A writer
that reports acceptance must own that prefix thereafter. Permanent zero progress
does not terminate by itself; the driver decides when to stop or retry.

While a reply is pending, new feed/step/finish calls return CM_BUSY. The delayed
variant must keep its committed model and sent offset across print failure, then
resume output without rerunning parsing or mutations. A retry is a drain, not
resubmission of the command. No new input is consumed at this boundary. This
failure policy is an original C contract, not a claim from the Scheme passage.

## Required changed-condition stream route

Implement `cm_feed`, `cm_finish`, and prefix-aware `cm_drain` after the complete-line
route. Feed accepts arbitrary byte chunks, reports the consumed prefix, and stops
immediately after one newline has queued one reply. Incomplete lines return
CM_PENDING. It buffers exactly256 non-newline bytes; a257th switches to discard
until the next newline, then emits one ERR3, clears the line state and can recover
after that reply drains. Malformed lines also recover at newline. Input remains
caller-owned and is not retained beyond copied partial bytes.

EOF is explicit `cm_finish`: if a partial line exists, execute it once; if an
oversized line is being discarded, queue ERR3; otherwise no extra reply. Mark
closed in all these cases. Pending output must drain first; finish on a pending
reply returns BUSY. Exit closes immediately with a pending BYE, so remaining
input in the chunk is unconsumed. Further feed returns CLOSED with consumed0.
The hosted `model_repl` uses actual fgetc/fwrite/fflush repeatedly, retains the
same session, flushes each reply, distinguishes read/print failure and supports
EOF after a partial final line. A nonzero process exit cannot promise persistent
recovery of the in-memory model; the session-driver experiment demonstrates
resumption while the session remains live.

## Teaching and assessment evidence

Read the exact inspected Fowler catalog, Single Pass Builder/Two Pass Builder
article sections and Scheme REPL section in the source register. The catalog is
only a brief definition; the book chapter is not claimed read. Trace one graph
from both syntaxes through edits into persistent semantic nodes, then trace a
query through a separate transient AST. Explain why a syntax tree alone does not
retain the model's input identities, dependency constraints and update behavior.

Predict cycle, dangling-reference, early-publication, stale-cache, substitution
precedence, repeated-print and trailing-field counterexamples. Six compiled
controls each violate one selected boundary. Independent Python checks use a
recursive graph oracle rather than the C scan, direct expected arithmetic and
explicit expected REPL transcripts. Tests compare valid answers, refused-state
snapshots and the effect/revision count. The delayed route changes chunk size,
every selected output-failure prefix, EOF and malformed-line recovery. Passing
author runs supply no elapsed learner attempt, independent human review,
physical execution, unbounded-language proof or production availability claim.

## Build and provider boundaries

Default CMake builds unsolved `src/model.c` and an explicit missing-prerequisite
stub `src/parser_dependency.c`; public tests must reject them semantically.
Supply your accepted parser through CM_PARSER_SOURCE and completed model through
CM_PROVIDER. The public provider/parsing.h is byte-identical to the accepted
public parsing_depth header. An explicitly supplied CMASTERY_ASSESSOR_ROOT selects
private model.c plus the actual sibling parsing_depth/pratt.c. Learner export
must copy neither assessor source. No implicit repository fallback completes a
learner solution. The campaign pins both canonical parser source/header bytes.
