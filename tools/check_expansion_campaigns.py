"""Bind current scoped execution receipts to the integrated authoring identity.

This checks existing observations; it never reruns a command or turns a scoped
success into target execution, independent review, or learner acceptance.
"""
from collections import Counter
import re
from common import ROOT, dump, read_json, sha
from v5_common import RECEIPTS, identity, manifest
from run_expansion_family import DEFAULT_PROFILES, mutation_coverage

B06 = {'kernel_lifecycle', 'architecture_ports', 'firmware_lifecycle',
       'subset_compiler', 'verification_fuzzing', 'linux_io_depth',
       'memory_defense', 'numeric_simd'}
SPECIAL = {'b04_depth', 'b05_depth', 'b045_systems', 'protocols', 'coroutines',
           'systems_certificates', 'serial_depth', 'ethernet_depth',
           'edu7_depth', 'http_depth', 'service_depth'}


def local_file(name):
    if not isinstance(name, str) or not name:
        raise ValueError('Missing repository file name')
    path = (ROOT / name).resolve()
    if not path.is_relative_to(ROOT.resolve()) or not path.is_file():
        raise ValueError('Missing or escaped evidence path: ' + name)
    return path


def publication_pin(name, old_hash, current_hash):
    """Qualify only the closed nineteen-document publication successor."""
    from rebind_modern_publication import approved_pin, MANIFEST, PROOF, TOOL
    if not approved_pin(name, old_hash, current_hash):
        return None
    return dict(path=name, execution_document_sha256=old_hash,
                current_publication_sha256=current_hash,
                state='original-execution-identity-retained;current-publication-validated-separately',
                new_c_execution=False,
                proof_sha256={p:sha(local_file(p)) for p in (MANIFEST,PROOF,TOOL)})


def source_binding(report):
    source_pins = report.get('inspected_paths', report.get('input_sha256'))
    if not isinstance(source_pins, dict) or not source_pins:
        return ['Receipt has no scoped source pins'], []
    errors = []; publication = []
    for name, digest in source_pins.items():
        try:
            current = sha(local_file(name))
            if not isinstance(digest, str) or current != digest:
                binding = publication_pin(name,digest,current) if isinstance(digest,str) else None
                if binding is None: errors.append('Stale source pin: ' + name)
                else: publication.append(binding)
        except ValueError as error:
            errors.append(str(error))
    return errors, publication


def pin_errors(report):
    return source_binding(report)[0]


def execution_errors(report):
    records = report.get('executions', report.get('commands', report.get('checks', [])))
    if not records:
        return ['No command observations']
    errors = []
    for index, record in enumerate(records):
        command = record.get('command', record.get('argv'))
        if not isinstance(command, list) or not command:
            errors.append(f'Command {index} has no exact argument array')
        # Negative controls intentionally return nonzero. Their recorded expected
        # result/semantic marker must be assessed by the original producer.
        if record.get('passed') is False and not record.get('optional'):
            errors.append(f'Command {index} failed its declared expectation')
        if record.get('log_sha256'):
            try:
                if sha(local_file(record['log'])) != record['log_sha256']:
                    errors.append(f'Command {index} log changed')
            except (ValueError, KeyError) as error:
                errors.append(str(error))
        elif not isinstance(record.get('stdout'), str):
            errors.append(f'Command {index} lacks captured output or a hashed log')
    return errors


def anchors(meta):
    tests = meta.get('tests') or [test for row in meta.get('exercises', []) for test in row['tests']]
    if not tests:
        tests = [test for row in meta.get('projects', []) if isinstance(row, dict) for test in row.get('tests', [])]
    return sorted(set(tests))


def generic_errors(family, meta, report):
    errors = []
    campaign = meta.get('campaign') or {}
    expected = set(campaign.get('profiles', DEFAULT_PROFILES))
    profiles = report.get('profiles', [])
    actual = [p['compiler'] + '-' + p['profile'] for p in profiles]
    if set(actual) != expected or len(actual) != len(expected):
        errors.append('Compiler profiles differ from the current contract')
    expected_anchors = anchors(meta)
    for profile in profiles:
        label = profile['compiler'] + '-' + profile['profile']
        if profile.get('passed') is not True or set(profile.get('anchors', [])) != set(expected_anchors):
            errors.append(label + ': missing or failed anchors')
        mapping, complete = mutation_coverage(family, expected_anchors,
            profile.get('compiled_mutant_controls', []), campaign.get('mutants_per_anchor', 1))
        if not complete or profile.get('controls_by_anchor') != mapping:
            errors.append(label + ': missing per-anchor compiled controls')
        if profile['profile'] == 'tsan' and profile.get('thread_instrumentation_confirmed') is not True:
            errors.append(label + ': thread instrumentation not demonstrated')
    rejected = [r.get('test') for r in report.get('learner_stub_rejections', []) if r.get('rejected') is True]
    if sorted(rejected) != expected_anchors:
        errors.append('Each original unsolved anchor must reach its intended rejection')
    return errors


