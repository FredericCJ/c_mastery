"""Reject stale, missing or overstated resource-COW evidence."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/ownership_cow_resources_validation.json'
COST='research/v5/modern/ownership_cow_resources_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'cow_resources_snapshots',2:'cow_resources_failures',3:'cow_resources_identity',4:'cow_resources_close',5:'cow_resources_base',6:'cow_resources_base'}
ANCHORS=set(CONTROL.values())|{'cow_resources_threads'}
TRACE='f699e9087c587ac1ea0f28e363e8e56f0d13d62c2ee902832797091d30ec0330'

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
            if kind=='independent-oracle':assert r['passed'] and (r['histories'],r['max_operations'],r['owners'],r['trace_sha256'])==(3275,5,3,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['unique_clone_calls'],r['detach_clone_calls'])==(1000,3496500,0,3) and r['store_size']>0 and r['token_size']>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/ownership/ownership.c','assessor/ownership/cow_resources/cow_resources.c','labs/ownership/include/imports/v4_algo.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-cow_resources-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==139 and len(mutants)==36 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==7 and len(q['executions'])==31 and len(q['generated_artifacts'])==2 and all(r['exit_code']==0 for r in q['executions'])
    for cc in ['gcc','clang']:
        core=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc];assert len(core)==1;r=core[0];assert {'pthread_mutex_lock','pthread_mutex_unlock','pthread_mutex_init','pthread_mutex_destroy'}<=set(r['symbols']) and not r['explicit_heap_imports'] and not r['arbitrary_helper_absence_claim'] and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        cases=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(cases)==1 and all(cases[0][k] for k in ['partial_clone_rollback','actual_two_readers','retained_old_version','joined_before_destroy','executed'])
        assert cases[0]['reader_snapshots']==800 and cases[0]['writer_attempts']==500
        tsan=[r for r in q['observations'] if r['kind']=='thread-sanitizer' and r['compiler']==cc];assert len(tsan)==1;r=tsan[0];assert r['language']=='c17' and r['backend']=='POSIX pthread' and not r['all_interleavings_proved'] and all(r[k] for k in ['actual_two_readers','actual_one_writer','retained_old_version','executed','diagnostic_clean'])
    tsbuild=[r for r in q['executions'] if '-fsanitize=thread' in r['command']];assert len(tsbuild)==2 and all('-std=c17' in r['command'] and '-pthread' in r['command'] for r in tsbuild)
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['OCR_BASE actual-struct ow_cow','raw-alias-bypass','OCR_SNAPSHOTS actual-owned-resources','OCR_FAILURES raw typed post-acquisition-partial-clone','OCR_IDENTITY four-slot-capacity','OCR_CLOSE no-new-snapshot','OCR_THREADS actual-two-readers','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-007.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/cow_resources/COW.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(unique_clone_calls=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(arbitrary_helper_absence_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(actual_two_readers=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(joined_before_destroy=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(partial_clone_rollback=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(retained_old_version=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='thread-sanitizer').update(all_interleavings_proved=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='thread-sanitizer').update(language='c11')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
