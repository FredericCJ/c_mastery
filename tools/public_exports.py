"""Reproduce public review archives without importing restricted evidence inputs."""
from __future__ import annotations

from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path, PurePosixPath
import subprocess
import sys
import zipfile

from common import ROOT, dump
from public_validation import LIMITS, run, run_command
from v2_common import source_identity

# Explicitly assigned public fixture/capture validators, inspected separately
# from author campaigns and generators. They provide finite data oracles and
# capture checks, not the withheld C implementations or transfer answers.
PUBLIC_LEARNER_TOOLS = {
    'tools/check_interrupt_handoff_capture.py', 'tools/check_interrupt_lease_capture.py',
    'tools/check_interrupt_time_capture.py', 'tools/check_network_arp_capture.py',
    'tools/check_network_ip_capture.py', 'tools/check_network_tcp_capture.py',
    'tools/verify_embedded_fixtures.py', 'tools/verify_legacy_checkpoint_fixtures.py',
    'tools/verify_systems_fixtures.py',
}

def safe_name(name):
    path = PurePosixPath(name)
    if not name or '\\' in name or ':' in name or path.is_absolute() or '..' in path.parts or path.as_posix() != name:
        raise ValueError('Unsafe archive path: ' + name)
    return path


def learner_file(name):
    """Allow public learning inputs; author scripts and assessment keys stay out."""
    path = safe_name(name)
    if name == 'research/enhancement/networking_contracts.json':
        # EDU7-v1 is a public wire-format contract used by T40/T41, not an
        # author solution or source-inspection record.
        return True
    if {'assessor', 'private', 'reviews', 'planning', '__pycache__', 'build'} & set(path.parts):
        return False
    if path.parts[0] == 'tools':
        # New tools require an explicit review rather than a broad directory grant.
        return name in PUBLIC_LEARNER_TOOLS
    if path.parts[:2] == ('research', 'v5'):
        return path.name.endswith('resources.json')
    if path.parts[0] == 'research':
        return path.name.endswith('resources.json')
    return path.parts[0] in {'curriculum', 'labs', 'fixtures', 'cmastery', 'references', 'site'} or len(path.parts) == 1


