"""Append reviewed extension routes while retaining all accepted module bytes."""
import hashlib
from common import ROOT,read_json,write,require_scoped_research_gate,load_taxonomy

def append_guide(name,title,body):
    path=ROOT/f'curriculum/guides/{name}.md'
    marker='<!-- performance-debugging guide extension -->'
    prefix=path.read_text(encoding='utf-8').split(marker,1)[0].rstrip()
    write(path,prefix+'\n\n'+marker+'\n\n## '+title+'\n\n'+body.strip()+'\n')

def main():
    for group in ['performance','debugging','memory']: require_scoped_research_gate(group)
    prior=read_json(ROOT/'reviews/baseline_2026_10_09_architecture_networking/final_acceptance.json')
    pins={r['path']:r['sha256'] for r in prior['canonical_source_snapshot']}
    handoffs={
      'T03':'Carry the object-extent, initialized-state, checked-size and nonzero-resize invariants into {ref}`T45`. Predict the first invalid transition before a tool run; keep lifetime and address-token observations separate.',
      'T13':'Carry the component effect and execution-context ledger into {ref}`T43`. Add explicit cost units, simultaneous storage peaks, completion events and observer overhead to the existing audit.',
      'T15':'Reuse one completed algorithm and its work counters in {ref}`T43`. Derive exact, asymptotic and amortized claims for named input families, then compare them with instruction/cycle and timing evidence.',
      'T16':'Reuse the allocator and ownership histories in {ref}`T45`. Inject one checked-size, failed-resize, cleanup or stale-alias defect and justify the repair without rebuilding the allocator.',
      'T21':'Carry first-cause context, validation and transactional cleanup into {ref}`T44` and {ref}`T45`. Diagnose every partial-failure prefix and retain the smallest same-failure trigger plus a competing-cause counterexample.',
      'T24':'Reuse partial-I/O cursors, descriptor identities and event histories in {ref}`T43` and {ref}`T44`. Measure return versus completion and diagnose short progress, EOF, retry, lost events and cleanup without another event loop.',
      'T25':'Use the existing controlled schedules and synchronization arguments in {ref}`T44`; use borrowing/reentrancy histories in {ref}`T45`. A detector run or longer sleep does not replace the happens-before and lifetime repair argument.',
      'T27':'Carry the actual map and startup range evidence into {ref}`T45`. Separate fixed .bss region overflow, incorrect copy/zero ranges, declared heap/stack collision and runtime stray-write corruption.',
      'T29':'Reuse exception-context and DMA ownership evidence in {ref}`T44` and {ref}`T45`. Distinguish a stopped core from an independently active device; a timeout or cancellation request alone does not permit reclamation.',
      'T30':'Carry the release/start/finish ledger and counter assumptions into {ref}`T43`. Add scoped latency, throughput, jitter and overhead experiments; retain the distinction between observed maxima and a justified bound.',
      'T31':'Reuse matching-image, valid-frame, startup and peripheral triage in {ref}`T44` and {ref}`T45`. Test competing causes for delayed symptoms and state what imprecise faults, damaged frames and debugger perturbation leave unresolved.',
      'T32':'Reuse regression, minimization and separate dynamic/static tool builds in {ref}`T44` and {ref}`T45`. Add prediction/observation/repair dossiers and delayed unseen variants; tool silence retains its configuration and executed-path limits.',
      'T34':'Extend the existing paired consumed-result harness, PMU ledger and build comparisons in {ref}`T43`. New experiments analyze clock/phase/IO/cgroup/compiler/counter evidence; their budgets do not buy a second benchmark framework.',
      'T35':'Carry checked resources and hardening audits into {ref}`T45`. Distinguish a causal contract repair from a mitigation that detects or changes consequences of selected failures.',
      'T37':'Reuse callable assembly, cache and quantitative architecture artifacts in {ref}`T43`. Compare declared work/instruction/CPI models with qualified counter observations; control-flow graph cycles and processor cycles remain separate measures.'
    }
    marker=b'<!-- performance-debugging handoff -->'
    for key,body in handoffs.items():
        path=ROOT/f'curriculum/modules/{key}.md'; relative=path.relative_to(ROOT).as_posix()
        prefix=path.read_bytes().split(marker,1)[0]
        if hashlib.sha256(prefix).hexdigest()!=pins[relative]: raise ValueError('Prior module bytes changed: '+key)
        tail=marker.decode()+'\n\n## Performance and debugging handoff\n\n'+body+'\n\nFollow {ref}`guide-performance-debugging` for the full route. The new project briefs budget only the additional experiment, diagnosis and repair; earlier implementation evidence stays in its original module.\n'
        path.write_bytes(prefix+tail.encode('utf-8'))
    append_guide('reading','Performance and diagnosis spine','''The new tracks assign small actual inspected passages: {ref}`R-clrs-4` for formal and amortized bounds; {ref}`R-ENH-HENNESSY-PATTERSON-6` for quantitative models; {ref}`R-PERF-VITTER-2001` for external-memory units; and {ref}`R-PERF-NIST-SP500-235` for static control-flow complexity. These measures answer different questions.

For diagnosis, {ref}`R-PD-DDMIN-ISSTA2000` supplies a reduction argument, {ref}`R-PD-VIDOVIC-ARM-2026` supplies a held example to critique, and exact GCC/GDB/Arm/ST locators supply the tool and target contracts. For memory experiments, the held {ref}`R-effective-c-2`, historical {ref}`R-MEM-ASAN-PAPER-2012` and versioned sanitizer/Memcheck/linker/source passages distinguish mechanisms from detection limits. A historical overhead figure or development-manual version is not a prediction for an installed setup.

The public performance/debugging research records broad search, reflection, focused search and source synthesis separately. Module reading cards retain exact locators and time allocations. Official documents and original teaching support substitutions where a held commercial source is unavailable; record the substitution and any narrower claim.''')
    append_guide('laboratory','Measurement and fault-experiment builds','''Keep native, debug, optimized, sanitized, coverage, profiling and firmware outputs in distinct build directories. A diagnostic runtime or compiler hook changes the binary and its dependencies; compare against the matching uninstrumented artifact. Record source/image hashes, compiler/linker/tool versions, input and clock/counter identities, actual returned statuses and raw reports.

Store controlled fault cases under a separate learner test target, with finite capacities/timeouts and paired repaired controls. Native exhaustion runs in a learner-owned child process with verified child-only limits and supervisor evidence. Undefined behavior has no promised native crash value or diagnostic string. ASan/UBSan and unsanitized Memcheck runs are separate; conditional MSan needs its own supported instrumented environment.

Use permitted per-process profiling and read-only cgroup observations first. A prepared delegated subtree supports the optional quota experiment. Hardware profiling and fault work reuse the recorded target map, drivers, clock and trace ownership. Static/captured-data analysis is available when a capability is missing, but it leaves the corresponding runtime or physical claim open. Follow {ref}`guide-performance-debugging` and the exact project limits.''')
    append_guide('profiles','Diagnostic and measurement boundaries','''An instrumented investigative variant has its own dependency contract. Sanitizers, profiling hooks and logs can add calls, allocation, code, stack and timing effects. Retain the uninstrumented profile-B/C component or profile-E image audit separately; a diagnostic variant cannot silently stand in for its original dependency or resource promise.

Add named cost units, simultaneous lifetime peaks, completion/reclamation events and observer overhead to the resource ledger. Fixed link sections, whole-path conditional stack estimates, sampled high water, allocator retention, process resident bytes and cgroup accounting are distinct entries. Reuse these ledgers in {ref}`T43`–{ref}`T45`; the directory does not add another implementation budget.''')
    append_guide('inventory','Investigation adapters on existing components','''Add exact work/lifetime counters, bounded event/timestamp adapters, a tri-state reducer, bounded compiler hooks and fault-dossier replay only where the new project asks for them. Reuse the earlier benchmark runner, parser, allocator, algorithm, event loop and driver. The new implementation is the experimental adapter or repair, not a duplicate mechanism library.

Defend cost units and model bounds, identify the first invalid state, preserve cleanup and ownership, and show regression plus delayed transfer. The project directory in {ref}`guide-performance-debugging` maps these additional milestones and their owning budgets.''')
    append_guide('integrations','Use the communication stacks as experimental subjects','''A completed stack provides several narrow subjects for {ref}`T43`–{ref}`T45`: frame/record work and traffic amplification, parser/buffer storage peaks, queueing and completion latency, short I/O and descriptor cleanup, or late DMA completion and borrowed-buffer lifetime. Select the component named by the new project; do not rebuild all three stacks or charge a second capstone.

For T40, keep layer responsibilities and logical record versus link traffic explicit. For T41, distinguish CPU processing, descriptor/device completion and physical observations. For T42, retain HTTP grammar/framing, socket cursor and deadline contracts while collecting cost or diagnosing a controlled defect. A fast response cannot compensate for incorrect framing or unsafe ownership. Existing milestones retain their budgets; the additional experiment belongs to its new owning module.''')
    append_guide('enhancement','Continue into cost and causal diagnosis','''The assembly, architecture and communication artifacts become experimental subjects in {ref}`T43`–{ref}`T45`. Follow {ref}`guide-performance-debugging` for complexity models, hosted/bare-metal profiling, progressive debugger practice and controlled memory/lifetime experiments. Earlier implementations and evidence are reused rather than charged twice.''')
    append_guide('audit','Performance and debugging requirements','''| Requested addition | Concrete teaching and evidence |
|---|---|
| Time/cycle/space and useful additional measures | {ref}`T43`: exact work, formal/amortized bounds, instruction/CPI models, storage peaks, traffic, latency/throughput, jitter and static CFG distinctions |
| Bare-metal, Linux and compiler profiling | {ref}`T43`: paired harness extension, permitted PMU accounting, cgroup/PSI observations, bounded hooks/gprof and qualified DWT/timer work |
| Component runtime and read/write/I/O | {ref}`T43` and {ref}`T44`: submit/return/completion, overlapping phases, CPU versus elapsed time, short progress, errors, buffering and persistence boundaries |
| Beginner through advanced debugging | {ref}`T44`: diagnostics/reduction/stepping/watches, optimized images/cores, I/O/concurrency and reset/fault/ISR/DMA dossiers |
| Common memory and storage traps | {ref}`T45`: bounds/NUL/sizes, exhaustion versus corruption, fixed regions/startup/collision, cleanup/leaks/retention, resize/UAF/invalid release, initialization and asynchronous lifetimes |
| Experiments, drills and prevention | Prediction, negative/repaired controls, causal explanation, invariant repair, regressions and delayed unseen transfer across all three tracks |

The independent content reviews and publishing receipts accept the curriculum and finite supplied-data checks. They do not claim that learner C, actual profiling/debugger/tool runs, child exhaustion or target hardware tests have been performed. Unavailable observations keep their associated claims open.''')
    taxonomy=load_taxonomy(); topics=sum(len(d['subtopics']) for d in taxonomy); objectives=sum(len(t['objectives']) for d in taxonomy for t in d['subtopics'])
    audit=ROOT/'curriculum/guides/audit.md'; text=audit.read_text(encoding='utf-8')
    first=text.split('\n\n',2)
    first[1]=f'The three original inputs remain unchanged: 35 domains, 395 topics and 1,185 exact objectives. The architecture/networking extension preserves its seven domains, 66 topics and 198 objectives. The performance/debugging extension adds three domains, 36 topics and 108 objectives. The combined map contains {len(taxonomy)} domains, {topics} topics and {objectives:,} objectives. Generated checklists link each objective to teaching and a local activity; integration and drill directories add reuse rather than replacing coverage.'
    write(audit,'\n\n'.join(first))
    print('Integrated shared routes and appended handoffs; prior module bytes retained.')

if __name__=='__main__': main()
