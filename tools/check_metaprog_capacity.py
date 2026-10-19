"""Guard exact capacity models, compiler diagnostics and actually executed32-bit image."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_capacity_validation.json'
COST='research/v5/modern/metaprog_capacity_diagnostics_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'capacity_fixed',2:'capacity_fixed',3:'capacity_runtime',4:'capacity_refusal',5:'capacity_refusal',6:'capacity_runtime',7:'capacity_base',8:'capacity_base'}
TRACE='b5bb297f1760137e379dd88496a2187df1def75f0a81ba93aac338f668a1f533'
def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['histories'],r['transitions'],r['arithmetic_cases'])==(9836,54615,11261) and r['trace_sha256']==TRACE
        assert r['analytic_models']=={'16':{'cases':3024,'admitted':1239},'32':{'cases':5376,'admitted':1626},'64':{'cases':11261,'admitted':2068}}
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'],r['size_t_bits'])==(100000,600000,64) and r['runtime_backing_size']==16 and min(r['fixed_size'],r['runtime_control_size'])>0
        assert all(len(r[k])==7 and all(t>=0 for t in r[k]) for k in ['fixed_seconds','runtime_seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/capacity/capacity.c','assessor/metaprog/capacity/capacity_template.inc'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-capacity-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==149 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    constants=[r for r in q['observations'] if r['kind']=='constant'];diags=[r for r in q['observations'] if r['kind']=='diagnostic'];objects=[r for r in q['observations'] if r['kind']=='object'];targets=[r for r in q['observations'] if r['kind']=='i386'];rust=[r for r in q['observations'] if r['kind']=='rust-const'];build=[r for r in q['observations'] if r['kind']=='incremental'];hosted=[r for r in q['observations'] if r['kind']=='hosted32-availability']
    assert len(constants)==10 and {(r['compiler'],r['capacity']) for r in constants}=={(c,n) for c in ['gcc','clang'] for n in [0,1,3,4,8]} and all(r['rejected']==(r['capacity'] in [0,3]) for r in constants)
    assert len(diags)==6 and {(r['compiler'],r['case']) for r in diags}=={(c,n) for c in ['gcc','clang'] for n in ['nonconstant-ICE','warning-versus-policy','VLA-supported-policy-refused']} and all(r['passed'] for r in diags)
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in objects:assert 'mp_storage_bound' in r['helpers'] and set(r['helpers'])<={'mp_storage_bound','memcpy','memset','__stack_chk_fail'} and r['text']>0 and r['masked_and_seen']
    assert len(targets)==3 and {r['mutation'] for r in targets}=={0,3,4} and all(r['executed'] and r['size_t_bits']==32 and r['no_undefined'] and r['no_needed'] and r['expected_refusal']==(r['mutation']!=0) for r in targets)
    assert len(rust)==1 and rust[0]['positive_executed'] and rust[0]['floating_type_rejected'] and len(build)==1 and build[0]['unchanged_no_work'] and len(hosted)==1 and hosted[0]['available']==(hosted[0]['exit_code']==0)
    assert len(q['executions'])==85 and sum(r['expected_exit']!=0 for r in q['executions'])==13
    for r in q['executions']:assert r['exit_code']==0 if r['expected_exit']==0 else r['exit_code']>0 and r['required_marker']
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in q['generated_artifacts']+hosted:assert sha(ROOT/r.get('path',r.get('log')))==r.get('sha256',r.get('log_sha256'))
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MPO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-002.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/capacity/CAPACITY.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['helpers'].append('hidden_ring'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(size_t_bits=32),lambda a,b:next(r for r in b['observations'] if r['kind']=='i386').update(executed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='i386').update(no_undefined=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-const').update(positive_executed=False),lambda a,b:next(r for r in b['executions'] if r['expected_exit']!=0).update(exit_code=0)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
