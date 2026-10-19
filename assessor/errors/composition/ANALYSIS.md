# T65.04 owning composition assessment

R1: split on the tag. For OK, map composition is ordinary function composition;
for error, neither side invokes a value callback. Bind associativity has the
same f-then-g trace if f succeeds, f alone if it fails, and neither on an initial
error. Effects do not by themselves defeat this trace-preserving equation.
Eager unused computation is a concrete counterexample: the result is IO7 on
both sides, but one counter is incremented only by the eager implementation.

R2: preserve the active member. An owning result's copied representation is not
a second owner. Adopt empties its source, clone-map allocates distinct storage
before invocation, and drop publishes EMPTY before destruction. An allocation
failure must not execute the map even if its numeric return would be discarded.
The allocator and release observer stay live until all owners are dropped.

R3: independently evaluate the declared16 and72 function tables, not the C
control-flow algorithm. Compare traces as well as values. The full mathematical
space of functions0..7 to0..7 has8^8 elements; do not claim that all pairs of
those functions were enumerated. The finite declared families are exhaustive
only in their own model. Reject each isolated compiled control by a normal
assertion before unsafe duplicate cleanup. Seven timing samples are descriptive,
not a universal overhead theorem; scalar and owning contracts differ.

R4: an error is an ordered list of at most four exact causes. Validate before
appending, preserve the root offset even at SIZE_MAX, and refuse capacity rather
than silently dropping context. A fifth layer is an operational FULL result,
not an allocation attempt. For an unseen transfer, give an owner whose callback
has a visible counter and an allocator that fails the clone. Ask the learner to
compare consuming and borrowing maps before offering a solution. In the chosen
borrowing model, identity is only payload equality; allocation observability
and failure distinguish it from returning the original owner.

Rust compiler evidence rejects reusing a moved non-Copy result; the corresponding
C invalid ownership copy must be prevented by the protocol. No C diagnostic is
promised for that semantic copy. The inspected documentation and installed
compiler versions differ and are separately labeled. This author analysis is
not an independent learner assessment or external-human review.
