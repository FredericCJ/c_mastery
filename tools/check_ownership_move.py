"""Reject stale or overstated move/jump and fallible-recovery evidence."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/ownership_move_validation.json'
COST='research/v5/modern/ownership_move_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'move_inline',2:'move_inline',3:'move_registry',4:'move_registry',5:'move_base',6:'move_base'}
ANCHORS=set(CONTROL.values())|{'move_admission','move_theft','move_retirement'}
TRACE='539961886c6c228e8e04853248a4f426e758f8fecd6248c563e9d2606d028901'

def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==6
        assert {r['test'] for r in p['controls']}=={f'ownership.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['cases'],r['lengths'],r['generation_seeds'],r['move_depths'],r['trace_sha256'])==(630,10,7,9,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['typed_allocation_attempts'],r['raw_allocations'],r['raw_releases'])==(10000,210000,0,1,1) and min(r['value_size'],r['token_size'],r['registry_bytes'])>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/ownership/ownership.c','assessor/ownership/move/move.c','assessor/ownership/move/registry.c','labs/ownership/include/imports/v4_algo.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-move-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==151 and len(mutants)==36 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==9 and len(q['executions'])==43 and len(q['generated_artifacts'])==4
    negatives=[r for r in q['executions'] if r['expected_compile_rejection']];assert len(negatives)==6 and all(r['exit_code']!=0 for r in negatives)
    assert all(r['exit_code']==0 or r in negatives for r in q['executions'])
    for cc in ['gcc','clang']:
        core=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc];assert len(core)==2 and {r['source'] for r in core}=={'move','registry'}
        for r in core:assert not r['explicit_heap_imports'] and not r['arbitrary_helper_absence_claim'] and r['callback_allocation_allowed'] and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        assert {'omm_init','omm_consume'}<=set(next(r for r in core if r['source']=='registry')['symbols'])
        cases=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(cases)==1 and all(cases[0][k] for k in ['inline_pointer_repaired','copied_tokens_stale','identical_holder_can_act_first','final_consumption_allowed','executed'])
        diag=[r for r in q['observations'] if r['kind']=='diagnostic-comparison' and r['compiler']==cc];assert len(diag)==1;r=diag[0];assert r['core_language']=='c17' and r['comparison_language']=='c23' and not r['ownership_type_enforcement'] and all(r[k] for k in ['opaque_negative_rejected','ordinary_copy_discard_accepted','extension_warning_rejected_by_werror','c23_recommended_warning_observed','explicit_void_cast_accepted'])
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['OMM_BASE actual-move','OMM_INLINE self-address','OMM_ADMISSION actual-allocation-failure','OMM_REGISTRY opaque-representation','OMM_THEFT identical-live-copy','OMM_RETIREMENT final-generation','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-003.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/move/MOVE.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(typed_allocation_attempts=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(arbitrary_helper_absence_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(identical_holder_can_act_first=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='diagnostic-comparison').update(ownership_type_enforcement=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:next(r for r in b['executions'] if r['expected_compile_rejection']).update(exit_code=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='diagnostic-comparison').update(core_language='c11')]

    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
