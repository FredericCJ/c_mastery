# T64 required accuracy and resource continuation

Retain PR-T64-05/06 and all their a/b/c objectives, EXTENSIONS.md contracts and
the B06 shared contract. Families A/C/F/H/I/J. Prerequisites are the learner
matrix kernels, binary64 error analysis, fixed tree and equal-contract timing
artifacts. C17 implementation, selected x86-64 binary64/SSE environment and
explicit GCC/Clang flags are assumptions; objdump/size/nm and compiler stack
reports are host evidence, not target dependencies.

## N9: inspect the operation that actually underflows

D1: compile the runtime-operand experiment under both compilers with O3,
frounding-math, no fast math, no contraction and retained helper calls. Inspect
the named underflow function for actual scalar SSE multiplication and actual
MXCSR save/load instructions. Execute gradual DBL_MIN×0.5 and FTZ results before
restoring the original process-local state. Do not accept a constant-folded
zero or source-only loop count as generated-code evidence. The tool does not
change host settings; all FP state changes are local to its process and restored.

Revise the error contract: gradual2^-1023 versus FTZ0 differs by2^-1023 absolute,
relative error1 against the nonzero gradual result, and2^51 subnormal steps in
the selected binary64 representation. A tolerance based only on relative error
rejects the flushed result; an absolute near-zero allowance can accept it only
if explicitly chosen for the application. Signed zero, NaN/Inf admission and DAZ
input behavior remain distinct questions. Use the existing ULP/absolute/relative
predicate and expose which policy changed, not merely the expected constant.

## N10: measured code/frame bounds and a declared traffic model

D2: compile the component at the same ordinary O3/no-fast-math/no-contraction
semantics used by the timing experiment, adding only stack-report generation.
Retain section sizes, symbol byte extents, disassembly and static frame reports.
The fixed transpose scratch is4096 doubles=32768bytes capacity; inspect whether
the generated matrix entry reserves it even for scalar/blocked mode. Do not
infer stack use from only the branch that executes. For recursive pairwise
reduction, derive the longest call chain for n<=257 and combine actual generated
frames; inlining/cloning requires revisiting the bound. Report runtime/library
frames and interrupt stack separately as unknown unless actually measured.

D3: state the source-access model before counting. Validation reads mk+kn input
elements; initialization writes mn; arithmetic performs mnk multiplies and adds.
The literal loop expression has two input reads and an output read/write per
inner iteration, but optimized register accumulation can eliminate some output
traffic. Transposition adds kn reads and kn scratch writes. Scalar active data is
8(mk+kn+mn)bytes; transpose adds8kn active scratch although the fixed capacity is
32768. A complete3×3 A/B/output tile contains27 doubles=216bytes when all three
tiles are resident. These are working-set/access formulas, not measured cache
misses or DRAM transactions. Explain tails, traversal order and why blocked reuse
depends on actual cache/compiler behavior.

D4: correlate these bounds with existing raw timing samples, which include
validation, transpose and full calls. The8MiB sweep is a changed working-set
condition, not proof that every cache is cold. Preserve all samples and avoid a
universal speedup claim. Run `python3 tools/run_b06_numeric_resources.py`; require
actual instruction/outcome checks and nonempty code/frame reports in both
compilers. An incorrect retained-operation or missing stack report fails the
evidence requirement even if numerical outputs pass.

N9 rubric maps T64.05.a/b/c to D1 and error-policy reasoning; N10 maps T64.06.a/b/c
to D2/D3/D4. Level0 equates source appearance with emitted code or source loads
with DRAM reads; level1 supplies elapsed times alone; level2 supplies the complete
named-target evidence and bounds; level3 independently changes matrix shape,
available stack or subnormal policy after seven days. Hints: retain a runtime
operand; separate static frame capacity from active elements; count exact source
operations before hypothesizing a cache effect. Assessor review of observed
assembly/frame evidence is in `research/v5/expansion/b06/numeric_codegen_review.md`
and must be treated as assisted material if consulted during assessment.

Only original synthetic numerical programs run locally. No external targets,
host floating-mode changes outside the process or physical board claims. The
source-bound report is numeric_resource_validation.json. Source semantics use
the inspected N1570 floating-environment and Intel FTZ passages in the followup
register; exact ISO C17 text remains an explicit access gap.
