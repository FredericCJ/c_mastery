# PR-T37-02: Register effects, instruction bytes and storage permission

This required additive contract implements the six original T37.03/T37.04
criteria and their original `AX-PR-T37-02-...-V1` variants. Their task text,
seven-day delay, rubrics and historical baseline bindings remain unchanged.
Use it with the parent project and legacy assessment shared contract. Source
locators are in `research/v5/expansion/legacy_architecture_bytes/resources.json`;
private technical analysis is withheld in `assessor/architecture_bytes/ANALYSIS.md`.

## Prerequisites and environment

Complete the B01 integer/extent/object exercises and PR-T36-08 typed wrapper
work. Import the exact `struct aw_record { int32_t id, rank; }` declaration from
`labs/algorithm_wrappers/wrappers.h`; this batch consumes its type, not the
wrapper's container implementation. Explain lifetime, effective type, unsigned
arithmetic, promotion and shift bounds before touching instruction bytes.

The baseline is ISO C17, eight-bit bytes, exact 32/64-bit integer types and an
eight-byte record. N1570 is inspected C11 draft evidence, not the final C17 or
C23 text. Assembly uses GNU tools, SysV x86-64 on the host and Armv7-M Thumb on
a dedicated QEMU MPS2 Cortex-M4. GNU section attributes and linker scripts are
implementation extensions. F439 placement is a separate cross-linked image;
it is not run on the MPS2 or a physical F439. Python, disassemblers, sanitizers
and clocks are host verification dependencies. Core functions use no library
decoder or byte-order library; the target final image has no unresolved helper.
The host storage demonstration may call `malloc`/`free` to obtain genuinely
allocated storage: implementing an allocator is not this criterion.

## Problem, representation and operations

Implement `bytes.h` in `learner.c` and the two learner assembly files. Preserve
the exact original fixture in `fixtures/algorithms/PR-T37-02.json`.

1. `ab_write(old,width,value,out)` accepts widths 1, 2 or 4 bytes and a value
   that fits. Predict and then capture actual full RAX after consecutive
   `AL=AA`, `AX=BBCC`, `EAX=89ABCDEF`, starting at `1122334455667788`.
   `ab_registers` writes the three actual values to a live three-element
   `uint64_t` array. Contrast an actual Thumb `MOVS R0,#AA` after nonzero upper
   state. Distinguish register operand width from instruction length.
2. `ab_word(p,n,big,out)` reads exactly the first four bytes, with `big` 0 for
   little endian and 1 for big endian. Additional bytes are untouched. Decode
   `78 56 34 12` under both wire orders, regardless of host byte order.
   `ab_address(base,index,scale,disp,width,extent,start,end)` returns a numeric
   half-open coordinate interval. Scale is 1,2,4,8; width is 1..8; displacement
   is signed 64-bit. Require the entire access inside `[0,extent)` before
   adding the numeric base. Reject multiplication, negative-offset and endpoint
   overflow. These synthetic numeric coordinates never become native pointers.
3. Encode/decode an eight-byte record: signed 32-bit ID then rank, each in
   chosen wire order, two's-complement wire representation. Both record APIs
   require exactly eight bytes. Decode into an actual declared `struct aw_record`.
   `ab_record_copy` copies bytes between two live valid typed records; identical
   source/destination is allowed. Demonstrate a typed pool and genuinely
   allocated backing initialized through an `struct aw_record` lvalue, and contrast
   compile-only `bad_cast.c`. Neither `_Alignas` nor a successful load changes
   the declared type of an unsigned-character array.
4. Write a bounded offline decoder with `struct ab_instruction` as its explicit result.
   x86-64 base scope accepts only `48 3D imm32` (CMP RAX, sign-extended immediate)
   and `77 rel8` (JA). The extension flag adds `3C imm8` (CMP AL) only. Thumb
   base scope accepts only CMP-immediate T1: `00101 Rn imm8`. Its encoding is
   16 bits; its register arithmetic is 32 bits. Extension adds conditional B
   T1 for conditions 0..13, excluding IT-block execution semantics. DExx and
   DFxx remain unsupported. Width prefixes 11101/11110/11111 require four
   available bytes and then return unsupported, not a fabricated instruction.
   Output unused fields are zero. CMP `signed_immediate` means extended 64-bit
   value for RAX, but the nonnegative immediate for AL and Thumb. Branch fields
   retain the signed byte displacement (x86) or signed scaled displacement
   (Thumb), condition and target. No decoded input is executed.
5. Compute JA at `0x1006` from `77 FE` using next PC, then relocate it. For the
   selected Thumb branch use current PC+4 and doubled signed imm8. This rule
   is instruction-specific: some other Thumb PC-relative instructions align
   that PC. The decoder admits even 32-bit Thumb PCs and rejects arithmetic
   wrap/underflow; it does not emulate all architectural address modes.
6. Assemble/disassemble the original byte fixture and inspect it independently
   of decoder output. GNU `cmpq $-1,%rax` chooses the shorter `48 83 F8 FF`.
   Therefore `encoding_att.S` and `encoding_intel.S` explicitly emit the
   required accumulator form with directives, then assemble real mnemonic
   comparisons. The directive is not misreported as a mnemonic choice. Require
   identical AT&T/Intel byte streams; compare `encoding_wrong.S`, whose 32-bit
   operand really changes the encoding. Decode the shorter form as unsupported.

All control/output pointers must name live objects of the declared type;
output objects are disjoint from input buffers and controls, except the stated
same-record copy. Byte pointers can be null only with zero available bytes.
No API can validate an arbitrary forged pointer or caller's false extent.
Null required controls, invalid modes and widths return `AB_INVALID`; a
recognized but incomplete instruction/word returns `AB_TRUNCATED`; excluded
encodings return `AB_UNSUPPORTED`; admitted coordinate arithmetic outside the
model returns `AB_RANGE`. Entire outputs stay unchanged on failure. Decoder
admission checks precede publication, and successful decode consumes only its
reported length. No function retains an input pointer or owns a resource.

