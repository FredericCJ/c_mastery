"""Bind initialized-set support to actual provider executions and exact traces."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
from run_ownership_frontier_investigation import SUCCESS,PARTIAL
REPORT='research/v5/modern/ownership_frontier_validation.json'
COST='research/v5/modern/ownership_frontier_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'frontier_partial',2:'frontier_dag',3:'frontier_partial',4:'frontier_admission',5:'frontier_partial',6:'frontier_dag',7:'frontier_base',8:'frontier_base',9:'frontier_callbacks'}
ANCHORS=set(CONTROL.values())
TRACE='05a85c57b6ecaf400b8418ba02a5debd000e4094d13010b842c2720bdc36c3aa'
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==9
        assert {r['test'] for r in p['controls']}=={f'ownership.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['cases'],r['topological_orders'],r['permutations'],r['allocation_failure_positions'],r['initialization_failure_nodes'],r['size_bits'],r['trace_sha256'])==(902,3,24,4,4,64,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['takes'],r['releases'])==(1000,105000,28000,28000) and r['owner_size']>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/ownership/ownership.c','assessor/ownership/frontier/frontier.c','labs/ownership/include/imports/v4_algo.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-frontier-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==156 and len(mutants)==54 and all(r['exit_code']==1 for r in mutants) and len(learners)==5 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==7 and len(q['executions'])==29 and len(q['generated_artifacts'])==2 and all(r['exit_code']==0 for r in q['executions'])
    assert any('/rust-1.93/lib/rustlib/' in p and p.endswith('.rlib') for p in q['external_input_sha256'])
    for cc in ['gcc','clang']:
        core=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc];assert len(core)==1;r=core[0];assert not r['explicit_heap_imports'] and r['callback_allocation_allowed'] and not r['arbitrary_helper_absence_claim'] and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        cases=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(cases)==1 and all(cases[0][k] for k in ['actual_typed_allocator','actual_prefix_constructor','partial_storage_release','uninitialized_value_not_destroyed','executed'])
    rust=[r for r in q['observations'] if r['kind']=='rust-drop'];assert len(rust)==2 and {r['optimization'] for r in rust}=={'0','3'}
    for r in rust:assert r['success_trace']==SUCCESS and r['partial_trace']==PARTIAL and r['executed'] and r['partial_storage_without_value'] and not r['arbitrary_dependency_enforcement_claim']
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['OCF_BASE actual-construction','OCF_DAG three-topological-orders','OCF_PARTIAL C-storage-acquired-but-uninitialized','OCF_ADMISSION all-lengths-and-graph','OCF_CALLBACKS clear-initialized-before-destroy','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-001.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/frontier/FRONTIER.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(topological_orders=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(releases=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(arbitrary_helper_absence_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(actual_typed_allocator=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(uninitialized_value_not_destroyed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-drop').update(arbitrary_dependency_enforcement_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-drop').update(partial_trace='wrong')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
