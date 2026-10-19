# Required certificate transfer: stream boundaries and explicit traversal storage

This companion completes the concrete implementation support for existing
`X-T36-001` V2 and `X-T36-002` V1. Their objectives, original rubric identities,
mandatory status and minimum seven-day delayed assessment remain unchanged.
Families B/C/D/F/H/I/J apply. The signed certificate and red-black policy are
reused from the completed graph/tree providers, not replaced with new solvers.

Prerequisites: derive source-rooted shortest-path witnesses and edge inequalities;
implement byte decoding, bounded arrays and a binary-tree certificate; bring
your actual `AGCertificate`, `CDCSR`, `ATCertificate`, `ag_certificate`,
`cd_csr_build` and recursive `at_check` artifacts. Record source/header identities
and any contract conversion. Author campaigns import named assessor references;
that is not learner integration. Read the original exercises and the contracts
in `labs/algorithm_frontier/CONTRACTS.md` and `labs/algorithm_tree/CONTRACTS.md`.

## C and execution profile

Use C17, eight-bit bytes, an `int` range including -32768, exact `int32_t` and
`int64_t`, live aligned distinct objects, and exclusive synchronous ownership.
Public structure fields expose representation; ISO C does not enforce opaque
state, borrowing, valid extents or synchronization. The input attacker controls
wire octets and copied certificate values, not host pointers or stream metadata.
No allocation, sockets, time source or asynchronous cancellation is involved.
Host tests may use stdio/Python. The Cortex-M4 MPS2 image uses explicit startup,
direct linking and original byte helpers; archive runtimes are forbidden. Audit
objects and the selected image, distinguishing references in discarded provider
functions. Emulation is not physical timing or board observation. No additional
C23 guarantee is assumed; published edition-specific comparison remains in the
existing source-access ledger.

## V2: a graph is not complete until its enclosing stream is complete

Implement `ct_init` and `ct_feed` in the learner file. A frame is six header
octets `47 43 01 V E S`, then E four-octet edges `(from,to,weight-high,weight-low)`.
Require 1<=V<=8, E<=24, S<V and both endpoints<V. Weight is a signed 16-bit
big-endian two's-complement **wire** integer; decode mathematically before the
representable conversion to C `int`. Parallel edges are permitted and preserve
their original within-source order through the actual CSR builder. Maximum
frame size is102 octets. The caller supplies a certificate at finalization with
the existing finite [-65534,65534] distance domain and rooted predecessor rules.
Do not invoke any shortest-path solver to validate it.

For the exact18-byte example
`47 43 01 03 03 00 00 01 00 01 00 02 00 02 02 01 ff fd`, accept source0,
reachable mask7, distances `[0,-1,2]`, predecessors `[0,2,0]`. The final edge is
2->1 of weight-3. Changing distance1 to0 must fail certification.

Every call receives a valid initialized stream and live output; `p` may be null
only for n=0. Nonempty input, output, stream and certificate backing are distinct.
The function borrows input only during the call and copies its octets. Nonfinal
calls return `CT_MORE` even at exactly the declared length: a subsequent octet
would invalidate the frame. No output publication occurs until a final call
has exact length and valid syntax, CSR and certificate. Finalization may carry
the last fragment or zero bytes. `CT_OK` publishes one complete CSR and retires
the stream. Extra octets, bad header/endpoints or capacity excess produce
`CT_INVALID`; incomplete final input produces `CT_TRUNCATED`; a rejected witness
produces `CT_CERTIFICATE`. These failures poison the stream. Any call after
success or poison returns `CT_STATE`; reset explicitly starts a new frame.
All non-success results preserve the entire output object. Invalid API arguments
return `CT_INVALID` without modifying the stream. Scratch bytes are not securely
erased and are not a secret-storage mechanism. Caller corruption of internal
controls is outside the input-adversary model.

Derive why per-chunk zero padding or publication at declared length loses a
framing property. Model states OPEN, COMMITTED and FAILED; final input is a
trusted enclosing-boundary assertion, not an inferred EOF from an empty chunk.
Explain that a valid certificate describes this exact graph, not later mutations.
At most102 input bytes are retained; decoding and construction use bounded
O(E+V) work and storage, and certification retains the existing O(V*(V+E)+E)
bound. An unbounded number of empty nonfinal calls requires an outer work budget;
this synchronous API gives no elapsed-time or fairness guarantee.

