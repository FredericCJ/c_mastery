"""Strict independent finite T44 certificates. Never runs learner C or diagnostic tools."""
from __future__ import annotations
from itertools import combinations
import argparse, copy, hashlib, json, tempfile
from pathlib import Path
from common import ROOT, META

FIXTURES=ROOT/'fixtures/debugging_lab'
PROJECTS={f'PR-T44-{n:02}' for n in range(1,11)}
KINDS=['observation','reduction','first_state','image_state','memory_state','io_state','concurrency','bare_metal','tool_evidence','regression']
TOPICS=[['T44.01','T44.02'],['T44.03'],['T44.04','T44.05'],['T44.06'],['T44.07'],['T44.08'],['T44.09'],['T44.10'],['T44.11'],['T44.12']]
checks=0
def require(condition,message):
    global checks
    checks+=1
    if not condition: raise ValueError(message)
def integer(x): return type(x) is int

def u32(value):
    require(integer(value) and 0<=value<2**32,'raw uint32 word/address')
    return value
def shape(d,keys): require(type(d) is dict and set(d)==set(keys),'exact fields: '+str(keys))
def inputs(value):
    if type(value) is dict:
        require(all(type(k) is str for k in value),'string JSON keys')
        for v in value.values(): inputs(v)
    elif type(value) is list:
        require(len(value)<=256,'finite input collection bound')
        for v in value: inputs(v)
    else: require(value is None or type(value) in (int,str),'strict input scalar; no booleans/floats as integers')
def equal(a,b):
    if type(a) is not type(b): return False
    if type(a) is dict: return set(a)==set(b) and all(equal(a[k],b[k]) for k in a)
    if type(a) is list: return len(a)==len(b) and all(equal(x,y) for x,y in zip(a,b))
    return a==b
def exact_json_text(text):
    def pairs(values):
        d={}
        for k,v in values:
            if k in d: raise ValueError('duplicate JSON key '+k)
            d[k]=v
        return d
    def invalid(v): raise ValueError('nonfinite JSON number '+v)
    return json.loads(text,object_pairs_hook=pairs,parse_constant=invalid)
def exact_json(path): return exact_json_text(path.read_text(encoding='utf-8'))

def metadata(path):
    match=META.match(path.read_text(encoding='utf-8-sig'))
    require(match is not None,'canonical fenced metadata')
    result=exact_json_text(match[1]);require(type(result) is dict,'metadata object')
    return result

