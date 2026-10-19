"""Current-source gate for exact original publication/recovery criteria."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/modern/ownership_publication_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CONTROL={1:'publication_prefix',2:'publication_prefix',5:'publication_close',6:'publication_close'}
ANCHORS={'publication_admission','publication_prefix','publication_close','publication_recovery','publication_limits','publication_original'}
TRACE='4cc738a0db252a33a3bc7fec9200b1963fe2e4fb4f7570e5c515e729edce7694'
def check(d,files=True):
    base(d)
    assert len(d['profiles'])==6 and {r['profile'] for r in d['profiles']}==PROFILES
    for r in d['profiles']:
        assert r['passed'] and set(r['anchors'])==ANCHORS and len(r['controls'])==4
        assert {c['test'] for c in r['controls']}=={f'modern_composition.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(c['semantic_rejection'] for c in r['controls'])
    assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
    for kind in ['independent-oracle','metrics']:
        rows=[r for r in d['observations'] if r['kind']==kind];assert len(rows)==6 and {r['profile'] for r in rows}==PROFILES
        for r in rows:
            q=r['result']
            if kind=='independent-oracle':assert q['passed'] and (q['outcome_words'],q['outcomes'],q['max_close_results'],q['original_prefix_rows'],q['trace_sha256'])==(81,3,4,12,TRACE)
            else:assert (q['iterations'],q['checksum'],q['attempts_per_sample'],q['releases_per_sample'])==(10000,700000,30000,30000) and q['recovery_size']>0 and len(q['seconds'])==7 and min(q['seconds'])>=0
    needed={'assessor/modern_composition/composition.c','assessor/modern_composition/recovery.c','assessor/ownership/ownership.c','assessor/errors/errors.c','assessor/coroutines/coroutines.c','assessor/messaging/messaging.c','labs/modern_composition/publication_depth/tests.c','labs/modern_composition/tests/recovery.c','tools/verify_modern_composition_recovery.py','fixtures/modern_composition/recovery.json'};assert needed<=set(d['input_sha256'])
    assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
    for r in d['compiler_dependency_closures']:assert r['repository_inputs'] and set(r['repository_inputs'])<=set(d['input_sha256']) and r['external_inputs']
    records=d['executions'];mutants=[r for r in records if 'modern-publication-depth-mutant-' in r['command'][0]];learners=[r for r in records if '-R' in r['command']];ctests=[r for r in records if '--verbose' in r['command']]
    assert len(mutants)==24 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
    allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in records)
    builds=[r for r in records if '--build' in r['command']];assert len(builds)==7 and all(r['exit_code']==0 for r in builds)
    if files:
        bindings(d)
        for r in mutants+learners:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert 'MC_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
        for r in ctests:
            text=(ROOT/r['log']).read_text(encoding='utf8');assert all(x in text for x in ['MCP_ADMISSION empty-invalid-range','MCP_PREFIX actual-parser-allocator-owner','MCP_CLOSE 81-outcome-words','MCP_RECOVERY precommit-old-owner','MCP_LIMIT final-publication','recoverable publication: 12 failure combinations','100% tests passed'])
        for r in d['generated_artifacts']:
            if r['path'].endswith('compile_commands.json'):
                commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
        m=json.loads((ROOT/'curriculum/v5/exercises/X-T66-011.md').read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0]);assert {'modern_composition.'+a for a in ANCHORS}<=set(m['lab']['tests']) and 'labs/modern_composition/publication_depth/PUBLICATION.md' in m['shared_contracts']
    return True
def selftest(d):
    edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(outcome_words=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a['observations'][0]['result'].update(original_prefix_rows=0),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(checksum=0),lambda a:a['input_sha256'].pop('assessor/ownership/ownership.c'),lambda a:next(r for r in a['observations'] if r['kind']=='metrics')['result'].update(releases_per_sample=29997),lambda a:next(r for r in a['executions'] if '--build' in r['command']).update(exit_code=1),lambda a:a['input_sha256'].pop('labs/modern_composition/tests/recovery.c'),lambda a:a['profiles'][0].update(anchors=['publication_prefix'])]
    for edit in edits:
        a=copy.deepcopy(d);edit(a)
        try:check(a,False)
        except (AssertionError,KeyError):continue
        raise AssertionError('accepted unsupported evidence')
    return len(edits)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());check(d);print(json.dumps(dict(passed=True,negative_controls=selftest(d) if a.self_test else 0,report_sha256=sha(ROOT/REPORT))))
