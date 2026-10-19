# Assessor analysis: authority, admission and terminal ownership

This file is withheld. The learner keeps the original eighteen objective rubrics
and delayed identities; the following argument distinguishes successful examples
from understanding. `command.c`, `service.c`, `models.py` and `differential.py`
are the separate original reference implementation and independent models.

## Authority and lifetime (.01a–.03c)

Syntax says that a request can be interpreted. It does not grant permission.
The supplied flawed target increments8 from7 for `[2,0]` without any trusted
permission. The repaired path must reject before publishing values or effects.
Role255 in opcode4 remains an untrusted byte. The two-pass parser first freezes
input into32 bytes and checks the entire request, then decodes; this makes
rejection's materialized-value count zero even when the final child is invalid.
The copy is not a defense against concurrent mutation during the copy: that is
excluded by the live stable-input precondition. Passing a user pointer of unknown
validity is not repaired by checking its length in ISO C.

For .01b, accident and attack with identical bytes have the same parser status;
intent changes the threat account, not syntax. The changed condition deliberately
alters public count/work after validation and the effect checker rejects. This
does not prove integrity under arbitrary same-process writes. The .02c control
forges structurally valid generation5 along with trusted state5; it succeeds and
must be reported as the boundary of an ordinary C representation. An answer that
calls the struct a capability protected by the language fails that criterion.

For .01c classify confidentiality separately: this target has no real secret and
therefore cannot demonstrate secret exfiltration. Integrity is the unauthorized
counter effect; availability is finite work/retention admission. The process
adapter actually writes99 through a same-process pointer, then forks from7: the
child reports99 while the parent's private object remains7. This is one private
mapping observation, not full sandboxing. Inherited descriptors still confer
authority and share open-file descriptions; an IPC service must validate messages.

For .02a the returned batch contains values, not a span into the ended local
array. Source reuse, local lifetime end and failure output preservation are
separate controls. Never demonstrate the incorrect borrow by dereferencing a
dangling pointer and claiming a reliable value oracle. The .02b eight-bit extent
is an intentional defined narrowing model:128*2 computes256 then conversion to
uint8_t yields0. No zero-sized allocation is overrun. The size_t variant is a
symbolic division bound plus actual boundary calls, not an enormous allocation.

The grammar's error order is observable. A truncated header precedes unknown
opcode; excessive count precedes exact extent; authority follows those and
aggregate work checks. Containers recurse in wire order, so an earlier child
error precedes a malformed later child. Two containers may nest; a third is
rejected before recursion. All output is staged. Optional metadata increases
extent and one work unit but not authorization. Compressed run255 in four bytes
fails before value expansion. A header count is not a measure of all loop work;
the separate input32, depth2, child4 and leaf-work4 bounds establish termination.

The effect layer checks all commands before updating either counter. Revocation
requires a new generation and denial; at UINT_MAX no fresh authority is issued.
Immutable accepted bytes do not make stale authorization current. Caller-owned
serialization prevents a change between the check and effect; atomics on the
generation alone would not make a compound policy/counter protocol safe.

## Arithmetic and formatting (.04a–.04c)

Minus one converted to size_t is SIZE_MAX; a count256 narrowed to uint8_t is0.
Neither conversion itself demonstrates an invalid access. Multiplication after
`count <= (limit-offset)/width`, with offset separately admitted, is justified in
mathematical integers then represented safely. Width0 is explicitly an empty
extent; a zero-width rule must not accidentally divide by zero. The extent mutant
uses an overly strict comparison, so the boundary11 discriminator rejects a
symptom-suppression repair that excludes valid inputs.

`%n` consumes `int *` and stores the number of output characters. Literal `%n`
with no preceding output writes0 to the supplied live marker. `%hn` requires
`short int *`, `%ln` requires `long int *`; mismatching those variadic arguments
is undefined behavior and is not executed. The fixed format `%s` treats `%n` as
ordinary data; snprintf reports2 even when capacities1 or2 truncate it. Compare
the nonnegative return against capacity, and never infer that a bounded write
means complete output. The tests preserve a neighboring guard byte but do not
equate that one sentinel with proof about all memory.

