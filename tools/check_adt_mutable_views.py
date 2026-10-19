"""Current-source coordinate, mutation, emitted SIMD and library-gap guard."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_mutable_views_validation.json'
CODEGEN='research/v5/modern/adt_view_codegen_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'view_geometry',2:'view_geometry',3:'view_transform',4:'view_transform',5:'view_vector',6:'view_vector',7:'view_reduction',8:'view_reduction',9:'view_base',10:'view_base'}
def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==set(CONTROL.values())
        assert len(p['controls'])==10 and {r['test'] for r in p['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==5 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==6 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and (r['cases'],r['enumerated_layouts'],r['seeded_layouts'],r['boundary_reductions'])==(98576,94464,4096,16) and r['trace_sha256']=='3748ccf45f2aba19156d03fcf666ad8c26fb50bf5c3695e4ce2e53b9346f566d'
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert r['view_size']>0 and r['view_alignment']>0 and r['elements']==2048 and r['iterations']==1000 and r['checksum']==2048000 and len(r['scalar_seconds'])==len(r['vector_seconds'])==7 and all(x>=0 for x in r['scalar_seconds']+r['vector_seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert {'assessor/adt/adt.c','assessor/adt/mutable_views/mutable_views.c','assessor/adt/mutable_views/vector.c'}<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-view-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==60 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==5 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    objects=[r for r in q['observations'] if r['kind']=='vector-object'];strict=[r for r in q['observations'] if r['kind']=='strict-core'];span=[r for r in q['observations'] if r['kind']=='span'];mdspan=[r for r in q['observations'] if r['kind']=='mdspan'];cost=[r for r in q['observations'] if r['kind']=='incremental']
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    assert all(r['packed_add_observed'] and not r['automatic_vectorization'] and set(r['undefined_symbols'])<={'__stack_chk_fail'} and r['allowed_host_symbols']==['__stack_chk_fail'] for r in objects)
    assert len(strict)==2 and {r['compiler'] for r in strict}=={'gcc','clang'} and all(r['passed'] for r in strict)
    assert len(span)==2 and {r['compiler'] for r in span}=={'g++','clang++'} and all(r['executed'] for r in span)
    assert len(mdspan)==2 and {r['compiler'] for r in mdspan}=={'g++','clang++'} and all(not r['available'] and r['execution']=='not-run' for r in mdspan)
    assert len(cost)==1 and cost[0]['unchanged_no_work'] and min(cost[0]['clean_seconds'],cost[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==36 and sum(r['exit_code']!=0 for r in q['executions'])==2
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale codegen source'
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8');assert (r['exit_code']==0 if r['expected_exit']==0 else r['exit_code']!=0)
            if r['required_marker']:assert r['required_marker'] in text
            if r['exit_code']:assert 'fatal error' in text and ('No such file or directory' in text or 'file not found' in text) and 'mdspan' in text
            if r['command'][0]=='objdump' and '-dr' in r['command']:assert 'paddq' in text and '%xmm' in text and '%ymm' not in text and '%zmm' not in text
        for row in mutants+learners:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for row in ctests:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert all(s in text for s in ['ADV_GEOMETRY exact-gcd-injectivity','ADV_TRANSFORM transpose-submatrix-borrow','ADV_VECTOR widened-four-lane-result','ADV_REDUCE equal-overflow-alias-policy padded-transpose-tail','ADV_BASE actual-readonly-strides','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T67-006.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'adt.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/adt/mutable_views/VIEWS.md' in m['shared_contracts']
    return True
def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][0].update(packed_add_observed=False),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:a['input_sha256'].pop('assessor/adt/adt.c'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:next(r for r in b['observations'] if r['kind']=='mdspan').update(execution='passed'),lambda a,b:b['observations'][0]['undefined_symbols'].append('memcpy'),lambda a,b:b['observations'][0].update(automatic_vectorization=True)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/CODEGEN).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
