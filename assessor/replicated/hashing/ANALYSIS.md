# Streaming hash assessor analysis

The full schedule names W0..W63. The circular version overwrites slot t mod16
only after computing sigma1(Wt-2)+Wt-7+sigma0(Wt-15)+Wt-16. Each source index
lies within the preceding sixteen words; the new value becomes Wt. This is a
representation proof, not merely equality of two implementations sharing bugs.
The Python oracle instead uses hashlib/hmac and also derives round constants
with exact floor cube roots of p*2^96 and initial words with square roots of
p*2^64. Its synthetic-length compression code is crosschecked before use.

At used55, adding0x80 leaves byte56 free for the length; at used56 the length
needs a second block. The temporary count is the original byte count, so the
encoded bit count remains correct after padding. At maximum byte count
2^61-1, used63 and bits=2^64-8. One further byte is refused before arithmetic
or reads/writes. The injected state proves only this admission/encoding
behavior, not a hash of 2^61-1 supplied bytes. Treat a student claiming such a
measurement as incorrect evidence.

HMAC reduces keys longer than64 with a complete hash, not truncation. Inner
and outer pad states each consume64 bytes. The inner message bound must
subtract64 from the hash bound. A single shared state, duplicate pad constant,
truncated long key or missing outer layer violates the construction. Context
clones contain key-derived states and expand the set of secret owners; clearing
one object cannot erase the other. Volatile stores are a bounded source-level
operation, not a guarantee that no prior secret copy exists.

The security specimen's original message and key total16 bytes, so glue is
0x80,39 zero bytes, and the eight-byte big-endian bit count128. Its padded
prefix is64 bytes. An attacker initializes words from the returned prefix hash,
sets the processed count64, and hashes the extension. The verifier's full input
has exactly that chaining state at the boundary, so forged and target tags
match. The attacker does not need the key bytes. The actual target is a raw-byte
authenticator; it does not parse the apparent admin-looking suffix. HMAC's
returned outer digest is not the state required to continue its inner hash,
and the corresponding attempted forgery fails in the local test. This is not
a general proof of HMAC security or an instruction to target a deployment.

Controls1/2 corrupt padding and length endianness;3/4 truncate long keys or
reuse the inner pad;5/6 omit length admission or final-state transition;
7/8 replace HMAC by prefix hashing or check only the first tag byte. Require
normal exit1 with RP_CHECK in the appropriate anchor. Crashes, toolchain
failure, sanitizer errors or tests that never reach the mutation do not count.

At level2 the learner must explain the actual loop/state invariants and all
failure preservation, not repeat the test names. At level3 ask for an unseen
chunk split, a cloned prefix with different suffixes, a changed word-schedule
representation and a precise counterexample to the claim that equal hashes
authenticate a sender. R1/R4 remain incomplete without the separately specified
tree proof, malformed-path checks and transactional framed repair.

Inspect rph_equal and compress in actual optimized object output. Count public
loop branches and data-dependent addresses, but do not assert compiler-wide
constant-time semantics from one disassembly. Compare recorded stack usage,
text size and raw equal-contract timings for both schedules; sanitizers and
debug profiles are diagnostic configurations, not fair optimized competitors.
