# T46.10 — authority, persistence and uncertain replies

This successor adds executable support to PR-T46-10 and its unchanged three
criterion-specific delayed variants. C17 remains the implementation baseline.
C23 does not make a callback a capability, make native pointers safe, or turn
an I/O failure into rollback. The format/transaction contracts below are original
curriculum contracts, not requirements of ISO C.

## Prerequisites and construction order

Bring your actual CVM1 interpreter, HOST budget/type enforcement, and T47 store
implementation. Read `labs/runtime/SPEC.md`, `labs/storage/FORMAT.md`,
`labs/storage/INTEGRATION.md`, and the supplied `store_host.c` adapter. First
predict permission and failure traces; then implement `learner.c` against
`services.h`; then run public tests; finally submit an independent request/crash
model and changed-format/retry argument. `run.py` requires explicit `--cvm`,
`--store`, `--adapter` providers and `--build`; `--source` selects your submission.
No assessor provider is a default. The medium is supplied infrastructure, not
an implementation of the service mechanism. Preserve intentional learner stubs.

## T46.10.a: an integer is not authority

Implement `vh_host` with service1 assignment, service2 read and service3 increment.
`Vh` borrows an opened store and stable configuration for each synchronous call.
Its32-bit capability mask enables bit N for service N; reject IDs>=32 before
shifting. IDs other than1/2/3 remain absent even if their bits are enabled.
A read-only VM and a write-only VM may share a database only through serialized
calls. Native code can forge a `Vh` or call the store directly: C provides no
capability unforgeability or automatic borrowing discipline. A service integer
never encodes a pointer, filesystem path or arbitrary native entry point.

The existing interpreter validates guest integer tags and charges its HOST
budget at its documented boundary. The adapter enforces the embedding policy
before touching the store. A denied call preserves the caller result sentinel91,
store event counters and contents. A permitted callback failure propagates as
CVM_HOST_ERROR, preserving the guest result but not promising absent persistence.
Absent callbacks, absent service31 with its bit enabled, service255, wrong tags,
zero budget and an already active transaction have distinct causes. A callback
may consume wall time or misbehave; T46.09 supplies the separate actual blocking
and pending experiments. This synchronous adapter does not promise cancellation.

Required artifacts: decision table, first-refusal traces, implementation, two-VM
capability tests, and an enforcement table distinguishing C types, runtime checks,
external verification and caller discipline. Compare a language-native capability
API: a type can narrow calls, but its real enforcement depends on inaccessible
constructors and the runtime boundary; reproducing its signature in C is insufficient.

## T46.10.b: portable bytes and explicit migration

LEGACY4 delegates the accepted adapter: key `vm-result` (9bytes), exactly four
little-endian bytes representing the modulo2^32 encoding of int32. Do not memcpy
an int32 representation into the format. Decode unsigned shifts then use a range-
safe arithmetic conversion; neither signed overflow nor out-of-range signed casts
are a portable codec. Test0,1,-1,INT32_MIN,INT32_MAX,-91,0x01020304, wrong length3,
active transaction refusal, restart and recovery. The store format is unchanged.

The changed service format VERSIONED5 is byte1 followed by four LE value bytes.
Reject every other version/length. With `allow_legacy=1`, reads may accept old
four-byte values. `vh_migrate` explicitly rewrites a valid old value transactionally;
repeating it on version1 performs no write. With allow_legacy0 the old value is
rejected. No implicit migration in a read, no reinterpretation of an unknown
version, and no claim that five bytes fit the old four-byte reader. Unknown future
versions need a specified successor. Explain how a language serialization package
would still require these compatibility and persistence rules.

Required artifacts: byte diagrams, codec derivation, mixed-version decision table,
implementation, migration/rejection tests, and an endian/data-model experiment.
The public target builder executes the same actual store/VM providers; host Python
oracles are not target dependencies.

## T46.10.c: a failed reply is not a failed effect

Use the supplied original `VhMedium`, which owns stable and pending67-page images.
A write stages a full page; a successful flush persists all pending pages in reverse
page order. A selected write/flush event returns failure and either loses pending
writes at restart or persists all of them before losing its reply. The model is
finite and does not claim arbitrary torn-write or real-device fault coverage.
The unchanged store commits an invalid marker+flush,32 redo images+flush,then a
commit marker+flush:37 modeled events. Inject failure before publication and lost
acknowledgment after persistence. Both guest calls return HOST_ERROR and preserve
result91. Reopen the poisoned store, recover/check it, and read the actual value
before deciding whether assignment needs retry. Never use a poisoned handle.

