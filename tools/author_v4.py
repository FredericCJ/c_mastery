"""Author the v4 learning overlay from retained, mechanism-specific teaching.

Historical briefs stay untouched. Units carry the actual topic operation and its
acceptance text, not a generic substituted engineering question. New executable
units are owned by their family manifests and fit inside the same envelopes.
"""
from __future__ import annotations
from collections import defaultdict, Counter
from copy import deepcopy
import math
import re
from common import ROOT, dump, load, read_json, write
from v4_common import V4, capture_baseline, family_units

PHASES = [
    ('Trustworthy small tools', 'Bounded record tool, checked input, explicit errors and repaired regressions.'),
    ('A portable toolbox', 'Bytes, containers, parsers, storage policies and algorithms shared by two consumers.'),
    ('Crossing execution boundaries', 'Actual files, pipes, processes and reset/UART adapters explain machine and OS boundaries.'),
    ('Responsive communicating systems', 'Telemetry and HTTP tolerate segmentation, overload, deadlines and shutdown.'),
    ('Original concurrent execution', 'Cooperative contexts, preemption, synchronization, scheduling and early protected execution.'),
    ('Programmable and durable components', 'Hosted VM and recoverable store reuse representations, allocation and fault harnesses.'),
    ('Multiple processors and ownership', 'SMP and AMP revalidate publication, cache/DMA visibility and reclamation separately.'),
    ('Composed protected systems', 'The same VM/store and protocol components operate across process and target boundaries.'),
    ('Architectural revision and defense', 'Changed requirements force a justified migration and new independent evidence.'),
]
PHASE_DOMAINS = {
    1: [1,2,3,4,5,6,7,32,44,45], 2: [12,13,14,15,16,17,18,19,20,21,22,33,34,35,36,43],
    3: [8,9,10,11,23,27,28,29,31,37,49], 4: [24,38,39,40,41,42],
    5: [25,30,48,50,51,52], 6: [46,47], 7: [26,53,54,55], 8: [56], 9: []}
