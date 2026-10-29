# Bounded trace, exact release identity and startup evidence

## 1. Identity and families

This additive PR-T31-04 contract assesses T31.07.a–c and T31.08.a–c, including each original delayed variant. Families A/B/C/D/F/G/H/I/J apply: derive, implement, falsify and transfer the mechanisms below. The historical eight hours and original acceptance records are unchanged. This is required expansion work without a learner-time ceiling.

## 2. Objectives and inspected references

T31.07.a distinguishes counter existence, enablement, width, frequency and interval validity. T31.07.b assesses bounded record ownership, loss and nested interrupt serialization. T31.07.c separates record cost, drain cost and observer effects. T31.08.a distinguishes file, virtual and load extents. T31.08.b binds attribution to the exact release. T31.08.c establishes initialized state and validates retained state after a reset.

Exact inspected passages and limitations are registered in `research/v5/expansion/legacy_embedded_debug_release/resources.json`: Arm DDI0403E.e DWT/CYCCNT and DEMCR passages, ELF program headers/PT_LOAD, GNU objcopy binary/strip/debug behavior and the original project/fixture/criteria. Reused CMSDK source passages have their existing resource identity. These are author inspections, not independent review. Published C17 text is unavailable; N1570 reasoning must be labeled as such.

## 3. Prerequisites and artifacts

Bring the original PR-T31-01 connection evidence, PR-T31-02 fault record/ELF attribution, and PR-T31-03 ownership/timestamp diagnosis; retain their exact IDs and limitations. Bring PR-T07-03 map/ELF inspection and PR-T11-01 freestanding dependency audit. Know unsigned modular arithmetic, distinct object lifetimes, interrupt masking, reset startup and load-versus-execution addresses. Supply your completed `release.c`, `target.c`, and host `manifest.py`; all provided versions remain deliberately unsolved. Host Python hashing, ELF inspection and independent oracles are verification tools, never target dependencies.

## 4. Execution and dependency profile

Core code is ISO C17, fixed storage, no allocation or library implementation of the taught queue/duration/startup mechanism. Hosted tests use GCC and Clang debug, optimized and ASan/UBSan profiles. Target code is the explicit GNU/Clang Arm extension and Cortex-M4 Thumb/soft-float ABI, QEMU `mps2-an386`, original startup/linker script, and explicitly supplied C runtime helper source. Use no libc, libgcc, compiler-rt or atomic runtime archive; inspect symbols, map, assembly, stack usage and source list. The selected CMSDK clock is the emulator board's 25 MHz virtual timer clock, not CYCCNT or a physical processor measurement. The host release parser accepts only the declared little-endian ELF32 Arm EXEC subset with at most sixteen program headers. No hardware flashing command is supplied or executed.

## 5. Concrete problem, inputs and outputs

Implement the public API exactly. Initialize `struct dt_trace` to zero; its three slots contain copied `{sequence,kind,timestamp,payload}` records. Emit attempts receive consecutive IDs, including full-queue drops. Start at10, fill with10/11/12, drop13, drain, then observe14. Drop-new leaves queued records unchanged. Count drops; do not wrap either finite counter. `UINT32_MAX` is reserved retirement for `next`; the final admitted ID is MAX−1. A saturated drop counter refuses a further drop unchanged. Draining remains legal after retirement. Empty/refused takes preserve the caller's output.

`struct dt_critical` provides reliable save-mask/restore-mask callbacks, the same live context and callbacks for every access to this queue. Demonstrate low-priority IRQ0 attempting a record while higher-priority IRQ1 becomes pending during entry. Both producer paths and task draining use this port. Preserve an already-set PRIMASK. Scope is one core with maskable interrupts; NMI, HardFault, SMP, arbitrary concurrent C threads and callers bypassing the port are excluded. Callbacks do not reenter queue operations or retain borrowed storage. A plain single-producer queue is not thereby an MPMC algorithm.

For `dt_duration`, provide one to eight `{before,after,frequency,width}` intervals, width1–32 and nonzero frequency. Each interval has a proven elapsed count strictly below one wrap, fixed frequency and counting state throughout. Split at frequency changes. Return sum of per-piece floor and ceiling nanoseconds, transactionally; reject invalid inputs or an unrepresentable total. Eight-bit250→4 is10 ticks; ten ticks at1MHz plus twenty at2MHz is20000ns. One tick at3Hz requires [333333333,333333334]ns. Samples cannot establish a missing wrap bound or infer that a halted counter measured wall time.

