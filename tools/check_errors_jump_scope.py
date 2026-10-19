"""Reject stale, vacuous or overclaimed T65.06 evidence."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_jump_scope_validation.json'
COST='research/v5/modern/errors_jump_scope_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'jump_resources',2:'jump_effects',3:'jump_nested',4:'jump_nested',5:'jump_effects',6:'jump_nested',7:'jump_stack',8:'jump_stack',9:'jump_resources'}
ANCHORS=set(CONTROL.values())|{'jump_admission'}
TRACE='d5a1d63a3d4eb36d0013d21fa35668ce76142c989dcf09fcbe8a85c8acb72283'

def metric(r):
    assert (r['iterations'],r['checksum'],r['allocation_calls'],r['release_calls'])==(1000,91000,35000,35000)
    assert min(r['frame_size'],r['scope_size'],r['report_size'])>0
    assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])

def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==9
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['nested_cases'],r['trace_sha256'])==(2160,TRACE)
            else:metric(r)
    assert {'assessor/errors/errors.c','assessor/ownership/ownership.c','assessor/errors/jump_scope/jump_scope.c'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    learner=next(c for c in d['compiler_dependency_closures'] if c['profile']=='learner');assert 'assessor/ownership/ownership.c' in learner['repository_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-jump_scope-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==168 and len(mutants)==54 and all(r['exit_code']==1 for r in mutants) and len(learners)==5 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==11 and len(q['executions'])==33 and len(q['generated_artifacts'])==2 and all(r['exit_code']==0 and not r['expected_failure'] for r in q['executions'])
    for cc in ['gcc','clang']:
        imports=[r for r in q['observations'] if r['kind']=='actual-imports' and r['compiler']==cc];assert len(imports)==1 and imports[0]['no_explicit_heap_imports'] and {'er_run','er_raise','er_defer','ow_defer','ow_finish'}<=set(imports[0]['symbols'])
        costs=[r for r in q['observations'] if r['kind']=='metrics' and r['compiler']==cc];assert len(costs)==1;metric(costs[0]['result'])
        cases=[r for r in q['observations'] if r['kind']=='defined-only' and r['compiler']==cc];assert len(cases)==3 and {r['profile'] for r in cases}=={'debug','release','asan'}
        for r in cases:assert r['switch_comparison_not'] and r['volatile_changed']==7 and r['external_effects']==[11,19,23] and r['zero_becomes_one'] and r['undefined_cases_executed'] is False
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['EJD_STACK actual-er-boundary','EJD_RESOURCES actual-T66-scope','EJD_NESTED separate-live-frames','EJD_EFFECTS published-counter-not-rollback','EJD_ADMISSION malformed-plan','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-006.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and {'labs/errors/jump_scope/JUMP.md','labs/errors/jump_scope/SEMANTICS.md'}<=set(m['shared_contracts'])
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['learner_stub_rejections'][0].update(semantic_rejection=False),lambda a,b:a['observations'][0]['result'].update(nested_cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(release_calls=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='actual-imports').update(symbols=[]),lambda a,b:next(r for r in b['observations'] if r['kind']=='defined-only').update(undefined_cases_executed=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='defined-only').update(volatile_changed=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][-1].update(exit_code=1)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
