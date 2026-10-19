# T59 image/epoch transfer assessor analysis

FW5's completion mask carries a controller-observation obligation that byte
equality cannot supply. The full-prefix/unsuccessful-status case is the critical
counterexample: all bits can match while the operation has not completed under
the assumed controller contract. The reference refuses metadata until every bit
in the unit mask is acknowledged and an independent whole-image comparison
matches. Old image/metadata are immutable through candidate work. A later reset
recovers from persistent bytes and valid metadata, not from a volatile progress
mask; signature verification must be repeated over that recovered immutable
extent. A CRC collision or adversarially rewritten record is outside CRC's claim.

There are8×33 plus4×65=524 partial-program traces. These cover the chosen ordered
bit-prefix fault model, not arbitrary analog failure or all subsets of image bits.
The separate294 journal cuts concern the record's own publication phase. Combining
the two proofs requires the actual device ordering/completion guarantee; host
tests alone cannot manufacture it.

FW6 treats recovery as a new trust/persistence transaction. Candidate construction
must not mutate live policy before the persistence callback succeeds. Epoch2
dominates every epoch1 sequence, but the version floor remains independently
monotonic. Old key1 is revoked in the same committed policy as the new key2;
separating these updates could expose a reset interval admitting the old key.
The finite two-key trust model intentionally reaches a terminal configuration.
An implementation that clears revocation to avoid that terminal state defeats
the security property. A third key needs external provisioning authority and a
new atomic policy format; do not overstate this bounded example as a complete
production boot chain or rollback-resistant hardware root of trust.
