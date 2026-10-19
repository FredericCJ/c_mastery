# Assessor analysis: queue ownership and lifecycle followthrough

This is reference material, excluded from learner export. Retain the original
six AX criterion IDs and delayed learner gate. Actual campaign receipts, not
this prose, determine which profiles ran.

## Queue and transfers

The acceptance linearization point is the serialized append after all checks.
A full send never owns an input. Copying the supplied live array then changing
the caller byte distinguishes the promised snapshot from the original pointer
specimen without invoking lifetime undefined behavior. Capacity rejection is
before all writes and head advancement. Zero length consumes one slot but no
byte object. Closing preserves accepted objects; abort turns only queued count
into discarded. The counter identity follows by induction over those three
state changes. A duplicate-on-pop mutant repeats a sequence and violates the
counter identity; a missing-copy mutant is independently distinguishable by
payload. The model's list and Python integers are independent of C ring-index
arithmetic, but its alphabet and trace depth are finite.

Pool descriptors separate queue slot availability, allocation descriptor
availability, total byte availability, and contiguous extent availability.
Alternating four-byte holes expose the last distinction. Transfer changes one
owner field after queue-space checks, so failed enqueue is rollback by no
mutation. Abort uses dequeue into consumer ownership followed by exactly one
release. Received buffers remain the consumer's obligation after queue close.
The generation ceiling permanently retires a descriptor. Forged/copy handles
are checked against a live registry, not capabilities enforced by ISO C.

Mutants: CL1 omits the snapshot, CL2 does not dequeue, CL7 overwrites full
storage, CL3 permits queue-owned release, CL12 confuses fragmentation with full.
The separate pointer-copy specimen uses live caller storage and intentionally
returns its changed byte. Do not count a crash or sanitizer complaint as its
required semantic result.

## Real wait enrollment and application receipts

One mutex binds predicates, queue mutation and both conditions. A successful
wait does not imply available space. The cleanup frame exists before waiting;
POSIX deferred cancellation reacquires the mutex before executing it. The
public test waits on the adapter's enrollment condition, then holds that mutex
while canceling to establish that completion cannot yet have occurred. Joining
establishes completion. An early destructor cannot discover an unregistered
concurrent accessor; admission closure and joins are caller preconditions.
The initial Clang release campaign exposed cleanup state that was modified
after a setjmp-based pthread cleanup frame and lost on cancellation. The repair
uses a volatile automatic cleanup record; this is an unwind preservation
requirement, not a replacement for the queue mutex or a general thread-safety
property of volatile. Initial failed evidence is retained separately.
CLP1 mistakes a wake for send completion; CLP2 leaks active-operation duty;
CLP3 bypasses the destructor guard. Threaded trace order comes from tickets
assigned under the queue mutex, not post-return logging order. Exactly 1,500
deliveries and each producer's 0..499 sequence must agree independently.

The acknowledgement adapter is an application state machine, not queue magic.
PENDING reserves identity/payload; applying once stores the result under the
same exclusion; APPLIED retry returns it. Terminal overflow retains RANGE so
failure retry cannot occupy a silently indeterminate state. Explicit client
acknowledgement releases storage but highwater rejects old retry. CL6 applies
the same effect on retry; CL11 neglects payload binding. Volatile state loss
breaks this guarantee. Durable storage composition is a separate curriculum
mechanism and cannot be inferred from this table.

## Teardown, restart and recovery

Lease identity is object generation, slot and slot generation. Close wakes only
queue wait classes. Condition reacquisition and callback join remain distinct
obligations. Every checked operation uses the stable registry before inspecting
slot state; no freed-pointer access is justified by a generation comparison.
CL4 destroys while a lease exists; CL9 wrongly completes condition reacquisition
at close. Three-object dependency teardown releases consumer-held provider
leases before provider destruction. Group restart validates every member's
phase/counter first, then proceeds in reverse dependency order. A failed
precheck leaves all objects destroyed; it does not expose a partial service.

The owner-death model requires a trusted death event. Ordinary exit-refusal in
CMX is useful prevention but does not implement this event. A trusted committed
shadow supplies the rollback value; a checksum cannot establish transaction
intent. CL5 computes a valid checksum for the wrong value, demonstrating that
distinction. CL10 exposes a dirty record to ordinary readers. Repair exclusion
and the two-attempt budget prevent an unlimited chain of failed repairers from
being reported as bounded recovery; they do not guarantee a live supervisor.

The Linux comparison executes actual thread exits while owning a robust mutex,
not a modeled flag. EOWNERDEAD grants ownership and reports inconsistent state.
CLP4 blesses the torn value instead of restoring the shadow. CLP5 omits the
consistent call: the first return may look successful but a later lock is
unrecoverable. Tests must check the second supervisor/lock. Repairer death
before consistency produces another owner-death event on the observed Linux
implementation. POSIX does not impose the model's repair-count limit.

No suite establishes full first-principles MCU scheduler integration, a memory
model proof, target IRQ timing or learner independence. Target object/link
closure and any emulator execution are separately labeled. Preserve all earlier
accepted CMX source bytes and receipts when binding this successor.
