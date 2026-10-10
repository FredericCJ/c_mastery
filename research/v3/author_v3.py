"""Author the additive OS/concurrency curriculum from explicit milestone records.

Only T49--T56 and fixtures/os_v3 are owned here. Run from any directory.
"""
from pathlib import Path
import json, runpy

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
DATA = {}
for name in ('topics_49_50.py', 'topics_51_52.py', 'topics_53_54.py', 'topics_55_56.py'):
    DATA.update(runpy.run_path(str(HERE / name))['DOMAINS'])
SPECS = runpy.run_path(str(HERE / 'oracles.py'))['SPECS']

META = {
49: ('Operating-system mechanisms and the Linux observatory', 'linux', ['T23','T24','T25','T37'], ['A','C'], ['Linux/WSL with procfs, GCC/Clang and debugger','Disposable cgroup subtree when delegation is available'], 'Treat a process as an ownership and protection boundary, a thread as an execution context, and scheduling as a policy over runnable contexts. Build models before comparing their predictions with observed Linux behavior.', [('R-OS3-OSPP-II','5.7.1 and 5.7.3 introductory paragraphs: uniprocessor and multiprocessor exclusion',2),('R-OS3-OSPP-II','7.2 introductory paragraphs: multiprocessor scheduling',2),('R-SYS-TLPI-2010','§3.4 Handling Errors from System Calls and Library Functions, pp. 48–55',2)], 'os_observatory'),
50: ('An original freestanding preemptive task runtime', 'bare-metal', ['T29','T30','T33','T49'], ['C','E'], ['Cortex-M4F compiler and emulator','Existing F439 board for physical timing; original startup and link scripts'], 'Start with a callable assembly context switch, then add preemption and task lifecycle. The kernel being built supplies its own services; no existing OS, libc, libgcc, compiler-rt, libatomic, vendor HAL or OpenAMP is linked into custom images.', [('R-OS3-OSPP-II','4.4.1 and 4.6.3 introductory paragraphs: TCB and context switching',3),('R-FREERTOS-CM4F-1120','xPortPendSVHandler',2),('R-OS3-ARM-DAI0298A','Printed pp.4 and15: FP frame ownership and PendSV',2)], 'execution'),
51: ('Synchronization and bounded inter-task communication', 'cross-track', ['T25','T50'], ['C','E'], ['Deterministic interleaving driver','Original runtime; MCU/emulator and Linux host'], 'A synchronization primitive is a state machine with ownership, progress and lifetime rules. Implement the wait protocol once, then derive blocking objects without duplicating its enrollment and timeout races.', [('R-OS3-OSPP-II','5.3.1 and 5.4.1: lock and condition-variable definitions',2),('R-OS3-OSPP-II','5.7.1 and 5.7.3 introductory paragraphs: uniprocessor and multiprocessor exclusion',2),('R-OS3-DIJKSTRA-EWD123','3.2 Synchronizing primitives; 4.3 Bounded buffer',2)], 'execution'),
52: ('Real-time scheduling, interference and admission', 'cross-track', ['T30','T37','T51'], ['C','E'], ['Deterministic job scheduler','F439 timing capture','Pinned FreeRTOS comparison build and Linux RT permissions when available'], 'A timely result requires a task model, resource bounds and an admission argument. Separate mathematical schedulability, a finite trace and measurements from an actual processor; none alone supplies all three.', [('R-LIU-LAYLAND-1973','3. The Environment',1),('R-LIU-LAYLAND-1973','4. A Fixed Priority Scheduling Algorithm',2),('R-OS3-BUTTAZZO-2011','4.3-4.6 introductory definitions: RM, EDF, DM and processor demand',3),('R-OS3-BUTTAZZO-2011','7.3 Terminology and assumptions; 10.7 System overhead',2),('R-OS3-LINUX-DEADLINE','2.1 Main algorithm; 3.1-3.4 task models and schedulability',2)], 'execution'),
53: ('Symmetric multiprocessing and parallel execution on x86-64', 'kernel', ['T48','T50','T51'], ['C','F'], ['QEMU PC with 1, 2 and 4 virtual CPUs','Original x86-64 image and AP trampoline','Host multicore machine for comparative timings'], 'Extend the one-core machine deliberately: secondary CPUs need explicit boot, private execution state, shared-memory synchronization and remote coordination. Parallel work must actually execute on multiple CPUs; a scheduler model is only one supporting oracle.', [('R-OS3-OSPP-II','5.7.1 and 5.7.3 introductory paragraphs: uniprocessor and multiprocessor exclusion',2),('R-OS3-OSPP-II','7.2 introductory paragraphs: multiprocessor scheduling',2),('R-OS3-MCS-1991','Pseudocode: test-and-set, ticket and MCS locks',2)], 'kernel_smp'),
54: ('Dual-core microcontroller AMP and shared-memory ownership', 'bare-metal', ['T50','T51','T52'], ['C','E'], ['NUCLEO-H745ZI-Q dual-core board with exact board and silicon revisions recorded','Two ELF images, debugger and logic analyzer','Selected board reference manual and cache application notes'], 'Build separate M7 and M4 images with static peripheral ownership. This is asymmetric multiprocessing, not a coherent SMP machine. Shared SRAM, cache state, DMA and notifications each require an explicit contract.', [('R-OS3-ST-AN5617','2.1-2.2 Use cases, shared memory and notification',2),('R-OS3-ST-AN5617','3.3.1-3.3.3 HSEM, EXTI/SEV and DMA',2),('R-OS3-ST-AN4839','3.1-3.2 Default settings and cache maintenance/data coherency example',2)], 'execution'),
55: ('Lock-free algorithms, reclamation and progress arguments', 'cross-track', ['T26','T51','T53'], ['C','F'], ['x86-64 lock-free atomic widths audited in disassembly','History/interleaving checker','Multicore execution; bounded node pools'], 'Separate atomicity, linearizability, memory reclamation and progress. A successful stress run does not prove any of them. The required CAS queue runs on x86-64; the H745 route uses ownership and SPSC without assuming cross-core exclusive-monitor behavior.', [('R-SYS-MICHAEL-SCOTT-1996','§3.1 Safety',2),('R-OS3-MS-CORRECTED','Memory-management assumptions and corrected non-blocking pseudocode',2),('R-OS3-HAZARD-2004','Printed p.492: hazard ownership and retire/scan methodology',1),('R-OS3-HERLIHY-WING-1990','Printed pp.465-467: FIFO histories and Section2.1 histories',2)], 'execution'),
56: ('Concurrent OS services and integrated systems', 'kernel', ['T46','T47','T48','T53','T54','T55'], ['C','F'], ['Original SMP kernel and isolated user processes','Disposable virtio disk image','H745 physical endpoint for final transfer'], 'Turn mechanisms into services with explicit process, thread, memory and I/O ownership. Preserve the VM and persistent store implementations; give the store one owner and communicate through bounded requests rather than sharing its internals.', [('R-OS3-OSPP-II','4.4.1 and 4.6.3 introductory paragraphs: TCB and context switching',2),('R-KER-OSTEP-PAGING','18.1 A Simple Example And Overview',2),('R-KER-VIRTIO-1-2','2.7 Split Virtqueues',2),('R-OS3-ST-AN5617','2.1-2.2 Use cases, shared memory and notification',2)], 'kernel_smp'),
}
ENTRY = {
49:'Reuse the process launcher, descriptor ledger and thread join exercises. Before the observatory, distinguish a return value from errno and explain which process owns a descriptor. Assembly context switching is taught later and is not an entry requirement here.',
50:'Reuse the reset/exception frame decoder, bounded ISR and ABI experiments. Draw the basic Cortex-M exception frame, identify PSP/MSP and show an ELF/map with accounted RAM before writing a scheduler.',
51:'Bring a working task lifecycle, a bounded wait queue and the C publication exercise. Explain why check-then-sleep can lose a wake before adding any blocking object.',
52:'Bring fixed-priority response-analysis, timer rollover and priority-inheritance traces. Distinguish release, start, completion, execution cost and deadline before calculating utilization.',
53:'Bring the T48 boot/trap/frame allocator and C atomic publication artifacts. Early AP startup does not require finishing the VM/store capstone. Add task and IPC prerequisites only at the milestones that use them.',
54:'Bring original MCU startup, the memory-master placement ledger and the single-core runtime. Explain which SRAM each core and DMA can access before declaring any shared buffer.',
55:'Bring an acquire/release publication argument, a bounded node lifetime ledger and actual multicore execution. Revisit the ABA counterexample before adding reclamation.',
56:'Bring accepted process isolation and SMP mechanisms. VM, store and physical AMP artifacts enter only when their service milestones require them; early process/memory work does not require the final application.'}
FIRST = {49:['PR-T23-01','PR-T25-01'],50:['PR-T29-01','PR-T37-07'],51:['PR-T50-04','PR-T25-03'],52:['PR-T30-04'],53:['PR-T48-03','PR-T25-03'],54:['PR-T50-01','PR-T37-06'],55:['PR-T25-03','PR-T53-05'],56:['PR-T48-08','PR-T53-06']}
EXTRA = {'PR-T50-06':['PR-T29-02'],'PR-T50-12':['PR-T50-11'],'PR-T51-04':['PR-T50-06'],'PR-T51-13':['PR-T50-05'],'PR-T52-03':['PR-T51-04'],'PR-T52-04':['PR-T51-03'],'PR-T52-11':['PR-T50-12','PR-T51-06'],'PR-T52-12':['PR-T49-09'],'PR-T53-06':['PR-T50-04','PR-T51-03'],'PR-T53-07':['PR-T51-01'],'PR-T53-09':['PR-T48-04'],'PR-T53-11':['PR-T51-06'],'PR-T54-03':['PR-T25-03'],'PR-T54-06':['PR-T29-04'],'PR-T54-08':['PR-T51-13'],'PR-T54-09':['PR-T50-12'],'PR-T55-06':['PR-T51-06'],'PR-T56-03':['PR-T48-04'],'PR-T56-04':['PR-T53-09'],'PR-T56-05':['PR-T51-06'],'PR-T56-07':['PR-T48-10'],'PR-T56-08':['PR-T48-07'],'PR-T56-09':['PR-T48-12'],'PR-T56-10':['PR-T48-13'],'PR-T56-12':['PR-T54-10','PR-T52-11','PR-T52-12']}