The decimal grammar is a policy choice rather than whatever strtol accepts.
Empty/sign/space/leading-zero forms fail in the ordinary policy; changed leading
zeros normalize to one digit. A 32-character maximum bounds scanning even when
all characters are zeros. No locale interpretation is introduced. Independent
cases must include equivalent numeric values with different admitted spellings.

## Quotas and recovery (.06a–.10c)

Four work units per request, six retained units, three jobs, two retries and two
container levels are different quantities. A fixed typed slot may contain unused
capacity while its logical work is charged conservatively; document both physical
sizeof storage and charged units. Costs2,3,5,1 admit TTFT and retain6. A cost5
request is structurally invalid, not merely rejected because the current pool
happens to be full. An independent reserved control slot remains available, but
without scheduling assumptions it has no promised response time. Its explicit
completion prevents a permanent accidental one-shot service.

Each admission reserves future terminal counter headroom for all outstanding
jobs. With one job and counters UINT_MAX-1, a second admission fails and shutdown
can still retire the existing job to UINT_MAX. A naïve terminal-overflow check at
finish would strand an already accepted resource. Slot generations retire rather
than wrap; generation exhaustion is distinct from queue fullness.

The ordinary workload executes1024 finite hostile bursts and returns retention
to zero each cycle. It is not an unbounded external flood. The .06c changed
analysis uses eight keys congruent1 modulo8; an inspectable list lookup takes1,
8,8 comparisons for first, last and absent key65. Average-case constant lookup
does not establish a worst-case adversarial bound. No claim about a production
hash function or benchmark is made by this small independently executed model.

Cancellation and completion are competing logical transitions under one owner.
The first consumes the ticket; the second is stale. Rejected authority still
produces a terminal outcome and releases its reservation. Both retry-before-close
and close-before-retry are inspected: shutdown releases once, blocks new jobs and
invalidates old tickets. A late callback is rejected while storage lives. If a
callback arrives after the owner has freed that storage, checking a ticket already
requires invalid memory access. The false assumption is that logical closure
proved external quiescence; repair requires joining/unregistering/draining that
producer, not a larger generation field. No real-thread quiescence is claimed.

Recovery owns a prefix of two resources. Each acquisition failure releases only
the earlier successful prefix in reverse order. Every new fault releases the old
owned pair before retrying. Exactly two recovery attempts are permitted, then
five consecutive injected faults finish in SAFE_STOP with no owned resources.
Logger failure increments bounded telemetry rather than recursively invoking the
failing logger. The fallback counter saturates; it does not describe durable
logging. A returned callback error must mean no new resource ownership, otherwise
the component cannot know how to roll it back.

## Withheld changed-condition checks and assessment

Use each original delayed ID, preserving seven-day separation. In addition to the
public example, choose a valid container whose last child has bad optional extent;
expect no published output and zero materialized values. Move revocation between
acceptance and terminal completion; require both no effect and one release. Set
two live jobs with only one terminal-count increment left in a deliberately
invalid control-state drawing; require the learner to identify why admission
must prevent that state rather than promise recovery from arbitrary corruption.
Reverse successful callback release order and ask which resource dependency can
fail. Give a borrowed public representation with a valid length but ended input
lifetime and require a nonexecuting lifetime proof or owned-copy repair.

Level2 requires the exact objective's expected result, independent discriminator,
causal argument and admitted assumptions. Level3 additionally transfers under
its stated changed condition without copying the worked reference. A model-only
schedule is not a native concurrency test. Six compiler profiles, finite
differential cases and mutation kills provide bounded author evidence only;
neither reviewer independence nor learner mastery follows. Preserve all failed
campaign attempts and current source/output identities when revising a defect.
