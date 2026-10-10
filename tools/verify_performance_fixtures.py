"""Independent finite T43 certificates. No learner C or private author spec is imported.

Synthetic operation/event ledgers check arithmetic and scoped semantics; they do not
certify native performance, PMU hardware, cgroup isolation, persistence or WCET.
"""
from __future__ import annotations
import argparse
import copy
import json
import math
from pathlib import Path
import re
import statistics
from common import ROOT

FIXTURES = ROOT / 'fixtures/performance'
checks = 0
KINDS = ['cost','amortized','mix_cfg','storage','blocks','events','transfers','samples','pmu','cgroup','profiling','dwt']
TOPICS = [[1,2],[3],[4,14],[5],[6],[7],[8],[9],[10],[11],[12],[13]]

def require(ok, message):
    global checks
    checks += 1
    if not ok:
        raise ValueError(message)

def integer(value, low=0, high=10**15):
    require(type(value) is int and low <= value <= high, 'integer scope/type')
    return value

def boolean(value):
    require(type(value) is bool, 'boolean scope/type')
    return value

def fields(value, keys):
    require(type(value) is dict and set(value) == set(keys.split()), 'record fields')

def sequence(value, maximum=128):
    require(type(value) is list and len(value) <= maximum, 'bounded sequence')
    return value

def strict_json(text):
    def unique(pairs):
        result={}
        for key,value in pairs:
            require(key not in result,'duplicate JSON key: '+key)
            result[key]=value
        return result
    return json.loads(text,object_pairs_hook=unique,parse_constant=lambda s: (_ for _ in ()).throw(ValueError('nonfinite JSON constant')))

def metadata(path):
    match=re.match(r'\s*```json\s*\n(.*?)\n```',path.read_text(encoding='utf-8'),re.S)
    require(match is not None,'canonical metadata fence')
    value=strict_json(match.group(1)); require(type(value) is dict,'canonical metadata object')
    return value

def equal(actual, expected, path='expected'):
    require(type(actual) is type(expected), path + ': exact type')
    if isinstance(actual, dict):
        require(set(actual) == set(expected), path + ': key set')
        for key in actual: equal(actual[key], expected[key], path + '.' + key)
    elif isinstance(actual, list):
        require(len(actual) == len(expected), path + ': length')
        for k, (a, e) in enumerate(zip(actual, expected)): equal(a, e, path + f'[{k}]')
    elif isinstance(actual, float):
        require(math.isfinite(actual) and math.isfinite(expected) and
                math.isclose(actual, expected, rel_tol=1e-12, abs_tol=1e-12), path + ': finite float')
    else:
        require(actual == expected, path + ': value')

def union_length(intervals):
    if not intervals: return 0
    start, end = sorted(intervals)[0]; total = 0
    for a, b in sorted(intervals)[1:]:
        if a > end: total += end - start; start, end = a, b
        else: end = max(end, b)
    return total + end - start

