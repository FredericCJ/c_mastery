# B01 withheld changed-condition assignments

Assessor distribution contract: release only the selected assignment, `transfer.h`,
`learner.c` and the public `depth.h`/foundation contract after the prior attempt has
been sealed and at least seven days have elapsed. Keep `reference.c`, `tests.c`,
`ANALYSIS.md`, `SEMANTIC_CASES.md` solutions and campaign logs withheld. These files implement today's assessor
support; there is no claim that a learner has attempted them or that seven days
have elapsed. Record original attempt, assistance, release and submission times.

Each assignment retains its existing `X-…-DELAYED` identity and parent objective
rubric in `curriculum/v5/exercises/`. Families are construction, boundary,
falsification and independent transfer; the framing assignment also includes
security and recovery. Prerequisites are the parent's accepted learner component,
its reasoning dossier, and the exact interfaces in `labs/foundation_depth/CONTRACTS.md`.
Import that learner component on assessment day. Author validation imports
`assessor/foundation_depth/depth.c`, explicitly a reference, never learner work.

Environment: C17, octets and the exact-width types required by `depth.h`; Linux
x86-64 hosted verification and Cortex-M4 compile/link comparison. Target code may
use only C operators, loops and these component functions. No libc implementation
of the studied mechanism, heap allocation, container library or implicit runtime
helper is admitted in the transfer core. Host checks may use Python, C library
comparison and sanitizers. There is no target execution implied by a cross-link.

All native objects must be live and truthfully sized; control and output storage
is disjoint unless an operation explicitly permits aliasing. No operation can
authenticate arbitrary pointer bits. Unless specified otherwise rejection
preserves outputs and state. Submit source, an invariant argument, independently
derived expected results, failure traces, object/link audit, and changed-condition
cost analysis. Record seed/bounds and every compiler flag. Passing a finite harness
does not establish the reasoning rubric or absolute mastery.

| Stable assignment | Required mechanism and concrete task | Observable criterion |
|---|---|---|
| X-T02-001-DELAYED | Implement `fd_step_value` returning status/old/new by value. At `INT_MAX`, report range and retain old/new equal to input. Under the two specified integer models in the parent analysis, derive `USHRT_MAX+1` and its narrowing separately. | Boundary values from `INT_MIN` through `INT_MAX`; explanation distinguishes promotion from later conversion and explains removal of output aliasing. |
| X-T04-001-DELAYED | Implement `fd_layout` for header followed by two counted segments, returning first offset, second offset and total. Counts are signed and must be admitted before conversion. No alignment padding is requested. Header12, counts7/10, widths9/11, ceiling180 is a required rejection. | Preserve all three fields on either-stage failure; derive combined admission and actual-width overflow limits, including zero widths. |
| X-T03-001-DELAYED | `FDSegmented` borrows two independent arrays. Implement copy-out from logical position into disjoint caller storage, with no allocation. Segments `{11,22}` and `{33,44,55}`, position1, count3 must copy three logical elements. | Reject capacity2 and logical exhaustion before any write or cursor advance; empty null segments are allowed. Reject an unrepresentable total. Explain why one contiguous borrowed pointer cannot represent a cross-segment result. |
| X-T03-002-DELAYED | Encode/decode version2: byte0=2, byte1=0, count little-endian bytes2–3, signed value little-endian bytes4–7. Count/value use greatest-width logical types and must fit unsigned16/signed32 wire ranges. | Reject reserved byte, wrong length, wider logical values and output-capacity mismatch without partial output; independently derive bytes for count258/value-2. |
| X-T03-003-DELAYED | Reserve one actual declared `FDValue` through `FDTypedStorage`, revise the base allocator's no-declared-type premise explicitly, and transfer slot ownership to an empty destination while invalidating source. An initializer acquires synthetic A then B; inject failure before B and after B acquisition, rolling back each acquired token. | Reset after transfer releases only through destination. Failed initialization releases backing but never invokes the full destructor; successful destruction releases B then A. Explain declared type and caller-enforced noncopyability. |
| X-T03-004-DELAYED | `fd_stride_move` operates on logical bytes separated by positive stride within one real physical array. Leave gap bytes unchanged. `fd_compact` moves a stream suffix to its prefix and increments a finite invalidation epoch. | Validate last logical byte extent before access; compare with a pre-operation snapshot. Stream length8/position3 compacts to length5/position0. Reject epoch exhaustion unchanged and invalidate all old semantic views, even if addresses remain live. |
| X-T03-005-DELAYED | Implement two registry slots, each with epochs1–3, pointing to stable live `FDPool` owners. A handle has registry slot/epoch and local slot/generation, no raw owner pointer. Unbind invalidates the epoch and never destroys the owner's values. Rebinding may change owner. | At epoch3 unbind permanently retires the registry slot; no wrap or reset while old handles exist. At most one binding per owner. Old handles remain stale across owner changes; outputs remain unchanged on failure. Explain registry lifetime as a remaining precondition. |
| X-T35-001-DELAYED | Implement eight-byte incremental framing. First nonempty accepted fragment captures trusted authority; later fragments cannot change it. Over-capacity feed rejects wholly. Completing a frame calls the existing checked decoder/admission path exactly once, then discards staging on success or failure. Cancel clears staging and authority. | Enumerate chunkings and cancellation positions. A zero-byte feed consumes nothing and cannot grant authority. `committed` is zero on an admitted partial/failed completed frame and one only on successful commit; metadata/capacity rejection preserves it. No implicit second-frame parsing. |