def write_archive(destination, names, root=ROOT):
    entries = {}
    with zipfile.ZipFile(destination, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for name in sorted(names):
            safe_name(name)
            path = root/name
            if not path.is_file() or path.resolve() != root.resolve()/name:
                raise ValueError('Missing or redirected export source: '+name)
            with path.open('rb') as stream:
                entries[name] = hashlib.file_digest(stream, 'sha256').hexdigest()
            archive.write(path, name)
    return entries


def extract(archive, destination):
    destination.mkdir(parents=True, exist_ok=False)
    with zipfile.ZipFile(archive) as bundle:
        names = bundle.namelist()
        if len(names) != len(set(names)):
            raise ValueError('Duplicate archive members')
        for name in names:
            safe_name(name)
            if (bundle.getinfo(name).external_attr >> 16) & 0o170000 == 0o120000:
                raise ValueError('Archive symlink')
        bundle.extractall(destination)


def reproduce(work_directory, report_path):
    work = Path(work_directory).absolute()
    if work.resolve() != work:
        raise ValueError('Redirected candidate directory')
    work.mkdir(parents=True, exist_ok=True)
    candidate = work/datetime.now(timezone.utc).strftime('public_%Y%m%dT%H%M%S%fZ')
    candidate.mkdir()
    baseline = run(candidate/'original', build=True)
    if not baseline['passed']:
        raise ValueError('Public build failed: '+str(candidate/'original/validation.json'))
    identity = baseline['source_identity']
    names = set(identity['files'])
    archives = []
    for kind, selected in [('sources', names), ('learner', {n for n in names if learner_file(n)}),
                           ('assessor', {n for n in names if n.startswith('assessor/')})]:
        archive = candidate/(kind+'.zip')
        entries = write_archive(archive, selected)
        if any(entries[name] != identity['files'][name] for name in entries):
            raise ValueError('Source changed while writing '+kind)
        archives.append(dict(kind=kind, path=str(archive), files=len(entries), entries=entries))
    extracted = candidate/'source-extracted'
    extract(candidate/'sources.zip', extracted)
    command = [sys.executable, extracted/'tools/build_v5.py', '--profile', 'public',
               '--output', extracted/'build/public']
    rebuilt = run_command(command, candidate, 'extracted-build', timeout=1200, cwd=extracted)
    if not rebuilt['passed']:
        raise ValueError('Extracted public build failed: '+rebuilt['log'])
    second = json.loads((extracted/'build/public/validation.json').read_text())
    if second['source_identity'] != identity or not second['passed']:
        raise ValueError('Extracted source identity or validation differs')
    if baseline['html'] != second['html']:
        raise ValueError('Public HTML regeneration is not a fixed point')
    learner = candidate/'learner-extracted'
    extract(candidate/'learner.zip', learner)
    checks = []
    for label, args in [('init', ['init', 'PR-T01-01', str(candidate/'learner-workspace')]),
                        ('study-init', ['study', 'init', str(candidate/'study')]),
                        ('study-next', ['study', 'next', str(candidate/'study')])]:
        checks.append(run_command([sys.executable, '-m', 'cmastery', *args], candidate,
                                  'learner-'+label, cwd=learner))
    from cmastery import route
    for project, unit in [('PR-T03-02', 'U-T03-02-01'),
                          ('PR-T24-05', 'U-T24-05-03'),
                          ('PR-T36-07', 'U-T36-07-01'),
                          ('PR-T43-09', 'U-T43-09-03')]:
        destination = candidate/('learner-'+unit)
        checks.append(run_command([sys.executable, '-m', 'cmastery', 'init', project,
                                   destination, '--unit', unit], candidate,
                                  'learner-'+unit, cwd=learner))
        if checks[-1]['passed']:
            if (destination/'ACTIVE.md').read_text() != route.render_node(unit):
                raise ValueError('Extracted learner effective specification differs: '+unit)
            if any({'assessor', 'private', 'reviews'} & set(p.relative_to(destination).parts)
                   for p in destination.rglob('*')):
                raise ValueError('Withheld path leaked into learner workspace: '+unit)
    if not all(c['passed'] for c in checks):
        raise ValueError('Extracted learner workflow failed')
    workspace = candidate/'learner-workspace'
    checked = run_command([sys.executable, '-m', 'cmastery', 'check', workspace,
                           '--compiler', 'gcc', '--profile', 'debug'], candidate,
                          'learner-check-unimplemented', cwd=learner)
    receipt_path = workspace/'evidence/latest.json'
    receipt = json.loads(receipt_path.read_text()) if receipt_path.is_file() else {}
    steps = receipt.get('steps', [])
    semantic = (checked['exit_code'] == 1 and receipt.get('status') == 'failed'
                and receipt.get('assessment') == 'learner' and len(steps) == 3
                and all(step.get('exit_code') == 0 for step in steps[:2])
                and steps[-1].get('exit_code') == 8
                and 'got NOT_IMPLEMENTED expected' in steps[-1].get('stdout', '')
                and any(t.get('name') == 'foundations.prefix' and t.get('failed')
                        for t in receipt.get('tests', [])))
    checked.update(passed=semantic, classification='expected-scaffold-rejection' if semantic
                   else 'unexpected-learner-check-outcome')
    checks.append(checked)
    if not semantic:
        raise ValueError('Extracted learner check did not semantically reject the unimplemented scaffold')
    if any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*')):
        raise ValueError('Assessor path leaked into ordinary initialized workspace')
    if source_identity() != identity:
        raise ValueError('Source changed during candidate reproduction')
    html_archive = candidate/'html.zip'
    write_archive(html_archive, baseline['html'], candidate/'original/html')
    return dict(schema_version=1, profile='public', passed=True, accepted=False, promoted=False,
                input_sha256=identity['sha256'], archives=archives, html_archive=str(html_archive),
                candidate_directory=str(candidate), extracted_build=rebuilt,
                learner_checks=checks, html_fixed_point=True, private_inputs_required=False,
                limits=LIMITS)
