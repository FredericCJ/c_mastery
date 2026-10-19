"""Guard exact partial-domain laws, version round trips and observable callback effects."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_partial_optics_validation.json'
COST='research/v5/modern/adt_optics_cost_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'optics_nested',2:'optics_nested',3:'optics_prism',4:'optics_prism',5:'optics_effects',6:'optics_effects',7:'optics_base',8:'optics_base'}
MARKERS=['ADL_NESTED partial-composition three-laws collateral-preserved','ADL_PRISM version-and-payload roundtrip partial-domain','ADL_EFFECT once-in-order external-effects-not-rollback','ADL_BASE actual-lens-prism laws']
HASHES=['478ec04ddf6f6aec5343b9fc95437e85161ce59931eb88b65d811e5ba72dbb54','aeae4765c92345c2f92309d9be55d6008ddbb4c02426b550593ae6946c62e562','b8b380e5736f326d2b1ff80d70f0f9ddb0eea141482a64768fe07c3d9359e327']

def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values())
        assert len(r['controls'])==8 and {x['test'] for x in r['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==6 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and (r['nested_cases'],r['prism_cases'],r['effect_cases'],r['law_cases'])==(10496,5120,24832,256) and r['trace_sha256']==HASHES
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'],r['callback_calls'])==(100000,3100000,100000)
        assert min(r['config_size'],r['config_alignment'],r['error_size'])>0
        assert all(len(r[k])==7 and all(x>=0 for x in r[k]) for k in ['direct_seconds','composed_seconds','callback_seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert {'assessor/adt/adt.c','assessor/adt/partial_optics/partial_optics.c'}<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-optics-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    objects=[r for r in q['observations'] if r['kind']=='object'];strict=[r for r in q['observations'] if r['kind']=='strict'];cost=[r for r in q['observations'] if r['kind']=='incremental']
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in objects:assert 'ad_lens_x' in r['symbols'] and set(r['symbols'])<=set(r['allowed']) and r['allowed']==['ad_lens_x','memcpy','memset','__stack_chk_fail']
    assert len(strict)==2 and {r['compiler'] for r in strict}=={'gcc','clang'} and all(r['passed'] for r in strict)
    assert len(cost)==1 and cost[0]['unchanged_no_work'] and min(cost[0]['clean_seconds'],cost[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==18 and all(r['exit_code']==0 for r in q['executions'])
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale cost source'
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in MARKERS+['100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T67-008.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'adt.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/adt/partial_optics/OPTICS.md' in m['shared_contracts']
    return True

def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(effect_cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][1]['symbols'].append('unpermitted_solver'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(callback_calls=0),lambda a,b:a['input_sha256'].pop('assessor/adt/adt.c'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:a['observations'][0]['result'].update(law_cases=0),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
