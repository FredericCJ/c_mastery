# Publication depth assessor analysis

R1: current and staged owners coexist before publication. A successful parser
does not transfer storage. Invalid/range/empty syntax must not even attempt
allocation. With a current three-block owner, kth failed allocation has k-1
successful temporaries and hence exactly k-1 temporary releases.

R2: mc_publish_pair delegates to actual er_parse_pair, ow_construct and
ow_publish. On success ow_publish transfers all staged fields and destroys
the old owner only afterward; the new owner survives. No automatic C struct
move or language destructor is inferred. The old pointers are checked only
on precommit failure, when they remain live.

R3: final destroy adds three releases to either precommit-failure or success
accounting. Control1 destroys old early; control2 overwrites its owners and
leaks them. Require their exact assertions. Separate stale-generation and
MAX tests protect cleanup headroom, not merely ordinary update success.

R4: close outcome1 retains its identity; outcomes0and2 both release it, with
2adding a diagnostic. Pending cells survive while either resource remains.
Second cleanup iterates only still-owned resources in reverse order. Thus
its callback consumption differs by first-attempt outcomes; a four-result
word is not always four actual close calls. If the first attempt drained,
repeated recover is invalid and leaves error sentinel99. Control5 forgets
retained ownership; control6 retains a released-error identity and causes an
illegal retry. The independent set oracle derives all81words without calling
the C implementation. Real close succeeds; before/after diagnostic injection
does not establish behavior of every kernel close error.

Publication and cleanup are separate events. A successful new five-resource
publication can leave old cleanup pending. A failed second open can leave
staged cleanup pending while old current remains authoritative. Both cases
need retained ownership, but the committed flag differs. MAX-1 is reserved
for final retirement, so counter exhaustion does not abandon current state.
These bounded author observations do not constitute an independent assessment.
