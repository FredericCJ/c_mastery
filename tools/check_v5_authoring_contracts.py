"""Regression tests for additive recipes and authored units; no learner evidence."""
from copy import deepcopy
import unittest
from author_v4 import authored_unit_minutes
from v5_common import merge_domain_fragments
import vocabulary
from run_expansion_family import campaign_config, mutation_coverage
from integrate_v5 import merge_resource_record
from check_expansion_campaigns import b06_errors, execution_errors, generic_errors, pin_errors, special_errors
from check_legacy_assessments import schema_errors, local_path, validate as validate_legacy
from common import ROOT, load, read_json
from validate import project_allocation_error


class ExportedStubAssertions(unittest.TestCase):
    markers = {'systems_certificates': 'SC_CHECK:', 'serial_depth': 'SD_CHECK:',
               'ethernet_depth': 'ED_CHECK:', 'edu7_depth': 'E7_CHECK:',
               'http_depth': 'HD_CHECK:', 'service_depth': 'SD_CHECK:'}

    def receipt(self, marker):
        return {'status': 'failed', 'steps': [{'exit_code': 0}, {'exit_code': 0},
                {'exit_code': 8, 'stdout': marker + ' failed: required behavior'}],
                'tests': [{'name': 'example.semantic', 'failed': True}]}

    def run_stub(self, meta, receipt):
        from pathlib import Path
        from unittest.mock import patch
        import check_exercise_integration as checker
        with patch.object(checker.cli, 'read', return_value=meta), \
             patch.object(checker.cli, 'dependency_contracts', return_value={}), \
             patch.object(checker.cli, 'run_checks', return_value=receipt):
            return checker.check_exported_stub(Path('workspace'), Path('temporary'))

    def test_specialized_families_reach_their_exact_assertion(self):
        for family, marker in self.markers.items():
            with self.subTest(family=family):
                result = self.run_stub({'family': family}, self.receipt(marker))
                self.assertEqual(result['status'], 'passed')
                self.assertEqual(result['assertion_marker'], marker)

    def test_b06_allowlist_and_explicit_campaign_marker(self):
        from check_exercise_integration import assertion_marker
        for family in ('architecture_ports', 'firmware_lifecycle', 'kernel_lifecycle', 'linux_io_depth',
                       'memory_defense', 'numeric_simd', 'subset_compiler', 'verification_fuzzing'):
            with self.subTest(family=family):
                self.assertEqual(assertion_marker({'id': family}), 'B06_CHECK')
        meta = {'family': 'service_depth', 'campaign': {'assertion_marker': 'DECLARED_CHECK:'}}
        self.assertEqual(self.run_stub(meta, self.receipt('DECLARED_CHECK:'))['status'], 'passed')
        self.assertEqual(self.run_stub(meta, self.receipt('SD_CHECK:'))['status'], 'failed')

    def test_missing_or_malformed_marker_fails_closed(self):
        from check_exercise_integration import assertion_marker
        with self.assertRaisesRegex(ValueError, 'no valid semantic assertion marker'):
            assertion_marker({'family': 'unknown_family'})
        for marker in ('', '   ', None, 1, True, [], {}):
            with self.subTest(marker=marker), self.assertRaisesRegex(ValueError, 'no valid semantic assertion marker'):
                assertion_marker({'family': 'service_depth', 'campaign': {'assertion_marker': marker}})

    def test_wrong_generic_or_crash_output_is_not_semantic_rejection(self):
        for family in self.markers:
            for output in ('B06_CHECK failed', 'OTHER_CHECK: failed', 'Segmentation fault', ''):
                with self.subTest(family=family, output=output):
                    self.assertEqual(self.run_stub({'family': family}, self.receipt(output))['status'], 'failed')

    def test_configure_or_compile_failure_cannot_pass_with_assertion_text(self):
        for phase in (0, 1):
            with self.subTest(phase=phase):
                receipt = self.receipt('SD_CHECK:')
                receipt['steps'][phase]['exit_code'] = 1
                self.assertEqual(self.run_stub({'family': 'service_depth'}, receipt)['status'], 'failed')

    def test_semantic_phase_status_and_failed_test_record_are_required(self):
        base = self.receipt('SD_CHECK:')
        variants = []
        for code in (0, None):
            value = deepcopy(base); value['steps'][2]['exit_code'] = code; variants.append(value)
        value = deepcopy(base); value['steps'] = value['steps'][:2]; variants.append(value)
        value = deepcopy(base); value['status'] = 'passed'; variants.append(value)
        value = deepcopy(base); value['tests'] = []; variants.append(value)
        value = deepcopy(base); value['tests'][0]['failed'] = False; variants.append(value)
        for index, receipt in enumerate(variants):
            with self.subTest(index=index):
                self.assertEqual(self.run_stub({'family': 'service_depth'}, receipt)['status'], 'failed')

    def test_every_current_expansion_family_has_an_explicit_marker_contract(self):
        from check_exercise_integration import assertion_marker
        edition = read_json(ROOT/'curriculum/edition.json')
        for family in edition['new_families']:
            with self.subTest(family=family):
                marker = assertion_marker(read_json(ROOT/'labs'/family/'family.json'))
                self.assertTrue(isinstance(marker, str) and marker.strip())


