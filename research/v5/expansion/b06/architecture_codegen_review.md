# AP7 author generated-code review

Actually inspected on2026-10-17: all36 source-bound helper/frame/section rows and
selected optimized RV64I/RV64IMA/Armv8/LSE disassemblies for32/64/128-bit lowering.
This is an author review of Clang21.1.8 artifacts, not an external review or
execution of the closure-only images. Exact commands, maps, raw disassembly,
object hashes and compiler stack reports are in `closure_validation.json`.

RV64I emits `__atomic_fetch_add_4`, `_8` or `_16` according to width and
`__udivdi3` for division, at every optimization. RV64IMA replaces32/64-bit calls
with `amoadd.w.aqrl`/`amoadd.d.aqrl` and division with `divu`;128-bit addition still
requires `__atomic_fetch_add_16`. All12 configurations retaining any helper fail
the deliberate no-runtime link with the precise required symbol names. All24
helper-free configurations link and have no final undefined references. Merely
using `-ffreestanding` does not supply or authorize a runtime provider.

For Armv8 without outlining, optimized64-bit addition is `ldaxr`/`stlxr` with a
retry branch;128-bit addition uses `ldaxp`, carry-propagating arithmetic and
`stlxp` with retry. Armv8.1+LSE uses `ldaddal` at32/64 bits, but128-bit addition
uses `caspal` in a compare/retry loop. LSE therefore does not eliminate every
retry loop. The initial ordinary pair load in the128-bit sequence only seeds
CAS: atomic success validates the observed value. Inferring atomicity from that
initial load alone would misread the implementation. `udiv` handles unsigned64
division in all Arm rows; no provider is emitted.

AtO2/Os, RV64IMA32/64 total `.text` is80 bytes and its two leaf frames are zero;
`_start` reserves32 bytes. Armv8's corresponding text is92 bytes and LSE76, with
the same zero leaf/32-byte entry frames. At128 bits Armv8 text104 grows to128
with LSE, while entry frames are48 in both. This concrete counterexample rejects
“new ISA always means smaller code.” O0 frames/code are larger in these selected
rows: Armv8 width128 has96-byte atomic frame and384-byte text versus0 and104
atO2. The report retains every row rather than extrapolating these examples.

Helper calls make full stack/resource closure unknown until provider inspection.
Even helper-free static frame sums exclude exception nesting and runtime startup.
These probe entries never establish SP, privilege or initial storage for target
execution; they are deliberately link-only images. The separate actual guests
perform their own startup and execution. No elapsed-time, lock-free fairness,
interrupt-safety or universal size result is inferred from instruction counts.
The private binutils2.46 disassemblers were extracted from Ubuntu distribution
packages without installing or changing system configuration.
