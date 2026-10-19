"""Validate the complete metaprog CMake campaign, without release acceptance."""
import argparse
import copy
import hashlib
import json
import os
import re
from pathlib import Path
from common import ROOT, dump, read_json, sha
from run_metaprog_current import BASE, MODULES, PROFILES, DIAGNOSTICS, contract, assertion, junit


def validate(d,files=True):
    meta,specs,markers,controls=contract()
    all_controls={n for values in controls.values() for n in values}
    expected=set(markers)|all_controls
    assert d['family']=='metaprog' and d['producer']=='tools/run_metaprog_current.py'
    assert d['passed'] is True and d['status']=='passed' and d['scope_source_stable'] is True and d['external_inputs_stable'] is True
    assert (d['physical_validation'],d['learner_mastery'],d['independent_review'])==('not-run','not-assessed','not-run')
    assert d['inspected_paths'] and d['external_input_sha256']
    assert d['scope_input_sha256']==hashlib.sha256(json.dumps(d['inspected_paths'],sort_keys=True).encode()).hexdigest()
    assert {'tools/run_metaprog.py','tools/run_metaprog_current.py','tools/run_resilience_scoped.py',
            'assessor/metaprog/metaprog.c'}<=set(d['inspected_paths'])
    rows=d['executions'];assert len(rows)==784
    assert len(d['profiles'])==6 and {p['compiler']+'-'+p['profile'] for p in d['profiles']}==set(PROFILES)
    logs={}
    if files:
        for path,digest in d['inspected_paths'].items():assert sha(ROOT/path)==digest,('source',path)
        for r in rows:
            assert sha(ROOT/r['log'])==r['log_sha256'],('log',r['log'])
            logs[r['log']]=(ROOT/r['log']).read_text(encoding='utf8')
        for r in d['generated_artifacts']:assert sha(ROOT/r['path'])==r['sha256']
        if os.name!='nt':
            for p,h in d['external_input_sha256'].items():assert sha(Path(p))==h,('environment',p)
    used=set()
    for p in d['profiles']:
        label=p['compiler']+'-'+p['profile']
        assert p['passed'] is True and p['c17_confirmed'] is True and p['observation_markers_confirmed'] is True
        assert p['anchors']==meta['tests'] and p['original_anchors']==['metaprog.'+a for a in BASE]
        assert set(p['supplementary_cases'])==set(markers)-{'metaprog.'+a for a in BASE} and len(p['supplementary_cases'])==42
        assert p['controls_by_module']==controls and set(p['compiled_mutant_controls'])==all_controls and len(p['compiled_mutant_controls'])==102
        assert p['ctest_count']==154 and len(p['test_results'])==154
        assert {r['test'] for r in p['test_results']}==expected and all(r['passed'] is True for r in p['test_results'])
        if p['profile']=='tsan':assert p['thread_instrumentation_confirmed'] is True
        assert len(p['direct_controls'])==102 and {r['test'] for r in p['direct_controls']}==all_controls
        assert len(p['oracle_results'])==11 and {r['module'] for r in p['oracle_results']}==set(MODULES)|{'original'}
        listrow=rows[p['list_execution']];ctest=rows[p['ctest_execution']]
        assert listrow['label']==label+'-list' and listrow['exit_code']==0 and '--show-only=json-v1' in listrow['command']
        assert ctest['label']==label+'-ctest' and ctest['exit_code']==0 and '--verbose' in ctest['command'] and '--output-junit' in ctest['command']
        listed_commands={}
        if files:
            for test in json.loads(logs[listrow['log']])['tests']:
                cmd=test['command']
                if len(cmd)>1 and cmd[1].endswith('/expect_failure.py'):cmd=cmd[2:]
                listed_commands[test['name']]=cmd
        for r in p['direct_controls']:
            assert r['anchor']==r['test'].replace('metaprog.mutation.','metaprog.').rsplit('.',1)[0]
            assert r['rejected'] is True and r['marker']==markers[r['anchor']]
            record=rows[r['execution']]
            assert record['kind']=='control' and record['label']==label+'-'+r['test'] and record['exit_code']==1
            assert record['command'][-1]=='{}'.format(r['anchor'].removeprefix('metaprog.'))
            assert 'mutant-' in record['command'][0]
            assert r['execution'] not in used;used.add(r['execution'])
            if files:
                assert record['command']==listed_commands[r['test']],('wrong named control command',r['test'])
                assert assertion(logs[record['log']],r['marker'],record['exit_code'])
        for r in p['oracle_results']:
            record=rows[r['execution']];assert r['passed'] is True and record['exit_code']==0 and record['label']==label+'-oracle-'+r['module']
            oracle='tools/verify_metaprog_fixtures.py' if r['module']=='original' else specs[r['module']]['oracle']
            assert record['command'][1].endswith('/'+oracle) and record['command'][2]=='--driver'
        if files:
            tests=json.loads(logs[listrow['log']])['tests']
            assert len(tests)==154 and {t['name'] for t in tests}==expected
            assert p['test_results']==junit(ROOT/p['junit']['path'])
            assert sha(ROOT/p['junit']['path'])==p['junit']['sha256']
            out=logs[ctest['log']]
            assert '100% tests passed' in out and all(m in out for s in specs.values() for m in s['observation_markers'])
            assert not any(x in out for x in DIAGNOSTICS)
    assert len(d['learner_stub_rejections'])==52 and {r['test'] for r in d['learner_stub_rejections']}==set(markers)
    for r in d['learner_stub_rejections']:
        assert r['rejected'] is True and r['marker']==markers[r['test']]
        record=rows[r['execution']]
        assert record['kind']=='learner' and record['label']=='learner-'+r['test'] and record['exit_code']==8
        assert '-R' in record['command'] and record['command'][record['command'].index('-R')+1]=='^'+re.escape(r['test'])+'$'
        assert r['execution'] not in used;used.add(r['execution'])
        if files:
            out=logs[record['log']]
            assert r['marker'] in out and '1/1 Test' in out and '(Failed)' in out and not any(x in out for x in DIAGNOSTICS)
    assert len(used)==6*102+52
    for i,r in enumerate(rows):
        assert r['exit_code']==0 or i in used
        if r['kind'] in ['control','learner']:assert i in used
    assert len(d['compiler_dependency_closures'])==7 and {r['profile'] for r in d['compiler_dependency_closures']}==set(PROFILES)|{'learner'}
    for r in d['compiler_dependency_closures']:
        assert r['repository_inputs'] and r['external_inputs']
        assert set(r['repository_inputs'])<=set(d['inspected_paths']) and set(r['external_inputs'])<=set(d['external_input_sha256'])
        g=r['generated_header'];assert g['producer_source']=='labs/metaprog/schema/schema_stamp.py'
        assert g['fields_source']==('labs/metaprog/schema/src/schema_fields.h' if r['profile']=='learner' else 'assessor/metaprog/schema/schema_fields.h')
        assert {g['producer_source'],g['fields_source']}<=set(d['inspected_paths'])
        v=rows[g['verification_execution']];assert v['exit_code']==0 and v['command'][-1]=='--check'
        assert v['command'][1].endswith('/'+g['producer_source']) and v['command'][2].endswith('/'+g['fields_source'])
        recipe=rows[g['recipe_execution']];assert recipe['exit_code']==0 and recipe['label']==r['profile']+'-link'
        if files:
            import importlib.util
            spec=importlib.util.spec_from_file_location('current_mps_stamp',ROOT/g['producer_source']);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            assert sha(ROOT/g['artifact']['path'])==g['artifact']['sha256']
            assert (ROOT/g['artifact']['path']).read_text()==module.generate(ROOT/g['fields_source'])
            assert g['recipe'] in logs[recipe['log']] and g['producer_source'] in g['recipe'] and g['fields_source'] in g['recipe']
            artifact=r['compile_commands'];assert sha(ROOT/artifact['path'])==artifact['sha256']
            cc=read_json(ROOT/artifact['path']);assert cc and all('-std=c17' in c['command'] for c in cc)
            if r['profile'].endswith('-tsan'):assert all('-fsanitize=thread' in c['command'] for c in cc)
    return True