class DescriptiveAllocations(unittest.TestCase):
    def test_new_work_has_no_historical_duration_ceiling(self):
        self.assertIsNone(project_allocation_error({'edition': 5, 'hours': 1000}))
        self.assertIsNone(project_allocation_error({'edition': 5, 'hours': 1}))

    def test_historical_schema_and_numeric_integrity_remain_checked(self):
        self.assertIsNotNone(project_allocation_error({'edition': 4, 'hours': 13}))
        for hours in (0, -1, True, 1.5, '12', None):
            with self.subTest(hours=hours):
                self.assertIsNotNone(project_allocation_error({'edition': 5, 'hours': hours}))


class FixtureCorrespondence(unittest.TestCase):
    def test_additive_inputs_count_but_missing_and_orphan_files_still_fail(self):
        import tempfile
        from pathlib import Path
        from common import dump
        from check_fixtures import check_fixture_catalog
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            dump(root/'fixtures/base.json', {})
            dump(root/'fixtures/added.json', {})
            dump(root/'curriculum/v5/expansion/legacy_assessments/PR-TEST.json',
                 {'supplied_artifacts': ['fixtures/added.json', 'labs/test/CONTRACTS.md']})
            projects = {'PR-TEST': {'meta': {'fixtures': ['fixtures/base.json']}}}
            self.assertEqual(check_fixture_catalog(projects, root),
                             {'fixtures/base.json', 'fixtures/added.json'})
            dump(root/'fixtures/orphan.json', {})
            with self.assertRaisesRegex(ValueError, 'unreferenced=.*orphan.json'):
                check_fixture_catalog(projects, root)
            (root/'fixtures/orphan.json').unlink()
            (root/'fixtures/added.json').unlink()
            with self.assertRaisesRegex(ValueError, 'missing=.*added.json'):
                check_fixture_catalog(projects, root)


