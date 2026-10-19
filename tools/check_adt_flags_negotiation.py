"""Current-source flags agreement and complete raw-constructor evidence guard."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_flags_negotiation_validation.json'
EXHAUSTIVE='research/v5/modern/adt_flags_exhaustive_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'flags_negotiation',2:'flags_negotiation',3:'flags_sets',4:'flags_sets',5:'flags_base',6:'flags_base'}
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==set(CONTROL.values())
        assert len(p['controls'])==6 and {r['test'] for r in p['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==3 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==6 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and r['cases']==283808 and r['statuses']=={'0':22095,'1':17781,'2':243932} and r['trace_sha256']=='6aa4a8b341acf575c53f728bd0a8acde2d8bc56d0aa42c9941ef8a6e14a3126a'
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert r['offer_size']==8 and r['agreement_size']==16 and r['capabilities_size']>=8 and r['iterations']==100000 and r['checksum']==350000 and len(r['seconds'])==7 and all(x>=0 for x in r['seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-flags-negotiation-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==36 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==3 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    permitted={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in permitted for r in rows)
    assert len(q['observations'])==6 and {(r['compiler'],r['mutation']) for r in q['observations']}=={(c,m) for c in ['gcc','clang'] for m in [0,5,6]} and all(r['passed'] for r in q['observations'])
    for row in q['observations']:
        assert row['result']==(dict(values=4294967296,accepted=8,rejected=4294967288,checksum=28,unchanged_rejections=True) if row['mutation']==0 else {})
    qr=q['executions'];compiled=[r for r in qr if '-c' in r['command']];linked=[r for r in qr if '-fno-lto' in r['command'] and '-c' not in r['command']];ran=[r for r in qr if Path(r['command'][0]).name in ['gcc-0','gcc-5','gcc-6','clang-0','clang-5','clang-6']]
    assert len(compiled)==12 and all(r['exit_code']==0 and '-fno-lto' in r['command'] and '-O3' in r['command'] and '-MD' in r['command'] for r in compiled)
    assert len(linked)==6 and all(r['exit_code']==0 for r in linked)
    assert len(ran)==6 and all(r['exit_code']==(0 if r['command'][0].endswith('-0') else 1) for r in ran)
    assert len(qr)==26 and all(r['exit_code']==0 or (r in ran and r['exit_code']==1) for r in qr)
    required={'labs/adt/flags_negotiation/exhaustive.c','labs/adt/include/adt.h','assessor/adt/adt.c','tools/run_adt_flags_exhaustive.py'};assert required<=set(q['input_sha256'])
    if files:
        bindings(d);bindings(q)
        for row in mutants+learners:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','runtime error:'])
        for row in ctests:
            text=(ROOT/row['log']).read_text(encoding='utf8');assert all(s in text for s in ['ADF_NEGOTIATION intersection-authority','ADF_SETS complete-three-bit-lattice','ADF_BASE actual-ad-flags','100% tests passed'])
        for row in ran:
            text=(ROOT/row['log']).read_text(encoding='utf8')
            if row['exit_code']==0:assert json.loads(text)==dict(values=4294967296,accepted=8,rejected=4294967288,checksum=28,unchanged_rejections=True)
            else:assert 'ADO_CHECK exhaustive raw='+('8' if row['command'][0].endswith('-5') else '0') in text
    return True
def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b['observations'][0]['result'].update(values=256),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['executions'] if '-c' in r['command'])['command'].remove('-fno-lto'),lambda a,b:next(r for r in b['executions'] if r['command'][0].endswith('gcc-5')).update(exit_code=0)]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');args=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/EXHAUSTIVE).read_text());check(d,q);n=selftest(d,q) if args.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