DOMAIN_PHASE = {f'T{d:02}': phase for phase, domains in PHASE_DOMAINS.items() for d in domains}
ARTIFACTS = [
    ('records.tool','PR-T01-03','Portable bounded list/record core with checked input and stable errors; export the library, keeping the CLI wrapper local.'),
    ('bytes.checked','PR-T04-01','Finite-width arithmetic reports overflow before changing output; division edge cases declared.'),
    ('bytes.cursor','PR-T03-02','Borrowed byte region with checked remaining length; failed reads preserve cursor state.'),
    ('parser.integer','PR-T12-03','Strict bounded signed-decimal conversion; reject overflow and malformed tails with an explicit consumed length and unchanged output.'),
    ('containers.ring','PR-T14-01','Caller-owned FIFO, explicit full/empty state and split spans; no implied concurrency guarantee.'),
    ('memory.pool','PR-T16-03','Typed fixed storage, bounded exhaustion and generation-checked handles; no stale identity reuse.'),
    ('codec.records','PR-T20-01','Canonical record bytes independent of host layout; partial/malformed input cannot publish a partial record.'),
    ('io.stream','PR-T19-04','Explicit partial progress, end/error distinction and ownership; adapters declare blocking and cancellation.'),
    ('io.async','PR-T21-02','Submit/cancel/complete ownership: cancellation requests do not imply completion; exactly one terminal release after backend quiescence.'),
    ('numeric.filter','PR-T17-05','Scalar numerical contract, bounded history, rounding/overflow and error tolerance declared.'),
    ('execution.tasks','PR-T50-04','Bounded task identity and ready/blocked/sleeping/exited transitions with explicit context ownership.'),
    ('sync.copied_queue','PR-T51-06','Copied bounded messages; atomic enrollment, timeout/cancel arbitration and teardown ownership.'),
    ('sync.spsc','PR-T55-01','Exactly one producer and consumer, acquire/release publication and explicit storage lifetime.'),
    ('runtime.vm','PR-T46-10','CVM1 image and existing host API; bounded resources, guest errors and host effects remain distinct.'),
    ('storage.pages','PR-T47-09','Existing byte-exact page format, caller serialization, complete-page I/O and explicit durability outcomes.'),
    ('kernel.frames','PR-T48-04','Reserved/owned/free physical frames and page-table ownership; no premature reuse.'),
    ('kernel.processes','PR-T56-02','Separate process/thread identities, bounded handles and exactly-once reaping.'),
    ('testing.faults','PR-T32-02','Deterministic input/fault schedule, independent oracle and minimized first divergence.'),
    ('measurement.capture','PR-T43-01','Units, clock, workload, source/build identity and uncertainty accompany every measurement.'),
]
DECISIONS = {
    'PR-T01-03': ('How will malformed input affect the bounded record tool?', ['reject the whole command','retain a valid prefix and report consumed input'], 'Compare empty, exact-capacity and malformed-tail traces.', 'A caller requires atomic batches.'),
    'PR-T14-01': ('How will the reusable ring represent full and empty?', ['explicit count','monotonic positions with bounded distance'], 'Trace capacity 1, capacity 7, wrap and a rejected push.', 'The interface changes to SPSC publication.'),
    'PR-T03-02': ('How will a cursor expose failed reads?', ['transactional output parameters','tagged result with explicit consumed count'], 'Try short spans and a failure after a valid prefix.', 'A streaming caller needs partial progress.'),
    'PR-T16-03': ('How will released slot identities be protected?', ['generation handles','borrow-scoped direct pointers'], 'Retain an alias through release and immediate slot reuse.', 'A completion can arrive after the owner is destroyed.'),
    'PR-T20-01': ('Which parts of record processing belong in the portable core?', ['core owns framing state','adapter owns framing and feeds complete records'], 'Feed the same record using file, short-read and resettable adapters.', 'Transport segmentation and reset semantics change.'),
    'PR-T24-04': ('How will the service schedule ready work?', ['single readiness loop','bounded worker pool'], 'Compare slow peers, fairness, memory ceilings and shutdown.', 'CPU-heavy requests become common.'),
    'PR-T50-05': ('Which timer representation fits this task bound?', ['sorted deadline list','binary min-heap'], 'Measure insertion/cancellation and worst traversal at the declared task count.', 'The task population or cancellation rate changes.'),
    'PR-T51-06': ('Who owns a message during delivery?', ['copy into queue-owned slots','transfer generation-checked ownership'], 'Inject timeout, cancellation and receiver destruction during handoff.', 'Payload size or DMA lifetime changes.'),
    'PR-T47-07': ('When can a commit report durability?', ['synchronous commit barrier','grouped barrier with explicit pending state'], 'Crash before and after every write/barrier boundary.', 'Latency requirements change without weakening durability.'),
    'PR-T54-06': ('How will CPU and DMA buffer ownership cross cores?', ['copied messages','exclusive cache-line buffer transfer'], 'Delay completion and reset a peer while it owns a buffer.', 'A peer reboots independently.'),
    'PR-T56-10': ('Who serializes the durable store?', ['single-owner service','shared access behind an explicit lock protocol'], 'Compare failure propagation, cancellation and recovery authority.', 'A client dies during a request.'),
}
for _stem, _question, _alternatives, _probe, _reversal in [
    ('P1','Where do parsing, validation and storage ownership meet?', ['transactional record staging','incremental staging with explicit rollback'], 'Inject a malformed tail and exhaustion after a valid prefix.', 'Records become larger than the staging budget.'),
    ('P2','Who retains bytes after a partial output operation?', ['caller retains pending spans','adapter owns a bounded copy'], 'Force short writes, interruption and terminal failure.', 'The source buffer must be reused before completion.'),
    ('P3','Who owns framing and resynchronization?', ['transport adapter','portable incremental decoder'], 'Split and corrupt frames at every byte boundary.', 'A second transport has different reset semantics.'),
    ('P4','When may a DMA buffer return to the producer?', ['copy after quiescence','exclusive generation-checked transfer'], 'Cancel and reset during an outstanding transfer.', 'A second consumer retains samples.'),
    ('P5','How are completion and cancellation serialized?', ['single event-loop owner','explicit synchronized state machine'], 'Race cancellation, completion and teardown.', 'Completion can occur on a second core.'),
    ('P6','Which measurement and numerical contracts survive reuse?', ['fixed-point bounded pipeline','floating-point pipeline with tolerance'], 'Compare error, worst-case input and measurement overhead.', 'The sampling rate or error budget changes.'),
    ('CAP-LINUX','How will execution, persistence and service lifetimes be separated?', ['single-owner services with messages','shared components with explicit locking'], 'Compare restart, partial failure and overload traces.', 'One service must restart without losing durable work.'),
    ('CAP-MCU','Who owns acquisition, transport and recovery across execution contexts?', ['copied bounded channels','exclusive buffer handoff'], 'Trace overload, peer reset, DMA cancellation and watchdog recovery.', 'A second core or peripheral retains a buffer.'),
]:
    DECISIONS['PR-'+_stem+'-02']=(_question,_alternatives,_probe,_reversal)

INTERFACE_PROJECTS = {
    'PR-T38-01':['UART'], 'PR-T38-02':['USART'], 'PR-T38-03':['I2C'],
    'PR-T38-04':['I2C'], 'PR-T38-05':['SPI'], 'PR-T38-06':['RS-232'],
    'PR-T38-07':['RS-422'], 'PR-T38-08':['RS-485'], 'PR-T38-09':['CAN'],
    'PR-T38-10':['CAN FD'], 'PR-T38-11':['CAN FD'], 'PR-T38-12':['CAN FD'],
    'PR-T39-02':['Ethernet'], 'PR-T39-04':['Ethernet'], 'PR-T39-05':['Ethernet'],
    'PR-T39-06':['Ethernet'], 'PR-T39-07':['Ethernet'], 'PR-T39-08':['Ethernet'],
    'PR-T40-02':['RS-485'], 'PR-T40-08':['RS-485'],
    'PR-T41-03':['Ethernet'], 'PR-T41-08':['Ethernet'],
}
REQUIRED_INTERFACES=['UART','USART','I2C','SPI','RS-232','RS-422','RS-485','CAN','CAN FD','Ethernet']
INTEGRATION_IMPORTS = {
    'P1':['containers.ring','memory.pool'], 'P2':['bytes.cursor','bytes.checked'],
    'P3':['bytes.cursor','codec.records'], 'P4':['containers.ring','codec.records'],
    'P5':['containers.ring','io.stream'], 'P6':['numeric.filter','measurement.capture'],
    'CAP-LINUX':['codec.records','containers.ring','io.stream','testing.faults'],
    'CAP-MCU':['codec.records','containers.ring','numeric.filter','execution.tasks'],
}
CRITICAL = {
    'records.tool':['T01.05.a','T01.06.a','T01.06.c'],
    'bytes.checked':['T04.03.a','T04.03.b','T04.03.c','T04.04.c'],
    'bytes.cursor':['T03.05.a','T03.05.b','T03.05.c','T03.06.c'],
    'parser.integer':['T12.08.a','T12.08.b','T12.08.c'],
    'containers.ring':['T14.01.c','T14.02.b','T14.06.a','T14.06.b','T14.06.c'],
    'memory.pool':['T16.06.a','T16.06.b','T16.06.c'],
    'codec.records':['T20.01.c','T20.02.a','T20.02.b','T20.02.c'],
    'io.stream':['T19.08.a','T19.08.b','T19.08.c'],
    'io.async':['T21.06.a','T21.06.b','T21.06.c'],
    'numeric.filter':['T17.06.a','T17.06.b','T17.06.c'],
    'execution.tasks':['T50.04.a','T50.04.b','T50.04.c'],
    'sync.copied_queue':['T51.06.a','T51.06.b','T51.06.c'],
    'sync.spsc':['T55.01.a','T55.01.b','T55.01.c'],
    'runtime.vm':['T46.10.a','T46.10.b','T46.10.c'],
    'storage.pages':['T47.09.a','T47.09.b','T47.09.c'],
    'kernel.frames':['T48.04.a','T48.04.b','T48.04.c'],
    'kernel.processes':['T56.02.a','T56.02.b','T56.02.c'],
    'testing.faults':['T32.04.a','T32.04.b','T32.05.c'],
    'measurement.capture':['T43.01.b','T43.01.c','T43.02.c'],
}