def foundation_transfer_errors(report):
    """Require observed executions, including semantic (not translation) controls."""
    from run_foundation_transfer import CASES, CONTROLS
    errors = pin_errors(report) + execution_errors(report)
    if report.get('passed') is not True or report.get('scope_source_stable') is not True:
        errors.append('Transfer campaign failed or changed scope during execution')
    profiles = report.get('profiles', [])
    actual = [p.get('compiler', '') + '-' + p.get('profile', '') for p in profiles]
    if sorted(actual) != sorted(DEFAULT_PROFILES):
        errors.append('Transfer compiler profiles incomplete or duplicated')
    commands = report.get('commands', [])
    def compiled(executable, macro):
        matches = [r for r in commands if '-D' + macro in r.get('command', [])
                   and '-o' in r['command'] and r['command'][r['command'].index('-o') + 1].replace('\\', '/').split('/')[-1] == executable]
        return (len(matches) == 1 and matches[0].get('exit_code') == 0
                and matches[0].get('expected_exit_code') == 0 and matches[0].get('passed') is True)
    def observed(executable, case, code, marker):
        matches = [r for r in commands if len(r.get('command', [])) == 2
                   and r['command'][0].replace('\\', '/').split('/')[-1] == executable
                   and r['command'][1] == case]
        return (len(matches) == 1 and matches[0].get('exit_code') == code
                and matches[0].get('expected_exit_code') == code
                and matches[0].get('passed') is True
                and marker in matches[0].get('stdout', '') + matches[0].get('stderr', ''))
    for profile in profiles:
        label = profile['compiler'] + '-' + profile['profile']
        if set(profile.get('observations', {})) != set(CASES) or any(
                not isinstance(n, int) or n <= 0 for n in profile.get('observations', {}).values()):
            errors.append(label + ': missing nonvacuous transfer cases')
        for case in CASES:
            if not observed(label, case, 0, 'FD_TRANSFER_OK:' + case + ':'):
                errors.append(label + ': missing transfer execution ' + case)
        for macro, case in CONTROLS:
            if not compiled(label + '-' + macro, macro):
                errors.append(label + ': missing successful control translation ' + macro)
            if not observed(label + '-' + macro, case, 1, 'FD_TRANSFER_CHECK:'):
                errors.append(label + ': missing compiled semantic rejection ' + macro)
    for compiler in ('gcc', 'clang'):
        for case in CASES:
            if not observed(compiler + '-learner', case, 1, 'FD_TRANSFER_CHECK:'):
                errors.append(compiler + ': transfer learner scaffold did not reject ' + case)
        for edition in ('c17', 'c23'):
            matches = [r for r in commands if len(r.get('command', [])) == 1
                       and r['command'][0].replace('\\', '/').split('/')[-1] == compiler + '-' + edition + '-comparisons']
            if len(matches) != 1 or matches[0].get('exit_code') != 0 or 'FD_COMPARE_OK:65542' not in matches[0].get('stdout', ''):
                errors.append(compiler + ': missing actual checked arithmetic comparison ' + edition)
    target = report.get('target', {})
    if target.get('undefined_symbols') != [] or target.get('libraries') != [] or not target.get('artifacts'):
        errors.append('Transfer strict object/image closure missing')
    return errors


def b06_errors(family, meta, report):
    errors = []
    expected_profiles = ['gcc-debug', 'gcc-release', 'clang-ubsan']
    if report.get('profiles') != expected_profiles or family not in report.get('families', []):
        errors.append('Missing B06 declared profile/family')
    counts = Counter(row.get('kind') for row in report.get('checks', []) if row.get('passed') is True)
    for anchor in anchors(meta):
        for profile in expected_profiles:
            if counts[profile + ':' + anchor] != 1:
                errors.append('Missing exact reference observation: ' + profile + ':' + anchor)
            mutant = profile + ':' + anchor.replace(family + '.', family + '.mutation.', 1)
            if counts[mutant] != 1:
                errors.append('Missing exact semantic mutant: ' + mutant)
        if counts['learner-stub-rejection:' + anchor] != 1:
            errors.append('Missing original learner rejection: ' + anchor)
    return errors


