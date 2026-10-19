# Original B01 verification completion

This required companion retains X-T02-001, X-T03-001 through X-T03-005 and
X-T04-001 and X-T35-001. It supplies specific missing construction and falsification support;
the original briefs, objective rubrics and withheld transfer assignments remain
authoritative. Completing this companion does not replace their reasoning or a
sealed, delayed learner attempt. The implementation edition is C17.

Prerequisites are the accepted learner `fd_extent`, cursor, codec, typed-slot and
pool artifacts under `labs/foundation_depth/CONTRACTS.md`, with their exact source
and contract revisions. Import the completed provider using
`FF_FOUNDATION_SOURCE` and `FF_FOUNDATION_INCLUDE`. Author campaigns explicitly
use the private reference. The public companion remains unsolved. No provider is
silently selected from another learner's or assessor's workspace. The optional
`CMASTERY_ASSESSOR_ROOT` is an explicit author-validation selection.

All called objects are live and truthfully sized, control is externally
serialized, and input/output storage is distinct unless stated otherwise. No
callback reentry or nonlocal exit is admitted. Host `malloc`, stdio, sanitizers
and Python are verification dependencies, not component mechanisms. Do not use a
codec, copying, allocator or container library to implement the studied work.

## Interval construction: X-T04-001-R3 and original interval variant

Implement `ff_interval(offset,count,width,limit,out)` by importing the completed
extent check. Success writes `[offset,offset+count*width)` to `begin,end` exactly
when its mathematical end is at most `limit`. This is an exclusive endpoint
limit: equality is admitted. Empty `[SIZE_MAX,SIZE_MAX)` is representable and
does not name or form a pointer. Null output returns INVALID. Arithmetic refusal
returns RANGE and preserves **both** endpoint fields; no early begin publication.
Inputs are passed by value, so out may be a live interval whose old fields were
used to supply the arguments. No interval endpoint is an object address.

Derive the temporary-state invariant, intermediate representability, O(1)
word operations/storage and counterexamples `(1,1,1,1)` and `(0,0,0,0)` for the
two supplied defective policies. Submit the interval implementation, independent
unbounded-integer admission table, sentinels, and a changed inclusive-last-index
argument: a last index of SIZE_MAX cannot be represented by a size_t end equal to
last+1. Reject or change representation; silently wrapping is not an adapter.

## Source character copying: X-T03-003-R2

Implement the initializer callback `ff_copy_initialize(context,destination)`.
Context points to a distinct live, initialized `FDValue`. Destination is the
actual fresh, appropriately aligned, allocated storage passed by `FDSlot`.
Copy `sizeof(FDValue)` character elements from the source representation using a
loop. Null callback arguments return nonzero without access. Successful return
is zero. It acquires no resource; the slot retains its existing ownership and
destruction protocol. Do not replace the actual allocation by a declared byte
array, and do not claim arbitrary all-zero bytes construct arbitrary C objects.

Compare this route with typed field/structure stores and with the existing
declared-FDValue storage adapter. Observe values memberwise after publication;
copying source padding does not turn it into semantic data or an equality key.
Submit a type/lifetime timeline citing the selected source rule and its edition.
The operation is O(sizeof(FDValue)) byte work, constant for this concrete type.
The host test performs genuine allocations for both routes and frees each once.
The failed-initialization control must never call a full destructor. Do not run
double frees, invalid alignment or expired borrows as golden-output cases.

## Symmetric codec defect and object observations: X-T03-002

Implement `ff_encode`/`ff_decode` as explicit adapters to the accepted eight-octet
version1 codec. Their observable contract is exactly the original contract.
The assessor's paired mutation reverses count octets in both directions. Its256
memberwise round trips intentionally pass; the separately fixed count258 vector
must reject it. This is the required falsification of round-trip sufficiency.