RULES_V4 = {
    'phases': PHASES, 'phase_domains': PHASE_DOMAINS, 'artifacts': ARTIFACTS, 'decisions': DECISIONS,
    'interface_projects': INTERFACE_PROJECTS, 'required_interfaces': REQUIRED_INTERFACES,
    'integration_imports': INTEGRATION_IMPORTS, 'critical': CRITICAL,
    'strands': {}, 'imports': {},
    'early_projects': ['PR-T01-03','PR-T04-01','PR-T03-02','PR-T14-01'],
    'boundary_unit': 'LAB-HS-02', 'boundary_fallback': 'PR-T23-02', 'early_observation': 'PR-T49-01',
    'budget': {'catalog_hours':4879,'core_hours':4799,'core_after_diagnostic_hours':4777},
    'consolidation_sets': '627 activated topic sets; five short reviews at a planned five minutes each',
}


def domain_phase(rules):
    return {f'T{d:02}': phase for phase, domains in rules['phase_domains'].items() for d in domains}


def section(body, name):
    match = re.search(r'^## '+re.escape(name)+r'\s*\n(.*?)(?=^## |\Z)', body, re.M|re.S)
    return match[1].strip() if match else ''


def clean(value):
    value=re.sub(r'\[([^\]]+)\]\((PR-[^)]+)\.md\)',r'\1 (\2)',value)
    return re.sub(r'\s+', ' ', re.sub(r'\{ref\}`([^`]+)`', r'\1', value)).strip()


def topic_passage(body, heading, topic):
    value = section(body, heading)
    match = re.search(r'^- \*\*'+re.escape(topic)+r':\*\*\s*(.*)', value, re.M)
    if match: return clean(match[1])
    paragraphs = [clean(x) for x in re.split(r'\n\s*\n', value) if x.strip()]
    return next((x for x in paragraphs if not x.startswith(('Before running,','This ','Submit contract,','For each objective,','1. Write','2. Establish'))), paragraphs[0] if paragraphs else '')


def phase_for(meta, rules=RULES_V4):
    key = meta['id']; owner = meta['module']
    if owner == 'INTEGRATION':
        stem=key.removeprefix('PR-').rsplit('-',1)[0]
        return {'P1':2,'P2':3,'P3':4,'P4':4,'P5':5,'P6':4,'CAP-LINUX':9,'CAP-MCU':9}[stem]
    if key == 'PR-T55-01': return 5
    if key in {'PR-T48-12','PR-T48-13','PR-T48-14'}: return 8
    if key in {'PR-T20-03','PR-T20-04'}: return 5
    if key == 'PR-T19-04': return 3
    return domain_phase(rules)[owner]


def strand_for(meta, rules=RULES_V4):
    n=meta['module']
    if n in rules.get('strands',{}): return rules['strands'][n]
    if n=='INTEGRATION':
        return 'firmware' if 'MCU' in meta['id'] or '-P4-' in meta['id'] else 'service'
    if n in {'T27','T28','T29','T30','T31','T38','T39','T40','T41','T50','T51','T52','T54'}: return 'firmware'
    if n in {'T46','T47','T48','T53','T56'}: return 'appliance'
    if n in {'T23','T24','T25','T26','T42','T49','T55'}: return 'service'
    return 'toolbox'


