# Required T68.05 qualifier-preserving device views

This supplement implements the original X-T68-005 R1-R4 and delayed transfer.
Complete X-T68-004; retain its unevaluated-versus-evaluated-expression argument,
typed call diagnostics and unchanged-output/effect distinction. Public
`receivers.h`, tests and this contract are supplied; implement `src/receivers.c`
and `src/receiver_macros.h`. The original `mp_generic_receivers` remains a real,
separately tested dependency with its const-policy/evaluation controls.

## Representation and admitted calls

Use ISO C17 hosted GCC/Clang, ordinary/optimized and address/undefined-behavior
sanitizer profiles. All objects are live, aligned, initialized and externally
serialized. Output storage is disjoint from the pointed word and callback
context. The pointers borrow and never retain storage. Null required word,
output or between-callback returns INVALID before any word access/callback;
context may be null only if the callback accepts it. Other invalid native
pointers are outside the contract. No allocation, locks or hidden device
library implements this mechanism.

`MPV_READ(pointer,out)` selects one of four actual function designators,
then calls it once. Pointees uint32, const uint32, volatile uint32 and const
volatile uint32 produce categories1,2,3,4 respectively, with the exact word
value. Categories are teaching provenance markers, not runtime type discovery.
The macro's controlling expression is unevaluated; its call argument is
evaluated once. Each admitted function reads once, then publishes value and
category. Null admission preserves output. Ordinary array-to-pointer
conversion is supported. Top-level const on a pointer is distinct from const
on its pointee; no default erased receiver is allowed.

`MPV_WRITE(pointer,allowed,value)` accepts only mutable or volatile mutable
uint32 pointers. Null is INVALID; any value bit outside allowed is RANGE
without a load or store. Otherwise store value once, including zero when
allowed is zero. This is whole-word replacement, not a preserved-bits update.
Const and const-volatile receivers, unrelated pointer types and mismatched
device-view structure types must receive translation diagnostics. An explicit
cast can hide a constraint, so a successful translation does not prove valid
qualification use. The cast-discard specimen is compiled only, never run on
an actually volatile-defined object through an unqualified lvalue.

`MPV_OBSERVE` accepts volatile and const-volatile pointers. After complete
admission, read once, invoke the supplied callback once, read again in a later
full expression, then publish `{before,after}`. The callback returns normally,
may change the word through a separate valid mutable-qualified alias, and
must not retain the pointer, mutate output, destroy/move storage or reenter
the operation. A const view of a mutable object does not make every alias
immutable. An actually const-defined object must never be modified. The
callback is explicit synchronous test/application work, not an interrupt
simulator or implicit device side effect. There is no fallible step after
the callback, hence no promised rollback of its effects.

## Distinct device command contracts

`struct mpv_rw{word,writable}` supports `mpv_update(view,clear,set)`. Reject any
requested clear/set bit outside writable before reading. On success perform
one read and one store of `(old & ~clear) | set`; set wins overlapping bits.
Unselected bits are preserved. This API applies only to an externally
serialized ordinary read/write register whose contract permits that access
sequence and preserved-bit value. It is not atomic against an interrupt,
other core or peripheral modification.

`struct mpv_w1_c{word,allowed}` instead supports `mpv_ack(view,flags)`: reject flags
outside allowed before access, then store flags once without reading.
W1C means write-one-to-clear in the proposed device contract. The hosted test
stores into a synthetic volatile C word and observes the command value; it
does not pretend RAM implements clearing hardware. For old0xa2/flags4 the
right command is4, while an inappropriate read/OR/write sends0xa6 and may
acknowledge unrelated pending bits on a real W1C device. This bounded original
defect demonstrates a wrong command property without touching a device.

Real adapters additionally need the device's access width, alignment, mapped
address, read side effects, reserved-bit rule, bus completion, barrier, reset
and interrupt-ownership contracts. No board register address is supplied here.
Volatile alone supplies neither a C happens-before relation nor the required
device barrier. Do not infer coherent multiword snapshots or atomic updates.

## Source and edition distinctions

The registered N1570 passages were read for generic selection and qualifiers.
The separate N2243 DR481 record is marked C17 and records the corrigendum
describing controlling-expression conversions as if applied: lvalue
conversion drops top-level qualifiers while pointee qualifications remain.
Treat that actual record as evidence; do not call the C17 question unresolved
merely because the earlier draft words differ. The exact published C17/C23
copies remain inaccessible. Both current compilers' direct top-level probe
selects category1, with any warning retained as an observation.

