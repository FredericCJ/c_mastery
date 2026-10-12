"""Exercise runner failure semantics, workspace isolation and stale evidence."""
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery import cli
from common import ROOT,dump
from v2_common import source_identity

class RunnerContracts(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix='cmastery CLI ')
        self.workspace=Path(self.temp.name)/'workspace'; self.workspace.mkdir()
        self.project='PR-T46-01'
        cli.save(self.workspace/'.cmastery/project.json',{'project':self.project,'family':'runtime','source_catalog':{'manual_evidence':['transfer']}})
        cli.save(self.workspace/'family.json',{'projects':[{'id':self.project,'tests':['case.one']}]})
        (self.workspace/'CMakeLists.txt').write_text('# test-only runner adapter',encoding='utf-8')
        self.environment={'capabilities':{k:{'available':True} for k in ['gcc','clang','cmake','ninja']}}
    def tearDown(self): self.temp.cleanup()
    def fake_run(self,xml):
        def invoke(command,**kwargs):
            if command[0]=='ctest':
                p=Path(command[command.index('--output-junit')+1]);p.parent.mkdir(parents=True,exist_ok=True);p.write_text(xml,encoding='utf-8')
            return {'command':command,'exit_code':0,'stdout':'','stderr':''}
        return invoke
    def test_project_selection_and_scoped_success(self):
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one" status="run"/></testsuite>')):
            result=cli.run_checks(self.workspace,'gcc','debug')
        self.assertEqual(result['status'],'passed')
        self.assertIn('^(case\\.one)$',result['steps'][-1]['command'])
        self.assertEqual(cli.report(self.workspace)['unresolved_claims'],['transfer'])
    def test_skipped_is_not_success(self):
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one"><skipped/></testcase></testsuite>')):
            self.assertEqual(cli.run_checks(self.workspace,'gcc','debug')['status'],'unavailable')
    def test_missing_required_test_is_not_success(self):
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="wrong"/></testsuite>')):
            self.assertEqual(cli.run_checks(self.workspace,'gcc','debug')['status'],'unavailable')
    def test_empty_suite_is_not_success(self):
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite/>')):
            self.assertEqual(cli.run_checks(self.workspace,'gcc','debug',all_tests=True)['status'],'unavailable')
    def test_removed_dependency_uses_fresh_cache_and_receipt(self):
        dep=Path(self.temp.name)/'runtime dependency';dep.mkdir()
        cli.save(dep/'family.json',{'family':'runtime'})
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one"/></testsuite>')):
            first=cli.run_checks(self.workspace,'gcc','debug',dependencies={'runtime':dep})
            second=cli.run_checks(self.workspace,'gcc','debug')
        a=first['steps'][0]['command'];b=second['steps'][0]['command']
        self.assertNotEqual(a[a.index('-B')+1],b[b.index('-B')+1])
        self.assertTrue(any('CMASTERY_RUNTIME_ROOT=' in arg for arg in a))
        self.assertFalse(any('CMASTERY_RUNTIME_ROOT=' in arg for arg in b))
        self.assertNotIn('external_sources',second)
    def test_dependency_accepts_explicit_and_legacy_identity(self):
        dep=Path(self.temp.name)/'storage dependency';dep.mkdir()
        for declaration in [{'family':'storage'},{'id':'storage'},{'family':'storage','id':'storage'}]:
            with self.subTest(declaration=declaration):
                cli.save(dep/'family.json',declaration)
                with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one"/></testsuite>')):
                    result=cli.run_checks(self.workspace,'gcc','debug',dependencies={'storage':dep})
                self.assertEqual(result['status'],'passed')
                self.assertIn('storage',result['dependencies'])
    def test_dependency_rejects_missing_mismatched_or_conflicting_identity(self):
        dep=Path(self.temp.name)/'storage dependency';dep.mkdir()
        declarations=[{}, {'family':'runtime'}, {'id':'runtime'},
                      {'family':'storage','id':'runtime'}, {'family':'runtime','id':'storage'},
                      {'family':None,'id':'storage'}]
        for declaration in declarations:
            with self.subTest(declaration=declaration):
                cli.save(dep/'family.json',declaration)
                with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke') as run:
                    with self.assertRaisesRegex(ValueError,'[Dd]ependency family identity'):
                        cli.run_checks(self.workspace,'gcc','debug',dependencies={'storage':dep})
                    run.assert_not_called()
    def test_kernel_assessor_dependencies_use_actual_legacy_catalog(self):
        for family in ['kernel','kernel_smp']:
            with self.subTest(family=family):
                cli.save(self.workspace/'.cmastery/project.json',{'project':self.project,'family':family})
                with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one"/></testsuite>')):
                    result=cli.run_checks(self.workspace,'gcc','debug',assessor=True)
                self.assertEqual(result['status'],'passed')
                self.assertEqual(set(result['dependencies']),{'runtime','storage'})
                configure=result['steps'][0]['command']
                self.assertIn('-DCMASTERY_STORAGE_ROOT='+str(ROOT/'assessor/storage'),configure)
                self.assertIn('-DCMASTERY_STORAGE_INCLUDE='+str(ROOT/'labs/storage/include'),configure)
    def test_unavailable_tool_does_not_execute(self):
        self.environment['capabilities']['gcc']['available']=False
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke') as run:
            self.assertEqual(cli.run_checks(self.workspace,'gcc','debug')['status'],'unavailable');run.assert_not_called()
    def test_failed_build_stops_tests(self):
        calls=[]
        def invoke(command,**kwargs):
            calls.append(command)
            return {'command':command,'exit_code':1 if '--build' in command else 0}
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=invoke):
            self.assertEqual(cli.run_checks(self.workspace,'gcc','debug')['status'],'failed')
        self.assertEqual(len(calls),2)
    def test_changed_source_invalidates(self):
        cli.save(self.workspace/'evidence/latest.json',{'status':'passed','source':cli.snapshot(self.workspace)})
        (self.workspace/'new.c').write_text('int changed;',encoding='utf-8')
        self.assertTrue(cli.report(self.workspace)['evidence']['stale'])
    def test_changed_dependency_invalidates(self):
        dep=Path(self.temp.name)/'dependency';dep.mkdir();(dep/'a.c').write_text('old')
        cli.save(self.workspace/'evidence/latest.json',{'status':'passed','source':cli.snapshot(self.workspace),'external_sources':{str(dep):cli.snapshot(dep)['sha256']}})
        (dep/'a.c').write_text('new')
        self.assertTrue(cli.report(self.workspace)['evidence']['stale'])
    def test_init_is_unsolved_and_non_overwriting(self):
        target=Path(self.temp.name)/'path with spaces'
        cli.instantiate(self.project,target)
        self.assertTrue((target/'CMakeLists.txt').is_file())
        self.assertFalse(any('assessor' in p.relative_to(target).parts for p in target.rglob('*')))
        self.assertFalse((target/'evidence/latest.json').exists())
        with self.assertRaises(ValueError): cli.instantiate(self.project,target)
        with self.assertRaises(ValueError): cli.instantiate(self.project,ROOT/'labs'/'should-not-create')
    def test_unknown_project_creates_no_destination(self):
        target=Path(self.temp.name)/'unknown'
        with self.assertRaises(ValueError): cli.instantiate('PR-UNKNOWN',target)
        self.assertFalse(target.exists())
    def test_advance_preserves_code_archives_evidence_and_rejects_other_family(self):
        target=Path(self.temp.name)/'advancing learner'
        cli.instantiate('PR-T46-01',target)
        source=target/'src/cvm.c'; source.write_text('/* learner implementation */',encoding='utf-8')
        cli.save(target/'evidence/latest.json',{'project':'PR-T46-01','status':'passed'})
        result=cli.advance(target,'PR-T46-02')
        self.assertEqual(source.read_text(),'/* learner implementation */')
        archive=Path(result['archived_evidence'])
        self.assertEqual(cli.read(archive/'latest.json')['project'],'PR-T46-01')
        self.assertEqual(cli.read(archive/'project.json')['project'],'PR-T46-01')
        self.assertEqual(cli.report(target)['evidence']['status'],'not-run')
        self.assertEqual(cli.read(target/'.cmastery/project.json')['project'],'PR-T46-02')
        with self.assertRaises(ValueError): cli.advance(target,'PR-T47-01')
        with self.assertRaises(ValueError): cli.advance(target,'PR-T46-02')

    def test_target_overrides_tests_and_records_cpu_count(self):
        cli.save(self.workspace/'family.json',{'default_target':'smp','targets':{
            'smp':{'cpu_counts':[1,2,4],'capabilities':[],'evidence_kind':'emulator'}},
            'projects':[{'id':self.project,'tests':['wrong'],'target_tests':{'smp':['case.one']}}]})
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one" status="run"/></testsuite>')):
            result=cli.run_checks(self.workspace,'gcc','debug',target='smp',cpus=4)
        self.assertEqual(result['status'],'passed')
        self.assertEqual(result['cpu_count'],4)
        self.assertEqual(result['evidence_kind'],'emulator')
        self.assertIn('-DCMASTERY_CPUS=4',result['steps'][0]['command'])
        with self.assertRaises(ValueError): cli.run_checks(self.workspace,'gcc','debug',target='undeclared')

    def test_target_tool_missing_and_physical_build_does_not_claim_observation(self):
        cli.save(self.workspace/'family.json',{'default_target':'board','targets':{
            'board':{'capabilities':['arm_compiler'],'physical':True,'evidence_kind':'physical'}},
            'projects':[{'id':self.project,'tests':['case.one']}]})
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke') as run:
            self.assertEqual(cli.run_checks(self.workspace,'gcc','debug')['status'],'unavailable')
            run.assert_not_called()
        self.environment['capabilities']['arm_compiler']={'available':True}
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',side_effect=self.fake_run('<testsuite><testcase name="case.one"/></testsuite>')):
            result=cli.run_checks(self.workspace,'gcc','debug')
        self.assertEqual(result['status'],'passed')
        self.assertEqual(result['physical_observations'],'not-run')
        self.assertEqual(result['evidence_kind'],'target-build-and-software-checks')

    def test_physical_capture_is_recorded_not_automatically_accepted(self):
        cli.save(self.workspace/'family.json',{'targets':{'test-board':{'physical':True}}})
        root=Path(self.temp.name)
        (root/'test-image.elf').write_bytes(b'TEST ONLY, NOT A FIRMWARE IMAGE')
        (root/'test-capture.csv').write_text('TEST ONLY, NOT HARDWARE EVIDENCE')
        manifest={key:'TEST ONLY' for key in ['board_revision','silicon_revision','clocks','cache_mpu','instrument',
              'instrumentation_overhead','trace_overruns','observations']}
        manifest.update(target='test-board',source_sha256=cli.snapshot(self.workspace)['sha256'],
                        images=['test-image.elf'],captures=['test-capture.csv'])
        cli.save(root/'manifest.json',manifest)
        result=cli.record_physical(self.workspace,root/'manifest.json')
        self.assertEqual(result['status'],'recorded')
        self.assertEqual(len(result['artifacts']),2)
        self.assertEqual(cli.report(self.workspace)['physical_records'][0]['status'],'recorded')
        (self.workspace/'changed.c').write_text('changed source')
        self.assertTrue(cli.report(self.workspace)['physical_records'][0]['stale'])
        with self.assertRaises(ValueError): cli.record_physical(self.workspace,root/'manifest.json')

    def test_unavailable_tsan_runtime_is_not_a_project_failure_or_pass(self):
        with patch.object(cli,'doctor',return_value=self.environment),patch.object(cli,'invoke',return_value={'exit_code':1,'stderr':'unsupported runtime'}):
            result=cli.run_checks(self.workspace,'gcc','tsan')
        self.assertEqual(result['status'],'unavailable')
        self.assertEqual(result['steps'],[])

def main(receipt=None):
    identity=source_identity()
    result=unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(RunnerContracts))
    passed=result.wasSuccessful() and identity['sha256']==source_identity()['sha256']
    edition='v3' if (ROOT/'curriculum/baseline_v2.json').exists() else 'v2'
    dump(ROOT/receipt if receipt else ROOT/'reviews'/edition/'cli_checks.json',{'status':'passed' if passed else 'failed','passed':passed,
        'input_sha256':identity['sha256'],'tests':result.testsRun,'failures':len(result.failures),'errors':len(result.errors),
        'scope':'Runner semantics with controlled subprocess responses plus real workspace isolation. Actual C execution is in lab_matrix.json.'})
    return 0 if passed else 1

if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--receipt')
    raise SystemExit(main(parser.parse_args().receipt))
