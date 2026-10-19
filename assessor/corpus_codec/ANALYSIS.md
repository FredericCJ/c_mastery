# Private compression assessment

Require representations, proofs and exact failure traces as well as working code.
The reference uses a31-node forest and parent chains for lengths, integer
canonical-code counts, bounded staging arrays, forward overlapping LZ expansion
and backward optimal parsing. Valid alternatives include a priority queue for
Huffman construction and a prefix trie decoder, provided code selection and
failure publication match the public contract. A table decoder must charge its
larger memory and validate holes, rather than treating unused entries as symbols.

For an empty forest all lengths are0; for a single positive symbol its length1
is a wire convention. Equal positive16-way frequencies give all lengths4. Three
length1 symbols exceed prefix capacity. An incomplete tree is not itself invalid:
the dictionary may leave unused prefixes, whose attempted decoding must fail.
Do not conflate prefix uniqueness, statistical optimality and integrity.

The greedy counterexample `00 00 00 01 00 00 00 00 00` is supplied only here. At
the later zero run, choosing a locally longest match may leave a costly literal;
forward shortest-path cost is independent of the implementation's backward DP.
Require learners to derive edge costs and recover a legal parse; an unexplained
minimal number is insufficient. Huffman recompression has a different objective:
the token histogram matters as well as length, and headers do not disappear.

Minimum expected code length follows the sibling-exchange/contraction induction
only for a fixed positive-frequency alphabet and unrestricted binary prefix
tree. For a new depth ceiling, clipping may produce prefix collisions. A sound
changed-contract analysis may derive package-merge/DP with its own state and
proof, or refuse an infeasible ceiling before changing output. It may not claim
this package already implements that separate algorithm.

The hdecode staging buffer is bounded by512 symbols; LZ expansion stages128
bytes. Check count and arithmetic before input indexing. An overlap copy must
read the newly appended prefix: a temporary snapshot contains no future decoded
bytes. On failure even the `used` value remains its original sentinel. Incomplete
parsing must not publish a prefix under a transactional API.

The four semantic mutants compile and return normal failure through CC_CHECK:
book oversubscription, padding, overlap and missing optimal matches. They are
not deliberate out-of-bounds exploits. The original target admits attacker bytes
only through local synthetic input. A bit flip can produce another valid message;
neither a clean sanitizer run nor a decoder status provides authentication.

Rubric dimensions are independent: code construction and format reasoning
(T15.15.a); work/storage/representation choices and changed optimization objective
(T15.15.b); independent models, negative controls and bounded inference
(T15.15.c). Score0 if unsafe or unsupported;1 for nominal roundtrip only;2 for
the full ordinary contract and justified evidence;3 requires independent delayed
optimal parsing, a valid counterexample and transfer reasoning without copying
this reference. Actual learner scores remain unrecorded by author campaigns.

Graduated hints: count occupied prefix slots; draw the live decoded prefix before
copying; list all legal edges from one input position; distinguish bytes in a
token stream from bits after entropy coding. Do not reveal the private
counterexample until the appropriate assessment stage.