EXTRA['PR-T53-08']=['PR-T56-01']
ENTRY[53]+=' Complete PR-T56-01 after the early SMP queue milestones, then return to PR-T53-08 for the actual user-thread affinity and migration gate.'
ENTRY[56]+=' Begin PR-T56-01 before PR-T53-08, then revisit the SMP/TLB milestones before PR-T56-04; follow explicit project dependencies across module numbers.'

def dump(path, obj):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def main():
    resource_map={r['id']:r for r in json.loads((ROOT/'references/resources.json').read_text(encoding='utf8'))['resources']}
    resource_map.update({r['id']:r for r in json.loads((HERE/'resources.json').read_text(encoding='utf8'))['resources']})
    domains=[]; total=0
    for n,rows in DATA.items():
        title,track,prereqs,profiles,equipment,purpose,readings,family=META[n]
        readings=list(readings)
        if n in (49,52):readings.append(('R-OS3-LINUX-EEVDF','Lag, eligibility and virtual-deadline selection',1))
        if n in (51,52):readings.append(('R-OS3-SHA-PI-1990','Printed p.1176: inversion example and task/resource assumptions',1))
        if n==52:readings.append(('R-OS3-FREERTOS-POSIX-1120','port.c scheduler startup/shutdown and POSIX dependencies; portmacro.h types/stack macros',1))
        domain=f'T{n}'; project_ids=[]; topics=[]; sections=[]
        for i,row in enumerate(rows,1):
            name,contract,steps,failures,evidence,transfer = row
            tid=f'{domain}.{i:02d}'; pid=f'PR-{domain}-{i:02d}'; project_ids.append(pid)
            hours=8 if i%3 else 10
            objectives=[{'id':tid+'.a','text':'Derive and defend the state, ownership and ordering contract for '+name.lower()+'.'}, {'id':tid+'.b','text':steps}, {'id':tid+'.c','text':evidence}]
            topics.append({'id':tid,'title':name,'elective':False,'objectives':objectives})
            fixture=f'fixtures/os_v3/{pid}.json'
            oracle,inputs,expected=SPECS[pid]
            case={'id':pid,'description':name+' finite independent example','oracle':oracle,'input':inputs,'expected':expected,'verification_method':'research/v3/verify_fixtures.py independently recomputes this declared finite example; implementation and hardware evidence remain separate','mechanism_contract':contract,'required_observation':evidence}
            model_scope={
                'PR-T50-02':'The capacity8 arithmetic fixture is a smaller independent pool example; it does not replace the sixteen-slot runtime contract.',
                'PR-T52-06':'This fixture models fixed-window periodic throttling, not the executable soft-CBS anchor.'
            }.get(pid,'')
            if model_scope: case['model_scope']=model_scope
            dump(ROOT/fixture,case)
            meta={'id':pid,'title':name,'module':domain,'topics':[tid],'status':'core','hours':hours,'profile':profiles[-1],'fixtures':[fixture],'edition':3,'evidence_family':family,'prelude_objectives':[]}
            meta['requires_projects']=list(dict.fromkeys(([f'PR-{domain}-{i-1:02d}'] if i>1 else FIRST[n])+EXTRA.get(pid,[])))
            body=f'''# {pid}: {name}

## Contract

{contract}

This {hours}-hour milestone includes implementation and verification. Reading, assessment and delayed retrieval are budgeted separately. Reuse accepted earlier components and record which source files changed. Caller storage, queue limits and task identities must be explicit; failure must preserve the last valid state or enter a documented failed state.

## Inputs and expected results

The supplied `{fixture}` contains a finite `{oracle}` example with hand-declared expected results. From the curriculum source checkout, run `python research/v3/verify_fixtures.py` to recompute its schedule, arithmetic or state transitions independently. This oracle does not execute your implementation. The implementation lives in the `{family}` learner workspace. Consult that family's manifest for the exact available project-selected tests. A missing mapping, a model-only pass or an unavailable physical target never closes the machine-execution gate.

{model_scope}

## Steps

{steps}

Before running, draw the transition or ownership diagram and predict the first observable divergence of one faulty variant. Keep the good and faulty traces under identical inputs. Record compiler, options, source/image hashes, queue/task limits and the clock used. Start with one boundary, then combine already accepted mechanisms.

## Failure cases

{failures}.

Exercise each named case deliberately. A timeout in the harness reports a failure to establish progress, not a successful timeout implementation. Avoid logging inside the measured critical path unless its cost is included.

## Acceptance and evidence

{evidence}

For each objective, retain the concrete source location, raw trace, independent expectation and explanation. A reviewer should be able to reproduce the fault and observe its repair without trusting a summary marker. Disassembly and linker evidence accompany freestanding claims; actual board measurements remain separate from host and emulator observations.

## Hints and recovery

First identify the invariant violated, then reduce the schedule to the shortest counterexample. Inspect the family's staged hints after recording your attempt. Freeze previously accepted boundaries while repairing this mechanism. Assessor implementations are separate artifacts and cannot be relabeled as learner work.

## Transfer challenge

{transfer} Attempt the variant after at least seven days. Predict the changed result before recompiling and explain whether the original argument still applies.
'''
            (ROOT/'curriculum/projects'/f'{pid}.md').write_text('```json\n'+json.dumps(meta,ensure_ascii=False,indent=2)+'\n```\n\n'+body,encoding='utf-8')
            sections.append(f'### {tid} — {name}\n\n{contract}\n\n{steps}\n\nA discriminating experiment exposes: {failures}. Required evidence: {evidence}\n\nDevelop this mechanism in {{ref}}`{pid}`.\n')
        project_hours=sum(8 if i%3 else 10 for i in range(1,len(rows)+1)); verify=len(rows)*3
        reading_hours=max(12,sum(x[2] for x in readings)+4)
        workload={'reading':reading_hours,'implementation':project_hours-verify,'verification':verify,'assessment':8,'consolidation':8}
        meta={'id':domain,'title':title,'track':track,'status':'core','prerequisites':prereqs,'prelude_objectives':[],'workload':workload,'readings':[{'resource_id':r,'locator':l,'purpose':resource_map[r]['reason'],'hours':h} for r,l,h in readings],'projects':project_ids,'profile':profiles,'equipment':equipment,'optional_project_hours':0,'edition':3}
        prose=f'''# {domain}: {title}

## Purpose and outcomes

{purpose}

The {len(rows)} projects contribute {project_hours} implementation/verification hours. Together with reading ({reading_hours}), assessment (8) and consolidation (8), the provisional module budget is {sum(workload.values())} hours. These are planning estimates, not measured learner calibration.

## Entry check and remediation

{ENTRY[n]} Where a prerequisite is missing, repair its smallest failing experiment first. Use prior projects by ID rather than duplicating their implementation hours. Explicit project dependencies govern the implementation route; module prerequisites describe the complete study background.

## Study sequence and teaching

'''+ '\n'.join(sections)+f'''
## Reading guidance

Read only the assigned locator before the related experiment, then return to it after the counterexample. State the source's machine and task-model assumptions next to your own. Historical pseudocode does not authorize undefined C races; Linux, ISO C and a microcontroller memory system are separate contracts. The source/search/reflection ledger is in `research/v3/`; filenames and graph records alone do not qualify as inspected readings.

## Implementation projects

Complete {', '.join('{ref}`'+p+'`' for p in project_ids)} in order. Project manifests distinguish model, host, emulator, cross-build and physical evidence. Later cumulative tests may require later mechanisms; an early milestone is assessed on its own contract and cannot borrow a later reference implementation.

## Mastery gate and feedback

Demonstrate one correct execution and one deliberately wrong variant per mechanism. Explain the first invalid transition, fix its cause, and repeat with an independently chosen input. Review safety, progress, resource lifetime and timing as distinct claims. A stress test supplies observations; bounded exhaustive schedules supply only the stated finite coverage. A deadlocked or unavailable run is not a pass.

## Consolidation and transfer

After seven days rebuild one mechanism from its public contract, change the contention or arrival pattern, and compare the new prediction with the result. After a month integrate two previously separate boundaries. Record surprises, remediation hours and the source passage that changed your reasoning.

## Environment and limits

{' '.join(equipment)}. Custom target images provide their own startup and required runtime helpers. Compiler-provided type headers and reviewed intrinsics are allowed; linked libc, libgcc, compiler-rt, libatomic, vendor HAL, OpenAMP and pre-existing kernels are excluded. FreeRTOS/Linux comparison programs are explicitly separate builds. Never infer physical WCET, electrical behavior or dual-core cache correctness from QEMU or a hosted model. The final learner record preserves unresolved or unavailable gates.
'''
        (ROOT/'curriculum/modules'/f'{domain}.md').write_text('```json\n'+json.dumps(meta,ensure_ascii=False,indent=2)+'\n```\n\n'+prose,encoding='utf-8')
        domains.append({'id':domain,'title':title,'depth':'implementation and experimental mastery','subtopics':topics});total+=len(rows)
    dump(ROOT/'curriculum/v3_taxonomy.json',{'edition':3,'domains':domains})
    dump(HERE/'authoring_manifest.json',{'edition':3,'domains':len(domains),'topics':total,'projects':total,'objectives':3*total,'ownership':['curriculum/modules/T49-T56.md','curriculum/projects/PR-T49..56*','curriculum/v3_taxonomy.json','fixtures/os_v3','research/v3/author*','research/v3/topics*'],'estimates_uncalibrated':True})
    print(f'Authored {len(domains)} domains, {total} projects, {3*total} objectives')

if __name__=='__main__': main()
