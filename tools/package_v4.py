"""Validate, stage and reproduce v4 exports before replacing final bundles."""
from datetime import datetime, timezone
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile
import accept_v4
from common import ROOT, read_json, dump
from v2_common import source_identity
from package import write_zip

ARCHIVES = ('c_mastery_v4_sources.zip', 'c_mastery_v4_learner.zip',
            'c_mastery_v4_assessor.zip', 'c_mastery_v4_html.zip')


def sha(path):
    """Bound memory while hashing retained execution inputs and large ZIPs."""
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def source_files(identity, receipts):
    """Select source and evidence without ingesting this packaging run's outputs.

    Scoped package_validation/guard records are assessment evidence. Only the
    two files this producer rewrites during packaging must be excluded; a
    basename prefix would silently remove required provider evidence.
    """
    changing = {ROOT / receipts / name for name in ('package_report.json', 'package_build.log')}
    files = [ROOT / p for p in identity['files']]
    for folder in ('planning', 'reviews'):
        files += [p for p in (ROOT / folder).rglob('*')
                  if p.is_file() and not {'private', '__pycache__'} & set(p.relative_to(ROOT).parts)
                  and p not in changing]
    for name in ('serve.ps1', 'c_programming_mastery_curriculum.md',
                 'c_programming_mastery_curriculum_prompt.md',
                 'c_programming_mastery_curriculum_prompt_expanded.md',
                 'c_programming_mastery_fine_grained_taxonomy.md'):
        if (ROOT / name).exists():
            files.append(ROOT / name)
    return sorted(set(files))


def bounded_directory(relative):
    """Do not follow a redirected build/dist directory outside its named root."""
    relative=Path(relative)
    if relative.is_absolute() or '..' in relative.parts:
        raise ValueError('Release directory must stay relative to the repository')
    path = ROOT / relative
    if path.resolve() != ROOT.resolve() / relative:
        raise ValueError('Unsafe release directory: ' + str(relative))
    path.mkdir(parents=True, exist_ok=True)
    return path


def promote(archives, dist, candidate):
    """Replace validated archives; restore prior files if promotion raises."""
    destinations = [dist / path.name for path in archives]
    for destination in destinations:
        if destination.resolve() != dist.resolve() / destination.name:
            raise ValueError('Unsafe final archive: ' + destination.name)
        if destination.exists() and not destination.is_file():
            raise ValueError('Final archive is not a file: ' + destination.name)
    backup = candidate / 'previous'
    backup.mkdir()
    previous = {}
    for destination in destinations:
        if destination.exists():
            saved = backup / destination.name
            shutil.copy2(destination, saved)
            previous[destination] = saved
    replaced = []
    try:
        for source, destination in zip(archives, destinations):
            source.replace(destination)
            replaced.append(destination)
    except BaseException:
        for destination in reversed(replaced):
            if destination in previous:
                previous[destination].replace(destination)
            else:
                destination.unlink()
        raise
    return destinations


