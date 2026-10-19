# VM storage boundaries and a compensated two-step workflow

## Prerequisites and exact component reuse

This packet supports AX-PR-T47-07-T47.07.b-DELAYED and the ordinary T47.12.a,
.b,.c criteria plus .a-DELAYED and .c-DELAYED. It does not supply asynchronous
queued-I/O completion for .b-DELAYED; that requires its separate actual adapter.
Complete PR-T47-01..11 and PR-T46-01..10 before the VM stage. The kernel stage
additionally requires completed PR-T48-01..13 and its selected guest profile.
The two-step workflow follows the receipt protocol, not a reverse prerequisite
from the foundation store to the later kernel or modern distributed curriculum.

Use actual `cvm.h`/CVM1, `store.h`/CST2, `store_host.h` and the Vh service
implementation described in `labs/runtime_followthrough/services/CONTRACTS.md`.
Vh LEGACY4 calls the original fixed-key four-byte integer adapter; VERSIONED5
supports checked versioned bytes and an atomic latest-operation receipt. Bring
completed learner implementations explicitly. CMake's assessor opt-in selects
reference providers only for author assessment; no private source is exported.
The new implementation is `workflow.c`; public stubs remain unfinished.

Hosted tests use ISO C17, GCC/Clang debug, optimized and ASan/UBSan profiles.
The original provider's separately bound report contains actual M4 and PPC64
execution; this new workflow does not inherit target execution merely by
linking the same C sources. Host verifier Python is not a target dependency.

## Budget, capacity, encoding and service authority

Execute real bytecode, not a callback invoked in isolation. Instruction/host
budgets, permitted service IDs, store capacity and commit outcomes are distinct.
The supplied capacity investigation fills the actual tree with ordered
`a000000`... keys until the next insertion returns STORE_FULL, aborts that
attempt and checkpoints. In the observed selected tree this retains103 keys.
Then the same fixed-key assignment is attempted with host budget0, enabled but
unknown service31, and valid service1 with budget1. Independently require:

- Zero host budget returns CVM_HOST_BUDGET without calling the host.
- Unknown service returns CVM_HOST_ERROR with captured STORE_INVALID and no I/O.
- Valid assignment returns CVM_HOST_ERROR with captured STORE_FULL, preserves
  result91, leaves no active/poisoned transaction and retains every original key.

The host status is deliberately observed separately: CVM_HOST_ERROR alone
cannot distinguish capacity, policy and uncertain persistence. The actual Vh
provider tests also cover absent callback, wrong tag, services255/31, denied
capabilities, signed extrema, wrong format lengths/versions, migration and
lost reply. No guest integer is a native path or pointer. The mask is embedding
policy, not memory isolation from hostile native code.

For the nonidempotent service, atomically commit `vm-result` and `vm-operation`
receipt through the actual store. The native client retains a sequential nonzero
ID. Repeating the latest ID and delta returns its stored historical result with
no second increment; older/gapped/conflicting IDs refuse. Reopen after uncertain
publication and reconcile with that identity. The original provider's74 cuts,
648 independent request histories and compiled controls remain separately bound
evidence; do not relabel them as new physical measurements.

## Actual kernel boundary

The frozen synchronous `service_user.c` adapter maps logical0..66 to physical
16..82 with actual `kf_slice`, then invokes CMK whole-page read/write/flush
syscalls. The syscall validates its process handle, complete user extent and
read-destination write permission before submitting a virtio request. It copies
4096 bytes through owned bounce storage and checks completion/status before
returning success. A successful write is not a flush. The selected kernel
rejects unsupported changed sector geometry rather than guessing a conversion.

Required replay uses the same private disposable image across two fresh guests.
Case33 commits assignment42 and loses its reply;34 commits41+1 with an atomic
receipt and loses its reply;35 refuses before the increment. The next guest
reads42 or41 as appropriate and retries the same ID, ending at42 without a
second increment. A read-only client must be refused before store counters
change, and the whole file outside [16*4096,83*4096) remains untouched. Separate
case0 checks invalid destination before submission and continued handle use;
case31 writes complete patterns at raw pages0/82/1023, flushes and reboots.
Case32 without flush is explicitly only a clean-restart comparison.

The bound guest logs/images/source identities support exactly those observations.
They are QEMU TCG on an ordinary local image, with networking disabled and no
host disk passthrough, not physical power-loss or native-hardware validation.
Learners supply their kernel implementation and use the public guest builder;
the assessment packet does not silently copy private kernel source.

