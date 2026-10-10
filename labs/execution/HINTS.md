# Staged hints

Use tier 1 after recording a prediction and failing trace. Use tier 2 only after attempting the invariant. Tier 3 describes a debugging experiment; assessor code remains separate.

| Mechanism | Tier 1: question | Tier 2: invariant | Tier 3: experiment |
|---|---|---|---|
| Admission/context | Which memory must survive a switch? | private aligned stack, initial hardware frame plus software registers | single-step SVC exception return, then alternate two sentinels through PendSV |
| Enrollment | What if signal arrives between check and sleep? | check, enroll and state change share serialization | signal before wait and after enrollment; neither loses the event |
| Timers | Which operation wins at exactly the deadline? | one serialized terminal transition; half-range clock contract | start near UINT32_MAX and permute tick/signal/cancel ordering |
| PI | Can a donated owner itself be waiting? | effective priority reaches a fixed point through all mutex edges | chain three tasks, then remove the highest donor by timeout |
| Condition | Does timeout mean the caller owns its mutex? | release/enroll atomic; every return reacquires | hold the mutex in another task while the condition expires |
| Queue | Who owns a blocked sender's item? | an accepted value is consumed once; close drains, abort discards | close a full channel with blocked sender, then repeat with abort |
| Barrier/once | What becomes of peers after failure? | break generation; once initializer remains unique | cancel one barrier waiter and fail one initializer |
| RW lock | What happens when the last writer leaves the queue? | readers need no exclusive owner and no pending writer | keep one reader active, cancel writer, observe pending readers progress |
| SPSC | When may the consumer read payload? | payload precedes release head; acquire head precedes read | reduce capacity to one and cross unsigned counter rollover |
| Hazard queue | Can a pointer be reclaimed before protection? | protect, validate, dereference; retirement is incarnation-specific | stall a hazard owner, run two collectors, force node reuse |
| Epoch/RCU | Does a quiet writer imply quiet readers? | reclaim only after all potentially observing readers leave | pin one participant across publication and attempt grace advancement |
| RT models | What costs did the equations omit? | each bound names arrivals, deadlines, overhead and blocking assumptions | inject blocking/jitter until a formerly admitted set misses |
| AMP | Does a doorbell count messages? | ring ownership is authoritative; IRQ notifications may coalesce | send bursts while delaying the receiver and compare exact sequences |

After each stage, mutate one assumption deliberately and show the expected failure. A passed finite trace supports the claim it exercises; write the remaining proof/measurement obligations separately.
