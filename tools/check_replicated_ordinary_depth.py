"""Observe retained original mechanisms separately from modern successors."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,PROFILES,base,bindings,sha
REPORT='research/v5/modern/replicated_ordinary_depth_validation.json'
CASES=['log','projection','clocks','counters','sets','sequence','merkle','wire','replay']
ANCHORS={'ordinary_'+x for x in CASES+['laws']}
CONTROL={i:'ordinary_'+CASES[(i-1)//2] for i in range(1,19)}|{19:'ordinary_laws',20:'ordinary_laws'}
def check(d,files=True):
    base(d)
    assert len(d['profiles'])==8 and {p['profile'] for p in d['profiles']}==PROFILES,'missing/duplicate profiles'
    wanted={f'replicated.mutation.{v}.{i}' for i,v in CONTROL.items()}
    for p in d['profiles']:assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==20 and {r['test'] for r in p['controls']}==wanted and all(r['semantic_rejection'] for r in p['controls']),'anchor/control missing'
    assert len(d['learner_stub_rejections'])==10 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections']),'learner rejection missing'
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==8 and {r['profile'] for r in oracles}==PROFILES,'oracle profile missing'
    for r in oracles:
        q=r['result'];assert q['passed'] and q['fixtures']==9 and q['baseline_comparisons']==16435 and q['register_comparisons']==6108,'wrong oracle scope'
        assert q['baseline_sha256']=='ac63093e02f0ef87cfad1c6cc48a49effd20548036f5754e02862996ee06553a' and q['register_sha256']=='557183042051e24ecf8d19e9b453cb88cac1c564310cab6c48241f36f2bb4c00','wrong oracle result'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'metrics missing'
    for r in metrics:
        q=r['result'];assert q['pn_size']>0 and q['register_size']>0 and q['runs']==100000 and len(q['cpu_seconds'])==7 and all(t>=0 for t in q['cpu_seconds']),'wrong measured workload'
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'},'closure missing'
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'actual input unpinned'
    rows=d['executions'];builds=[r for r in rows if '--build' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']];mutants=[r for r in rows if 'replicated-ordinary-depth-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']]
    assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'unobserved build'
    assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'unobserved CTest'
    assert len(mutants)==160 and all(r['exit_code']==1 for r in mutants),'mutant failure wrong'
    assert len(learners)==10 and all(r['exit_code'] not in [0,None] for r in learners),'learner failure wrong'
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows),'unexplained failure'
    if files:
        bindings(d)
        for r in ctests:
            s=(ROOT/r['log']).read_text();assert all('ORDINARY_BASE '+c+' passed' in s for c in CASES) and 'ORDINARY_LAWS pn-triples262144 permutations6 register-triples1000' in s and '100% tests passed' in s,'unobserved original/law case'
        for r in mutants+learners:
            s=(ROOT/r['log']).read_text();assert 'RP_CHECK' in s and not any(x in s for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash not semantic rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda x:x['learner_stub_rejections'].pop(),lambda x:x['observations'][0]['result'].update(register_comparisons=1),lambda x:x.update(input_sha256={}),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),lambda x:next(r for r in x['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda x:next(r for r in x['executions'] if '-R' in r['command']).update(exit_code=0)]
    for change in changes:
        x=copy.deepcopy(d);change(x)
        try:check(x,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupt evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());check(d);n=selftest(d) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=8,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
