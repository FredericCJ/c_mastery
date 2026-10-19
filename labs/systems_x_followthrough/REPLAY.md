# Repeated replay under changed conditions

X-T44-001 V2 retains the actual two-run classifier in
`labs/debugging_followthrough/replay.py`. It compares state, exit status and
full stdout; a mismatch is UNRESOLVED, never a stable failing candidate. The
owned child fixture alternates the same failure identity and a passing result
through a private build-directory counter. Twelve actual fresh-process pairs
exercise mixed results. This controlled outcome schedule represents instability
in an otherwise hidden environment; it does not claim intrinsic nondeterminism.
Reducing using only the first failing run would incorrectly keep every candidate.

Keep exact source/binary/symbol/configuration/input/environment identities, bound
each child and retain both outcomes. More repetitions can expose instability,
but agreement in any finite sample cannot prove determinism. Retry limits must
not silently turn UNRESOLVED into PASS or FAIL. Grammar-aware tree reduction and
token reduction retain their distinct neighborhoods in the existing companion;
a one-deletion certificate is scoped to that declared neighborhood and failure.
