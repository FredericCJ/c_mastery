# Logical bits, explicit wire formats and stateful register access

This successor implements PR-T04-02's nine retained criteria and nine original
seven-day variants. Stable IDs and tasks remain in ASSESSMENT.json. Families:
reasoning/construction for representation, boundaries and falsification for bit
operations, diagnosis/portability for layout, failure and composition for split
spans/registers/version negotiation. A layout observation cannot substitute for
the register state-machine argument or a portable wire contract.

Prerequisites are T01 reproducible builds, T02 interfaces, T03 same-object bounds,
and the completed PR-T04-01 integer component. Required learner artifacts are the
integer header/implementation, original memory helpers, and compiler manifest.
The reference integrates the actual integer `fi_encode_cells`/`fi_decode_cells`;
subsequent learner integration must identify the learner's implementation. Do not
call a library bitset/codec routine to replace the mechanism. ISO C17 is the core
baseline. Host intrinsics, packed layout and ELF plugins are explicitly separate
compiler/ABI comparisons. No C23 mechanism is claimed by this package.

## Operations, ownership and failure

`FbWord` is at least32 unsigned value bits. Logical width is1..16, independent of
storage size. Bit operations interpret the input modulo2^width, explicitly masking
high carrier bits. Outputs are caller-owned live objects, never retained. Input
span extents must describe actual live arrays; numeric validation cannot prove
that promise. Read-only input and output objects must not overlap unless the
specified operation explicitly permits it. Invalid enum, width, null required
pointer or malformed span returns INVALID; invalid counts/fields return RANGE.
Every rejection leaves caller outputs unchanged. SKIPPED is a separate successful
consumption outcome with explicitly zero value/flags and published consumed size.

| API | Exact contract |
| --- | --- |
| `fb_shift` | admit count0..width-1 before evaluation; LEFT/RIGHT or left rotation; zero rotation succeeds without a width-sized shift; mask input/result |
| `fb_extract` | field is half-open `[low,high)` with0≤low≤high≤width; empty returns0; full returns masked input |
| `fb_insert` | value fits high-low bits; empty accepts only0; other bits preserve masked input; reject excess value bits instead of truncating them |
| `fb_patch` | validate both edits before publication; SEQUENTIAL applies first then second; SIMULTANEOUS admits overlapping assignments only if every overlapping bit agrees, otherwise CONFLICT |
| `fb_stats` | population, highest-set-bit index+1, leading zeros within logical width; zero yields0,0,width |
| `fb_encode/decode` | four logical octets, explicit BIG/LITTLE order; CHAR_BIT must be8; encoding capacity≥4; decoding reads exactly first4 bytes and allows trailing bytes |
| `fb_decode_two` | decode first four bytes of concatenation of two borrowed spans; no pointer between objects, no sum of lengths that can overflow, null only for empty span |
| `fb_record` | parse version/flags/declared size before payload; wire rules below; unknown record either rejects or skips, never exposes a decoded unknown payload |
| `fb_negotiate` | validate descriptor prefix and available extent before any trailing field; explicit version1/2 feature/size rules below |

All scalar bit operations use constant storage; stats costs O(width), currently
bounded by16. Codec/negotiation costs are bounded by the fields actually read, not
by untrusted declared lengths. A supported intrinsic comparison may replace the
stats loop only with explicit zero/logical-width adaptation and capability proof.
Inspect generated instructions/helpers before asserting speed or runtime closure.

## Original teaching register

`FbRegister` owns an8-bit state and read/write counters. Initialization accepts
state0..255 and read_clear0/1. Raw reads/writes require an initialized live object;
counters use defined unsigned wrap and are observations, not synchronization.
Bits0..3 are ordinary writable controls. Writing ones to bits4..7 clears those
status bits, writing zeros preserves them. A raw write replaces all low controls.
An optional read-to-clear mode returns the pre-read byte and then clears all high
status bits. No actual MMIO or operating-system register is accessed.

`fb_register_update(reg,low_value,clear_status)` accepts low_value0..15 and a mask
contained entirely in high4 bits. It sets exactly the low controls and clears
only requested high status, with no read. This is deliberate: reading and writing
back status ones would clear unrequested events, and read-to-clear makes even a
discarded read a mutation. This contract is for the explicit synthetic device;
real hardware requires its own width/order/atomicity/barrier/side-effect facts.

## Wire and binary-interface policies

The value codec maps0x12345678 to BIG `[18,52,86,120]` or LITTLE
`[120,86,52,18]`. Its format is independent of host byte order, struct padding and
bit-field allocation. Decode offsets0,1,3 from a larger character array, and split
after each possible byte across two separately allocated arrays. Lengths0..3 must
reject unchanged. Never form a pointer spanning those arrays before admission.

The teaching record has version at byte0, flags at1, unsigned big-endian size at
2..3, and known payload at4..7. Known version1 permits only flag bit0 and requires
size8. A prefix shorter than4 or declared size beyond available input is SHORT;
size<4 is RANGE. Unknown version/flags are UNSUPPORTED under REJECT_UNKNOWN.
SKIP_UNKNOWN accepts only a fully available declared record, returns SKIPPED and
consumed size, and does not read or interpret its payload. This lets an outer
scanner advance while retaining the distinction between ignored and understood
data. Known malformed sizes still reject under either policy.