Changed condition: replace assignment with increment. Retrying an unknown effect
without a protocol can apply the delta twice. `vh_increment` uses one strictly
sequential, nonzero64-bit operation ID supplied by a serialized native client.
Store `vm-operation` (12-byte key) as17 bytes:version1,idLE64,deltaLE32,resultLE32.
Atomically commit new `vm-result` and this receipt in one store transaction.
If the latest ID and delta repeat, return its saved result without writes. Reject
older IDs, skipped IDs, changed payload for the same ID, id0, result overflow,
and incrementing past UINT64_MAX. A valid last-ID receipt permits only its exact
retry. The client must durably retain the logical ID across retries. Helpers
`vh_increment`/`vh_migrate` are privileged native APIs; only `vh_host` checks the
capability mask. All calls require exclusive store access; no read/modify/write
race or multi-client exactly-once guarantee is implied.

This receipt remembers only the latest accepted operation. If a later assignment
changes the value, a retry of the latest increment returns that increment's historical
result, not the subsequently assigned current value. If the client loses its ID,
if another client shares the sequence, or if a more recent increment displaced the
receipt, this protocol cannot reconcile an arbitrary old request. State those
limits and design an alternative retention policy before claiming more.

Required artifacts: transaction state diagram, ownership/poison rules, operation-ID
protocol, all74 failure-cut observations, independent functional request model,
recovery traces and cost accounting. Fixed bounded keys make codec work O(1), but
store lookup follows its tree; accepted commits write32 images and require three
flushes. Charge retries/recovery separately. A duplicate reads a receipt without
committing. RAM for the supplied two-image medium is substantial; do not silently
claim it fits the baseline64KiB Cortex-M profile.

## Execution profiles and evidence

Native GCC/Clang C17 O0/O3 plus ASan/UBSan are host-side checks. PPC64 ELFv2 big-endian
execution tests actual codec behavior. The separate **MPS2-AN386-SSRAM4M** emulation
profile uses4MiB at0x00000000 and4MiB at0x20000000 according to the inspected QEMU
v10.2.1 `hw/arm/mps2.c` AN386 mapping. It reserves256KiB for the generated-frame
bound and retains both medium snapshots; this is not the original64KiB VM target,
a board measurement, or evidence about a different QEMU version. Startup masks
interrupts, initializes .data/.bss and runs the complete public fixture. Link only
explicit original C/assembly objects; inspect every undefined object symbol and
the final image. No libc/libgcc/compiler-rt/libatomic archive is permitted. The
original tiny diagnostic formatter supports only literal text,%u,%s. New providers
require a renewed stack/closure audit, not an assumption based on this reference.

## Assessment and transfer

Pass each criterion only with the exact task, causal reasoning, implementation and
changed-condition evidence. Matching output alone earns at most superficial credit.
Submit pre/postconditions, resource/ownership table, raw commands/results, pinned
source/tool identities, and explanations of what the finite checks cannot establish.
The published tests are guided diagnostics. The assessor separately holds protocol
histories and compiling incorrect implementations. An independent attempt and
minimum-seven-day variant remain required learner evidence; author execution does
not supply either. Variants are two-VM capabilities(a), explicit format migration(b),
and increment reconciliation(c), each using the actual prerequisite components.

Graduated hints: (1) separate permission from service dispatch and from VM tag/fuel
rules; (2) draw the persistent bytes before writing a codec; (3) distinguish the
commit decision from the arrival of its reply; (4) put the receipt and effect in
the same transaction, then identify which requests your bounded receipt forgets.

## Local security scope and status

Only original curriculum VM/store programs and synthetic values on owned in-memory
images are involved. The attacker controls a guest service ID/tag/value; it cannot
supply a native pointer or modify the embedding mask. The harmless violation in
the ignore-mask specimen is a read-only guest changing synthetic vm-result. Repair
is mandatory early policy enforcement plus regression checks; it does not isolate
malicious native callbacks. No third-party target, credentials, host-security
change or board flashing is part of this exercise. Implementation and source-bound
execution status are recorded separately in the attached completion dispositions;
physical observations, external review and actual independent/delayed attempts
remain separate. No finite campaign proves absolute mastery.
