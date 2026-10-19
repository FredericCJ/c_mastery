# X-T14-017 private assessor analysis

R1 (`T14.03.b`): scalar read/write frontiers imply write never exceeds read; retained order follows original traversal. Assignment count excludes positions already in place. The logical end is not the allocation end or a size mutation. An implementation that erases each match separately can produce the same sequence while doing quadratic movement. Reject an explanation that treats unspecified tail values as removed originals.

R2 (`T14.03.c`): the bounded owner deliberately evaluates all decisions before publishing a stable partition. This supports a read-only context alias into the original array and strong owner/output failure behavior. It cannot undo external predicate logging. Every original right remains in exactly one partition until physical erase or clear. Stage copies are not new owners. C++ owning move assignment may destroy the destination resource during compaction; our no-drop-before-erase policy is a stronger, different contract that costs staging. Do not claim to reproduce the complete C++ generic algorithm or exception model.

R3 (`T14.15.b`): after publication the token's owner, epoch and logical end must match current pending state. Generation headroom is checked before callbacks so erase cannot fail from wrap. A pending clear consumes all rights and invalidates the token. Matching public fields can be forged, and an old pointer after owner lifetime is outside the contract. The reversed-prefix and wrong-ID-drop mutants are independently detected without executing real invalid frees. Ask for the earliest wrong state, not merely the final failure marker.

## Withheld transfer

Use a different ID/value permutation, a predicate context pointing to a middle original item, and a failure after several removals have been decided. Require predicted original count, keep count, tail IDs, callback attempts and drop counts before execution. Then clear while pending and try both a copied current token and an earlier token. Finally explain the difference between rejected stale metadata and a dangling owner pointer without dereferencing one. Compare actual compiler/library logs, but accept different permitted unspecified-tail and destruction-timing observations when the standard-level claim remains correct.

## Evidence limits

The independent Python oracle expresses list partition and ownership sets without copying the C loop. It is an author-created independent model, not independent human assessment. The C++ executable is a separate standard-library experiment; its selected implementation and C++17 mode do not turn current working-draft source passages into published-edition evidence. Finite cases and sanitizers do not establish universal safety or learner mastery.
