"""Reject missing, stale, or overstated reference-counting evidence."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
from c_source import find_unique
REPORT='research/v5/modern/ownership_references_validation.json'
COST='research/v5/modern/ownership_references_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'references_cycle',2:'references_base',3:'references_base'}
ANCHORS=set(CONTROL.values())|{'references_overflow','references_failures','references_threads','references_race'}
TRACE='e19380f555ae90bfccd00963370dbf027ca3f1852623314504629158ebda5527'
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==3
        assert {r['test'] for r in p['controls']}=={f'ownership.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['histories'],r['max_operations'],r['implementations'],r['trace_sha256'])==(12787,7,2,TRACE)
            else:assert (r['iterations'],r['checksum'])==(10000,70000) and r['atomic_size']>0 and r['atomic_alignment']>0 and r['lock_free'] in [0,1] and len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/ownership/ownership.c','assessor/ownership/references/references.c','labs/ownership/include/imports/v4_algo.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-references-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==121 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==12 and len(q['executions'])==52 and len(q['generated_artifacts'])==3
    for cc in ['gcc','clang']:
        a=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc];assert len(a)==1
        a=[r for r in q['observations'] if r['kind']=='optimized-boundaries' and r['compiler']==cc];assert len(a)==1 and a[0]['actual_race_rounds']==256 and all(a[0][k] for k in ['actual_allocated_cycle','actual_four_threads','executed'])
        a=[r for r in q['observations'] if r['kind']=='thread-sanitizer-comparison' and r['compiler']==cc];assert len(a)==1;r=a[0]
        assert r['language']=='c17' and (r['original_race_exit'],r['minimal_fence_exit'],r['stronger_fence_exit'],r['stronger_provider_exit'])==(66,66,0,0) and r['provider_unchanged'] and not any(r[k] for k in ['original_diagnostic_clean','suppression_used','all_interleavings_proved'])
    models=[r for r in q['observations'] if r['kind']=='rc11-strong-cas-comparison'];assert len(models)==4
    assert {(r['case'],r['mutation'],r['exit_code'],r['complete_executions']) for r in models}=={(2,None,0,2),(2,'-DORC_RELAX_RELEASE=1',42,None),(1,None,0,4),(1,'-DORC_RESURRECT=1',42,None)}
    assert all(r['exact_atomic_function_slice'] and not r['original_weak_cas'] and not r['heap_reclamation_modeled'] for r in models)
    original=[r for r in q['observations'] if r['kind']=='rc11-original-weak-cas'];assert len(original)==1;r=original[0];assert not r['arbitrary_retry_proof'] and r['timeout_seconds']==45 and (r['status'],r['exit_code']) in [('timeout-unvalidated',None),('bounded-client-passed',0)]
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    warnings=[r for r in q['executions'] if r['exit_code']==66];negatives=[r for r in q['executions'] if r['exit_code']==42];timeouts=[r for r in q['executions'] if r['exit_code'] is None]
    assert len(warnings)==4 and len(negatives)==2 and len(timeouts)==int(original[0]['exit_code'] is None)
    for r in q['executions']:
        assert r['exit_code'] in r['expected_exit_codes'] and r['timed_out']==(r['exit_code'] is None)
        if r['exit_code'] is None:assert '-DORC_CASE=1' in r['command'] and '-DORC_STRONG_CAS=1' not in r['command']
    assert sum('-fsanitize=thread' in r['command'] for r in q['executions'])==8
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r in warnings:assert 'WARNING: ThreadSanitizer: data race' in text and ('ORC_RACE synchronized-start rounds256' in text or 'ORC_FENCE release-rmw-acquire-fence' in text)
            if r in negatives:assert 'Error:' in text and 'GenMC v0.19.0' in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['ORC_BASE actual-nonatomic','ORC_OVERFLOW actual-maximum-refusal','ORC_CYCLE actual-allocated-two-nodes','ORC_FAILURE raw-versus-typed-acquisition','ORC_THREADS actual-four-owned-workers','ORC_RACE synchronized-start rounds256','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):assert all('-std=c17' in c['command'] for c in json.loads((ROOT/r['path']).read_text()))
        provider=(ROOT/'assessor/ownership/ownership.c').read_text(encoding='utf8');slice_record=next(r for r in q['generated_artifacts'] if r['path'].endswith('atomic_provider.c'));assert (ROOT/slice_record['path']).read_text(encoding='utf8')=='#include "ownership.h"\n#include <limits.h>\n'+provider[find_unique(provider,'static OwStatus atomic_increment(')[0]:find_unique(provider,'OwStatus ow_cow_clone(')[0]]
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-006.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/references/REFERENCES.md' in m['shared_contracts']
    return True
def selftest(d,q):
    def observation(r,kind):return next(x for x in r['observations'] if x['kind']==kind)
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:observation(a,'metrics')['result'].update(checksum=1),lambda a,b:observation(b,'thread-sanitizer-comparison').update(original_diagnostic_clean=True),lambda a,b:observation(b,'thread-sanitizer-comparison').update(suppression_used=True),lambda a,b:observation(b,'optimized-boundaries').update(actual_four_threads=False),lambda a,b:observation(b,'optimized-boundaries').update(actual_race_rounds=0),lambda a,b:observation(b,'incremental').update(unchanged_no_work=False),lambda a,b:b['executions'][0].update(exit_code=1),lambda a,b:observation(b,'rc11-strong-cas-comparison').update(original_weak_cas=True),lambda a,b:observation(b,'rc11-strong-cas-comparison').update(heap_reclamation_modeled=True),lambda a,b:observation(b,'rc11-original-weak-cas').update(arbitrary_retry_proof=True),lambda a,b:observation(b,'thread-sanitizer-comparison').update(language='c11'),lambda a,b:observation(b,'rc11-original-weak-cas').update(status='passed')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