## Necessity and reasoning

The misconceptions are independent: upper register state is not uniformly
preserved; wire order is not host order; start-in-range is not whole-access
validity; alignment is not effective-type permission; instruction length is
not operand width; and disassembler agreement is not permission to execute
untrusted bytes. Derive each predicate before consulting the test expectation.

Provide pre/postconditions, the prepublication invariant, explicit signed
conversion/shift proofs, ownership/lifetime and failure-state tables. Show why
INT64_MIN magnitude and sign extension cannot overflow the implementation.
Core work and storage are bounded O(1); the byte-copy loop is O(sizeof Record).
No decoder allocation or hidden registration exists. Distinguish the model's
range policy from ISA wrap behavior, C semantics, ABI register preservation,
operating-system clocks and board bus reachability.

## Deliverables and verification

D1 is the six-leaf prediction/derivation dossier and full field annotations.
D2 is the original core, native/Thumb register experiments, typed and allocated
adapters, and offline decoder. D3 is source-bound raw execution/disassembly,
independent Python arithmetic/byte oracle, whole-output rejection evidence and
at least the five semantic mutations exercised by the assessor. D4 is each
original V1 investigation below; D5 records resource costs and evidence limits.

Run GCC and Clang at debug, optimized and Address/UndefinedBehaviorSanitizer
settings. The public test selectors are `registers`, `bytes`, `storage`,
`decode`, `branches`, plus host `allocated` and `timing`. A learner scaffold
must compile and fail a named semantic assertion; compilation failure is not
the expected learning signal. Run the M4 tests in the dedicated emulator and
audit final symbols, program headers and map. Do not run `bad_cast`.

Expected discriminators include the original three RAX values; word
`0x12345678`; `[0x1014,0x1018)`; sign extension of imm32 −1; and JA returning to
`0x1006`. Independently check all 65,536 Thumb first halfwords (6,144 wide
prefixes, 2,048 selected CMPs, 57,344 unsupported short forms), 5,472 numeric
extent packets, 50 record wire packets and 7,936 relative-branch packets.
Truncations and unsupported prefixes must preserve output. These exhaustive
finite domains do not prove a general decoder, arbitrary pointer safety or
absolute mastery.

## Changed conditions and measurement

* `AX-PR-T37-02-T37.03.a-V1`: compare original partial/full-register loops with
  the same count/output contract. Count zero returns the initial value; any
  positive count returns `(17+add) mod 256`. Use two work sizes, 31 alternating
  paired repetitions, monotonic clock resolution, CPU-before/after values and
  retained raw samples. Explain the merge dependency and inspect actual loop
  bytes. Different instruction lengths, front-end behavior, scheduling and
  warm state confound a latency attribution. No speedup threshold is a pass gate.
* `AX-PR-T37-02-T37.03.b-V1`: change wire order independently of the executing
  host, including INT32_MIN/MAX, and explain why casting cannot implement it.
* `AX-PR-T37-02-T37.03.c-V1`: cross-link the actual typed `ab_cpu_only` object
  into F439 CCM at `0x10000000`, and a candidate into AHB SRAM at `0x20000000`.
  Inspect actual map/symbol sizes and whole intervals. CCM satisfies lifetime,
  type, CPU-access and alignment obligations but fails DMA reachability under
  inspected RM0090 §2.3.1. SRAM placement alone does not configure or synchronize
  DMA. No board access or flashing is authorized by this author campaign.
* `AX-PR-T37-02-T37.04.a-V1`: add the newly inspected `3C ib` and Thumb B T1
  forms through the explicit extension mode; retain unsupported behavior for
  every neighboring excluded encoding and incomplete wide prefix.
* `AX-PR-T37-02-T37.04.b-V1`: relocate the same bytes to multiple bases and
  reject the original-PC mutant. Explain numeric boundary rejection separately
  from the selected ISA's modular PC behavior.
* `AX-PR-T37-02-T37.04.c-V1`: compare the actual two syntax spellings, emitted
  bytes and changed operand-size control; identify directives versus mnemonics.

The seven-day independent task uses an unfamiliar admissible input/placement
and requires a fresh derivation. Author execution does not satisfy the delay.

## Rubric and hints

Apply each original leaf rubric separately. Level 0 has a wrong state or
unsafe admission. Level 1 reproduces fixture output without its causal rule.
Level 2 derives the applicable ISA/C/board rule, implements it, preserves all
failure outputs and kills its specific mutant. Level 3 independently handles
the original changed condition, supplies a new counterexample and explains
which evidence does not transfer. A timing chart alone cannot earn level 3.

Hints, used only after retaining an unaided attempt: first draw input/output
bit positions and the whole half-open interval; next split byte collection
from signed interpretation; finally introduce a temporary instruction result
and publish it only once. For a stubborn encoding mismatch, inspect whether
the assembler selected a shorter immediate form before blaming byte order.

## Security and validation limits

Only original local code, synthetic bytes and dedicated teaching emulator
images are used. Arbitrary instruction data is decoded but never executed.
Invalid casts are analysis-only. No external system, real data, host security
setting, board flash or deployed firmware is touched. A malformed stream
rejected without output mutation is a bounded parser property, not a code
execution claim.

Content and assessor implementations are authored; actual execution is reported
by the current source-bound `legacy_architecture_bytes/validation.json` and
`exports.json`, not inferred from this prose. External review, independent
learner assessment and physical observations remain separate and unperformed.