class Recipes(unittest.TestCase):
    def setUp(self):
        self.base = {'phases': [{}, {}, {}], 'domains': {}, 'new_executable_counts': {}, 'fixture_oracles': []}
        self.fragment = {'id': 'T57', 'domain': {'family': 'linux_kernel', 'prefix': 'KL', 'phase': 10},
                         'new_executable_counts': {'KL': 2}, 'fixture_oracles': ['verify_linux_kernel.py']}

    def merge(self, fragment=None, base=None):
        return merge_domain_fragments(base or self.base, [('T57', fragment or self.fragment)])

    def test_merge_does_not_mutate_inputs(self):
        before = deepcopy(self.base)
        result = self.merge()
        self.assertEqual(result['domains']['T57']['phase'], 10)
        self.assertEqual(self.base, before)

    def test_domain_collision(self):
        self.base['domains']['T57'] = self.fragment['domain']
        with self.assertRaisesRegex(ValueError, 'multiply owned'):
            self.merge()

    def test_filename_identity(self):
        self.fragment['id'] = 'T58'
        with self.assertRaisesRegex(ValueError, 'identity'):
            self.merge()

    def test_phase_cannot_depend_on_arrival_order(self):
        self.base['phases'] = []
        with self.assertRaisesRegex(ValueError, 'frozen'):
            self.merge()

    def test_prefix_cannot_reuse_another_domain(self):
        self.base['domains']['T58'] = {'family': 'ports', 'prefix': 'KL', 'phase': 10}
        with self.assertRaisesRegex(ValueError, 'collision'):
            self.merge()

    def test_counts_cannot_belong_to_another_prefix(self):
        self.fragment['new_executable_counts'] = {'OW': 2}
        with self.assertRaisesRegex(ValueError, 'count'):
            self.merge()

    def test_oracle_path_escape(self):
        self.fragment['fixture_oracles'] = ['../verify_escape.py']
        with self.assertRaisesRegex(ValueError, 'oracle'):
            self.merge()

    def test_shared_oracle_reused_without_duplicate_execution(self):
        second = deepcopy(self.fragment)
        second.update(id='T58', new_executable_counts={'PO': 2})
        second['domain'].update(family='ports', prefix='PO')
        result = merge_domain_fragments(self.base, [('T57', self.fragment), ('T58', second)])
        self.assertEqual(result['fixture_oracles'], ['verify_linux_kernel.py'])


class AuthoredUnits(unittest.TestCase):
    def setUp(self):
        self.meta = {'id': 'PR-T57-01', 'hours': 4}
        self.unit = dict(id='LAB-KL-01', title='Exact mechanism', minutes=240, steps=['Construct'],
                         checks=['Independent model agrees'], objectives=['T57.01.a'], new_executable=True,
                         tests=['kernel.ownership'], scaffolding='bounded choice', reasoning=['Invariant'],
                         deliverables=['C state machine'], rubric=['Reject stale release'],
                         variants=['Change callback completion order'], assessor_analysis='assessor/x/analysis.md')

    def test_preserves_authored_minutes(self):
        self.assertEqual(authored_unit_minutes(self.meta, [self.unit]), [240])

    def test_rejects_generic_filler(self):
        with self.assertRaisesRegex(ValueError, 'no authored'):
            authored_unit_minutes(self.meta, [])

    def test_no_silent_budget_redistribution(self):
        self.unit['minutes'] = 180
        with self.assertRaisesRegex(ValueError, 'reconcile'):
            authored_unit_minutes(self.meta, [self.unit])

    def test_executable_requires_test(self):
        self.unit['tests'] = []
        with self.assertRaisesRegex(ValueError, 'concrete tests'):
            authored_unit_minutes(self.meta, [self.unit])

    def test_analysis_unit_is_not_executable(self):
        self.unit.update(new_executable=False, tests=[])
        self.assertEqual(authored_unit_minutes(self.meta, [self.unit]), [240])

    def test_no_empty_rubric(self):
        self.unit['rubric'] = []
        with self.assertRaisesRegex(ValueError, 'Incomplete'):
            authored_unit_minutes(self.meta, [self.unit])


