"""Validate the complete ownership CMake campaign, without release acceptance."""
import argparse
import copy
import hashlib
import json
import os
import re
from pathlib import Path
from common import ROOT, dump, read_json, sha
from run_ownership_current import BASE, MODULES, PROFILES, DIAGNOSTICS, SYMBOLIZER_ENV, contract, assertion, junit, fence_diagnostic


def exact_fence_diagnostic(output):
    """Admit only the two named read-versus-destruction report sites."""
    assert fence_diagnostic(output,66)
    parts=output.split('WARNING: ThreadSanitizer:')[1:]
    assert 1<=len(parts)<=2
    for part in parts:
        trace=part.split('SUMMARY: ThreadSanitizer:',1)[0]
        assert part.startswith(' data race')
        accesses=re.findall(r'^\s*(?:Previous )?(read|write) of size (\d+) at (0x[0-9a-f]+)[^\n]*:\n((?:\s+#[^\n]*\n)+)',
                            trace,re.MULTILINE|re.IGNORECASE)
        assert len(accesses)==2 and {x[0].lower() for x in accesses}=={'read','write'}
        read=next(x for x in accesses if x[0].lower()=='read')
        write=next(x for x in accesses if x[0].lower()=='write')
        assert read[1:3]==write[1:3] and read[1] in {'4','8'}
        def frame(stack,function,path,line,first=False):
            number='0' if first else r'\d+'
            return re.search(r'^\s*#'+number+r' '+function+r' [^\n]*'+re.escape(path)+':'+str(line)+r'(?=[: (\n])',stack,re.MULTILINE)
        assert frame(read[3],'upgrader','labs/ownership/references/tests/tests.c',32,True)
        assert frame(write[3],'race_destroy','labs/ownership/references/tests/tests.c',28)
        assert frame(write[3],'ow_atomic_drop','assessor/ownership/ownership.c',154)
        if write[1]=='4':assert frame(write[3],'release','assessor/ownership/ownership.c',214,True)
        else:assert frame(write[3],'race_destroy','labs/ownership/references/tests/tests.c',28,True)
    return True


