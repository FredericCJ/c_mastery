# Required T68.06 schema evolution and stale-consumer investigation

This is required support for X-T68-006 R1-R4 and X-T68-006-DELAYED, in addition
to retained `metaprog.reflection`. Complete X-T68-005 and PR-T20-01; import your
accepted `codec.records` implementation through `api_depth/architecture/arch.h`.
The actual `ar_encode/ar_decode` version2 contract is used for the signed field.
Retain that artifact's source/acceptance evidence and pass its workspace as an
explicit dependency. An assessor dependency is author validation only.

Public `include/schema.h`, declared `src/schema_fields.h`, host identity utility,
tests and this contract are supplied. Implement `src/schema.c` using the field
list. Do not use a reflection/serialization library. The private solution and
held changed-schema technical analysis remain under assessor/. Host Python
hashing/oracle/build tools are verification dependencies, not target code.

## Representation, prerequisites and environment

Use hosted ISO C17, exact-width integer types and eight-bit octets, GCC and Clang
debug/release/address+undefined-behavior sanitizer profiles. All native objects
are live, correctly aligned, initialized as specified, externally serialized
and disjoint; output objects and wire arrays do not overlap inputs, each other,
or callback-accessible storage. Extents refer to real backing objects. Null
required pointers are INVALID without publication; `mps_fields(NULL)` returns0.
Other invalid pointers, concurrent mutations and lifetime violations are caller
errors, not safely diagnosable operational failures. Context may be null if its
callback accepts it. No allocations or retained user pointers are involved.

The trusted declared schema has consecutive indices0..N-1, N<=8, unique increasing
tags1..255, named distinct fields, first versions1..MPS_VERSION<=255, required
flags0/1 and representable ordered limits. U16 uses uint16/2wire octets, U32
uint32/4 and S32 int32/5. These are authoring premises, not an untrusted schema
loader. Changed lists must preserve them. Required fields start in version1.
The supplied schema is code(tag1,U16,required), value(tag2,U32,required),
note(tag3,S32,optional,since2). The native record has a `present` bit mask and
generated typed fields. All members have initialized values; values of absent
fields are ignored. Required bits must be present and no unknown bit is allowed.
Only present values are checked against the registered limits.

Generate declarations, descriptor names (`#name`), `offsetof`, native sizes,
wire widths, field count, typed get/set dispatch, validation, equality, wire
operations and visitor selection from this one list. Do not cast arbitrary
offsets into field pointers. The descriptor table has static immutable lifetime;
`mps_fields` publishes a borrowed pointer and count. It discovers this declared
record only. C does not supply arbitrary type introspection through these macros.

## Wire, migration and failure model

The wire is version,count followed by exactly count records of tag,width,payload
in strictly increasing tag order. There is no native padding on wire. Count is
bounded by the declared schema; total length is2..64. U16/U32 payloads are big
endian arithmetic encodings. S32 payload is exactly the imported version2
signed32 big-endian value plus its XOR octet. XOR detects some accidental
changes; it is not authentication and collisions are easy to construct.

For present7,code0x1234,value0x89abcdef,note-2 at version2 the exact19octets are
`02 03 01 02 12 34 02 04 89 ab cd ef 03 05 ff ff ff fe 01`.
At version1 only required fields are representable, making12octets. A version2
reader accepts version1 and initializes absent note to0 while leaving its bit
clear. Encoding that decoded record at version2 changes only the version byte.
This is explicit migration, not native struct compatibility. Version0 or a
future version, unknown/duplicate/unordered tags, wrong widths, trailing or
truncated input and missing required fields are INVALID. The reader does not
silently preserve unknown fields. An understood present value outside its
registered limits is RANGE.

`mps_encode(version,record,wire,capacity,used)` admits pointers/version, validates
the whole record, rejects any present field introduced after the requested
version with RANGE, then stages the complete wire. Capacity below that exact
length is FULL, including zero capacity with a valid pointer. INVALID/RANGE/FULL
preserve every output byte and used. On success publish exactly used bytes,
leaving the rest unchanged. `mps_decode` stages the complete native record,
validates the exact extent and all fields, then publishes once. Any refusal
preserves output. Decode never consumes a prefix as success.

`mps_equal` first validates both records; invalid records return INVALID and
leave equality output unchanged. Otherwise presence masks must agree and each
present value must agree. Absent native values and padding are not compared.
`mps_visit` validates before effects, invokes exactly once per present field in
registry order and stops when the callback returns0. STOP publishes the number
of invocations including that stopping call. Complete success publishes the
total. Admission/validation refusal leaves visited unchanged and calls nothing.
Callback effects already performed are not rolled back. Callbacks return
normally, do not reenter these operations, mutate/relocate/destroy the record,
descriptor or output, or retain borrowed arguments. They may change only their
separately owned context. No asynchronous use is admitted.

## Required investigations and artifacts

1. D1: submit the registry expansion and operation source, valid-state proof,
   actual imported codec source identity, exact wire predictions and descriptor
   offset/native-size/wire-width table. Run schema_wire/semantics/visit/base and
   retained reflection. Native offsets are observations, not wire positions.
2. D2: show the preprocessing result and explain `#`/`##`, argument expansion and
   declared-schema scope. Compare the executed Rust macro_rules repetition and
   derived PartialEq specimen: Rust derives a field-wise implementation from a
   language item; this C implementation separately maintains a registry. Neither
   example proves arbitrary C reflection or C++26 conformance. N1570 is a C11
   draft; exact published C17/C23 remain access gaps.