class VocabularyShape(unittest.TestCase):
    def setUp(self):
        self.entry = dict(id='V-T65-example', term='example protocol', owner='T65',
                          kind='course', definition='An original teaching protocol.',
                          authority=[dict(resource_id='R-EXAMPLE', locator='Contract')],
                          edition_notes=[dict(edition='C17', note='Protocol, not a language guarantee.')],
                          idiom=['State the example protocol.'])
        self.resources = [dict(id='R-EXAMPLE', assignable=True, locators=['Contract'])]

    def errors(self):
        return vocabulary.validate([self.entry], self.resources, [dict(id='T65', subtopics=[])])[0]

    def test_valid_notes_render(self):
        self.assertEqual(self.errors(), [])
        self.assertIn('**C17:** Protocol, not a language guarantee.', vocabulary._entry_body(self.entry, self.resources))

    def test_string_notes_rejected_before_renderer(self):
        self.entry['edition_notes'] = 'C17 protocol'
        self.assertTrue(any('edition_notes' in error for error in self.errors()))

    def test_unrendered_usage_rejected(self):
        self.entry['idiomatic_usage'] = self.entry.pop('idiom')[0]
        self.assertTrue(any('idiomatic_usage' in error for error in self.errors()))

    def test_distinction_strings_rejected_before_renderer(self):
        for value in ('timeout', ['timeout'], [{'term': 'timeout'}]):
            with self.subTest(value=value):
                self.entry['distinguish_from'] = value
                self.assertTrue(any('distinguish_from' in error for error in self.errors()))

    def test_avoid_strings_rejected_before_renderer(self):
        for value in ('safe', ['safe'], [{'form': 'safe'}]):
            with self.subTest(value=value):
                self.entry['avoid'] = value
                self.assertTrue(any('avoid' in error for error in self.errors()))

    def test_distinction_and_replacement_render(self):
        self.entry['distinguish_from'] = [{'term': 'interface shape',
            'distinction': 'The protocol also constrains legal transition order.'}]
        self.entry['avoid'] = [{'form': 'safe', 'replace_with': 'checked transition'}]
        self.assertEqual(self.errors(), [])
        rendered = '\n'.join(vocabulary._entry_body(self.entry, self.resources))
        self.assertIn('legal transition order', rendered)
        self.assertIn('checked transition', rendered)


class CampaignControls(unittest.TestCase):
    def test_declared_tsan_is_retained(self):
        profiles, minimum = campaign_config('messaging', {'campaign': {'profiles': ['gcc-tsan'], 'mutants_per_anchor': 2}})
        self.assertEqual((profiles, minimum), (['gcc-tsan'], 2))

    def test_unknown_profile_rejected(self):
        with self.assertRaisesRegex(ValueError, 'profile'):
            campaign_config('messaging', {'campaign': {'profiles': ['gcc-pretend']}})

    def test_total_mutants_cannot_hide_uncovered_anchor(self):
        _, passed = mutation_coverage('x', ['x.a', 'x.b'], ['x.mutation.a.0', 'x.mutation.a.1'], 1)
        self.assertFalse(passed)

    def test_suffix_collision_does_not_count(self):
        _, passed = mutation_coverage('x', ['x.a'], ['x.mutation.abc'], 1)
        self.assertFalse(passed)

    def test_two_actual_controls_per_anchor(self):
        mapping, passed = mutation_coverage('x', ['x.a'], ['x.mutation.a.0', 'x.mutation.a.1'], 2)
        self.assertTrue(passed)
        self.assertEqual(len(mapping['x.a']), 2)


class ResourcePreservation(unittest.TestCase):
    def setUp(self):
        self.prior = dict(id='R-EXAMPLE', title='Named primary paper', url='https://example.org/paper',
                          authors=['Author'], edition='1', year=2020, locators=['Section1'], work_id='accepted-work')

    def test_accepted_generated_fields_remain(self):
        incoming = {k: v for k, v in self.prior.items() if k != 'work_id'}
        self.assertEqual(merge_resource_record(self.prior, incoming, True), self.prior)

    def test_accepted_scope_cannot_change(self):
        incoming = dict(self.prior, locators=['Section2'])
        with self.assertRaisesRegex(ValueError, 'Accepted'):
            merge_resource_record(self.prior, incoming, True)

    def test_unaccepted_inspection_can_expand(self):
        incoming = dict(self.prior, locators=['Section1', 'Section2'])
        self.assertEqual(merge_resource_record(self.prior, incoming), incoming)

    def test_work_identity_cannot_be_reassigned(self):
        incoming = dict(self.prior, url='https://example.org/different')
        with self.assertRaisesRegex(ValueError, 'identity'):
            merge_resource_record(self.prior, incoming)

    def test_unaccepted_missing_identity_field_can_be_completed(self):
        prior = dict(self.prior)
        del prior['edition']
        self.assertEqual(merge_resource_record(prior, self.prior), self.prior)