For `dt_image`, validate base/offset word alignment, at least eight vector bytes, and representable exclusive end. Base08000000/size1024/vector-offset256 gives vector08000100/end08000400. It checks arithmetic only, not actual mapping, MSP, permissions or device identity.

Build the supplied `layout.c` using `layout.ld`: executable Flash span08000000, initialized RAM VMA20000000 with load address08001000, and zero-fill BSS. Build the changed text origin08000100 too. Use actual readelf/objcopy to obtain headers, raw binary with FF-filled load holes, stripped deployment ELF, separate debug file and map. Implement `segments`, `build` and `attribute` in `manifest.py` with the supplied signatures. Check all file and address extents, file-size≤memory-size, alignment, virtual/load overlaps, flags and executable file-backed entry. `p_paddr` as load address is a selected GNU embedded convention; the generic System V ABI leaves physical addresses unspecified for its usual applications. A zero-fill tail is RAM initialization, not bytes to program. Reject substituted layout/map/symbols/revision/flags/target or capture identity. Hash the exact original ELF, deployment, separate symbols and map, plus canonical flags/source revision/target and segment descriptors. The capture names this manifest identity and an even Thumb instruction address inside an executable file-backed segment. Hash equality supplies consistency under trusted input provenance, not authentication or proof of what ran on hardware.

`dt_startup` copies eight initialized bytes and zeros eight BSS bytes from a deliberately poisoned live `struct dt_ram`. Cold startup discards the retained record. Warm startup retains only version1, magic44545231, the current nonzero image token and the specified XOR check. The check detects selected corruption only and is not cryptographic authentication. The actual M4 guest poisons data/BSS before both boots, retains `.noinit`, requests SYSRESETREQ, then checks initialized cookie42/zero BSS/current retained token. Its small token42 is not the host manifest SHA. Software reset is not physical power loss; no `.noinit` persistence guarantee follows for another reset class or board.

## 6. Why each mechanism is needed

The initial author campaign exposed another boundary: implicit GNU program headers for shifted text included ELF header bytes inside PT_LOAD. Stripping changed those loaded bytes, so exact deployment equivalence correctly failed. The supplied linker script now declares the two load segments explicitly; retain an implicit-header negative control. Do not ignore differences merely because the instruction bytes still look equal.

Reject these plausible explanations with counterexamples: every Cortex-M4 has a working cycle counter; unsigned subtraction measures any elapsed duration; a dropped record need not consume an ID; clearing PRIMASK on leave restores the caller; tracing overhead is a universal constant; ELF file size is Flash range; RAM VMA is its load address; stripped symbols destroy release identity; a fault address can be decoded with any nearby build; reset guarantees zero RAM; a checksum authenticates a crash record. Each has a distinct tested boundary or explicitly unobserved premise.

## 7. Required reasoning and invariants

Prove ring head/count bounds, FIFO record copying, attempt-ID monotonicity, loss accounting and cleanup after finite retirement. Establish that saved-mask critical sections cover publication and preserve nesting; derive why the higher-priority producer cannot observe a partial lower-priority record. All objects are live, suitably aligned, initialized according to this contract and nonoverlapping; controls/outputs cannot alias records or input arrays. No mutation/reentry occurs through callbacks. C cannot enforce these premises.

Derive the modulo delta under its strict wrap premise, checked widened product and outward rounding; bound each intermediate and staged sum. Explain why summing per-piece rounded bounds may widen uncertainty. Check ELF extents before slicing; use file-backed bytes for PC attribution, not BSS or holes. Startup must not clear the retained area accidentally, and invalid retained state must never become authoritative. Queue operations are O(1), duration O(n≤8), parser O(p²+input bytes) for p≤16, memory fixed except host-owned copied manifest data. Report code/stack size from the actual target build.

## 8. Implementation and experimental deliverables

Submit source, contracts, invariant arguments, dependency audit, exact build artifacts and raw observations. Execute `tests.c` and `replay.c`; check complete output using `oracle.py`, not a pass marker alone. Use `guest.py` with explicit completed source/driver/helper paths. Probe DWT capability/enable/read progression before accepting it as a clock. If it does not move, record that result and use the separately identified CMSDK timer for the experiment; do not fabricate CYCCNT values. For sleep/halt analysis state the exact timer, board clock and stopped/running conditions; use the retained PR-T30-04 emulator halt experiment only with its exact scope or execute a new controlled observation.

