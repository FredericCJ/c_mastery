"""Guard actual typed-record, diagnostic, cost and finite semantic evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_logging_validation.json'
COST='research/v5/modern/metaprog_logging_diagnostics_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'logging_eval',2:'logging_atomic',3:'logging_atomic',4:'logging_copy',5:'logging_copy',6:'logging_atomic',7:'logging_base',8:'logging_base'}
TRACE='7109f9b8364e82b81f7375b72a2e511485bdfb9a243b31e87b45f985a6d59108'
DIAGNOSTICS={'raw-number','wrong-struct','omitted-field','empty-field','five-fields','unprotected-comma'}
def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['cases'],r['accepted'],r['invalid'],r['full'],r['range'])==(5248,468,3201,684,895) and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'])==(10000,40000) and r['sink_size']>r['record_size']>r['field_size']>0
        assert all(len(r[k])==7 and all(t>=0 for t in r[k]) for k in ['macro_seconds','direct_seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/logging/logging.c','assessor/metaprog/logging/logging_macros.h'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-logging-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==149 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    obs=q['observations'];assert len(obs)==27
    def kind(k):return [r for r in obs if r['kind']==k]
    assert len(kind('typed-positive'))==2 and {r['compiler'] for r in kind('typed-positive')}=={'gcc','clang'}
    assert all(r['executed'] and r['arities']==[0,1,2,3,4] and r['protected_comma'] and r['numeric_conversion']==3 for r in kind('typed-positive'))
    assert len(kind('diagnostic'))==12 and {(r['compiler'],r['case']) for r in kind('diagnostic')}=={(c,k) for c in ['gcc','clang'] for k in DIAGNOSTICS} and all(r['rejected'] and r['stage']=='translation' for r in kind('diagnostic'))
    assert len(kind('numeric-warning-policy'))==2 and {r['compiler'] for r in kind('numeric-warning-policy')}=={'gcc','clang'} and all(r['executed'] and r['value']==3 and r['werror_rejected']==(r['compiler']=='clang') for r in kind('numeric-warning-policy'))
    assert len(kind('preprocess'))==2 and {r['compiler'] for r in kind('preprocess')}=={'gcc','clang'} and all(r['selected']==['mpl_log1','mpl_log2','mpl_log3','mpl_log4'] and min(r['bytes'],r['tokens'])>0 for r in kind('preprocess'))
    assert len(kind('object'))==4 and {(r['compiler'],r['optimization']) for r in kind('object')}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in kind('object'):assert set(r['helpers'])<={'memcpy','memset','__stack_chk_fail'} and set(r['functions'])=={'mpl_number','mpl_bytes','mpl_write',*('mpl_log'+str(n) for n in range(5))} and all(n>0 for n in r['functions'].values())
    assert len(kind('newer-mode'))==4 and {(r['compiler'],r['mode']) for r in kind('newer-mode')}=={('gcc','c2x'),('clang','c2x'),('g++','c++20'),('clang++','c++20')} and all(r['executed'] and (r['mode']!='c2x' or r['c17_rejected']) for r in kind('newer-mode'))
    assert len(kind('incremental'))==1 and kind('incremental')[0]['unchanged_no_work'] and min(kind('incremental')[0]['clean_seconds'],kind('incremental')[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==66 and sum(r['expected']=='diagnostic' for r in q['executions'])==15 and all(r['exit_code']==(1 if r['expected']=='diagnostic' else 0) for r in q['executions']) and len(q['generated_artifacts'])==11
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in q['generated_artifacts']:assert sha(ROOT/r['path'])==r['sha256']
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MPO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-004.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/logging/LOGGING.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(accepted=0),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['helpers'].append('hidden_logger'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='typed-positive').update(executed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='preprocess')['selected'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='newer-mode' and r['mode']=='c2x').update(c17_rejected=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='numeric-warning-policy').update(value=4),lambda a,b:next(r for r in b['executions'] if r['expected']=='diagnostic').update(exit_code=0)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
