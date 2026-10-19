"""Require actual boundary executions, semantic controls and current source pins."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/ownership_authority_boundary_validation.json'
COST='research/v5/modern/ownership_authority_boundary_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'authority_defect',2:'authority_bounds',3:'authority_rights',4:'authority_revoke',5:'authority_rights',6:'authority_wire',7:'authority_rights',8:'authority_revoke'}
ANCHORS=set(CONTROL.values())|{'authority_atomic'}
TRACE='c2bccf9613b2089954fec79d3578bbac3f93842f745c4af44411c0c4ca4a9e30'
FOREIGN='77e910d095e1e330f4bde71b877a732cf07826be4a22e54d175d0522b77e829c'
def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==8
        assert {r['test'] for r in p['controls']}=={f'ownership.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        rows=[r for r in d['observations'] if r['kind']==kind];assert len(rows)==6 and {r['profile'] for r in rows}==PROFILES
        for row in rows:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['cases'],r['trace_sha256'])==(40320,TRACE)
            else:assert (r['iterations'],r['checksum'],r['wire_bytes'])==(10000,490000,16) and r['root_size']>0 and r['view_size']>0 and len(r['seconds'])==7 and min(r['seconds'])>=0
    needed={'assessor/ownership/ownership.c','assessor/ownership/authority_boundary/authority_boundary.c','labs/ownership/include/ownership.h','labs/ownership/authority_boundary/include/authority_boundary.h','labs/ownership/authority_boundary/AUTHORITY.md'};assert needed<=set(d['input_sha256']) and needed<=set(q['input_sha256'])
    assert 'labs/ownership/authority_boundary/tests/foreign.c' in q['input_sha256']
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'ownership-authority_boundary-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==151 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert len(q['executions'])==427 and len(q['observations'])==10 and len(q['generated_artifacts'])==2 and all(r['exit_code']==0 for r in q['executions'])
    foreign=[r for r in q['observations'] if r['kind']=='foreign-byte-boundary'];assert len(foreign)==6 and {r['profile'] for r in foreign}==PROFILES
    for r in foreign:assert r['language']=='c17' and r['requests']==66 and r['trace_sha256']==FOREIGN and r['python_serialized'] and r['executed'] and r['trusted_parent']=='tenant1 READ interval2..6 selected in C server'
    inputs=[r for r in q['executions'] if r['stdin_hex'] is not None];assert len(inputs)==396 and {len(bytes.fromhex(r['stdin_hex'])) for r in inputs}==set(range(18))
    builds=[r for r in q['executions'] if any('foreign.c' in x for x in r['command']) and '-M' not in r['command']];assert len(builds)==6 and all('-std=c17' in r['command'] for r in builds)
    for cc in ['gcc','clang']:
        a=[r for r in q['observations'] if r['kind']=='object-imports' and r['compiler']==cc];assert len(a)==1;r=a[0];assert not any(r[k] for k in ['direct_heap_imports','flawed_import','whole_process_claim']) and not set(r['symbols'])&{'malloc','calloc','realloc','free','ow_flawed_write'}
    def one(kind):
        a=[r for r in q['observations'] if r['kind']==kind];assert len(a)==1;return a[0]
    r=one('exported-composition');assert r['executed'] and not r['private_provider_copied'] and r['actual_source']=='assessor/ownership/ownership.c'
    r=one('incremental');assert r['unchanged_no_work'] and min(r['clean_seconds'],r['unchanged_seconds'])>=0
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'OW_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:assert '100% tests passed' in (ROOT/r['log']).read_text(encoding='utf8')
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-010.md').read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0]);assert {'ownership.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/ownership/authority_boundary/AUTHORITY.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports')['symbols'].append('malloc'),lambda a,b:next(r for r in b['observations'] if r['kind']=='object-imports').update(whole_process_claim=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='foreign-byte-boundary').update(python_serialized=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='foreign-byte-boundary').update(language='c11'),lambda a,b:next(r for r in b['executions'] if r['stdin_hex'] is not None).update(stdin_hex=None),lambda a,b:next(r for r in b['observations'] if r['kind']=='foreign-byte-boundary').update(trusted_parent='client pointer'),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='exported-composition').update(private_provider_copied=True)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted unsupported evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);print(json.dumps(dict(passed=True,negative_controls=selftest(d,q) if a.self_test else 0,report_sha256=sha(ROOT/REPORT))))
