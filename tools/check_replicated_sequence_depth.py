"""Reject stale/incomplete source-bound T76.06 author evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/replicated_sequence_depth_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
ANCHORS={'seq_tree','seq_snapshot','ot_context','ot_delivery'}
CONTROL={1:'seq_tree',2:'seq_tree',3:'seq_snapshot',4:'seq_snapshot',5:'ot_context',6:'ot_context',7:'ot_delivery',8:'ot_delivery',9:'ot_context',10:'ot_delivery'}

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def check(d,files=True):
    assert d.get('passed') and d.get('status')=='passed' and d.get('source_stable') and d.get('external_inputs_stable'),'failed/unstable report'
    assert {p['profile'] for p in d['profiles']}==PROFILES and len(d['profiles'])==8,'missing/duplicate profiles'
    wanted={f'replicated.mutation.{a}.{i}' for i,a in CONTROL.items()}
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS,'missing anchor'
        assert len(p['controls'])==10 and {c['test'] for c in p['controls']}==wanted and all(c['semantic_rejection'] for c in p['controls']),'unobserved semantic control'
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections']),'missing intended compiling learner rejection'
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle']
    metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracle)==8 and {r['profile'] for r in oracle}==PROFILES,'missing oracle profile'
    for r in oracle:
        q=r['result'];assert q['passed'] and q['tree_histories']==96 and q['tree_rows']==9312 and q['ot_histories']==96 and q['ot_rows']==15456 and q['histories_with_three_operations']==96,'wrong oracle scope'
        assert q['tree_status_counts']=={'0':4147,'3':2342,'1':1445,'2':1282} and q['ot_status_counts']=={'0':10137,'7':5035,'4':188},'wrong boundary outcomes'
        assert q['tree_output_sha256']=='a74778ca189b1c6afc293bdf8da911117c15ea6e4bf4e6869c0c94ad05094de5' and q['ot_output_sha256']=='7bdaaa0190499ba2967fb966c163b9a6ad041d8a37b0f648b988a61d8a05f158','changed independent trace'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'missing cost observations'
    for r in metrics:
        q=r['result'];assert all(q[k]>0 for k in ['tree_size','server_size','client_size']) and q['node_capacity']==16 and q['log_capacity']==16 and q['snapshot_bytes']==368,'wrong measured cost'
    assert {c['profile'] for c in d['compiler_dependency_closures']}==PROFILES|{'learner'},'missing actual dependency closure'
    assert d['input_sha256'] and d['external_input_sha256'] and d['generated_artifacts'],'missing source/build evidence'
    for c in d['compiler_dependency_closures']:
        assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual compiler input'
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed','overclaimed evidence'
    commands=d['executions'];assert commands,'missing commands'
    builds=[r for r in commands if '--build' in r['command']];assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'failed/unobserved build'
    ctests=[r for r in commands if '--verbose' in r['command']];assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'failed/unobserved ctest'
    mutants=[r for r in commands if 'replicated-sequence-depth-mutant-' in r['command'][0]];assert len(mutants)==80 and all(r['exit_code']==1 for r in mutants),'wrong mutant outcomes'
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
            out=(ROOT/r['log']).read_text(encoding='utf8');assert all(m in out for m in ['SEQUENCE_TREE early-delete unicode queue-versus-placeholder','SEQUENCE_SNAPSHOT bits2944 quiescent-fence retained-descendants','OT_CONTEXT exact-prefix vectors retained-transforms duplicate-delete log-full','OT_COUNTEREXAMPLE retained-context=axb blind-original-context=abx','OT_DELIVERY gap duplicate-conflict pending-through-other-commit','100% tests passed']),'missing native/fault observation'
            assert all('replicated.'+a in out for a in ANCHORS),'anchor absent from actual output'
        for r in mutants+learners:
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash/sanitizer is not intended rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),
        lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda x:x['learner_stub_rejections'].pop(),
        lambda x:x['observations'][0]['result'].update(histories_with_three_operations=1),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(input_sha256={}),
        lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),
        lambda x:next(r for r in x['executions'] if 'replicated-sequence-depth-mutant-' in r['command'][0]).update(exit_code=-11),
        lambda x:next(r for r in x['executions'] if '-R' in r['command']).update(exit_code=0), lambda x:next(r for r in x['observations'] if r['kind']=='metrics')['result'].update(log_capacity=7)]
    for change in changes:
        altered=copy.deepcopy(d);change(altered)
        try:check(altered,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupted evidence')
    return len(changes)
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');args=ap.parse_args();d=json.loads((ROOT/REPORT).read_text(encoding='utf8'));check(d);n=selftest(d) if args.self_test else 0;print(json.dumps({'passed':True,'profiles':8,'negative_controls':n,'report_sha256':sha(ROOT/REPORT)}))