def special_errors(family, meta, report):
    errors = []
    commands = report.get('commands', [])
    configurations = {}
    suites = {}
    for row in commands:
        argv = row.get('command', [])
        if argv and argv[0] == 'cmake' and '-S' in argv:
            options = {x.split('=', 1)[0]: x.split('=', 1)[1] for x in argv if x.startswith('-D') and '=' in x}
            if not options.get('-DCMASTERY_ASSESSOR_ROOT'):
                continue
            mode = 'asan' if options.get('-DCMASTERY_SANITIZE') == 'ON' else options.get('-DCMAKE_BUILD_TYPE', '').lower()
            label = options.get('-DCMAKE_C_COMPILER', '') + '-' + mode
            if label in DEFAULT_PROFILES and row.get('passed') is True:
                configurations[argv[argv.index('-B') + 1]] = label
        if argv and argv[0] == 'ctest' and '--test-dir' in argv:
            suites[argv[argv.index('--test-dir') + 1]] = row
    if set(configurations.values()) != set(DEFAULT_PROFILES):
        errors.append('Missing six concrete configured compiler profiles')
    for folder, label in configurations.items():
        row = suites.get(folder, {})
        output = row.get('stdout', '')
        if row.get('passed') is not True or '100% tests passed' not in output:
            errors.append(label + ': no passing CTest suite')
        observed = re.findall(r'Test\s+#\d+:\s+(\S+)\s+\.+\s+Passed\b', output)
        observations = Counter(observed)
        for anchor in anchors(meta):
            if observations[anchor] != 1:
                errors.append(label + ': no named observation for ' + anchor)
            mutation = anchor.replace(family + '.', family + '.mutation.', 1)
            # These original producers name one control for the early batch
            # kernels, and two distinct controls for the later mechanisms.
            expected = [mutation] if family in {'b04_depth', 'b05_depth', 'b045_systems'} else [mutation + '.1', mutation + '.2']
            for name in expected:
                if observations[name] != 1:
                    errors.append(label + ': missing exact semantic control observation ' + name)
    negatives = [row for row in commands if row.get('expected') == 1 and row.get('passed') is True]
    if len(negatives) < len(anchors(meta)):
        errors.append('Insufficient separately observed learner rejection commands')
    oracle = report.get('oracle')
    if not oracle:
        errors.append('No independent oracle observations')
    elif oracle.get('passed') is False or oracle.get('status') == 'failed':
        errors.append('Independent oracle observations failed')
    return errors


def object_runtime_errors(meta, report):
    """The current producer uses generic seven-profile observations plus LLVM.

    Preserve exact current anchors/controls and the criterion-specific gate;
    the historical commands-only six-profile schema cannot describe this run.
    """
    errors = generic_errors('object_runtime', meta, report)
    required = {'assessor/object_runtime/object_runtime.c',
                'assessor/object_runtime/object_runtime_vm.c',
                'assessor/runtime/cvm.c', 'assessor/ownership/ownership.c',
                'assessor/algorithms/algo.c'}
    if not required <= set(report.get('inspected_paths', {})):
        errors.append('Object runtime: missing exact consumed provider identities')
    try:
        from check_object_runtime_followthrough import inspect as inspect_runtime
        compiler_ref = report.get('compiler_evidence', {})
        compiler_path = local_file(compiler_ref.get('path'))
        if sha(compiler_path) != compiler_ref.get('sha256'):
            errors.append('Object runtime: compiler evidence identity changed')
        host_ref = report.get('host_evidence', {})
        host_path = local_file(host_ref.get('path'))
        if sha(host_path) != host_ref.get('sha256'):
            errors.append('Object runtime: host evidence identity changed')
        errors += pin_errors(read_json(host_path)) + execution_errors(read_json(host_path))
        inspect_runtime(read_json(ROOT/'research/v5/modern/object_runtime_objective_readiness.json'),
                        report, read_json(compiler_path))
        wrapper = report.get('wrapper_executions', [])
        errors += execution_errors({'executions': wrapper})
        if not wrapper or any(r.get('exit_code') != 0 for r in wrapper):
            errors.append('Object runtime: failed or missing actual wrapper executions')
        target = report.get('target_audit', {})
        if target.get('status') != 'linked-no-unresolved-symbols' or target.get('execution') != 'not-run':
            errors.append('Object runtime: missing separate core target audit or false execution claim')
    except (ValueError, KeyError, OSError, TypeError) as error:
        errors.append('Object runtime: ' + str(error))
    return errors


