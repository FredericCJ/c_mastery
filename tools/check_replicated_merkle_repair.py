"""Reject stale/incomplete source-bound T76.07 author evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/replicated_merkle_repair_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']}
ANCHORS={'merkle_proof','merkle_wire','merkle_repair','merkle_cost'}
CONTROL={1:'merkle_proof',2:'merkle_proof',3:'merkle_proof',4:'merkle_proof',5:'merkle_wire',6:'merkle_wire',7:'merkle_repair',8:'merkle_repair',9:'merkle_cost',10:'merkle_cost'}

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
        q=r['result'];assert q['passed'] and q['proof_rows']==2311 and q['independently_verified_paths']==2284 and q['repair_rows']==2090 and q['successful_requests']==16224,'wrong oracle scope'
        assert q['repair_status_counts']=={'0':1698,'5':301,'3':91},'wrong boundary outcomes'
        assert q['proof_output_sha256']=='6a9b30d3171568241ddcc376b7c9c4db44a512cd2c1f0c61f626ad16dfb3264a' and q['repair_output_sha256']=='ac399d190ec06496a394c4bac1ae0dfbb195d5ca3886ffe55e974eb6f42ab246','changed independent trace'
    assert len(metrics)==8 and {r['profile'] for r in metrics}==PROFILES,'missing cost observations'
    for r in metrics:
        q=r['result'];assert all(q[k]>0 for k in ['tree_size','proof_size','cost_size']) and q['max_leaves']==16 and q['leaf_bytes']==32 and q['request_wire']==27 and q['response_wire']==108 and q['runs']==200,'wrong measured cost'
        assert (q['sparse_hashes'],q['full_hashes'],q['direct_hashes'])==(122,64,48) and (q['sparse_requests'],q['full_requests'],q['direct_requests'])==(5,31,16),'unequal cost experiment'
        assert all(q[k]>=0 for k in ['sparse_cpu_seconds','full_cpu_seconds','direct_cpu_seconds']),'invalid struct lr_cpu observation'
    assert {c['profile'] for c in d['compiler_dependency_closures']}==PROFILES|{'learner'},'missing actual dependency closure'
    assert d['input_sha256'] and d['external_input_sha256'] and d['generated_artifacts'],'missing source/build evidence'
    for c in d['compiler_dependency_closures']:
        assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs'],'unpinned actual compiler input'
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed','overclaimed evidence'
    commands=d['executions'];assert commands,'missing commands'
    builds=[r for r in commands if '--build' in r['command']];assert len(builds)==9 and all(r['exit_code']==0 for r in builds),'failed/unobserved build'
    ctests=[r for r in commands if '--verbose' in r['command']];assert len(ctests)==8 and all(r['exit_code']==0 for r in ctests),'failed/unobserved ctest'
    mutants=[r for r in commands if 'replicated-merkle-repair-mutant-' in r['command'][0]];assert len(mutants)==80 and all(r['exit_code']==1 for r in mutants),'wrong mutant outcomes'
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
            out=(ROOT/r['log']).read_text(encoding='utf8');assert all(m in out for m in ['MERKLE_PROOFS leaves136 index-count-path domains chunk-boundaries','MERKLE_MALFORMED reordered-siblings mismatched-count depth5 empty-proof unchanged-output','MERKLE_WIRE chunks108 truncations108 nonce padding missing-byte malformed-length oversized-reply','MERKLE_REPAIR failed-node-no-publication counts0through16','MERKLE_COST equal0 sparse5 full31 sparse-bytes675 full-bytes4185','MERKLE_DIRECT requests16 bytes2160','100% tests passed']),'missing native/fault observation'
            assert all('replicated.'+a in out for a in ANCHORS),'anchor absent from actual output'
        for r in mutants+learners:
            out=(ROOT/r['log']).read_text(encoding='utf8');assert 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']),'crash/sanitizer is not intended rejection'
    return True
def selftest(d):
    changes=[lambda x:x.update(passed=False),lambda x:x.update(source_stable=False),lambda x:x['profiles'].pop(),lambda x:x['profiles'].append(x['profiles'][0]),
        lambda x:x['profiles'][0]['controls'].pop(),lambda x:x['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda x:x['learner_stub_rejections'].pop(),
        lambda x:x['observations'][0]['result'].update(independently_verified_paths=1),lambda x:x['compiler_dependency_closures'].pop(),lambda x:x.update(input_sha256={}),
        lambda x:x.update(physical_validation='passed'),lambda x:next(r for r in x['executions'] if '--build' in r['command']).update(exit_code=1),
        lambda x:next(r for r in x['executions'] if 'replicated-merkle-repair-mutant-' in r['command'][0]).update(exit_code=-11),
        lambda x:next(r for r in x['executions'] if '-R' in r['command']).update(exit_code=0), lambda x:next(r for r in x['observations'] if r['kind']=='metrics')['result'].update(direct_requests=7)]
    for change in changes:
        altered=copy.deepcopy(d);change(altered)
        try:check(altered,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted corrupted evidence')
    return len(changes)
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');args=ap.parse_args();d=json.loads((ROOT/REPORT).read_text(encoding='utf8'));check(d);n=selftest(d) if args.self_test else 0;print(json.dumps({'passed':True,'profiles':8,'negative_controls':n,'report_sha256':sha(ROOT/REPORT)}))
