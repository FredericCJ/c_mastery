"""Tracked producers for the executable-unit acceptance receipts.

Subcommands:
  machine-hosted         run tools/check_machine_hosted_v4.py (Linux/WSL) and record
                         reviews/v5/machine_hosted_acceptance.json
  firmware-integration   run tools/verify_v4_lab_units.py for the four profiles and record
                         reviews/v5/firmware_integration_acceptance.json
  family NAME            assessor pass, learner rejection and mutation rejection for a
                         fifth-edition family; records reviews/v5/new_family_acceptance.json

On Windows the C campaigns re-execute themselves under WSL. Every receipt pins
the inspected sources and never relabels an earlier success as current.
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import sys
import time
from common import ROOT, dump, read_json, sha
from v2_common import source_identity
from v5_common import RECEIPTS, manifest, pins, python_command, tree_files

MACHINE_FAMILIES = ['machine_image', 'hosted_systems']
UNIT_FAMILIES = ['algorithms', 'communications', 'foundations', 'firmware_io', 'integration']
PROFILES = [('gcc', 'debug'), ('gcc', 'release'), ('clang', 'debug'), ('clang', 'asan')]


def receipt_path(name):
    return ROOT / RECEIPTS / name


def pending(name, identity):
    dump(receipt_path(name), {'schema_version': 5, 'status': 'pending', 'passed': False,
                              'input_sha256': identity['sha256']})


def finish(name, identity, record):
    changed = source_identity()['sha256'] != identity['sha256']
    record.update(schema_version=5, input_sha256=identity['sha256'], recorded_at=datetime.now(timezone.utc).isoformat(),
                  source_changed_during_run=changed, physical_observations='not-run')
    if changed:
        record.update(status='failed', passed=False, reason='Canonical sources changed during execution.')
    dump(receipt_path(name), record)
    print(name + ': ' + record['status'])
    return 0 if record['passed'] else 1


def run_logged(command, log):
    log.parent.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
    log.write_text(completed.stdout + '\n' + completed.stderr, encoding='utf-8', newline='\n')
    return {'command': [str(c) for c in command], 'exit_code': completed.returncode,
            'elapsed_seconds': round(time.monotonic() - started, 3), 'log': log.relative_to(ROOT).as_posix(),
            'log_sha256': sha(log)}


def unit_count(families):
    return sum(len(read_json(ROOT / 'labs' / f / 'family.json').get('units', [])) for f in families)


def machine_hosted(native):
    identity = source_identity(); name = 'machine_hosted_acceptance.json'; pending(name, identity)
    work = Path('build/v5-machine-hosted')
    execution = run_logged(python_command('tools/check_machine_hosted_v4.py', '--work', work.as_posix(),
                                          platform='host' if native else 'wsl'), ROOT / work / 'campaign.log')
    report_file = ROOT / work / 'report.json'
    report = read_json(report_file) if report_file.is_file() else {'status': 'missing', 'failures': ['no report written']}
    passed = execution['exit_code'] == 0 and report.get('status') == 'passed' and not report.get('failures')
    inspected = [p for f in MACHINE_FAMILIES for p in tree_files('labs/' + f) + tree_files('assessor/' + f)]
    inspected += [ROOT / 'tools/check_machine_hosted_v4.py', ROOT / 'tools/run_unit_campaigns_v5.py']
    record = {**{k: v for k, v in report.items() if k != 'status'}, 'status': 'passed' if passed else 'failed', 'passed': passed,
              'execution': execution, 'reproduction_command': ['python', 'tools/run_unit_campaigns_v5.py', 'machine-hosted'],
              'counts': {'new_units': unit_count(MACHINE_FAMILIES),
                         'assessor_family_profile_combinations': len(report.get('profiles', [])),
                         'unimplemented_units_rejected': sum(len(l.get('rejected_units', [])) for l in report.get('learners', [])),
                         'compiled_mutants_rejected': len(report.get('mutations', []))},
              'inspected_paths': pins(inspected)}
    return finish(name, identity, record)


def firmware_integration(native):
    identity = source_identity(); name = 'firmware_integration_acceptance.json'; pending(name, identity)
    reports = []; passed = True; inspected = {}
    for compiler, profile in PROFILES:
        execution = run_logged(python_command('tools/verify_v4_lab_units.py', '--compiler', compiler, '--profile', profile,
                                              '--report', '--receipts-dir', RECEIPTS, platform='host' if native else 'wsl'),
                               ROOT / 'build' / f'v5-labs32-{compiler}-{profile}.log')
        path = receipt_path(f'labs32-{compiler}-{profile}.json')
        report = read_json(path) if path.is_file() else {}
        outcomes = [o for run in report.get('runs', []) for o in run.get('outcomes', [])]
        ok = execution['exit_code'] == 0 and bool(outcomes)
        passed = passed and ok
        for entry in report.get('inspected_paths', []):
            inspected[entry['path']] = entry['sha256']
        reports.append({'compiler': compiler, 'profile': profile, 'path': path.relative_to(ROOT).as_posix(),
                        'sha256': sha(path) if path.is_file() else None, 'outcomes': len(outcomes), 'passed': ok,
                        'execution': execution})
    for relative, digest in list(inspected.items()):
        file = ROOT / relative
        if not file.is_file() or sha(file) != digest:
            passed = False
    record = {'status': 'passed' if passed else 'failed', 'passed': passed, 'reports': reports,
              'counts': {'new_executable_units': unit_count(UNIT_FAMILIES), 'profiles': len(PROFILES),
                         'total_expected_outcomes': sum(r['outcomes'] for r in reports)},
              'scope': 'Executable mechanism evidence and isolated negative controls for the retained v4 unit families; '
                       'no physical hardware evidence.',
              'limitations': ['Clang ASan/UBSan instruments hosted unit binaries only.',
                              'Cortex-M4/M7 cross-compilation audits images; they do not execute on a board.'],
              'findings': [], 'inspected_paths': dict(sorted(inspected.items()))}
    return finish(name, identity, record)


def family(name_of_family, native):
    """Assessor pass, learner rejection and mutation rejection for a fifth-edition family."""
    sys.path.insert(0, str(ROOT))
    from cmastery.cli import instantiate, run_checks  # noqa: E402
    import tempfile
    identity = source_identity(); name = 'new_family_acceptance.json'
    existing = read_json(receipt_path(name)) if receipt_path(name).is_file() else {'families': {}}
    if os.name == 'nt' and not native:
        command = python_command('tools/run_unit_campaigns_v5.py', 'family', name_of_family, '--native', platform='wsl')
        return subprocess.run(command, cwd=ROOT).returncode
    meta = read_json(ROOT / 'labs' / name_of_family / 'family.json')
    campaign_meta = meta.get('campaign')
    campaign_meta = campaign_meta if isinstance(campaign_meta, dict) else {}
    producer = campaign_meta.get('producer')
    if name_of_family in {'kernel_lifecycle', 'architecture_ports', 'firmware_lifecycle', 'subset_compiler',
                          'verification_fuzzing', 'linux_io_depth', 'memory_defense', 'numeric_simd'}:
        # B06 uses one shared producer with a family selector and explicit output.
        # Do not overwrite the all-eight-family campaign with a one-family run.
        destination = RECEIPTS + '/' + name_of_family + '_campaign.json'
        return subprocess.run(python_command('tools/run_b06_campaign.py', '--family', name_of_family,
                                             '--output', destination), cwd=ROOT).returncode
    if name_of_family == 'foundation_depth' or producer:
        producer = producer or 'tools/run_foundation_depth.py'
        target = (ROOT / producer).resolve()
        if (not target.is_file() or target.parent != (ROOT / 'tools').resolve()
                or not target.name.startswith('run_') or target.suffix != '.py'):
            raise ValueError('Family campaign producer must be a tracked tools/run_*.py: ' + producer)
        receipt = campaign_meta.get('receipt', 'foundation_depth_campaign.json')
        if Path(receipt).name != receipt or not receipt.endswith('_campaign.json'):
            raise ValueError('Invalid family campaign receipt name: ' + receipt)
        destination = RECEIPTS + '/' + receipt
        command = python_command(producer, '--report', destination)
        return subprocess.run(command, cwd=ROOT).returncode
    if meta.get('exercises'):
        raise ValueError('Exercise family needs an explicit tracked campaign producer: ' + name_of_family)
    campaign = meta.get('campaign', {})
    units = meta.get('units', [])
    entries = meta.get('projects', [])
    project = entries[0]['id'] if entries and isinstance(entries[0], dict) else (entries[0] if entries else units[0]['project'])
    with tempfile.TemporaryDirectory(prefix='c mastery ' + name_of_family + ' ') as directory:
        workspace = Path(directory) / 'learner'
        instantiate(project, workspace, family_name=name_of_family)
        learner = run_checks(workspace, 'gcc', 'debug', False, 240, all_tests=True)
        rejects = learner['status'] == 'failed' and len(learner['steps']) == 3 and learner['steps'][2]['exit_code'] not in (None, 0)
        checks = []
        for compiler, profile in [(c, p) for c, p in PROFILES]:
            result = run_checks(workspace, compiler, profile, True, 300, all_tests=True)
            checks.append({'compiler': compiler, 'profile': profile, 'status': result['status'],
                           'mutation_score': result.get('mutation_score')})
            print(f'{name_of_family} {compiler}/{profile}: {result["status"]}', flush=True)
    mutation_catalog = ROOT / 'assessor' / name_of_family / 'v5_mutations.json'
    mutations = {'required': name_of_family in manifest()['new_families'], 'catalog': mutation_catalog.is_file(),
                 'scores': [c['mutation_score'] for c in checks if c.get('mutation_score')]}
    mutation_ok = (not mutations['required']) or (mutations['catalog'] and mutations['scores'] and
                                                  all(s.get('rejected') == s.get('total') for s in mutations['scores']))
    passed = rejects and all(c['status'] == 'passed' for c in checks) and mutation_ok
    inspected = pins(tree_files('labs/' + name_of_family) + tree_files('assessor/' + name_of_family) +
                     [ROOT / 'tools/run_unit_campaigns_v5.py', ROOT / 'cmastery/cli.py'])
    existing['families'][name_of_family] = {'representative_project': project, 'learner_scaffold_rejects': rejects,
                                            'checks': checks, 'mutations': mutations, 'units': len(units),
                                            'campaign': campaign, 'passed': passed, 'inspected_paths': inspected}
    all_passed = all(f['passed'] for f in existing['families'].values())
    record = {**existing, 'status': 'passed' if all_passed else 'failed', 'passed': all_passed,
              'inspected_paths': {k: v for f in existing['families'].values() for k, v in f['inspected_paths'].items()},
              'scope': 'Fifth-edition family suites: assessor pass under four profiles, learner scaffold rejection and '
                       'mutation rejection; per-objective reasoning, target and physical evidence remain separate.'}
    return finish(name, identity, record)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    for key in ('machine-hosted', 'firmware-integration'):
        sub.add_parser(key).add_argument('--native', action='store_true')
    fam = sub.add_parser('family'); fam.add_argument('name'); fam.add_argument('--native', action='store_true')
    args = parser.parse_args()
    if args.command == 'machine-hosted':
        return machine_hosted(args.native)
    if args.command == 'firmware-integration':
        return firmware_integration(args.native)
    return family(args.name, args.native)


if __name__ == '__main__':
    raise SystemExit(main())