def validate(d,files=True):
    meta,specs,markers,controls=contract()
    all_controls={n for values in controls.values() for n in values}
    expected=set(markers)|all_controls
    assert d['family']=='ownership' and d['producer']=='tools/run_ownership_current.py'
    assert d['passed'] is True and d['status']=='passed' and d['scope_source_stable'] is True and d['external_inputs_stable'] is True
    assert (d['physical_validation'],d['learner_mastery'],d['independent_review'])==('not-run','not-assessed','not-run')
    assert d['inspected_paths'] and d['external_input_sha256']
    assert d['scope_input_sha256']==hashlib.sha256(json.dumps(d['inspected_paths'],sort_keys=True).encode()).hexdigest()
    assert {'tools/run_ownership.py','tools/run_ownership_current.py','tools/run_resilience_scoped.py',
            'assessor/ownership/ownership.c','assessor/errors/errors.c','assessor/algorithms/v4_algo.c',
            'assessor/foundations/foundation.c','assessor/peripheral_depth/dma/dma.c'}<=set(d['inspected_paths'])
    rows=d['executions'];assert len(rows)==932
    assert all(r['environment']==SYMBOLIZER_ENV for r in rows)
    assert '/usr/bin/'+Path('/usr/bin/addr2line').name in d['external_input_sha256'] or any(p.endswith('/x86_64-linux-gnu-addr2line') for p in d['external_input_sha256'])
    investigation=rows[d['fence_investigation_guard_execution']]
    assert investigation['exit_code']==0 and investigation['label']=='existing-fence-investigation-guard'
    assert investigation['command'][1].endswith('/tools/check_ownership_references.py') and investigation['command'][2]=='--self-test'
    assert {'research/v5/modern/ownership_references_validation.json','research/v5/modern/ownership_references_investigation_validation.json',
            'research/v5/modern/ownership_references_resources.json','research/v5/modern/ownership_references_research_gate.json'}<=set(d['inspected_paths'])
    assert len(d['profiles'])==8 and {p['compiler']+'-'+p['profile'] for p in d['profiles']}==set(PROFILES)
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
        assert p['anchors']==meta['tests'] and p['original_anchors']==['ownership.'+a for a in BASE]
        assert set(p['supplementary_cases'])==set(markers)-{'ownership.'+a for a in BASE} and len(p['supplementary_cases'])==63
        assert p['controls_by_module']==controls and set(p['compiled_mutant_controls'])==all_controls and len(p['compiled_mutant_controls'])==88
        assert p['ctest_count']==161 and len(p['test_results'])==161
        assert {r['test'] for r in p['test_results']}==expected
        failed={r['test'] for r in p['test_results'] if not r['passed']}
        if p['profile']=='tsan':
            assert p['thread_instrumentation_confirmed'] is True and p['diagnostic_clean'] is False
            assert failed=={'ownership.references_race'} and len(p['expected_diagnostics'])==1
            diagnostic=p['expected_diagnostics'][0]
            assert diagnostic['test']=='ownership.references_race' and diagnostic['observed'] is True and diagnostic['diagnostic_clean'] is False and diagnostic['expected_exit_code']==66
            r=rows[diagnostic['execution']]
            assert r['kind']=='expected-fence-diagnostic' and r['exit_code']==66 and r['label']==label+'-original-fence-race'
            assert r['command'][-1]=='references_race' and Path(r['command'][0]).name=='ownership-references'
            assert diagnostic['execution'] not in used;used.add(diagnostic['execution'])
            variant=p['stronger_order_comparison']
            assert variant['passed'] is True and variant['original_provider_unchanged'] is True and variant['changed_memory_order'] is True
            cc=rows[variant['compile_execution']];run=rows[variant['run_execution']]
            assert cc['exit_code']==0 and run['exit_code']==0 and cc['label']==label+'-stronger-build' and run['label']==label+'-stronger-race'
            assert '-std=c17' in cc['command'] and '-fsanitize=thread' in cc['command'] and any(x.endswith('/tests/ordering_adapter.c') for x in cc['command'])
            assert run['command']==[cc['command'][-1],'references_race']
            if files:
                assert fence_diagnostic(logs[r['log']],r['exit_code']) and exact_fence_diagnostic(logs[r['log']])
                out=logs[run['log']];assert 'ORC_RACE synchronized-start rounds256' in out and not any(x in out for x in DIAGNOSTICS)
        else:
            assert not failed and p['diagnostic_clean'] is True and not p['expected_diagnostics'] and 'stronger_order_comparison' not in p
        assert len(p['direct_controls'])==88 and {r['test'] for r in p['direct_controls']}==all_controls
        assert len(p['oracle_results'])==11 and {r['module'] for r in p['oracle_results']}==set(MODULES)|{'original'}
        listrow=rows[p['list_execution']];ctest=rows[p['ctest_execution']]
        assert listrow['label']==label+'-list' and listrow['exit_code']==0 and '--show-only=json-v1' in listrow['command']
        assert ctest['label']==label+'-ctest' and ctest['exit_code']==(8 if p['profile']=='tsan' else 0) and '--verbose' in ctest['command'] and '--output-junit' in ctest['command']
        if p['profile']=='tsan':
            assert p['ctest_execution'] not in used;used.add(p['ctest_execution'])
        for r in p['direct_controls']:
            assert r['rejected'] is True and r['marker']==markers[r['anchor']]
            record=rows[r['execution']]
            assert record['kind']=='control' and record['label']==label+'-'+r['test'] and record['exit_code']==1
            assert record['command'][-1]=='{}'.format(r['anchor'].removeprefix('ownership.'))
            assert '-mutant-' in record['command'][0]
            assert r['execution'] not in used;used.add(r['execution'])
            if files:assert assertion(logs[record['log']],r['marker'],record['exit_code'])
        for r in p['oracle_results']:
            record=rows[r['execution']];assert r['passed'] is True and record['exit_code']==0 and record['label']==label+'-oracle-'+r['module']
            oracle='tools/verify_ownership_fixtures.py' if r['module']=='original' else specs[r['module']]['oracle']
            assert record['command'][1].endswith('/'+oracle) and record['command'][2]=='--driver'
        if files:
            tests=json.loads(logs[listrow['log']])['tests']
            assert len(tests)==161 and {t['name'] for t in tests}==expected
            assert p['test_results']==junit(ROOT/p['junit']['path'])
            assert sha(ROOT/p['junit']['path'])==p['junit']['sha256']
            out=logs[ctest['log']]
            assert all(m in out for s in specs.values() for m in s['observation_markers'])
            if p['profile']=='tsan':
                import xml.etree.ElementTree as ET
                assert '1 tests failed out of 161' in out
                for t in ET.parse(ROOT/p['junit']['path']).getroot().iter('testcase'):
                    output=t.findtext('system-out','')
                    if t.attrib['name']=='ownership.references_race':assert exact_fence_diagnostic(output)
                    else:assert not any(x in output for x in DIAGNOSTICS)
            else:assert '100% tests passed' in out and not any(x in out for x in DIAGNOSTICS)
    assert len(d['learner_stub_rejections'])==73 and {r['test'] for r in d['learner_stub_rejections']}==set(markers)
    for r in d['learner_stub_rejections']:
        assert r['rejected'] is True and r['marker']==markers[r['test']]
        record=rows[r['execution']]
        assert record['kind']=='learner' and record['label']=='learner-'+r['test'] and record['exit_code']==8
        assert '-R' in record['command'] and record['command'][record['command'].index('-R')+1].startswith('^ownership\\.')
        assert r['execution'] not in used;used.add(r['execution'])
        if files:
            out=logs[record['log']]
            assert r['marker'] in out and '1/1 Test' in out and '(Failed)' in out and not any(x in out for x in DIAGNOSTICS)
    assert len(used)==8*88+73+4
    for i,r in enumerate(rows):
        assert r['exit_code']==0 or i in used
        if r['kind'] in ['control','learner','expected-fence-diagnostic']:assert i in used
    if files:
        result=json.loads(logs[investigation['log']]);assert result['passed'] is True and result['negative_controls']==25
        # Reuse the existing strict contract, including the preserved original
        # diagnostic, stronger comparison and bounded RC11 limitations.
        from check_ownership_references import check
        check(read_json(ROOT/'research/v5/modern/ownership_references_validation.json'),read_json(ROOT/'research/v5/modern/ownership_references_investigation_validation.json'))
    assert len(d['compiler_dependency_closures'])==9 and {r['profile'] for r in d['compiler_dependency_closures']}==set(PROFILES)|{'learner'}
    for r in d['compiler_dependency_closures']:
        assert r['repository_inputs'] and r['external_inputs']
        assert set(r['repository_inputs'])<=set(d['inspected_paths']) and set(r['external_inputs'])<=set(d['external_input_sha256'])
        if files:
            artifact=r['compile_commands'];assert sha(ROOT/artifact['path'])==artifact['sha256']
            cc=read_json(ROOT/artifact['path']);assert cc and all('-std=c17' in c['command'] for c in cc)
            if r['profile'].endswith('-tsan'):assert all('-fsanitize=thread' in c['command'] for c in cc)
    return True


