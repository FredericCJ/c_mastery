"""Require observed option/owner semantics, compiler comparisons and current inputs."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/errors_owning_option_validation.json'
COST='research/v5/modern/errors_option_investigation_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'option_owner',2:'option_owner',3:'option_owner',4:'option_owner',5:'option_scalar',6:'option_scalar',7:'option_failure',8:'option_failure',9:'option_base',10:'option_base'}
ANCHORS=set(CONTROL.values())
TRACE='5cd727554e24e375c845b867a48954ae04aa44df411516296367c841636e08bc'

def check(d,q,files=True):
    base(d);base(q)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for p in d['profiles']:
        assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==10
        assert {r['test'] for r in p['controls']}=={f'errors.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        rows=[r for r in d['observations'] if r['kind']==kind];assert len(rows)==6 and {r['profile'] for r in rows}==PROFILES
        for row in rows:
            r=row['result']
            if kind=='independent-oracle':assert r['passed'] and (r['scalar_cases'],r['owner_histories'],r['trace_sha256'])==(771,5192,TRACE)
            else:
                assert (r['iterations'],r['checksum'],r['allocation_attempts'],r['releases'])==(10000,560000,70000,70000)
                assert min(r[x] for x in ['scalar_size','scalar_align','owner_size','owner_align','result_size','result_align','pointer_size'])>0
                assert all(len(r[x])==7 and all(t>=0 for t in r[x]) for x in ['scalar_seconds','owner_seconds'])
    assert {'assessor/errors/errors.c','assessor/errors/owning_option/owning_option.c','assessor/ownership/ownership.c','labs/ownership/include/imports/v4_algo.h'}<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for c in d['compiler_dependency_closures']:assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
    rows=d['executions'];mutants=[r for r in rows if 'errors-owning_option-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==173 and len(mutants)==60 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [None,0] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    obs=q['observations'];assert len(obs)==13 and len(q['executions'])==38
    negative=[r for r in q['executions'] if r['expected_failure']];assert len(negative)==7 and all(r['exit_code'] not in [None,0] for r in negative)
    assert all(r['exit_code']==0 or r['expected_failure'] for r in q['executions'])
    for cc in ['gcc','clang']:
        diag=[r for r in obs if r['kind']=='C08-diagnostic' and r['compiler']==cc];assert len(diag)==3 and {r['case'] for r in diag}=={1,2,3} and all(r['rejected'] and not r['executed'] for r in diag)
        unit=[r for r in obs if r['kind']=='unit-positive' and r['compiler']==cc];assert len(unit)==1 and unit[0]['executed'] and not unit[0]['noreturn_invoked']
    layouts=[r for r in obs if r['kind']=='layout'];assert len(layouts)==3 and {r['compiler'] for r in layouts}=={'gcc','clang','arm-none-eabi-gcc'} and {r['pointer_bits'] for r in layouts}=={32,64}
    for r in layouts:assert r['object_observed'] and not r['executed'] and r['values'][2:5]==[4,8,4] and (r['pointer_bits']==32)==(r['elf32'] and r['arm'])
    rust=[r for r in obs if r['kind']=='rust-comparison'];assert len(rust)==1;r=rust[0];assert r['executed'] and r['moved_value_rejected'] and r['drop_count']==1 and r['zero_id'] and r['listed_niches'] and not r['blanket_abi_equivalence'] and r['u32_option_size']>r['u32_size']
    builds=[r for r in obs if r['kind']=='incremental'];assert len(builds)==1 and builds[0]['unchanged_no_work'] and min(builds[0]['clean_seconds'],builds[0]['unchanged_seconds'])>=0
    assert len(q['generated_artifacts'])==9
    if files:
        bindings(d);bindings(q)
        for r in q['executions']:
            text=(ROOT/r['log']).read_text(encoding='utf8')
            if r['required_marker']:assert r['required_marker'] in text
            if r['expected_failure']:assert 'error' in text
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'ER_CHECK' in text and not any(s in text for s in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(s in text for s in ['EOP_OWNER actual-T66-allocator zero-ID','EOP_FAILURE allocation-refusal','EOP_SCALAR present-zero','EOP_BASE actual-niche','100% tests passed'])
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T65-001.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'errors.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/errors/owning_option/OPTION.md' in m['shared_contracts'] and 'X-T66-009' not in m['prerequisite_exercises']
        f=json.loads((ROOT/'labs/errors/family.json').read_text(encoding='utf8'));row=next(r for r in f['exercises'] if r['id']=='X-T65-001');assert any(r['family']=='ownership' and r['artifact']=='memory.adversarial_allocator' for r in row['dependencies'])
    return True

def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(owner_histories=1),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(releases=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='layout').update(executed=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-comparison').update(blanket_abi_equivalence=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='C08-diagnostic').update(executed=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='incremental').update(unchanged_no_work=False),lambda a,b:next(r for r in b['executions'] if r['expected_failure']).update(exit_code=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='rust-comparison').update(moved_value_rejected=False)]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
