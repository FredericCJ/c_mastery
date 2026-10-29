"""Public route integrity and rejection regressions; never claims pedagogy passed."""
from __future__ import annotations
from copy import deepcopy
from contextlib import redirect_stdout
import hashlib
import io
import json
import os
import re
from pathlib import Path
import tempfile
import subprocess
import shutil
import sys
import unittest
from unittest.mock import patch
from common import ROOT, load_taxonomy
from cmastery import cli, learning, route
from generate_native_route import generate, BASE
from native_teaching_revision import qualifies


def errors(value):
    failures=[]
    entries={n['id']:n for n in value['nodes']}
    if len(entries)!=len(value['nodes']):failures.append('duplicate entry')
    if len(value['order'])!=len(set(value['order'])) or set(value['order'])!=set(entries):failures.append('unreachable or duplicated work')
    position={key:i for i,key in enumerate(value['order'])}
    contexts={c['id'] for c in value['contexts']}
    catalog=learning.catalog()
    for entry in value['nodes']:
        if entry['context'] not in contexts:failures.append('unknown context')
        for edge in entry['prerequisites']:
            if edge['kind'] not in {'concept','component','environment','preparation'}:failures.append('unknown prerequisite type')
            if edge['kind'] in {'concept','component'}:
                if edge['target'] not in entries:failures.append('dangling prerequisite')
                elif position.get(edge['target'],1e9)>=position.get(entry['id'],-1):failures.append('hard cycle or consumer before producer')
            if edge['kind']=='component':
                art=catalog['artifacts'].get(edge['artifact'])
                if art is None or edge['target']!=art['producer_unit'] or str(edge['contract_version'])!=str(art['contract_version']):failures.append('component contract mismatch')
        for name in entry['sources']+entry['contracts']:
            try:
                if not route.public_path(ROOT,name).is_file():failures.append('missing public input')
            except ValueError:failures.append('unsafe public input')
    return failures


