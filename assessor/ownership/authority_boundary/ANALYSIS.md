# Authority boundary assessor analysis

R1: modulo2^32 makes (2^32-4)+8 equal4. The mask confines all physical stores to
indices0..15, yet logical tenant0 writes tenant1 indices12..15. The independent
security property is tenant separation, so quiet ASan is expected and does not
falsify the defect. No control-flow takeover follows.

R2: for offset<=length and count<=length-offset, offset+count<=length without
unsigned wrap. The trusted parent lies within one eight-cell tenant. Thus the
sum of its start, request offset and count remains<=8, and tenant*8 yields a
whole-array bound<=16. Empty one-past intervals never dereference. Validated is
private to this translation unit and built only after numeric and authority
checks, but this is an architectural convention, not C enforcement against
arbitrary memory writes. The server must choose the trusted parent out of band.

R3: status precedence is invalid view/expired view, malformed request, stale
request epoch, then denied authority. The mathematical oracle uses integer
addition and bit sets, independently of subtraction-based C validation. It
includes MAX-sized extents and all rights but fixes the trusted subview to2..6.
Do not promote that finite enumeration into a universal proof. Separate tests
cover original full-parent boundaries, exact bytes, overlap-safe decode and
finite retirement. Controls1/2 restore original wrapped/one-past checks;
3widens a read-only child;4omits request epoch;5omits tenant matching;6publishes
decoded output before syntax validation;7omits operation rights;8omits view
expiry. Require each exact semantic assertion and a compiled positive control.

R4: a wire request supplies values, never authority pointers. Syntactically
valid count/epoch/rights can still be unauthorized. Attenuation composes by
interval inclusion and rights-set inclusion. Root epoch invalidates all native
views, then request epoch prevents replay under a fresh view. MAX retires
permanently; resetting the epoch would resurrect old values. C callers can
forge the public view's fields if the cooperative premise is removed; a private
typedef or nonsecret generation is not a substitute for process isolation.

Delayed required change is implemented by nested read-only subviews and separate
read/write rights. An additional independent analysis transfer examines a trusted
principal-to-parent table; evaluate its authentication, lifetime and revocation
premises independently. Do not count that learner attempt as an author-executed
network/authentication service. No such service is supplied or required for the
bounded original mechanism.

Readiness is evidence-backed author support only. Preserve the original rubric
IDs, assessment delay and exact reference access gaps. External reviewers and
unaided learners have not been observed by these campaigns.
