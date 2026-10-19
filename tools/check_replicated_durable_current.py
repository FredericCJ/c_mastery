"""Current source closure for durable-log author implementation, not acceptance."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,PROFILES,base,bindings,sha
REPORT='research/v5/modern/replicated_durable_current_validation.json'
ANCHORS={'replicated.durable_log','replicated.durable_snapshot','replicated.durable_bytes'}
CONTROL={1:'durable_log',6:'durable_log',2:'durable_snapshot',4:'durable_snapshot',3:'durable_bytes',5:'durable_bytes'}
def check(d,files=True):
    base(d);assert d['exhaustive_prefix_run'],'missing deep cut experiment'
    assert len(d['profiles'])==8 and {r['profile'] for r in d['profiles']}==PROFILES,'missing profile'
    wanted={f'replicated.mutation.{a}.{i}' for i,a in CONTROL.items()}
    for p in d['profiles']:assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==6 and {c['test'] for c in p['controls']}==wanted and all(c['semantic_rejection'] for c in p['controls']),'missing durable evidence'
    assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections']),'learner rejection absent'
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'},'dependency profile absent'
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual compiler dependency'
    oracles=[r for r in d['observations'] if r.get('oracle')=='verify_replicated_durable.py'];deep=[r for r in d['observations'] if r.get('kind')=='all-byte-and-flush-prefixes']
    assert len(oracles)==8 and {r['profile'] for r in oracles}==PROFILES and all(r['result']['passed'] for r in oracles),'oracle missing'
    assert len(deep)==1 and deep[0]['profile']=='gcc-release' and deep[0]['passed'] and 'TEARS 139502 139502' in deep[0]['stdout'],'byte/flush prefixes absent'
    rows=d['executions'];builds=[r for r in rows if '--build' in r['command']];ctests=[r for r in rows if '--output-on-failure' in r['command'] and '-R' not in r['command']]
    mutants=[r for r in rows if 'replicated-durable-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']]
    assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'build absent'
    assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'CTest absent'
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants),'wrong semantic control'
    assert len(learners)==3 and all(r['exit_code'] not in [None,0] for r in learners),'wrong learner rejection'
    faults=[r for r in rows if 'durable_faults' in r['command']];tears=[r for r in rows if 'durable_tears' in r['command']]
    assert len(faults)==8 and all(r['exit_code']==0 for r in faults) and len(tears)==1 and tears[0]['exit_code']==0,'fault experiments not run'
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows),'unexplained failed command'
    if files:
        bindings(d)
        for r in faults:assert 'FAULTS 417 417' in (ROOT/r['log']).read_text(),'fault count absent'
        assert 'TEARS 139502 139502' in (ROOT/tears[0]['log']).read_text(),'tear count absent'
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text();assert 'RP_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash not semantic rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x.update(exhaustive_prefix_run=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['learner_stub_rejections'].pop(),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(input_sha256={}),lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['observations'] if r.get('kind')).update(stdout='TEARS 1 1'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),lambda x:next(r for r in x['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11)]
    for change in changes:
        x=copy.deepcopy(d);change(x)
        try:check(x,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupt evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());check(d);n=selftest(d) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=8,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