Measure seven raw samples for the identical300-item volatile workload in each mode: disabled, record/drop without drain, record plus drain. Keep checksum44850 and logical work identical. The no-drain mode must drop297 records; distinguish that policy from lossless draining. Retain first-use translation/cache effects rather than selecting favorable samples. Run the supplied original periodic-interference model to falsify constant-additive overhead; it is not a cache or physical IRQ measurement. Plan a real-board follow-up with calibrated frequency, counter halt/sleep behavior, interrupt workload, repeated paired runs, uncertainty and raw artifacts. Physical execution is separate required evidence when that environment is available.

## 9. Verification and acceptance

All admitted cases and unchanged-output failures must match independent expectations. Require six host profiles, every deliberate semantic mutant rejected by its intended assertion after successful compilation, and unsolved stubs rejected. The complete independent replay covers4096 queue histories/49152 operations and1024 piecewise conversions per profile. Require three M4 compiler/optimization profiles, nested-IRQ serialization, exact mask restoration, actual poisoned-startup/software-reset checks, positive dependency audits and intended target-control failures. Require actual original and shifted ELF builds, exact readelf segment agreement, FF-hole/data/BSS checks, and all artifact-substitution controls. A crash, compiler failure or sanitizer finding is not a successful semantic negative control. Recalculate source/log hashes; changed sources invalidate the corresponding evidence until rerun.

## 10. Rubric per original objective

For every row, level0 is missing/contradictory evidence; level1 is a happy-path implementation or unexplained test output; level2 requires the stated contracts, boundaries, independent expected result and diagnosis; level3 requires the named changed-condition transfer with an unaided explanation after the original minimum seven-day delay. Author execution is not learner assessment.

* T31.07.a: establish counter premises and modulo/rounding proof; independently predict piecewise frequency conversion, overflow and unavailable-counter outcomes.
* T31.07.b: explain attempted versus delivered sequence, saturation and saved-mask serialization; demonstrate actual nested producers and diagnose the unmasked control.
* T31.07.c: preserve equivalent workload, raw timing/loss/drain evidence and uncertainty; explain a nonadditive contention counterexample without claiming model numbers were measured.
* T31.08.a: derive checked vector and segmented load extents, holes/BSS/RAM distinctions; diagnose changed load layout without programming a device.
* T31.08.b: reconstruct exact release/capture consistency, reject all substitutions and preserve separate debug symbols; state trusted provenance limits.
* T31.08.c: show poisoned RAM restoration, retained-record policy and reset-class limits; execute warm-version/image mismatch and distinguish QEMU software reset from cold physical startup.

## 11. Graduated hints and assessor separation

Hint1: write state transitions before code and mark what is preserved on refusal. Hint2: full emit increments attempt/loss counters but not ring occupancy; interval rounding needs quotient and remainder. Hint3: compare VMA/load/file/memory columns separately and stage the manifest before publishing it. Hint4: identify exactly which linker ranges are poisoned, copied, cleared and retained. Assessor source/analysis is distributed separately and must never be copied into a learner workspace.

## 12. Changed assumptions and delayed transfer

Retain all six original delayed IDs and minimum seven-day requirement. They respectively require piecewise frequency conversion, multiple interrupt producers with a separate synchronization argument, contention-sensitive observer effects, segmented ELF holes/RAM loads, stripped deployment plus bound symbols, and warm retained-record version/validity. These are implemented author-side controls and supplied unsolved learner tasks. Repeat under a different compiler/optimization profile and revise proofs if width, reset class, interrupt model or load convention changes. Do not claim the emulator validates an F439 Flash controller or physical cold-start behavior.

## 13. Laboratory containment

Only original synthetic records, original generated ELF files and the dedicated QEMU image are targets. No host security changes, hardware flashing, deployed firmware, external services, credentials or other people's data. Mutants alter only the teaching implementation. Success is a harmless record/order/startup/identity assertion or its expected local failure. Repair each defect, retain its regression, and state the residual premises; artifact integrity checks do not establish secure boot.

## 14. Status and limits

Authored implementation and executable fixtures are distinct from current source-bound campaign results, retained in `reviews/v5/expansion/legacy_embedded_debug_release/`. Independent human review and delayed learner execution are not recorded by author campaigns. Physical programming, calibrated board measurements, physical power loss and silicon reset persistence remain unobserved. Finite tests and oracles can falsify errors under stated bounds; they do not prove absolute mastery or every execution.