## V1: move traversal state without changing the tree predicate

Implement `ct_tree(c,policy,workspace,capacity,height)`. For capacity64, accept
and reject exactly the same certificate set as the retained recursive
`at_check`: policy0 ordered tree and cached height; policy1 adds AVL balance;
policy2 substitutes red-black root/color/path conditions for cached AVL height.
All policies require valid indices, root/child parents, unique reachability,
strict **ancestor** key intervals and exactly the live/count set. A NIL child
has height0 and black contribution1. Policy2's convention is not the textbook's
exclusion-of-current-node black-height definition; equality is what is tested.

Use no recursion and no heap. The caller owns a workspace of64 frames and64
visited entries; capacity0..64 controls the usable frame prefix. A frame records
the pending node/parent, open int64 key interval, phase, and completed child
height/black contributions. Scratch may change on any admitted call. The height
output remains unchanged on every reject. Invalid certificate or arguments
return `ALGO_BAD`; insufficient workspace returns distinct `CT_SPACE`99; success
returns `ALGO_OK` and publishes the recomputed height. Capacity64 cannot run out
before rejecting a malformed graph because a slot is marked before descent.
An empty valid tree needs no frames. A64-node ordered chain needs64 frames and
is valid for policy0, rejected for policy1. Report workspace `high_water`.

Prove the loop invariant: each frame corresponds to a marked, uniquely reached
node; its phase records which child results are complete; pending intervals
encode all ancestor constraints. Show why decrementing used before publishing
to the parent preserves postorder. Work is O(capacity+reachable nodes), explicit
storage O(capacity), and call-stack use has a fixed bound independent of tree
height for this nonrecursive implementation. Moving frames to caller storage
does not make total memory constant. Compare measured emitted function sizes
and compiler stack reports against the original recursive provider at identical
compiler/options; account for recursion depth and inlining rather than summing
unrelated function frames or calling `.su` output a physical high-water result.

## Deliverables and verification

Submit the two implementations, wire/state and traversal derivations, ownership
and rejection matrix, independently computed witnesses, exact provider identities,
commands/raw results, source/object/image audits and a measured resource table.
Run `python3 labs/certificate_transfer/run.py --implementation <your-source-dir>`
for the public learner path. The directory must contain `stream.c` and `tree.c`;
the default uses the deliberately unsolved learner file. Imported prerequisite
providers are explicit command arguments, never silently supplied assessor code.
The author-only runner is `python3 tools/run_certificate_transfer.py` under the
documented WSL toolchain. It compares the actual recursive provider, independently
checks descendant closure/leaf elimination with seed360021, and requires compiling
early-publication, unsigned-wire, lost-interval and stale-height mutants to fail
semantically. Public boundaries include every three-fragment split of the18-byte
frame, every truncation, reset, malformed headers, maximum input, a64-deep tree,
capacity failure, integer-extreme keys and malformed index graphs.

Level0 violates a contract; level1 replays ordinary examples; level2 derives each
original criterion, supplies working construction and falsifies the relevant
mutant; level3 independently handles the withheld changed representation and
new malformed witnesses after seven days. Require level2 for every applicable
criterion and no unexplained failure. A test name or green build does not supply
the proof, resource argument or independent assessment.

Hints after an unaided attempt: (1) write the publication boundary and pending
frame states; (2) separate syntax admission from witness validation, and mark a
tree node before descending; (3) save both child results in the parent frame,
then apply the exact recursive postcondition. The assessor's separate analysis
and delayed package are excluded from learner exports.

Security demonstration: only these original local programs with synthetic byte
frames/index certificates are targets. A harmless success for a flawed specimen
is early publication or acceptance of a malformed certificate. It requires the
corresponding removed gate; it demonstrates neither memory corruption nor code
execution. Repair the gate, rerun the same witness and independent regressions;
sanitizers alone do not detect this defined semantic error. Pointer validity,
exclusive ownership and trusted finalization remain caller obligations.

Validation is recorded in the source-bound companion report. External review,
actual learner substitution, delayed unaided performance and physical measurement
are separate dimensions. The corpus is the two original exercises and the named
retained providers, plus the bounded inspected CLRS13.1 passage registered beside
the lab; no entire-book inspection or absolute mastery claim follows.
