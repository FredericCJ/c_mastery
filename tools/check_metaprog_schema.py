"""Guard actual schema, changed-consumer and generated-input evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/metaprog_schema_validation.json'
COST='research/v5/modern/metaprog_schema_evolution_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'schema_semantics',2:'schema_wire',3:'schema_visit',4:'schema_wire',5:'schema_wire',6:'schema_semantics',7:'schema_base',8:'schema_base'}
TRACE='4f3898acec7ece4b1cf885ae4ffa0241efaf3363380656e12e17b1f3ac9bd428'

def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['cases'],r['encode_cases'],r['decode_cases'],r['visit_cases'],r['accepted'],r['stopped'])==(9811,6048,3523,240,1390,40) and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'],r['wire_bytes'])==(10000,1330000,19) and min(r['record_size'],r['descriptor_size'])>0 and len(r['seconds'])==7 and all(t>=0 for t in r['seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/schema/schema.c','assessor/metaprog/schema/schema_fields.h','assessor/api_depth/architecture/core.c','labs/metaprog/schema/schema_stamp.py'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for row in d['compiler_dependency_closures']:
        generated=[r for p,r in row['generated_link_inputs'].items() if p.endswith('/schema_stamp.h')];assert len(generated)==1
        g=generated[0];assert g['validation']=='actual-generator-check-passed' and g['producer_source']=='labs/metaprog/schema/schema_stamp.py' and g['schema_source']==('labs/metaprog/schema/src/schema_fields.h' if row['profile']=='learner' else 'assessor/metaprog/schema/schema_fields.h') and g['recipe'][-1]=='--check' and g['sha256']
        assert any(a['path']==g['archived_path'] and a['sha256']==g['sha256'] for a in d['generated_artifacts'])
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-schema-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']];checks=[r for r in rows if r['command'][-1]=='--check']
    assert len(rows)==168 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests) and len(checks)==7 and all(r['exit_code']==0 for r in checks)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    obs=q['observations'];assert len(obs)==21
    def kind(k):return [r for r in obs if r['kind']==k]
    assert len(kind('source-check'))==1 and kind('source-check')[0]['stale_rejected'] and kind('source-check')[0]['current_accepted'] and kind('source-check')[0]['all_input_bytes']
    assert len(kind('changed-schema'))==4 and {(r['compiler'],r['layout']) for r in kind('changed-schema')}=={(c,l) for c in ['gcc','clang'] for l in ['native','packed']}
    for r in kind('changed-schema'):assert r['executed'] and r['stale_consumer_compiled'] and r['wire']=='030401021234020489abcdef0305fffffffe010402000f' and r['quality_limit']==15 and r['visited']==4 and len(r['offsets'])==4
    assert len(kind('layout-comparison'))==2 and {r['compiler'] for r in kind('layout-comparison')}=={'gcc','clang'} and all(r['wire_unchanged'] and r['native_offsets_changed'] and r['extension']=='-fpack-struct=1' for r in kind('layout-comparison'))
    for k in ['stale-static','stale-link','current-link','preprocess']:assert len(kind(k))==2 and {r['compiler'] for r in kind(k)}=={'gcc','clang'}
    assert all(r['rejected'] and r['hash_words']==4 for r in kind('stale-static')) and all(r['rejected'] and not r['old_object_executed'] for r in kind('stale-link')) and all(r['executed'] for r in kind('current-link')) and all(min(r['bytes'],r['tokens'])>0 for r in kind('preprocess'))
    assert len(kind('object'))==4 and {(r['compiler'],r['optimization']) for r in kind('object')}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in kind('object'):assert {'ar_encode','ar_decode'}<=set(r['imports'])<={'ar_encode','ar_decode','__stack_chk_fail','memcpy','memset'}
    assert len(kind('language-comparison'))==1 and kind('language-comparison')[0]['executed'] and kind('language-comparison')[0]['language']=='Rust2021'
    assert len(kind('incremental'))==1 and kind('incremental')[0]['unchanged_no_work'] and min(kind('incremental')[0]['clean_seconds'],kind('incremental')[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==53 and sum(r['expected']=='diagnostic' for r in q['executions'])==5 and all(r['exit_code']==(1 if r['expected']=='diagnostic' else 0) for r in q['executions']) and len(q['generated_artifacts'])==8 and len({r['path'] for r in q['generated_artifacts']})==8
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
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-006.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/schema/SCHEMA.md' in m['shared_contracts'] and 'codec.records' in m['prerequisite_artifacts']
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(accepted=0),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['imports'].append('schema_library'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='changed-schema').update(visited=3),lambda a,b:next(r for r in b['observations'] if r['kind']=='stale-static').update(hash_words=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='stale-link').update(old_object_executed=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='language-comparison').update(executed=False),lambda a,b:next(r for r in b['executions'] if r['expected']=='diagnostic').update(exit_code=0),lambda a,b:next(r for r in a['compiler_dependency_closures'][0]['generated_link_inputs'].values() if r.get('producer_source')=='labs/metaprog/schema/schema_stamp.py').update(validation='assumed')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
