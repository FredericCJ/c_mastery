"""Reject stale/incomplete source-bound T76.05 author evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/replicated_deltas_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
ANCHORS={'delta_sets','delta_wire','delta_durable','delta_protocol'}
CONTROL={1:'delta_sets',2:'delta_sets',3:'delta_wire',4:'delta_durable',5:'delta_protocol',6:'delta_durable',7:'delta_durable',8:'delta_protocol',9:'delta_protocol',10:'delta_wire'}
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
        q=r['result'];assert q['passed'] and q['histories']==32 and q['transition_rows']==3104 and q['snapshot_comparisons']==9312 and q['causal_base_checks']==413,'wrong oracle scope'
        assert q['status_counts']=={'0':2546,'9':546,'2':5,'11':7},'wrong boundary outcomes'
        assert q['output_sha256']=='413f19deb061a99aac52aba516661775c0b5478858614332edd40207a6d43c4d','changed independent trace'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'missing cost observations'
    for r in metrics:
        q=r['result'];assert all(q[k]>0 for k in ['set_size','state_size','replica_size','packet_size','ack_size']) and q['history_capacity']==4 and q['tag_capacity']==24 and q['snapshot_bytes']==128,'wrong measured cost'
    assert {c['profile'] for c in d['compiler_dependency_closures']}==PROFILES|{'learner'},'missing actual dependency closure'
    assert d['input_sha256'] and d['external_input_sha256'] and d['generated_artifacts'],'missing source/build evidence'
    for c in d['compiler_dependency_closures']:
        assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual compiler input'
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed','overclaimed evidence'
    commands=d['executions'];assert commands,'missing commands'
    builds=[r for r in commands if '--build' in r['command']];assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'failed/unobserved build'
    ctests=[r for r in commands if '--verbose' in r['command']];assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'failed/unobserved ctest'
    mutants=[r for r in commands if 'replicated-delta-mutant-' in r['command'][0]];assert len(mutants)==80 and all(r['exit_code']==1 for r in mutants),'wrong mutant outcomes'
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
            out=(ROOT/r['log']).read_text(encoding='utf8');assert all(m in out for m in ['DELTA_BITS 1024','DELTA_CUTS 222','DELTA_LIMITS sequence-last epoch-last stale-authority unchanged-refusal','DELTA_TOMBSTONE delayed-old-add-resurrects-after-unsafe-drop=1','DELTA_PROTOCOL third-replica lost-ack fallback stale-session causal-refusal','100% tests passed']),'missing native/fault observation'
            assert all('replicated.'+a in out for a in ANCHORS),'anchor absent from actual output'
        for r in mutants+learners:
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash/sanitizer is not intended rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),
        lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda x:x['learner_stub_rejections'].pop(),
        lambda x:x['observations'][0]['result'].update(causal_base_checks=1),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(input_sha256={}),
        lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),
        lambda x:next(r for r in x['executions'] if 'replicated-delta-mutant-' in r['command'][0]).update(exit_code=-11),
        lambda x:next(r for r in x['executions'] if '-R' in r['command']).update(exit_code=0), lambda x:next(r for r in x['observations'] if r['kind']=='metrics')['result'].update(history_capacity=7)]
    for change in changes:
        altered=copy.deepcopy(d);change(altered)
        try:check(altered,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupted evidence')
    return len(changes)
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');args=ap.parse_args();d=json.loads((ROOT/REPORT).read_text(encoding='utf8'));check(d);n=selftest(d) if args.self_test else 0;print(json.dumps({'passed':True,'profiles':8,'negative_controls':n,'report_sha256':sha(ROOT/REPORT)}))