The security target is only this original local frame/counter implementation and
its deliberately flawed assessor control, using synthetic bytes in an unprivileged
process. The attacker controls fragment bytes and chunking; trusted per-frame
authority remains outside attacker control. The harmless violation is a denied
fresh request incrementing a counter after cancellation of an authorized partial
frame. Exploitability requires stale authority reuse; a crash, sanitizer finding,
arbitrary code execution or privilege escalation is not the success criterion.
Repair by revoking staged authority, test both authority transitions, and explain
why this does not authenticate an external peer or roll back external effects.

The additional parent variant `X-T35-001-QUERY` implements `fd_effect`. The effect
is a trusted harness enum, separate from wire flags. Both effects require the
same valid eight-byte record. QUERY is public and returns the old counter in
`observed`, ignoring the requested count/value without performing their arithmetic;
it never changes the counter and does not require update authority. UPDATE uses
the original authorization and range rules and returns the committed counter in
`observed` only on success. Invalid effect/control pointers reject first; invalid
records reject next. Every failure preserves both outputs. The output objects
must be distinct and live. Required discriminators are a denied valid query, a
query whose hypothetical update would overflow, and an unauthorized UPDATE with
each of the four syntactically valid wire flags. Explain why reusing update
admission for queries changes this contract and why interpreting a wire flag as
the trusted effect or permission breaks the threat model.

Complete the prerequisite variants before the delayed attempt; these are required
parts of the existing briefs, not substitutes for the unseen transfer:

| Parent | Additional supplied interface and contract | Discriminating obligation |
|---|---|---|
| X-T02-001 | Release only prompts1–4 from SEMANTIC_CASES.md, with the header and integer-model premises. | Distinguish function-body sequencing, argument evaluation, translation constraints and runtime preconditions; no execution oracle for UB. |
| X-T03-001 | `FDOwnerView` retains the real owner pointer/length and absolute half-open begin/end offsets plus a relative position. `fd_owner_subview` offsets from the unconsumed suffix, retains owner identity, resets position, and permits `out==source`. Invalid metadata returns INVALID, inadmissible range RANGE. | All source/output fields remain unchanged on rejection; compute within the admitted owner interval before forming pointers. |
| X-T03-001 | `fd_read_tagged` parses `A5 02 low high` from a cursor, returns a little-endian16-bit value and advances by4. It may leave trailing data. Bad tag/count is INVALID; missing header/payload is RANGE. | The consumer owns rollback: any rejection preserves cursor and output, even after reading the two-byte prefix. The cursor primitives themselves retain their original advancing contract. |
| X-T03-002 | `FDLeastRecord` uses uint_least16_t/int_least32_t; `fd_least_encode` and decode retain the version2 octet protocol and range checks. | Admit values by mathematical range, not sizeof equality; compare fields rather than padding. Greatest-width tests model possible wider least types when this host has none. |
| X-T03-004 | `fd_inplace_xor` transforms one array using a single pointer, accepting null only at length0. | Prove each byte read precedes its own write; no disjoint two-pointer restrict promise is made. Verify involution for every byte/mask. |
| X-T03-005 | `fd_pool_borrow` returns a const pointer only for a currently valid handle; null output is INVALID and stale handles retain output. Borrow expires at release, relocation or owner destruction. | Never read an expired borrow to test invalidation. Handle validity is checked separately and const does not extend lifetime. |
| X-T03-005 | `fd_registry_relocate` moves a bound pool to distinct, live destination storage with no owned contents. No native local handles, borrows, concurrent operations or other bindings may survive the move. Copy values/generations, retire the old storage slots, then update the registry pointer; preserve registry epoch and external logical handles. Null/bad slot is INVALID; absent/retired source or already-bound destination is STATE. | An external logical handle continues to return the same value after relocation; old slots no longer own it. Borrow revocation and exclusive access are caller obligations. Destination aliasing with control storage is excluded. |

These adapters are O(1) in the fixed registry/record model, except in-place XOR
which is O(n), and need O(1) extra storage. Owner-relative subview creation cannot
prove that a caller's advertised owner length is truthful. The relocation operation
is intentionally synchronous; changing that assumption requires reclamation and
publication protocols rather than a cast or a wider generation counter.

Hints are released in order and logged: (1) write the state and ownership diagram;
(2) stage admission before publishing any output; (3) model ranges/identities as
integers before forming pointers. A requested hint earns guided credit for that
step; it does not retroactively establish unaided transfer.

Assessment scale per linked objective: 0 = absent/incorrect or violated admitted
contract; 1 = ordinary examples pass but proof/failure/ownership missing; 2 =
implementation and bounded negative tests agree with a valid contract argument;
3 = unaided changed-condition derivation, counterexample and defended limitations.
Require at least2 for every applicable criterion. Award3 only from an actual
sealed learner attempt, never from the supplied author reference. An alternative
representation is valid if it satisfies the same observable contract and explains
its cost and enforcement differences.