def selftest(d):
    edits=[lambda x:x.update(passed=False),lambda x:x.update(scope_source_stable=False),lambda x:x.update(external_inputs_stable=False),
           lambda x:x.update(physical_validation='passed'),lambda x:x.update(learner_mastery='passed'),lambda x:x['profiles'].pop(),
           lambda x:x['profiles'][0]['supplementary_cases'].pop(),lambda x:x['profiles'][0]['test_results'].pop(),
           lambda x:x['profiles'][0]['test_results'][0].update(passed=False),lambda x:x['profiles'][0]['direct_controls'].pop(),
           lambda x:x['profiles'][0]['direct_controls'][0].update(anchor='metaprog.unrelated'),
           lambda x:x['executions'][x['learner_stub_rejections'][0]['execution']]['command'].__setitem__(-2,'^metaprog\\.unrelated$'),
           lambda x:x['profiles'][0]['controls_by_module']['owning_ring'].pop(),lambda x:x['profiles'][0]['oracle_results'].pop(),
           lambda x:x['profiles'][0].update(c17_confirmed=False),
           lambda x:x['profiles'][0].update(observation_markers_confirmed=False),lambda x:x['learner_stub_rejections'].pop(),
           lambda x:x['learner_stub_rejections'][10].update(marker='MP_CHECK '),lambda x:x['learner_stub_rejections'][10].update(rejected=False),
           lambda x:x['compiler_dependency_closures'].pop(),lambda x:x['compiler_dependency_closures'][0].pop('generated_header'),lambda x:x['executions'][x['profiles'][0]['ctest_execution']].update(exit_code=1),
           lambda x:x['executions'][x['profiles'][0]['direct_controls'][0]['execution']].update(exit_code=-11),
           lambda x:x['executions'][x['learner_stub_rejections'][0]['execution']].update(exit_code=0),lambda x:x.update(scope_input_sha256='wrong')]
    for edit in edits:
        changed=copy.deepcopy(d);edit(changed)
        try:validate(changed,False)
        except (AssertionError,KeyError,ValueError,IndexError):continue
        raise AssertionError('Accepted invalid metaprog evidence')
    for out,marker,code in [('MPO_CHECK line1','MP_CHECK ',1),('MP_CHECK ','MP_CHECK ',-11),('MP_CHECK ','MP_CHECK ',None),
                            ('MP_CHECK: AddressSanitizer','MP_CHECK ',1),('MP_CHECK: ThreadSanitizer','MP_CHECK ',1),('MP_CHECK: runtime error:','MP_CHECK ',1)]:
        assert not assertion(out,marker,code)
    assert assertion('MP_CHECK OCF line1','MP_CHECK OCF line',1)
    return len(edits)+7


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output');p.add_argument('--self-test',action='store_true');a=p.parse_args()
    d=read_json(ROOT/a.report);validate(d);n=selftest(d) if a.self_test else 0
    result={'passed':True,'accepted':False,'negative_controls':n,'report':a.report,'report_sha256':sha(ROOT/a.report),'checker_sha256':sha(Path(__file__))}
    if a.output:dump(ROOT/a.output,result)
    print(json.dumps(result))