def selftest(d):
    edits=[lambda x:x.update(passed=False),lambda x:x.update(scope_source_stable=False),lambda x:x.update(external_inputs_stable=False),
           lambda x:x.update(physical_validation='passed'),lambda x:x.update(learner_mastery='passed'),lambda x:x['profiles'].pop(),
           lambda x:x['profiles'][0]['supplementary_cases'].pop(),lambda x:x['profiles'][0]['test_results'].pop(),
           lambda x:x['profiles'][0]['test_results'][0].update(passed=False),lambda x:x['profiles'][0]['direct_controls'].pop(),
           lambda x:x['profiles'][0]['controls_by_module']['references'].pop(),lambda x:x['profiles'][0]['oracle_results'].pop(),
           lambda x:x['profiles'][0].update(c17_confirmed=False),lambda x:x['profiles'][3].update(thread_instrumentation_confirmed=False),
           lambda x:x['profiles'][0].update(observation_markers_confirmed=False),lambda x:x['learner_stub_rejections'].pop(),
           lambda x:x['learner_stub_rejections'][10].update(marker='OW_CHECK:'),lambda x:x['learner_stub_rejections'][10].update(rejected=False),
           lambda x:x['compiler_dependency_closures'].pop(),lambda x:x['executions'][x['profiles'][0]['ctest_execution']].update(exit_code=1),
           lambda x:x['executions'][x['profiles'][0]['direct_controls'][0]['execution']].update(exit_code=-11),
           lambda x:x['executions'][x['learner_stub_rejections'][0]['execution']].update(exit_code=0),lambda x:x.update(scope_input_sha256='wrong'),
           lambda x:x['profiles'][3].update(diagnostic_clean=True),lambda x:x['profiles'][3]['expected_diagnostics'].clear(),
           lambda x:x['profiles'][3]['expected_diagnostics'][0].update(test='ownership.references_threads'),
           lambda x:x['profiles'][3]['stronger_order_comparison'].update(changed_memory_order=False),
           lambda x:x['executions'][x['profiles'][3]['expected_diagnostics'][0]['execution']].update(exit_code=0),
           lambda x:x['executions'][0].update(environment={'TSAN_OPTIONS':'suppressions=ignore.txt'})]
    for edit in edits:
        changed=copy.deepcopy(d);edit(changed)
        try:validate(changed,False)
        except (AssertionError,KeyError,ValueError,IndexError):continue
        raise AssertionError('Accepted invalid ownership evidence')
    for out,marker,code in [('OW_CHECK OCF line1','OW_CHECK:',1),('OW_CHECK:','OW_CHECK:',-11),('OW_CHECK:','OW_CHECK:',None),
                            ('OW_CHECK: AddressSanitizer','OW_CHECK:',1),('OW_CHECK: ThreadSanitizer','OW_CHECK:',1),('OW_CHECK: runtime error:','OW_CHECK:',1)]:
        assert not assertion(out,marker,code)
    assert assertion('OW_CHECK OCF line1','OW_CHECK OCF line',1)
    sample=('WARNING: ThreadSanitizer: data race\n'
            '  Write of size 8 at 0x1234 by thread T1:\n'
            '    #0 race_destroy /repo/labs/ownership/references/tests/tests.c:28\n'
            '    #1 ow_atomic_drop /repo/assessor/ownership/ownership.c:154\n\n'
            '  Previous read of size 8 at 0x1234 by thread T2:\n'
            '    #0 upgrader /repo/labs/ownership/references/tests/tests.c:32\n\n'
            'SUMMARY: ThreadSanitizer: data race\nORC_RACE synchronized-start rounds256')
    assert exact_fence_diagnostic(sample)
    bad_traces=[sample.replace('upgrader','unrelated'),sample.replace('race_destroy','unrelated'),
                sample+'\nWARNING: ThreadSanitizer: heap-use-after-free',sample+'\nWARNING: ThreadSanitizer: data race\nunrelated',
                sample.replace('tests.c:32','tests.c:33'),sample.replace('tests.c:28','tests.c:29'),
                sample.replace('ownership.c:154','ownership.c:155'),sample.replace('Previous read of size 8','Previous read of size 4'),
                sample.replace('0x1234 by thread T2','0x5678 by thread T2'),sample.replace('Previous read','Previous write'),
                sample.replace('#0 upgrader','#1 upgrader')]
    for text in bad_traces:
        try:exact_fence_diagnostic(text)
        except AssertionError:continue
        raise AssertionError('Accepted an unrelated diagnostic')
    return len(edits)+7+len(bad_traces)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output');p.add_argument('--self-test',action='store_true');a=p.parse_args()
    d=read_json(ROOT/a.report);validate(d);n=selftest(d) if a.self_test else 0
    result={'passed':True,'accepted':False,'negative_controls':n,'report':a.report,'report_sha256':sha(ROOT/a.report),'checker_sha256':sha(Path(__file__))}
    if a.output:dump(ROOT/a.output,result)
    print(json.dumps(result))