GCC's inspected volatile-access documentation distinguishes sequence-point
accesses from ordering of nonvolatile memory and explains assignment without
an implicit reread. These compiler rules do not replace an ISA or device
contract. Separate Clang LLVM IR observations count volatile operations at
O0/O3 for host and Cortex-M4; GCC host assembly is also preserved and inspected.
The M4 ELF32 images have no undefined symbols or dynamic dependencies, but
their artificial function entry has no startup or board adapter and is never
executed. These are compile/link/access audits, not target execution.

## Concrete deliverables and independent checks

- D1/R1: implement four typed receiver policies using function-designator
  selection, plus the two write and observe categories. Prove each evaluated
  pointer expression occurs once. Preserve original7/9 result316 and two
  evaluations, plus unsigned wrap. Provide unsupported and invalid unselected
  expression translation specimens with valid positive controls.
- D2/R2: distinguish top-level pointer const, pointee const/volatile, array
  conversion and duplicate compatible associations. Read the exact registered
  source passages and explain the C11-draft/DR481/current-compiler boundary.
  A no-default selector rejects unlisted types; it is not a universal trait.
- D3/R3: compare the independent4804-case category/command/refusal oracle,
  all four anchors and eight ordinary compiled semantic controls. Null and
  bad-mask refusals preserve prior outputs/word. A mutable alias changes17
  to23 between the two explicitly separated reads; the const view must still
  observe it. Keep diagnostics, semantic assertions and sanitizer failures
  as distinct outcomes.
- D4/R4: implement the volatile transfer and distinct RW/W1C adapters. Inspect
  actual access instructions/IR and link dependencies, then apply all thirteen
  dimensions below. Explain why a peripheral changing bits during RMW breaks
  the external-serialization premise, and why switching to an atomic object
  without an actual device contract is not a repair.

The held delayed task combines a const volatile observation view of a mutable
word, a caller effect counter, a mask refusal and a W1C acknowledgement. Its
author support is implemented; the independently attempted learner variant
still must be assessed after the original minimum delay. Level1 reproduces
one successful read; level2 meets every admitted/refused access and explains
the source/qualification boundary; level3 transfers the contract and identifies
why an unknown read-clear peripheral needs additional authority and evidence.

Graduated hints: select names before calls; list the receiver expression in
the preprocessed tokens; separate pointer qualification from pointed-to type;
then count permitted reads/stores before introducing any register wrapper.
Use the private assessor analysis only after the attempted solution is sealed.

## Thirteen cost and enforcement dimensions

| Dimension | Evidence/argument |
|---|---|
| Representation | A borrowed word pointer plus uint32 mask in each distinct device view; actual read/view sizeof values, no erased tag object. |
| Operations/invariants | Every core operation O(1); exact admitted access counts, valid mask and source/output separation. Callback runtime is separately bounded by its provider. |
| Ownership/lifetime | Borrow lasts through return/callback; const view does not own or freeze other aliases; all storage survives callback. |
| Failure | Complete null/mask admission before access; output/word unchanged on refusal; no rollback of explicitly admitted callback effects. |
| Enforcement | Generic/type constraints get diagnostics; explicit casts can bypass them; valid object lifetime, mask meaning and caller serialization remain premises. |
| Dispatch | Generic selection is compile-time function selection, not runtime type inspection; the explicit callback is still an indirect call. |
| Allocation/traffic | No heap or library device operation; one-word reads/writes or two reads around callback, separate ordinary output traffic. |
| Stack | Scalar temporaries and callback call frame; raw compiler stack reports, not universal stack proof. |
| Code/access | Actual O0/O3 symbols, helpers, disassembly and IR. Compiler merging of two plain loads versus two volatile loads uses distinct valid functions, not a cast-discard runtime experiment. |
| Timing | Seven10000-read generic/direct samples use the same volatile object, category3 and checksum70000. This measures the same host contract, not bus latency. |
| Build/link | Actual clean/unchanged builds; strict diagnostic specimens; M4 freestanding object/link audit with zero undefined/dynamic imports, no execution. |
| Composition | Existing numeric generic producer plus explicit read/write/observe and distinct register contracts; no automatic ISR or peripheral integration. |
| Portability/limits | Source semantics, compiler access definition, ISA and board contract are separately labeled; C17/C23 exact editions, physical access and independent learner/reviewer evidence remain gaps. |

## Security scope and validation status

Only original local programs and synthetic volatile words are exercised. The
adversary model is a mistaken or attacker-influenced requested mask/flags value;
the enforced property is no outside-mask command and no access on refusal.
The flawed read/OR/ack may send extra bits only under its stated W1C premise;
the harmless observable is the wrong synthetic command. Repair uses the
distinct write-only command API and regression checks all mask boundaries.
Public structs remain forgeable and volatile is not a security boundary.
No deployed equipment, external service, credentials or host settings change.
Reports distinguish six hosted profiles from M4 link-only cells. Finite tests
and source review do not establish every C execution or learner mastery.