def calculate(kind,i):
    inputs(i)
    if kind=='observation':
        shape(i,['payload','records','values','capacity','events'])
        require(1<=i['capacity']<=8 and len(i['events'])<=16,'bounded log')
        digest=hashlib.sha256(i['payload'].encode('ascii')).hexdigest()
        valid=[]
        for r in i['records']:
            shape(r,['image','symbols_for','input_sha256'])
            valid.append(r['image']==r['symbols_for'] and r['input_sha256']==digest)
        require(all(0<=n<=16 for n in i['values']),'bounded validation inputs')
        return {'replay_valid':valid,'durable_validation':[1<=n<=4 for n in i['values']],
                'faulty_assignment_accepts':[True for n in i['values']],
                'retained_log':i['events'][-i['capacity']:],'separate_first_cause':i['events'][0],
                'warnings_prove_correctness':False}
    if kind=='reduction':
        shape(i,['chunks','triggers','candidate','trajectory'])
        c=i['chunks']; groups=[set(g) for g in i['triggers']]
        require(c==list(range(1,9)) and groups==[{1,7,8},{2,3}],'declared finite reduction experiment')
        def result(s):
            require(len(s)==len(set(s)) and set(s)<=set(c),'unique valid chunks')
            if any(g<=set(s) for g in groups): return 'FAIL'
            return 'UNRESOLVED' if set(s)&groups[0] else 'PASS'
        allsets=[list(s) for n in range(9) for s in combinations(c,n)]
        failing=[s for s in allsets if result(s)=='FAIL']; candidate=i['candidate']
        removals=[result(candidate[:n]+candidate[n+1:]) for n in range(len(candidate))]
        history=[result(s) for s in i['trajectory']]
        require(history and all(x=='FAIL' for x in history),'same-failure reduction trajectory')
        require(all(set(b)<set(a) for a,b in zip(i['trajectory'],i['trajectory'][1:])),'strict decreasing reduction history')
        require(i['trajectory'][-1]==candidate,'trajectory/certificate binding')
        return {'candidate_result':result(candidate),'single_deletion_results':removals,
                'one_minimal':result(candidate)=='FAIL' and all(x!='FAIL' for x in removals),
                'global_minimum_size':min(map(len,failing)),'candidate_size':len(candidate),
                'failing_configurations':len(failing),'trajectory_results':history,
                'localization_proves_causal_repair':False}
    if kind=='first_state':
        shape(i,['capacity','events','frames','hardware_width_bytes','requested_width_bytes','hardware_slots','requested_watches','threads'])
        cap=i['capacity']; require(1<=cap<=8,'capacity')
        good=bad=0; good_states=[]; bad_states=[]; statuses=[]; first=None
        for n,event in enumerate(i['events'],1):
            require(event in ('acquire','release'),'bounded counter event')
            if event=='acquire':
                if good<cap: good+=1; statuses.append('accepted')
                else: statuses.append('full')
                if bad<=cap: bad+=1
            else:
                require(good>0 and bad>0,'release owned counter'); good-=1;bad-=1;statuses.append('released')
            good_states.append(good);bad_states.append(bad)
            if bad>cap and first is None: first=n
        require(len(i['frames'])==2,'two selected frames')
        for f in i['frames']: shape(f,['name','status'])
        require(all(integer(i[k]) and i[k]>0 for k in ['hardware_width_bytes','requested_width_bytes','hardware_slots','requested_watches','threads']),'positive watch geometry')
        return {'correct_states':good_states,'faulty_states':bad_states,'statuses':statuses,'first_invalid_event':first,
                'selected_frame_status':i['frames'][0]['status'],'caller_status':i['frames'][1]['status'],
                'hardware_width_fits':i['requested_width_bytes']<=i['hardware_width_bytes'],
                'hardware_slots_fit':i['requested_watches']<=i['hardware_slots'],
                'software_watch_covers_all_threads':i['threads']==1}
    if kind=='image_state':
        shape(i,['images','entry_rsp','exit_rsp','preserved_before','preserved_after','line_ranges','pc','saved_sections','requested_sections','optimized_local'])
        identities=[]
        for r in i['images']:
            shape(r,['bytes_ascii','sha256','symbols_for_sha256'])
            h=hashlib.sha256(r['bytes_ascii'].encode('ascii')).hexdigest()
            identities.append(h==r['sha256']==r['symbols_for_sha256'])
        matches=[r[2] for r in i['line_ranges'] if r[0]<=i['pc']<r[1]]
        require(len(matches)==1 and all(len(r)==3 and r[0]<r[1] for r in i['line_ranges']),'one half-open source range')
        return {'matched_images':identities,'entry_sysv_alignment':i['entry_rsp']%16==8,
                'stack_restored':i['entry_rsp']==i['exit_rsp'],'preserved_register_restored':i['preserved_before']==i['preserved_after'],
                'source_line':matches[0],'snapshot_available':[s in i['saved_sections'] for s in i['requested_sections']],
                'optimized_local_available':i['optimized_local']!='not_materialized','snapshot_is_full_history':False}
    if kind=='memory_state':
        shape(i,['cases']);bad=[];classes=[]
        for c in i['cases']:
            shape(c,['class','extent','events'])
            require(c['class'] in ('bounds','lifetime','initialization','resize') and 0<=c['extent']<=16,'memory model class/extent')
            live=False; initialized=False;alias=False;invalid=None
            for n,event in enumerate(c['events'],1):
                shape(event,['op','offset']);op=event['op'];offset=event['offset']
                require(op in ('allocate','write','free','read','use_value','alias','resize_success','resize_failure','alias_read'),'memory model operation')
                failure=False
                if op=='allocate': live=True;initialized=False
                elif op=='write': failure=not live or not 0<=offset<c['extent'];initialized=not failure
                elif op=='free': failure=not live;live=False;alias=False
                elif op=='read': failure=not live or not 0<=offset<c['extent']
                elif op=='use_value': failure=not live or not initialized
                elif op=='alias': failure=not live;alias=live
                elif op=='resize_success': failure=not live;alias=False
                elif op=='resize_failure': failure=not live
                elif op=='alias_read': failure=not live or not alias
                if failure and invalid is None: invalid=n
            bad.append(invalid);classes.append(c['class'] if invalid is not None else 'valid_under_model')
        return {'first_invalid_event':bad,'classification':classes,'prescribed_ub_output':False,'model_establishes_tool_detection':False}
    if kind=='io_state':
        shape(i,['payload_hex','write_results','read_results','failed_errno','logging_errno','sigpipe_dispositions','fd','old_generation','new_generation','close_error','base_descriptors','acquisitions','descriptor_cap'])
        raw=bytes.fromhex(i['payload_hex']);off=0;offsets=[]; emitted=[]
        for result in i['write_results']:
            if integer(result):
                require(0<result<=len(raw)-off,'positive bounded write progress');emitted.extend(raw[off:off+result]);off+=result
            else: require(result in ('EINTR','EAGAIN','EWOULDBLOCK'),'supported no-progress status')
            offsets.append(off)
        require(off==len(raw),'complete finite script')
        require(i['read_results'] and all(integer(x) and 0<=x<=16 for x in i['read_results']),'read result model')
        require(i['close_error'] in ('EIO','EINTR') and i['new_generation']>i['old_generation'],'Linux close/reuse scenario')
        require(0<=i['base_descriptors']<=i['descriptor_cap']<=16 and 0<=i['acquisitions']<=16,'capped per-process resource model')
        admitted=min(i['acquisitions'],i['descriptor_cap']-i['base_descriptors'])
        require(all(s in ('default_unblocked','ignored') for s in i['sigpipe_dispositions']),'SIGPIPE disposition scope (default is unblocked)')
        return {'offsets':offsets,'emitted_hex':bytes(emitted).hex(),'read_bytes':sum(i['read_results']),
                'eof_seen':0 in i['read_results'],'captured_errno':i['failed_errno'],'late_errno':i['logging_errno'],
                'epipe_return_observable':[s=='ignored' for s in i['sigpipe_dispositions']],
                'linux_closed_generation':i['old_generation'],'wrong_retry_closes_generation':i['new_generation'],
                'admitted_acquisitions':admitted,'exhaustion': 'EMFILE' if admitted<i['acquisitions'] else 'none',
                'write_proves_durability':False}
    if kind=='concurrency':
        shape(i,['wait_edges','accesses','happens_before','bad_wait_events','repaired_wait_events'])
        vertices={v for edge in i['wait_edges'] for v in edge};reach={v:set() for v in vertices}
        for a,b in i['wait_edges']: reach[a].add(b)
        for _ in vertices:
            for v in vertices: reach[v]|={w for x in list(reach[v]) for w in reach[x]}
        accesses={a['id']:a for a in i['accesses']}
        require(len(accesses)==len(i['accesses']),'unique access IDs')
        hb={k:set() for k in accesses}
        for a in accesses.values(): shape(a,['id','thread','object','op']);require(a['op'] in ('read','write'),'access operation')
        for edge in i['happens_before']:
            require(type(edge) is list and len(edge)==2,'HB edge pair')
            a,b=edge;require(a in hb and b in hb and a!=b,'strict HB identity');hb[a].add(b)
        for _ in hb:
            for v in hb: hb[v]|={w for x in list(hb[v]) for w in hb[x]}
        require(all(v not in hb[v] for v in hb),'acyclic happens-before relation')
        races=[]
        for a,b in combinations(accesses.values(),2):
            if a['thread']!=b['thread'] and a['object']==b['object'] and 'write' in (a['op'],b['op']) and b['id'] not in hb[a['id']] and a['id'] not in hb[b['id']]: races.append([a['id'],b['id']])
        bad=i['bad_wait_events'];fixed=i['repaired_wait_events']
        require(bad==['consumer_checks_false','producer_sets_true','producer_signals','consumer_waits'],'declared lost-wakeup history')
        require(fixed==['consumer_locks','consumer_checks_false','consumer_wait_releases_mutex','producer_locks','producer_sets_true','producer_signals','producer_unlocks','consumer_reacquires_mutex','consumer_checks_true'],'declared predicate repair history')
        return {'wait_cycle':any(v in reach[v] for v in vertices),'unordered_conflicts':races,'lost_wakeup_in_bad_history':True,
                'predicate_seen_after_reacquire':fixed[-1]=='consumer_checks_true','forced_history_proves_all_schedules':False,'prescribed_race_value':False}
    if kind=='bare_metal':
        shape(i,['vtor','reset_vector','irq','pre_psp','exc_return','basic_words','faults','dma_events','clock_events'])
        for key in ['vtor','reset_vector','pre_psp','exc_return']:u32(i[key])
        require(type(i['basic_words']) is list and len(i['basic_words'])==8,'eight raw basic-frame words')
        for word in i['basic_words']:u32(word)
        require(integer(i['irq']) and 0<=i['irq']<240,'Cortex-M4 external interrupt range')
        require(i['vtor']%128==0 and i['vtor']+4*(i['irq']+16)+4<=2**32,'aligned bounded vector entry')
        require(i['pre_psp']>=32,'basic frame does not underflow address space')
        require(i['exc_return']==0xFFFFFFFD and len(i['basic_words'])==8 and i['pre_psp']%8==0,'basic PSP/thread frame')
        require(i['basic_words'][7]&(1<<24) and not i['basic_words'][7]&(1<<9) and i['basic_words'][7]&0x1ff==0,'Thumb, thread, no padding scoped frame')
        faults=[]
        for f in i['faults']:
            shape(f,['cfsr','hfsr','bfar'])
            for key in ['cfsr','hfsr','bfar']:u32(f[key])
            faults.append({'precise_bus':bool(f['cfsr']&0x200),'bfar_valid':bool(f['cfsr']&0x8000),'forced_hardfault':bool(f['hfsr']&0x40000000),'usable_bfar':f['bfar'] if f['cfsr']&0x8000 else None})
        owner='CPU';owners=[];bad=None
        for n,event in enumerate(i['dma_events'],1):
            if event=='give': require(owner=='CPU','give owner');owner='DMA'
            elif event=='cancel_request': require(owner=='DMA','request is not stopped')
            elif event=='cpu_write':
                if owner!='CPU' and bad is None: bad=n
            elif event=='ack_stop': require(owner=='DMA','observed stop acknowledgment');owner='CPU'
            else: raise ValueError('unsupported DMA event')
            owners.append(owner)
        clock=False;clock_bad=None
        for n,event in enumerate(i['clock_events'],1):
            if event=='enable': clock=True
            elif event=='configure':
                if not clock and clock_bad is None: clock_bad=n
            else: raise ValueError('clock stage')
        return {'reset_pc':i['reset_vector']&~1,'reset_thumb':bool(i['reset_vector']&1),'exception_number':i['irq']+16,
                'vector_address':i['vtor']+4*(i['irq']+16),'frame_base':i['pre_psp']-32,'stacked_pc':i['basic_words'][6],
                'fault_classification':faults,'dma_owner_after':owners,'first_invalid_dma_event':bad,
                'first_invalid_clock_stage':clock_bad,'model_establishes_physical_fault':False}
    if kind=='tool_evidence':
        shape(i,['u8_pairs','candidate_pair','tool_claims','evidence_categories'])
        require(all(len(p)==2 and all(integer(x) and 0<=x<=255 for x in p) for p in i['u8_pairs']+[i['candidate_pair']]),'u8 input scope')
        reference=[min(a+b,255) for a,b in i['u8_pairs']];wrong=[(a+b)%256 for a,b in i['u8_pairs']]
        require(i['tool_claims']==['clean_ASan_proves_all_memory','unavailable_Memcheck_is_pass','static_alarm_is_executed_failure'],'declared false tool inferences')
        require(i['evidence_categories']==['model','unavailable','static_alarm','executed_scope'],'declared evidence labels')
        a,b=i['candidate_pair']
        return {'reference':reference,'faulty_u8_results':wrong,'differential_mismatches':[n for n,(a,b) in enumerate(zip(reference,wrong)) if a!=b],
                'candidate_fails':min(a+b,255)!=(a+b)%256,'global_smallest_failing_sum':next(s for s in range(511) if min(s,255)!=s%256),
                'tool_claims_valid':[False,False,False],'model_is_actual_tool_detection':False}
    if kind=='regression':
        shape(i,['capacity','start','attempts','patches','builds','day_initial','day_transfer','evidence_records'])
        require(0<=i['start']<=i['capacity']<=8 and 0<i['attempts']<=16,'regression scope')
        require(i['patches']==['reject_before_commit','clamp_after_commit'] and i['builds']==['debug','release','instrumented'],'declared patches/matrix')
        correct=i['start'];temporary=i['start'];accepted=[];violations=[]
        for n in range(1,i['attempts']+1):
            admitted=correct<i['capacity'];accepted.append(admitted)
            if admitted: correct+=1
            temporary+=1
            if temporary>i['capacity']: violations.append(n)
            temporary=min(temporary,i['capacity'])
        valid=[]
        for r in i['evidence_records']:
            shape(r,['category','claim'])
            valid.append((r['category'],r['claim']) in [('model','finite_history'),('runtime','executed_scope'),('physical','electrical_timing')])
        require(i['day_transfer']>i['day_initial'],'positive delayed transfer')
        return {'causal_patch_accepts':accepted,'causal_final':correct,'clamp_transient_violations':violations,
                'same_final_state_proves_same_history':False,'required_builds':i['builds'],
                'delayed_days':i['day_transfer']-i['day_initial'],'claim_scope_valid':valid}
    raise ValueError('unknown fixture kind '+kind)

