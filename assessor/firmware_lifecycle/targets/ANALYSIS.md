# T59 extension assessor analysis

Publication depends on the bytes recovered, not on whether a programming call
returned. Every new record unit precedes the marker, and the old record is never
erased during staging. A torn marker can equal the complete marker before all32
program-bit positions have been processed because some desired bits are already1;
this is valid if every payload unit was completed first. Accepting only marker
byte0 is a distinct defect and the byte-subset fixture kills it. CRC plus
complement fields detect selected corruption; coordinated changes with recomputed
CRC remain possible, so no authentication follows.

Verifier tests require exact full-image/key/signature binding and reject any
verdict other than1. They do not test Ed25519/RSA/ECDSA or trusted key storage.
After key1 is revoked, image key2 may remain acceptable under the independent
version floor. The callback context and byte storage must remain immutable
through verification; a TOCTOU mutation would invalidate this argument.

Health bits from one generation cannot be reused after reset. A finite maximum
is a deliberate refusal, not modulo ordering. Nested fault logging preserves the
first evidence and records only the fact of recursion; it cannot make arbitrary
memory corruption recoverable. Withheld transfer: allow program units to overlap
the marker and a payload field. Require a new commit design, not reuse of the
existing cut proof. Another variant revokes the only key before installing its
replacement; the learner must diagnose the resulting unavailable recovery path.
