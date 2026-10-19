"""Reject stale, missing or nonsemantic packet/ring evidence."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/adt_packet_ring_validation.json'
COST='research/v5/modern/adt_packet_cost_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'packet_ring',2:'packet_ring',3:'packet_lifetime',4:'packet_lifetime',5:'packet_pipe',6:'packet_pipe',7:'packet_base',8:'packet_base'}
MARKERS=['ADP_RING every-origin segmented-zero-copy cancellation-retains-bytes','ADP_LIFETIME owner-generation release-only-close finite-retirement','ADP_PIPE actual-one-byte-pipe-writes truncated-EOF codec-versions','ADP_BASE exact-error-precedence borrowed-cursor checked-size']
TRACE='64129feac15580a37355d7ed983dc4263867692096499f71c08bf5905422330c'
HISTORY='678d4a87d7243c754adc14695b60762cc65483e5747dcaa969d51c28b2ae12c3'
IMPORTS={'assessor/adt/adt.c','assessor/adt/packet_ring/packet_ring.c','assessor/foundations/foundation.c','assessor/api_depth/architecture/core.c','assessor/hosted_systems/frame.c','assessor/hosted_systems/common.h'}
def check(d,q,files=True):
    base(d)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values())
        assert len(r['controls'])==8 and {x['test'] for x in r['controls']}=={f'adt.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracles=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics']
    assert len(oracles)==6 and {r['profile'] for r in oracles}==PROFILES
    for row in oracles:
        r=row['result'];assert r['passed'] and (r['cases'],r['admitted'],r['histories'],r['transitions'])==(26080,2208,32774,163894) and r['trace_sha256']==TRACE and r['history_sha256']==HISTORY
    assert len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'],r['payload'])==(10000,4930000,29) and min(r['ring_size'],r['ring_alignment'])>0
        assert all(len(r[k])==7 and all(x>=0 for x in r[k]) for k in ['borrow_seconds','copy_seconds'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256'])
    assert IMPORTS<=set(d['input_sha256'])
    rows=d['executions'];mutants=[r for r in rows if 'adt-packet-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(mutants)==48 and all(r['exit_code']==1 for r in mutants)
    assert len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners)
    assert len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256']
    objects=[r for r in q['observations'] if r['kind']=='object'];strict=[r for r in q['observations'] if r['kind']=='strict'];cost=[r for r in q['observations'] if r['kind']=='incremental'];census=[r for r in q['observations'] if r['kind']=='census'];layout=[r for r in q['observations'] if r['kind']=='representation']
    assert len(objects)==4 and {(r['compiler'],r['optimization']) for r in objects}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in objects:assert {'ad_packet','f_checked_size','f_take','ar_decode','hs_read_frame'}<=set(r['symbols']) and set(r['symbols'])<=set(r['allowed']) and r['allowed']==['ad_packet','f_checked_size','f_take','ar_decode','hs_read_frame','memcpy','memset','__stack_chk_fail']
    assert len(strict)==2 and {r['compiler'] for r in strict}=={'gcc','clang'} and all(r['passed'] for r in strict)
    assert len(cost)==1 and cost[0]['unchanged_no_work'] and min(cost[0]['clean_seconds'],cost[0]['unchanged_seconds'])>=0
    assert len(census)==1 and census[0]['passed'] and census[0]['compiler']=='g++' and 'Behavioral checks: passed' in census[0]['output']
    assert len(layout)==1 and set(layout[0]['layout'])=={'ADPacket','AdpRing','AdpToken','AdpPart','AdpLease'} and all(len(v)==2 and min(v)>0 for v in layout[0]['layout'].values())
    assert len(q['executions'])==26 and all(r['exit_code']==0 for r in q['executions'])
    assert q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    if files:
        bindings(d)
        for p,h in q['input_sha256'].items():assert sha(ROOT/p)==h,'stale cost source'
        for p,h in q['external_input_sha256'].items():assert Path(p).is_file() and sha(Path(p))==h
        for r in q['executions']:
            assert sha(ROOT/r['log'])==r['log_sha256'];text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in MARKERS+['100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T67-010.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'adt.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/adt/packet_ring/PACKET.md' in m['shared_contracts']
    return True
def selftest(d,q):
    changes=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(cases=1),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['symbols'].append('unpermitted_solver'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(payload=0),lambda a,b:a['input_sha256'].pop('assessor/hosted_systems/frame.c'),lambda a,b:a['profiles'].append(a['profiles'][0]),lambda a,b:a['observations'][0]['result'].update(history_sha256='wrong'),lambda a,b:next(r for r in b['observations'] if r['kind']=='census').update(passed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='representation')['layout'].pop('AdpToken')]
    for change in changes:
        a,b=copy.deepcopy((d,q));change(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted incorrect evidence')
    return len(changes)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,profiles=6,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
