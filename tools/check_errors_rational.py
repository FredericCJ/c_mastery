"""Require exact rational grids, signed transfer, diagnostics and semantic controls."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_rational_validation.json'
COST='research/v5/modern/errors_rational_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'rational_consumer',2:'rational_add',3:'rational_add',4:'rational_add',5:'rational_signed',6:'rational_signed',7:'rational_base',8:'rational_base'}
ANCHORS=set(CONTROL.values())|{'rational_opaque','rational_admission'}
TRACE='98f0f6d5cfc39beb7081a3a7ee9a61bd40669715472de916b28ac0c0fd1bb8ee'
COUNTS=dict(unsigned_normalizations=65585,unsigned_consumers=1024,unsigned_additions=5204,signed_normalizations=65578,signed_additions=8651,signed_consumers=70,fibonacci_cases=47,euclid_last_steps=45)

def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==8
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and all(r[k]==v for k,v in COUNTS.items()) and r['trace_sha256']==TRACE
            else:
                assert (r['iterations'],r['checksum'],r['core_allocation_calls'])==(10000,210000,0) and min(r['unsigned_size'],r['signed_size'])>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/errors/errors.c','assessor/errors/rational/rational.c','assessor/errors/rational/opaque.c'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-rational-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==163 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==9 and len(q['executions'])==31 and len(q['generated_artifacts'])==4
    rejected=[r for r in q['executions'] if r['expected_compile_rejection']];assert len(rejected)==2 and all(r['exit_code'] not in [0,None] and r['required_marker']=='incomplete' for r in rejected)
    assert all(r['exit_code']==0 or r in rejected for r in q['executions'])
    for cc in ['gcc','clang']:
        core=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc];assert len(core)==2 and {r['source'] for r in core}=={'rational','opaque'}
        for r in core:assert 'er_refine' in r['symbols'] and not r['explicit_heap_imports'] and not r['arbitrary_helper_absence_claim'] and r['callback_allocation_allowed']==(r['source']=='opaque') and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        cases=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(cases)==1 and all(cases[0][k] for k in ['signed_minimum','opaque_allocation_failure','empty_before_release','executed'])
        neg=[r for r in q['observations'] if r['kind']=='opaque-translation' and r['compiler']==cc];assert len(neg)==1 and all(neg[0][k] for k in ['negative_rejected','positive_executed','privacy_is_not_memory_safety']) and neg[0]['language_mode']=='c17'
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['EFR_BASE actual-refine','EFR_CONSUMER fabricated-denominator-rejected','EFR_ADD naive-versus-gcd-cancel','EFR_SIGNED minimum-magnitude-no-abs','EFR_OPAQUE real-private-type','EFR_ADMISSION structural-invalid-before-arithmetic','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-009.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/rational/RATIONAL.md' in m['shared_contracts']
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(signed_additions=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(arbitrary_helper_absence_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(signed_minimum=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(empty_before_release=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='opaque-translation').update(privacy_is_not_memory_safety=False),lambda a,b:next(r for r in b['executions'] if r['expected_compile_rejection']).update(exit_code=0)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
