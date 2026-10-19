# Exact P6 private derivations

## AX-PR-P6-01-T17.02.b

Unchanged numeric tests force binary64 intermediate rounding and ordered/reordered cancelling FIR results0/1; companion records error against rational1 and refuses contract-invalid numerical output.

Changed condition: Exact widened int32 convolution and denominator4, signed ties-even and unchanged range refusal are exercised by4096 independent convolutions plus20003 rounding values per native profile. Derive int64 bounds rather than reuse a floating tolerance.

## AX-PR-P6-01-T17.06.b

Actual unchanged FIR produces exact numerators0/4/16/24/16 and rational outputs0/1/4/6/4, with zero prehistory and explicit int32/int64 scaling. It is distinct from V4Filter.

Changed condition: Actual half-pole IIR impulse and every split preserve state. Derive geometric absolute summability for|a|<1 and explicit a1/>1 counterexamples; no higher-order stability claim.

## AX-PR-P6-01-T17.06.c

All splits, capacity/domain refusals and every transaction failure ordinal exercise both accepted-prefix and atomic rollback policies, followed by correct suffix continuation.

Changed condition: Actual scalar exact in-place operation saves current input; both partial overlap directions are refused unchanged. The stronger staged vector wrapper separately accepts both partial directions.

## AX-PR-P6-01-T34.01.b

Public policy rejects nonfinite/out-of-domain data and admits telemetry only under2^-40 absolute plus2^-40 relative error. Tiny/tail/cancellation/large fixtures and semantic mutants gate later timing.

Changed condition: Audit compares binary64 object bytes and rejects near-zero2^-44 and signed-zero differences that telemetry permits; ordered FIR results remain bitwise across scalar/compiler/AVX. Contraction/reassociation remain disabled.

## AX-PR-P6-02-T34.06.b

Actual GCC/Clang reports and disassembly identify ordered reduction/dependency/alias effects. Internal staged history is disjoint from output under truthful restrict; unaligned loads make no fabricated32-byte alignment promise.

Changed condition: The unchanged public vector wrapper stages all inputs before stores and accepts both overlapping directions. Protected-page tests exercise permitted offsets without running illegal restrict overlap.

## AX-PR-P6-02-T34.07.b

Actual provider checks CPUID AVX/OSXSAVE before XGETBV and XCR0 bits1/2. Native tests exercise lengths0..256; protected-page companion covers0..9, both boundaries and four offsets, with scalar tails and independent expected outputs.

Changed condition: Four actual retained native binaries run in QEMU Nehalem with avx_usable0, auto selects scalar and forced AVX refuses unchanged. This satisfies the selected emulator fallback experiment, not physical no-AVX hardware observation.

## AX-PR-P6-02-T34.09.a

Unchanged paired harness generates inputs/allocations outside timing, includes validation/staging/reset explicitly, verifies every output afterward and consumes results. Raw empty overhead and exact binaries/maps are retained.

Changed condition: GCC O3/LTO and Clang O3 pass actual scalar/compiler/AVX code gates and retain calls. Clang LTO actually introduces packed arithmetic into nominal scalar_kernel: its correctness-only execution is retained but its timing category is rejected. Exact binaries/maps/disassembly remain reviewable.

## AX-PR-P6-02-T34.09.c

Three accepted actual native profiles retain thirteen rotated paired samples per workload/cache/backend, CPU/ISA/compiler/flags/clock/affinity observations and median/min/max. Every correctness failure would fail the campaign; synthetic fixture numbers remain arithmetic.

Changed condition: Predicted footprints cross observed48KiB/1280KiB/30720KiB cache metadata. Actual n256 block1/64/1024/16384 distributions and paired ratios test the transition without demanding monotonicity or claiming exact cache residency.