def imports_for(meta, phase, rules=RULES_V4):
    owner=meta['module']; key=meta['id']
    if owner in rules.get('imports',{}):
        declared=rules['imports'][owner]
        return list(declared.get(key, declared.get('*', [])))
    if owner=='INTEGRATION': return rules['integration_imports'][key.removeprefix('PR-').rsplit('-',1)[0]][:]
    if owner=='T01': return []
    result=[]
    if owner in {'T14','T15','T16','T17','T18','T19','T20','T46','T47','T48'}:result+=['bytes.checked']
    if owner in {'T15','T18','T19','T20','T40','T41','T42','T46','T47'}:result+=['bytes.cursor']
    if owner in {'T20','T24','T25','T40','T41','T51','T54','T55'}:result+=['containers.ring']
    if owner in {'T24','T40','T41','T42'}:result+=['codec.records']
    if key in {'PR-T23-02','PR-T23-04'}:result+=['bytes.cursor']
    if key in {'PR-T28-06','PR-T29-03','PR-T29-04'}:result+=['containers.ring']
    if owner in {'T51','T52','T53','T54','T56'}: result+=['execution.tasks']
    if owner=='T53': result+=['kernel.frames']
    if owner=='T56': result+=['kernel.frames','sync.copied_queue']
    if key in {'PR-T48-12','PR-T48-13','PR-T48-14','PR-T56-09'}: result+=['runtime.vm']
    if key in {'PR-T48-13','PR-T48-14','PR-T56-10','PR-T56-11'}: result+=['storage.pages']
    if key.startswith('PR-T54-') and int(key[-2:])>=3: result+=['sync.spsc']
    return list(dict.fromkeys(result))


def authored_unit_minutes(meta, additions):
    """New-domain units are authored milestones, never generic filler slots."""
    if not additions:
        raise ValueError('New domain project has no authored units: ' + meta['id'])
    required = {'id', 'title', 'minutes', 'steps', 'checks', 'objectives',
                'new_executable', 'scaffolding', 'reasoning', 'deliverables',
                'rubric', 'variants', 'assessor_analysis'}
    for unit in additions:
        missing = required - set(unit)
        if missing or any(not unit[k] for k in required - {'new_executable'}):
            raise ValueError('Incomplete authored unit ' + unit.get('id', '?') + ': ' + str(sorted(missing)))
        if type(unit['minutes']) is not int or not 60 <= unit['minutes'] <= 240:
            raise ValueError('Authored unit estimate must be 60–240 minutes: ' + unit['id'])
        if type(unit['new_executable']) is not bool or (unit['new_executable'] and not unit.get('tests')):
            raise ValueError('Executable authored unit needs concrete tests: ' + unit['id'])
    minutes = [u['minutes'] for u in additions]
    if sum(minutes) != meta['hours'] * 60:
        raise ValueError('Authored unit estimates do not reconcile with project: ' + meta['id'])
    return minutes


