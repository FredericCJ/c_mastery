"""Reject stale/incomplete source-bound T76.03 author evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/replicated_clocks_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
ANCHORS={'clock_identity','clock_store','clock_policy'}
CONTROL={1:'clock_identity',2:'clock_identity',3:'clock_store',4:'clock_store',5:'clock_policy',6:'clock_policy'}
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def check(d,files=True):
    assert d.get('passed') and d.get('status')=='passed' and d.get('source_stable') and d.get('external_inputs_stable'),'failed/unstable report'
    assert {p['profile'] for p in d['profiles']}==PROFILES and len(d['profiles'])==8,'missing/duplicate profiles'
    wanted={f'replicated.mutation.{a}.{i}' for i,a in CONTROL.items()}
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS,'missing anchor'
        assert len(p['controls'])==6 and {c['test'] for c in p['controls']}==wanted and all(c['semantic_rejection'] for c in p['controls']),'unobserved semantic control'
    assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections']),'missing intended compiling learner rejection'
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle']
    metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracle)==8 and {r['profile'] for r in oracle}==PROFILES,'missing oracle profile'
    for r in oracle:
        q=r['result'];assert q['passed'] and q['histories']==128 and q['events']==6144 and q['reachability_pairs']==144384 and q['wire_records']==128 and q['timestamp_nonconverse_counterexamples']>0,'wrong oracle scope'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'missing cost observations'
    for r in metrics:
        q=r['result'];assert q['observed_reads']==1 and q['observed_writes']==34 and q['observed_flushes']==3 and q['snapshot_wire_bytes']==160 and q['clock_size']>0,'wrong measured cost'
    assert {c['profile'] for c in d['compiler_dependency_closures']}==PROFILES|{'learner'},'missing actual dependency closure'
    assert d['input_sha256'] and d['external_input_sha256'] and d['generated_artifacts'],'missing source/build evidence'
    for c in d['compiler_dependency_closures']:
        assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual compiler input'
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed','overclaimed evidence'
    commands=d['executions'];assert commands,'missing commands'
    builds=[r for r in commands if '--build' in r['command']];assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'failed/unobserved build'
    ctests=[r for r in commands if '--verbose' in r['command']];assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'failed/unobserved ctest'
    mutants=[r for r in commands if 'replicated-clock-mutant-' in r['command'][0]];assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants),'wrong mutant outcomes'
    learners=[r for r in commands if '-R' in r['command']];assert len(learners)==3 and all(r['exit_code'] not in [0,None] for r in learners),'wrong learner outcomes'
    allowed={id(r) for r in mutants+learners}
    assert all(r['exit_code']==0 or id(r) in allowed for r in commands),'unexplained failed command'
    if files:
        for p,h in d['input_sha256'].items():
            full=(ROOT/p).resolve();assert full.is_relative_to(ROOT.resolve()) and full.is_file() and sha(full)==h,'stale source '+p
        for p,h in d['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h,'changed external input '+p
        for r in d['generated_artifacts']:
            p=(ROOT/r['path']).resolve();assert p.is_relative_to(ROOT.resolve()) and sha(p)==r['sha256'],'changed build evidence'
        for r in commands:
            p=(ROOT/r['log']).resolve();assert p.is_relative_to(ROOT.resolve()) and sha(p)==r['log_sha256'],'changed raw log'
        for r in ctests:
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'CLOCK_CUTS 148' in out and '100% tests passed' in out,'missing native/fault observation'
            assert all('replicated.'+a in out for a in ANCHORS),'anchor absent from actual output'
        for r in mutants+learners:
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash/sanitizer is not intended rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),
        lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda x:x['learner_stub_rejections'].pop(),
        lambda x:x['observations'][0]['result'].update(reachability_pairs=1),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(input_sha256={}),
        lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),
        lambda x:next(r for r in x['executions'] if 'replicated-clock-mutant-' in r['command'][0]).update(exit_code=-11),
        lambda x:next(r for r in x['executions'] if '-R' in r['command']).update(exit_code=0), lambda x:next(r for r in x['observations'] if r['kind']=='metrics')['result'].update(observed_flushes=0)]
    for change in changes:
        altered=copy.deepcopy(d);change(altered)
        try:check(altered,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupted evidence')
    return len(changes)
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');args=ap.parse_args();d=json.loads((ROOT/REPORT).read_text(encoding='utf8'));check(d);n=selftest(d) if args.self_test else 0;print(json.dumps({'passed':True,'profiles':8,'negative_controls':n,'report_sha256':sha(ROOT/REPORT)}))
