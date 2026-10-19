"""Guard exact typed-format, partial-prefix, diagnosis and pipe evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_format_stream_validation.json'
COST='research/v5/modern/metaprog_format_investigation_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'format_parse',2:'format_atomic',3:'format_atomic',4:'format_stream',5:'format_stream',6:'format_stream',7:'format_base',8:'format_base'}
TRACE='9c770a92157e5382fb6ca11da0433114111c0189b82fed91ec10faefbf26eeeb'
def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['cases'],r['format_cases'],r['stream_cases'],r['accepted_formats'])==(12812,7240,5572,1527) and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'])==(10000,1120000) and min(r['program_size'],r['stream_size'])>0 and len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/format_stream/format_stream.c'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-format_stream-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==149 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    obs=q['observations'];assert len(obs)==13
    diagnostics=[r for r in obs if r['kind']=='category-diagnostic'];assert len(diagnostics)==4 and {(r['compiler'],r['case']) for r in diagnostics}=={(c,n) for c in ['gcc','clang'] for n in [1,2]} and all(r['rejected'] and r['phase']=='translation' for r in diagnostics)
    positives=[r for r in obs if r['kind']=='category-positive'];assert len(positives)==2 and {r['compiler'] for r in positives}=={'gcc','clang'} and all(r['executed'] and r['argument_evaluations']==1 for r in positives)
    pipes=[r for r in obs if r['kind']=='local-pipe'];assert len(pipes)==4 and {(r['compiler'],r['optimization']) for r in pipes}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']} and all(r['executed'] and r['backpressure'] and r['offsets']==[0,2,4,5] and r['calls_before_cancel']==4 and r['calls_after_cancel']==5 and r['descriptors_closed']==2 for r in pipes)
    languages=[r for r in obs if r['kind']=='language-comparison'];assert len(languages)==2 and {r['compiler'] for r in languages}=={'g++','clang++'} and all(r['mode']=='C++20' and r['literal_rejected'] and r['positive_executed'] and r['runtime_format_refused'] and r['rolling_draft_not_published_edition'] for r in languages)
    incremental=[r for r in obs if r['kind']=='incremental'];assert len(incremental)==1 and incremental[0]['unchanged_no_work'] and min(incremental[0]['clean_seconds'],incremental[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==31 and sum(r['expected_failure'] for r in q['executions'])==6 and all((r['exit_code'] not in [None,0]) if r['expected_failure'] else r['exit_code']==0 for r in q['executions'])
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r['expected_failure']:assert 'error:' in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MPO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-008.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/format_stream/FORMAT.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(stream_cases=0),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='local-pipe').update(offsets=[0,5]),lambda a,b:next(r for r in b['observations'] if r['kind']=='local-pipe').update(descriptors_closed=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='language-comparison').update(runtime_format_refused=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='category-positive').update(executed=False),lambda a,b:next(r for r in b['executions'] if r['expected_failure']).update(exit_code=0)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