class ScopedCampaignEvidence(unittest.TestCase):
    def special_fixture(self):
        commands = []
        output = ('1/3 Test #1: demo.a ........ Passed 0.01 sec\n'
                  '2/3 Test #2: demo.mutation.a.1 ........ Passed 0.01 sec\n'
                  '3/3 Test #3: demo.mutation.a.2 ........ Passed 0.01 sec\n'
                  '100% tests passed, 0 tests failed out of 3\n')
        for compiler in ('gcc', 'clang'):
            for mode in ('debug', 'release', 'asan'):
                folder = compiler + '-' + mode
                options = ['-DCMAKE_C_COMPILER=' + compiler, '-DCMASTERY_ASSESSOR_ROOT=assessor/demo',
                           '-DCMAKE_BUILD_TYPE=' + ('Debug' if mode != 'release' else 'Release')]
                if mode == 'asan':
                    options.append('-DCMASTERY_SANITIZE=ON')
                commands += [dict(command=['cmake', '-S', 'labs/demo', '-B', folder, *options], passed=True),
                             dict(command=['ctest', '--test-dir', folder], passed=True, stdout=output)]
        commands.append(dict(command=['learner'], expected=1, passed=True, stdout='intended rejection'))
        return {'commands': commands, 'oracle': {'passed': True, 'cases': 1}}

    def test_special_controls_require_actual_named_passes(self):
        self.assertEqual(special_errors('demo', {'tests': ['demo.a']}, self.special_fixture()), [])

    def test_special_missing_mutant_not_hidden_by_suite_summary(self):
        report = self.special_fixture()
        report['commands'][1]['stdout'] = report['commands'][1]['stdout'].replace(
            '3/3 Test #3: demo.mutation.a.2 ........ Passed 0.01 sec\n', '')
        self.assertTrue(any('demo.mutation.a.2' in e for e in special_errors('demo', {'tests': ['demo.a']}, report)))

    def test_special_start_line_is_not_execution_evidence(self):
        report = self.special_fixture()
        report['commands'][1]['stdout'] = report['commands'][1]['stdout'].replace(
            '1/3 Test #1: demo.a ........ Passed 0.01 sec', 'Start 1: demo.a')
        self.assertTrue(any('no named observation' in e for e in special_errors('demo', {'tests': ['demo.a']}, report)))

    def test_special_failed_oracle_is_not_truthy_success(self):
        report = self.special_fixture()
        report['oracle']['passed'] = False
        self.assertIn('Independent oracle observations failed', special_errors('demo', {'tests': ['demo.a']}, report))

    def test_whole_edition_hash_is_not_a_source_pin_map(self):
        self.assertTrue(pin_errors({'input_sha256': 'a' * 64}))

    def test_escaped_evidence_is_rejected(self):
        self.assertTrue(pin_errors({'inspected_paths': {'../outside.c': 'a' * 64}}))

    def test_command_without_output_cannot_be_counted(self):
        self.assertTrue(execution_errors({'commands': [{'command': ['cc'], 'passed': True}]}))

    def test_expected_negative_output_is_preserved(self):
        report = {'commands': [{'command': ['original-target'], 'returncode': 1,
                                'passed': True, 'expected': 1, 'stdout': 'ASSERT: invariant'}]}
        self.assertEqual(execution_errors(report), [])

    def test_b06_missing_one_mutant_is_not_hidden_by_total(self):
        profiles = ['gcc-debug', 'gcc-release', 'clang-ubsan']
        checks = [{'kind': profile + ':demo.a', 'passed': True} for profile in profiles]
        checks += [{'kind': profile + ':demo.mutation.a', 'passed': True} for profile in profiles[:2]]
        checks += [{'kind': 'learner-stub-rejection:demo.a', 'passed': True}]
        errors = b06_errors('demo', {'tests': ['demo.a']},
                           {'profiles': profiles, 'families': ['demo'], 'checks': checks})
        self.assertEqual(errors, ['Missing exact semantic mutant: clang-ubsan:demo.mutation.a'])

    def test_tsan_label_requires_instrumentation_observation(self):
        profile = {'compiler': 'gcc', 'profile': 'tsan', 'passed': True, 'anchors': ['demo.a'],
                   'compiled_mutant_controls': ['demo.mutation.a'],
                   'controls_by_anchor': {'demo.a': ['demo.mutation.a']}}
        report = {'profiles': [profile], 'learner_stub_rejections': [{'test': 'demo.a', 'rejected': True}]}
        errors = generic_errors('demo', {'tests': ['demo.a'], 'campaign': {'profiles': ['gcc-tsan']}}, report)
        self.assertEqual(errors, ['gcc-tsan: thread instrumentation not demonstrated'])


