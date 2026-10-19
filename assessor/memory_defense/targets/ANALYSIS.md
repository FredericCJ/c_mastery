# T63 extension assessor analysis

Generation is an identity check only while the control object remains live and
generations never repeat. The rejected event still points to valid control
storage in this model. Freeing the control object first and then checking its
generation would itself violate lifetime. Changing owner prevents a current
generation from granting another principal access. The stale-generation mutant
uses the same principal so authorization alone cannot hide its defect.

The log format encodes NUL and terminal controls without truncating binary input.
Prechecking division avoids wrapped4*n admission. It deliberately spends four
bytes per input byte; changing to a context-sensitive encoding changes the proof.
The destruction mutant closes too late: the reentrant callback sees state1 even
though pointers were detached. Reentrancy is a control-state contract, not merely
double-free avoidance. Reverse-order destruction mirrors acquisition dependency.

CFI accepts all same-signature callbacks including admin; it prevents only the
selected type mismatch. The authority repair checks operation/grant before
dispatch. Withheld variant gives the event a borrowed payload that is destroyed
on cancellation; a generation check cannot make dereferencing the expired payload
valid. Require either owned copying, retained ownership or quiescence. Another
variant permits the destroy callback to free the control block; implementation
must avoid all later control accesses or strengthen the API precondition.

The three-entry continuation isolates four properties. Bounds ensure an existing
entry; read-only mapping prevents modification of the array; CFI checks target
type; the request/grant comparison supplies the authorization policy. None of the
first three implies the fourth. The controlled index changes only selection,
while trusted request remains0, so admin and clear are both meaningful unauthorized
same-type operations. A repair that merely constrains index<=2 still fails both.
The mapping observation is selected Linux linker/loader evidence and must be
revisited on another ABI or linker. C const is a language access restriction,
not proof of that mapping. The specimen never attempts an incompatible call
without CFI or an out-of-bounds array access.
