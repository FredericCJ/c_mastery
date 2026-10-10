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

def main():
    identity=source_identity()
    result=unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(RunnerContracts))
    passed=result.wasSuccessful() and identity['sha256']==source_identity()['sha256']
    dump(ROOT/'reviews/v2/cli_checks.json',{'status':'passed' if passed else 'failed','passed':passed,
        'input_sha256':identity['sha256'],'tests':result.testsRun,'failures':len(result.failures),'errors':len(result.errors),
        'scope':'Runner semantics with controlled subprocess responses plus real workspace isolation. Actual C execution is in lab_matrix.json.'})
    return 0 if passed else 1

if __name__=='__main__': raise SystemExit(main())
