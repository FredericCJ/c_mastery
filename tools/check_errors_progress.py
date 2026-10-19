"""Guard actual stream composition, product-state tables and native controls."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_progress_validation.json'
COST='research/v5/modern/errors_progress_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'progress_core',2:'progress_core',3:'progress_cancel',4:'progress_epochs',5:'progress_cancel',6:'progress_epochs',7:'progress_uncertain',8:'progress_uncertain',9:'progress_base',10:'progress_base'}
ANCHORS=set(CONTROL.values())|{'progress_native'}
TRACE='a581f12ecbfe4a99b1634b4287eb877f018a32e31b4d6ad071bed280676665e0'

def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==10
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['transfer_cases'],r['composed_cases'],r['epoch_cases'],r['trace_sha256'])==(2940,540,3960,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['allocation_calls'],r['copied_frame_bytes'])==(10000,350000,0,5) and min(r['stream_size'],r['stream_align'])>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/errors/errors.c','assessor/errors/progress/progress.c','assessor/api_depth/stream.c','labs/api_depth/stream/stream.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-progress-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==175 and len(mutants)==60 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    obs=q['observations'];assert len(obs)==9 and len(q['executions'])==28
    negative=[r for r in q['executions'] if r['expected_failure']];assert len(negative)==4 and all(r['exit_code']==1 for r in negative)
    assert all(r['exit_code']==0 or r['expected_failure'] for r in q['executions'])
    for cc in ['gcc','clang']:
        controls=[r for r in obs if r['kind']=='native-control' and r['compiler']==cc];assert len(controls)==2 and {r['mutation'] for r in controls}=={2,4} and all(r['semantic_rejection'] and r['actual_syscalls'] for r in controls)
        positive=[r for r in obs if r['kind']=='native-positive' and r['compiler']==cc];assert len(positive)==1 and positive[0]['actual_syscalls'] and not positive[0]['semantic_rejection'] and positive[0]['mutation']==0
        imports=[r for r in obs if r['kind']=='object-imports' and r['compiler']==cc];assert len(imports)==1 and imports[0]['actual_producers'] and imports[0]['no_explicit_heap_symbols'] and {'er_transfer','sf_init','sf_start','sf_pump','sf_cancel'}<=set(imports[0]['symbols']) and not set(imports[0]['symbols'])&{'malloc','free','realloc','calloc'}
    builds=[r for r in obs if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    assert len(q['generated_artifacts'])==2
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r['expected_failure']:assert not any(s in text for s in ['AddressSanitizer','runtime error:'])
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['EPT_CORE actual-sf-copy actual-er-transfer','EPT_CANCEL copied-owner retained-prefix','EPT_EPOCHS accepted-vs-durable','EPT_UNCERTAIN invalid-report','EPT_BASE actual-er-transfer','EPT_NATIVE actual-pipe-EAGAIN quota-two cancellation-prefix actual-fsync-two-epochs','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-003.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/progress/PROGRESS.md' in m['shared_contracts']
        f=json.loads((ROOT/'labs/errors/family.json').read_text(encoding='utf8'));row=next(r for r in f['exercises'] if r['id']=='X-T65-003');assert any(r['family']=='api_depth' and r['artifact']=='io.stream' and r['producer']=='PR-T19-04' for r in row['dependencies'])
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(epoch_cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='native-positive').update(actual_syscalls=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='native-control').update(semantic_rejection=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['executions'] if r['expected_failure']).update(exit_code=0),lambda a,b:b['generated_artifacts'].pop()]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