def derive(kind, i):
    if kind == 'cost':
        fields(i, 'arrays bound_ns finite_ns graph')
        out = []
        for a in sequence(i['arrays'], 8):
            for x in sequence(a, 32): integer(x, -100, 100)
            # Count original inversion pairs and stopping comparisons, not insertion sort.
            shifts = sum(a[j] > a[k] for k in range(len(a)) for j in range(k))
            comparisons = sum(sum(x > a[k] for x in a[:k]) + int(any(x <= a[k] for x in a[:k]))
                              for k in range(1, len(a)))
            places = max(0, len(a)-1)
            out.append(dict(sorted=sorted(a), comparisons=comparisons, shifts=shifts,
                            placements=places, writes=shifts+places))
        ns = sequence(i['bound_ns']); fs = sequence(i['finite_ns'])
        for n in ns + fs: integer(n, 1, 1000)
        costs = [3*n*n+5*n+7 for n in ns]
        fields(i['graph'], 'V E Z')
        v, e, z = (integer(i['graph'][k], 0, 100) for k in ('V','E','Z'))
        return dict(cases=out, bound_costs=costs,
                    finite_bound_holds=all(3*n*n <= c <= 15*n*n for n,c in zip(ns,costs)),
                    observed_piecewise=[n if n<=128 else n*n for n in fs],
                    graph_work=3*v+2*e+z)
    if kind == 'amortized':
        fields(i, 'appends item_bytes charge')
        n=integer(i['appends'], 1, 64); size=integer(i['item_bytes'],1,16); charge=integer(i['charge'],1,8)
        costs=[]; capacities=[]; potentials=[]; amort=[]; credits=[]
        cap=0; total=0; phi=0; transient=0
        for live in range(n):
            copies=live if live==cap else 0
            if live==cap:
                new=max(1,2*cap); transient=max(transient,(cap+new)*size); cap=new
            cost=1+copies; next_phi=2*(live+1)-cap
            costs.append(cost); capacities.append(cap); potentials.append(next_phi)
            amort.append(cost+next_phi-phi); total+=cost; phi=next_phi
            credits.append(charge*(live+1)-total)
        return dict(costs=costs, capacities=capacities, potentials=potentials, amortized=amort,
                    credit=credits, total_actual=total, total_amortized=sum(amort), copies=total-n,
                    final_capacity=cap, live_bytes=n*size, capacity_bytes=cap*size,
                    transient_capacity_peak=transient, prefix_credit_nonnegative=all(x>=0 for x in credits))
    if kind == 'mix_cfg':
        fields(i, 'mix clock_hz cfgs n variants limits')
        clock=integer(i['clock_hz'],1,10**10); n=integer(i['n'],1,100)
        count=cycles=0
        for pair in sequence(i['mix'],8):
            require(type(pair) is list and len(pair)==2,'instruction mix pair')
            a=integer(pair[0],0,10000); c=integer(pair[1],1,100); count+=a; cycles+=a*c
        require(count>0,'nonempty instruction mix'); metrics=[]
        for graph in sequence(i['cfgs'],4):
            fields(graph, 'nodes edges model')
            nodes=integer(graph['nodes'],2,32); edges=sequence(graph['edges'],64); pairs=[]
            for edge in edges:
                require(type(edge) is list and len(edge)==2,'CFG edge pair')
                pairs.append((integer(edge[0],0,nodes-1),integer(edge[1],0,nodes-1)))
            require(len(set(pairs))==len(pairs),'duplicate CFG edge')
            def reach(start, reverse=False):
                found={start}
                for _ in range(nodes):
                    found |= {v for u,v in ((b,a) if reverse else (a,b) for a,b in pairs) if u in found}
                return found
            require(len(reach(0))==nodes and len(reach(nodes-1,True))==nodes,'connected entry/exit CFG')
            require(graph['model'] in ('sequential','nested'),'body-count model')
            metrics.append(dict(cyclomatic=len(pairs)-nodes+2, body_executions=2*n if graph['model']=='sequential' else n*n))
        fields(i['limits'],'code retained'); limits={k:integer(v,0,10000) for k,v in i['limits'].items()}
        accepted=[]
        for v in sequence(i['variants'],4):
            fields(v,'name cycles code retained'); require(type(v['name']) is str,'variant name')
            for key in ('cycles','code','retained'): integer(v[key],0,10000)
            if all(v[key]<=limits[key] for key in limits): accepted.append(v['name'])
        return dict(instructions=count, cycles=cycles, average_cpi=float(cycles/count),
                    time_ns=float(cycles*10**9/clock), cfgs=metrics, acceptable_variants=accepted)
    if kind == 'storage':
        fields(i,'objects sections frames paths interrupt_bytes')
        objects=sequence(i['objects'],16); boundaries={0}; names=set()
        for o in objects:
            fields(o,'name capacity live allocated_from allocated_to live_from live_to')
            require(type(o['name']) is str and o['name'] not in names,'unique ownership object'); names.add(o['name'])
            for key in set(o)-{'name'}: integer(o[key],0,10000)
            require(o['live']<=o['capacity'] and o['allocated_from']<=o['live_from']<=o['live_to']<=o['allocated_to'],'lifetime/capacity')
            boundaries.update(o[k] for k in ('allocated_from','allocated_to','live_from','live_to'))
        ledger=[]
        for t in sorted(boundaries):
            allocated=live=retained=slack=0
            for o in objects:
                if o['allocated_from']<=t<o['allocated_to']:
                    allocated+=o['capacity']
                    if o['live_from']<=t<o['live_to']: live+=o['live']; slack+=o['capacity']-o['live']
                    else: retained+=o['capacity']
            ledger.append(dict(time=t, live=live, allocated=allocated, retained=retained, active_slack=slack))
        fields(i['sections'],'text rodata data bss')
        for v in i['sections'].values(): integer(v,0,100000)
        require(type(i['frames']) is dict,'frame map'); frame=i['frames']
        for f in frame.values():
            fields(f,'bytes qualifier'); integer(f['bytes'],0,10000)
            require(f['qualifier'] in ('static','dynamic,bounded','dynamic'),'stack qualifier')
        bounds=[]
        for path in sequence(i['paths'],8):
            sequence(path,16); require(all(type(x) is str for x in path),'call path names')
            known=len(set(path))==len(path) and all(x in frame and frame[x]['qualifier']!='dynamic' for x in path)
            bounds.append(sum(frame[x]['bytes'] for x in path) if known else None)
        interrupt=integer(i['interrupt_bytes'],0,10000); known=[x for x in bounds if x is not None]
        sections=i['sections']
        return dict(ledger=ledger, peaks={k:max(x[k] for x in ledger) for k in ('live','allocated','retained','active_slack')},
                    flash_bytes=sections['text']+sections['rodata']+sections['data'], ram_sections=sections['data']+sections['bss'],
                    path_bounds=bounds, known_path_peak=max(known) if known else None,
                    conditional_interrupt_bound=max(known)+interrupt if known else None)
    if kind == 'blocks':
        fields(i,'item_bytes block_items memory_items cases')
        item=integer(i['item_bytes'],1,16); block=integer(i['block_items'],1,64); memory=integer(i['memory_items'],2,128)
        require(memory%block==0 and 2*block<=memory,'D=P=1 PDM buffer geometry'); capacity=memory//block; out=[]
        for refs in sequence(i['cases'],8):
            cache=[]; dirty=set(); read=written=0
            for ref in sequence(refs,128):
                fields(ref,'item mode'); index=integer(ref['item'],0,127)
                require(ref['mode'] in ('read','write'),'block reference mode'); b=index//block
                if b in cache: cache.remove(b)
                else:
                    read+=1
                    if len(cache)==capacity:
                        victim=cache.pop(0)
                        if victim in dirty: written+=1; dirty.remove(victim)
                cache.append(b)
                if ref['mode']=='write': dirty.add(b)
            written+=len(dirty); useful=len(refs)*item; rb=read*block*item; wb=written*block*item
            out.append(dict(read_blocks=read, write_blocks=written, read_bytes=rb, write_bytes=wb,
                            useful_bytes=useful, amplification=float((rb+wb)/useful) if useful else None))
        return dict(cases=out)
    if kind == 'events':
        fields(i,'cases'); out=[]
        for c in sequence(i['cases'],8):
            fields(c,'mode submit returned completed observation_end thread_cpu phases')
            require(c['mode'] in ('blocking','async'),'component mode')
            for k in ('submit','returned','completed','observation_end'): integer(c[k],0,10000)
            s,r,d,end=(c[k] for k in ('submit','returned','completed','observation_end'))
            require(s<=min(r,d) and end>=max(r,d) and end>s,'event ordering')
            if c['mode']=='blocking': require(d<=r,'blocking completion boundary')
            cpu=sequence(c['thread_cpu'],8)
            for x in cpu: integer(x,0,end-s)
            intervals=[]
            for pair in sequence(c['phases'],16):
                require(type(pair) is list and len(pair)==2,'phase endpoints')
                a=integer(pair[0],s,end); b=integer(pair[1],a,end); intervals.append((a,b))
            out.append(dict(latency=d-s, call_span=r-s, observation_span=end-s, process_cpu=sum(cpu),
                            wall_minus_cpu=end-s-sum(cpu), inferred_wait=None, phase_sum=sum(b-a for a,b in intervals),
                            phase_union=union_length(intervals), borrow_release=max(r,d), throughput_per_second=float(10**6/(end-s))))
        return dict(cases=out)
    if kind == 'transfers':
        fields(i,'cases'); out=[]
        for c in sequence(i['cases'],8):
            fields(c,'operation requested responses write_end fsync_end device_bytes')
            require(c['operation'] in ('read','write'),'transfer operation'); wanted=integer(c['requested'],1,1024)
            responses=sequence(c['responses'],16); accepted=attempts=0; status='retry_budget_exhausted'
            for response in responses:
                fields(response,'result errno'); result=integer(response['result'],-1,wanted-accepted)
                require(response['errno'] in (None,'EINTR','EAGAIN','EIO'),'transfer errno'); attempts+=1
                require((result==-1)==(response['errno'] is not None),'result/errno contract')
                if result>0:
                    accepted+=result
                    if accepted==wanted: status='complete'; break
                elif result==0:
                    status='eof' if c['operation']=='read' else 'zero_progress'; break
                elif response['errno']=='EIO': status='failed'; break
            write_end=integer(c['write_end'],0,10000); sync=c['fsync_end']; device=integer(c['device_bytes'],0,10**9)
            if sync is not None:
                integer(sync,write_end,10000); require(status=='complete' and c['operation']=='write','sync certificate boundary')
            out.append(dict(accepted_bytes=accepted, attempts=attempts, status=status,
                            requested_sync=sync is not None, sync_span=None if sync is None else sync-write_end,
                            end_to_end=write_end if sync is None else sync,
                            device_amplification=float(device/accepted) if accepted else None))
        return dict(cases=out)
    if kind == 'samples':
        fields(i,'a b overhead quantile_numerator quantile_denominator resolution deadline reference')
        a=sequence(i['a'],64); b=sequence(i['b'],64); overhead=sequence(i['overhead'],64)
        require(len(a)==len(b)>0 and len(overhead)>0,'paired nonempty sample sets')
        for x in a+b+overhead: integer(x,0,10**12)
        den=integer(i['quantile_denominator'],1,1000); num=integer(i['quantile_numerator'],1,den)
        for key in ('resolution','deadline','reference'): integer(i[key],0,10**12)
        rank=(num*len(a)+den-1)//den-1; am=float(statistics.median(a)); bm=float(statistics.median(b)); om=float(statistics.median(overhead))
        differences=[x-y for x,y in zip(a,b)]
        return dict(median_a=am, median_b=bm, paired_differences=differences,
                    median_paired_difference=float(statistics.median(differences)), difference_of_medians=am-bm,
                    quantile_a=sorted(a)[rank], quantile_b=sorted(b)[rank], overhead_median=om,
                    sensitivity_a=am-om if am>om else None,
                    deadline_misses_a=sum(x>i['deadline'] for x in a), deadline_misses_b=sum(x>i['deadline'] for x in b),
                    max_reference_jitter_a=max(abs(x-i['reference']) for x in a), max_reference_jitter_b=max(abs(x-i['reference']) for x in b))
    if kind == 'pmu':
        fields(i,'cases'); out=[]
        for c in sequence(i['cases'],8):
            fields(c,'permitted supported events'); permit=boolean(c['permitted']); support=boolean(c['supported']); scaled=[]; groups=[]
            events=sequence(c['events'],2); require(len(events)==2,'instructions/cycles pair')
            require([e['name'] for e in events]==['instructions','cycles'],'PMU event identity/order')
            for e in events:
                fields(e,'name raw enabled running group'); raw=integer(e['raw'],0,2**64-1); enabled=integer(e['enabled'],0,10**15)
                running=integer(e['running'],0,enabled); groups.append(integer(e['group'],0,100))
                scaled.append(float(raw*enabled/running) if running and permit and support else None)
            valid=all(x is not None for x in scaled) and scaled[0]>0 and groups[0]==groups[1]
            # A same-group certificate also needs identical enabled/running intervals.
            valid=valid and events[0]['enabled']==events[1]['enabled'] and events[0]['running']==events[1]['running']
            status='denied' if not permit else ('unsupported' if not support else ('not_running' if None in scaled else 'observed'))
            out.append(dict(scaled=scaled, cpi=float(scaled[1]/scaled[0]) if valid else None, status=status))
        return dict(cases=out)
    if kind == 'cgroup':
        fields(i,'window_us controllers delegated peak_reset_same_fd writeback_supported quota before after')
        window=integer(i['window_us'],1,10**12); controllers=sequence(i['controllers'],3)
        require(len(set(controllers))==len(controllers) and all(x in ('cpu','memory','io') for x in controllers),'controller list')
        delegated=boolean(i['delegated']); reset=boolean(i['peak_reset_same_fd']); writeback=boolean(i['writeback_supported'])
        fields(i['quota'],'max period'); maximum=i['quota']['max']; period=integer(i['quota']['period'],1,10**12)
        if maximum is not None: integer(maximum,1,10**12)
        for snapshot in (i['before'],i['after']):
            fields(snapshot,'cpu memory io psi')
            fields(snapshot['cpu'],'usage_usec user_usec system_usec nr_periods nr_throttled throttled_usec')
            fields(snapshot['memory'],'current peak')
            fields(snapshot['psi'],'cpu memory io')
            require(type(snapshot['io']) is dict,'keyed device map')
            for device, counts in snapshot['io'].items():
                require(type(device) is str and len(device.split(':'))==2 and all(x.isdecimal() for x in device.split(':')),'major:minor')
                fields(counts,'rbytes wbytes rios wios')
                for v in counts.values(): integer(v,0,2**64-1)
            for category in ('cpu','memory','psi'):
                for v in snapshot[category].values(): integer(v,0,2**64-1)
        before=i['before']; after=i['after']
        def delta(a,b):
            require(set(a)==set(b),'same keyed baseline'); out={k:b[k]-a[k] for k in a}
            require(all(v>=0 for v in out.values()),'counter reset requires new baseline'); return out
        cpu=delta(before['cpu'],after['cpu']) if 'cpu' in controllers else None
        psi=delta(before['psi'],after['psi']); io=None
        if 'io' in controllers:
            require(set(before['io'])==set(after['io']),'same device baseline')
            io={dev:delta(before['io'][dev],after['io'][dev]) for dev in before['io']}
        memory=None if 'memory' not in controllers else dict(current_change=after['memory']['current']-before['memory']['current'],
                    read_peak=after['memory']['peak'], fresh_window_peak=after['memory']['peak'] if reset else None)
        return dict(route='delegated' if delegated else 'read_only', cpu_delta=cpu,
                    cpu_fraction=None if cpu is None else float(cpu['usage_usec']/window), configured_quota_fraction=None if maximum is None else float(maximum/period),
                    memory=memory, io_delta=io, psi_delta=psi,
                    writeback_attribution='supported_filesystem_still_shared_inode_limit' if writeback else 'not_supported_at_this_scope')
    if kind == 'profiling':
        fields(i,'traces sampling child_counts child_total actual_child baseline instrumented code_before code_after')
        traces=[]
        for trace in sequence(i['traces'],4):
            fields(trace,'capacity dropped events'); cap=integer(trace['capacity'],1,64); dropped=integer(trace['dropped'],0,1000)
            events=sequence(trace['events'],64); require(len(events)<=cap,'bounded hook buffer')
            stack=[]; totals={}; previous=0
            for event in events:
                fields(event,'action function time'); t=integer(event['time'],previous,10000); previous=t
                name=event['function']; require(type(name) is str and 0<len(name)<=32,'hook function name')
                require(event['action'] in ('enter','exit'),'hook event action')
                if event['action']=='enter':
                    totals.setdefault(name,dict(calls=0,inclusive=0,exclusive=0)); totals[name]['calls']+=1; stack.append([name,t,0])
                else:
                    require(bool(stack) and stack[-1][0]==name,'LIFO hook pairing')
                    _,start,child=stack.pop(); span=t-start; totals[name]['inclusive']+=span; totals[name]['exclusive']+=span-child
                    if stack: stack[-1][2]+=span
            require(dropped>0 or not stack,'incomplete trace must be declared')
            complete=dropped==0 and not stack
            traces.append(dict(complete=complete, open_frames=[f[0] for f in stack], totals=totals if complete else None))
        fields(i['sampling'],'period_ns samples'); period=integer(i['sampling']['period_ns'],1,10**9); samples=i['sampling']['samples']
        require(type(samples) is dict and type(i['child_counts']) is dict and set(i['child_counts'])==set(i['actual_child']),'profiling keyed counts')
        for value in list(samples.values())+list(i['child_counts'].values())+list(i['actual_child'].values()): integer(value,0,10**12)
        calls=sum(i['child_counts'].values()); require(calls>0,'positive child calls'); total=integer(i['child_total'],0,10**12)
        estimate={k:float(total*v/calls) for k,v in i['child_counts'].items()}
        baseline=integer(i['baseline'],1,10**12); instrumented=integer(i['instrumented'],0,10**12)
        cb=integer(i['code_before'],0,10**9); ca=integer(i['code_after'],0,10**9)
        return dict(traces=traces, sampled_self_ns={k:v*period for k,v in samples.items()}, estimated_child_ns=estimate,
                    child_error_ns={k:estimate[k]-i['actual_child'][k] for k in estimate},
                    overhead_fraction=float((instrumented-baseline)/baseline), code_growth=ca-cb)
    if kind == 'dwt':
        fields(i,'cases'); out=[]
        for c in sequence(i['cases'],12):
            fields(c,'width start end max_interval clock_hz trcena nocyc enabled halted setup_owned overhead')
            width=integer(c['width'],32,32); start=integer(c['start'],0,2**width-1); end=integer(c['end'],0,2**width-1)
            bound=integer(c['max_interval'],0,2**33); clock=integer(c['clock_hz'],1,10**10); overhead=integer(c['overhead'],0,10**9)
            for k in ('trcena','nocyc','enabled','halted','setup_owned'): boolean(c[k])
            delta=(end-start)%2**width; status='bounded'; duration=net=None; observed=None
            if not c['setup_owned']: status='setup_not_owned_project_policy'
            elif not c['trcena']: status='trace_disabled_unknown'
            elif c['nocyc']: status='unsupported_counter'
            elif not c['enabled']: status='counter_disabled'
            elif bound>=2**width: status='ambiguous_wrap'
            elif delta>bound: status='outside_interval_bound'
            else:
                observed=delta
                if c['halted']: status='debug_halt_wall_time_unsupported'
                else: duration=float(delta*10**9/clock); net=float((delta-overhead)*10**9/clock) if delta>overhead else None
            out.append(dict(status=status, cycles=observed, duration_ns=duration, sensitivity_ns=net))
        return dict(cases=out)
    raise ValueError('unknown performance certificate kind')

