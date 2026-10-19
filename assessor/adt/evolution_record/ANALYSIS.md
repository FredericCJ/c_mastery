# Assessor: Byte preservation is not permission

Admit all headers/payloads through the retained cursor and TLV contracts before
publishing the owned record. Since count≤8 and payload≤16,1+8*(2+16)=145 bounds
all staging arithmetic. Singularity applies only to known tags1/2. Unknown
duplicates form ordered lists per tag; stable tag sorting gives a unique byte
form for permutations across different tags without erasing within-tag order.
Canonicalization is idempotent. Sorting equal tags by payload would silently
change a future last-occurrence interpretation and is therefore unlawful.

The authentication domain includes its final zero byte and the complete record,
including unknown duplicates, count and lengths. Hashing recognized projection
creates equivalence classes that are too broad for the downstream interpreter.
The supplied local attacker does not break HMAC: it changes unauthenticated
semantic information that the vulnerable target still consumes. The observed
privileged counter is an original synthetic property witness, not machine
privilege escalation. Complete-transcript authentication blocks that edit;
the independent whitelist then blocks authenticated but unauthorized semantics.

Counter overflow is checked before publication; neither ordinary nor privileged
state may change on refusal. Compare all authentication bytes. The fixed-loop
source is not a new timing proof. The actual keyed provider is imported rather
than substituted by the old bounded SHA helper or a toy checksum. Public-key
signatures are an explicit transcript-policy comparison, not an executed
asymmetric implementation.

The codec adapter is deliberately opaque: actual signed16/32 encoding and
checksum rules belong to codec.records, while the relay stores exact bytes.
The original single-TLV supports255 bytes and borrows its source; the new record
has16-byte fields and owns copies. Both distinctions must appear in the learner's
contract. The independent oracle compares complete buffers and failure state,
not only recognized semantic fields, and host HMAC provides a separate
implementation for the exact transcript. All eight semantic mutants require
normal assertion exits. Do not substitute a sanitizer fault for the declared
failure mechanism or infer a learner result from the author matrix.
