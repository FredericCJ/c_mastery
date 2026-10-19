"""Reject stale, incomplete or overclaimed storage successor campaign evidence."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/codec_validation.json'
EXPORT='research/v5/expansion/legacy_storage_followthrough/codec_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'variable',2:'migration',3:'atomic',4:'migration',5:'fresh'}
ANCHORS={'original','migration','variable','atomic','fresh','reopen'}
TRACE='b108f33d17d82750cff2e5fa45ab61724b599dcdecf8c688c0527f83f8c27d5c'
def check(d,files=True):
    base(d)
    assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==5
        assert {r['test'] for r in p['controls']}=={f'storage_followthrough.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    assert len(d['observations'])==6 and {r['profile'] for r in d['observations']}==PROFILES
    for row in d['observations']:
        r=row['result'];assert row['kind']=='independent-oracle' and r['passed'] and (r['layout_pages'],r['extent_cases'],r['trace_sha256'])==(4,1024,TRACE)
    needed={'assessor/storage/store.c','labs/storage/include/store.h','labs/storage/FORMAT.md','assessor/storage_followthrough/codec/codec.c','labs/storage_followthrough/codec/codec.h','labs/storage_followthrough/codec/CONTRACTS.md','labs/storage_followthrough/codec/tests.c','tools/verify_storage_codec.py'};assert needed<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'storage-codec-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==115 and len(mutants)==30 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    if files:
        bindings(d)
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'SFC_CHECK' in text and not any(s in text for s in ['AddressSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and 'SFC fresh validated cuts=38 reads=5' in text
            assert all('SFC '+a+' validated' in text for a in ANCHORS)
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        spec=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T47-01.json').read_text());assert {c['id'] for c in spec['criteria']}=={'AX-PR-T47-01-T47.01.'+x for x in 'abc'}
        for r in json.loads((ROOT/'research/v5/expansion/legacy_storage_followthrough/codec_resources.json').read_text())['resources']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
    return True
def selftest(d):
    edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(extent_cases=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a['observations'].pop(),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:a['observations'][0]['result'].update(layout_pages=0),lambda a:a['profiles'][0].update(anchors=['fresh']),lambda a:a.update(external_inputs_stable=False),lambda a:a['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda a:a.update(independent_review='passed')]
    for edit in edits:
        a=copy.deepcopy(d);edit(a)
        try:check(a,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted unsupported evidence')
    return len(edits)
def check_export(e,files=True):
    assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied']
    assert e['input_sha256'] and e['exported_artifacts'] and e['explicit_actual_provider']=='assessor/storage/store.c'
    assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
    assert len(e['executions'])==8
    failed=[r for r in e['executions'] if r['exit_code']!=0]
    assert len(failed)==2 and {r['required_marker'] for r in failed}=={'SFC_CHECK','Supply the completed storage provider'}
    assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [0,None] for r in e['executions'])
    if files:
        for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h
        for p,h in e['exported_artifacts'].items():assert p in e['input_sha256'] and h==e['input_sha256'][p] and 'assessor' not in p.split('/')
        for r in e['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256']
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
    return True
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d);check_export(e);n=selftest(d) if a.self_test else 0
    if a.self_test:
        for field,value in [('private_sources_copied',True),('source_stable',False),('exported_artifacts',{}),('physical_validation','passed')]:
            bad=copy.deepcopy(e);bad[field]=value
            try:check_export(bad,False)
            except (AssertionError,KeyError):n+=1;continue
            raise AssertionError('accepted unsupported export')
    print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT),export_sha256=sha(ROOT/EXPORT))))
