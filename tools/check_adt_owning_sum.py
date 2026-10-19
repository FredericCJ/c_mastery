"""Exact six-profile sum ownership and diagnostic support; no learner acceptance."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_owning_sum_validation.json'
DIAGNOSTICS='research/v5/modern/adt_sum_diagnostics_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'sum_owner',2:'sum_rollback',3:'sum_value',4:'sum_rollback',5:'sum_owner',6:'sum_value',7:'sum_base',8:'sum_base'}

def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==set(CONTROL.values())
        assert len(p['controls'])==8 and {c['test'] for c in p['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(c['semantic_rejection'] for c in p['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==6 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and r['histories']==10512 and r['transitions']==56384 and r['trace_sha256']=='57e1d1f344d94af61e77ff2704ed93c6b935f8639a4a4dd98da83be118792184'
    assert len(metrics)==6
    for row in metrics:
        r=row['result'];assert r['value_size']>r['tag_size']>0 and r['iterations']==10000 and len(r['samples'])==7 and all(v>=0 for v in r['samples'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    executions=d['executions'];mutants=[r for r in executions if 'adt-owning-sum-mutant-' in r['command'][0]];learners=[r for r in executions if '-R' in r['command']]
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners)
    permitted={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in permitted for r in executions)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and len(q['observations'])==14 and len(q['executions'])==28
    expected={(c,s) for c in ['gcc','clang'] for s in ['old-complete','new-missing','new-default','default-enum-warning','new-complete']}|{('rustc-1.93',s) for s in ['old-complete','new-missing','new-default','new-complete']}
    assert {(r['compiler'],r['case']) for r in q['observations']}==expected
    assert sum(r['compile_rejected'] for r in q['observations'])==5
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale diagnostics source'
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h,'changed diagnostics compiler'
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            assert (r['exit_code']==0 if r['expected_exit']==0 else r['exit_code']!=0)
            if r['required_marker']:assert r['required_marker'] in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(s in text for s in ['AddressSanitizer','runtime error:'])
    return True

def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(transitions=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][1].update(compile_rejected=False),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed')]
    for mutate in changes:
        a,b=copy.deepcopy((d,q));mutate(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/DIAGNOSTICS).read_text());check(d,q);n=selftest(d,q) if args.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
