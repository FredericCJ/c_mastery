# Command authority and bounded recovery, version 1

This executable successor deepens all eighteen leaf criteria of PR-T35-01
(T35.01–.04) and PR-T35-03 (T35.06/.10), including their exact `AX-…-DELAYED`
identities. Read the unchanged project supplements and
`curriculum/v5/expansion/legacy_assessments/shared-contract.md` for the four
deliverables, objective rubrics, seven-day delayed assessment and evidence rules.
This contract supplies the concrete mechanism, environment and independent
experimental obligations. Historical hours and acceptance remain unchanged.

## Prerequisites and environment

Use the learner's parent decoder/quota artifacts and fixture predictions; require
integer conversion, lifetime, checked extents and exclusive ownership reasoning
before implementing the extensions. PR-T35-03 consumes the learner's decoder
through `command.h`. The implementation is C17, profile B for PR-T35-01 and C for
PR-T35-03. The component uses fixed caller storage, typed assignments and loops;
it does not call a parser, quota, recovery or ownership library. Host tests may
use libc and Python; the separate process comparison uses Linux/POSIX fork,
pipe and wait. That adapter is not a freestanding dependency.

Use GCC and Clang debug, optimized and address/undefined-sanitized builds. Record
versions, flags, ABI widths and observed `sizeof` values. The host differential
driver explicitly requires 32-bit `unsigned`; the component's terminal generation
is `UINT_MAX` on its actual implementation. N1570 supplies inspected C11 draft
clauses, not exact published C17/C23. Do not infer another edition's guarantees
from a successful compiler mode. No compiler helper or freestanding closure is
claimed by these hosted runs.

## Representation and common rules

All control and output objects are live, correctly aligned and distinct from
input and each other. Input storage stays readable and unmodified during a call.
The caller serializes every operation on authority, service and recovery state;
ordinary fields are not atomics. Control objects may be initialized only before
use or after all possible users and callbacks have ceased. A stale ticket is
safe to reject only while its service object is still alive. Arbitrary corrupted
pointers, concurrent ordinary writes, longjmp and callback reentry are outside
the admitted interface. Required null arguments return `SC_INVALID` except that
`sc_service_init` requires a nonnull live destination. Empty decoder input may
use null storage but is a length error. Initialize the recovery object to zero.

`struct sc_batch` owns up to four commands, each with at most four `uint16_t` values.
No decoded pointer borrows packet storage. `struct sc_authority` has a nonzero generation
and an `allowed` bit owned by the trusted policy controller, not by the packet.
Generation `UINT_MAX` is terminal. The public C fields are an inspectable
representation, not unforgeable capabilities: arbitrary same-process writes can
forge them. The effect operation rechecks shape and current authority, but cannot
authenticate those ordinary fields against an attacker who may rewrite both.

## Problem: syntax, admission and effects

Implement `sc_decode(bytes,length,authority,extended,batch,meter)`. Base grammar is
`opcode,count,count × u16be`; opcode1 is ordinary, opcode2 privileged, count0–4,
exact extent `2+2*count`. No trailing bytes. The original fixture remains exact:
`01 02 00 01 00 02` succeeds with values1,2; its four-byte prefix is a length
error; `02 00` without authority is denied; count5 is a count error; opcode3 is
unknown in the base grammar. Error precedence is short header, opcode, count,
exact extent, aggregate resource budget, then authorization.

The changed grammar is selected explicitly by `extended != 0`:

* Opcode3: `[3,children,(child_length,child_bytes)...]`, children0–4 and exact
  outer extent. At most two nested container levels; leaf commands may occur at
  depth2. Visit children in wire order. An empty container is an admitted no-op
  batch; the queue requires at least one command and therefore rejects it.
* Opcode4: `[4,count,claimed_role,optional_flag,values...,optional_u16]`.
  Flag0/1 determines the exact extent; other flags are invalid. Treat this as a
  privileged command. The claimed role grants no authority. Optional metadata
  consumes one work unit but changes neither authority nor effects.
* Opcode5: `[5,run,u16be]` expands one value `run` times. Its four wire bytes do
  not justify arbitrary expansion. Run0–4 only; reject larger runs before any
  decoded value work. It normalizes to an ordinary command.

All packets have a 32-byte admission bound, aggregate four-command capacity and
four work units. A leaf costs `max(1,count)` plus optional metadata. Container
headers consume bounded parsing work but no decoded work units. First copy at
most32 stable bytes, then validate the entire tree and reserve all capacities
before materializing values. Publish the complete batch only on success. Failure
preserves the prior batch byte-for-byte. `struct sc_meter` counts copied bytes, preflight
headers and materialized values; rejected packets have zero materialized values.
Invalid control arguments leave outputs untouched. A packet longer than32 sets
the meter to zero and returns length in base mode or budget in extended mode.
Prove the copy, depth and node bounds rather than calling a low header count a
bound on every instruction.