Inspect a live FDRecord through unsigned char, record sizeof/alignment/offsetof
and observed representation separately from canonical bytes. Run the same golden
wire on a genuinely different ABI/endian execution where available. A matching
layout or unspecified padding byte is a permissible observation, not a portable
layout promise. No packed-struct or unaligned typed cast is executed. The supplied
PowerPC64 ELFv2 probe uses the separately pinned original raw Linux entry/write
adapter and an emulator. It is selected-image execution, not physical hardware.

## Cursor algebra and pool histories: X-T03-001/X-T03-005

For actual arrays of length0 through8, enumerate every position and admitted
partition a,b. Two takes must describe the same sequence and final position as
one take of a+b. Form pointers only after the bounds proof. Retain the original
null-empty and nested-subview tests and their distinct lifetime premises.

The independent Python pool model tracks mathematical issue counts/free slots,
not native addresses or the C control representation. Enumerate every length5
history over acquire into two saved-handle registers, release/get either,
foreign-owner lookup and invalid index: 32,768 histories. Add a complete
generation-retirement history. Failed acquire/get preserve outputs; wrong owner,
stale and invalid-index values cannot authorize access. Each driver history is a
fresh function invocation with its own pool lifetime; no handles escape. This is
not permission to reset a live pool. The withheld registry dossier already gives
the concrete epoch-reset revival counterexample and required maintenance premise.

## Semantic and code-generation comparisons

Compile the pointer constraint with warnings-as-errors disabled and an explicit
compiler diagnostic-severity policy, and separately under pedantic rejection.
Record messages and exit codes: ISO requires a diagnostic, not rejection. Run
only defined nested function-body cases: separate states give6 and shared-state
calls permit result/final pairs(5,4) or(4,3). Do not execute the unsequenced
argument-expression alternative. Distinguish admitted observations from the
complete semantic argument and exact published-edition access gaps.

Run the existing restricted/plain transform comparison at O0/O2/O3 with disjoint
arrays, identical outputs, separate translation units and no link-time
optimization. Preserve actual samples, resolution, size and disassembly. Zero
or equal timing is legal; no speedup or worst-case latency threshold is required.
Retain the checked-builtin/C23-draft comparison as a separately labeled source
and mode; the companion component remains C17.

## Independent local state oracle: X-T35-001

The exact original decoder/authorization core is compared with Python unbounded
integers over1,440 structured records,1,024 seeded byte/length cases and10 explicit
prefix/trailing cases. The original synthetic counter is the sole protected
effect. The oracle checks parsing precedence, both signed endpoints, product and
final-sum bounds, denial, unchanged state and all four syntactically valid flags.
Compile separate premature-publication and wire-permission controls; each must
return normally but disagree with the independent expected state. A crash or a
tool error is not that witness. Retain inputs, seed, bound and exact mismatch.
The existing withheld transfer package supplies QUERY, all128 fragment chunkings,
both authority transitions, cancel/reset and over-capacity nonpublication.

## Deliverables, assessment and hints

Submit D1 contracts, typing/identity/formation proofs and misleading-explanation
rebuttals; D2 source plus exact imported revisions; D3 independent vectors,
generated histories, intended semantic-control rejections and actual commands;
D4 changed inclusive-limit/ABI/equality/lifetime arguments and cost limits.
Every original rubric criterion is scored separately. Level0 is missing/wrong;
level1 passes nominal examples without its proof/failure account; level2 meets
the contract and finite falsification obligations with justified limits; level3
requires an actual unaided changed-condition attempt. Author reference evidence
cannot award a learner score. A finite passing campaign is not universal proof.

Hints in order: stage both endpoints; distinguish representation copying from
arbitrary-byte construction; intentionally make both codec sides share a defect;
model issued identities independently of the C generation field. Keep assessor
implementations/analysis and exact solutions outside learner distribution. Use
the original delayed release protocol, not an invented elapsed-time claim.

All demonstrations use original local code, synthetic values, unprivileged host
processes or dedicated emulation. There is no external target, credential,
privilege escalation, physical measurement or host security-setting change.
Actual status belongs in the source-bound campaign; source availability, external
human review and independent learner performance remain separate dimensions.
