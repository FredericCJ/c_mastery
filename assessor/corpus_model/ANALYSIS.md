# Assessor analysis — X-T15-021

Keep this file and model.c out of learner exports. Assess three separate
criteria: domain state and transactions; actual syntax-to-model versus
query-to-AST composition; persistent read/eval/print and changed output conditions.
Using a general parser or a stateless calculator alone does not satisfy them.

The model is a stable-ID runtime graph. Temporary configuration edits are not a
second persistent AST; two distinct lexical shapes construct identical edits,
and the direct API bypasses syntax entirely. Reordering records must not alter
the resulting values. Forward references require whole-batch resolution. Reject
zero-weight dependency cycles, since edges describe semantic dependencies and
are not simplified away. Cached dependent values must change after an input
replacement. Inspect actual code composing the frozen Pratt provider, not merely
test names mentioning parsing.

For rollback, snapshot destination bytes before a refused batch; a failure after
editing an early slot must not leak that slot. Intermediate signed32 products
and sums have their own admission conditions. The Python recursive oracle uses
arbitrary-precision integers and checks these conditions explicitly. It does not
borrow the C repeated scan. Its generated graph cases use each frontend and the
direct API, then mutation histories. It does not exhaust all16-node graphs or
all signed32 inputs. Runtime valid-state checking is bounded but is not memory
safety against forged pointers or concurrent writers.

The expression-reference counterexample is input0=-3 followed by `$0^2`. Correct
grouping gives9. A bare substitution gives-9. INT32_MIN expands through two
representable decimal tokens and subtraction; the accepted lexer is unchanged.
The parser's expanded source offsets are never asserted to be original offsets.
No claim to implement original tokenizer objective T15.14.a is made by this
semantic-model refinement; that prerequisite remains independently assessed.

The stream counterexample accepts one byte of `OK\n`, then reports zero progress.
The model revision is already1. Retry must emit only `K\n`, retain revision1 and
refuse the attempted intervening command with consumed0. Rebuilding the reply
from its start duplicates output; rerunning the mutation increments revision.
The sink owns accepted prefixes, and publication cannot retract those bytes.
Process exit after failed fwrite loses this in-memory recovery state; the hosted
driver reports failure, while the session experiment deliberately keeps it live.

Required delayed reference: change the read chunk, interrupt output at every
selected accepted-prefix position, vary write windows, recover after malformed
and oversized lines, and finish with/without a final newline. Keep a literal
independent transcript plus revision/effect count. The oracle executes actual
stdio read/eval/print processes in addition to in-memory callbacks. EOF and exit
are different input boundaries; exit leaves later bytes unconsumed.

Compiled semantic controls1..6 respectively admit cycles, publish a staged
model before validation, miscompute linear caches, omit symbol grouping, restart
a partial reply, and accept trailing CSV fields. Each must compile and fail a
named CM_CHECK assertion. Sanitizer crashes or compile failures do not count as
causal controls. Default learner and missing prerequisite sources remain
incomplete. Explicit supplied providers are needed for success.

Source scope is narrow: Fowler's catalog is a short definition, the2005 article
shows single/two-pass reader construction and alternate syntax, and the Scheme
section describes read/eval/print with an evaluator callback. Our bounded graph,
transactional updates and recoverable print policy are original worked contracts.
No complete Fowler book chapter, full Scheme implementation or production DSL
equivalence has been inspected or claimed.

Author-team bounded source review by lab_batch covered command commit/reply
prefix authority, EOF and grouped minimum expansion under the stated lifetime
premises. It did not inspect or execute the independent oracle, reread the
parser, or supply independent human review. All formal external/learner/physical
dimensions remain open.
