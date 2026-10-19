# B06 numeric generated-code review, 2026-10-17

Author inspection, not independent review. Read the actual retained GCC and Clang
component/FP disassemblies and `.su` reports named by numeric_resource_validation.
The receipt pins their exact bytes and input sources; this prose interprets that
selected run rather than inventing hardware measurements.

Both retained `underflow` functions load runtime operands, execute `mulsd`, store
and reload the volatile result. The complete executables contain MXCSR save/load
instructions; actual tests observe2^-1023 in gradual mode and0 with FTZ, then
restore saved state. This closes the earlier constant-folding weakness for this
experiment. It does not establish an optimizer-independent C floating format.

GCC's component `.text` is3375bytes; Clang's is3030bytes. These are whole object
code extents, excluding linked runtime code, relocation overhead and loaded page
rounding. Their reported `nd_matrix` frames are32992 and33016bytes respectively.
Disassembly confirms each entry reserves the large scratch frame before choosing
the algorithm mode: GCC uses page probes then the remaining subtraction, Clang
one large subtraction. Scalar/blocked modes therefore do not avoid the reserved
transpose capacity in these outputs. Do not describe active scratch0 as stack0.

The pairwise helper frames are80bytes GCC and40bytes Clang; wrappers report32/8.
Actual prologues reveal a convention difference: GCC's number includes its return
address here, whereas Clang's equals pushes plus subtraction. Conservatively add
8bytes to every reported frame for this x86-64 bound. A source longest path for
n<=257 has at most10 pair activations (ceil(log2(257)) splits plus the leaf).
Bounds of40+10×88=920bytes GCC and16+10×48=496bytes Clang cover the component's
selected recursive path conservatively, not caller/library/interrupt frames.
GCC partially inlines recursive levels; treating every source activation as a
full emitted frame overestimates rather than assumes the optimization persists.

The objects contain compiler-generated external calls (including memset and,
for the selected GCC defaults, stack-check failure support). The new `nm -u`
record exposes these dependencies. This hosted measurement is not a strict
freestanding link-closure receipt. A target port must supply/audit its permitted
runtime or change flags with a fresh correctness/resource campaign.

The declared source-access and active-working-set formulas in RESOURCE_ANALYSIS
are derivations, not measured memory-controller counters. Neither the8MiB sweep
nor observed elapsed times identifies every cache level's state. The measurements
therefore support comparisons under the stated workload/flags and equal numeric
contracts, with no universal fastest algorithm or physical board claim.
