"""Reconcile the dependency map and effort illustrations from canonical budgets."""
import math
from collections import defaultdict
from common import ROOT, load, read_json, dump, write, total_hours, domain_dependencies

def main():
    taxonomy,modules,projects,resources=load()
    if set(modules)!={d['id'] for d in taxonomy}: raise ValueError('Finish all module contracts before route accounting')
    preferred=['T01','T02','T03','T04','T05','T06','T07','T08','T09','T12','T18','T21','T32','T14','T15','T16','T19','T20','T22','T13','T11','T10','T17','T23','T24','T25','T26','T27','T28','T29','T30','T31','T33','T34','T35']
    preferred += sorted(set(modules)-set(preferred))
    order=[]; pending=set(modules)
    dependencies=domain_dependencies(modules,projects)
    while pending:
        available=[k for k in preferred if k in pending and dependencies[k]<=set(order)]
        if not available: raise ValueError('Prerequisite cycle or missing domain')
        key=available[0]; order.append(key); pending.remove(key)
    families=read_json(ROOT/'curriculum/integration_families.json')['families']
    communication_families=[]
    for key,title,reuse in [('T40','EDU7-v1: all seven OSI responsibilities',['T15','T18','T28','T38']),
                            ('T41','Ethernet layer 1 through HTTP',['T18','T27','T28','T29','T39','T42']),
                            ('T42','HTTP server from scratch over OS sockets',['T15','T18','T23','T24','T32'])]:
        if key in modules:
            milestones=modules[key]['meta']['projects']
            communication_families.append({'id':key,'title':title,'milestones':milestones,
                'owning_module_project_hours':sum(projects[p]['meta']['hours'] for p in milestones),
                'reused_modules':reuse,'additional_family_hours':0,
                'accounting':'A view of owning-module work; prerequisites and reused artifacts retain their original budgets.'})
    performance_tracks=[]
    for key in ['T43','T44','T45']:
        if key in modules:
            milestones=modules[key]['meta']['projects']
            performance_tracks.append({'id':key,'title':modules[key]['meta']['title'],'milestones':milestones,
                'owning_module_project_hours':sum(projects[p]['meta']['hours'] for p in milestones),
                'additional_directory_hours':0,
                'accounting':'Directory of owning-module projects, with reused earlier artifacts charged only in their original modules.'})
    integr=sum(projects[p]['meta']['hours'] for f in families for p in f['milestones'])
    overhead=sum(sum(f['additional_workload'].values()) for f in families)
    catalog=sum(total_hours(m['meta']) for m in modules.values())+integr+overhead
    elective=sum(p['meta']['hours'] for p in projects.values() if p['meta']['status']=='elective')
    # A conservative mixed-domain core retains its common reading/review budget.
    core=catalog-elective
    diagnostic=total_hours(modules['T01']['meta'])
    entry_check=modules['T01']['meta']['workload']['assessment']
    passed_diagnostic_core=core-(diagnostic-entry_check)
    architecture_hours=sum(total_hours(m['meta']) for k,m in modules.items() if 36<=int(k[1:])<=42)
    performance_hours=sum(total_hours(m['meta']) for k,m in modules.items() if 43<=int(k[1:])<=45)
    new_family_hours=sum(total_hours(m['meta']) for k,m in modules.items() if 46<=int(k[1:])<=56)
    os_v3_hours=sum(total_hours(m['meta']) for k,m in modules.items() if 49<=int(k[1:])<=56)
    fifth_edition_hours=sum(total_hours(m['meta']) for k,m in modules.items() if int(k[1:])>=57)
    enhancement_hours=architecture_hours+performance_hours
    summary={'catalog_hours':catalog,'enhancement_module_hours':enhancement_hours,
        'architecture_networking_module_hours':architecture_hours,'performance_debugging_module_hours':performance_hours,
        'new_required_family_hours':new_family_hours,'os_concurrency_v3_module_hours':os_v3_hours,
        'current_through_T42_catalog_hours':catalog-performance_hours-new_family_hours-fifth_edition_hours,
        'current_35_domain_catalog_hours':catalog-enhancement_hours-new_family_hours-fifth_edition_hours,'core_with_full_diagnostic_hours':core,'diagnostic_full_module_hours':diagnostic,
        'elective_implementation_verification_hours':elective,'integration_implementation_verification_hours':integr,
        'entry_check_hours':entry_check,'core_after_passed_diagnostic_hours':passed_diagnostic_core,
        'integration_additional_reading_assessment_consolidation_hours':overhead,
        'core_weeks_at_10h':math.ceil(core/10),'core_weeks_at_20h':math.ceil(core/20),
        'accounting':'Module projects are already in module implementation+verification. Integration adds only new boundary work. Mixed-module reading/review overhead retained conservatively in core. Diagnostic remediation conditional; no calendar promise.'}
    if fifth_edition_hours:
        summary['fifth_edition_module_hours']=fifth_edition_hours
    dump(ROOT/'curriculum/program.json',{'order':order,'summary':summary,'integration_families':families,'communication_families':communication_families,'performance_debugging_tracks':performance_tracks})
    dump(ROOT/'reviews/workload_reconciliation.json',summary)
    family_guide=ROOT/'curriculum/guides/communication-stacks.md'
    if communication_families and family_guide.exists():
        marker='<!-- generated milestone accounting -->'
        guide=family_guide.read_text(encoding='utf-8').split(marker)[0].rstrip()+'\n\n'+marker+'\n\n## Milestone directory and accounting\n\n'
        guide+='These are module milestones, not an additional capstone assignment. Their implementation and verification hours are already included in the module workload.\n\n'
        for family in communication_families:
            guide+='### '+family['title']+'\n\n| Milestone | Work | Hours |\n|---|---|---|\n'
            for pid in family['milestones']:
                pm=projects[pid]['meta']
                guide+='| '+f'{{ref}}`{pid}`'+' | '+pm['title']+' | '+str(pm['hours'])+' |\n'
            guide+='\nOwning-module project work: **'+str(family['owning_module_project_hours'])+' hours**. Added family budget: **0 hours**.\n\n'
        write(family_guide,guide)
    measurement_guide=ROOT/'curriculum/guides/performance-debugging.md'
    if performance_tracks and measurement_guide.exists():
        marker='<!-- generated drill directory -->'
        guide=measurement_guide.read_text(encoding='utf-8').split(marker)[0].rstrip()+'\n\n'+marker+'\n\n## Project and drill directory\n\n'
        guide+='These milestones are already charged in their owning modules. The directory adds **0 hours**. Reuse the named prior implementations; each new brief budgets its additional experiment, diagnosis or repair.\n\n'
        for track in performance_tracks:
            guide+='### '+track['title']+'\n\n| Project | Experiment or drill | Hours |\n|---|---|---|\n'
            for pid in track['milestones']:
                pm=projects[pid]['meta']
                guide+='| '+f'{{ref}}`{pid}`'+' | '+pm['title']+' | '+str(pm['hours'])+' |\n'
            guide+='\nOwning-module project work: **'+str(track['owning_module_project_hours'])+' hours**.\n\n'
        write(measurement_guide,guide)
    rows=[]
    for k in order:
        meta=modules[k]['meta']; w=meta['workload']; eh=sum(projects[p]['meta']['hours'] for p in meta['projects'] if projects[p]['meta']['status']=='elective')
        rows.append('| '+f'{{ref}}`{k}`'+' | '+meta['track']+' | '+', '.join(meta['prerequisites'])+' | '+' / '.join(str(w[v]) for v in ['reading','implementation','verification','assessment','consolidation'])+' | '+str(total_hours(meta))+' | '+str(eh)+' |')
    content='''# Workload and illustrative schedules

Effort is estimated for deliberate independent study. A failed gate adds remediation; a passed diagnostic can remove remedial work. These estimates do not prescribe a completion deadline.

## Reconciled module budgets

The five values are reading / implementation / verification / assessment / consolidation hours. Project hours are already contained in implementation plus verification. Do not add the project directory totals a second time. The elective column is implementation/verification work inside a mixed module; common reading and review remain conservatively in the core estimate.

| Module | Track | Domain foundations | Five budgets (h) | Total (h) | Elective project slice (h) |
|---|---|---|---|---|---|
'''+ '\n'.join(rows)+'\n\n'
    content+='## Integration additions\n\n| Family | New implementation + verification | Reading | Assessment | Consolidation | Total |\n|---|---|---|---|---|---|\n'
    for f in families:
        ph=sum(projects[p]['meta']['hours'] for p in f['milestones']); w=f['additional_workload']
        content+='| '+f['id']+' | '+str(ph)+' | '+' | '.join(str(w[x]) for x in ['reading','assessment','consolidation'])+' | '+str(ph+sum(w.values()))+' |\n'
    content+=f'''\n## Planning totals

The T49–T56 operating-system, concurrency and multicore route contributes **{os_v3_hours:,} hours**, already included in the totals below. The remaining runtime/store/kernel allocation is **{new_family_hours-os_v3_hours:,} hours**. These are provisional project estimates, not measured learner completion times.

The architecture and networking domains T36–T42 contribute **{architecture_hours:,} hours**. The performance and debugging domains T43–T45 contribute **{performance_hours:,} hours**. The runtime, storage, kernel and OS/concurrency families contribute **{new_family_hours:,} hours**. All are already included below. The current allocation to the original 35-domain catalog is **{catalog-enhancement_hours-new_family_hours-fifth_edition_hours:,} hours**; the current allocation through T42 is **{catalog-performance_hours-new_family_hours-fifth_edition_hours:,} hours**. The archived first edition remains the source of historical estimates. Communication and runtime family directories add no second budget. Reuse previous implementations and charge only newly assigned work.
'''+(f'''
The fifth-edition domains T57 onward contribute **{fifth_edition_hours:,} hours**, already included in the totals below.
''' if fifth_edition_hours else '')+f'''
The full catalog estimate is **{catalog:,} hours**. The conservative core estimate including the complete T01 diagnostic/remediation allocation is **{core:,} hours**. Elective implementation/verification slices total **{elective:,} hours**. T01's full allocation is **{diagnostic} hours**; after passing the initial diagnostic, retain its evidence and remove only its unneeded remedial activities from your personal plan.

With the {entry_check}-hour initial entry check passed and no T01 remediation required, the core estimate becomes **{passed_diagnostic_core:,} hours**, or {math.ceil(passed_diagnostic_core/10)} / {math.ceil(passed_diagnostic_core/20)} weeks at 10 / 20 hours per week. Other targeted mathematics or semantic remediation is additional when needed.

New integration implementation/verification adds {integr} hours, plus {overhead} hours of reading, assessment and delayed transfer. This {integr+overhead}-hour integration slice is already included in both the full-catalog and core totals above. The shared protocol, containers and their prior tests are charged in their original modules, not again in both capstones. Each capstone budget pays for its platform-specific assembly, campaigns and defense.

At **10 hours/week**, the conservative core arithmetic is {core}/10, rounded up to **{math.ceil(core/10)} weeks**. At **20 hours/week**, it is {core}/20, rounded up to **{math.ceil(core/20)} weeks**. Breaks, equipment access, prior knowledge and retests change elapsed time. These are workload illustrations, not promises of mastery.

## Weekly rhythm

For a ten-hour week, reserve about two hours for reading/prediction, five for a narrow implementation milestone, two for verification/diagnosis and one for assessment or delayed retrieval. Use the actual module budget when a phase needs more verification. A twelve-hour project may span weeks.

For a twenty-hour week, complete one or two small milestones with their checks, preserve a reading/review block and revisit an older component. Keep retrieval separated in time rather than using all available hours to start new code. Carry failed gates into the next week deliberately.

## Personal plan

Track planned and actual hours in the five categories, with reused evidence identified by its original project ID. Keep core, elective and conditional remediation columns. When one milestone expands, split an independently reviewable boundary rather than hiding a large project behind its original estimate. Hardware waiting time is a scheduling constraint, not implementation effort; use the portable/Linux route while waiting.
'''
    write(ROOT/'curriculum/guides/workload.md',content)
    route='''# Roadmap and prerequisite map

Domain numbering preserves coverage, not chronology. Begin with the diagnostic, build a shared portable core, then branch into Linux and firmware. Contracts, error handling and tests recur from the first project; their deeper modules consolidate practice.

For the recommended working sequence use the [project route](project-route.md). It brings introductory diagnosis, bounds experiments and measurement forward through selected prerequisites. The complete-domain order below is a reference traversal, not a demand to delay those projects.

## Phases and handoffs

| Phase | Main work | Handoff and gate |
|---|---|---|
| Entry | T01 plus targeted mathematical remediation | Predict, repair and explain a small multi-file program |
| Precise C | T02–T06, selected T12, early T18/T21/T32 habits | Object/representation/contract evidence on small components |
| Tools and reusable mechanisms | T07–T09, T12–T16, T18–T22 | Audited portable container/parser/allocator library; P1/P3 |
| Runtime boundary | T10–T11/T13, selected Linux interface foundations | Explain actual entry, linking and helper closure; P2 |
| Linux branch | T23–T25 plus testing/security refinements | Bounded telemetry, ownership/backpressure/shutdown; P5 |
| Firmware branch | T27–T31 plus selected T25 leaves | Reset, drivers, contexts, DMA and timing; P4 |
| Deeper reasoning | T17, T26 awareness, T32–T35 | Numerical, formal, performance and resilience evidence; P6 |
| Algorithms and architecture | T36–T37 after their named C/tool/ownership foundations | Handwritten assembly, invariant-backed algorithms, machine and MCU evidence |
| Communication foundations | T38–T39 after drivers and relevant architecture preludes | Measured serial/electrical channels and audited Ethernet frame ownership |
| Complete stack families | T40–T42 after their distinct prerequisites | Seven-layer telemetry, Ethernet-to-HTTP and socket-based HTTP server defenses |
| Measurement and diagnosis | T43–T45 after their named foundations; use introductory habits from the first component | Multi-axis cost evidence, causal diagnosis, repaired fault experiments and delayed transfer |
| Capstones | Shared core plus completed relevant branches | Independently defended Linux and firmware applications |
| Runtime, storage and kernel | T46–T48 after their explicit prerequisites | Portable VM, recoverable store, isolated kernel processes and an integrated persistent guest |
| OS, real-time and multicore | T49–T56 through stage-specific prerequisites | Original task runtime, synchronization, x86 SMP, physical MCU AMP and concurrent services |

T18 ownership and API foundations precede T14 container design. T29 requires selected race/atomic/ordering leaves from T25, rather than finishing all POSIX threading projects. Module front cards enumerate the precise preludes. Complete an explicitly named prelude before its dependent project even if the full domain is scheduled later. T26 preserves its awareness/elective route; T55 adds required advanced concurrency implementation.

## A valid full-domain order

The following order is computed from full prerequisites and the owning domains of selected module and project objective preludes. It supports full-domain completion. Early projects may begin after their own named entry/prelude gates; this serial traversal does not require postponing them until all later milestone foundations are complete:

'''+ ' → '.join(f'{{ref}}`{k}`' for k in order)+'\n\n'
    route+='## Domain dependencies and selected preludes\n\n| Domain | Full domain foundations | Selected objective preludes |\n|---|---|---|\n'
    for k in order:
        meta=modules[k]['meta']
        preludes=list(dict.fromkeys(meta['prelude_objectives']+[p for pid in meta['projects'] for p in projects[pid]['meta'].get('prelude_objectives',[])]))
        route+='| '+f'{{ref}}`{k}`'+' | '+(', '.join(f'{{ref}}`{p}`' for p in meta['prerequisites']) or 'Entry')+' | '+(', '.join(f'{{ref}}`{p}`' for p in preludes) or 'None additional')+' |\n'
    route+='''
## Parallel progress

After the portable foundations, a Linux learner can advance processes/events while a firmware learner completes reset/driver work. Both retain a common protocol and data-structure core. To complete the full core curriculum, return to the other branch and defend both capstones. A branch-only portfolio is useful but is not full-core completion.

Source reading and proof/measurement techniques recur at progressively deeper gates. Where a selected prelude introduces a mechanism early, mark those exact objectives complete with evidence and reuse them later; do not double-count the work. Use the complete objective map to identify unassessed leaves before final review.
'''
    write(ROOT/'curriculum/guides/roadmap.md',route)
    print(summary)

if __name__=='__main__': main()
