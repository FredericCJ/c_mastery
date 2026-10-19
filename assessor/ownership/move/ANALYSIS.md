# T66.03 relocation and authority analysis

Byte copying an inline owner preserves a pointer into the old object's array.
Move must copy the bytes, point data at the destination array and rebind self,
then empty source. The heap case transfers the same allocated base/allocator.
All admission occurs before either owner changes; valid callbacks cannot fail
release. The self pointer detects an ordinary raw copy only under the authentic
control premise. Rewriting public fields or retaining raw aliases defeats a
claim of comprehensive memory safety.

The opaque registry separates object representation from copyable token values.
Logical transfer rotates generation without relocating the private payload.
That invalidates all old copies, while consuming MAX remains legal and retires
the slot. Allocate/initialize before generation publication, so failed allocation
does not burn identity or publish ownership. Registry destruction requires every
value consumed and all caller references quiescent; a token generation cannot
make a freed registry pointer safe to inspect.

The original local duplicate-holder demonstration deliberately succeeds: the
copied holder acts first and obtains new authority, while the old token becomes
stale. Calling this a failure of holder authentication is correct; claiming that
secrecy of fields authenticates the original holder is not. Repeated consumption
is still prevented through the checked API. Same-address-space malicious memory
access is outside this wrapper's containment claim.

Warn-unused-result probes execute compiler diagnostics, not a linearity proof.
Plain struct copy remains a valid C translation and plain return values can be
discarded. The C23 draft's recommended nodiscard diagnostics and explicit void
cast exception remain separate from actual GCC/Clang observations. No duplicated
native allocation is consumed twice to make a dramatic failure demonstration.

Assess every original criterion: ordinary real OwOwner transfer and source
poisoning; invalid transitions and allocation counts; actual diagnostic outcomes;
inline-pointer repair; and changed opaque token authority with finite exhaustion.
The integer oracle supports bounded generations, not arbitrary C pointer validity,
weak memory or independent learner mastery. A copied token is not an additional
owned reference unless a distinct retain protocol is explicitly implemented.
