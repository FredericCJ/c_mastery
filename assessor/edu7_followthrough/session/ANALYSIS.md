# Withheld two-peer session analysis

<a id="t4006a"></a>
## T40.06.a

The key witness contains two separate EsPeer/E7Session objects. After A queues
TOKEN, A's pending flag blocks DATA. After B accepts its completed L4 message,
B alone can send. Losing its ACK does not return A's permission. B's response
can complete at L4 while A must retain the SDU because A has not resolved its
pending authority. The retry's exact old fragment produces a duplicate ACK,
not another TOKEN event; A then finalizes and can process the held response.
A stale completion ID/transport incarnation cannot release the pending state.
An incompatible OPEN receives an explicit identity-bound seven-byte ERROR; its
receiver marks uncertainty without acquiring permission.
The cancellation variant keeps uncertainty sticky; even eventual completion
is not permission to resurrect the old owner. EARLYTOKEN and CANCEL each produce
a real two-permission counterexample, while NEGOTIATE mistakes transport receipt
for compatible L5 parameters. Distinguish a public epoch from authenticated identity.

<a id="t4006b"></a>
## T40.06.b

A request checkpoint1 is recorded in B.receipt[0]; B response checkpoint1 is
recorded in A.receipt[1]. Neither modifies the opposite direction. The ordinary
SET request9/value2500 yields exact TLV response
0101010302020009030209c4040101. Python derives these bytes independently using
big-endian scalar packing. Full output capacity is checked before publishing
any staged application or receipt. A capacity1 call leaves entire peer and event
unchanged, including the held complete receive message. Valid retry commits once.
The sender API admits an exact replay of its copied last acknowledged payload
under a fresh L4 ID. An identical later request at checkpoint1 replays the cached response; altered
bytes reject. Malformed checkpoint2 rejects without committing or leaving a
pending token. EARLYCOMMIT, CAPACITY, DIRECTION and DUPLICATE distinguish four
separate misconceptions. A sent checkpoint records L4 completion, not remote
application acceptance; retain that limitation when reasoning about RESUME.

<a id="t4006c"></a>
## T40.06.c

Both endpoints call actual greater-incarnation transport-open methods only after
quiescence. L5 retains its epoch1 receipt and accepts RESUME checkpoint1. A stale
L5 epoch refuses even in the new valid L4 namespace. A fresh peer without RAM
receipts refuses. Closing ends authority but retains epoch retirement; reopening
at epoch2 clears receipts. EPOCH and REBOOT must fail for the intended assertion.

The intentionally flawed cut_effect uses actual E7 prepare and actual decoding/
application work, increments an independently retained synthetic pulse, then loses
all volatile state before commit. Repeating the request increments it twice.
No hardware pulse or filesystem durability was executed. A correct durable
external operation needs a shared atomic transaction, or durable idempotent
identity with recovery at the actuator boundary. Merely persisting afterward
has the same cut; persisting before can suppress an effect that never happened.
This is the required changed-condition analysis, not a claim of a supplied
production persistence mechanism.

## Assessor practice and limits

Keep implementations and this answer sheet out of learner packages. Require
unaided traces and the original seven-day changed artifacts. Author compiler,
model and target runs are not independent assessment or electrical validation.
The selected serialized fixed-storage contract has no concurrent-callback proof,
no cryptographic peer authentication and no durable exactly-once guarantee.