def candidates(family, meta):
    if family in B06:
        return [ROOT / RECEIPTS / (family + '_campaign.json'), ROOT / RECEIPTS / 'b06_core_campaign.json',
                ROOT / 'research/v5/expansion/b06/core_validation.json']
    campaign = meta.get('campaign') or {}
    name = campaign.get('receipt', family + '_campaign.json') if isinstance(campaign, dict) else family + '_campaign.json'
    return [ROOT / RECEIPTS / name, ROOT / 'reviews/v5/expansion/b07' / name,
            ROOT / 'reviews/v5/expansion/b01' / name]


def inspect_family(family):
    meta = read_json(local_file('labs/' + family + '/family.json'))
    paths = [path for path in candidates(family, meta) if path.is_file()]
    if not paths:
        return dict(family=family, passed=False, errors=['Missing tracked execution receipt'])
    # Prefer the integrated designated receipt. A newer failed run is not hidden
    # by selecting an older success from another location.
    path = paths[0]
    report = read_json(path)
    source_errors, publication = source_binding(report)
    errors = source_errors + execution_errors(report)
    if report.get('passed') is not True:
        errors.append('Original campaign did not pass')
    if report.get('source_changed_during_run') is True or report.get('scope_source_stable') is False:
        errors.append('Sources changed during the recorded run')
    if family in B06:
        errors += b06_errors(family, meta, report)
    elif family in SPECIAL:
        errors += special_errors(family, meta, report)
        if family=='service_depth':
            from check_service_adapters_v5 import validate as validate_adapters
            adapter=report.get('adapter_campaign',{})
            try:
                adapter_path=local_file(adapter.get('path'))
                if sha(adapter_path)!=adapter.get('sha256'):errors.append('Service adapter receipt changed')
                errors+=validate_adapters(read_json(adapter_path))
            except ValueError as error:errors.append(str(error))
        if family=='b05_depth':
            from check_request_adapters import validate as validate_requests
            adapter=report.get('request_adapter_campaign',{})
            try:
                adapter_path=local_file(adapter.get('path'))
                if sha(adapter_path)!=adapter.get('sha256'):errors.append('Request adapter receipt changed')
                errors+=validate_requests(read_json(adapter_path))
            except ValueError as error:errors.append(str(error))
    elif family == 'network_followthrough':
        from check_network_followthrough import validate as validate_network
        errors += validate_network(report)
    elif family == 'object_runtime':
        errors += object_runtime_errors(meta, report)
    elif family == 'adt':
        from check_adt_marker_contract import validate as validate_adt
        errors += validate_adt(report)
    elif family in {'ownership', 'errors', 'metaprog', 'replicated'}:
        import importlib
        checker = importlib.import_module('check_'+family+'_current')
        try:
            checker.validate(report, files=True)
        except (AssertionError, KeyError, ValueError, IndexError) as error:
            errors.append('Complete '+family+' original/successor campaign rejected: '+str(error))
    elif family == 'foundation_depth':
        profiles = report.get('profiles', [])
        if {p['compiler'] + '-' + ('asan' if p['profile'] == 'sanitize' else p['profile']) for p in profiles} != set(DEFAULT_PROFILES):
            errors.append('Foundation profiles incomplete')
        if any(p.get('status') != 'passed' or p.get('compiled_mutants_rejected', 0) < 8
               or p.get('learner_stubs_rejected', 0) < 8 for p in profiles):
            errors.append('Foundation controls incomplete')
        transfer = report.get('changed_condition_campaign', {})
        try:
            transfer_path = local_file(transfer.get('path'))
            if sha(transfer_path) != transfer.get('sha256'):
                errors.append('Changed-condition receipt digest differs')
            errors += foundation_transfer_errors(read_json(transfer_path))
        except ValueError as error:
            errors.append(str(error))
    else:
        errors += generic_errors(family, meta, report)
    # These additions carry mechanism-specific oracle, runtime-closure or
    # statistical limits beyond the common compiler/anchor contract. Their
    # narrow checkers must remain part of integrated evidence acceptance.
    if family in {'corpus_codec', 'corpus_query', 'corpus_ecc', 'corpus_sketch', 'corpus_tdigest', 'corpus_spatial', 'corpus_scene', 'corpus_money', 'corpus_ring', 'corpus_shunting', 'corpus_limbs', 'corpus_munch', 'corpus_events', 'corpus_model', 'corpus_treiber'}:
        import importlib
        checker = importlib.import_module('check_'+family)
        errors += checker.validate(report)
    elif family in {'corpus_index', 'numeric_interval', 'parsing_depth', 'text_composition', 'corpus_storage', 'corpus_erase', 'corpus_escape'}:
        import importlib
        folder = {'corpus_index': 'corpus_followthrough', 'numeric_interval': 'corpus_interval',
                  'parsing_depth': 'corpus_parsing', 'text_composition': 'corpus_text',
                  'corpus_storage': 'corpus_storage', 'corpus_erase': 'corpus_erase', 'corpus_escape': 'corpus_escape'}[family]
        checker = importlib.import_module('check_'+family)
        errors += checker.check(read_json(ROOT/'research/v5/expansion'/folder/'completion_dispositions.json'))
    return dict(family=family, passed=not errors, errors=errors,
        receipt=path.relative_to(ROOT).as_posix(), receipt_sha256=sha(path),
        declared_anchors=anchors(meta), source_pins=len(report.get('inspected_paths', report.get('input_sha256', {}))),
        command_observations=len(report.get('executions', report.get('commands', report.get('checks', [])))),
        target_audit=report.get('target_audit'), limitations=report.get('limitations', report.get('limits', [])),
        execution_source_identity=report.get('scope_input_sha256',report.get('input_sha256')),
        execution_receipt_rewritten=False,
        whole_source_bytes_current=not source_errors and not publication,
        publication_successors=publication,
        source_binding_scope=('historical execution retained; exact current publication validated separately; no new C execution'
                              if publication else 'current scoped source bytes' if not source_errors else 'unvalidated source differences'))


