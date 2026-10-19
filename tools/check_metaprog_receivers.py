"""Guard source-bound receiver, qualification and emitted-access claims."""
import argparse
import copy
import json
from pathlib import Path
from check_replicated_replay_depth import ROOT,base,bindings,sha
from run_metaprog_receiver_audit import ACCESS
REPORT='research/v5/modern/metaprog_receivers_validation.json'
COST='research/v5/modern/metaprog_receiver_audit_validation.json'
PROFILES={c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']}
CONTROL={1:'receiver_dispatch',2:'receiver_dispatch',3:'receiver_effects',4:'receiver_device',5:'receiver_device',6:'receiver_device',7:'receiver_base',8:'receiver_base'}
TRACE='e7a2abee1b1d6a3d64daff37698d6038a5c42168849c0a36f7e227731e5288f8'
DIAGNOSTICS={'unsupported','const-write','cv-write','unselected-invalid','compatible-duplicate','variable-association','function-association','discard-volatile','wrong-device-view'}
def check(d,q,files=True):
    base(d);assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==set(CONTROL.values()) and len(r['controls'])==8
        assert {x['test'] for x in r['controls']}=={f'metaprog.mutation.{v}.{k}' for k,v in CONTROL.items()} and all(x['semantic_rejection'] for x in r['controls'])
    assert len(d['learner_stub_rejections'])==4 and {r['test'] for r in d['learner_stub_rejections']}==set(CONTROL.values()) and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    oracle=[r for r in d['observations'] if r['kind']=='independent-oracle'];metrics=[r for r in d['observations'] if r['kind']=='metrics'];assert len(oracle)==6 and {r['profile'] for r in oracle}==PROFILES and len(metrics)==6 and {r['profile'] for r in metrics}==PROFILES
    for row in oracle:
        r=row['result'];assert r['passed'] and (r['cases'],r['accepted'],r['invalid'],r['range'],r['callback_calls'])==(4804,3746,26,1032,941) and r['trace_sha256']==TRACE
    for row in metrics:
        r=row['result'];assert (r['iterations'],r['checksum'])==(10000,70000) and min(r['read_size'],r['rw_size'],r['w1c_size'])>0
        assert all(len(r[k])==7 and all(t>=0 for t in r[k]) for k in ['generic_seconds','direct_seconds'])
    assert {'assessor/metaprog/metaprog.c','assessor/metaprog/receivers/receivers.c','assessor/metaprog/receivers/receiver_macros.h'}<=set(d['input_sha256']) and {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    rows=d['executions'];mutants=[r for r in rows if 'metaprog-receivers-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
    assert len(rows)==149 and len(mutants)==48 and all(r['exit_code']==1 for r in mutants) and len(learners)==4 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
    assert q['passed'] and q['source_stable'] and q['external_inputs_stable'] and q['input_sha256'] and q['external_input_sha256'] and q['physical_validation']=='not-run' and q['independent_review']=='not-run' and q['learner_mastery']=='not-assessed'
    obs=q['observations'];assert len(obs)==38
    def kind(k):return [r for r in obs if r['kind']==k]
    assert len(kind('typed-positive'))==2 and {r['compiler'] for r in kind('typed-positive')}=={'gcc','clang'} and all(r['executed'] and r['categories']==[1,2,3,4] and r['top_pointer']==r['array_category']==1 for r in kind('typed-positive'))
    assert len(kind('diagnostic'))==18 and {(r['compiler'],r['case']) for r in kind('diagnostic')}=={(c,k) for c in ['gcc','clang'] for k in DIAGNOSTICS} and all(r['rejected'] for r in kind('diagnostic'))
    assert len(kind('raw-qualifier'))==2 and {r['compiler'] for r in kind('raw-qualifier')}=={'gcc','clang'} and all(r['executed'] and r['category']==1 for r in kind('raw-qualifier'))
    assert len(kind('explicit-cast'))==2 and {r['compiler'] for r in kind('explicit-cast')}=={'gcc','clang'} and all(r['compiled'] and not r['executed'] for r in kind('explicit-cast'))
    assert len(kind('preprocess'))==2 and {r['compiler'] for r in kind('preprocess')}=={'gcc','clang'} and all(min(r['bytes'],r['tokens'])>0 for r in kind('preprocess'))
    assert len(kind('object'))==4 and {(r['compiler'],r['optimization']) for r in kind('object')}=={(c,o) for c in ['gcc','clang'] for o in ['0','3']}
    for r in kind('object'):assert set(r['helpers'])<={'__stack_chk_fail'} and set(r['functions'])=={'mpv_read_m','mpv_read_c','mpv_read_v','mpv_read_cv','mpv_write_m','mpv_write_v','mpv_observe_v','mpv_observe_cv','mpv_update','mpv_ack'} and all(n>0 for n in r['functions'].values())
    assert len(kind('volatile-ir'))==4 and {(r['target'],r['optimization']) for r in kind('volatile-ir')}=={(t,o) for t in ['host','cortex-m4'] for o in ['0','3']}
    for r in kind('volatile-ir'):
        assert not r['executed'] and set(r['functions'])==set(ACCESS)
        for fn,(loads,stores) in ACCESS.items():assert r['functions'][fn]==dict(volatile_loads=loads,volatile_stores=stores,synchronization_instruction=False)
    assert len(kind('freestanding-link'))==2 and {r['optimization'] for r in kind('freestanding-link')}=={'0','3'} and all(r['target']=='cortex-m4' and r['elf_class']=='ELF32' and r['machine']=='ARM' and r['undefined_symbols']==r['dynamic_dependencies']==[] and not r['executed'] for r in kind('freestanding-link'))
    assert len(kind('access-comparison'))==1 and kind('access-comparison')[0]['loads']=={'plain_twice':1,'volatile_twice':2} and not kind('access-comparison')[0]['executed']
    assert len(kind('incremental'))==1 and kind('incremental')[0]['unchanged_no_work'] and min(kind('incremental')[0]['clean_seconds'],kind('incremental')[0]['unchanged_seconds'])>=0
    assert len(q['executions'])==81 and sum(r['expected']=='diagnostic' for r in q['executions'])==18 and all(r['exit_code']==(1 if r['expected']=='diagnostic' else 0) for r in q['executions']) and len(q['generated_artifacts'])==22
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
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T68-005.md').read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert {'metaprog.'+a for a in CONTROL.values()}<=set(m['lab']['tests']) and 'labs/metaprog/receivers/RECEIVERS.md' in m['shared_contracts']
    return True
def selftest(d,q):
    edits=[lambda a,b:a.update(passed=False),lambda a,b:a['profiles'].pop(),lambda a,b:a['profiles'][0]['controls'].pop(),lambda a,b:a['learner_stub_rejections'].pop(),lambda a,b:a['observations'][0]['result'].update(accepted=0),lambda a,b:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a,b:a.update(input_sha256={}),lambda a,b:a.update(learner_mastery='passed'),lambda a,b:a['compiler_dependency_closures'].pop(),lambda a,b:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a,b:b['observations'].pop(),lambda a,b:next(r for r in b['observations'] if r['kind']=='object')['helpers'].append('hidden_device'),lambda a,b:b.update(source_stable=False),lambda a,b:b.update(physical_validation='passed'),lambda a,b:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='typed-positive').update(executed=False),lambda a,b:next(r for r in b['observations'] if r['kind']=='volatile-ir')['functions']['mpv_ack'].update(volatile_loads=1),lambda a,b:next(r for r in b['observations'] if r['kind']=='freestanding-link').update(executed=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='explicit-cast').update(executed=True),lambda a,b:next(r for r in b['executions'] if r['expected']=='diagnostic').update(exit_code=0),lambda a,b:next(r for r in b['observations'] if r['kind']=='volatile-ir')['functions']['mpv_update'].update(synchronization_instruction=True),lambda a,b:next(r for r in b['observations'] if r['kind']=='freestanding-link')['undefined_symbols'].append('memcpy')]
    for edit in edits:
        a,b=copy.deepcopy((d,q));edit(a,b)
        try:check(a,b,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted wrong evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());q=json.loads((ROOT/COST).read_text());check(d,q);n=selftest(d,q) if a.self_test else 0;print(json.dumps(dict(passed=True,negative_controls=n,report_sha256=sha(ROOT/REPORT))))
