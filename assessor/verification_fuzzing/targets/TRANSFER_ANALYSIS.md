# VT1-VT7 assessor analysis

For b>0 write M=q*b+r with0<=r<b. If a<=q then a*b<=q*b<=M. If a>q,
a>=q+1 and a*b>=q*b+b>M. Zero b yields zero without division. The independent
SMT lemma checks this integer bridge; CBMC checks the C threshold/low-word/error
contract. The initial combined bit-vector equivalence timed out, which remains
in a separate report. Neither successful subproof silently certifies the other.
Narrowing rejects negative values before any unsigned conversion.

Descending lower-bound partitions `key>query` from `key<=query`; equality must
choose the high half endpoint, not advance low. Identities are irrelevant to
comparison but matter to caller stability. The borrowed queue owns registrations,
not the pointees. `prepare` failure has no notification; a successful removal
detaches before a busy-guarded callback, making recursive operations fail without
touching the next entry. The callback's direct writes would violate preconditions.

Streaming validity depends only on the concatenated bytes since reset, not the
segmentation. Finish preserves state and output on failure; extra bytes poison
the frame. The session fuzz model must retain that poisoned state until explicit
reset. Five compiled mutants make the public assertions causally discriminating.

The shrink schedule contains two faulty-release events5 and6. A shrinker that
accepts any violation can delete5 and keep6, changing the diagnosis. The correct
signature retains event5 and the submit/cancel dependencies. Deletion-minimality
does not imply a globally shortest or semantically equivalent program.

The publication proof needs acquire to read the release's value. An unrelated
flag read or relaxed read creates no such edge. All shared payloads in the weak
example are atomic, so the example does not disguise undefined data races as
weak-memory behavior. The actual host rounds do not claim to witness the relaxed
counterexample; the finite premise model admits it. Assess this distinction
explicitly, and require a stronger model for release-sequence changes.
