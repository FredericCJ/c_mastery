# Assessor analysis: bounded original fuzz target

The core validates cap before reading input, then admits exactly the declared
grammar. The optional profile is explicit and creates a canonical temporary
frame for the unchanged decoder. Output staging prevents invalid flags/lengths
from leaking partial values. Stateless repeated calls and stateful session
sequences are different test architectures: resetting before every session
message would hide duplicate/out-of-order failures.

The maximum arithmetic sum is4*65535, fitting uint32, and total addition is
checked before publication. A successful final sequence65535 advances next to
65536 and further packets are refused; a later greater epoch resets authority.
No claim of authenticated identity or enforcement of caller lifetime follows.

The intentional payload-read defect exists only in the original local specimen.
Its header predicate explains the lower length bound2. A reducer must preserve
the diagnosed function/read class and check the corrected control; it must not
replace a bounds violation with an unrelated assertion or timeout. ASan can
report this selected access without proving all pointer safety or exploitability.
The separate wrong-value mutant is defined execution and requires a semantic
oracle. The separate255 timeout specimen demonstrates containment and saved
replay, not parser completeness or a host denial-of-service target.

Meaningful seeds reach accepted payloads, nearby truncations, empty state,
excessive count, wrong opcode, trailing bytes and optional-field transitions.
Random corpus size alone does not show this variety. LLVM coverage feedback
selects mutations, while the explicit grammar/output oracle decides acceptance.
All source/tool/corpus and report identities must remain bound; tampering,
compiler failure, unrelated sanitizer output and invented physical evidence
must fail the guard. Preserve the minimum-day and independent learner boundary.
