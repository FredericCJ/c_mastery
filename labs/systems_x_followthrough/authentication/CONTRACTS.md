# MAC admission interface for copied redo records

X-T47-001 V2 requests an interface and a trust argument, not a new cryptographic
implementation. `SxMac` borrows an opaque key context and canonical message only
until synchronous return. Success writes exactly32 tag bytes; failure may change
its private context/tag scratch, but cannot publish a recovery result. It must
not reenter, retain pointers, alter records/checkpoint or access dead objects.
Operations sharing a context are externally serialized. A caller supplies a
completed provider explicitly; the local check imports the unchanged course
`rph_hmac` implementation. This adapter itself implements no hash primitive.

Canonical input is 32 bytes: eight domain bytes `SXREDO1\0`, store identity as
unsigned64 little endian, then generation, value, checksum and commit each as
unsigned32 little endian. No native padding is authenticated. Store identity is
nonzero and bound to trusted configuration. Check every committed prefix tag
before delegating actual `b5_recover`; stop at its first uncommitted tail. Require
the original checksum, generation-contiguity and failure-atomic publication too.
A valid tag never excuses malformed structure. Null/range/invalid-control errors
and callback/tag/structural failure preserve the caller result. Buffers/control
objects are live and disjoint; exclusive ownership keeps authenticated bytes
unchanged until the synchronous recovery returns. At most eight records exist.

Provision an unpredictable secret through a trusted owner; fixed test keys are
synthetic and supply no deployment security. Protect checkpoint generation/value
and store identity independently, select/rotate key versions without ambiguity,
and erase known copies at their actual lifetimes. The interface does not protect
keys from a hostile process, promise constant-time generated code, prevent key
compromise, or authenticate another format. This is not a CST2 disk migration.

An attacker controlling copied records can preserve XOR checksum while changing
a value; the keyed tag now rejects that change in the bounded experiment. Old
authenticated records at/below a trusted checkpoint are harmlessly skipped by
the original API. Rolling back the checkpoint and all authenticated records
together remains possible unless a separate monotonic authority prevents it.
An attacker can also truncate the stream or forge the first uncommitted marker;
this interface does not authenticate absence or a complete expected log length.
An independently authenticated commit head/manifest is needed to reject that
rollback rather than accepting an older prefix. No full hostile-disk theorem
is claimed by checking individual record tags.
Authentication does not create freshness, durability, availability, secrecy,
exactly-once external effects or a recovery result after a failed callback.

Implement the unsolved adapter, derive framing/trust/lifetime rules, compare the
actual keyed provider against the independent host oracle, and reject missing-tag
and missing-store-domain controls. Retain the original delayed minimum and
separate independent learner and physical evidence. Actual CST2 37×37 interrupted
recovery is supplied by the frozen durability companion, not by this record API.
