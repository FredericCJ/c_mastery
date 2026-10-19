"""Bind exact ordinary/nested predicate grids and publication controls."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_serial_timer_validation.json'
COST='research/v5/modern/errors_serial_timer_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'timer_lifetime',2:'timer_lifetime',3:'timer_window',4:'timer_window',5:'timer_epoch',6:'timer_epoch',7:'timer_base',8:'timer_base'}
ANCHORS=set(CONTROL.values())|{'timer_relation','timer_admission'}
TRACE='c43ae081d3545a2ba2814081f076e2e054dbce4763bf578226d78e9c5cc081f2'
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==8
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['policy_cases'],r['serial_cases'],r['window_cases'],r['epoch_cases'],r['cycles'],r['trace_sha256'])==(324,417,70263,1875,336,TRACE) and r['cycle_witness']==[0,11,6]
            else:
                assert (r['iterations'],r['checksum'],r['allocation_calls'])==(10000,770000,0) and min(r['window_size'],r['epoch_size'])>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/errors/errors.c','assessor/errors/serial_timer/serial_timer.c'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-serial_timer-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==151 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==9 and len(q['executions'])==33 and len(q['generated_artifacts'])==2 and all(r['exit_code']==0 for r in q['executions'])
    for cc in ['gcc','clang']:
        core=[r for r in q['observations'] if r['kind']=='bounded-core' and r['compiler']==cc];assert len(core)==1;r=core[0];assert 'er_serial16' in r['symbols'] and r['no_explicit_heap_imports'] and r['object_observed'] and not r['arbitrary_helper_absence_claim'] and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
        cases=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(cases)==1 and cases[0]['cumulative_residence'] and cases[0]['absolute_epoch'] and cases[0]['executed']
        checked=[r for r in q['observations'] if r['kind']=='checked-comparison' and r['compiler']==cc];assert len(checked)==2 and {r['profile'] for r in checked}=={'debug','release'}
        for r in checked:assert r['raw_overflow_value']==0 and r['staged_overflow_value']==91 and r['operands_evaluated_once'] and r['executed']
    assert any(p.endswith('/stdckdint.h') for p in q['external_input_sha256'])
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['EAT_BASE actual-add16-policy','EAT_RELATION widths-two-through-sixteen','EAT_WINDOW actual-serial-import','EAT_LIFETIME accumulated-residence','EAT_EPOCH low-width-independent','EAT_ADMISSION finite-capacity','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-007.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/serial_timer/TIMERS.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(window_cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='bounded-core')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='bounded-core').update(arbitrary_helper_absence_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(cumulative_residence=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='optimized-boundaries').update(absolute_epoch=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='checked-comparison').update(staged_overflow_value=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='checked-comparison').update(operands_evaluated_once=False)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
