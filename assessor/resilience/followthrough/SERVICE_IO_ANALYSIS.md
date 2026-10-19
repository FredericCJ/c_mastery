# Local service I/O analysis

The input parser owns a fixed18-octet buffer. Length rejection occurs after two
octets; complete-field rejection occurs after18. No call consumes the next
coalesced frame. Output is transactional although parser progress is not rolled
back. A truncated frame poisons the connection-level parse; scanning ahead for
another apparent header would invent resynchronization absent from the protocol.
The oracle's grammar and prefix arithmetic are independent of the C state object.

Scope retention begins before read and gate ownership begins before the durable
effect. They end after the last synchronous send or terminal I/O return. Mutant102
releases the gate after commit but before write; a full socket send buffer gives
a concrete interval where the request still owns backend work but capacity has
already been granted back. The positive control holds the creator reference
until that interval, releases it, then confirms final destruction only after
cancel wake and thread join. A future true asynchronous backend would additionally
need its own explicit quiescence acknowledgment; return from a submit call would
not suffice.

The request scope's milliseconds convert exactly to the imported read deadline's
nanoseconds within a checked signed64 bound. The journal receives whole seconds
from the same origin. This is an explicit adapter, not an accidental comparison
of independent monotonic epochs. Reboot needs a new clock-origin protocol.
Synchronous storage cannot be forcibly undone when its caller deadline passes.
Therefore an effect may be committed with no delivered reply; retry identity
and durable notification recovery address that uncertainty.

Every admitted duplicate consumes the common work credit. Staging prevents a
gate refusal from burning it, while an admitted backend refusal still costs
work. The backend outcome is recorded independently of reply success. Cancellation
readiness is checked at poll boundaries and scope cancellation at admission;
neither revokes a commit already made. MSG_NOSIGNAL is local to each send and
avoids changing host-wide signal handling.

The retained deadline producer performs actual reads. The retained reactor sums
completion bytes from joined-to-be native workers; it is not a generic event
dispatcher. These named contracts are narrower than the new service adapter.
Native AF_UNIX traffic is real local IPC; the separate retained TCP fault model
must be integrated and assessed under its own virtual network assumptions.

Assess changed conditions using a slow reader, a cancellation wake after durable
commit, and another same-ID request. Require exact progress and ownership counts,
not a timing guess based on sleeps. The bounded wait in the test polls protected
state and fails if commit is not observed. Sanitizers cover the actual runs;
they neither prove all interleavings nor establish physical latency guarantees.
