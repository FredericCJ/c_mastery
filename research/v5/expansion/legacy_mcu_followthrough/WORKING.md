# T50 successor authoring in progress

This is an author work note, not completion evidence. Canonical assessments
are not attached and no support closure exists yet.

Ownership: `/root/build_workflows`; new `labs/mcu_followthrough`,
`assessor/mcu_followthrough`, `research/v5/expansion/legacy_mcu_followthrough`,
`reviews/v5/expansion/legacy_mcu_followthrough` and narrowly named tools.
Do not edit frozen `assessor/execution`, T48 providers, or other workers' paths.

All36 exact T50 original criterion tasks,36 delayed variants,12 original
project contracts and transfer challenges,12 fixtures, module T50 and current
execution provider sources have been inspected. Existing X-T50-001 is only a
modular-timer subset. Existing stage1 is PendSV-mediated, not a direct
ABI-call context switch; original raw IDs are not public generation handles;
cmx_fault is image-fatal; baseline SysTick remains periodic.

Current implementation: `core/mcu.h`, private `core/mcu.c`, public tests and
run.py. Generation-checked task wrapper binds the actual `cmx.c`; permanent
idle, exact initial integer frame, top alignment for odd word counts,
transactional two-step admission, scan selection excluding idle, single-result
join, execution-owner-gated reap and generation retirement. Fault wrapper
retains first cause and halts scheduling for inconsistent owned resources;
safe unowned faults detach waits and permit peers. Independent timers use
stable sequence/generation, half-range admission, saturated retirement.
Additional primitives implement bounded long division, periodic skip policy,
timer ceil/split conversion, priority encoding, and deterministic sleep/event
accounting. Actual target paths are still required.

Pilot reference execution passes eight distinct groups with GCC O2 and Clang
O0, using the frozen original cmx.c. `core_pilot1` was a shell invocation
failure (empty argument), not semantic evidence. `core_pilot2` and
`core_clang_pilot1` are successful provisional runs. No final source-bound
campaign or mutation/export guard has run. Public incomplete learner.c still
needs authoring.

Planned next target work:

1. Original reset/vector/linker with data in two regions, BSS loader poison,
   retained region, wrong-LMA/BSS and dependency controls; explicit original
   division helpers and width/compiler dependency observations.
2. True Thumb ABI-call switch, three disjoint stacks, nested C and forced
   r4-r11 sentinels, entry-return trampoline, high FP/FPSCR variant; omitted
   register and bad trampoline controls, O0/O2/frame-pointer alternatives.
3. Original PendSV/SVC/SysTick successor binding Mf and cmx, forced all integer
   and FP sentinels, lazy/eager and nested handlers, deferred/coalesced switch
   evidence, mask restoration/context refusal, task-local UsageFault and MPU
   guard variant, explicit fatal poisoned-owner path, actual stack handoff.
4. Actual one-shot/tickless timer adapter with continuously running time source,
   wake-before-sleep injection, early/full/split intervals and no double credit;
   clock-stops-in-sleep counterexample remains separate.
5. Bounded acquisition/compute/output integration, overload/recovery diagnostic
   backpressure, borrowed DMA lifetime changed premise, all original exact
   criteria/variants/brief transfers, separate assessor derivations, source
   ledger and all exports/guards before attaching72 IDs.

Sources inspected now: cached `build/execution-sources/aapcs32.rst` pinned
bd017eeca341c06915b9a02ae111b480416d248f (2025Q4): r4-r11/r9 platform role,
s16-s31, FPSCR policy, public SP modulo8 and universal stack constraints.
Official raw URL opened again. PM0214 revision10 cached text: pp42-44 exception
entry/frame/EXC_RETURN, pp48-49 WFI/WFE wake, pp246-253 SysTick layout/control/
reload/current/calibration and FPU access. Figure12 p43 was rendered with
PyMuPDF and actually inspected; PNG in reviews/.../sources/pm0214-frame.png.
`pdftoppm` is unavailable in WSL; PyMuPDF renders successfully. PDF skill read
and announced; read-only work requires no artifact operation marker. Continue
FPCCR pp253-255 and selected fault/MPU register inspection before implementation.
Retain exact revisions, no physical or whole-manual claim. No published C17
wording newly inspected.

After T50: parent also assigned exact T54,T55,T56 reconciliation and any
remaining implementations/changed conditions. Modules T54/T55/T56 study
sequences read; exact criteria/briefs and current newer providers still need
focused audits. Existing broad ledger is legacy_amp_services/depth_dispositions
(96 original criteria); it is historical, not closure. Newer advanced_concurrency
and service adapter packages may match subsets only. Preserve all existing
providers and do not infer criterion completion from a general family anchor.

T48 is complete/frozen separately: support SHA
242bda3d3fe1fded2a8adfa1a61b01fff859a632da16e94f2385ca9741ab2ca8;
campaign fc4060636e5677ff87adcad12c6ff4b292f1afb176027e5500307e46627a7287;
guard128 PASS. Runtime frozen report remains
4762346850eb4388b7d3f2269b2c4f2c71d688db1a9eab012a90c46bf70df730.
Root received guard invocation and exact78-ID scope. Do not change its sources.
