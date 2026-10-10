"""Merge inspected additions and rebuild the complete third-edition route."""
import json
from common import ROOT, dump, read_json, write, parse
from integrate_v2 import main as integrate_previous

def main():
    additions=[]
    for addition_path in sorted((ROOT/'research/v3').glob('*resources.json')):
        addition=read_json(addition_path)
        additions+=addition if isinstance(addition,list) else addition['resources']
    path=ROOT/'references/resources.json'; registry=read_json(path)
    resources={r['id']:r for r in registry['resources']}
    seen=set()
    for item in additions:
        key=item['id']
        if key in seen: raise ValueError('Repeated v3 resource identity: '+key)
        seen.add(key)
        if item.get('reuse'):
            if key not in resources: raise ValueError('Unknown reused resource: '+key)
        else:
            # Existing normalized identity fields are generated, not inspection.
            prior={k:v for k,v in resources.get(key,{}).items() if k not in {'work_id','edition_id','inspection_ids'}}
            incoming={k:v for k,v in item.items() if k not in {'work_id','edition_id','inspection_ids'}}
            if prior and prior!=incoming: raise ValueError('Conflicting v3 resource identity: '+key)
            resources[key]=item
    registry['resources']=list(resources.values()); dump(path,registry)
    reading_path=ROOT/'research/v3/target_readings.json'
    if reading_path.exists():
        for module_id,readings in read_json(reading_path).items():
            module=ROOT/'curriculum/modules'/(module_id+'.md')
            meta,body=parse(module)
            for reading in readings:
                existing=next((r for r in meta['readings'] if (r['resource_id'],r['locator'])==
                              (reading['resource_id'],reading['locator'])),None)
                if existing is None: meta['readings'].append(reading)
                elif existing!=reading: raise ValueError('Conflicting target reading: '+module_id)
            if sum(r['hours'] for r in meta['readings'])>meta['workload']['reading']:
                raise ValueError('Target readings exceed module reading budget: '+module_id)
            write(module,'```json\n'+json.dumps(meta,ensure_ascii=False,indent=2)+'\n```\n\n'+body)
    integrate_previous()
    edition=read_json(ROOT/'curriculum/edition.json')
    edition.update(edition='C Mastery 3',schema_version=3,previous_edition_commit='40bf1b2',
        previous_edition_counts={'domains':48,'projects':337,'objectives':1605},
        advanced_required_domains=[f'T{i}' for i in range(49,57)],
        freestanding_policy='strict: no libc/libgcc/compiler-rt/libatomic/HAL/OpenAMP; original helpers and startup',
        physical_parallel_target='NUCLEO-H745ZI-Q (Cortex-M7/M4 AMP)',
        software_parallel_target='x86-64 QEMU SMP: 1, 2, 4 CPUs')
    edition['targets']=list(dict.fromkeys(edition['targets']+['x86-64 QEMU SMP','STM32H745 Cortex-M7/M4 AMP']))
    dump(ROOT/'curriculum/edition.json',edition)
    handoffs={
        'T23':'T49 observes these interfaces; T56 implements bounded spawn/exit/wait, demand-zero/COW, mappings and file-handle mechanisms in the educational kernel.',
        'T24':'T51 implements blocking wait/wake, channels, events and counted notifications. T56 crosses process boundaries with explicit capabilities and shared-memory offsets.',
        'T25':'T50 and T51 implement the task and synchronization machinery under the hosted interfaces. T53 and T54 test the changed coherent-SMP and physical-AMP assumptions.',
        'T26':'The original awareness/elective identities remain intact. T55 is the required advanced implementation route for progress, lock-free queues, hazard pointers, epoch reclamation and concurrent histories.',
        'T29':'T50 builds original exception/context-switch machinery. T54 revisits interrupt handoff with two MCU cores, explicit shared SRAM, caches and DMA ownership.',
        'T30':'T52 extends analysis to blocking/jitter/overheads, ceilings, CBS, overload and partitioning. The original FreeRTOS integration remains a separate comparison; T50 implements the original runtime.',
        'T43':'T49–T56 reuse this measurement ledger for dispatch work, context switches, wakeup latency, interference, contention, parallel speedup and I/O. Count reused work once.',
        'T44':'T50–T56 extend controlled diagnosis to context corruption, missed wakeups, priority inversion, remote scheduling, cache visibility and late completion. A longer stress run is not a causal explanation.',
        'T45':'T51/T55/T56 extend lifetime drills to wait nodes, task stacks, stale handles, ABA, reclamation, COW frames and DMA buffers. Repair ownership before increasing retry counts.',
        'T48':'Preserve this bounded single-core route and ABI regression. T53 adds a separate SMP profile; T56 introduces thread/process separation, blocking IPC, memory faults and concurrent services.'}
    for key,text in handoffs.items():
        module=ROOT/'curriculum/modules'/f'{key}.md'
        marker='<!-- third-edition handoff -->'
        body=module.read_text(encoding='utf-8').split(marker)[0].rstrip()
        if key=='T30':
            body=body.replace('Writing a new scheduler kernel is a separate elective.',
                'The original scheduler implementation is assessed separately in T50; it is not required to pass this integration milestone.')
        write(module,body+'\n\n'+marker+'\n\n## Third-edition implementation handoff\n\n'+text+'\n')

if __name__=='__main__': main()
