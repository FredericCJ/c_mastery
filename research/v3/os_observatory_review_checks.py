"""Focused regression checks for observer/runner review; Linux/WSL only."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT))
from cmastery import cli
spec=importlib.util.spec_from_file_location('cgroup_run',ROOT/'labs/os_observatory/cgroup_run.py')
cgroup=importlib.util.module_from_spec(spec); spec.loader.exec_module(cgroup)
ENV={'capabilities':{k:{'available':True} for k in ['gcc','cmake','ninja']}}

class ReviewChecks(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix='os review ')
        self.addCleanup(self.temp.cleanup)
        self.folder=Path(self.temp.name)

    def workspace(self,project='PR-T49-03'):
        workspace=self.folder/'work'
        cli.instantiate(project,workspace,family_name='os_observatory')
        return workspace

    def test_target_cannot_mislabel_unrelated_project(self):
        with self.assertRaisesRegex(ValueError,'does not support'):
            cli.run_checks(self.workspace(),'gcc','debug',target='freertos-posix')

    def test_missing_external_is_unavailable(self):
        with patch.object(cli,'doctor',return_value=ENV):
            result=cli.run_checks(self.workspace('PR-T52-11'),'gcc','debug',target='freertos-posix')
        self.assertEqual(result['status'],'unavailable')
        self.assertIn('external source freertos',result['reason'])
        self.assertEqual(result['physical_observations'],'not-run')

    def test_dirty_pinned_external_rejected(self):
        workspace=self.workspace('PR-T52-11'); dependency=self.folder/'external'; dependency.mkdir()
        contract=cli.read(workspace/'family.json')['external_dependencies']['freertos']
        for name in contract['required_files']:
            file=dependency/name; file.parent.mkdir(parents=True,exist_ok=True); file.write_text('placeholder')
        answers=[{'exit_code':0,'stdout':contract['commit']+'\n'}, {'exit_code':0,'stdout':' M tasks.c\n'}]
        with patch.object(cli,'doctor',return_value=ENV), patch.object(cli,'invoke',side_effect=answers):
            with self.assertRaisesRegex(ValueError,'clean pinned commit'):
                cli.run_checks(workspace,'gcc','debug',target='freertos-posix',external={'freertos':dependency})

    def test_tsan_runtime_failure_not_a_test_pass(self):
        answers=[{'exit_code':0,'stdout':''},{'exit_code':66,'stderr':'unsupported runtime'}]
        with patch.object(cli,'doctor',return_value=ENV), patch.object(cli,'invoke',side_effect=answers):
            result=cli.run_checks(self.workspace(),'gcc','tsan')
        self.assertEqual(result['status'],'unavailable')
        self.assertEqual(result['steps'],[])
        self.assertEqual(len(result['diagnostic_runtime_probe']),2)

    def test_capture_is_recorded_not_accepted_and_tampering_is_stale(self):
        workspace=self.workspace(); family=cli.read(workspace/'family.json')
        family['targets']['board']={'physical':True}; cli.save(workspace/'family.json',family)
        (self.folder/'image.elf').write_bytes(b'fixture image')
        (self.folder/'trace.csv').write_text('tick,event\n0,boot\n')
        manifest={key:'fixture only' for key in ['board_revision','silicon_revision','clocks','cache_mpu','instrument',
                    'instrumentation_overhead','trace_overruns','observations']}
        manifest.update(target='board',source_sha256=cli.snapshot(workspace)['sha256'],images=['image.elf'],captures=['trace.csv'])
        cli.save(self.folder/'capture.json',manifest)
        result=cli.record_physical(workspace,self.folder/'capture.json')
        self.assertEqual(result['status'],'recorded'); self.assertIn('requires assessment',result['acceptance'])
        receipt=next((workspace/'evidence/physical').glob('*/receipt.json'))
        artifact=next(iter(result['artifacts']))
        (receipt.parent/artifact).write_bytes(b'changed')
        self.assertTrue(cli.report(workspace)['physical_records'][0]['stale'])

    def test_fake_cgroup_directory_rejected_without_running_workload(self):
        parent=self.folder/'fake'; parent.mkdir(); (parent/'cgroup.controllers').write_text('cpu memory')
        run=subprocess.run([sys.executable,str(ROOT/'labs/os_observatory/cgroup_run.py'),'--parent',str(parent),
                            '--',sys.executable,'-c','print("SHOULD_NOT_EXECUTE")'],capture_output=True,text=True,timeout=10)
        self.assertEqual(run.returncode,77)
        result=json.loads(run.stdout); self.assertEqual(result['status'],'unavailable')
        self.assertNotIn('stdout',result)
        self.assertEqual(sorted(p.name for p in parent.iterdir()),['cgroup.controllers'])

    def test_entry_failure_does_not_execute_workload(self):
        read_fd,write_fd=os.pipe()
        try:
            run=subprocess.Popen([sys.executable,str(ROOT/'labs/os_observatory/cgroup_run.py'),'--enter',
                                  str(self.folder/'nonexistent'),str(write_fd),sys.executable,'-c','print("SHOULD_NOT_EXECUTE")'],
                                  pass_fds=(write_fd,),stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
            os.close(write_fd); write_fd=None
            stdout,_=run.communicate(timeout=10); receipt=os.read(read_fd,4096).decode()
            self.assertEqual(run.returncode,77); self.assertEqual(stdout,''); self.assertTrue(receipt.startswith('unavailable:'))
        finally:
            os.close(read_fd)
            if write_fd is not None: os.close(write_fd)

    def test_mount_resolution_uses_longest_mount_and_octal_escapes(self):
        mounts='1 0 0:1 / / rw - ext4 root rw\n2 1 0:2 / /review\\040group rw - cgroup2 cgroup rw\n3 2 0:3 / /review\\040group/nested rw - tmpfs tmpfs rw\n'
        with patch.object(Path,'read_text',return_value=mounts):
            self.assertTrue(cgroup.cgroup2_mount(Path('/review group/child')))
            self.assertFalse(cgroup.cgroup2_mount(Path('/review group/nested/child')))

if __name__=='__main__': unittest.main(verbosity=2)
