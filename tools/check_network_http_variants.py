"""Require current exact author observations; no family-pass mastery inference."""
import argparse,copy,json
from pathlib import Path
from common import ROOT,read_json,sha
from register_network_http_variants import BASE,IDS
REPORT=BASE+'/validation.json';EXPORT=BASE+'/export_validation.json';BASELINE=BASE+'/baseline_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
ORACLE=dict(passed=True,seed=16706,cases=dict(length=6318,date=6333,checksum=4096),trace_sha256='53d7e9cd38e5f7205f4e18aa6fc8abc954b1e473429a219c9e6c2783a3fb71c9')
def check(d,e,b,files=True):
    assert d['passed'] and d['source_stable'] and d['input_sha256'] and d['external_input_sha256'] and d['artifact_sha256']
    assert d['physical']=='not-run' and d['independent_assessment']=='not-run'
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert set(r['anchors'])=={'checksum','length','date','owned','framing','cost'}
        for case in r['anchors']:assert 'NV_OK:'+case in r['anchors'][case]
        assert 'checksum:7140:pseudoheader=38939' in r['anchors']['checksum']
        assert 'date:146097:' in r['anchors']['date'] and 'close-admit-200' in r['anchors']['owned']
        assert len([s for s in r['anchors']['cost'].splitlines() if s.startswith('NV_COST:')])==7
        assert r['oracle']==ORACLE and r['retained_parser_cases']==144 and r['retained_parser_partitions']>1000
        assert len(r['controls'])==8 and {q['mutation'] for q in r['controls']}==set(range(1,9))
    assert len(d['executions'])==611 and len(d['compiler_dependencies'])==370
    assert all(r['exit_code']==r['expected_exit'] and r['exit_code'] in [0,1] for r in d['executions'])
    kills=[r for r in d['executions'] if r['required_marker']=='NV_CHECK:'];assert len(kills)==56 and all(r['exit_code']==1 for r in kills)
    tsan=[r for r in d['executions'] if r['command'][0].endswith(('-gcc-tsan','-clang-tsan')) or Path(r['command'][0]).name in ['gcc-tsan','clang-tsan']]
    assert len(tsan)==2 and all(r['exit_code']==0 and r['required_marker']=='NV_OK:owned' for r in tsan)
    assert d['target']==dict(profile='Cortex-M4-O2',executed=False,section_gc=False,undefined=[])
    assert 'NV_AVR_PASS:' in d['avr'];avr=json.loads(d['avr'].splitlines()[0]);assert avr['size_t_bits']==16 and avr['wire_bits']==32 and len(avr['cases'])==7 and not avr['physical']
    assert 'fixtures/network_http/05_semantics.json' in d['input_sha256'] and 'fixtures/network_http/09_wire.json' in d['input_sha256']
    assert e['passed'] and e['source_stable'] and not e['private_sources_copied'] and e['input_sha256']
    assert len(e['exercises'])==5 and {r['id'] for r in e['exercises']}==set(IDS) and len(e['executions'])==40
    for r in e['exercises']:assert len(r['executions'])==8 and r['assets'] and not r['private_sources_copied']
    for r in e['executions']:assert (r['exit_code']==r['expected_exit']==0) or (r['expected_exit'] is None and r['exit_code'] not in [None,0])
    assert b['passed'] and b['source_stable'] and b['input_sha256'] and b['artifact_sha256']
    assert b['physical']=='not-run' and b['independent_assessment']=='not-run'
    assert len(b['profiles'])==6 and {r['profile'] for r in b['profiles']}==PROFILES
    for r in b['profiles']:
        assert r['anchors']==['length','date','lifecycle'] and len(r['controls'])==3
        assert r['framing']=='original CL6/zero-chunk5 and unchanged conflict output' and 'controlled fd reuse' in r['socket']
    assert len(b['executions'])==159 and len(b['compiler_dependencies'])==72
    assert all(r['exit_code']==r['expected_exit'] and r['exit_code'] in [0,1] for r in b['executions'])
    assert len([r for r in b['executions'] if r['exit_code']==1])==18 and 'HTTP_MODELS_PASS:' in b['models']
    if files:
        import sys
        sys.path.insert(0,str(ROOT))
        from cmastery import exercises,learning
        assert e.get('selected_catalog_sha256')=={ident:learning.digest(exercises.exercise(ident)) for ident in IDS}, 'current selected exercise contracts'
        for data in [d,e,b]:
            for p,h in data['input_sha256'].items():assert sha(ROOT/p)==h,'stale '+p
            for r in data['executions']:
                assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
                if r['required_marker']:assert r['required_marker'] in text
                if r in kills:assert not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for data in [d,b]:
            for p,h in data['external_input_sha256'].items():assert sha(Path(p))==h,'external changed '+p
            for p,h in data['artifact_sha256'].items():assert sha(ROOT/p)==h,'artifact changed '+p
        gate=read_json(ROOT/BASE/'research_gate.json');assert gate['status']=='passage-inspected'
        for p,h in gate['input_sha256'].items():assert sha(ROOT/p)==h,'stale reading '+p
        for r in e['exercises']:
            for p,h in r['assets'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
        assert sha(Path(e['checksum_composition']['output']))==e['checksum_sha256']
        for id in IDS:
            text=(ROOT/f'curriculum/v5/exercises/{id}.md').read_text(encoding='utf8');assert 'labs/network_http_variants/CONTRACTS.md' in text
    return True
def selftest(d,e,b):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a.update(source_stable=False),lambda a,b:a.update(input_sha256={}),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['anchors'].pop('framing'),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['profiles'][0]['oracle'].update(seed=0),lambda a,b:a['profiles'][0].update(retained_parser_cases=1),lambda a,b:a['profiles'][0]['anchors'].update(date='NV_OK:date:1'),lambda a,b:a['profiles'][0]['anchors'].update(cost='NV_OK:cost:'),lambda a,b:a['target'].update(executed=True),lambda a,b:a['target'].update(undefined=['memcpy']),lambda a,b:a.update(avr=''),lambda a,b:a['compiler_dependencies'].pop(),lambda a,b:a['executions'].pop(),lambda a,b:next(r for r in a['executions'] if r['required_marker']=='NV_CHECK:').update(exit_code=-11),lambda a,b:a.update(physical='passed'),lambda a,b:b['exercises'].pop(),lambda a,b:b['executions'].pop(),lambda a,b:b.update(private_sources_copied=True)]
    for change in changes:
        a,f=copy.deepcopy(d),copy.deepcopy(e);change(a,f)
        try:check(a,f,b,False)
        except (AssertionError,KeyError,ValueError,IndexError):continue
        raise AssertionError('accepted altered evidence')
    extra=[lambda c:c['profiles'].pop(),lambda c:c['executions'].pop(),lambda c:c.update(source_stable=False),lambda c:next(r for r in c['executions'] if r['exit_code']).update(exit_code=-11),lambda c:c.update(physical='passed')]
    for change in extra:
        c=copy.deepcopy(b);change(c)
        try:check(d,e,c,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted altered baseline')
    return len(changes)+len(extra)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();d,e,b=[read_json(ROOT/q) for q in [REPORT,EXPORT,BASELINE]];check(d,e,b);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e,b) if args.self_test else 0,current_reports={q:sha(ROOT/q) for q in [REPORT,EXPORT,BASELINE]})))