`sc_apply` checks the entire batch before effects. Ordinary values add to an
ordinary `uint32_t` counter; each privileged command adds one to a separate
synthetic counter. Reject shape/work inconsistency and arithmetic overflow; both
counters remain unchanged on failure. Privileged commands must carry the live
allowed nonterminal generation. `sc_revoke` clears permission and increments a
nonterminal generation; at the terminal generation it remains denied and returns
exhausted. No wrap/reuse. Same-owner validation and effect are one logical
operation; moving them to different threads requires a separately proved protocol.

Implement `sc_decimal` without strtol: ASCII `0` or `[1-9][0-9]*`, value0–4,
length1–32, no sign/space/leading zero. Reject before multiplication can overflow.
In the changed policy leading zeros are admitted and canonicalized to exactly one
digit plus NUL. Preserve both outputs on failure. Implement `sc_extent` with
`offset <= limit` and, for nonzero width, `count <= (limit-offset)/width` before
forming the product; width0 gives an empty extent at offset. These contracts do
not permit converting a negative count into a huge size and hoping it fits.

## Problem: retained work, cancellation and recovery

`struct sc_service` stores three owned request copies, total work at most6, and one
separate pending control operation. `sc_submit` admits valid nonempty work1–4
only when total capacity, a nonretired slot and future terminal-counter headroom
exist. Rejection leaves the service and output ticket unchanged. Costs2,3,5,1
admit true,true,false,true; cost5 is an invalid per-request shape. A valid request
which exceeds remaining aggregate budget is `SC_BUDGET`; exhausted slot capacity
is `SC_FULL`. A ticket contains slot and nonwrapping generation, not a pointer
to caller storage. Each successful submission reserves one future terminal and
release-counter increment so cleanup cannot later fail from those counters.

`sc_finish` consumes a valid ticket once: cancellation has no effects; completion
attempts the authority-checked effect. Both success and effect rejection release
the copied request and its budget once. A subsequent finish is stale, not a
second terminal event. Increment the slot generation, retiring at `UINT_MAX`.
`sc_retry` admits two retries per live job; exhausted retries retain the job until
the owner explicitly cancels or completes it. `sc_shutdown` closes admission,
cancels all remaining jobs without effects, consumes pending control and is
idempotent. Later callbacks with old tickets are stale while the service remains
live. Shutdown is not proof that an external callback has stopped accessing it.

`sc_control` admits the reserved control even when data work is full; a second
pending control is full. `sc_control_complete` consumes it, with stale status
when none is pending. A closed service admits neither data nor control. Control
reservation is not a scheduler fairness or latency guarantee.

`struct sc_recovery` states are RUNNING0, RECOVERING1, SAFE_STOP2. Each fault releases the
previously owned prefix of two synthetic resources in reverse order, then may
start one of at most two lifetime recovery attempts. Acquire resources0 then1;
failed acquisition owns nothing new, and the component rolls back earlier
successes. A failed attempt remains RECOVERING. A successful attempt returns
RUNNING with both resources owned. The next fault after two attempts releases
them before entering permanent SAFE_STOP. Repeated faults cannot restart it.
Callbacks must return normally, not reenter/mutate the state, and honor success
ownership; they may not retain released resources. Logger failure increments a
saturating fallback counter without recursive logging or retry. It is a synthetic
failure signal, not a guarantee about any real logging backend.

## Required evidence and criterion mapping

For each row retain D1 prediction/derivation, D2 implementation or explicit
analysis, D3 independent boundary evidence and D4 its original delayed task.
The exact AX ID is `AX-PR-T35-01-<objective>` for .01–.04 and
`AX-PR-T35-03-<objective>` for .06/.10; append `-DELAYED` for transfer. Do not
replace the original task with the shorter table description.