def validate_catalog(paths,references,projects):
    require(projects==PROJECTS,'exact ten T44 projects')
    require({p.relative_to(ROOT).as_posix() for p in paths}==set(references),'complete recursive fixture/catalog correspondence')
    require(len(paths)==10,'exact ten files including nested artifacts')
    require(set(references.values())==PROJECTS,'every fixture bound to its owning canonical project')
    require(all(list(references.values()).count(pid)==1 for pid in PROJECTS),'one unique fixture per project')

def canonical_catalog(project_folder,module_path):
    module=metadata(module_path)
    require(module.get('id')=='T44' and module.get('projects')==sorted(PROJECTS),'canonical module identity/project list')
    references={};projects=set()
    for p in sorted(project_folder.rglob('*')):
        if not p.is_file():continue
        if p.suffix!='.md':
            require(not p.name.startswith('PR-T44-'),'unexpected owned project format')
            continue
        meta=metadata(p)
        if meta.get('module')!='T44' and not p.name.startswith('PR-T44-'):continue
        require(meta.get('module')=='T44' and meta.get('id') in PROJECTS,'canonical project owner/identity')
        pid=meta['id'];index=int(pid[-2:])-1
        require(p.relative_to(project_folder).as_posix()==pid+'.md','canonical owned project file path')
        require(pid not in projects,'unique canonical project ID')
        require(meta.get('topics')==TOPICS[index],'canonical project/topic binding')
        require(meta.get('fixtures')==[f'fixtures/debugging_lab/{pid}.json'],'canonical project/fixture binding')
        projects.add(meta['id'])
        for ref in meta['fixtures']:
            require(ref not in references,'duplicate canonical fixture binding');references[ref]=meta['id']
    require(projects==PROJECTS,'complete recursive canonical owned project set')
    return references,projects

