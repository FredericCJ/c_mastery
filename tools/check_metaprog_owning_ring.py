"""Require current exact ownership, mutation, language and cost evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_owning_ring_validation.json'
COST='research/v5/modern/metaprog_owning_cost_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'owning_failure',2:'owning_failure',3:'owning_transfer',4:'owning_erased',5:'owning_transfer',6:'owning_erased',7:'mono_base',8:'mono_base'}
TRACE='a44bd9351cdb41e8278400a66265dfbdee9aaa81472d9c51c410c470833e2323'
IMPORTS={'assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc','assessor/metaprog/owning_ring/owning_ring.c','assessor/metaprog/owning_ring/ring_template.inc','assessor/metaprog/owning_ring/manual.c','assessor/metaprog/owning_ring/erased.c'}
def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values())
        assert len(r['controls'])==8 and {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['histories'],r['implementations'],r['transitions'])==(9335,3,134682) and r['trace_sha256']==TRACE
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'])==(10000,100000) and r['erased_storage_size']==5*r['element_size'] and min(r['typed_size'],r['erased_control_size'],r['element_alignment'])>0
        assert all(len(r[k])==7 and all(t>=0 for t in r[k]) for k in ['typed_seconds','manual_seconds','erased_seconds'])
    assert IMPORTS<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-owning-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==173 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    objects=[r for r in q['observations'] if r['kind']=='object'];diagnostics=[r for r in q['observations'] if r['kind']=='diagnostic'];links=[r for r in q['observations'] if r['kind']=='link'];pre=[r for r in q['observations'] if r['kind']=='preprocess'];rust=[r for r in q['observations'] if r['kind']=='rust-associated'];build=[r for r in q['observations'] if r['kind']=='incremental']
    assert len(objects)==16 and {(r['compiler'],r['optimization'],r['instance']) for r in objects}=={(c,o,i) for c in ['gcc','clang'] for o in ['0','3'] for i in ['one-generated','two-generated','manual','erased']}
    for r in objects:assert len(set(r['exports']))==(8 if r['instance']=='two-generated' else 4) and r['text']>0 and set(r['helpers'])<={'memset','memcpy','__stack_chk_fail'}
    assert len(diagnostics)==8 and {(r['compiler'],r['case']) for r in diagnostics}=={(c,k) for c in ['gcc','clang'] for k in ['mismatched-type','private-name','guard-suppression','duplicate-definition']} and all(r['passed'] for r in diagnostics)
    assert len(links)==2 and {r['compiler'] for r in links}=={'gcc','clang'} and all(r['passed'] for r in links)
    assert len(pre)==2 and {r['compiler'] for r in pre}=={'gcc','clang'} and all(len(set(r['definitions']))==8 and min(r['bytes'],r['tokens'])>0 for r in pre)
    assert len(rust)==1 and rust[0]['positive_executed'] and rust[0]['negative']=='E0053'
    assert len(build)==1 and build[0]['unchanged_no_work'] and min(build[0]['clean_seconds'],build[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==94 and sum(r['expected_exit']!=0 for r in q['executions'])==9
    for r in q['executions']:assert r['exit_code']==0 if r['expected_exit']==0 else r['exit_code']>0 and r['required_marker']
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale cost source '+p
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in q['generated_artifacts']:assert sha(ROOT/r['path'])==r['sha256']
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MPO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(x in text for x in ['MPO_BASE','MPO_FAILURE','MPO_TRANSFER','MPO_ERASED','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-001.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/owning_ring/OWNING.md' in m['shared_contracts']
    return True
def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['helpers'].append('hidden_ring_library'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(erased_storage_size=0),lambda a,b:a['input_sha256'].pop('assessor/metaprog/ring_template.inc'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-associated').update(positive_executed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='preprocess')['definitions'].pop(),lambda a,b:next(r for r in b['executions'] if r['expected_exit']!=0).update(exit_code=0)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if args.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
