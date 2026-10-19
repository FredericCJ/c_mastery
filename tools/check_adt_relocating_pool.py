"""Eight-profile pool lifetime/identity and actual category-diagnostic guard."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_relocating_pool_validation.json'
DIAGNOSTICS='research/v5/modern/adt_pool_diagnostics_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan','tsan']}
CONTROL={1:'pool_identity',2:'pool_identity',3:'pool_retirement',4:'pool_retirement',5:'pool_relocation',6:'pool_relocation',7:'pool_native',8:'pool_native',9:'pool_base',10:'pool_base'}
def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==8 and {p['profile'] for p in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==set(CONTROL.values())
        assert len(p['controls'])==10 and {r['test'] for r in p['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==8 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and (r['successor_histories'],r['successor_transitions'],r['baseline_histories'],r['baseline_transitions'])==(3968,30848,2657,19844)
        assert r['trace_sha256']==['3b1888f3a19cc97ab59c327944821f3fb61ae8651ec79e05f90ae86b45847d97','d7d4ae7831be94e2518c4c188d78027675b81327304168b81f7a50c14c6920a4']
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert r['pool_size']>0 and r['handle_size']>0 and r['snapshot_size']>r['handle_size'] and r['iterations']==100000 and r['checksum']==12300000 and len(r['raw_seconds'])==len(r['safe_seconds'])==7 and all(x>=0 for x in r['raw_seconds']+r['safe_seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert {'assessor/adt/adt.c','assessor/adt/relocating_pool/relocating_pool.c','labs/adt/relocating_pool/relocating_pool.h'}<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-pool-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==80 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==5 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    probes=[r for r in q['observations'] if r['case']!='incremental'];cost=[r for r in q['observations'] if r['case']=='incremental']
    assert len(probes)==8 and {(r['compiler'],r['case']) for r in probes}=={(c,k) for c in ['gcc','clang'] for k in ['copy','wrong','rebuild']}|{('rustc-1.93',k) for k in ['marker','wrong']}
    assert all(r['compile_rejected']==(r['case']=='wrong') and r['executed']==(r['case']!='wrong') for r in probes)
    assert len(cost)==1 and cost[0]['unchanged_no_work'] and min(cost[0]['clean_seconds'],cost[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==19 and sum(r['exit_code']!=0 for r in q['executions'])==3
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale diagnostic source'
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8');assert (r['exit_code']==0 if r['expected_exit']==0 else r['exit_code']!=0)
            if r['required_marker']:assert r['required_marker'] in text
        for row in mutants+learners:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for row in ctests:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert all(s in text for s in ['ADP_ID nominal-runtime-generation','ADP_RETIRE finite-no-wrap','ADP_MOVE fresh-mutex-transferred-block joined-restarted-readers=6000','ADP_NATIVE copied-read-close-join atomic-witness=999 reads=30000','ADP_BASE actual-fixed-address-pool','100% tests passed'])
        for profile in ['gcc-tsan','clang-tsan']:
            artifacts=[r for r in d['generated_artifacts'] if r['path'].endswith(profile+'-compile_commands.json')];assert len(artifacts)==1
            commands=json.loads((ROOT/artifacts[0]['path']).read_text());assert commands and all('-fsanitize=thread' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T67-005.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'adt.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/adt/relocating_pool/POOL.md' in m['shared_contracts']
    return True
def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(successor_transitions=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][1].update(compile_rejected=False),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:a['observations'][0]['result'].update(baseline_histories=0),lambda a,b:a['input_sha256'].pop('assessor/adt/adt.c'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:next(r for r in b['observations'] if r['case']=='incremental').update(unchanged_no_work=False)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/DIAGNOSTICS).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=8,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