def verify_document(document, identifier):
    fields(document,'id description kind input expected verification_method')
    require(document['id']==identifier,'fixture ID/file/project identity')
    require(identifier in [f'PR-T43-{n:02}' for n in range(1,13)],'owned fixture identifier')
    require(document['kind']==KINDS[int(identifier[-2:])-1],'project/certificate kind binding')
    require(type(document['description']) is str and bool(document['description'].strip()), 'description')
    require(type(document['verification_method']) is str and bool(document['verification_method'].strip()), 'method')
    equal(derive(document['kind'],document['input']), document['expected'])

def catalog(root=ROOT, fixture_root=None):
    owned=fixture_root or root/'fixtures/performance'
    module=metadata(root/'curriculum/modules/T43.md')
    expected_ids=[f'PR-T43-{n:02}' for n in range(1,13)]
    require(module['projects']==expected_ids,'canonical twelve-project traversal')
    require(module['id']=='T43','canonical module identity')
    expected_projects={root/f'curriculum/projects/{identifier}.md' for identifier in expected_ids}
    actual_projects=set()
    for path in (root/'curriculum/projects').rglob('*.md'):
        card=metadata(path)
        if path.name.startswith('PR-T43-') or card.get('module')=='T43': actual_projects.add(path)
    require(actual_projects==expected_projects,'recursive owned project file set')
    paths=set()
    for identifier in expected_ids:
        meta=metadata(root/f'curriculum/projects/{identifier}.md')
        require(meta['id']==identifier and meta['module']=='T43','canonical project identity')
        require(meta['topics']==[f'T43.{n:02}' for n in TOPICS[int(identifier[-2:])-1]],'canonical project topic binding')
        require(type(meta['title']) is str and bool(meta['title'].strip()),'project title')
        require(meta['fixtures']==[f'fixtures/performance/{identifier}.json'],'canonical fixture correspondence')
        paths.add(owned/f'{identifier}.json')
    actual={p for p in owned.rglob('*') if p.is_file()}
    require(actual==paths,'recursive performance fixture file set')
    return sorted(paths)

