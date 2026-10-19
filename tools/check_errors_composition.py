"""Reject incomplete, vacuous or stale evidence for original T65.04 claims."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_composition_validation.json'
COST='research/v5/modern/errors_composition_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'compose_owner',2:'compose_failure',3:'compose_owner',4:'compose_cause',5:'compose_cause',6:'compose_failure',7:'compose_owner',8:'compose_base',9:'compose_base'}
ANCHORS=set(CONTROL.values())|{'compose_laws','compose_effects'}
TRACE='c4adff85dfa01166240eb16ff0cbe20f2943a0d86b22fe563a06a7fed286f199'
def metric(r):
    assert (r['iterations'],r['checksum'],r['allocation_calls'],r['release_calls'])==(10000,735000,140000,140000)
    assert min(r['result_size'],r['result_align'],r['error_size'])>0
    assert all(len(r[x])==7 and all(t>=0 for t in r[x]) for x in ['scalar_seconds','owned_seconds'])
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==9
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['map_cases'],r['bind_cases'],r['cause_cases'],r['owner_cases'],r['trace_sha256'])==(8192,165888,162,32,TRACE)
            else:metric(r)
    assert {'assessor/errors/errors.c','assessor/errors/owning_option/owning_option.c','assessor/ownership/ownership.c','assessor/errors/composition/composition.c'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    learner=next(c for c in d['compiler_dependency_closures'] if c['profile']=='learner');assert 'assessor/errors/owning_option/owning_option.c' in learner['repository_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-composition-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==181 and len(mutants)==54 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==6 and len(q['executions'])==32 and len(q['generated_artifacts'])==4
    negative=[r for r in q['executions'] if r['expected_failure']];assert len(negative)==1 and negative[0]['exit_code'] not in [None,0] and 'erc_bad_copy' in negative[0]['command']
    assert all(r['exit_code']==0 or r['expected_failure'] for r in q['executions'])
    for cc in ['gcc','clang']:
        analysis=[r for r in q['observations'] if r['kind']=='dispatch-analysis' and r['compiler']==cc];assert len(analysis)==1;r=analysis[0]
        assert set(r['indirect_sites'])=={'er_map','er_then','erc_map_clone'} and all(x>=1 for x in r['indirect_sites'].values()) and r['actual_owner_imports'] and r['precedes_timing'] and r['no_explicit_heap_imports']
        costs=[r for r in q['observations'] if r['kind']=='metrics-after-inspection' and r['compiler']==cc];assert len(costs)==1;metric(costs[0]['result'])
        last_inspection=max(i for i,r in enumerate(q['executions']) if r['log'].endswith(cc+'-erc_map_clone-dispatch.log') or r['log'].endswith(cc+'-er_then-dispatch.log'))
        timing=next(i for i,r in enumerate(q['executions']) if r['log'].endswith(cc+'-metrics.log'));assert last_inspection<timing
    rust=[r for r in q['observations'] if r['kind']=='rust-comparison'];assert len(rust)==1;r=rust[0];assert r['moved_value_rejected'] and r['drop_count']==1 and r['lazy_calls']==0 and r['eager_calls']==1 and r['inner_offset_preserved'] and r['executed']
    builds=[r for r in q['observations'] if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r['expected_failure']:assert 'error' in text and 'use of moved value' in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['ERC_LAWS finite-table','ERC_BASE actual-er-map-then','ERC_EFFECTS eager-unused-one','ERC_OWNER actual-allocator','ERC_FAILURE clone-admission','ERC_CAUSE bounded-inner-outer','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-004.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/composition/COMPOSITION.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['learner_stub_rejections'][0].update(semantic_rejection=False),lambda a,b:a['observations'][0]['result'].update(bind_cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(release_calls=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='dispatch-analysis')['indirect_sites'].update(er_then=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-comparison').update(moved_value_rejected=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-comparison').update(inner_offset_preserved=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['executions'] if r['expected_failure']).update(exit_code=0)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
