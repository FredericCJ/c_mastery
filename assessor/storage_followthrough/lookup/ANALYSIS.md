# Lookup and reentry technical analysis

The92record oracle contains prefix00/0000 followed by three-byte records over
first octets0,1,128,255. Python bytes sorting independently supplies unsigned
lexicographic order. The original choose() breaks only for a strictly smaller
probe, so equality advances right; the retained BAD_SEPARATOR control changes
that comparison. Query every separator itself, not merely every inserted value
under a potentially matching wrong insertion policy.

Normalization changes key equivalence. Printable ASCII A..Z folds, punctuation
does not, and high-bit/control data rejects. Alpha and ALPHA intentionally replace
the same canonical key. Bind checks that preexisting keys already satisfy this
policy, avoiding a silent migration with ambiguous collision handling. C cannot
prevent direct calls to the wrapped raw store; that is a stated discipline limit.
Stage the complete normalized bytes and length before publishing, including when
the input/output byte objects overlap.

The original scan callback gets pointers into a transient local node. Nonzero
means ordinary stop; accepting zero records can still require one invocation.
Copying the full validated record set before client callbacks supplies a different
contract: the actual source engine is no longer on the stack, so callback deletion
and a nested capture into another snapshot are permitted under serialization.
The outer217-record inline capacity prevents external ownership; no source or
page pointers are stored. This is snapshot copying, not shared version/reclamation
or lock-free concurrent access. It also makes lifetime cost explicit.

The full store checker is necessary before capture because the linked-leaf scan
alone need not reveal an allocated unreachable page or duplicated internal child.
Use a valid active tree as the starting point, then change one invariant at a time.
The visited-page check rejects a shared subtree on its second visit without a
directed cycle. A leaf self-cycle is separately bounded by the scan's seen mask.
Admission failure preserves the destination snapshot, and the synthetic active
fixtures perform no backend reads while checking their private typed nodes.

Finite exhaustive two-octet normalization is complete only for that stated input
space. The92records, four corruptions and reentrant deletion trajectory are
bounded examples. Independent learner explanation/transfer, human review and
physical validation are not implied by the author tests.
