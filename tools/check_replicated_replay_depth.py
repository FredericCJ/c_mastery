"""Current-source and observed-outcome gate for required replay support."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/replicated_replay_depth_validation.json'
TARGET='research/v5/modern/replicated_replay_target_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
CONTROL={1:'replay_prediction',5:'replay_prediction',3:'replay_snapshot',6:'replay_snapshot',4:'replay_durable',7:'replay_durable',2:'replay_effects',8:'replay_effects',9:'replay_transport',10:'replay_transport',12:'replay_native',13:'replay_native'}
ANCHORS=set(CONTROL.values())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def bindings(d):
    for p,h in d['input_sha256'].items():
        path=(ROOT/p).resolve();assert path.is_relative_to(ROOT.resolve()) and path.is_file() and sha(path)==h,'stale source '+p
    for p,h in d['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h,'changed external input '+p
    for row in d['generated_artifacts']:
        path=(ROOT/row['path']).resolve();assert path.is_relative_to(ROOT.resolve()) and sha(path)==row['sha256'],'changed build artifact'
    for row in d['executions']:
        path=(ROOT/row['log']).resolve();assert path.is_relative_to(ROOT.resolve()) and sha(path)==row['log_sha256'],'changed execution log'
def base(d):
    assert d.get('passed') and d['status']=='passed' and d['source_stable'] and d['external_inputs_stable'],'failed or unstable'
    assert d['input_sha256'] and d['external_input_sha256'] and d['generated_artifacts'] and d['executions'],'missing identities'
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed','overclaimed result'
def check(d,target,files=True):
    base(d);base(target)
    assert len(d['profiles'])==8 and {x['profile'] for x in d['profiles']}==PROFILES,'missing/duplicate profile'
    wanted={f'replicated.mutation.{a}.{i}' for i,a in CONTROL.items()}
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS,'missing anchor'
        assert len(p['controls'])==12 and {c['test'] for c in p['controls']}==wanted and all(c['semantic_rejection'] for c in p['controls']),'missing semantic rejection'
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections']),'compiling learner rejection absent'
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracle)==8 and {r['profile'] for r in oracle}==PROFILES,'oracle profile missing'
    for r in oracle:
        q=r['result'];assert q['passed'] and q['ordinary_histories']==9841 and q['changed_order_histories']==768 and q['histories']==10609 and q['snapshots']==319681,'oracle scope changed'
        assert q['trace_sha256']=='20f8525742189477e32355a233f086126410bcc913130a972c9721bd727a88c9','independent trace mismatch'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'metrics missing'
    for r in metrics:
        q=r['result'];assert q['session_size']>0 and q['durable_size']>q['session_size'] and q['snapshot_bytes']==512 and q['input_bytes']==16 and q['framed_bytes']==18 and q['horizon']==8 and q['runs']==2000,'cost workload mismatch'
        assert len(q['cpu_seconds'])==7 and all(x>=0 for x in q['cpu_seconds']),'cost samples absent'
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'},'actual dependency closure absent'
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual input'
    commands=d['executions'];builds=[r for r in commands if '--build' in r['command']];ctests=[r for r in commands if '--verbose' in r['command']]
    mutants=[r for r in commands if 'replicated-replay-depth-mutant-' in r['command'][0]];learners=[r for r in commands if '-R' in r['command']]
    assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'unobserved compile'
    assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'unobserved ctest'
    assert len(mutants)==96 and all(r['exit_code']==1 for r in mutants),'mutant outcome mismatch'
    assert len(learners)==6 and all(r['exit_code'] not in [None,0] for r in learners),'learner outcome mismatch'
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in commands),'unexplained failed command'
    assert len(target['observations'])==3 and {r['mode'] for r in target['observations']}=={'O0','O3','wrong-endian'},'target profiles absent'
    for r in target['observations']:assert r['passed'] and r['traces']==128 and r['semantic_rejection']==(r['mode']=='wrong-endian'),'target scope mismatch'
    assert len(target['purity_audits'])==2 and {r['optimization'] for r in target['purity_audits']}=={'-O0','-O3'} and target['hidden_time_rejected'],'purity controls absent'
    for r in target['purity_audits']:assert r['passed'] and set(r['symbols'])<=set(r['allowed']) and 'time' not in r['allowed'],'purity allowlist mismatch'
    failed=[r for r in target['executions'] if r['exit_code']];assert len(failed)==1 and failed[0]['exit_code']==1 and failed[0]['expected_exit']==1 and 'wrong-endian' in failed[0]['command'][-1],'target control outcome mismatch'
    if files:
        bindings(d);bindings(target)
        for row in ctests:
            text=(ROOT/row['log']).read_text(encoding='utf8')
            for marker in ['REPLAY_PREDICTION suffix-replacement','REPLAY_SNAPSHOT bits4096','REPLAY_DURABLE cuts296','REPLAY_EFFECTS confirmed-only','REPLAY_TRANSPORT actual-parser','REPLAY_COUNTEREXAMPLES hidden-random','REPLAY_NATIVE local-two-player','100% tests passed']:assert marker in text,'unobserved '+marker
            assert all('replicated.'+a in text for a in ANCHORS),'anchor not executed'
        for row in mutants+learners:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert 'RP_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash is not semantic rejection'
        for row in target['executions']:
            text=(ROOT/row['log']).read_text(encoding='utf8');cmd=row['command']
            if 'qemu-ppc64' in cmd[0] and '--version' not in cmd:
                assert ('RP_CHECK big-endian canonical-state' if row['exit_code'] else 'REPLAY_BE_PASS traces128 canonical-full-state') in text,'target result missing'
            if cmd[0]=='readelf' and '-h' in cmd:assert all(s in text for s in ['big endian','PowerPC64','abiv2']),'target identity missing'
        body=(ROOT/'curriculum/v5/exercises/X-T76-009.md').read_text(encoding='utf8');m=json.loads(body.split('```json\n',1)[1].split('\n```',1)[0]);assert {'replicated.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/replicated/replay_depth/REPLAY.md' in m['shared_contracts'],'canonical exercise disconnected'
    return True
def selftest(d,t):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a.update(source_stable=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(snapshots=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(physical_validation='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if '--build' in r['command']).update(exit_code=1),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(hidden_time_rejected=False),lambda a,b:b['purity_audits'][0]['allowed'].append('time'),lambda a,b:next(r for r in b['executions'] if r['exit_code']==1).update(exit_code=0)]
    for change in changes:
        a,b=copy.deepcopy(d),copy.deepcopy(t);change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted altered evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());t=json.loads((ROOT/TARGET).read_text());check(d,t);n=selftest(d,t) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=8,target_profiles=3,negative_controls=n,report_sha256=sha(ROOT/REPORT),target_sha256=sha(ROOT/TARGET))))
