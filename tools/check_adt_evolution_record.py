"""Guard lossless duplicates, real primitive imports and synthetic authorization repair."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_evolution_record_validation.json'
COST='research/v5/modern/adt_evolution_cost_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'evolution_records',2:'evolution_records',3:'evolution_auth',4:'evolution_auth',5:'evolution_policy',6:'evolution_policy',7:'evolution_base',8:'evolution_base'}
MARKERS=['ADE_RECORD stable-duplicates actual-cursor-codec owned-relay','ADE_AUTH all-unknown-occurrences stable-canonical-transcript','ADE_POLICY local-privileged-counter witness authenticated-not-authorized','ADE_BASE actual-unknown-bytes known-validation borrowed-lifetime']
TRACE='5b4056e27f18bd1d6fc28bb01c4cf120062c9229ffbd67e4eb0565a49659a1f1'

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
        r=row['result'];assert r['passed'] and (r['cases'],r['admitted'])==(10100,5244) and r['trace_sha256']==TRACE and r['apply_statuses']=={'0':80,'1':4856,'2':0,'3':1409,'4':3751,'5':4}
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'],r['bytes'])==(1000,235000,145)
        assert min(r['record_size'],r['record_alignment'])>0
        assert all(len(r[k])==7 and all(x>=0 for x in r[k]) for k in ['relay_seconds','authentication_seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert {'assessor/adt/adt.c','assessor/adt/evolution_record/evolution_record.c','assessor/foundations/foundation.c','assessor/api_depth/architecture/core.c','assessor/replicated/hashing/hashing.c'}<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-evolution-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    objects=[r for r in q['observations'] if r['kind']=='object'];strict=[r for r in q['observations'] if r['kind']=='strict'];cost=[r for r in q['observations'] if r['kind']=='incremental']
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in objects:assert {'ad_evolve_read','ad_evolve_write','f_take','rph_hmac','rph_equal','rph_clear'}<=set(r['symbols']) and set(r['symbols'])<=set(r['allowed']) and r['allowed']==['ad_evolve_read','ad_evolve_write','f_take','rph_hmac','rph_equal','rph_clear','memcpy','memset','__stack_chk_fail']
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
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T67-009.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'adt.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/adt/evolution_record/EVOLUTION.md' in m['shared_contracts']
    return True

def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][1]['symbols'].append('unpermitted_solver'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(bytes=0),lambda a,b:a['input_sha256'].pop('assessor/adt/adt.c'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:a['observations'][0]['result'].update(admitted=0),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