## New workflow representation and ownership

Implement `workflow.h`. Use three independent exclusive CST2 images: coordinator
journal, debit participant, credit participant. Each participant is the actual
Vh VERSIONED5 service with read/increment authority; initial values are10 and20,
and neither has an existing operation receipt. Only this coordinator may mutate
them until completion. Distinct C pointers are checked, but distinct underlying
images and exclusive ownership remain caller obligations. All opened stores
are inactive and live; native configuration is stable and callbacks cannot
reenter or retain borrowed buffers. Constructors use fresh control storage.

There is one workflow per image set, transferring a positive signed32 amount;
the assessment uses1,4,10. These are synthetic integer counters, not a bank
account/overdraft policy. A durable decline flag records a policy decision
before any credit submission. The journal key is `workflow` (8 bytes), value7
bytes: version1,phase,decline,positive amountLE32. Phases1..5 are PREPARED,
DEBITED,COMPENSATING,FINISHED,REVERSED. COMPENSATING/REVERSED require decline1;
FINISHED requires decline0. Unknown version/length/phase/amount refuses.

Create verifies the journal/participant receipt namespace is fresh and commits
PREPARED before any effect. Each `sw_step` performs the next action then commits
its phase. PREPARED calls debit ID1 with `-amount`, then records DEBITED.
DEBITED either calls credit ID1 with `amount` and records FINISHED, or records
COMPENSATING without submitting credit. COMPENSATING calls debit ID2 with
`amount` and records REVERSED. Compensation uses a new identity because it is
a new effect; reusing debit ID1 conflicts with its original payload.

If an effect persists but the phase update is lost, reopen all three images,
reload the intent and retry the same participant ID. Its receipt prevents a
second effect. A journal or participant failure poisons the workflow and returns
SW_RECOVER; do not use it until all affected stores are reopened. Uncertainty
from credit must never select compensation: credit may already have persisted.
An unexpected permanent participant refusal remains a reconciliation/policy
problem; this bounded coordinator does not invent an automatic alternative.
Terminal step calls are idempotent. Create/open failures preserve the caller's
control; step failure retains diagnostic `last_store_result` and poison.

## Failure workload, costs and falsification

For amount4 expect completion at6/24, or a durable decline followed by
compensation at10/20. Observe all actual write/flush boundaries of every
participant and journal stage; inject no-pending versus all-pending persistence
at each (962 histories). Reopen all images and finish through the same API.
Also cut intent creation at37 events under both policies (74 cases), preserving
the control sentinel and10/20 because no effect precedes the intent. A dedicated
persisted-credit/lost-reply case must resume with decline0 and reach6/24.

The independent Python oracle derives15 phase/balance/intent records across
six workflows, including reopen after every step. It is separate from the C
cut harness. Four compiled controls publish DEBITED before the effect, reuse
ID1 for compensation, compensate an uncertain credit, or ignore failed phase
publication. Require a named semantic assertion after successful compilation.
Public stubs must compile and fail the relevant mechanism assertions.

Retained CST2 commit writes32 afterimages plus marker traffic and three flushes;
its pending checkpoint costs additional work before later transactions. Count
journal and participant work separately. Three image owners and fixed metadata
bound memory; this protocol is not an atomic transaction across all images.
Observers may see the debit before credit or compensation. Compensation can
itself fail and require recovery. It cannot undo an arbitrary external action.

## Deliverables and assessment

D1: derive refusal precedence, complete-byte encoding, commit versus reply
authority and the compensation state machine. D2: implement the exact workflow
and compose completed VM/store/kernel providers through their public APIs.
D3: retain independent expected traces, source-bound campaign/guest receipts,
all failed attempts, mutations, output preservation and exact namespace checks.
D4: compare same-ID recovery with blind retry; explain the limits of latest
receipts, isolated images, fixed client ordering and physical-device evidence.

Superficial work reports only the nominal final balance. Satisfactory work
derives and passes the required refusal, cut and retry cases. Independent
assessment changes the step order or introduces an irreversible external effect
and requires a new argument, not a claim that compensation is rollback. Hints:
first draw effect and reply separately; next persist the decision before credit
admission; finally cut between participant commit and journal phase commit.
Assessor source/analysis remains private. Actual delayed attempts, independent
review and physical observations are separate from author support.

All targets are original local programs or disposable curriculum guests with
synthetic values. No host security settings, operational devices or external
service are changed. Finite checks do not prove absolute mastery or universal
durability.
