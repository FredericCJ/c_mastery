"""Guard full-input cache equality, publication cuts and actual Ninja outcomes."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_build_cache_validation.json'
COST='research/v5/modern/metaprog_cache_investigation_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'cache_inputs',2:'cache_inputs',3:'cache_schedule',4:'cache_schedule',5:'cache_publication',6:'cache_schedule',7:'cache_base',8:'cache_base',9:'cache_files',10:'cache_files'}
ANCHORS=set(CONTROL.values())|{'cache_parallel'}
TRACE='abc0ee6f86bf71ac39a3e99f676721a36c3985f142a0df0b8329dba2eb857e3a'

def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==10
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and r['cases']==10145 and r['outcomes']=={'0':2039,'1':941,'2':7165} and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'])==(10000,210000) and min(r['plan_size'],r['entry_size'])>0 and len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/build_cache/build_cache.c','assessor/metaprog/build_cache/cache_file.c','assessor/api_depth/architecture/core.c'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-build_cache-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==175 and len(mutants)==60 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    obs=q['observations'];assert len(obs)==8
    for kind in ['missing-implicit','order-only','command-change','generated-consumer']:
        selected=[r for r in obs if r['kind']==kind];assert len(selected)==2 and {r['compiler'] for r in selected}=={'gcc','clang'}
        for r in selected:
            if kind in ['missing-implicit','order-only']:assert r['stale']!=r['expected'] and r['repaired']==r['expected']
            elif kind=='command-change':assert r['before']!=r['after']==r['expected']
            else:assert r['compiled_executed'] and r['byte_reproducible'] and r['actual_codec'] and r['unchanged_no_work'] and min(r['clean_seconds'],r['unchanged_seconds'])>=0
    assert len(q['executions'])==41 and all(r['exit_code']==0 for r in q['executions']) and len(q['generated_artifacts'])==12 and len({r['path'] for r in q['generated_artifacts']})==12
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
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MPB_FILES process-cuts17 refusal-boundaries17 old-or-new prefix-corruption-refused actual-codec-entry no-power-cut-claim' in text and 'MPB_PARALLEL actual-two-workers exclusive-outputs dependency-wake failure-drain joined-publication' in text
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-010.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/metaprog/build_cache/CACHE.md' in m['shared_contracts'] and {'codec.records','testing.faults'}<=set(m['prerequisite_artifacts'])
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(outcomes={}),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='missing-implicit').update(repaired=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='order-only').update(stale=next(x for x in b['observations'] if x['kind']=='order-only')['expected']),lambda a,b:next(r for r in b['observations'] if r['kind']=='generated-consumer').update(compiled_executed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='command-change').update(after=0),lambda a,b:b['generated_artifacts'].pop(),lambda a,b:a['profiles'][0]['anchors'].remove('cache_parallel')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
