# Owning-sum technical analysis

The abstraction is a disjoint sum of empty, uint32, two uint32 values and a
finite byte sequence. Pointer identity, allocation addresses, padding and
callback counters are not semantic text equality. The concrete representation
nevertheless carries release authority; copying it with assignment does not
create that authority. A valid clone establishes a second distinct allocation
and independent final-drop obligation.

The original cardinality is 1 + 2^32 + 2^64. The extension adds
sum(n=0..65535, 256^n), including one empty byte sequence that is still distinct
from EMPTY. The product's two uint32 fields multiply cardinalities; tagged
alternatives add them. Native padding and allocation choices do not multiply
semantic values, and equal measurement does not imply equal variants.

| Existing operation | Required TEXT behavior |
| --- | --- |
| measure | Return byte length, including zero; never read number/pair members. |
| number/preview | Reject unchanged; a default zero result falsely claims NUMBER. |
| copy/assignment | Clone through source policy before releasing destination. |
| move | Transfer pointer, extent and policy; empty source, no clone. |
| drop/replacement | Release exact prior policy once, including empty text. |
| initialization | Only unowned storage; reinitializing a live owner would leak. |

The key transaction is clone into an unpublished local, then release the old
destination, then publish the staged value. A callback failure returns before
old-owner destruction, including when input aliases a suffix of the old
payload. The callback, not the sum, owns partial allocation cleanup. Different
source/destination policies must not be interchanged during replacement.
Self-copy/move are handled before any callback. Move empties the source before
dropping the destination; non-reentry prevents observation of intermediates.

Drop clears the tag and then releases cached pointer/length/policy. This order
does not grant reentrancy: a callback could otherwise destroy or replace the
destination before the final assignment. Every callback is nonfailing and
normal-returning by contract; longjmp or retained borrowed inputs invalidate
the reasoning. Public C fields do not statically enforce these restrictions.

The original pair sum is at most2*(2^32-1), fitting uint64. An old default branch
can compile and return0 for new TEXT, confusing empty measurement with complete
variant handling. -Wswitch-enum catches omitted named enumerators under the
observed diagnostic policy even with default. It cannot prove payload validity,
callback ownership, absence of forged values or every client's logical intent.
The Rust nonexhaustive-match negative and wildcard positive make that distinction
concrete without claiming language equivalence or whole-language coverage.

Controls intentionally fail before invalid ownership is dereferenced or freed
twice. The test-only emergency registry reclaims actual allocations on exit so
leak diagnostics cannot mask the intended semantic assertion; counters still
expose a missing release. Require the exact rejected assertion and successful
normal control, not merely a nonzero process exit.

The trace oracle models values and obligations independently of C union layout.
Its finite schedules cannot prove arbitrary callbacks or concurrent lifetimes.
The separate caller contract and adversarial changed-condition reasoning are
mandatory. No author support grants learner level3 or human independence.