def package(accept=None, receipts='reviews/v4', archives=ARCHIVES, candidate_prefix='v4_',
            build_script='tools/build_v4.py', schema_version=4, fixed_point=(), learner_filter=None,
            author_environment_manifest=None):
    # Reviews and execution receipts are deliberately outside canonical identity.
    # Re-evaluate them, including their current status, before creating exports.
    (accept or accept_v4.main)()
    acceptance = read_json(ROOT / receipts / 'acceptance.json')
    identity = source_identity()
    if not acceptance.get('accepted') or acceptance['input_sha256'] != identity['sha256']:
        raise ValueError('Current accepted source identity is required')
    dist = bounded_directory(Path('dist'))
    candidates = bounded_directory(Path('build/release_candidates'))
    candidate = candidates / datetime.now(timezone.utc).strftime(candidate_prefix + '%Y%m%dT%H%M%S%fZ')
    if candidate.resolve().parent != candidates.resolve():
        raise ValueError('Unsafe release candidate')
    candidate.mkdir(exist_ok=False)
    files = source_files(identity, receipts)
    author_environment=None
    environment_report={'provisioned':False,'files':0}
    if author_environment_manifest is not None:
        from candidate_author_environment import discover,validate_manifest,stage,verify_staged
        manifest_path=(ROOT/author_environment_manifest).absolute()
        if manifest_path.resolve()!=manifest_path or not manifest_path.is_relative_to(ROOT.resolve()) or not manifest_path.is_file():
            raise ValueError('Author environment manifest must be a regular unredirected repository file')
        author_environment=read_json(manifest_path)
        archive_names={p.relative_to(ROOT).as_posix() for p in files}
        expected_environment=discover(ROOT,identity,archive_names)
        validate_manifest(author_environment,expected_environment)
    source, learner, assessor, html_zip = [candidate / name for name in archives]
    source_entries = write_zip(source, files, ROOT)
    learner_files = [p for p in files if 'assessor' not in p.relative_to(ROOT).parts
                     and p.relative_to(ROOT).parts[0] not in {'reviews', 'planning'}]
    if learner_filter is not None:
        learner_files = [p for p in learner_files if learner_filter(p.relative_to(ROOT))]
    learner_entries = write_zip(learner, learner_files, ROOT)
    assessor_entries = write_zip(assessor, [p for p in files
                                          if p.relative_to(ROOT).parts[0] == 'assessor'], ROOT)
    with zipfile.ZipFile(learner) as archive:
        if any('assessor' in Path(name).parts for name in archive.namelist()):
            raise ValueError('Assessor implementation leaked to learner bundle')
        if learner_filter is not None and any(not learner_filter(Path(name)) for name in archive.namelist()):
            raise ValueError('Author-only material leaked to learner bundle')
    html = ROOT / 'build/html'
    html_files = [p for p in html.rglob('*') if p.is_file() and '.doctrees' not in p.parts
                  and p.name not in {'.buildinfo', '.buildinfo.bak'}]
    if not html_files:
        raise ValueError('Published HTML is missing')
    html_entries = write_zip(html_zip, html_files, html)
    check = candidate / 'rebuild'
    check.mkdir()
    with zipfile.ZipFile(source) as archive:
        for name in archive.namelist():
            if not (check / name).resolve().is_relative_to(check.resolve()):
                raise ValueError('Unsafe archive member')
        archive.extractall(check)
    # Ensure archive bytes, rather than a post-write filesystem observation, are
    # the accepted inputs that the independent publisher is about to rebuild.
    for entry in source_entries:
        if sha(check / entry['path']) != entry['sha256']:
            raise ValueError('Source changed while archiving: ' + entry['path'])
    for relative, digest in identity['files'].items():
        if sha(check / relative) != digest:
            raise ValueError('Archive differs from accepted source: ' + relative)
    if author_environment is not None:
        environment_report=stage(ROOT,check,author_environment,expected_environment,archive_names)
        environment_report.update(manifest=manifest_path.relative_to(ROOT).as_posix(),manifest_file_sha256=sha(manifest_path))
        dump(candidate/'author_environment_manifest.json',author_environment)
        dump(candidate/'author_environment.json',environment_report)
    run = subprocess.run([sys.executable, '-X', 'utf8', str(check / build_script)],
                         cwd=check, capture_output=True, text=True, encoding='utf-8')
    (ROOT / receipts / 'package_build.log').write_text(run.stdout + '\n' + run.stderr, encoding='utf-8')
    if run.returncode:
        raise ValueError('Extracted-source build failed; see package_build.log')
    if author_environment is not None:verify_staged(ROOT,check,author_environment)
    rebuilt = check / 'build/html'
    rebuilt_files = [p for p in rebuilt.rglob('*') if p.is_file() and '.doctrees' not in p.parts
                     and p.name not in {'.buildinfo', '.buildinfo.bak'}]
    original_map = {entry['path']: entry['sha256'] for entry in html_entries}
    rebuilt_map = {p.relative_to(rebuilt).as_posix(): sha(p) for p in rebuilt_files}
    if original_map != rebuilt_map:
        raise ValueError('Extracted build differs from published HTML')
    markdown = 'c_programming_mastery_curriculum.md'
    if sha(ROOT / markdown) != sha(check / markdown):
        raise ValueError('Extracted standalone Markdown differs')
    # Tracked generated files must be a fixed point of the extracted rebuild.
    for relative in fixed_point:
        if not (check / relative).is_file() or sha(ROOT / relative) != sha(check / relative):
            raise ValueError('Extracted rebuild changed a tracked generated file: ' + relative)
    if source_identity()['sha256'] != identity['sha256']:
        raise ValueError('Canonical source changed during packaging')
    # This also binds reviews, acceptance, inventories and standalone outputs,
    # which are exported but intentionally absent from canonical identity.
    for entry in source_entries:
        file = ROOT / entry['path']
        if not file.is_file() or sha(file) != entry['sha256']:
            raise ValueError('Export input changed during packaging: ' + entry['path'])
    current_html = {p.relative_to(html).as_posix(): sha(p) for p in html.rglob('*')
                    if p.is_file() and '.doctrees' not in p.parts
                    and p.name not in {'.buildinfo', '.buildinfo.bak'}}
    if current_html != original_map:
        raise ValueError('Published HTML changed during packaging')
    staged = [source, learner, assessor, html_zip]
    entries = [source_entries, learner_entries, assessor_entries, html_entries]
    archive_records = [{'path': (Path('dist') / p.name).as_posix(),
                        'sha256': sha(p), 'files': len(rows)} for p, rows in zip(staged, entries)]
    report = {'schema_version': schema_version, 'status': 'passed', 'passed': True,
              'input_sha256': identity['sha256'], 'archives': archive_records,
              'learner_excludes_assessor': True, 'fresh_extracted_source_rebuild': True,
              'reproduced_html_files': len(original_map), 'standalone_sha256': sha(check / markdown),
              'extraction': check.relative_to(ROOT).as_posix(),
              'author_environment':environment_report,
              'scope': 'Fresh source rebuild with existing locked publishing environment and any explicitly recorded local author inputs; provisioned inputs remain outside the ZIPs, so that result is not self-contained archive reproduction. '
                       'Hardware and learner mastery are not asserted.'}
    promote(staged, dist, candidate)
    return report


def main(receipts='reviews/v4', schema_version=4, **options):
    if receipts == 'reviews/v4' and (ROOT / 'curriculum/v5/baseline.json').is_file():
        raise SystemExit('The fifth edition is active: run tools/package_v5.py; reviews/v4 is historical.')
    report_path = ROOT / receipts / 'package_report.json'
    dump(report_path, {'schema_version': schema_version, 'status': 'pending', 'passed': False})
    try:
        report = package(receipts=receipts, schema_version=schema_version, **options)
    except BaseException as error:
        dump(report_path, {'schema_version': schema_version, 'status': 'failed', 'passed': False,
                           'error': type(error).__name__ + ': ' + str(error)})
        raise
    # Publish success only after every final archive has been promoted.
    dump(report_path, report)
    print('Four bundles exported; fresh HTML/Markdown reproduction passed.')


if __name__ == '__main__':
    main()