Plugin descriptor octets use version BE16 at0, size BE16 at2, features BE32 at4.
Version1 requires size≥8 and defaults max_items=0; version2 requires size≥12 and
reads max_items BE32 at8. Only feature bit0 is understood. Both versions allow
available trailing extension bytes to be ignored, but never permit declared size
beyond actual available extent. Unknown version/features reject. Version1's
absence of max_items must never become an out-of-bounds read or a fabricated
capability. `plugin.h` defines a separate hosted C ABI entry returning required
size and copying the entire descriptor only when supplied capacity suffices.
Version negotiation validates descriptor content, not arbitrary plugin code
safety, pointer truthfulness or another ISA's calling convention.

## Criterion-specific work

| T04 objective | Ordinary submission | Required changed condition |
| --- | --- | --- |
| .05.a | count-1,0,15,16 at logical16; prove admitted operand type/range before shift; zero rotation | width8 inside at-least32 carrier with input0x1234; explicit high-bit masking |
| .05.b | half-open field derivation, empty/full/end-at-width/excess-value cases,0x1234 rotate4 | overlapping `[0,4)` then `[2,6)` edits; derive sequential result and simultaneous conflict/agreement rule |
| .05.c | popcount/width/leading-zero zero policy, all2568-bit inputs against a bit-list oracle | actual supported intrinsic wrapper with guarded zero and adjusted logical width; verify generated code and same contract |
| .06.a | explicit BIG encode/decode with unsigned wide intermediates and supplied vector | separate LITTLE encoding, inverse proof and same decoded value |
| .06.b | character-buffer offsets0,1,3, every short length and output sentinels | two independently owned spans, all splits and unavailable suffixes; no cross-object pointer construction |
| .06.c | explicit codec versus declared packed-object interpretation and unsafe-overlay analysis; reorder fields | unknown version and flags, fully specified reject/skip outcomes before payload, header-only skip case |
| .07.a | actual3/5/9 unsigned-int bit-field size/alignment/representation, compiler target/options | unsigned-long bit-fields or packing change; compare ABI compatibility using observed field widths/placement, no portable ordering assertion |
| .07.b | state trace showing unrequested W1C status loss through read-modify-write, repair and regression | enable read-to-clear; show old returned value versus new stored state and no-read repair |
| .07.c | field widths, offsets, endian/version/compiler options in a binary-interface manifest; consumer-checkable versus trusted facts | independently compile plugin producers/consumers, size query, exact buffer allocation, old/new/unknown/truncated descriptors; no read of absent version1 tail |

Submit `learner.c`, criterion-indexed derivations, independent oracle, layout and
plugin manifests, command records, object/maps/stack audit, failure traces and
changed-condition dossier. Keep original tasks and minimum seven-day delay.
Guided order is logical masks→fields/stats→explicit codec→bounded spans→versioned
records→register state machine/ABI comparison→independent transfer.

Graduated hints: (1) write a logical bit list before a C expression; (2) distinguish
logical width from the promoted carrier width; (3) stage both edits before commit;
(4) model a register read/write as transitions, not ordinary variable access;
(5) inspect the size prefix before deciding which fields exist. A changed packing
flag does not turn a C object into a portable wire format.

## Execution and assessment contract

Run the exported `run.py --cc gcc --source learner.c --integer-source <completed
integer.c> --integer-include <directory-containing-integer.h> --runtime <original
memory-helper.c> --build <new-directory>`, then Clang and your O0/O3/sanitizer
manifest. Missing prerequisites must fail explicitly. The unfinished public
template compiles and fails semantic assertions. A reference path may be supplied
only for author/assessor checks and is never labeled a learner artifact.

The target builder accepts the same three explicit providers, plus `--driver`,
`--cc arm-none-eabi-gcc|clang` and `--opt 0|3`. It directly links five objects for
Cortex-M4/mps2-an386, with original startup/linker script and no runtime archive.
Inspect every object/image/map and stack report, including implicit compiler
helpers. QEMU is an emulation result, not a physical register observation.

Each ordinary and delayed criterion is scored separately:0 absent/wrong;1 code or
recalled rule without assumptions;2 ordinary case correct but boundary/proof
incomplete;3 correct implementation, exact contract, independent evidence and
limits;4 independent changed-condition derivation and defended counterexample.
Require≥3 for every applicable criterion plus all memory/publication/protocol
gates; do not award independent mastery from author reference runs. Record hints,
raw failures, environment/input identities and actual delay rather than a global
complete flag. Passing finite tests does not prove every possible implementation.

Security scope is original local teaching code and synthetic bytes/status. The
harmless success condition for the flawed register operation is cleared synthetic
status that was not requested; for malformed descriptors it is a rejected header
or a bounded wrong-capability mutant. No external target or privileged operation
is involved. Pointer forgery/unsafe overlays remain compile-only analysis; no UB
result is assigned a portable value or called arbitrary code execution.

Sources: named actual N1570 C11 draft integer/shift/struct passages; GCC15.2
bit-operation builtins, packing pragmas and packed attribute documentation.
Exact source captures/limitations are linked through ASSESSMENT.json. C17 remains
the compilation baseline; an inspected C11 draft is not the published C17 text.
