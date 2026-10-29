"""Reject unsupported allocator accounting, consumer and recursion evidence."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/ownership_allocator_audit_validation.json'
COST='research/v5/modern/ownership_allocator_audit_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'allocator_audit_rollback',2:'allocator_audit_resize',3:'allocator_audit_poison',4:'allocator_audit_recursive',5:'allocator_audit_base',6:'allocator_audit_base'}
ANCHORS=set(CONTROL.values())|{'allocator_audit_counters','allocator_audit_consumers'}
TRACE='c68afdd73b55fb389315d3d482be004eb3998e2c589da8be9dff2ebe2af0b766'
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==6
        assert {r['test'] for r in p['controls']}=={f'ownership.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==7 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        selected=[r for r in d['observations'] if r['kind']==kind];assert len(selected)==6 and {r['profile'] for r in selected}==PROFILES
        for row in selected:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['histories'],r['max_operations'],r['alphabet'],r['trace_sha256'])==(19608,5,7,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['attempts'],r['upstream_attempts'],r['releases'],r['upstream_releases'])==(10000,2940000,140000,140000,70000,140000) and r['audit_size']>0
                assert len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    needed={'assessor/ownership/ownership.c','assessor/ownership/allocator_audit/allocator_audit.c','assessor/ownership/allocator_audit/consumers.c','assessor/algorithms/v4_algo.c','assessor/errors/errors.c','labs/ownership/include/imports/v4_algo.h'};assert needed<=set(d['input_sha256']) and needed<=set(q['input_sha256'])
    assert {'assessor/errors/owning_option/owning_option.c','labs/errors/owning_option/include/owning_option.h','labs/ownership/allocator_audit/tests/owned_consumer.c'}<=set(q['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-allocator_audit-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==176 and len(mutants)==36 and all(r['exit_code']==1 for r in mutants) and len(learners)==7 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['observations'])==13 and len(q['executions'])==43 and len(q['generated_artifacts'])==4 and all(r['exit_code']==0 for r in q['executions'])
    for cc in ['gcc','clang']:
        for source in ['allocator_audit.c','consumers.c']:
            selected=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc and r['source']==source];assert len(selected)==1;r=selected[0];assert not r['direct_libc_heap_imports'] and not r['whole_call_graph_no_allocation_claim'] and not set(r['symbols'])&{'malloc','calloc','realloc','free'}
            if source=='consumers.c':assert {'ow_allocator','er_parse_pair'}<=set(r['symbols'])
    consumers=[r for r in q['observations'] if r['kind']=='actual-owned-consumer'];assert len(consumers)==6 and {r['profile'] for r in consumers}==PROFILES
    for r in consumers:assert r['language']=='c17' and r['producer']=='PR-T65-01' and all(r[k] for k in ['zero_ID_present','create_clone_failure_sweeps','independent_backing','all_owners_drained','executed'])
    builds=[r for r in q['executions'] if any('owned_consumer.c' in x for x in r['command']) and '-M' not in r['command']];assert len(builds)==6 and all('-std=c17' in r['command'] for r in builds)
    def one(kind):
        a=[r for r in q['observations'] if r['kind']==kind];assert len(a)==1;return a[0]
    r=one('typed-ABI-identity');assert r['producer']=='PR-T14-02' and r['unchanged_declaration'] and r['header_sha256']==d['input_sha256']['labs/algorithms/include/v4_algo.h']==d['input_sha256']['labs/ownership/include/imports/v4_algo.h']
    r=one('exported-composition');assert set(r['explicit_workspaces'])=={'algorithms','errors'} and not r['private_provider_copied'] and r['executed'] and set(r['actual_sources'])=={'assessor/algorithms/v4_algo.c','assessor/errors/errors.c','assessor/ownership/ownership.c'}
    r=one('incremental');assert r['unchanged_no_work'] and min(r['clean_seconds'],r['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        assert (ROOT/'labs/ownership/include/imports/v4_algo.h').read_bytes()==(ROOT/'labs/algorithms/include/v4_algo.h').read_bytes()
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['OAA_BASE actual-struct ow_allocator','in-place-provider','OAA_ROLLBACK injection-and-upstream-failure','OAA_POISON observer-before-upstream-release','full-upstream-shrink-refusal','OAA_CONSUMERS actual-frontier-COW-failure-sweeps','OAA_RECURSIVE actual-T65-error-format','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-009.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/allocator_audit/ALLOCATOR.md' in m['shared_contracts']
        f=json.loads((ROOT/'labs/ownership/family.json').read_text(encoding='utf8'));a=next(r for r in f['exercises'] if r['id']=='X-T66-009');assert {(r['family'],r['artifact'],r['producer']) for r in a['dependencies']}=={('algorithms','containers.array','PR-T14-02'),('errors','errors.result','PR-T65-04')}
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(upstream_releases=70000),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(whole_call_graph_no_allocation_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='actual-owned-consumer').update(zero_ID_present=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='actual-owned-consumer').update(all_owners_drained=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='actual-owned-consumer').update(language='c11'),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='exported-composition').update(private_provider_copied=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='typed-ABI-identity').update(unchanged_declaration=False),lambda a,b:b['input_sha256'].pop('assessor/errors/owning_option/owning_option.c')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