def author(rules=RULES_V4, catalog=V4, baseline=None):
    baseline=baseline or capture_baseline()
    taxonomy, modules, projects, resources=load()
    topics={t['id']:t for d in taxonomy for t in d['subtopics']}
    known_resources={r['id'] for r in resources}
    legacy={p['id']:p for p in read_json(ROOT/'curriculum/project_graph.json')['projects']}
    additions=family_units(); extra_by_project=defaultdict(list)
    for unit in additions:
        if unit['project'] not in projects: raise ValueError('Unknown executable unit owner '+unit['project'])
        extra_by_project[unit['project']].append(unit)
    units=[]; by_project={}; overlays={}; art_owner={a:p for a,p,_ in rules['artifacts']}
    for key, project in sorted(projects.items()):
        meta=project['meta']; body=project['body']; phase=phase_for(meta,rules)
        additions=sorted(extra_by_project[key], key=lambda x:x['id'])
        authored = meta['module'] in rules.get('authored_domains', [])
        if authored:
            minutes = authored_unit_minutes(meta, additions)
            count = len(additions)
        else:
            count=max(math.ceil(meta['hours']/3),len(additions)+1,len(meta['topics']))
            # Each unit owns an exact integer number of minutes. No float drift in budgets.
            minutes=[meta['hours']*60//count]*count
            for i in range(meta['hours']*60%count): minutes[i]+=1
        family=legacy[key].get('lab_family'); contract=section(body,'Contract')
        if key in {'PR-CAP-LINUX-03','PR-CAP-MCU-03'}:family='integration'
        local=[]; project_topics=meta['topics']
        readings=modules[meta['module']]['meta']['readings'] if meta['module'] in modules else []
        if not readings:
            seen=set()
            for topic in project_topics:
                for r in modules[topic.split('.')[0]]['meta']['readings']:
                    if r['resource_id'] not in seen: readings.append(r);seen.add(r['resource_id'])
        imports=imports_for(meta,phase,rules)
        imports=[a for a in imports if art_owner.get(a)!=key]
        for i in range(count):
            extra=additions[i] if i<len(additions) else None
            topic=topics[project_topics[i%len(project_topics)]]
            operation=topic_passage(body,'Steps',topic['id']) or topic['objectives'][1]['text']
            acceptance=topic_passage(body,'Acceptance and evidence',topic['id']) or topic['objectives'][2]['text']
            failures=section(body,'Failure cases')
            faulty=next((clean(x) for x in failures.split('\n') if x.startswith('- Challenge')),clean(failures.split('\n\n')[0]))
            final=i==count-1
            kind='implementation' if i<count-1 else 'adaptation'
            if meta.get('status')=='awareness':kind='observation'
            if meta['module']=='INTEGRATION' and key.endswith(('-01','-04')) and 'CAP-' in key:kind='design' if key.endswith('-01') else 'defense'
            uid=extra['id'] if extra else 'U-'+key.removeprefix('PR-')+f'-{i+1:02}'
            selected=[]
            if final and family:
                fm=read_json(ROOT/'labs'/family/'family.json'); entries=fm.get('projects',[])
                row=entries.get(key,{}) if isinstance(entries,dict) else next((e for e in entries if isinstance(e,dict) and e['id']==key),{})
                selected=row.get('tests',[])
            title=extra['title'] if extra else (('Integrate and challenge: ' if final else 'Build and inspect: ')+topic['title'])
            objective_ids=[o['id'] for o in topic['objectives']]
            if final:objective_ids=list(dict.fromkeys(o['id'] for t in project_topics for o in topics[t]['objectives']))
            if extra and extra.get('objectives'):objective_ids=extra['objectives']
            scope='host'
            if meta['module'] in {'T27','T28','T29','T30','T31','T38','T39','T40','T41','T54'}:scope='model'
            if extra:scope=extra.get('target_scope',extra.get('evidence_kind','model' if extra['family']=='firmware_io' else 'host'))
            if scope not in {'host','model','emulator','physical'}:scope='host'
            rows=[operation, 'Before looking at previous source, predict one boundary transition for '+topic['title']+'.',
                ('Connect the new boundary to '+', '.join(imports)+'. Keep the earlier regression and record only changed assumptions.' if final and imports else 'Produce the smallest independently inspectable implementation or trace for the operation above.')]
            if extra:rows=extra.get('steps',rows)
            unit={'id':uid,'project':key,'title':title,'hours':minutes[i]/60,'minutes':minutes[i],
                'budget_owner':key,'budget_categories':['implementation','verification'],'phase':phase,'strand':strand_for(meta,rules),
                'kind':kind,'status':meta['status'],'objectives':objective_ids,'requires':([local[-1]['id']] if local else []),
                'requires_reasons':({local[-1]['id']:'Continue the same implementation with its established contract.'} if local else {}),
                'imports':imports[:],'exports':[],'recall_sets':list(dict.fromkeys(x.rsplit('.',1)[0] for x in objective_ids)),
                'critical_objectives':[],'family':extra['family'] if extra else family,'tests':extra['tests'] if extra else selected,
                'new_executable':bool(extra),'decision_required':False,'scaffolding':('worked comparison' if phase<=2 else 'bounded choice' if phase<=4 else 'independent design' if phase<=7 else 'architectural revision'),
                'steps':rows,'checks':extra.get('checks',[acceptance,faulty]) if extra else [acceptance,faulty],
                'readings':extra.get('readings',extra.get('references',readings)) if extra else readings,'claims':[{'id':uid+'.behavior','scope':scope}],
                'target_scope':scope,'milestone':final,'project_completion':final,'reuse_checkpoint':final,
                'reuses_units':[u['id'] for u in local] if final else [],
                'remaining_evidence':extra.get('remaining_evidence',legacy[key].get('manual_evidence',[])) if extra else legacy[key].get('manual_evidence',[]),
                'stop_condition':acceptance if not extra else extra.get('scope',acceptance),
                'attempt_policy':'Seal a prediction, attempt before feedback, then retain actual source/trace evidence. Full rubric is reserved for a milestone.'}
            if extra:
                unit['declared_imports']=[a['id'] if isinstance(a,dict) else a for a in extra.get('artifact_inputs',[])]
                unit['declared_exports']=[a['id'] if isinstance(a,dict) else a for a in extra.get('artifact_outputs',[])]
            if authored:
                for field in ('new_executable', 'scaffolding', 'reasoning', 'deliverables', 'rubric',
                              'variants', 'assessor_analysis', 'kind', 'scope', 'validation_status',
                              'required_exercise_assessments'):
                    if field in extra:
                        unit[field] = deepcopy(extra[field])
            unit['readings']=[dict(r,resource_id=r.get('resource_id',r.get('resource')),
                purpose=r.get('purpose','Read for this unit: '+title)) for r in unit['readings']]
            local.append(unit);units.append(unit)
        by_project[key]=local
        concrete={u['family'] for u in local if u['family']}
        if len(concrete)==1:
            inherited=next(iter(concrete))
            for u in local:
                if u['family'] is None:u['family']=inherited
            if not local[-1]['tests']:
                fm=read_json(ROOT/'labs'/inherited/'family.json');entries=fm.get('projects',[])
                row=entries.get(key,{}) if isinstance(entries,dict) else next((e for e in entries if isinstance(e,dict) and e['id']==key),{})
                local[-1]['tests']=row.get('tests',[])
        elif len(concrete)>1:
            for previous,u in zip(local,local[1:]):
                if previous['family']!=u['family']:
                    u['workspace_transition']={'from_family':previous['family'],'to_family':u['family'],
                        'procedure':'Initialize a separate workspace for the new family, retain the earlier workspace and evidence, and import compatible components explicitly. Advance never overwrites code across families.'}
                    u['steps'].insert(0,u['workspace_transition']['procedure'])
        overlays[key]={'title':meta['title'],'phase':phase,'contract':contract,'units':[u['id'] for u in local],
            'engineering_question':rules['decisions'][key][0] if key in rules['decisions'] else 'Which assumption in '+topics[project_topics[0]]['title']+' determines the allowed transition, and how will '+topic_passage(body,'Acceptance and evidence',project_topics[0])+' be demonstrated?',
            'transfer':section(body,'Transfer challenge'),'budget_hours':meta['hours'],
            'legacy_notice':'The original project is the coverage and budget envelope. Follow these v4 units and their capability prerequisites; historical whole-domain ordering is not a new readiness gate.'}
    # Host/model increments stay usable while original physical/guest obligations
    # remain explicit gates at the project boundary. Capture registration does
    # not pretend that a manual bench is an executable compiler target.
    for key,local in by_project.items():
        last=local[-1]
        if key in {'PR-CAP-MCU-02','PR-CAP-MCU-03','PR-CAP-MCU-04'}:
            last.setdefault('capture_targets',{})['stm32f439']={'physical':True,'manual_only':True}
            last['claims'].append({'id':last['id']+'.integrated-physical','scope':'physical','target':'stm32f439'})
            purpose={'02':'deployed image, startup and actual-interface behavior','03':'recoverable peripheral failure, controlled software fault and reset/recovery','04':'changed-condition comparison and integrated physical timing'}[key[-2:]]
            last['steps'].append('Capture and assess this integrated application\'s own '+purpose+' on the recorded F439 board. Register the current source/image and raw traces locally; earlier component portfolios do not satisfy this claim.')
        if key in rules['interface_projects']:
            interfaces=rules['interface_projects'][key]
            last['capture_targets']={'interface-bench':{'physical':True,'manual_only':True,'interfaces':interfaces}}
            last['claims'].append({'id':last['id']+'.physical','scope':'physical','target':'interface-bench','interfaces':interfaces})
            last['steps'].append('Register and assess current raw '+', '.join(interfaces)+' captures, image hashes and changed-condition observations using the physical-evidence dossier. Model tests alone leave this project gate open.')
        if key in {'PR-T50-12','PR-T52-10'} or key.startswith('PR-T54-'):
            target='stm32h745' if key.startswith('PR-T54-') else 'stm32f439'
            last['claims'].append({'id':last['id']+'.physical','scope':'physical','target':target})
        if last['family']=='kernel_smp':
            last['claims'].append({'id':last['id']+'.guest','scope':'emulator','target':'x86-smp','cpus':[1,2,4]})
            if key=='PR-T56-08':
                last['tests'].append('kernel_boot_smoke')
        if key=='PR-T50-06':
            last['claims'].append({'id':last['id']+'.guest','scope':'emulator','target':'cortex-m4-qemu'})
            last['target_tests']={'cortex-m4-qemu':['execution.image_cortex-m4-qemu','execution.cortex_m4_preemptive']}
    cap=by_project['PR-CAP-MCU-04'][-1]
    physical_sources=[by_project[p][-1]['id'] for p in rules['interface_projects']]
    cap['claims'].append({'id':cap['id']+'.interfaces','scope':'physical','target':'interface-bench',
        'interfaces':rules['required_interfaces'],'source_units':physical_sources})
    cap['claims'].append({'id':cap['id']+'.boards','scope':'physical','targets':['stm32f439','stm32h745'],
        'source_units':[by_project[p][-1]['id'] for p in ['PR-T50-12','PR-T52-10','PR-T54-10']]})
    for dep in physical_sources+[by_project[p][-1]['id'] for p in ['PR-T50-12','PR-T52-10','PR-T54-10']]:
        cap['requires'].append(dep)
        cap['requires_reasons'][dep]='Final physical portfolio consumes this accepted target/interface capture and its reviewed original project obligations.'
    artifacts=[]
    for aid,owner,contract in rules['artifacts']:
        producer=by_project[owner][-1]
        producer['exports'].append(aid)
        critical=rules['critical'][aid]
        artifacts.append({'id':aid,'producer_unit':producer['id'],'producer_project':owner,'contract_version':'1',
            'public_contract':contract,'critical_objectives':critical,'profiles':[projects[owner]['meta']['profile']],
            'targets':['host','model','emulator','physical'],'consumers':[],
            'export_requirements':['source files and public headers','CMake target and actual relative directories',
                'allocation/blocking/callback/ISR/retention rules','source and contract fingerprints',
                'complete pinned dependency closure','accepted claims with target and environment scope'],
            'compatibility':'Keep existing C/binary interfaces; adapters declare clock units, representation and ownership conversions.'})
    amap={a['id']:a for a in artifacts}; umap={u['id']:u for u in units}
    for unit in units:
        for aid in unit.get('declared_exports',[]):
            if not isinstance(aid,str):raise ValueError('Artifact ID must be a string: '+str(aid))
            if aid in amap:continue
            unit['exports'].append(aid)
            art={'id':aid,'producer_unit':unit['id'],'producer_project':unit['project'],'contract_version':'1',
                'public_contract':unit['stop_condition'],'critical_objectives':unit['objectives'],
                'profiles':[projects[unit['project']]['meta']['profile']],'targets':[unit['target_scope']],
                'consumers':[],'compatibility':'Export only the declared source/header interface; binary observations remain evidence with their original target scope.'}
            artifacts.append(art);amap[aid]=art
    for unit in units:
        for aid in unit.get('declared_imports',[]):
            if aid not in amap:raise ValueError('No producer for declared artifact '+aid+' used by '+unit['id'])
            if aid not in unit['imports']:unit['imports'].append(aid)
    for key, local in by_project.items():
        if projects[key]['meta']['module']=='INTEGRATION':
            stem,number=key.rsplit('-',1)
            previous=stem+'-'+f'{int(number)-1:02}'
            if previous in by_project:
                prior=by_project[previous][-1];first=local[0]
                first['requires'].append(prior['id'])
                first['requires_reasons'][prior['id']]='Continue the actual integrated component or architectural dossier from the preceding stage; preserve its implementation and regressions.'
                if prior['family']!=first['family']:
                    first['workspace_transition']={'from_family':prior['family'],'to_family':first['family'],
                        'procedure':'Retain the preceding project workspace and reviewed dossier. Initialize this family in a separate workspace, carry forward applicable learner source and regressions, and link the declared component exports explicitly.'}
                    first['steps'].insert(0,first['workspace_transition']['procedure'])
                elif first['family']:
                    first['steps'].insert(0,'Advance the existing '+first['family']+' workspace from '+previous+'; retain its source and pinned imports while adding this campaign.')
        if not key.startswith('PR-T01-'):
            dep=by_project['PR-T01-03'][-1]['id']
            local[0]['requires'].append(dep)
            local[0]['requires_reasons'][dep]='Know how to build, run, bound and diagnose a small C component; no application library is implicitly linked.'
    for unit in units:
        # Imports gate only the first increment, but every consumer records its pinned lineage.
        first=by_project[unit['project']][0]
        for aid in unit['imports']:
            art=amap[aid]; art['consumers'].append(unit['id'])
            dep=art['producer_unit']
            consumer=unit if unit['new_executable'] else first
            if dep==consumer['id']:raise ValueError('Unit imports its own future output: '+dep)
            if dep not in consumer['requires']:
                consumer['requires'].append(dep);consumer['requires_reasons'][dep]='Import '+aid+': '+art['public_contract']
        if unit['milestone']:
            unit['critical_objectives']=list(dict.fromkeys(o for a in unit['imports'] for o in amap[a]['critical_objectives']))
        if unit['project'] in rules['decisions'] and unit['milestone']:
            question,alts,probe,reversal=rules['decisions'][unit['project']]
            unit.update(decision_required=True,decision_before=first['id'],decision_prompt={
                'question':question,'alternatives':alts,'discriminator':probe,'reversal_trigger':reversal})
            first['steps'].insert(0,'Before the main implementation, seal the design decision for '+unit['id']+': '+question+' Compare '+ ' versus '.join(alts)+'. '+probe)
    # Preserve exact within-family dependencies where they represent an actual
    # consumed mechanism; remove named false entry gates from the v4 graph only.
    for key, project in projects.items():
        first=by_project[key][0]; owner=project['meta']['module']
        for p in project['meta'].get('requires_projects',[]):
            if p not in by_project:continue
            if key=='PR-T55-01' and p.startswith('PR-T53-'):continue
            if key in {'PR-T48-01','PR-T48-02','PR-T48-03','PR-T48-04'} and p.startswith(('PR-T46-','PR-T47-')):continue
            if phase_for(projects[p]['meta'],rules)>first['phase']:continue
            dep=by_project[p][-1]['id']
            if dep not in first['requires']:
                first['requires'].append(dep);first['requires_reasons'][dep]='Explicit project contract consumes the prior '+projects[p]['meta']['title']+' capability.'
    # Early visible results precede deep catalog consolidation. Their true DAG
    # ancestors are pulled forward, never ignored or automatically accepted.
    early_projects=rules['early_projects']
    preferred=[by_project['PR-T01-01'][0]['id']]+[u['id'] for p in early_projects for u in by_project[p]]
    preferred+=[rules['boundary_unit']] if rules['boundary_unit'] in umap else [by_project[rules['boundary_fallback']][0]['id']]
    preferred += [by_project[rules['early_observation']][0]['id']]
    ranks={u:i for i,u in enumerate(preferred)}
    def promote(uid,rank,seen=None):
        seen=set() if seen is None else seen
        if uid in seen:return
        seen.add(uid);ranks[uid]=min(ranks.get(uid,99999),rank)
        for dep in umap[uid]['requires']:promote(dep,rank-0.01,seen)
    for uid in preferred:promote(uid,ranks[uid])
    order=[];done=set();active=None;last_strand={};pending=set(umap)
    for uid in rules.get('order_prefix',[]):
        # An accepted route prefix is replayed verbatim; its readiness is re-verified, never assumed.
        if uid not in pending:raise ValueError('Preserved route unit is missing: '+uid)
        u=umap[uid]
        if not set(u['requires'])<=done:raise ValueError('Preserved route prefix is not ready at '+uid)
        order.append(uid);done.add(uid);pending.remove(uid)
        active=None if u['milestone'] else u['project']
        if u['milestone']:last_strand[u['strand']]=len(order)
    while pending:
        ready=[umap[x] for x in pending if set(umap[x]['requires'])<=done]
        if not ready:
            raise ValueError('Unit dependency cycle: '+str({x:[d for d in umap[x]['requires'] if d not in done] for x in sorted(pending)[:20]}))
        ready.sort(key=lambda u:(ranks.get(u['id'],100000),u['phase'],0 if u['project']==active else 1,last_strand.get(u['strand'],-1),u['project'],u['id']))
        u=ready[0];order.append(u['id']);done.add(u['id']);pending.remove(u['id'])
        active=None if u['milestone'] else u['project']
        if u['milestone']:last_strand[u['strand']]=len(order)
    sets=[]
    topic_projects=defaultdict(list)
    for p,record in projects.items():
        for t in record['meta']['topics']:topic_projects[t].append(p)
    for tid,topic in topics.items():
        if tid.split('.')[0] in rules.get('authored_domains', []):
            row = rules.get('authored_recall', {}).get(tid)
            if not row or {v['kind'] for v in row.get('variants', [])} != {'mechanism', 'trace-or-small-implementation', 'changed-condition'}:
                raise ValueError('New topic needs three substantive authored recall variants: ' + tid)
            if set(row.get('objectives', [])) != {o['id'] for o in topic['objectives']}:
                raise ValueError('Authored recall loses objective mapping: ' + tid)
            sets.append(deepcopy(row))
            continue
        owning=topic_projects[tid]; body=projects[owning[0]]['body']; objectives=[o['id'] for o in topic['objectives']]
        objective_text={o['id']:o['text'] for o in topic['objectives']}
        operation=topic_passage(body,'Steps',tid) or topic['objectives'][1]['text']
        failure=section(body,'Failure cases').split('\n\n')[0]
        change=section(body,'Transfer challenge')
        variants=[
            {'id':tid+'-explain','kind':'mechanism','objectives':objectives,
             'prompt':'Without opening the old implementation, reconstruct '+topic['title']+'. '+topic['objectives'][0]['text']+' State the precondition, state transition and one counterexample.',
             'checklist':list(objective_text.values())},
            {'id':tid+'-trace','kind':'trace-or-small-implementation','objectives':objectives,
             'prompt':'For '+topic['title']+': '+operation+' Predict normal, boundary and failure outcomes before running the code. '+clean(failure),
             'checklist':[topic['objectives'][1]['text'],'Show the first divergent transition and justify the independent expectation.',topic['objectives'][2]['text']]},
            {'id':tid+'-transfer','kind':'changed-condition','objectives':objectives,
             'prompt':'For '+topic['title']+', reconstruct the relevant interface or invariant before consulting earlier source. '+clean(change or topic['objectives'][2]['text']),
             'checklist':[topic['objectives'][0]['text'],topic['objectives'][1]['text'],'Explain which assumptions survive and which evidence must be repeated.']},
        ]
        sets.append({'id':tid,'title':topic['title'],'objectives':objectives,'objective_text':objective_text,
            'projects':owning,'variants':variants,'resource_ids':[r['resource_id'] for r in modules[tid.split('.')[0]]['meta']['readings']],
            'elective':bool(topic.get('elective')),'grading':'Score only demonstrated objectives. Hints or consulted code remain assisted practice; no automatic whole-topic mastery.'})
    sets+=list(rules.get('vocabulary_sets',[]))
    categories=Counter()
    for m in modules.values():categories.update(m['meta']['workload'])
    integrations=read_json(ROOT/'curriculum/integration_families.json')['families']
    for f in integrations:categories.update(f['additional_workload'])
    categories['integration_implementation_verification']=sum(p['meta']['hours'] for p in projects.values() if p['meta']['module']=='INTEGRATION')
    budget={**rules['budget'],
        'project_implementation_verification_hours':sum(p['meta']['hours'] for p in projects.values()),'categories':dict(categories),
        'project_owners':{p:{'hours':r['meta']['hours'],'units':[u['id'] for u in by_project[p]],'minutes':sum(u['minutes'] for u in by_project[p]),'status':r['meta']['status']} for p,r in projects.items()},
        'review_replacements':[
            {'old_activity':'Module assessment: retrospective alternatives/dossier prose','replacement':'Sealed design comparison and milestone rubric','category':'assessment','hours':160},
            {'old_activity':'Module consolidation: manual delayed reconstruction','replacement':rules['consolidation_sets'],'category':'consolidation','hours':261.25},
            {'old_activity':'Integration assessment and consolidation','replacement':'Changed-condition handoff, deduplicated critical recall and transfer defense','category':'assessment_and_consolidation','hours':32}],
        'policy':'Replacements consume existing categories. The remaining assessment/consolidation funds explanations, longer variants and review. No full dossier per unit; remediation and equipment waiting are separate reported contingencies.'}
    dump(catalog/'units.json',{'schema_version':4,'units':units,'order':order,'phases':[{'id':i+1,'title':p[0],'gate':p[1]} for i,p in enumerate(rules['phases'])],
        'policy':'Unit/claim readiness is authoritative in v4; the preserved v3 project graph is an identity/coverage view.'})
    dump(catalog/'artifacts.json',{'schema_version':4,'artifacts':artifacts})
    dump(catalog/'recall.json',{'schema_version':4,'sets':sets,'policy':{'gaps_days':[1,3,7,14,30],'freshness_days':14,'session_minutes':15,'sets_per_session':2,
        'after_sequence':'maintenance at consumption and cumulative milestones','failure':'corrective feedback then next-day independent attempt','missed_sessions':'one overdue opportunity per topic','interval_claim':'Configurable engineering defaults; no scientifically optimal C schedule is asserted.'}})
    dump(catalog/'project_overlays.json',{'schema_version':4,'projects':overlays})
    dump(catalog/'budget.json',budget)
    dump(catalog/'migration.json',{'schema_version':4,'baseline_commit':baseline['commit'],'projects':{p:rules.get('migration_note','retained identity and budget; v4 unit overlay') for p in projects},
        'objectives':{o:'retained' for o in baseline['objectives']},'receipt_policy':'Read old schema-2 receipts without assigning new acceptance; source changes preserve history and reopen only supported affected claims.'})
    dump(catalog/'summary.json',{'domains':len(modules),'projects':len(projects),'objectives':sum(len(t['objectives']) for d in taxonomy for t in d['subtopics']),
        'units':len(units),'new_executable_units':sum(u['new_executable'] for u in units),'recall_sets':len(sets),
        'recall_variants':sum(len(s['variants']) for s in sets),'artifacts':len(artifacts),'families':len(list((ROOT/'labs').glob('*/family.json'))),
        'status':'authored; acceptance requires independent validation and execution receipts'})
    print(read_json(catalog/'summary.json'))


def main():
    author()


if __name__=='__main__':main()
