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

    def correction_fixture(self,private=False):
        """Synthetic bytes and real correction semantics, not source inspection."""
        from resource_identity_v5 import digest
        self.correction_evidence='research/v5/expansion/fixture/identity_evidence.json'
        self.correction_previous='research/v5/expansion/fixture/history_identity_01/previous_registry.json'
        records={};entries=[];previous=[]
        self.captures=[]
        for index,short in enumerate(('hlc','crdt','delta','ot')):
            key='R-FIXTURE-'+short.upper()
            name=('research/private/fixture/'+short+'.pdf' if private else 'build/replicated_sources/'+short+'.pdf')
            self.write(name,('Synthetic binary '+short).encode()+b'\x00\xff')
            self.captures.append(name)
            prior=dict(id=key,title='Synthetic '+short,authors=['Placeholder'],edition='Fixture',year=2000,url='https://example.invalid/'+short)
            incoming=dict(prior,authors=['Fixture Author'])
            previous.append(prior)
            records[key]=dict(capture=name,sha256=env.sha(self.root/name),observation='Synthetic byte identity only')
            entries.append(dict(id=key,previous=prior,incoming=incoming,
                previous_sha256=digest(prior),incoming_sha256=digest(incoming),identity_fields=['authors']))
        self.write(self.correction_previous,{'resources':previous})
        self.write(self.correction_evidence,{'records':records})
        for entry in entries:
            entry['evidence']=dict(path=self.correction_evidence,sha256=env.sha(self.root/self.correction_evidence),id=entry['id'])
        self.correction_policy=dict(schema_version=1,previous_registry=dict(path=self.correction_previous,sha256=env.sha(self.root/self.correction_previous)),corrections=entries)
        self.write(env.CORRECTIONS,self.correction_policy)
        for name in env.CORRECTION_CONSUMERS:self.write(name,(env.ROOT/name).read_bytes())
        self.names.update({env.CORRECTIONS,self.correction_previous,self.correction_evidence,*env.CORRECTION_CONSUMERS})
        return env.discover(self.root,self.identity,self.names)

    def replace_correction_evidence(self,records):
        self.write(self.correction_evidence,{'records':records})
        for entry in self.correction_policy['corrections']:
            entry['evidence']['sha256']=env.sha(self.root/self.correction_evidence)
        self.write(env.CORRECTIONS,self.correction_policy)

    def test_four_correction_captures_stage_for_actual_consumer_without_zip_payloads(self):
        from resource_identity_v5 import corrections
        expected=self.correction_fixture()
        selected={row['destination']:row for row in expected['inputs']}
        self.assertEqual(set(selected),{self.input,*self.captures})
        for index,name in enumerate(self.captures):
            authority=selected[name]['authority'][0]
            self.assertEqual(authority['resource_id'],self.correction_policy['corrections'][index]['id'])
            self.assertEqual(authority['provider_sha256'],env.sha(self.root/env.CORRECTIONS))
            self.assertEqual(authority['evidence']['sha256'],env.sha(self.root/self.correction_evidence))
            self.assertEqual(authority['evidence']['pointer'],'/records/'+authority['resource_id']+'/capture')
            self.assertEqual(authority['consumer_source_sha256'],{p:env.sha(self.root/p) for p in env.CORRECTION_CONSUMERS})
            self.assertEqual(authority['previous_registry'],self.correction_policy['previous_registry'])
        archive=Path(self.temp.name)/'correction-source.zip'
        with zipfile.ZipFile(archive,'w') as z:
            for name in sorted(self.names):z.write(self.root/name,name)
        prior=archive.read_bytes()
        with zipfile.ZipFile(archive) as z:
            self.assertTrue(set(self.captures).isdisjoint(z.namelist()))
            z.extractall(self.out)
        with self.assertRaisesRegex(ValueError,'Missing or escaping resource path'):corrections(self.out)
        staged=env.stage(self.root,self.out,expected,expected,self.names)
        self.assertEqual(staged['files'],5)
        self.assertEqual(corrections(self.out),corrections(self.root))
        self.assertEqual(archive.read_bytes(),prior)

    def test_private_correction_captures_require_opt_in_and_stay_outside_archives(self):
        import package_v4
        expected=self.correction_fixture(private=True)
        with patch.object(package_v4,'ROOT',self.root):
            identity={'files':{name:env.sha(self.root/name) for name in self.names},'sha256':self.identity['sha256']}
            selected={p.relative_to(self.root).as_posix() for p in package_v4.source_files(identity,'reviews/v5')}
        self.assertTrue(set(self.captures).isdisjoint(selected))
        with self.assertRaisesRegex(ValueError,'ZIP alone'):env.require_opt_in(expected,None)
        env.stage(self.root,self.out,expected,expected,self.names)
        self.assertTrue(all((self.out/name).is_file() for name in self.captures))
        with self.assertRaisesRegex(ValueError,'Private resource correction capture'):
            env.discover(self.root,self.identity,self.names|{self.captures[0]})

    def test_correction_capture_is_deduplicated_with_existing_active_pin(self):
        self.correction_fixture()
        provider=json.loads((self.root/self.provider).read_text(encoding='utf8'))
        provider['source_sha256'][self.captures[0]]=env.sha(self.root/self.captures[0])
        self.write(self.provider,provider)
        expected=env.discover(self.root,self.identity,self.names)
        self.assertEqual(len(expected['inputs']),5)
        row=next(r for r in expected['inputs'] if r['destination']==self.captures[0])
        self.assertEqual(len(row['authority']),2)
        self.assertEqual({r['provider'] for r in row['authority']},{self.provider,env.CORRECTIONS})

    def test_stale_correction_policy_requires_fresh_manifest(self):
        expected=self.correction_fixture()
        self.correction_policy['rationale']='Changed policy identity requires a fresh explicit manifest'
        self.write(env.CORRECTIONS,self.correction_policy)
        current=env.discover(self.root,self.identity,self.names)
        with self.assertRaisesRegex(ValueError,'input authorities'):env.validate_manifest(expected,current)
        self.assertEqual(list(self.out.iterdir()),[])

    def test_stale_correction_evidence_or_capture_is_rejected_by_real_consumer(self):
        self.correction_fixture()
        path=self.root/self.correction_evidence;before=path.read_bytes()
        path.write_bytes(before+b' ')
        with self.assertRaisesRegex(ValueError,'Missing identity correction evidence'):
            env.discover(self.root,self.identity,self.names)
        path.write_bytes(before)
        self.write(self.captures[0],b'changed synthetic capture')
        with self.assertRaisesRegex(ValueError,'Changed identity evidence capture'):
            env.discover(self.root,self.identity,self.names)
        self.assertEqual(list(self.out.iterdir()),[])

    def test_predecessor_repin_does_not_bypass_correction_semantics(self):
        self.correction_fixture()
        previous=json.loads((self.root/self.correction_previous).read_text(encoding='utf8'))
        previous['resources'][0]['title']='Wrong predecessor, even with a fresh archive digest'
        self.write(self.correction_previous,previous)
        self.correction_policy['previous_registry']['sha256']=env.sha(self.root/self.correction_previous)
        self.write(env.CORRECTIONS,self.correction_policy)
        with self.assertRaisesRegex(ValueError,'does not match historical registry'):
            env.discover(self.root,self.identity,self.names)

    def test_corrupt_correction_identity_is_not_a_generic_capture_authority(self):
        self.correction_fixture()
        self.correction_policy['corrections'][0]['incoming']['authors']=['Unpinned replacement']
        self.write(env.CORRECTIONS,self.correction_policy)
        with self.assertRaisesRegex(ValueError,'Corrupt recorded identity correction'):
            env.discover(self.root,self.identity,self.names)

    def test_correction_authority_metadata_and_consumers_must_ship(self):
        self.correction_fixture()
        required={env.CORRECTIONS,self.correction_previous,self.correction_evidence,*env.CORRECTION_CONSUMERS}
        for name in required:
            with self.subTest(name=name),self.assertRaisesRegex(ValueError,'authority is absent from source archive'):
                env.discover(self.root,self.identity,self.names-{name})
        expected=env.discover(self.root,self.identity,self.names)
        consumer=env.CORRECTION_CONSUMERS[0]
        self.write(consumer,(self.root/consumer).read_bytes()+b'\n# Changed consumer identity\n')
        with self.assertRaisesRegex(ValueError,'input authorities'):
            env.validate_manifest(expected,env.discover(self.root,self.identity,self.names))

    def test_unselected_evidence_record_and_unrelated_capture_are_not_discovered(self):
        expected=self.correction_fixture()
        records=json.loads((self.root/self.correction_evidence).read_text(encoding='utf8'))['records']
        records['R-NOT-SELECTED']={'capture':'build/never-created.pdf','sha256':'0'*64}
        self.replace_correction_evidence(records)
        self.write('research/v5/expansion/unrelated/resources.json',{'capture':'build/also-never-created.pdf','sha256':'0'*64})
        current=env.discover(self.root,self.identity,self.names)
        self.assertEqual([r['destination'] for r in current['inputs']],[r['destination'] for r in expected['inputs']])

    def test_correction_capture_paths_remain_strictly_local(self):
        self.correction_fixture()
        records=json.loads((self.root/self.correction_evidence).read_text(encoding='utf8'))['records']
        records['R-FIXTURE-HLC']['capture']='../outside.pdf'
        self.replace_correction_evidence(records)
        with self.assertRaisesRegex(ValueError,'Noncanonical resource path'):
            env.discover(self.root,self.identity,self.names)

    def test_correction_manifest_chain_cannot_be_edited(self):
        expected=self.correction_fixture()
        row=next(r for r in expected['inputs'] if r['destination']==self.captures[0])
        mutations=[lambda a:a.update(resource_id='R-OTHER'),
            lambda a:a.update(provider_sha256='0'*64),
            lambda a:a['evidence'].update(record_sha256='0'*64),
            lambda a:a['evidence'].update(pointer='/records/R-OTHER/capture'),
            lambda a:a['previous_registry'].update(sha256='0'*64),
            lambda a:a['consumer_source_sha256'].update({env.CORRECTION_CONSUMERS[0]:'0'*64})]
        for mutate in mutations:
            bad=deepcopy(expected)
            current=next(r for r in bad['inputs'] if r['destination']==row['destination'])
            mutate(current['authority'][0])
            with self.assertRaises(ValueError):env.validate_manifest(bad,expected)

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
    env.dump(output,dict(passed=result.wasSuccessful(),tests=result.testsRun,skipped=len(result.skipped),output=stream.getvalue(),source_sha256={p:env.sha(env.ROOT/p) for p in ['tools/candidate_author_environment.py','tools/check_candidate_author_environment.py','tools/check_candidate_exports_v5.py','tools/package_v4.py','tools/package_v5.py','tools/check_release_v5.py','tools/resource_identity_v5.py','tools/resource_inputs_v5.py']},scope='Isolated exact-manifest, active-row and validated correction-policy authority, path, immutable archive, local staging and acceptance-first release plumbing guards. Real corrections(root) reads synthetic staged captures; no PDF content or new source inspection. Real ZIPs use temporary fixtures and a declared build seam; no real private input copied or repository release accepted.'))
    print(stream.getvalue());return int(not result.wasSuccessful())


if __name__=='__main__':raise SystemExit(main())
