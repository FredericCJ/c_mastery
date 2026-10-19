"""Guard owner/trace laws, controlled refusals and finite reduction evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_law_traces_validation.json'
COST='research/v5/modern/metaprog_law_investigation_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'law_owner',2:'law_canonical',3:'law_canonical',4:'law_canonical',5:'law_shrink',6:'law_shrink',7:'law_base',8:'law_base'}
TRACE='04a918e9d3f12141155fb379bf1865683a70eda982edd56a8dbf2914f2d72dc3'
def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['cases'],r['hash_cases'],r['replay_cases'],r['shrink_cases'],r['admitted_replays'],r['failure_witnesses'])==(24238,1614,22400,224,3948,240) and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'])==(1000,21000) and min(r['owner_size'],r['trace_size'])>0 and len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/law_traces/law_traces.c','assessor/metaprog/law_traces/fault_adapter.c'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-law_traces-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==161 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    obs=q['observations'];assert len(obs)==6
    refusals=[r for r in obs if r['kind']=='unexpected-allocation-refusal'];assert len(refusals)==4 and {(r['compiler'],r['optimization']) for r in refusals}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    assert all(r['executed'] and r['first_replay_unchanged'] and r['shrink_candidate_unchanged'] and r['final_live']==0 and r['positive_reduction']==3 for r in refusals)
    language=[r for r in obs if r['kind']=='language-comparison'];assert len(language)==1 and language[0]['language']=='Rust2021' and language[0]['executed'] and language[0]['derived_cache_is_semantic'] and language[0]['custom_canonical_law'] and not language[0]['stable_hash_claim']
    incremental=[r for r in obs if r['kind']=='incremental'];assert len(incremental)==1 and incremental[0]['unchanged_no_work'] and min(incremental[0]['clean_seconds'],incremental[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==18 and all(r['exit_code']==0 for r in q['executions']) and len(q['generated_artifacts'])==1
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
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-007.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/law_traces/LAWS.md' in m['shared_contracts'] and 'testing.faults' in m['prerequisite_artifacts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(admitted_replays=0),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='unexpected-allocation-refusal').update(shrink_candidate_unchanged=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='unexpected-allocation-refusal').update(final_live=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='language-comparison').update(stable_hash_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='language-comparison').update(executed=False),lambda a,b:b['executions'][5].update(exit_code=1)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
