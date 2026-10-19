"""Reject stale/incomplete source-bound T76.04 author evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/replicated_membership_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
ANCHORS={'membership_join','membership_wire','membership_auth','membership_replay'}
CONTROL={1:'membership_join',2:'membership_join',3:'membership_wire',4:'membership_auth',5:'membership_auth',6:'membership_auth',7:'membership_replay',8:'membership_replay',9:'membership_wire'}
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def check(d,files=True):
    assert d.get('passed') and d.get('status')=='passed' and d.get('source_stable') and d.get('external_inputs_stable'),'failed/unstable report'
    assert {p['profile'] for p in d['profiles']}==PROFILES and len(d['profiles'])==8,'missing/duplicate profiles'
    wanted={f'replicated.mutation.{a}.{i}' for i,a in CONTROL.items()}
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS,'missing anchor'
        assert len(p['controls'])==9 and {c['test'] for c in p['controls']}==wanted and all(c['semantic_rejection'] for c in p['controls']),'unobserved semantic control'
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections']),'missing intended compiling learner rejection'
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle']
    metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracle)==8 and {r['profile'] for r in oracle}==PROFILES,'missing oracle profile'
    for r in oracle:
        q=r['result'];assert q['passed'] and q['histories']==128 and q['transition_rows']==12416 and q['canonical_snapshots']==12416 and q['mac_packets']==12416 and q['model_law_cases']==4368,'wrong oracle scope'
        assert q['status_counts']=={'0':8095,'3':2737,'4':841,'2':615},'wrong boundary outcomes'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'missing cost observations'
    for r in metrics:
        q=r['result'];assert q['counter_size']>0 and q['permit_size']>0 and q['capacity']==6 and q['snapshot_bytes']==160 and q['packet_bytes']==64,'wrong measured cost'
    assert {c['profile'] for c in d['compiler_dependency_closures']}==PROFILES|{'learner'},'missing actual dependency closure'
    assert d['input_sha256'] and d['external_input_sha256'] and d['generated_artifacts'],'missing source/build evidence'
    for c in d['compiler_dependency_closures']:
        assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual compiler input'
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed','overclaimed evidence'
    commands=d['executions'];assert commands,'missing commands'
    builds=[r for r in commands if '--build' in r['command']];assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'failed/unobserved build'
    ctests=[r for r in commands if '--verbose' in r['command']];assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'failed/unobserved ctest'
    mutants=[r for r in commands if 'replicated-membership-mutant-' in r['command'][0]];assert len(mutants)==72 and all(r['exit_code']==1 for r in mutants),'wrong mutant outcomes'
    learners=[r for r in commands if '-R' in r['command']];assert len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners),'wrong learner outcomes'
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
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'MEMBERSHIP_BITS 1280' in out and 'MEMBERSHIP_LAWS 4368' in out and 'MEMBERSHIP_AUTH forged-other=refused valid-high=policy-dependent' in out and '100% tests passed' in out,'missing native/fault observation'
            assert all('replicated.'+a in out for a in ANCHORS),'anchor absent from actual output'
        for r in mutants+learners:
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash/sanitizer is not intended rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),
        lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda x:x['learner_stub_rejections'].pop(),
        lambda x:x['observations'][0]['result'].update(mac_packets=1),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(input_sha256={}),
        lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),
        lambda x:next(r for r in x['executions'] if 'replicated-membership-mutant-' in r['command'][0]).update(exit_code=-11),
        lambda x:next(r for r in x['executions'] if '-R' in r['command']).update(exit_code=0), lambda x:next(r for r in x['observations'] if r['kind']=='metrics')['result'].update(capacity=7)]
    for change in changes:
        altered=copy.deepcopy(d);change(altered)
        try:check(altered,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupted evidence')
    return len(changes)
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');args=ap.parse_args();d=json.loads((ROOT/REPORT).read_text(encoding='utf8'));check(d);n=selftest(d) if args.self_test else 0;print(json.dumps({'passed':True,'profiles':8,'negative_controls':n,'report_sha256':sha(ROOT/REPORT)}))
