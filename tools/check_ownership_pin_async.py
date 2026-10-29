"""Reject stale, missing or overstated asynchronous pin evidence."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/ownership_pin_async_validation.json'
COST='research/v5/modern/ownership_pin_async_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'pin_async_timeout',2:'pin_async_notification',3:'pin_async_identity',4:'pin_async_counters',5:'pin_async_base',6:'pin_async_base',7:'pin_async_dma',8:'pin_async_dma'}
ANCHORS=set(CONTROL.values())|{'pin_async_host'}
TRACE='65de2be27b61c80c78261d9debf34f58b8735b344150aa81a983a2006ca7c0c5'
REQUIRED={'pin_async.c':{'f_submit','f_cancel','f_complete','ow_pin_submit','ow_pin_complete'},'pin_host.c':{'pthread_create','pthread_join','pthread_self','pthread_mutex_lock'},'pin_dma.c':{'pd_validate','pd_submit','pd_cancel','pd_poll','pd_finalize'}}
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==8
        assert {r['test'] for r in p['controls']}=={f'ownership.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==7 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['histories'],r['max_operations'],r['alphabet'],r['trace_sha256'])==(37449,5,8,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['terminal_releases'])==(10000,2940000,70000) and r['machine_size']>0 and r['port_size']>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    needed={'assessor/ownership/ownership.c','assessor/ownership/pin_async/pin_async.c','assessor/ownership/pin_async/pin_host.c','assessor/ownership/pin_async/pin_dma.c','assessor/foundations/foundation.c','assessor/peripheral_depth/dma/dma.c','labs/ownership/pin_async/imports/dma.h','labs/ownership/pin_async/imports/dma.c'};assert needed<=set(d['input_sha256']) and needed<=set(q['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-pin_async-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==200 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==7 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==15 and len(q['executions'])==47 and len(q['generated_artifacts'])==6
    failures=[r for r in q['executions'] if r['exit_code']!=0];assert len(failures)==2 and all(r['exit_code']==r['expected_exit']==1 for r in failures)
    assert len([r for r in failures if 'cmake'==r['command'][0]])==1 and len([r for r in failures if r['command'][-1]=='pin_async_dma'])==1
    assert all(r['exit_code']==r['expected_exit'] for r in q['executions'])
    for cc in ['gcc','clang']:
        for source,required in REQUIRED.items():
            selected=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc and r['source']==source];assert len(selected)==1;r=selected[0];assert required<=set(r['symbols']) and not r['explicit_heap_imports'] and not r['freestanding_claim'] and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        cases=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(cases)==1 and all(cases[0][k] for k in ['actual_foreign_receiver','callback_held_before_done','actual_join_before_reuse','actual_fasync_provider','actual_pd_provider','executed']) and not cases[0]['physical_device'] and cases[0]['dma_writes']=='original native RAM model'
        tsan=[r for r in q['observations'] if r['kind']=='thread-sanitizer' and r['compiler']==cc];assert len(tsan)==1;r=tsan[0];assert r['language']=='c17' and r['backend']=='POSIX pthread' and not r['all_interleavings_proved'] and all(r[k] for k in ['foreign_receiver','actual_join','executed','diagnostic_clean'])
    tsbuild=[r for r in q['executions'] if '-fsanitize=thread' in r['command']];assert len(tsbuild)==2 and all('-std=c17' in r['command'] and '-pthread' in r['command'] for r in tsbuild)
    def one(kind):
        a=[r for r in q['observations'] if r['kind']==kind];assert len(a)==1;return a[0]
    r=one('public-import-identity');assert r['producer']=='PR-T28-06' and not r['canonical_firmware_io_dma_claim'] and r['header']==d['input_sha256']['labs/peripheral_depth/dma/dma.h'] and r['unfinished_stub']==d['input_sha256']['labs/peripheral_depth/dma/dma.c']
    r=one('exported-composition');assert r['external_pd_source']=='assessor/peripheral_depth/dma/dma.c' and r['external_pd_sha256']==d['input_sha256'][r['external_pd_source']] and r['public_header_and_stub_identical'] and not r['private_provider_copied'] and r['executed']
    r=one('missing-provider');assert r['rejected'] and r['configuration_failure']
    r=one('unfinished-provider');assert r['compiled'] and r['semantic_rejection']
    r=one('incremental');assert r['unchanged_no_work'] and min(r['clean_seconds'],r['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for name in ['dma.h','dma.c']:assert (ROOT/'labs/ownership/pin_async/imports'/name).read_bytes()==(ROOT/'labs/peripheral_depth/dma'/name).read_bytes()
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['OPA_BASE actual-struct ow_pin','OPA_TIMEOUT recovery-required','OPA_NOTIFICATION different-executor','OPA_IDENTITY stale-ticket','OPA_COUNTERS reserve-terminal-capacity','OPA_HOST actual-foreign-callback','OPA_DMA actual-Pd-struct f_async-struct ow_pin','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-008.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/pin_async/PIN.md' in m['shared_contracts']
        f=json.loads((ROOT/'labs/ownership/family.json').read_text(encoding='utf8'));a=next(r for r in f['exercises'] if r['id']=='X-T66-008');assert any(r['family']=='foundations' and r['artifact']=='io.async' and r['producer']=='PR-T21-02' for r in a['dependencies'])
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(terminal_releases=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(freestanding_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(actual_foreign_receiver=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(actual_join_before_reuse=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(physical_device=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='thread-sanitizer').update(all_interleavings_proved=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='thread-sanitizer').update(language='c11'),lambda a,b:next(r for r in b['observations'] if r['kind']=='exported-composition').update(private_provider_copied=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='missing-provider').update(rejected=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='unfinished-provider').update(compiled=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='public-import-identity').update(canonical_firmware_io_dma_claim=True)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
