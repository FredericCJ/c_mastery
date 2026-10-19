# PR-T37-02 technical analysis (withheld)

T37.03.a: the three actual values are 11223344556677AA,
112233445566BBCC and 0000000089ABCDEF. The 32-bit write is a zero-extension
rule in long mode, not a C unsigned-conversion rule. MOVS immediate writes the
whole 32-bit Thumb destination. The timing loop makes both candidates return
the same value; an AL write retains a merge dependency while EAX can break it.
The next AND also zeros upper bits. Therefore this is a measured dependency
investigation with instruction-size/front-end confounds, not a measured
universal partial-register penalty. Inspect raw paired samples and migrations.

T37.03.b: `index*scale+disp` is a coordinate, not yet a pointer. Check scaling,
signed displacement and `width<=extent-pos` before base addition. Negating
INT64_MIN directly is wrong; `-(disp+1)+1` uses a representable signed step
then unsigned magnitude. The EXTENT mutant accepts position20/width4 in an
extent23; no invalid memory access is needed to demonstrate the defect.
Wire bytes are collected by unsigned shifts; negative records are reconstructed
with `-(INT32_MAX-distance)-1` equivalent arithmetic, never an out-of-range
unsigned-to-signed cast. The ENDIAN mutant is caught by the fixed known word,
not merely an encoder/decoder round trip sharing the same error.

T37.03.c: a declared pool has the proper type from declaration. Actual malloc
storage has no declared type and is initialized through the typed lvalue;
the byte-copy operation is between live initialized typed records. The aligned
unsigned-character array retains its declared type; do not execute the bad
cast or interpret compiler silence as acceptance of its semantics. The actual
linked CCM object is a typed, aligned, CPU-accessible counterexample to the
claim that those properties imply DMA reachability. RM0090 p68 explicitly
limits CCM to the CPU. The SRAM candidate still needs a DMA configuration,
ownership and quiescence argument in a later peripheral experiment.

T37.04.a: `48` selects REX.W; `3D` selects accumulator CMP; the four following
little-endian FF bytes form imm32 −1, sign-extended to64 bits. Thumb05 2A is
halfword2A05, CMP-immediate T1 with Rn2/imm5. It is a16-bit instruction operating
on32-bit values. All other short instructions are excluded. Wide-prefix
classification is exhaustive but is not a decoder for wide instructions.
The extension toggle must not silently broaden the original base mode;
EXTENSION defeats exactly that promise and fails the preservation assertion.

T37.04.b: JA uses next-PC1008 plus signed−2, hence1006. The PC mutant uses
current PC and returns1004. Thumb selected B uses PC+4+2*signextend(imm8).
DE/DF are not conditional B in this scope. Near integer boundaries the API
rejects arithmetic wrap; this admission choice differs from general ISA
address semantics and must be stated, not smuggled into an ISA claim.

T37.04.c: assembler selection is itself evidence. GAS chooses48 83 F8 FF for
CMP RAX,−1. The original48 3D form is intentionally emitted with directives,
then independently disassembled and compared to the manual table. Genuine
AT&T/Intel mnemonic comparisons with12345678 force imm32 and match exactly.
The changed32-bit operand loses REX.W and therefore cannot be called a
syntax-only variation. The unknown shorter form must remain unsupported by
the finite decoder even though it is a valid instruction.

Assess full prepublication state on errors, and require an independent oracle
for each layer: arithmetic big integers, Python explicit-endian bytes, manual
opcode fields, actual objcopy bytes and linker symbol intervals. Do not accept
the C test's own expected expression as the only oracle. The campaign runs
five defined semantic mutants on host and emulator, plus separately compiled
learner stubs. None intentionally executes a forbidden pointer access.

The finite halfword space, selected relative offsets and raw timing samples
provide bounded author evidence. They do not establish independent learner
mastery, external assessment, a general disassembler or physical board timing.
