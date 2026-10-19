"""Guard exact whole-arena, fixed-point and bounded search author evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_arena_flow_validation.json'
COST='research/v5/modern/adt_arena_cost_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'arena_certificate',2:'arena_certificate',3:'arena_worklist',4:'arena_worklist',5:'arena_outcomes',6:'arena_outcomes',7:'arena_base',8:'arena_base'}
MARKERS=['ADF_ARENA whole-owned-certificate','ADF_FLOW cyclic-union-reschedule owned-reverse-ring','ADF_OUTCOME budget-not-no-fixed-point oscillation-with-solutions','ADF_BASE actual-whole-arena']
HASHES=['c8908ae832b8c08d39948c182d5a053b256440476d547280523a330dd3895732','18d99431d0db38416f7c9af419209060759c45e405f1609e911969437de78d39','8f982969a13895bcd03951e92b4fc5ae309aadc609a538c330bac804a48d5e2d']

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
        r=row['result'];assert r['passed'] and (r['arena_cases'],r['flow_cases'],r['boolean_cases'],r['max_observed_evaluations'])==(9117,37123,22066,64) and r['trace_sha256']==HASHES and r['boolean_outcomes']=={'0':8625,'1':2,'2':12126,'3':1313}
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['nodes'],r['tree_visits'],r['checksum'],r['flow_iterations'],r['flow_checksum'],r['flow_evaluations'])==(10000,16,65535,327680000,1000,2040000,37)
        assert min(r['arena_size'],r['arena_alignment'],r['session_size'])>0
        assert all(len(r[k])==7 and all(x>=0 for x in r[k]) for k in ['base_seconds','cached_seconds','flow_seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert {'assessor/adt/adt.c','assessor/adt/arena_flow/arena_flow.c'}<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-flow-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    objects=[r for r in q['observations'] if r['kind']=='object'];strict=[r for r in q['observations'] if r['kind']=='strict'];cost=[r for r in q['observations'] if r['kind']=='incremental']
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in objects:assert 'ad_eval' in r['symbols'] and set(r['symbols'])<=set(r['allowed']) and r['allowed']==['ad_eval','memcpy','memset','__stack_chk_fail']
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
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T67-007.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'adt.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/adt/arena_flow/FLOW.md' in m['shared_contracts']
    return True

def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(flow_cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][1]['symbols'].append('unpermitted_solver'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(tree_visits=16),lambda a,b:a['input_sha256'].pop('assessor/adt/adt.c'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:a['observations'][0]['result']['boolean_outcomes'].update({'3':0}),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(flow_evaluations=0)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
