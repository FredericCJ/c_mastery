"""Current effect order, resource cleanup, generated-policy and equal-work guards."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_policy_effects_validation.json'
COST='research/v5/modern/metaprog_policy_cost_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'policy_failure',2:'policy_failure',3:'policy_saturation',4:'policy_order',5:'policy_saturation',6:'policy_order',7:'policy_base',8:'policy_base'}
TRACE='035e24a34a3f9b00fc73336358157b0e31f0f1e9d94789bf9e741c64dca85a47'
def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['cases'],r['callbacks'],r['acquisition_failures'],r['projection_failures'],r['range_refusals'])==(18968,93114,9402,4718,3546) and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['reject_checksum'],r['saturate_checksum'],r['callback_calls_each'])==(10000,100000,50000,40000) and min(r['item_size'],r['ops_size'])>0
        assert all(len(r[k])==7 and all(t>=0 for t in r[k]) for k in ['reject_dynamic_seconds','reject_specialized_seconds','saturate_dynamic_seconds','saturate_specialized_seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/policy_effects/policy_effects.c','assessor/metaprog/policy_effects/policy_template.inc'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-policy_effects-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==149 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    pre=[r for r in q['observations'] if r['kind']=='preprocess'];objects=[r for r in q['observations'] if r['kind']=='object'];rust=[r for r in q['observations'] if r['kind']=='rust-const-policy'];build=[r for r in q['observations'] if r['kind']=='incremental']
    assert len(pre)==2 and {r['compiler'] for r in pre}=={'gcc','clang'} and all(r['definitions']==['mpp_dynamic','mpp_reject','mpp_saturate'] and min(r['bytes'],r['tokens'])>0 for r in pre)
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in objects:assert set(r['helpers'])<={'memcpy','memset','__stack_chk_fail'} and set(r['functions'])=={'mpp_dynamic','mpp_reject','mpp_saturate'} and all(n>0 for n in r['functions'].values())
    assert len(rust)==1 and rust[0]['executed'] and len(build)==1 and build[0]['unchanged_no_work'] and min(build[0]['clean_seconds'],build[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==36 and all(r['exit_code']==0 for r in q['executions'])
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in q['generated_artifacts']:assert sha(ROOT/r['path'])==r['sha256']
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MPO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-003.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/policy_effects/POLICY.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(callbacks=0),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['helpers'].append('hidden_fold'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(callback_calls_each=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-const-policy').update(executed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='preprocess')['definitions'].pop()]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
