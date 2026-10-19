# Assessor analysis (withhold from learners)

The parser is a finite scanner plus bounded stack, not a whole-document tree.
In DATA, flush a pending text run before consuming '<'; this prevents a child
START from overtaking preceding text. LT distinguishes opening and closing
names. Require the first name character before permitting the closing '>'.
ENTITY accepts exactly the five finite spellings and appends a decoded byte
without feeding it through markup recognition. Empty tags schedule START then
END even when START waits. A separate pending flag is required because stack
progress is committed before delivery. Callback ACCEPT is the only operation
which removes that pending event. Retrying from the input byte would duplicate
parsed progress; advancing after WAIT would lose delivery.

Counterexamples: `<a></b>` defeats depth-only matching; `<a/>` with a full queue
defeats dropping a WAIT event; `<n>12345</n>` at text capacity1 defeats a consumer
which treats each text callback as a complete value. Missing consumer sequence
advance rejects the next valid event. All four controls compile successfully
and must reach EV_CHECK, not merely fail compilation or trigger a sanitizer.
No correct implementation may claim malformed input leaves the parser wholly
unchanged: accepted prefixes and callbacks already happened. The transactional
obligation belongs to each queue admission and each semantic-consumer event.

The sum consumer validates a complete event before changing numeric value,
sequence or stack. Decimal multiply/add is admitted via `(MAX-digit)/10`.
END checks total capacity before adding the completed leaf. A document which
later fails can have a partial internal sum; only DOCUMENT grants result
publication authority. Separate callbacks from ordinary consumer execution to
make copying, lifetime and backpressure inspectable.

Distinguish the direct total-overflow seam from public reachability: with a
uint32 event identity, at least three events per leaf and uint32 leaf values,
the reachable sum is below uint64 maximum. Explain why the conservative check
still has a meaningful local counterexample after a changed counter model.
Do not report that injected state as an observed ordinary document history.

R1 requires the exact grammar, bounded stack and counterexamples to XML/SAX
equivalence; merely recognizing fixture delimiters earns level1. R2 requires
the consumed-prefix/pending-event invariant, reentry refusals and terminal
authority. R3 requires a separate queue plus segmentation-invariant consumer,
overflow snapshots and final-document authority. R4 requires independent
Expat comparisons, four causal defects under both capacities, resource
derivation and object/link audit. Passing the supplied suite alone is level1
evidence until the learner explains each mechanism and limitation.

V1 changes text capacity16 to1, callback schedule and feed boundaries while
holding mathematical document content fixed; coalesce only consecutive TEXT
events at equal depth for oracle comparison. V2 separates parser/callback from
semantic consumption with two copied slots, repeated WAIT, deliberate input
overwrite and complete-document publication. Delay both assignments at least
seven days, change names/nesting/numbers and require independent reconstruction.
The supplied author executions do not award that delayed assessment.

Primary XML rules explain matched element names and proper nesting. The local
grammar intentionally rejects XML features and admits raw ']]>'; a conforming
XML processor requires more. SAX character segmentation is a useful comparison,
but its Java exceptions and managed references are not ISO C lifetime checks.
The external verifier is allowed to use Expat because it is not a target
dependency. Random finite trees and all split positions of selected strings
remain incomplete coverage, not an arbitrary-document proof.
