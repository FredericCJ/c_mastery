"""Reject stale packets, missing costs and invented performance equivalence."""
import argparse
import copy
import json
from pathlib import Path
from common import ROOT,read_json,sha
from run_metaprog_measurement_closure import NAMES,CONTRACTS,DIMENSIONS,WORK,REPORT,derived

def check(d,files=True):
    assert d['passed'] and d['source_stable'] and d['external_inputs_stable'] and d['input_sha256'] and len(d['current_reports'])==20
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed'
    assert len(d['packets'])==10 and [p['name'] for p in d['packets']]==NAMES
    for i,(p,work,contract) in enumerate(zip(d['packets'],WORK,CONTRACTS),1):
        assert p['criteria']==[f'X-T68-{i:03}-R{j}' for j in range(1,5)] and len(p['reports'])==2 and p['dimensions']==DIMENSIONS and p['contract']=='labs/metaprog/'+p['name']+'/'+contract
        assert (p['workload'],p['series'],p['equivalent_groups'],p['equivalence_limit'])==work
        raw=p['raw_metrics'];assert len(raw)==6 and {r['profile'] for r in raw}=={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
        assert p['derived_metrics']==[dict(profile=r['profile'],series=derived(r['result'],p['series'])) for r in raw]
        for r in raw:
            assert r['result']['iterations']>0 and all(len(r['result'][s])==7 and all(t>=0 for t in r['result'][s]) for s in p['series'])
        assert p['build_observations'] and all(r['unchanged_no_work'] and min(r['clean_seconds'],r['unchanged_seconds'])>=0 for r in p['build_observations'])
    assert len(d['observations'])==10 and {r['name'] for r in d['observations']}==set(NAMES) and all(r['result']['passed'] for r in d['observations'])
    assert len(d['executions'])==10 and all(r['exit_code']==0 for r in d['executions'])
    if files:
        for p,h in d['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
        for p,h in d['current_reports'].items():assert sha(ROOT/p)==h
        for p,h in d['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in d['executions']:assert sha(ROOT/r['log'])==r['log_sha256']
        for p in d['packets']:
            r=read_json(ROOT/p['readiness']);assert all(x['author_readiness']=='ready-bounded' for x in r['criteria'])
            main=read_json(ROOT/p['reports'][0]);assert [x for x in main['observations'] if x['kind']=='metrics']==p['raw_metrics'] and any('stack' in x['path'] for x in main['generated_artifacts'])
    return True

def selftest(d):
    edits=[lambda x:x.update(passed=False),lambda x:x['packets'].pop(),lambda x:x['packets'][0]['criteria'].pop(),lambda x:x['observations'].pop(),lambda x:x.update(current_reports={}),lambda x:x.update(input_sha256={}),lambda x:x.update(learner_mastery='passed'),lambda x:x.update(physical_validation='passed'),lambda x:x['executions'][0].update(exit_code=1),lambda x:x['packets'][0]['build_observations'][0].update(unchanged_no_work=False),lambda x:x['packets'][0].update(name='invented'),lambda x:x['packets'][0]['dimensions'].pop(),lambda x:x['packets'][2].update(equivalent_groups=[WORK[2][1]]),lambda x:x['packets'][0]['derived_metrics'][0]['series']['typed_seconds'][0].update(iterations_per_cpu_second=1),lambda x:x['packets'][0]['raw_metrics'].pop()]
    for edit in edits:
        q=copy.deepcopy(d);edit(q)
        try:check(q,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incomplete aggregate')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read_json(ROOT/REPORT);check(d);n=selftest(d) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
