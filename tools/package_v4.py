"""Validate, stage and reproduce v4 exports before replacing final bundles."""
from datetime import datetime, timezone
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile
import accept_v4
from common import ROOT, read_json, sha, dump
from v2_common import source_identity
from package import write_zip

ARCHIVES = ('c_mastery_v4_sources.zip', 'c_mastery_v4_learner.zip',
            'c_mastery_v4_assessor.zip', 'c_mastery_v4_html.zip')


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


def package():
    # Reviews and execution receipts are deliberately outside canonical identity.
    # Re-evaluate them, including their current status, before creating exports.
    accept_v4.main()
    acceptance = read_json(ROOT / 'reviews/v4/acceptance.json')
    identity = source_identity()
    if not acceptance.get('accepted') or acceptance['input_sha256'] != identity['sha256']:
        raise ValueError('Current accepted source identity is required')
    dist = bounded_directory(Path('dist'))
    candidates = bounded_directory(Path('build/release_candidates'))
    candidate = candidates / datetime.now(timezone.utc).strftime('v4_%Y%m%dT%H%M%S%fZ')
    if candidate.resolve().parent != candidates.resolve():
        raise ValueError('Unsafe release candidate')
    candidate.mkdir(exist_ok=False)
    files = [ROOT / p for p in identity['files']]
    for folder in ['planning', 'reviews']:
        files += [p for p in (ROOT / folder).rglob('*')
                  if p.is_file() and not {'private', '__pycache__'} & set(p.relative_to(ROOT).parts)
                  and not p.name.startswith('package')]
    for name in ['serve.ps1', 'c_programming_mastery_curriculum.md',
                 'c_programming_mastery_curriculum_prompt.md',
                 'c_programming_mastery_curriculum_prompt_expanded.md',
                 'c_programming_mastery_fine_grained_taxonomy.md']:
        if (ROOT / name).exists():
            files.append(ROOT / name)
    files = sorted(set(files))
    source, learner, assessor, html_zip = [candidate / name for name in ARCHIVES]
    source_entries = write_zip(source, files, ROOT)
    learner_files = [p for p in files if 'assessor' not in p.relative_to(ROOT).parts
                     and p.relative_to(ROOT).parts[0] not in {'reviews', 'planning'}]
    learner_entries = write_zip(learner, learner_files, ROOT)
    assessor_entries = write_zip(assessor, [p for p in files
                                          if p.relative_to(ROOT).parts[0] == 'assessor'], ROOT)
    with zipfile.ZipFile(learner) as archive:
        if any('assessor' in Path(name).parts for name in archive.namelist()):
            raise ValueError('Assessor implementation leaked to learner bundle')
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
    run = subprocess.run([sys.executable, '-X', 'utf8', str(check / 'tools/build_v4.py')],
                         cwd=check, capture_output=True, text=True, encoding='utf-8')
    (ROOT / 'reviews/v4/package_build.log').write_text(run.stdout + '\n' + run.stderr, encoding='utf-8')
    if run.returncode:
        raise ValueError('Extracted-source build failed; see package_build.log')
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
    report = {'schema_version': 4, 'status': 'passed', 'passed': True,
              'input_sha256': identity['sha256'], 'archives': archive_records,
              'learner_excludes_assessor': True, 'fresh_extracted_source_rebuild': True,
              'reproduced_html_files': len(original_map), 'standalone_sha256': sha(check / markdown),
              'extraction': check.relative_to(ROOT).as_posix(),
              'scope': 'Fresh source rebuild with existing locked publishing environment. '
                       'Hardware and learner mastery are not asserted.'}
    promote(staged, dist, candidate)
    return report


def main():
    report_path = ROOT / 'reviews/v4/package_report.json'
    dump(report_path, {'schema_version': 4, 'status': 'pending', 'passed': False})
    try:
        report = package()
    except BaseException as error:
        dump(report_path, {'schema_version': 4, 'status': 'failed', 'passed': False,
                           'error': type(error).__name__ + ': ' + str(error)})
        raise
    # Publish success only after every final archive has been promoted.
    dump(report_path, report)
    print('Four v4 bundles exported; fresh HTML/Markdown reproduction passed.')


if __name__ == '__main__':
    main()
