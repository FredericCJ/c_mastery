"""Isolated strict manifest/staging guards; no private or current receipt copies."""
import argparse
from contextlib import ExitStack
from copy import deepcopy
import hashlib
import io
import json
from pathlib import Path
import tempfile
import subprocess
import unittest
from unittest.mock import patch
import zipfile

import candidate_author_environment as env


class CandidateEnvironmentGuards(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)/'source';self.root.mkdir();self.out=Path(self.temp.name)/'candidate';self.out.mkdir()
        self.row={'id':'fixture-exact-author-row','dimensions':{'implementation':{'state':'bounded-fixture'}}}
        self.write(env.ACTIVE,{'updates':[self.row]})
        self.input='build/capture/required.bin';self.write(self.input,b'isolated exact author evidence\x00')
        self.provider='research/v5/expansion/fixture/completion_dispositions.json'
        self.write(self.provider,{'updates':[self.row],'source_sha256':{self.input:env.sha(self.root/self.input),'/external/not-copied.bin':'a'*64}})
        self.names={env.ACTIVE,self.provider}
        files={p:env.sha(self.root/p) for p in self.names};self.identity={'files':files,'sha256':hashlib.sha256(json.dumps(files,sort_keys=True).encode()).hexdigest()}
        self.expected=env.discover(self.root,self.identity,self.names)

    def write(self,name,value):
        path=self.root/name;path.parent.mkdir(parents=True,exist_ok=True)
        path.write_bytes(value if isinstance(value,bytes) else (json.dumps(value)+'\n').encode());return path

    def test_actual_stage_is_exact_and_archive_remains_excluded(self):
        archive=Path(self.temp.name)/'source.zip'
        with zipfile.ZipFile(archive,'w') as z:
            for name in self.names:z.write(self.root/name,name)
        original=archive.read_bytes()
        result=env.stage(self.root,self.out,self.expected,self.expected,self.names)
        self.assertEqual(result['files'],1);self.assertFalse(result['archive_self_contained'])
        self.assertEqual((self.out/self.input).read_bytes(),(self.root/self.input).read_bytes())
        self.assertEqual(archive.read_bytes(),original)
        with zipfile.ZipFile(archive) as z:self.assertNotIn(self.input,z.namelist())

    def test_missing_opt_in_is_not_silent(self):
        with self.assertRaisesRegex(ValueError,'ZIP alone'):env.require_opt_in(self.expected,None)
        env.require_opt_in(self.expected,deepcopy(self.expected))

    def test_manifest_generation_and_actual_packager_select_same_paths(self):
        import package_v4
        manifest_name='reviews/environment.json'
        with patch.object(package_v4,'ROOT',self.root):
            before={p.relative_to(self.root).as_posix() for p in package_v4.source_files(self.identity,'reviews/v5')}
            expected=env.discover(self.root,self.identity,before|{manifest_name})
            self.write(manifest_name,expected)
            after={p.relative_to(self.root).as_posix() for p in package_v4.source_files(self.identity,'reviews/v5')}
        self.assertEqual(after,before|{manifest_name})
        self.assertNotIn(self.input,after)
        env.validate_manifest(expected,env.discover(self.root,self.identity,after))

    def release_fixture(self):
        """Accepted isolated fixture; never calls the repository acceptance gate."""
        self.write('reviews/v5/acceptance.json',{'accepted':True,'input_sha256':self.identity['sha256']})
        self.write('build/html/index.html',b'<p>isolated publication</p>\n')
        self.write('c_programming_mastery_curriculum.md',b'# Isolated publication\n')
        import package_v4
        name='reviews/environment.json'
        with patch.object(package_v4,'ROOT',self.root):
            names={p.relative_to(self.root).as_posix() for p in package_v4.source_files(self.identity,'reviews/v5')}
        manifest=env.discover(self.root,self.identity,names|{name});self.write(name,manifest)
        return name,manifest

    def release_context(self):
        import package_v4
        context=ExitStack();context.enter_context(patch.object(package_v4,'ROOT',self.root))
        context.enter_context(patch.object(package_v4,'source_identity',return_value=self.identity))
        return context

    def fixture_rebuild(self,command,**kwargs):
        cwd=Path(kwargs['cwd']);target=cwd/'build/html/index.html';target.parent.mkdir(parents=True)
        target.write_bytes((self.root/'build/html/index.html').read_bytes())
        return subprocess.CompletedProcess(command,0,'isolated rebuild seam','')

    def test_release_acceptance_precedes_manifest_or_archive_work(self):
        import package_v4
        name,_=self.release_fixture()
        def refused():raise ValueError('actual acceptance refusal fixture')
        with self.release_context(),patch.object(env,'discover') as discover,patch.object(package_v4,'write_zip') as archive,patch.object(package_v4,'promote') as promote:
            with self.assertRaisesRegex(ValueError,'acceptance refusal'):
                package_v4.package(accept=refused,receipts='reviews/v5',author_environment_manifest=Path(name))
            discover.assert_not_called();archive.assert_not_called();promote.assert_not_called()
        for accepted,digest in ((False,self.identity['sha256']),(True,'0'*64)):
            self.write('reviews/v5/acceptance.json',{'accepted':accepted,'input_sha256':digest})
            with self.release_context(),patch.object(env,'discover') as discover,patch.object(package_v4,'write_zip') as archive:
                with self.assertRaisesRegex(ValueError,'Current accepted'):
                    package_v4.package(accept=lambda:None,receipts='reviews/v5',author_environment_manifest=Path(name))
                discover.assert_not_called();archive.assert_not_called()

    def test_release_archive_hash_uses_bounded_reads(self):
        import package_v4
        path=self.write('build/large-archive.zip',b'isolated bounded hash fixture\0'*4096)
        expected=hashlib.sha256(path.read_bytes()).hexdigest()
        with patch.object(Path,'read_bytes',side_effect=AssertionError('Unbounded file read')):
            self.assertEqual(package_v4.sha(path),expected)

    def test_release_real_archive_selection_after_acceptance_matches_manifest(self):
        import package_v4
        name,manifest=self.release_fixture()
        def repeat_acceptance():self.write('reviews/v5/acceptance.json',{'accepted':True,'input_sha256':self.identity['sha256']})
        with self.release_context(),patch.object(package_v4.subprocess,'run',side_effect=self.fixture_rebuild):
            report=package_v4.package(accept=repeat_acceptance,receipts='reviews/v5',author_environment_manifest=Path(name))
        self.assertTrue(report['passed']);self.assertEqual(report['author_environment']['files'],1)
        with zipfile.ZipFile(self.root/'dist'/package_v4.ARCHIVES[0]) as archive:
            # write_zip adds its own manifest after selecting source files.
            selected=[r['path'] for r in json.loads(archive.read('artifact_manifest.json'))['files']]
            self.assertEqual(set(archive.namelist()),set(selected)|{'artifact_manifest.json'})
            self.assertEqual(env.names_hash(selected),manifest['archive_selection_sha256'])
            self.assertNotIn(self.input,archive.namelist())
            self.assertIn('reviews/v5/acceptance.json',archive.namelist());self.assertIn(name,archive.namelist())
        with zipfile.ZipFile(self.root/'dist'/package_v4.ARCHIVES[1]) as archive:self.assertNotIn(self.input,archive.namelist())
        self.assertTrue((self.root/report['extraction']/self.input).is_file())

    def test_release_new_acceptance_filename_requires_fresh_manifest(self):
        import package_v4
        name,_=self.release_fixture()
        def changed_acceptance_selection():self.write('reviews/v5/new-acceptance-detail.json',{'retained':True})
        with self.release_context(),patch.object(package_v4,'write_zip') as archive,patch.object(package_v4,'promote') as promote:
            with self.assertRaisesRegex(ValueError,'archive selection'):
                package_v4.package(accept=changed_acceptance_selection,receipts='reviews/v5',author_environment_manifest=Path(name))
            archive.assert_not_called();promote.assert_not_called()

    def test_release_default_does_not_provision_or_promote_missing_environment(self):
        import package_v4
        self.release_fixture()
        def missing_environment(command,**kwargs):
            self.assertFalse((Path(kwargs['cwd'])/self.input).exists())
            return subprocess.CompletedProcess(command,1,'','required author input absent')
        with self.release_context(),patch.object(package_v4.subprocess,'run',side_effect=missing_environment),patch.object(env,'stage') as stage,patch.object(package_v4,'promote') as promote:
            with self.assertRaisesRegex(ValueError,'Extracted-source build failed'):
                package_v4.package(accept=lambda:None,receipts='reviews/v5')
            stage.assert_not_called();promote.assert_not_called()

    def test_release_changed_supplied_input_prevents_promotion(self):
        import package_v4
        name,_=self.release_fixture()
        for location in ('supplier','extracted'):
            with self.subTest(location=location):
                # Each failed candidate is retained; selected source names stay unchanged.
                self.write(self.input,b'isolated exact author evidence\x00')
                def changing_rebuild(command,**kwargs):
                    result=self.fixture_rebuild(command,**kwargs)
                    path=(self.root if location=='supplier' else Path(kwargs['cwd']))/self.input
                    path.write_bytes(b'changed during extracted build')
                    return result
                with self.release_context(),patch.object(package_v4.subprocess,'run',side_effect=changing_rebuild),patch.object(package_v4,'promote') as promote:
                    with self.assertRaisesRegex(ValueError,'environment input changed'):
                        package_v4.package(accept=lambda:None,receipts='reviews/v5',author_environment_manifest=Path(name))
                    promote.assert_not_called()

    def test_exact_manifest_authority_fields_cannot_be_changed(self):
        mutations=[lambda d:d['inputs'].clear(),lambda d:d['inputs'].append(deepcopy(d['inputs'][0])),lambda d:d['inputs'][0].update(sha256='0'*64),lambda d:d['inputs'][0].update(destination='../escape'),lambda d:d['inputs'][0]['authority'][0].update(provider_sha256='0'*64),lambda d:d['inputs'][0]['authority'][0].update(active_ids=['another-row']),lambda d:d.update(source_identity_sha256='0'*64),lambda d:d.update(active_dispositions_sha256='0'*64),lambda d:d.update(archive_selection_sha256='0'*64),lambda d:d['inputs'][0].update(supplier_relative_path='/external/file')]
        for mutate in mutations:
            bad=deepcopy(self.expected);mutate(bad)
            with self.assertRaises(ValueError):env.validate_manifest(bad,self.expected)

    def test_no_external_absolute_supplier_is_copied(self):
        self.assertEqual(len(self.expected['external_inputs_not_copied']),1)
        self.assertFalse(self.expected['external_inputs_not_copied'][0]['copied'])
        env.stage(self.root,self.out,self.expected,self.expected,self.names)
        self.assertEqual([p.relative_to(self.out).as_posix() for p in self.out.rglob('*') if p.is_file()],[self.input])

    def test_nonmatching_and_historical_providers_do_not_authorize(self):
        other='build/unrelated.bin';self.write(other,b'not authorized')
        doc={'updates':[dict(self.row,id='not-active')],'source_sha256':{other:env.sha(self.root/other)}}
        self.write('research/v5/expansion/other/support.json',doc)
        doc['updates']=[self.row];self.write('research/v5/expansion/history/support.json',doc)
        self.assertEqual(env.discover(self.root,self.identity,self.names),self.expected)

    def test_duplicate_active_row_is_rejected(self):
        self.write(env.ACTIVE,{'updates':[self.row,self.row]})
        with self.assertRaisesRegex(ValueError,'Duplicate'):env.discover(self.root,self.identity,self.names)

    def test_stale_supplier_prevents_any_write(self):
        self.write(self.input,b'changed source')
        with self.assertRaisesRegex(ValueError,'supplier changed'):env.stage(self.root,self.out,self.expected,self.expected,self.names)
        self.assertEqual(list(self.out.iterdir()),[])

    def test_stale_excluded_pin_is_not_rehashed(self):
        self.write(self.input,b'changed source')
        with self.assertRaisesRegex(ValueError,'Stale excluded'):env.discover(self.root,self.identity,self.names)

    def test_existing_destination_and_archive_member_refused(self):
        target=self.out/self.input;target.parent.mkdir(parents=True);target.write_bytes(b'preserve')
        with self.assertRaisesRegex(ValueError,'overwrite'):env.stage(self.root,self.out,self.expected,self.expected,self.names)
        self.assertEqual(target.read_bytes(),b'preserve')
        with self.assertRaisesRegex(ValueError,'overlaps'):env.stage(self.root,self.out,self.expected,self.expected,self.names|{self.input})

    def test_changed_staged_bytes_are_not_accepted(self):
        env.stage(self.root,self.out,self.expected,self.expected,self.names)
        (self.out/self.input).write_bytes(b'changed by build')
        with self.assertRaisesRegex(ValueError,'input changed'):env.verify_staged(self.root,self.out,self.expected)

    def test_unlisted_neighbors_are_not_staged(self):
        self.write('build/capture/unlisted.bin',b'never copy a directory')
        env.stage(self.root,self.out,self.expected,self.expected,self.names)
        self.assertFalse((self.out/'build/capture/unlisted.bin').exists())

    def test_traversal_absolute_and_noncanonical_names_rejected(self):
        for name in ('../outside','build/../outside','/absolute','C:/absolute','build\\capture','build//capture','build/./capture'):
            with self.assertRaises(ValueError):env.bounded(self.out,name,exists=False)

    def test_source_symlink_rejected(self):
        original=self.root/self.input;outside=Path(self.temp.name)/'outside';outside.write_bytes(original.read_bytes());original.unlink()
        try:original.symlink_to(outside)
        except (OSError,NotImplementedError):self.skipTest('Symlink privilege unavailable')
        with self.assertRaisesRegex(ValueError,'redirected'):env.stage(self.root,self.out,self.expected,self.expected,self.names)

    def test_destination_parent_symlink_rejected(self):
        outside=Path(self.temp.name)/'outside-dir';outside.mkdir()
        try:(self.out/'build').symlink_to(outside,target_is_directory=True)
        except (OSError,NotImplementedError):self.skipTest('Symlink privilege unavailable')
        with self.assertRaisesRegex(ValueError,'redirected'):env.stage(self.root,self.out,self.expected,self.expected,self.names)
        self.assertEqual(list(outside.iterdir()),[])


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    output=(env.ROOT/args.output).absolute()
    if output.exists() or output.resolve()!=output or not output.is_relative_to((env.ROOT/'reviews').resolve()):raise ValueError('Fresh report under reviews required')
    stream=io.StringIO();result=unittest.TextTestRunner(stream=stream,verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(CandidateEnvironmentGuards))
    env.dump(output,dict(passed=result.wasSuccessful(),tests=result.testsRun,skipped=len(result.skipped),output=stream.getvalue(),source_sha256={p:env.sha(env.ROOT/p) for p in ['tools/candidate_author_environment.py','tools/check_candidate_author_environment.py','tools/check_candidate_exports_v5.py','tools/package_v4.py','tools/package_v5.py','tools/check_release_v5.py']},scope='Isolated exact-manifest, authority, path, immutable archive, local staging and acceptance-first release plumbing guards. Real ZIPs use temporary fixtures and a declared build seam; no real private input copied or repository release accepted.'))
    print(stream.getvalue());return int(not result.wasSuccessful())


if __name__=='__main__':raise SystemExit(main())