class RouteIntegrity(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.active=route.load()

    def test_canonical_generated_outputs_and_all_objectives(self):
        for name,value in generate().items():
            actual=json.loads((ROOT/BASE/name).read_text())
            self.assertEqual(route.digest(actual),route.digest(value),"Stale generated output: "+name)
        self.assertEqual(errors(self.active),[])
        wanted={o['id'] for d in load_taxonomy() for t in d['subtopics'] for o in t['objectives']}
        coverage=json.loads((ROOT/BASE/'coverage.json').read_text())['objectives']
        self.assertEqual(wanted,{r['objective'] for r in coverage})
        self.assertTrue(all(r['encounters'] for r in coverage))
        migration=json.loads((ROOT/BASE/'migration.json').read_text())
        self.assertEqual({n['id'] for n in self.active['nodes']},{n['id'] for n in migration['records']})
        self.assertEqual(wanted,set(migration['objective_ids']))

    def test_cycles_dangling_order_lost_work_reject(self):
        bad=deepcopy(self.active);bad['nodes'][0]['prerequisites'].append(dict(kind='concept',target='MISSING',reason='negative fixture'))
        self.assertIn('dangling prerequisite',errors(bad))
        bad=deepcopy(self.active);key=bad['nodes'][0]['id'];bad['nodes'][0]['prerequisites'].append(dict(kind='concept',target=key,reason='negative fixture'))
        self.assertIn('hard cycle or consumer before producer',errors(bad))
        bad=deepcopy(self.active);bad['order'].pop()
        self.assertIn('unreachable or duplicated work',errors(bad))

    def test_component_exists_is_not_assessed_or_exported(self):
        entry={'prerequisites':[dict(kind='component',target='producer',artifact='bytes',contract_version=2)]}
        self.assertEqual(route.blockers(entry,set(),{'bytes'}),['assessment:producer'])
        self.assertEqual(route.blockers(entry,{'producer'},set()),['export:bytes@2'])
        self.assertEqual(route.blockers(entry,{'producer'},{'bytes'}),[])
        self.assertTrue(route.blockers({'prerequisites':[dict(kind='environment',target='unavailable-tool')]},set()))
        self.assertEqual(route.blockers({'prerequisites':[dict(kind='preparation',target='optional')]},set()),[])

    def test_pilot_order_has_real_producers_before_representation_changes(self):
        position={key:i for i,key in enumerate(self.active['order'])}
        sequence=['LAB-HS-03','U-T14-05-01','U-T14-05-04','U-T24-05-02','LAB-RZ-05','LAB-RZ-06','U-T24-05-03']
        self.assertEqual(sorted(sequence,key=position.get),sequence)
        for producer in ['U-T15-05-04','U-T14-01-04','U-T14-05-04']:
            self.assertLess(position[producer],position['U-T36-07-01'])
        self.assertLess(position['U-T55-01-03'],position['U-T43-09-03'])

    def test_foundation_follows_c_bounds(self):
        pos={k:i for i,k in enumerate(self.active['order'])}
        for prior in ['U-T01-03-03','U-T03-02-03','U-T04-01-04']:
            self.assertLess(pos[prior],pos['U-T01-01-01'])

    def test_successor_semantic_prerequisites_are_actual_ancestors(self):
        nodes={n['id']:n for n in self.active['nodes']}
        def ancestors(key):
            result=set()
            def visit(current):
                for edge in nodes[current]['prerequisites']:
                    if edge['kind'] in {'concept','component'} and edge['target'] not in result:
                        result.add(edge['target']);visit(edge['target'])
            visit(key);return result
        for prior in ['U-T24-03-03','X-T15-019','U-T55-01-03','U-T24-05-03']:
            self.assertIn(prior,ancestors('U-T43-09-03'))
        for prior in ['X-T03-001','U-T03-01-03','U-T02-01-03']:
            self.assertIn(prior,ancestors('U-T03-02-01'))
        text=route.render_node('U-T43-09-02')
        self.assertIn('supplied reference adapters',text)
        self.assertIn('deferred to U-T43-09-03',text)

    def test_fresh_profile_cannot_offer_advanced_expansion_work(self):
        advanced={'X-T50-001','X-T51-001','X-T52-001','X-T43-001','X-T14-003','X-T14-004'}
        with tempfile.TemporaryDirectory() as folder:
            learning.study_init(folder)
            packet=learning.study_next(folder)
            self.assertFalse(advanced & {x['id'] for x in packet['available_exercises']})
            self.assertTrue(packet['available_units'])
        for entry in self.active['nodes']:
            if entry['kind']=='exercise':
                self.assertTrue(route.blockers(entry,set(),capabilities=route.capabilities()),entry['id'])

    def test_expansion_gates_retain_actual_mechanisms_and_early_producers(self):
        nodes={n['id']:n for n in self.active['nodes']}
        def targets(key):return {e['target'] for e in nodes[key]['prerequisites'] if e['kind'] in {'concept','component'}}
        for exercise,prior in [('X-T50-001','U-T50-05-03'),('X-T51-001','U-T51-01-03'),
                               ('X-T52-001','U-T52-03-04'),('X-T43-001','U-T43-08-03')]:
            self.assertIn(prior,targets(exercise))
            text=route.render_node(exercise)
            self.assertNotIn('Required baseline units: none',text)
            self.assertIn(prior,re.search(r'(?m)^Required baseline units: (.*)$',text)[1])
        for exercise in ['X-T14-003','X-T14-004']:
            self.assertTrue({'U-T14-01-04','U-T03-02-03','U-T01-01-02'}<=targets(exercise))
        self.assertIn('U-T14-06-03',targets('X-T14-005'))
        self.assertIn('U-T15-03-03',targets('X-T15-003'))
        self.assertNotIn('U-T04-01-04',targets('X-T04-001'))
        self.assertNotIn('U-T03-02-03',targets('X-T03-001'))
        self.assertEqual(route.blockers(nodes['X-T04-001'],{'U-T01-03-03'},capabilities=route.capabilities()),[])
        self.assertNotIn('U-T14-09-01',targets('X-T14-016'))
        self.assertIn('single-root insertion/split',nodes['X-T14-016']['active_increment']['task'])
        for key in ['X-T56-002','X-T56-003','X-T56-004']:
            self.assertFalse(any(target.startswith(('U-T48','U-T50','U-T51','U-T53','U-T56')) for target in targets(key)))

    def test_expansions_inherit_upstream_context_without_future_adapter_gates(self):
        units=cli.read(ROOT/'curriculum/v5/units.json')['units']
        first={}
        for unit in units:first.setdefault(unit['project'],unit)
        source=cli.read(ROOT/BASE/'contextual-v1.source.json')
        exceptions=source['exercise_prerequisite_policy']['portable_core_exceptions']
        for entry in self.active['nodes']:
            if entry['kind']!='exercise' or entry['id'] in exceptions:continue
            gates={edge['target'] for edge in entry['prerequisites'] if edge['kind'] in {'concept','component'}}
            self.assertTrue(set(first[entry['project']]['requires'])<=gates,entry['id'])

    def test_graph_successor_requires_exact_exported_components(self):
        wanted={'containers.depth.ring','containers.depth.heap','containers.depth.csr','algorithms.depth.graph'}
        n=route.node('U-T36-07-01')
        actual={e['artifact'] for e in n['prerequisites'] if e['kind']=='component'}
        self.assertTrue(wanted<=actual)
        catalog=learning.catalog()['artifacts']
        for key in wanted:
            self.assertIn(key,catalog)
            self.assertIn('required_header',catalog[key])
        all_assessed={x['id'] for x in self.active['nodes']}
        missing=route.blockers(n,all_assessed,capabilities=route.capabilities())
        for key in wanted:self.assertIn('export:'+key+'@1',missing)

    def test_route_generation_is_hash_seed_independent(self):
        for seed in ['0','1']:
            result=subprocess.run([sys.executable,str(ROOT/'tools/generate_native_route.py'),'--check'],
                                  cwd=ROOT,env=dict(os.environ,PYTHONHASHSEED=seed),capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)

    def test_active_component_export_link_and_header_rejection(self):
        from cmastery import exercises
        cases=[('foundation.cursor','X-T03-001','U-T03-02-01'),
               ('containers.depth.csr','U-T14-08-03','U-T15-05-01')]
        for artifact,producer,consumer in cases:
            with self.subTest(artifact=artifact), tempfile.TemporaryDirectory() as folder:
                base=Path(folder);workspace=base/'producer';workspace.mkdir()
                header=learning.catalog()['artifacts'][artifact]['required_header']
                (workspace/'include').mkdir();(workspace/'src').mkdir()
                (workspace/'include'/Path(header).name).write_bytes((ROOT/header).read_bytes())
                (workspace/'src/provider.c').write_text('/* Export plumbing fixture, no mastery claim. */\nint route_export_fixture(void){return 0;}\n')
                specification={'id':producer,'path':'ACTIVE.md','sha256':route.specification_identity(producer)}
                (workspace/'ACTIVE.md').write_text(route.render_node(producer))
                cli.save(workspace/'.cmastery/project.json',{'active_specification':specification})
                record={'id':'test-acceptance','workspace':str(workspace),'source':cli.snapshot(workspace),
                        'revision':{'source':cli.snapshot(workspace)['files']},'assessment':{},'active_specification':specification}
                accepted={producer:{'valid':True,'record':record}}
                declaration={'contract_version':'1','cmake_target':'route_component','profiles':['C'],'targets':['host'],
                             'sources':['src/provider.c'],'headers':['include/'+Path(header).name],
                             'include_dirs':['include'],'dependencies':[]}
                cli.save(base/'export.json',declaration)
                def command(argv):
                    output=io.StringIO()
                    with redirect_stdout(output):status=cli.main([str(arg) for arg in argv])
                    self.assertEqual(status,0,output.getvalue())
                    return json.loads(output.getvalue())
                with patch.object(learning,'accepted',return_value=accepted),patch.object(exercises,'accepted',return_value=accepted),patch.object(learning,'append',return_value={'id':'test-event'}):
                    result=command(['artifact','export',base/'profile',workspace,artifact,base/'bundle','--manifest',base/'export.json'])
                    self.assertEqual(result['artifact'],artifact)
                    dest=base/'consumer';dest.mkdir()
                    cli.save(dest/'.cmastery/project.json',{'unit':consumer,'active_specification':{'id':consumer}})
                    linked=command(['artifact','link',base/'profile',dest,base/'bundle','--profile','C','--target','host'])
                    self.assertEqual(linked['artifact'],artifact)
                    self.assertIn(artifact,learning.linked_state(dest))
                    (dest/'main.c').write_text('int route_export_fixture(void); int main(void){return route_export_fixture();}\n')
                    (dest/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(route_handoff C)\ninclude(.cmastery/components.cmake)\nadd_executable(consume main.c)\ntarget_link_libraries(consume PRIVATE route_component)\n')
                    for argv in [['cmake','-S',str(dest),'-B',str(base/'build')],['cmake','--build',str(base/'build')],[str(base/'build/consume')]]:
                        result=subprocess.run(argv,capture_output=True,text=True)
                        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                    forged=base/'forged';shutil.copytree(base/'bundle',forged)
                    forged_header=forged/declaration['headers'][0]
                    forged_header.write_text('/* Wrong API under a self-consistent manifest. */\n')
                    manifest=cli.read(forged/'artifact.json')
                    manifest['files'][declaration['headers'][0]]=hashlib.sha256(forged_header.read_bytes()).hexdigest()
                    manifest.pop('sha256');manifest['sha256']=learning.digest(manifest)
                    cli.save(forged/'artifact.json',manifest)
                    learning.verify_bundle(forged)
                    with self.assertRaisesRegex(ValueError,'header content'):
                        learning.artifact_link(base/'profile',dest,forged,'C','host')
                    with self.assertRaisesRegex(ValueError,'header content'):
                        learning.verify_component_api(manifest)
                    altered=workspace/'include'/Path(header).name;altered.write_text(altered.read_text()+'\n/* changed API */\n')
                    record['source']=cli.snapshot(workspace);record['revision']['source']=cli.snapshot(workspace)['files']
                    with self.assertRaisesRegex(ValueError,'unchanged public header'):
                        learning.artifact_export(base/'profile',workspace,artifact,base/'rejected',base/'export.json')

    def test_old_profile_migration_preserves_ledger_and_acceptances(self):
        with tempfile.TemporaryDirectory() as folder:
            learning.study_init(folder)
            cfg=learning.settings(folder);self.assertEqual(cfg['route'],'contextual-v1')
            cfg.pop('route');cli.save(learning.area(folder)/'profile.json',cfg)
            before=(learning.area(folder)/'ledger.jsonl').read_bytes()
            self.assertIsNone(route.selected(folder))
            result=route.migrate(folder)
            self.assertEqual(result['new_acceptances'],0)
            self.assertTrue((learning.area(folder)/'ledger.jsonl').read_bytes().startswith(before))
            self.assertEqual(learning.accepted(folder),{})
            self.assertEqual(route.selected(folder)['id'],'contextual-v1')

    def test_study_readiness_and_sheet_use_active_view(self):
        with tempfile.TemporaryDirectory() as folder:
            learning.study_init(folder)
            packet=learning.study_next(folder)
            self.assertEqual(packet['route']['id'],'contextual-v1')
            self.assertTrue(packet['route']['available'])
            sheet=Path(folder)/'sheet.md';learning.study_sheet(folder,sheet,'U-T01-03-01')
            self.assertIn('Active route: contextual-v1',sheet.read_text())

    def test_effective_graph_card_and_canonical_revision_proof(self):
        text=route.render_node('U-T15-05-01')
        self.assertIn('negative',text)
        self.assertNotIn('duplicate-heavy input exposing a partition',text)
        self.assertNotIn('## Working card',route.project_brief('PR-T15-05'))
        packet=json.loads((ROOT/BASE/'teaching_revisions.json').read_text())
        for row in packet['records']:
            self.assertTrue(qualifies(row['path'],row['predecessor_sha256'],root=ROOT),row['path'])
            self.assertFalse(qualifies(row['path'],'0'*64,root=ROOT))
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);path=root/BASE/'teaching_revisions.json';path.parent.mkdir(parents=True);path.write_text(json.dumps(packet))
            row=packet['records'][0];path=root/row['path'];path.parent.mkdir(parents=True);path.write_bytes((ROOT/row['path']).read_bytes()+b'\nUnreviewed edit.\n')
            self.assertFalse(qualifies(row['path'],row['predecessor_sha256'],root=root))

    def test_cli_init_exports_effective_card_without_assessor_code(self):
        with tempfile.TemporaryDirectory() as folder:
            destination=Path(folder)/'learner'
            cli.instantiate('PR-T15-05',destination,unit_id='U-T15-05-01')
            self.assertIn('ACTIVE.md',(destination/'PROJECT.md').read_text())
            self.assertEqual((destination/'ACTIVE.md').read_text(),route.render_node('U-T15-05-01'))
            self.assertIn('ACTIVE.md',(destination/'UNIT.md').read_text())
            self.assertIn('ACTIVE.md',(destination/'ASSESSMENT.md').read_text())
            self.assertFalse(any('assessor' in p.relative_to(destination).parts for p in destination.rglob('*')))
            self.assertEqual(cli.read(destination/'.cmastery/project.json')['assessment'],'learner')

    def test_successor_explicit_package_and_criterion_aliases(self):
        from native_successors import resolve
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            teaching=root/'labs/demo/CONTRACTS.md';teaching.parent.mkdir(parents=True);teaching.write_text('Public instruction')
            support=root/'research/support.json';support.parent.mkdir(parents=True)
            support.write_text(json.dumps({'updates':[{'id':'record', 'criterion':'AX-ONE', 'dimensions':{'teaching':{'evidence':['labs/demo/CONTRACTS.md','assessor/private.md']}}}]}))
            data={'project':'PR-ONE','criteria':[{'id':'AX-ONE','objective':'O1'}], 'successor_support':{'path':'research/support.json','criterion_ids':['AX-ONE']}}
            self.assertEqual(resolve(data,root)[0]['public_contracts'],['labs/demo/CONTRACTS.md'])
            support.write_text(json.dumps({'id':'provider','original_criterion_ids':['AX-ONE'], 'public_artifacts':['labs/demo/CONTRACTS.md'],'assessor_artifacts':['assessor/private.md']}))
            self.assertEqual(resolve(data,root)[0]['public_contracts'],['labs/demo/CONTRACTS.md'])
            support.write_text(json.dumps({'updates':[{'id':'X-UNRELATED-V1','dimensions':{'teaching':{'evidence':['labs/demo/CONTRACTS.md']}}}]}))
            # The explicit parent binding authorizes package teaching, but keeps
            # its scope additive rather than replacing a criterion's task.
            data['successor_support']['scope']='Additional variant package; original criteria unchanged.'
            result=resolve(data,root)[0]
            self.assertEqual(result['public_contracts'],['labs/demo/CONTRACTS.md'])
            self.assertIn('Additional variant',result['binding_scopes'][0])
            data['successor_support']['criterion_ids']=['AX-OTHER']
            self.assertEqual(resolve(data,root)[0]['public_contracts'],[])

    def test_freestanding_addition_retains_original_implementation_tasks(self):
        text=route.render_node('U-T11-01-01')
        data=cli.read(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T11-01.json')
        self.assertIn(data['criteria'][0]['task'],text)
        self.assertNotIn('not an additional implementation assignment',text)
        for resolution in route.node('U-T11-01-01')['criterion_resolutions']:
            self.assertEqual(resolution['mode'],'additive')

    def test_all_explicit_successors_resolve_without_private_contracts(self):
        records=json.loads((ROOT/BASE/'criterion_resolutions.json').read_text())['criteria']
        self.assertTrue(records)
        for record in records:
            self.assertEqual(record['unresolved_bindings'],[],record['criterion'])
            self.assertTrue(all(p.startswith(('labs/','curriculum/')) and 'assessor' not in Path(p).parts for p in record['public_contracts']))
        graph=route.render_node('U-T36-07-01')
        self.assertIn('labs/algorithm_frontier/CONTRACTS.md',graph)
        self.assertIn('No new assembly graph solver',graph)

    def test_effective_contract_tamper_and_source_change_reject(self):
        with tempfile.TemporaryDirectory() as folder:
            destination=Path(folder)/'learner'
            cli.instantiate('PR-T15-05',destination,unit_id='U-T15-05-01')
            route.verify_workspace_specification(destination)
            active=destination/'ACTIVE.md';original=active.read_text();active.write_text(original+'Changed acceptance.\n')
            with self.assertRaisesRegex(ValueError,'changed or is missing'):
                route.verify_workspace_specification(destination)
            active.write_text(original)
            with patch.object(route,'specification_identity',return_value='0'*64):
                with self.assertRaisesRegex(ValueError,'specification changed'):
                    route.verify_workspace_specification(destination)

    def test_operative_successor_replaces_predecessor_and_teaches_context(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            def put(name, text):
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text)
            def js(name, value):put(name,json.dumps(value))
            js('curriculum/edition.json',{'active_route':'route.json'})
            js('route.json',{'schema_version':1,'id':'test-route','contexts':[{'id':'graph','problem':'Trace dependencies','teaching':'curriculum/teaching.md'}], 'nodes':[{'id':'UNIT','kind':'unit','project':'PR-ONE','title':'Graph','context':'graph','why_now':'A dependency cycle must reject.','next_consumer':'compiler','prerequisites':[], 'objectives':['O1'],'sources':['curriculum/projects/PR-ONE.md'],'contracts':['labs/current/CONTRACTS.md'],'criterion_resolutions':[{'criterion':'AX-ONE','mode':'replace','public_contracts':['labs/current/CONTRACTS.md'],'binding_scopes':[]}]}]})
            js('curriculum/v5/units.json',{'units':[{'id':'UNIT','title':'Graph','steps':['OBSOLETE assembly sort requirement'],'checks':['OBSOLETE diagnostic'],'stop_condition':'OBSOLETE gate'}]})
            put('curriculum/projects/PR-ONE.md','# Old project\n\nOBSOLETE full project requirement')
            put('curriculum/teaching.md','# Context\n\nA graph worklist reaches a fixed point.')
            put('labs/current/CONTRACTS.md','# Current contract\n\nNEW frontier certificate requirement.')
            js('curriculum/v5/expansion/legacy_assessments/PR-ONE.json',{'criteria':[{'id':'AX-ONE','objective':'O1','title':'Frontier','task':'OBSOLETE criterion requirement','rubric':{'2':'Independent correctness'},'variants':[{'id':'AX-ONE-V1','minimum_days':7,'task':'OBSOLETE variant requirement'}]}]})
            with patch('cmastery.assessments.learner_brief',return_value='OBSOLETE criterion requirement'):
                text=route.render_node('UNIT',root)
            self.assertNotIn('OBSOLETE',text)
            self.assertEqual(text.count('NEW frontier certificate requirement.'),1)
            self.assertIn('A graph worklist reaches a fixed point.',text)
            self.assertIn('AX-ONE-V1',text)
            self.assertIn('Independent correctness',text)

    def test_live_acceptance_reopens_when_effective_contract_changes(self):
        with tempfile.TemporaryDirectory() as folder:
            entry={'unit':'U-T01-03-01','workspace':folder,'unit_contract':learning.digest(learning.unit('U-T01-03-01')),
                   'active_specification':{'id':'U-T01-03-01','sha256':'0'*64}}
            valid, reason=learning.acceptance_valid(entry)
            self.assertFalse(valid)
            self.assertEqual(reason,'effective learner contract changed')

    def test_project_only_initialization_selects_effective_successor(self):
        with tempfile.TemporaryDirectory() as folder:
            destination=Path(folder)/'project'
            cli.instantiate('PR-T36-07',destination)
            self.assertTrue((destination/'ACTIVE.md').is_file())
            text=(destination/'ACTIVE.md').read_text()
            self.assertNotIn('Implement lower-bound, stable insertion and heap repair kernels under both target ABIs',text)
            self.assertIn('No new assembly graph solver',text)
            self.assertEqual(cli.read(destination/'.cmastery/project.json')['active_specification']['id'],'PR-T36-07')

    def test_publisher_uses_same_effective_card_and_preserves_historical_route(self):
        from publish_native_route import prepare
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder);page=source/'learning/route.md';page.parent.mkdir(parents=True)
            old='(v5-route)=\n# Historical route\n';page.write_text(old)
            subset=dict(self.active,order=['U-T36-07-01'])
            expected=route.render_node('U-T36-07-01')
            with patch.object(route,'load',return_value=subset):
                docs,sections=prepare(source,[old])
            self.assertIn('learning/historical-route',docs)
            self.assertIn('(v5-historical-route)=',(source/'learning/historical-route.md').read_text())
            self.assertEqual((source/'learning/effective/U-T36-07-01.md').read_text(),expected)
            self.assertIn('effective/U-T36-07-01.md',page.read_text())
            self.assertNotIn(old,sections)

    def test_public_path_escape_rejects(self):
        for name in ['../private.md','/tmp/private.md','assessor/algorithms/algo.c']:
            with self.assertRaises(ValueError):route.public_path(ROOT,name)

    def test_inline_document_links_keep_their_original_public_base(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            for name in ['labs/component/CONTRACTS.md','labs/helper/api.h','assessor/private.md','tools/author_answer.py','research/private.json','reviews/key.md','planning/solution.md']:
                p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('Public fixture' if 'assessor' not in name else 'WITHHELD_TEST_SENTINEL')
            text='[API](../helper/api.h) [online](https://example.com/a) [anchor](#argument) [private](../../assessor/private.md)'
            text+=' [tool](../../tools/author_answer.py) [metadata](../../research/private.json) [review](../../reviews/key.md) [plan](../../planning/solution.md)'
            actual=route.inline_markdown(text,'labs/component/CONTRACTS.md',root)
            self.assertIn('[API](labs/helper/api.h)',actual)
            self.assertIn('[online](https://example.com/a)',actual)
            self.assertIn('[anchor](#argument)',actual)
            self.assertNotIn('](../../assessor/',actual)
            self.assertNotIn('WITHHELD_TEST_SENTINEL',actual)
            destination=Path(folder)/'workspace'
            with patch.object(route,'node',return_value={'supplied_artifacts':[]}),patch.object(route,'render_node',return_value=actual):
                exported=route.export_assets('synthetic',destination,root)
            self.assertIn('labs/helper/api.h',exported)
            self.assertTrue((destination/'labs/helper/api.h').is_file())
            self.assertFalse((destination/'assessor').exists())
            for private in ['tools','research','reviews','planning']:
                self.assertFalse((destination/private).exists())
            for private in ['tools/author_answer.py','research/private.json','reviews/key.md','planning/solution.md']:
                with self.assertRaises(ValueError):route.public_path(root,private)
            wire=root/'research/enhancement/networking_contracts.json';wire.parent.mkdir(parents=True);wire.write_text('{"public":"wire format"}')
            names=set();text=route.inline_markdown('[wire](../../research/enhancement/networking_contracts.json)','curriculum/projects/example.md',root,names)
            self.assertIn('research/enhancement/networking_contracts.json',names)
            self.assertIn('](research/enhancement/networking_contracts.json)',text)

    def test_asset_parent_symlink_cannot_expose_assessor_or_redirect_writes(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)/'root';root.mkdir();(root/'assessor').mkdir();(root/'labs').mkdir()
            (root/'assessor/answer.h').write_text('WITHHELD_TEST_SENTINEL')
            (root/'labs/redirected').symlink_to(root/'assessor',target_is_directory=True)
            with self.assertRaises(ValueError):route.public_path(root,'labs/redirected/answer.h')
            with patch.object(route,'node',return_value={'supplied_artifacts':['labs/redirected/answer.h']}),patch.object(route,'render_node',return_value=''):
                with self.assertRaises(ValueError):route.export_assets('synthetic',Path(folder)/'rejected',root)
            dest=Path(folder)/'destination';dest.mkdir();(dest/'labs').symlink_to(root/'assessor',target_is_directory=True)
            with self.assertRaises(ValueError):route.public_path(dest,'labs/answer.h')


if __name__=='__main__':unittest.main()
