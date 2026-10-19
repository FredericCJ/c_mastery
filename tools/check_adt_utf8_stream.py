"""Current-source incremental/owner UTF-8 and borrow-comparison guard."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_utf8_stream_validation.json'
DIAGNOSTICS='research/v5/modern/adt_utf8_diagnostics_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'utf8_chunks',2:'utf8_finalization',3:'utf8_finalization',4:'utf8_chunks',5:'utf8_owner',6:'utf8_owner',7:'utf8_base',8:'utf8_base'}
def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==set(CONTROL.values())
        assert len(p['controls'])==8 and {r['test'] for r in p['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==6 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and r['cases']==233265 and r['complete_small_strings']==65793 and r['small_string_splits']==197121 and r['possible_prefixes']==17652 and r['scalar_encodings']==1112064 and r['trace_sha256']=='3d69ffdda5c35fada43752b59c5cafbf3e25a70e672bfeb3671ca60e4e07fa5e'
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert r['retained_bytes']==3 and r['stream_size']>=3 and r['owner_size']>0 and r['iterations']==10000 and r['checksum']==30000 and len(r['seconds'])==7 and all(x>=0 for x in r['seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert {'assessor/adt/adt.c','assessor/adt/owning_sum/owning_sum.c','labs/adt/owning_sum/owning_sum.h'}<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-utf8-stream-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    permitted={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in permitted for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    assert len(q['observations'])==3 and {(r['case'],r['compile_rejected'],r['executed']) for r in q['observations']}=={('last-use',False,True),('live-borrow',True,False),('owned-copy',False,True)}
    assert len(q['executions'])==6 and sum(r['exit_code']!=0 for r in q['executions'])==1
    failed=next(r for r in q['executions'] if r['exit_code']!=0);assert failed['required_marker']=='E0506' and failed['expected_exit']==1
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale diagnostic source'
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            assert (r['exit_code']==0 if r['expected_exit']==0 else r['exit_code']!=0)
            if r['required_marker']:assert r['required_marker'] in text
        for row in mutants+learners:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','runtime error:'])
        for row in ctests:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert all(s in text for s in ['ADU_CHUNKS copied-pending','ADU_FINALIZE incomplete-refusal','ADU_OWNER actual-owning-sum','ADU_BASE actual-ad-text','100% tests passed'])
    return True
def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][1].update(compile_rejected=False),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(retained_bytes=4),lambda a,b:a['observations'][0]['result'].update(possible_prefixes=0),lambda a,b:a['input_sha256'].pop('assessor/adt/owning_sum/owning_sum.c')]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/DIAGNOSTICS).read_text());check(d,q);n=selftest(d,q) if args.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
