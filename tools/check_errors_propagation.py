"""Guard exact parser/model observations and statement-only compiler evidence."""
import argparse
import copy
import json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_propagation_validation.json'
COST='research/v5/modern/errors_propagation_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'prop_macro',2:'prop_stream',3:'prop_stream',4:'prop_stream',5:'prop_macro',6:'prop_ordinary',7:'prop_ordinary',8:'prop_base',9:'prop_base'}
ANCHORS=set(CONTROL.values())
TRACE='37d1136729e06eaeffa2934e5360bd09239eb66f61a4270191b17bd96d7caf6c'

def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==9
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['ordinary_cases'],r['segmented_cases'],r['trace_sha256'])==(19548,55959,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['allocation_calls'])==(10000,121590000,0) and min(r['parser_size'],r['parser_align'])>0
                assert all(len(r[x])==7 and all(t>=0 for t in r[x]) for x in ['base_seconds','explicit_seconds','macro_seconds'])
    assert {'assessor/errors/errors.c','assessor/errors/propagation/propagation.c','assessor/errors/propagation/propagation_macro.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-propagation-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==155 and len(mutants)==54 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    obs=q['observations'];assert len(obs)==9 and len(q['executions'])==17
    negative=[r for r in q['executions'] if r['expected_failure']];assert len(negative)==4 and all(r['exit_code'] not in [None,0] for r in negative)
    assert all(r['exit_code']==0 or r['expected_failure'] for r in q['executions'])
    for cc in ['gcc','clang']:
        diag=[r for r in obs if r['kind']=='statement-refusal' and r['compiler']==cc];assert len(diag)==2 and {r['case'] for r in diag}=={1,2} and all(r['rejected'] and not r['executed'] for r in diag)
        positive=[r for r in obs if r['kind']=='statement-positive' and r['compiler']==cc];assert len(positive)==1 and positive[0]['executed'] and positive[0]['if_else'] and positive[0]['fresh_name']
        expansion=[r for r in obs if r['kind']=='expansion' and r['compiler']==cc];assert len(expansion)==1 and expansion[0]['actual_tokens'] and not expansion[0]['statement_expression']
    builds=[r for r in obs if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    assert len(q['generated_artifacts'])==2
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r['expected_failure']:assert 'error' in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['EPR_ORDINARY checked-pieces','EPR_STREAM signed-first absolute-errors','EPR_MACRO single-expression single-destination','EPR_BASE actual-parser','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-002.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/propagation/PROPAGATION.md' in m['shared_contracts']
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(segmented_cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='statement-positive').update(fresh_name=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='expansion').update(statement_expression=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='statement-refusal').update(executed=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['executions'] if r['expected_failure']).update(exit_code=0),lambda a,b:b['generated_artifacts'].pop()]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