| Objective | Necessary ordinary discriminator | Changed-condition discriminator |
|---|---|---|
| T35.01.a | Asset/input/trusted-authority ledger; unauthorized valid `02 00` | Claimed role255 still denied |
| T35.01.b | Same truncated bytes classify identically whether accidental or hostile | Mutate accepted public count/work before effect; recheck, distinguish arbitrary pointer corruption |
| T35.01.c | Separate confidentiality/integrity/availability claims and shared-address-space writes | Real child private-memory write versus same-process write; inherited descriptors remain authority |
| T35.02.a | Output preservation, truncation, source reuse and owned values | Return a decoded copy after local input lifetime ends; no dangling borrow execution |
| T35.02.b | Derive eight-bit extent0 versus required256, without invalid write | Symbolic `count > SIZE_MAX/2`, checked native extent and no huge allocation |
| T35.02.c | Tagged opcode/count/value ownership and all shape checks | Forge a structurally valid public authority generation; explain residual same-process power |
| T35.03.a | Exact opcode/count/extent grammar and depth bound | Optional metadata changes extent/work, never authority |
| T35.03.b | Original flawed decoder increments only a synthetic counter | Revoke between decode and apply; no privileged effect and no wrapped identity |
| T35.03.c | Preflight the entire request before materialized values | Four-byte compressed run255 rejected with zero decoded work |
| T35.04.a | Negative conversion, count256 narrowing, checked product and bounded `%n` marker | `%hn` needs short-int pointer and `%ln` long-int pointer; wrong-type case is analysis-only |
| T35.04.b | Fixed-format repair prints literal `%n`, marker unchanged | Capacities1,2,3 distinguish truncation from complete output |
| T35.04.c | Reject signs, spaces, ambiguity, empty/huge digits | Leading-zero admission plus canonical one-digit output |
| T35.06.a | Work/retention/slots/retries/depth each has units and release point | Decoded expansion budget independent of wire length |
| T35.06.b | Replay2,3,5,1, no publication on rejection | Reserved control accepted while data capacity full; explicit consume |
| T35.06.c | 1024 bounded bursts and worst-valid request work | Eight colliding teaching keys yield worst lookup8, not average1 |
| T35.10.a | Gen1 validation then revocation rejects effect | Terminal generation never recycled; immutable request copy alone does not freeze external policy |
| T35.10.b | Five faults, two attempts, reverse prefix rollback before safe stop | Logger failures use bounded fallback and cannot restart recovery |
| T35.10.c | Both cancel/complete orders, revocation and retry/shutdown orders | Late callback cannot justify freeing a service before real quiescence |

## Verification, rubric and hints

Build the learner source with the public tests. Cases are `base`, `authority`,
`extensions`, `arithmetic` and separate service cases `quota`, `schedules`,
`recovery`. `oracle_driver.c` is a host-only framed test adapter. Author validation
uses an independently written Python grammar and structured mutation corpus,
semantic mutations, raw logs and exact source hashes. Reproduce the original
fixture predictions independently before comparing results. Report logical work,
fixed storage (`sizeof` on the actual ABI), recursion depth and maximum retained
requests; do not equate operation counts with measured worst-case execution time.

Every original criterion requires its specific level2 result, causal derivation
and an independently chosen discriminating input. Level1 repeats a supplied
output without explaining the trust/ownership/budget boundary. Level0 includes
wrong authorization, unchanged-state violations or a false safety claim. Level3
requires the original delayed changed problem after at least seven days, new
predictions and evidence under changed assumptions. Author runs are not that
learner evidence. A finite test pass is not absolute mastery.

Hint1: mark the last point before input can influence an externally visible
effect. Hint2: distinguish a byte bound, decoded-work bound and retained-resource
bound. Hint3: draw the owned request and live policy as separate objects, then
place revocation, cancellation and destruction on that diagram. For formatting,
write the exact variadic pointer type before evaluating the conversion. For
recovery, account for each acquired prefix on every exit before adding logging.

## Security scope and validation limits

Targets are only these original curriculum programs with synthetic counters,
owned temporary child processes and local test inputs. The packet attacker
controls bytes, role claims, timing and repetitions, not trusted policy storage.
The intentionally flawed authority target accepts only literal opcode2/count0
and increments its provided counter; its success condition is that unauthorized
increment. The original format target receives literal `%n` and a live `int *`:
its harmless success condition is marker77 becoming0. The repaired target prints
literal text and preserves77. Type mismatches, invalid lifetime dereferences and
arbitrary-code-execution claims are excluded from these demonstrations. Analyze
their prerequisites without executing unrelated undefined behavior.

No network listener, credentials, privilege change, external target, host policy
change or firmware action is involved. Sanitizers observe selected executed
paths, not complete memory safety. The service explores serialized logical
orders; it does not establish native-thread race freedom, scheduler fairness,
real transport overload resistance or physical recovery. Fork isolates the
tested private-memory writes, while inherited file descriptions and other OS
authorities require separate controls. Published C17/C23, independent review,
learner transfer and physical observations remain explicit gaps. Current author
execution is recorded separately in the source-bound successor report, never
retroactively in historical receipts.
