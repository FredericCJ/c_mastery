"""Bind exact ordinary/nested predicate grids and publication controls."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_contract_modes_validation.json'
COST='research/v5/modern/errors_contract_modes_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'modes_bounds',2:'modes_exhaustion',3:'modes_bounds',4:'modes_bounds',5:'modes_concurrent',6:'modes_base',7:'modes_base'}
ANCHORS=set(CONTROL.values())|{'modes_admission'}
TRACE='16a1b8bc022db9a0b2ff7ef432601a253324ee7c0540d667286dabc7d5efebf7'
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==7
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['ordinary_cases'],r['counter_cases'],r['buffer_cases'],r['size_t_bits'],r['trace_sha256'])==(182709,182869,1188,64,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['allocation_calls'])==(10000,560000,0) and min(r['buffer_size'],r['shared_size'])>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/errors/errors.c','assessor/errors/contract_modes/contract_modes.c'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-contract_modes-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==144 and len(mutants)==42 and all(r['exit_code']==1 for r in mutants) and len(learners)==5 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==13 and len(q['executions'])==49 and len(q['generated_artifacts'])==2
    runtime=[r for r in q['executions'] if r['known_runtime_failure']];assert len(runtime)==4 and all(r['exit_code'] not in [0,None] for r in runtime)
    assert all(r['exit_code']==0 or r['known_runtime_failure'] for r in q['executions'])
    for cc in ['gcc','clang']:
        core=[r for r in q['observations'] if r['kind']=='actual-imports' and r['compiler']==cc];assert len(core)==1;r=core[0];assert r['no_explicit_heap_imports'] and {'er_contract_counter','mtx_lock','mtx_unlock','mtx_init','mtx_destroy'}<=set(r['symbols']) and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        cases=[r for r in q['observations'] if r['kind']=='assert-evaluation' and r['compiler']==cc];assert len(cases)==2 and {r['mode'] for r in cases}=={'enabled','disabled'}
        for r in cases:assert r['executed'] and r['essential']==(1 if r['mode']=='enabled' else 0)
        limits=[r for r in q['observations'] if r['kind']=='c11-tsan-boundary' and r['compiler']==cc];assert len(limits)==1;r=limits[0];assert r['native_control_passed'] and r['minimal_control_runtime_failure'] and r['component_runtime_failure'] and not r['clean_tsan_execution']
        posix=[r for r in q['observations'] if r['kind']=='posix-concurrent' and r['compiler']==cc];assert len(posix)==2 and {r['profile'] for r in posix}=={'release','tsan'}
        for r in posix:assert r['executed'] and r['reports']==8000 and r['joined']
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in runtime:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['ThreadSanitizer','DEADLYSIGNAL','0x000000000018'])
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['ECM_BASE actual-counter-modes','ECM_BOUNDS every-mode-gates-memory','ECM_EXHAUSTION report-limit-preserves','ECM_ADMISSION empty-span-null','ECM_CONCURRENT four-threads','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-008.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/contract_modes/MODES.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(buffer_cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='actual-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='actual-imports').update(no_explicit_heap_imports=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='c11-tsan-boundary').update(clean_tsan_execution=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='posix-concurrent').update(reports=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='assert-evaluation').update(essential=7),lambda a,b:next(r for r in b['observations'] if r['kind']=='c11-tsan-boundary').update(minimal_control_runtime_failure=False)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
