# T61 solver assessor analysis

The arithmetic proof ranges over all uint32 operands under CBMC's selected data
model. Search/ring proofs are finite over declared lengths/operations; an
unwinding assertion is essential to distinguish proven loop completeness from
unexplored iterations. The witness false assertion must be reachable under the
same admissible initial-state assumptions. Adding n>8 after n<=8 causes apparent
success because no execution remains, not because the implementation improved.

The lower-bound property checks both sides of the partition and the returned
range. Ring properties preserve count/head bounds and exact transition/refusal;
they do not establish ownership of external payloads. The three mutations are
independent of the witness/vacuity controls. Withheld transfer changes sorted
ordering to a comparator with equivalence classes; requiring strict total order
when the real API supplies only a preorder changes the theorem. Require an
explicit assumption counterexample rather than silently strengthening the client.