def required_supplements():
    """Exact legacy successor required in addition to the family campaigns."""
    path=ROOT/'research/v5/expansion/performance_delegated/completion_dispositions.json'
    try:
        from check_performance_delegated_package import validate
        validate(read_json(path))
        return [dict(id='AX-PR-T43-10-T43.11.a-DELAYED',passed=True,errors=[],
                     support=path.relative_to(ROOT).as_posix(),support_sha256=sha(path),
                     scope='Actual original-workload prepared delegation; no PMU, physical or independent learner promotion')]
    except (OSError,AssertionError,ValueError,KeyError,TypeError,IndexError) as error:
        return [dict(id='AX-PR-T43-10-T43.11.a-DELAYED',passed=False,errors=[repr(error)])]


def current_campaigns_pass(rows,supplements,before,after):
    return (bool(rows) and all(r['passed'] for r in rows) and before==after
            and len(supplements)==1
            and supplements[0].get('id')=='AX-PR-T43-10-T43.11.a-DELAYED'
            and supplements[0].get('passed') is True)


def inspect():
    before = identity()['sha256']
    rows = []
    for family in manifest()['new_families']:
        try:
            rows.append(inspect_family(family))
        except (OSError, ValueError, KeyError, TypeError) as error:
            rows.append(dict(family=family, passed=False, errors=[str(error)]))
    supplements=required_supplements()
    after = identity()['sha256']
    passed = current_campaigns_pass(rows,supplements,before,after)
    return dict(schema_version=1, passed=passed, status='passed' if passed else 'failed',
        input_sha256=before, source_changed_during_run=before != after, families=rows,required_supplements=supplements,
        physical_validation='not-run', learner_mastery='not-assessed', independent_review='not-established',
        scope='Current source pins or explicitly qualified publication successors, with exact original observations; execution receipts and their original whole-source identities remain unchanged.',
        limits=['Shared B06 receipt observations are counted once in their original report, not eight executions.',
                'Only each producer\'s recorded scope is verified. Unexecuted variants and investigations remain required.',
                'This binding does not retroactively add unrecorded inputs or results to a historical campaign.'])


def main():
    report = inspect()
    dump(ROOT / RECEIPTS / 'expansion_campaigns.json', report)
    print({'passed': report['passed'], 'families': len(report['families']),
           'failures': {row['family']: row['errors'] for row in report['families'] if not row['passed']}})
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
