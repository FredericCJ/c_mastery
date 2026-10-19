"""Reject stale, conflated or incomplete T76 author-readiness evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/v5/modern/'
READINESS=PREFIX+'replicated_objective_readiness.json'

def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def pinned(values):
    assert values
    for p,h in values.items():
        path=(ROOT/p).resolve();assert path.is_relative_to(ROOT.resolve()) and path.is_file() and sha(p)==h,'stale '+p

def build_check(d,files=True):
    assert d['passed'] and d['status']=='passed' and d['source_stable'] and d['external_inputs_stable']
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed'
    modules={'ordinary_depth','followthrough','outbox','clocks','hashing','membership','deltas','sequence_depth','merkle_repair','wire_depth','replay_depth'}
    expected={(c,m) for c in ['gcc','clang'] for m in modules}
    assert len(d['measurements'])==22 and {(r['compiler'],r['module']) for r in d['measurements']}==expected
    for r in d['measurements']:
        assert r['copies_only'] and r['build_rows']>0 and r['changed_header']=='labs/replicated/include/replicated.h'
        assert all(r[k]>=0 for k in ['clean_seconds','unchanged_seconds','header_change_seconds'])
    rows=d['executions'];assert len(rows)==92 and all(r['exit_code']==0 for r in rows)
    assert sum('-S' in r['command'] for r in rows)==22
    assert sum('--build' in r['command'] for r in rows)==66
    if files:
        pinned(d['input_sha256']);pinned(d['current_reports'])
        for p,h in d['external_input_sha256'].items():assert Path(p).is_file() and hashlib.sha256(Path(p).read_bytes()).hexdigest()==h
        for r in rows:
            assert sha(r['log'])==r['log_sha256']
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if '-unchanged.log' in r['log']:assert 'no work to do' in text.lower()
            if '-header-change.log' in r['log']:assert 'Building C object' in text and 'Linking C executable' in text
    return True

def check(d,updates,cost,build,files=True):
    build_check(build,files)
    expected={f'X-T76-{i:03d}-R{j}' for i in range(1,10) for j in range(1,5)}
    assert len(d['criteria'])==36 and {r['id'] for r in d['criteria']}==expected
    assert len(d['variants'])==9 and {r['id'] for r in d['variants']}=={f'X-T76-{i:03d}-DELAYED' for i in range(1,10)}
    assert len(d['current_reports'])==13 and len(d['guard_observations'])==11
    assert all(r['exit_code']==0 and r['result']['passed'] and r['result']['negative_controls']>=13 for r in d['guard_observations'])
    for r in d['criteria']:
        assert r['author_readiness']=='ready-bounded' and len(r['demonstration'])>160
        assert r['independent_learner']=='not-assessed' and r['external_review']=='not-run' and r['physical_evidence']=='not-claimed'
        assert r['teaching'] and r['implementation_and_observation'] and r['anchors'] and r['limits']
        if files:pinned(r['pins'])
    for r in d['variants']:
        assert r['author_support']=='implemented-and-author-validated' and r['independent_learner']=='not-assessed'
        if files:pinned(r['pins'])
    wanted=expected|{f'X-T76-{i:03d}' for i in range(1,10)}|{f'X-T76-{i:03d}-DELAYED' for i in range(1,10)}|{f'G-T76-{i:02d}-{s}' for i in range(1,10) for s in ['DELAYED','INVESTIGATION']}|{'G-T76-COST-MEASUREMENTS'}
    assert len(updates['updates'])==73 and {u['id'] for u in updates['updates']}==wanted
    for row in updates['updates']:
        dims=row['dimensions'];assert dims['teaching']['state']=='complete' and dims['implementation']['state']=='complete' and dims['author_validation']['state']=='passed'
        assert dims['external_review']['state']=='not-run' and dims['independent_learner']['state']=='not-assessed' and dims['physical_observation']['state']=='not-applicable-host-contract'
    assert len(cost['mechanisms'])==9 and {r['exercise'] for r in cost['mechanisms']}=={f'X-T76-{i:03d}' for i in range(1,10)}
    dimensions={'Representation','Reachable storage','Lifetime','Mutation/aliasing','Allocation','Movement/copying','Destruction','Dispatch','Concurrency','Failure','Compilation','Binary/stack','Boundary'}
    for row in cost['mechanisms']:
        assert len(row['dimensions'])==13 and {d['dimension'] for d in row['dimensions']}==dimensions
        assert all(len(d['analysis'])>75 for d in row['dimensions']) and len(row['build_observations'])==2
    if files:
        pinned(d['input_sha256']);pinned(d['current_reports']);pinned(updates['input_sha256']);pinned(cost['current_reports'])
        gaps=read(PREFIX+'replicated_gap_ledger.json')
        assert not gaps['comprehensive_complete'] and gaps['author_implementation_complete']
        assert {g['id'] for g in gaps['gaps'] if g['status']=='unavailable'}=={'G-T76-C17-SOURCE','G-T76-C23-SOURCE'}
        assert len(gaps['gaps'])==21
    return True

def selftest(d,u,c,b):
    changes=[lambda a,z,c,b:a['criteria'].pop(),lambda a,z,c,b:a['criteria'].append(a['criteria'][0]),lambda a,z,c,b:a['criteria'][0].update(independent_learner='passed'),lambda a,z,c,b:a['criteria'][0].update(physical_evidence='passed'),lambda a,z,c,b:a['variants'].pop(),lambda a,z,c,b:a['current_reports'].pop(next(iter(a['current_reports']))),lambda a,z,c,b:a['guard_observations'][0].update(exit_code=1),lambda a,z,c,b:z['updates'].pop(),lambda a,z,c,b:z['updates'][0]['dimensions']['implementation'].update(state='planned'),lambda a,z,c,b:c['mechanisms'][0]['dimensions'].pop(),lambda a,z,c,b:b['measurements'].pop(),lambda a,z,c,b:b['measurements'][0].update(copies_only=False),lambda a,z,c,b:b['executions'][4].update(exit_code=1),lambda a,z,c,b:b.update(physical_validation='passed')]
    for change in changes:
        values=copy.deepcopy((d,u,c,b));change(*values)
        try:check(*values,files=False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted invalid closure evidence')
    return len(changes)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args()
    values=(read(READINESS),read(PREFIX+'replicated_completion_dispositions.json'),read(PREFIX+'replicated_cost_matrix.json'),read(PREFIX+'replicated_build_cost_validation.json'))
    check(*values);n=selftest(*values) if args.self_test else 0
    print(json.dumps(dict(passed=True,criteria=36,updates=73,cost_dimensions=117,negative_controls=n,readiness_sha256=sha(READINESS))))
