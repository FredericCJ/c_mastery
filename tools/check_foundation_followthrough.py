"""Current-source gate for exact original B01 completion support."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
from check_expansion_campaigns import foundation_transfer_errors
from register_foundation_followthrough import BASE,IDS
REPORT=BASE+'/validation.json';TRANSFER='reviews/v5/expansion/b01_followthrough/foundation_transfer_successor.json';EXPORT=BASE+'/export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'interval','copy','codec','partition','lifecycle'}

def check(d,t,e,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert set(r['anchors'])==ANCHORS and all('FF_OK:'+a+':' in r['anchors'][a] for a in ANCHORS)
        assert len(r['controls'])==9 and {str(q['mutation']) for q in r['controls']}=={'1','2','3','4','premature','failed-destructor','generation-wrap','FD_MUTATE_DEFENSE','FD_MUTATE_FLAG_AUTHORITY'} and all(q['semantic_rejection'] for q in r['controls'])
        assert r['oracle']==dict(passed=True,histories=32769,transitions=163873,trace_sha256='06f8bb3560c3aa360fda1ba399b8451102208ce72017f0a7b87d77ae22aaebe9')
        assert r['defense_oracle']==dict(passed=True,cases=2474,structured=1440,random_cases=1024,seed=721205,trace_sha256='0202a961f3cb2940a6b1b41c5f30700ecf91e0028ed255ef6f911ca751b5e056')
        assert set(r['original_anchors'])=={'semantic','extent','cursor','codec','lifecycle','overlap','handles','defense'}
        assert r['original_oracle']['c_execution']=='passed' and r['original_oracle']['differential_cases']==2048
    assert len(d['comparisons'])==6 and {r['profile'] for r in d['comparisons']}=={a+'-'+b for a in ['gcc','clang'] for b in ['O0','O2','O3']}
    for r in d['comparisons']:assert len(r['samples'])==20 and 'FD_COMPARE_OK:65542' in r['result'] and 'edition=201710' in r['result']
    assert len(d['diagnostics'])==4 and {(r['compiler'],r['case']) for r in d['diagnostics']}=={(c,n) for c in ['gcc','clang'] for n in [1,2]}
    for r in d['diagnostics']:assert r['diagnostic'] and not r['executed'] and r['continued_translation']==(r['case']==1)
    assert len(d['targets'])==3 and {r['profile'] for r in d['targets']}=={'ppc64-O0','ppc64-O2','cortex-m4-O2'}
    for r in d['targets']:
        assert not r['undefined'] and not r['needed']
        if r['profile'].startswith('ppc64'):assert r['executed'] and 'FF_BE_PASS' in r['observation'] and 'wire=01030102fffffffe' in r['observation']
        else:assert not r['executed'] and r['no_section_gc']
    assert len(d['compiler_dependencies'])==246 and len(d['executions'])==657
    for r in d['executions']:assert r['exit_code']==r['expected_exit'] and r['exit_code'] in [0,1]
    kills=[r for r in d['executions'] if r['required_marker']=='FF_CHECK:'];assert len(kills)==42 and all(r['exit_code']==1 for r in kills)
    assert len([r for r in d['executions'] if r['required_marker']=='FF_OK:roundtrip:256' and r['exit_code']==0])==6
    assert len([r for r in d['executions'] if r['required_marker']=='AssertionError:' and r['exit_code']==1])==18
    assert len([r for r in d['executions'] if r['command'][0]=='ldd' and '--version' not in r['command']])==94
    # The established transfer gate verifies actual commands, all twelve controls,
    # both checked-arithmetic editions, learner stubs and current source pins.
    assert not foundation_transfer_errors(t)
    assert e['passed'] and e['status']=='passed' and e['source_stable'] and e['input_sha256'] and not e['private_sources_copied']
    assert e['explicit_providers']==['assessor/foundation_followthrough/follow.c','assessor/foundation_depth/depth.c']
    assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
    assert len(e['exercises'])==8 and {r['id'] for r in e['exercises']}==set(IDS) and len(e['executions'])==64
    for r in e['exercises']:assert len(r['executions'])==8 and r['assets'] and not r['private_sources_copied']
    for r in e['executions']:assert (r['exit_code']==r['expected_exit']==0) or (r['expected_exit'] is None and r['exit_code'] not in [None,0])
    if files:
        import sys
        sys.path.insert(0,str(ROOT))
        from cmastery import exercises,learning
        assert e.get('selected_catalog_sha256')=={ident:learning.digest(exercises.exercise(ident)) for ident in IDS}, 'current selected exercise contracts'
        bindings(d)
        for r in d['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r in kills:assert not any(s in text for s in ['AddressSanitizer','runtime error:'])
        for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h,'stale export '+p
        for r in e['exercises']:
            for p,h in r['assets'].items():assert e['input_sha256'][p]==h and 'assessor' not in p.split('/')
        for r in e['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256']
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text()
        gate=json.loads((ROOT/BASE/'research_gate.json').read_text());assert gate['status']=='passage-inspected'
        for p,h in {**gate['input_sha256'],**gate['external_source_sha256']}.items():assert sha(ROOT/p)==h
        for id in IDS:
            text=(ROOT/f'curriculum/v5/exercises/{id}.md').read_text(encoding='utf8');assert 'labs/foundation_followthrough/CONTRACTS.md' in text
    return True

def selftest(d,t,e):
    changes=[lambda a,b,c:a.update(passed=False),lambda a,b,c:a.update(source_stable=False),lambda a,b,c:a.update(input_sha256={}),lambda a,b,c:a['profiles'].pop(),lambda a,b,c:a['profiles'][0]['controls'].pop(),lambda a,b,c:a['profiles'][0]['oracle'].update(histories=1),lambda a,b,c:a['profiles'][0]['defense_oracle'].update(seed=1),lambda a,b,c:a['profiles'][0]['original_anchors'].pop(),lambda a,b,c:a['comparisons'].pop(),lambda a,b,c:a['comparisons'][0]['samples'].pop(),lambda a,b,c:a['diagnostics'][0].update(continued_translation=False),lambda a,b,c:a['targets'][0].update(executed=False),lambda a,b,c:a['targets'][2].update(executed=True),lambda a,b,c:a['targets'][0].update(undefined=['memcpy']),lambda a,b,c:a['compiler_dependencies'].pop(),lambda a,b,c:next(r for r in a['executions'] if r['required_marker']=='FF_CHECK:').update(exit_code=-11),lambda a,b,c:b['profiles'].pop(),lambda a,b,c:b.update(scope_source_stable=False),lambda a,b,c:c['exercises'].pop(),lambda a,b,c:c['executions'].pop(),lambda a,b,c:c.update(private_sources_copied=True),lambda a,b,c:c.update(learner_mastery='passed'),lambda a,b,c:c.update(explicit_providers=[])]
    for change in changes:
        a,b,f=copy.deepcopy(d),copy.deepcopy(t),copy.deepcopy(e);change(a,b,f)
        try:check(a,b,f,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted altered B01 evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();d,t,e=[json.loads((ROOT/q).read_text()) for q in [REPORT,TRANSFER,EXPORT]];check(d,t,e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,t,e) if args.self_test else 0,current_reports={q:sha(ROOT/q) for q in [REPORT,TRANSFER,EXPORT]})))
