# PR-T03-01 exact criterion analysis

Use the original nine criterion rubrics and delayed IDs. This source is withheld from learner exports. The component is profile A; the actual TLS adapter is separately labeled GCC GNU17/POSIX. No expired pointer, deliberately misaligned conversion or signaling/trapping representation is an executable value oracle.

## T03.01.a

| Object | Creation and owner | Last permitted observation | End / forbidden continuation |
|---|---|---|---|
| `lifetime`'s `automatic` | Entry into the function block, caller's activation | Member23 while that call remains active | Block exit; no escaped pointer is evaluated |
| `lifetime`'s `fixed` | Static initialization, whole program | Member17 while the program executes | Program termination |
| `OaOwner.data` | Successful allocator callback, unique owner | Prefix copy before the matching `put` callback | Release; stored numeric history survives, pointer views do not |
| worker `owned` | Worker thread creation, worker TLS instance | Main's synchronized write then worker's scalar copy | Worker exit after publication cleared; join reads only `observed` |

Automatic records last through their enclosing block; static records through program execution; allocated records through their release; each worker TLS record through that thread's execution. The worker waits under a predicate after publishing its own object. Main accesses it under the same mutex while the creator remains alive, writes73, clears the publication and releases the predicate. Join observes the copied scalar result, never the ended TLS object. Main's distinct TLS record remains31. This exercises the documented GCC `__thread` contract, not an invented ISO cross-thread permission. Shared-global and omitted-write mutants fail different observations. TSan covers only this schedule and selected access paths.

## T03.01.b

Growth failure must preserve pointer, length4, generation1 and bytes. The old pointer comparison is evaluated only while that object is still alive. Successful replacement allocates disjoint new storage before releasing old storage; copied prefix and zero suffix are checked. A uintptr_t is captured before release; later logging evaluates the integer and the new pointer, never an old pointer value. This is historical address observation and does not recover a live object. Growth/shrink/release account for three completed allocations and no retained owner. UINT_MAX generation refuses another allocation; the old allocation can still be released. Ordinary libc realloc's same-address behavior remains a separate contract, not simulated by claiming that two simultaneously live malloc objects coincide.

## T03.01.c

Both returned local-array and block compound-literal pointers lose their referents when the function's block ends. The repair copies before that end into caller storage, then returns a view whose owner is explicitly the caller. Insufficient capacity preserves both bytes and metadata. An aggregate copy copies its pointer field; changing the original pointee then changes what that shallow view observes. `oa_deep_copy` allocates nothing and uses distinct caller storage, preserving `c` when the original becomes `b`. Reject the borrow-instead-of-copy mutant by output identity and subsequent independent value. Zero-length views still require the declared valid controls in this API; do not infer other libraries' nullable-empty conventions.

## T03.02.a

The selected native layout is measured, not built into the component. Each field occupies its actual sizeof interval at its offsetof; all other bytes are padding. The harness explicitly writes different character values to identified padding after member initialization, then compares member equality and representation equality separately. No field is read before initialization and no padding value is serialized. Reordering double/int32/char changes offsets even when total size remains16. The two structure types are distinct and not interchangeable simply because their storage sizes match.

## T03.02.b

N1570 6.2.6.2 permits sign-and-magnitude, ones' complement and two's complement, with implementation choices/padding qualifications. N3096 6.2.6.2 defines the two's-complement sign contribution; its representation clause still permits unspecified padding and non-value representations. These are inspected draft facts, not a claim to have read inaccessible final editions. Base C does not promise the lab's IEEE binary64/quiet-NaN layout. The selected native and i386 runs change pointer/long width and record alignment while retaining the deliberately fixed wire int32/binary64 encoding. Neither run changes endianness; do not claim an executed big-endian test.

## T03.02.c

Equal records can have different padding. Floating +0 and -0 compare equal but have different selected binary64 representations; NaN does not equal itself by floating `==`. The application equivalence deliberately groups all quiet NaNs. Canonical output `z,-2,1.5` is hex `7afffffffe3ff8000000000000`. Both zero signs encode eight zero bytes; all quiet NaNs encode `7ff8000000000000`. Convert negative int32 to uint32 modulo2^32, then emit shifts rather than copying host integer order. The binary64 codec has an explicit target representation precondition, plus refusal checks; it is not a universal float serializer. Byte-equality and negative-zero normalization mutants distinguish the policies.

## T03.03.a

Padding is `(A - start % A) % A`, after power-of-two/contained-slice admission. Derive0,7,1,0 for the four fixture offsets. Check length against total-start before any start+length; check padding and width by subtraction before publication. The interval [1,8) has no room for a width1 item at8; [1,9) does. The independent oracle enumerates candidate aligned offsets rather than duplicating the formula, and retains all3690 cases per profile. Width0 is an empty arithmetic interval; it is not evidence of a zero-size C object. A live typed array's stride preserves element alignment; the wrapper selects actual records by index.

## T03.03.b

N1570 6.3.2.3p7 constrains the converted pointer's alignment; deferring only the eventual dereference is insufficient reasoning. `oa_decode_u32` instead reads four unsigned chars, so byte-offset1 is admitted and no stricter pointer is ever formed. The selected optimized assembly is retained. If the compiler chooses a wide unaligned machine load, that implements the defined C byte accesses under its target rules. It does not establish that a different typed C cast is valid. ISA capability, compiler lowering and ISO source semantics are distinct layers.

## T03.03.c

The arithmetic interface proves geometric capacity only. `OaStorage` proves actual type and base alignment by declaration, while its lifetime and exclusive access remain caller obligations. A result at an aligned address inside an arbitrary char array would not supply this declared-record contract. The selected extended-alignment type requires64; max_align_t is16 in the actual native/i386 observations and cannot promise that stronger alignment. The wide-admission mutant confuses fundamental maximum with every supported alignment and must fail without performing an invalid conversion. Integer-pointer mappings are implementation-defined; a modulo observation is not an unconditional ISO address validator.

## Assessor evidence limits

Require the six compiled mutants, five unfinished-component rejections, explicit allocation-failure cleanup, independent placement oracle, actual i386 loader execution and TLS controls. A first campaign exposed test-harness leakage on an early mutant assertion; the corrected harness frees its tracked allocations while LeakSanitizer remains enabled. The changed-schedule, ABI and source arguments remain required even when ordinary tests pass. External review, delayed learner attempts and physical observations are separate, unperformed dimensions.