def self_test(documents):
    controls=[]
    def reject(label, number, mutation):
        d=copy.deepcopy(documents[number-1]); mutation(d)
        try: verify_document(d,f'PR-T43-{number:02}')
        except (ValueError,TypeError,KeyError): controls.append(label); return
        raise ValueError('negative control accepted: '+label)
    reject('expected integer becomes boolean',1,lambda d:d['expected']['cases'][0].update(comparisons=True))
    reject('expected integer becomes float',1,lambda d:d['expected']['cases'][0].update(comparisons=11.0))
    reject('wrong numeric expectation',2,lambda d:d['expected'].update(total_actual=28))
    reject('input count becomes boolean',2,lambda d:d['input'].update(appends=True))
    reject('invalid CFG vertex',3,lambda d:d['input']['cfgs'][0]['edges'][0].__setitem__(1,-1))
    reject('overlapping duplicate ownership',4,lambda d:d['input']['objects'][1].update(name='work'))
    reject('undefined block mode',5,lambda d:d['input']['cases'][0][0].update(mode='discard'))
    reject('negative block index',5,lambda d:d['input']['cases'][0][0].update(item=-1))
    reject('event precedes submission',6,lambda d:d['input']['cases'][0].update(completed=-1))
    reject('over-accepted write',7,lambda d:d['input']['cases'][0]['responses'][0].update(result=9))
    reject('error after positive transfer',7,lambda d:d['input']['cases'][0]['responses'][0].update(errno='EINTR'))
    reject('empty paired samples',8,lambda d:d['input'].update(a=[]))
    reject('running exceeds enabled',9,lambda d:d['input']['cases'][0]['events'][0].update(running=21))
    reject('permission boolean becomes integer',9,lambda d:d['input']['cases'][0].update(permitted=1))
    reject('cgroup reset masquerades as delta',10,lambda d:d['input']['after']['cpu'].update(usage_usec=900))
    reject('cgroup boolean becomes integer',10,lambda d:d['input'].update(delegated=0))
    reject('hook exit violates LIFO',11,lambda d:d['input']['traces'][0]['events'][2].update(function='work'))
    reject('DWT flag becomes integer',12,lambda d:d['input']['cases'][0].update(trcena=1))
    reject('counter out of width',12,lambda d:d['input']['cases'][0].update(end=2**32))
    reject('fixture identity corruption',1,lambda d:d.update(id='PR-T43-99'))
    reject('extra expected field',1,lambda d:d['expected'].update(wcet=0))
    reject('wrong project certificate',1,lambda d:d.update(kind=documents[1]['kind'],input=documents[1]['input'],expected=documents[1]['expected']))
    reject('whitespace description',1,lambda d:d.update(description='  \n'))
    reject('whitespace verification method',1,lambda d:d.update(verification_method='  \t'))
    try: strict_json('{"id":"a","id":"b"}')
    except ValueError: controls.append('duplicate JSON key')
    else: raise ValueError('duplicate JSON keys accepted')
    # Exercise recursive catalog comparison without mutating canonical owned files.
    import tempfile
    with tempfile.TemporaryDirectory(prefix='t43-catalog-') as directory:
        base=Path(directory)/'fixtures'; base.mkdir()
        for n in range(1,13): (base/f'PR-T43-{n:02}.json').write_text('{}',encoding='utf-8')
        catalog(fixture_root=base)
        for relative in ('orphan.json','nested/orphan.json','unexpected.txt'):
            p=base/relative; p.parent.mkdir(parents=True,exist_ok=True); p.write_text('{}',encoding='utf-8')
            try: catalog(fixture_root=base)
            except ValueError: controls.append('catalog '+relative)
            else: raise ValueError('orphan accepted: '+relative)
            p.unlink()
        source=Path(directory)/'canonical'; (source/'curriculum/modules').mkdir(parents=True); (source/'curriculum/projects').mkdir()
        module_text=(ROOT/'curriculum/modules/T43.md').read_text(encoding='utf-8')
        (source/'curriculum/modules/T43.md').write_text(module_text,encoding='utf-8')
        for identifier in [f'PR-T43-{n:02}' for n in range(1,13)]:
            (source/f'curriculum/projects/{identifier}.md').write_text((ROOT/f'curriculum/projects/{identifier}.md').read_text(encoding='utf-8'),encoding='utf-8')
        catalog(root=source,fixture_root=base)
        wrong=module_text.replace('"id": "T43"','"id": "T99"',1)
        (source/'curriculum/modules/T43.md').write_text(wrong,encoding='utf-8')
        try: catalog(root=source,fixture_root=base)
        except ValueError: controls.append('wrong canonical module ID')
        else: raise ValueError('wrong module ID accepted')
        (source/'curriculum/modules/T43.md').write_text(module_text,encoding='utf-8')
        for relative in ('PR-T43-13.md','nested/PR-T43-13.md','PR-EXTRA-01.md'):
            p=source/'curriculum/projects'/relative; p.parent.mkdir(parents=True,exist_ok=True)
            p.write_text('```json\n'+json.dumps(dict(id=Path(relative).stem,module='T43'))+'\n```\n',encoding='utf-8')
            try: catalog(root=source,fixture_root=base)
            except ValueError: controls.append('catalog '+relative)
            else: raise ValueError('orphan project accepted')
            p.unlink()
        p=source/'curriculum/projects/PR-T43-01.md'; original=p.read_text(encoding='utf-8')
        p.write_text(original.replace('"T43.01"','"T43.03"',1),encoding='utf-8')
        try: catalog(root=source,fixture_root=base)
        except ValueError: controls.append('wrong canonical topic binding')
        else: raise ValueError('wrong topic binding accepted')
    return controls

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--self-test',action='store_true'); args=ap.parse_args()
    documents=[]; failures=[]
    try: paths=catalog()
    except (ValueError,KeyError,FileNotFoundError) as error:
        print(json.dumps(dict(fixtures=0,checks=checks,failures=[str(error)]))); return 1
    for path in paths:
        try:
            d=strict_json(path.read_text(encoding='utf-8')); verify_document(d,path.stem); documents.append(d)
        except (ValueError,TypeError,KeyError) as error: failures.append(path.name+': '+str(error))
    controls=[]
    if args.self_test and not failures:
        try: controls=self_test(documents)
        except (ValueError,TypeError,KeyError) as error: failures.append('negative controls: '+str(error))
    print(json.dumps(dict(fixtures=len(paths),checks=checks,negative_controls=len(controls),controls=controls,failures=failures),indent=2))
    return bool(failures)

if __name__=='__main__': raise SystemExit(main())