3. D3: run the independent oracle, all six compiler profiles, eight compiled
   semantic mutants and four compiling learner-stub refusals. Add an optional
   quality:uint16 tag4,width2,since3,range0..15; regenerate all operations from the
   changed list, including equality, validation and visitor. Preserve exact wire
   bytes under consistent `-fpack-struct=1` translation as a GCC/Clang extension
   experiment. Never link incompatible layouts and execute them together.
4. D4: make an old handwritten equality consumer that still compiles after the
   quality field is added and incorrectly ignores a changed quality value.
   Generate and enforce a full-input fingerprint; demonstrate stale source,
   stale compiled object, and fully regenerated positive controls. Submit the
   build dependency proof, failure logs and all13cost dimensions below.

The supplied host `schema_stamp.py` hashes complete field-list bytes and its own
generator bytes. Its `--check` compares the entire expected generated header.
Generated consumers can embed all four64-bit expected words in static assertions.
An old object calls the old hash-specific guard symbol; the new core defines
only the new one, causing a link failure. This linker experiment relies on the
observed GCC/Clang symbol significance; ISO C minimum identifier significance
alone is not a universal guarantee for the long symbol. No stale ABI object is
executed. A consumer that deliberately omits every guard remains possible.
Fingerprints detect accidental staleness, not malicious builds, authentication,
semantic equivalence or mathematical impossibility of a hash collision.

The delayed variant is held until the unaided assessment: change a field's
presence/version/range obligations, provide an old consumer that remains
syntactically valid, and require a migration/refusal policy plus a rejected
stale artifact and independently derived bytes. Merely changing a constant or
re-running the assisted quality case does not demonstrate independent transfer.

## Verification and assessment

Use `tools/run_metaprog_schema.py`, `verify_metaprog_schema.py`,
`run_metaprog_schema_evolution.py`, and `check_metaprog_schema.py --self-test`.
Reports record exact commands, source/tool dependencies and observed outcomes.
Independent Python uses integer byte conversion and a separate record parser;
it does not import generated descriptors or the C implementation. It is still
finite, with an explicit input model. A sanitizer silence does not prove all
wire inputs safe. Required semantic controls must compile and fail ordinary
MPO_CHECK assertions, not merely terminate under a sanitizer.

R1 requires the same-list representation and complete operations; R2 requires
correct semantic/enforcement distinctions and an actually observed comparison;
R3 requires independent byte and unchanged-output evidence under changed layout;
R4 requires actual added-field migration, stale artifacts and positive rebuild.
Use the original four rubric IDs: nominal anchors alone earn at most level1;
level2 requires all listed reasoning and evidence; level3 requires an unaided
changed-assumption case with its counterexample. Preserve assistance and delayed
assessment dates. Author tests are not learner acceptance or independent review.

Graduated hints: (1) distinguish schema index, native offset and wire tag;
(2) generate a typed switch, not a type-punned offset dereference;
(3) stage the complete result before publication and treat a guard-free consumer
as a counterexample to universal regeneration guarantees.

## Cost and enforcement ledger

| Dimension | Evidence and bound |
| --- | --- |
| Representation | Actual sizeof/offsetof table; present bits plus generated native fields; descriptors have names and native/wire metadata. |
| Reachable storage | Static N descriptors/string names, caller record/wire and local64-octet encoding stage; no retained caller pointer. |
| Lifetime | Record/byte borrows last through the call; static descriptors outlive them; callbacks cannot retain borrowed values. |
| Mutation/aliasing | Disjoint external serialization is a premise; generated setters operate on exact declared members. |
| Allocation | Zero target allocations; host Python/build allocations are separate. |
| Movement/copying | Encode stages then copies exact wire length; decode stages one record; padding is never serialized or compared. |
| Destruction | No owned dynamic storage, destructor or recursive cleanup. |
| Dispatch | Generated typed switches and descriptor scans; visitor indirect call once per selected field; actual codec calls and object disassembly retained. |
| Concurrency | No shared concurrent mutation or atomic publication guarantee; no locks/threads in the core. |
| Failure | No output publication on refused codec/extent/version/range; callback effects remain on STOP. |
| Compilation | Preprocessor bytes/tokens, clean/unchanged build time, static/link/source staleness controls; extension flags recorded. |
| Binary/stack | Six-profile size/stack/object logs; compiler-generated memcpy/memset and stack helpers, if emitted, are recorded as hosted dependencies. No freestanding claim. |
| Boundary | Canonical big-endian tagged wire versus native layout; explicit version/unknown-field policy; Rust comparison makes no C ABI claim. |

For N fields and W<=64 wire bytes, validation/equality/visit/encode are O(N+W)
under fixed-width operations; decode's tag lookup is O(N^2+W), since each input
field scans the registry. Stack is O(W+sizeof(record)), no recursive frames.
Seven10000-encode CPU-time samples have zero separate warmup and include all
required checks/copies; report timer resolution and variability. They do not
compare against an easier six-byte contract or claim physical latency.

All adversarial inputs target this original local teaching program with
synthetic bytes. The harmless success condition is detecting a wrong equality,
invalid acceptance or premature output publication. No external target or real
data is involved. Repair by restoring required invariants and rerun the named
regressions; checksum/fingerprint limitations remain. Actual source-bound
execution status lives in the matching reports. Physical execution, independent
human review and unaided learner assessment remain unperformed.