def catalog(folder=FIXTURES):
    paths=sorted(p for p in folder.rglob('*') if p.is_file())
    references,projects=canonical_catalog(ROOT/'curriculum/projects',ROOT/'curriculum/modules/T44.md')
    validate_catalog(paths,references,projects)
    return paths,references

def validate_fixture(path,f,binding):
    require(path.resolve().is_relative_to(FIXTURES.resolve()) and path.suffix=='.json','fixture path/format')
    shape(f,['id','kind','description','input','expected','verification_method'])
    require(f['id']==path.stem==binding,'fixture ID/file/project binding')
    require(f['id'] in PROJECTS,'owned fixture ID')
    index=int(f['id'][-2:])-1;require(f['kind']==KINDS[index],'project/kind binding')
    require(type(f['description']) is str and bool(f['description'].strip()) and type(f['verification_method']) is str and bool(f['verification_method'].strip()),'oracle scope')
    require(equal(calculate(f['kind'],f['input']),f['expected']),'derived strict expected fields differ: '+path.name)

def verify_all():
    paths,refs=catalog()
    for path in paths:
        validate_fixture(path,exact_json(path),refs[path.relative_to(ROOT).as_posix()])
    return paths

def negative_controls(paths):
    rejected=[]
    def rejects(label,operation):
        try: operation()
        except ValueError: rejected.append(label)
        else: raise ValueError('negative control accepted: '+label)
    for path in paths:
        f=exact_json(path);bad=copy.deepcopy(f['expected']);key=next(iter(bad))
        bad[key]=None if bad[key] is not None else 'corrupt'
        require(not equal(calculate(f['kind'],f['input']),bad),'each changed expected field rejected');rejected.append(path.name+':expected')
    f=exact_json(paths[0]);bad=copy.deepcopy(f['input']);bad['capacity']=True
    try: calculate(f['kind'],bad)
    except ValueError: rejected.append('boolean-as-integer')
    else: raise ValueError('boolean integer corruption accepted')
    bad=copy.deepcopy(f['input']);bad['capacity']=3.0
    try: calculate(f['kind'],bad)
    except ValueError: rejected.append('float-as-integer')
    else: raise ValueError('float integer corruption accepted')
    # The production catalog/record validators receive actual mutated in-memory inputs.
    # Canonical files are never changed by controls.
    _,refs=catalog()
    rejects('unexpected-nested-file',lambda:validate_catalog(paths+[FIXTURES/'nested/unexpected.json'],refs,PROJECTS))
    rejects('missing-file',lambda:validate_catalog(paths[1:],refs,PROJECTS))
    rejects('missing-canonical-project',lambda:validate_catalog(paths,refs,PROJECTS-{'PR-T44-01'}))
    changed=copy.deepcopy(refs);changed[paths[0].relative_to(ROOT).as_posix()]='PR-T44-02'
    rejects('duplicate-project-binding',lambda:validate_catalog(paths,changed,PROJECTS))
    changed=copy.deepcopy(exact_json(paths[0]));changed['id']='PR-T44-99'
    rejects('wrong-ID',lambda:validate_fixture(paths[0],changed,'PR-T44-01'))
    changed=copy.deepcopy(exact_json(paths[0]));changed['kind']='reduction'
    rejects('wrong-project-kind',lambda:validate_fixture(paths[0],changed,'PR-T44-01'))
    changed=copy.deepcopy(exact_json(paths[0]));changed['unexpected']=0
    rejects('unexpected-record-field',lambda:validate_fixture(paths[0],changed,'PR-T44-01'))
    rejects('duplicate-JSON-key',lambda:exact_json_text('{"n":1,"n":2}'))
    rejects('nonfinite-JSON-number',lambda:exact_json_text('{"n":NaN}'))
    changed=copy.deepcopy(exact_json(paths[0]));changed['description']='  \n'
    rejects('whitespace-description',lambda:validate_fixture(paths[0],changed,'PR-T44-01'))
    changed=copy.deepcopy(exact_json(paths[0]));changed['verification_method']='\t'
    rejects('whitespace-method',lambda:validate_fixture(paths[0],changed,'PR-T44-01'))
    frame=exact_json(FIXTURES/'PR-T44-08.json')['input']
    for label,index,value in [('negative-frame-word',0,-1),('wide-frame-word',0,2**32),('negative-xPSR',7,-2**24)]:
        changed=copy.deepcopy(frame);changed['basic_words'][index]=value
        rejects(label,lambda changed=changed:calculate('bare_metal',changed))
    changed=copy.deepcopy(frame);changed['faults'][1]['bfar']=-1
    rejects('negative-stale-BFAR',lambda:calculate('bare_metal',changed))
    changed=copy.deepcopy(frame);changed['pre_psp']=16
    rejects('frame-address-underflow',lambda:calculate('bare_metal',changed))
    changed=copy.deepcopy(frame);changed['vtor']+=1
    rejects('unaligned-vector-base',lambda:calculate('bare_metal',changed))
    changed=copy.deepcopy(frame);changed['irq']=240
    rejects('external-IRQ-range',lambda:calculate('bare_metal',changed))
    graph=exact_json(FIXTURES/'PR-T44-07.json')['input'];access_ids=[a['id'] for a in graph['accesses']]
    changed=copy.deepcopy(graph);changed['happens_before']=[[access_ids[0],access_ids[0]]]
    rejects('self-HB-edge',lambda:calculate('concurrency',changed))
    changed=copy.deepcopy(graph);changed['happens_before']=[[access_ids[0],access_ids[1]],[access_ids[1],access_ids[0]]]
    rejects('cyclic-HB-edges',lambda:calculate('concurrency',changed))
    with tempfile.TemporaryDirectory(prefix='t44_catalog_') as temporary:
        base=Path(temporary);projects=base/'projects';projects.mkdir();module=base/'T44.md'
        original_module=(ROOT/'curriculum/modules/T44.md').read_text(encoding='utf-8')
        module.write_text(original_module,encoding='utf-8')
        for pid in sorted(PROJECTS):(projects/(pid+'.md')).write_bytes((ROOT/'curriculum/projects'/(pid+'.md')).read_bytes())
        canonical_catalog(projects,module)
        nested=projects/'nested';nested.mkdir();extra=nested/'PR-EXTRA-01.md'
        extra.write_text('```json\n'+json.dumps({'id':'PR-EXTRA-01','module':'T44','fixtures':[]})+'\n```\n',encoding='utf-8')
        rejects('nested-metadata-owned-project',lambda:canonical_catalog(projects,module));extra.unlink();nested.rmdir()
        module.write_text(original_module.replace('"id": "T44"','"id": "T99"',1),encoding='utf-8')
        rejects('wrong-canonical-module-ID',lambda:canonical_catalog(projects,module));module.write_text(original_module,encoding='utf-8')
        m=metadata(module);m['projects']=m['projects'][:-1]
        module.write_text('```json\n'+json.dumps(m)+'\n```\n',encoding='utf-8')
        rejects('wrong-canonical-module-project-list',lambda:canonical_catalog(projects,module));module.write_text(original_module,encoding='utf-8')
        project=projects/'PR-T44-01.md';original=project.read_text(encoding='utf-8')
        project.write_text(original.replace('"T44.01"','"T44.03"',1),encoding='utf-8')
        rejects('wrong-canonical-project-topic',lambda:canonical_catalog(projects,module))
    require(not equal({'n':True},{'n':1}) and not equal({'n':1.0},{'n':1}),'strict expected number controls')
    return rejected+['strict-expected-types']

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--negative-controls',action='store_true');args=p.parse_args()
    paths=verify_all();negative=negative_controls(paths) if args.negative_controls else []
    print(json.dumps({'fixtures':len(paths),'checks':checks,'negative_controls':negative,'scope':'Finite static models only; no learner C, GDB, sanitizer, core or hardware result established'},indent=2))
if __name__=='__main__': main()
