"""Reject a T67 aggregate inferred from headings, stale packets or absent samples."""
import argparse
import copy
import json
from pathlib import Path
from common import ROOT,read_json,sha
from run_adt_measurement_closure import NAMES,REPORT
def check(d,files=True):
    assert d['passed'] and d['source_stable'] and d['external_inputs_stable'] and d['input_sha256'] and len(d['current_reports'])==20
    assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed'
    assert len(d['packets'])==10 and {p['name'] for p in d['packets']}==set(NAMES)
    for i,p in enumerate(d['packets'],1):assert p['criteria']==[f'X-T67-{i:03}-R{j}' for j in range(1,5)] and len(p['reports'])==2
    guards=[r for r in d['observations'] if r['kind']=='current-guard'];builds=[r for r in d['observations'] if r['kind']=='incremental']
    assert len(guards)==10 and {r['name'] for r in guards}==set(NAMES) and all(r['result']['passed'] for r in guards)
    assert len(builds)==4 and {r['name'] for r in builds}==set(NAMES[:4]) and all(r['unchanged_no_work'] and min(r['clean_seconds'],r['unchanged_seconds'])>=0 for r in builds)
    assert len(d['executions'])==25 and all(r['exit_code']==0 for r in d['executions'])
    if files:
        for p,h in d['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
        for p,h in d['current_reports'].items():assert sha(ROOT/p)==h
        for p,h in d['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in d['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for p in d['packets']:
            r=read_json(ROOT/p['readiness']);assert all(x['author_readiness']=='ready-bounded' for x in r['criteria'])
            main=read_json(ROOT/p['reports'][0]);metrics=[x for x in main['observations'] if x['kind']=='metrics'];assert len(metrics)>=6
            assert any('stack' in x['path'] for x in main['generated_artifacts'])
    return True
def selftest(d):
    edits=[lambda x:x.update(passed=False),lambda x:x['packets'].pop(),lambda x:x['packets'][0]['criteria'].pop(),lambda x:x['observations'].pop(),lambda x:x.update(current_reports={}),lambda x:x.update(input_sha256={}),lambda x:x.update(learner_mastery='passed'),lambda x:x.update(physical_validation='passed'),lambda x:x['executions'][0].update(exit_code=1),lambda x:next(r for r in x['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda x:x['packets'][0].update(name='invented')]
    for edit in edits:
        q=copy.deepcopy(d);edit(q)
        try:check(q,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incomplete aggregate')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read_json(ROOT/REPORT);check(d);n=selftest(d) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