class LegacyAssessments(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        taxonomy, _, cls.projects, _ = load()
        cls.objectives = {o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
        cls.units = {u['id']: u for u in read_json(ROOT/'curriculum/v5/units.json')['units']}
        base = ROOT/'curriculum/v5/expansion/legacy_assessments'
        cls.schema = read_json(base/'schema.json')
        cls.example = read_json(base/'PR-T01-01.json')

    def errors(self, data):
        return validate_legacy(data, self.schema, self.projects, self.objectives, self.units)

    def test_actual_contract_is_valid(self):
        self.assertEqual(self.errors(self.example), [])

    def test_known_checkpoint_and_capstone_names_are_admissible(self):
        for pid in ('PR-P1-01', 'PR-P6-02', 'PR-CAP-LINUX-01', 'PR-CAP-MCU-04'):
            self.assertEqual(schema_errors(pid, self.schema['properties']['project']), [])
            cid = 'AX-' + pid + '-T20.01.a'
            self.assertEqual(schema_errors(cid, self.schema['properties']['criteria']['items']['properties']['id']), [])
        for pid in ('PR-P7-01', 'PR-P1-03', 'PR-CAP-HOST-01', 'PR-CAP-LINUX-05'):
            self.assertTrue(schema_errors(pid, self.schema['properties']['project']))

    def test_domain_must_match_actual_parent_module(self):
        value = deepcopy(self.example)
        value['domain'] = 'INTEGRATION'
        self.assertTrue(any('parent module' in e for e in self.errors(value)))

    def test_every_shared_contract_has_unique_publication_target(self):
        from publish_contracts_v5 import LEGACY, links, registered, target
        names = registered()
        targets = [target(name) for name in names]
        self.assertEqual(len({row[0] for row in targets}), len(targets))
        self.assertEqual([name for name in names if target(name)[1] is None], [LEGACY])
        name = 'curriculum/v5/exercises/modern-cost-contract.md'
        self.assertIn(name, names)
        rendered = links('[Required cost contract](' + name + ')', names)
        self.assertIn(target(name)[0], rendered)
        self.assertNotIn('](', rendered)
        relative = links('[Cost](modern-cost-contract.md)', names,
                         current='curriculum/v5/exercises/example.md')
        self.assertIn(target(name)[0], relative)

    def test_legacy_publication_keeps_project_filename_when_linking_contracts(self):
        import json
        import tempfile
        from pathlib import Path
        from unittest.mock import patch
        import publish_legacy_assessments_v5 as publisher
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)/'checkout'
            base = root/'curriculum/v5/expansion/legacy_assessments'
            base.mkdir(parents=True)
            (base/(self.example['project'] + '.json')).write_text(json.dumps(self.example), encoding='utf-8')
            (base/'shared-contract.md').write_text('# Shared learner obligations\n', encoding='utf-8')
            output = Path(temp)/'source'
            for folder, identifier in [('projects', self.example['project']), ('modules', self.example['domain'])]:
                page = output/folder/(identifier + '.md')
                page.parent.mkdir(parents=True, exist_ok=True)
                page.write_text('# Parent\n', encoding='utf-8')
            with patch.object(publisher, 'ROOT', root):
                publisher.prepare(output, '')
            page = output/'learning/legacy-assessments'/(self.example['project'] + '.md')
            self.assertTrue(page.is_file())
            self.assertIn(self.example['criteria'][0]['id'], page.read_text(encoding='utf-8'))
            self.assertIn('legacy-assessment-shared-contract', page.read_text(encoding='utf-8'))
            self.assertFalse((output/'curriculum').exists())

    def test_project_unit_objectives_retain_all_four_traceability_facets(self):
        from audit_expansion_v5 import project_facet
        rows = read_json(ROOT/'research/v5/expansion/b06/coverage.json')['objective_map']
        for row in rows:
            for facet in ('teaching', 'implementation_or_analysis', 'boundary_adversarial', 'independent_assessment'):
                rendered = project_facet(row, facet)
                self.assertEqual(len(rendered), 1)
                key = 'implementation_analysis' if facet == 'implementation_or_analysis' else facet
                self.assertEqual(rendered[0]['specified_evidence'], row[key])
                self.assertEqual(rendered[0]['status'], 'authored-not-learner-assessed')
                self.assertEqual(rendered[0]['remaining'], row['remaining_required'])

    def test_changed_baseline_binding_rejects(self):
        value = deepcopy(self.example)
        value['baseline_binding']['files'][0]['sha256'] = '0'*64
        self.assertTrue(any('changed' in e for e in self.errors(value)))

    def test_assessor_analysis_cannot_be_declared_as_learner_input(self):
        value = deepcopy(self.example)
        value['supplied_artifacts'].append(value['assessor_analysis'])
        self.assertTrue(any('invalid learner assessment asset' in e for e in self.errors(value)))

    def test_unrelated_objective_cannot_inflate_coverage(self):
        value = deepcopy(self.example)
        value['criteria'][0]['objective'] = 'T02.01.a'
        self.assertTrue(any('outside' in e for e in self.errors(value)))

    def test_missing_assessor_analysis_anchor_rejects(self):
        value = deepcopy(self.example)
        value['criteria'][0]['assessor_anchor'] = '#nonexistent-assessment'
        self.assertTrue(any('anchor' in e for e in self.errors(value)))

    def test_immediate_repeat_is_not_delayed_transfer(self):
        value = deepcopy(self.example)
        value['criteria'][0]['variants'][0]['minimum_days'] = 0
        self.assertTrue(any('seven' in e for e in self.errors(value)))

    def test_unregistered_passage_cannot_be_claimed_read(self):
        value = deepcopy(self.example)
        value['source_records'][0]['passages'].append('A chapter never inspected')
        self.assertTrue(any('outside inspected' in e for e in self.errors(value)))

    def test_planned_inspection_is_not_passage_evidence(self):
        from unittest.mock import patch
        register = read_json(ROOT/self.example['source_records'][0]['path'])
        for record in register['resources']:
            if record['id'] == self.example['source_records'][0]['id']:
                record['inspection']['status'] = 'planned'
        with patch('check_legacy_assessments.read_json', return_value=register):
            self.assertTrue(any('explicitly uninspected' in e for e in self.errors(self.example)))

    def test_learner_page_has_obligations_without_solution_text(self):
        from cmastery.assessments import learner_brief
        rendered = learner_brief(self.example)
        for criterion in self.example['criteria']:
            self.assertIn(criterion['task'], rendered)
            self.assertIn(criterion['verification']['expected'], rendered)
        self.assertNotIn('Withheld discriminator:', rendered)
        self.assertNotIn(self.example['assessor_analysis'], rendered)

    def test_cli_export_carries_required_criteria_without_assessor_files(self):
        import tempfile
        from pathlib import Path
        from cmastery import cli
        with tempfile.TemporaryDirectory() as temp:
            workspace = Path(temp)/'learner'
            cli.instantiate('PR-T01-01', workspace)
            rendered = (workspace/'ASSESSMENT.md').read_text(encoding='utf-8')
            self.assertIn(self.example['criteria'][0]['task'], rendered)
            self.assertIn('ASSESSMENT.md', (workspace/'PROJECT.md').read_text(encoding='utf-8'))
            self.assertFalse(any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*')))
            record = read_json(workspace/'.cmastery/project.json')
            self.assertEqual(record['supplemental_assessment']['learner_assessment'], 'not-assessed')

    def test_schema_extensions_cannot_silently_go_unchecked(self):
        self.assertTrue(schema_errors({}, {'type': 'object', 'not': {}}))

    def test_export_includes_supplemental_inputs_outside_original_project(self):
        import tempfile
        from pathlib import Path
        from cmastery import cli
        with tempfile.TemporaryDirectory() as temp:
            workspace = Path(temp)/'learner'
            cli.instantiate('PR-T14-09', workspace)
            data = read_json(workspace/'ASSESSMENT.json')
            self.assertIn('fixtures/legacy_depth/PR-T14-09.json', data['supplied_artifacts'])
            for name in data['supplied_artifacts']:
                self.assertEqual((workspace/name).read_bytes(), (ROOT/name).read_bytes())
            self.assertFalse((workspace/'assessor').exists())

    def test_export_rejects_assessor_and_escaped_supplemental_inputs_before_copy(self):
        import tempfile
        import json
        from pathlib import Path
        from cmastery import assessments
        for bad in ('assessor/answer.c', 'fixtures/../../answer.c', '/answer.c',
                    'fixtures\\answer.json', 'fixtures/missing.json'):
            with self.subTest(path=bad), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)/'root'
                source = root/'curriculum/v5/expansion/legacy_assessments/PR-TEST.json'
                source.parent.mkdir(parents=True)
                fixture = root/'fixtures/valid.json'
                fixture.parent.mkdir()
                fixture.write_text('{}', encoding='utf-8')
                source.write_text(json.dumps({'project': 'PR-TEST', 'shared_contracts': [],
                    'supplied_artifacts': ['fixtures/valid.json', bad]}), encoding='utf-8')
                destination = Path(temp)/'learner'
                destination.mkdir()
                with self.assertRaises(ValueError):
                    assessments.export(root, 'PR-TEST', destination)
                self.assertEqual(list(destination.iterdir()), [])

    def test_advance_archives_assessment_and_installs_new_obligations(self):
        import tempfile
        from pathlib import Path
        from cmastery import cli
        with tempfile.TemporaryDirectory() as temp:
            workspace = Path(temp)/'learner'
            cli.instantiate('PR-T01-01', workspace)
            old = (workspace/'ASSESSMENT.json').read_bytes()
            cli.advance(workspace, 'PR-T01-02')
            history = list((workspace/'evidence/history').iterdir())
            self.assertEqual(len(history), 1)
            self.assertEqual((history[0]/'ASSESSMENT.json').read_bytes(), old)
            new = read_json(workspace/'ASSESSMENT.json')
            self.assertEqual(new['project'], 'PR-T01-02')
            meta = read_json(workspace/'.cmastery/project.json')
            self.assertEqual(meta['supplemental_assessment']['criteria'],
                             [c['id'] for c in new['criteria']])
            self.assertEqual(meta['supplemental_assessment']['learner_assessment'], 'not-assessed')
            self.assertIn(new['criteria'][0]['task'], (workspace/'ASSESSMENT.md').read_text(encoding='utf-8'))
            self.assertFalse(any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*')))


if __name__ == '__main__':
    unittest.main()
