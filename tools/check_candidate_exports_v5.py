"""Build and reproduce review candidates without accepting or promoting a release.

Uses the release packager's file selection, ZIP writer, learner predicate and
fixed-point list. All outputs stay in a fresh candidate directory. The current
author publication must pass first; this does not waive any acceptance gate.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time
import zipfile

from common import ROOT, dump, read_json
from package import write_zip
from package_v4 import source_files
from package_v5 import FIXED_POINT, learner_file
from v2_common import source_identity
from v5_common import RECEIPTS


def sha(path):
    """Archives can exceed RAM; hash files through a bounded buffer."""
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def verify_identity(expected, observed):
    if observed.get('files') != expected['files'] or observed.get('sha256') != expected['sha256']:
        before, after = expected['files'], observed.get('files', {})
        changed = sorted(name for name in set(before) | set(after) if before.get(name) != after.get(name))
        raise ValueError('Rebuilt canonical source identity differs: '+str(changed[:20]))


def rebuilt_identity(extracted, candidate):
    # Import the extracted canonical selector in a fresh interpreter. Comparing
    # only its build manifest or FIXED_POINT misses other regenerated sources.
    command = [sys.executable, '-X', 'utf8', '-c',
               "import json,sys; sys.path.insert(0,'tools'); from v2_common import source_identity; print(json.dumps(source_identity()))"]
    raw, errors = candidate/'rebuilt_identity.json', candidate/'rebuilt_identity.stderr'
    started = time.monotonic()
    with raw.open('w', encoding='utf8', newline='\n') as output, errors.open('w', encoding='utf8', newline='\n') as stderr:
        process = subprocess.run(command, cwd=extracted, stdout=output, stderr=stderr)
    execution = dict(command=command,cwd=str(extracted),exit_code=process.returncode,
                     seconds=round(time.monotonic()-started,3),raw=str(raw),raw_sha256=sha(raw),
                     stderr=str(errors),stderr_sha256=sha(errors))
    dump(candidate/'identity_execution.json', execution)
    if process.returncode:
        raise ValueError('Post-build source identity failed; inspect '+str(errors))
    return read_json(raw), execution


def html_map(folder):
    return {p.relative_to(folder).as_posix(): sha(p) for p in folder.rglob('*')
            if p.is_file() and '.doctrees' not in p.parts
            and p.name not in {'.buildinfo', '.buildinfo.bak'}}


def verify_members(names, destination):
    for name in names:
        path = Path(name)
        if path.is_absolute() or '..' in path.parts or not (destination/path).resolve().is_relative_to(destination.resolve()):
            raise ValueError('Unsafe archive member: '+name)


def verify_export_files(files, base):
    base = base.resolve()
    for path in files:
        if not path.is_file() or not path.is_relative_to(base) or path.resolve() != path:
            raise ValueError('Redirected or non-regular export source: '+str(path))


def run(work_directory, report_path, author_environment_manifest=None):
    identity = source_identity()
    publication = read_json(ROOT/RECEIPTS/'publication.json')
    if not publication.get('passed') or publication.get('input_sha256') != identity['sha256']:
        raise ValueError('A current successful author publication is required for reproduction')
    parent = work_directory.absolute()
    if parent.resolve() != parent:
        raise ValueError('Candidate directory must not traverse a redirected path')
    parent.mkdir(parents=True, exist_ok=True)
    candidate = parent/datetime.now(timezone.utc).strftime('v5_review_%Y%m%dT%H%M%S%fZ')
    candidate.mkdir(exist_ok=False)
    files = [p for p in source_files(identity, RECEIPTS) if p.resolve() != report_path.resolve()]
    verify_export_files(files, ROOT)
    from candidate_author_environment import discover,require_opt_in,stage,verify_staged
    archive_names={p.relative_to(ROOT).as_posix() for p in files}
    expected_environment=discover(ROOT,identity,archive_names)
    supplied_environment=None
    if author_environment_manifest is not None:
        manifest_path=(ROOT/author_environment_manifest).absolute()
        verify_export_files([manifest_path],ROOT)
        supplied_environment=read_json(manifest_path)
    require_opt_in(expected_environment,supplied_environment)
    source = candidate/'sources.zip'
    learner = candidate/'learner.zip'
    assessor = candidate/'assessor.zip'
    html_archive = candidate/'html.zip'
    print('Creating source candidate', flush=True)
    entries = write_zip(source, files, ROOT)
    learner_files = [p for p in files if 'assessor' not in p.relative_to(ROOT).parts
                     and p.relative_to(ROOT).parts[0] not in {'reviews', 'planning'}
                     and learner_file(p.relative_to(ROOT))]
    print('Creating learner and assessor candidates', flush=True)
    learner_entries = write_zip(learner, learner_files, ROOT)
    assessor_entries = write_zip(assessor, [p for p in files if p.relative_to(ROOT).parts[0]=='assessor'], ROOT)
    with zipfile.ZipFile(learner) as archive:
        for name in archive.namelist():
            path = Path(name)
            if 'assessor' in path.parts or path.parts[0] in {'reviews','planning'} or not learner_file(path):
                raise ValueError('Author material in learner candidate: '+name)
    extracted = candidate/'extracted'
    extracted.mkdir()
    print('Extracting and verifying candidate bytes', flush=True)
    with zipfile.ZipFile(source) as archive:
        verify_members(archive.namelist(), extracted)
        archive.extractall(extracted)
    for entry in entries:
        if sha(extracted/entry['path']) != entry['sha256']:
            raise ValueError('Archive bytes differ: '+entry['path'])
    for name, digest in identity['files'].items():
        if sha(extracted/name) != digest:
            raise ValueError('Candidate source identity differs: '+name)
    environment=dict(provisioned=False,files=0,archive_self_contained=True)
    if supplied_environment is not None:
        environment=stage(ROOT,extracted,supplied_environment,expected_environment,archive_names)
        dump(candidate/'author_environment_manifest.json',supplied_environment)
        environment.update(manifest=str(manifest_path),manifest_file_sha256=sha(manifest_path))
    dump(candidate/'author_environment.json',environment)
    before_html = html_map(ROOT/'build/html')
    if not before_html:
        raise ValueError('Current HTML publication missing')
    html_files = [ROOT/'build/html'/name for name in sorted(before_html)]
    verify_export_files(html_files, ROOT/'build/html')
    html_entries = write_zip(html_archive, html_files, ROOT/'build/html')
    if {r['path']:r['sha256'] for r in html_entries} != before_html:
        raise ValueError('HTML changed while creating candidate archive')
    command = [sys.executable, '-X', 'utf8', str(extracted/'tools/build_v5.py')]
    log = candidate/'rebuild.log'
    print('Running actual extracted-source build: '+str(log), flush=True)
    started = time.monotonic()
    with log.open('w', encoding='utf8', newline='\n') as output:
        process = subprocess.run(command, cwd=extracted, stdout=output, stderr=subprocess.STDOUT)
    execution = dict(command=command,cwd=str(extracted),exit_code=process.returncode,
                     seconds=round(time.monotonic()-started,3),raw=str(log),raw_sha256=sha(log))
    dump(candidate/'execution.json', execution)
    if process.returncode:
        raise ValueError('Extracted-source build failed; inspect '+str(log))
    after_identity, identity_execution = rebuilt_identity(extracted, candidate)
    verify_identity(identity, after_identity)
    if supplied_environment is not None:verify_staged(ROOT,extracted,supplied_environment)
    if before_html != html_map(extracted/'build/html'):
        raise ValueError('Extracted HTML differs from current publication')
    for name in ['c_programming_mastery_curriculum.md', *FIXED_POINT]:
        if sha(ROOT/name) != sha(extracted/name):
            raise ValueError('Generated output is not a fixed point: '+name)
    if source_identity()['sha256'] != identity['sha256']:
        raise ValueError('Canonical source changed during reproduction')
    for entry in entries:
        if sha(ROOT/entry['path']) != entry['sha256']:
            raise ValueError('Export input changed during reproduction: '+entry['path'])
    if before_html != html_map(ROOT/'build/html'):
        raise ValueError('Original publication changed during reproduction')
    return dict(passed=True,accepted=False,promoted=False,input_sha256=identity['sha256'],
                candidate_directory=str(candidate),execution=execution,
                rebuilt_source_identity=after_identity['sha256'],identity_execution=identity_execution,
                archives=[dict(path=str(path),sha256=sha(path),files=len(rows)) for path,rows in
                          [(source,entries),(learner,learner_entries),(assessor,assessor_entries),(html_archive,html_entries)]],
                reproduced_html_files=len(before_html),fixed_point_files=len(FIXED_POINT)+1,
                learner_excludes_assessor=True,
                author_environment=environment,
                scope='Actual candidate ZIP selection and extracted-source publication reproduction in the explicitly recorded author environment; this is not a self-contained ZIP claim when excluded inputs were supplied. No release acceptance, promotion, learner mastery or physical claim.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-directory',type=Path,default=ROOT/'build/candidate_checks')
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--author-environment-manifest',type=Path,help='Explicit exact active-pin environment manifest; supplied bytes stay outside every archive')
    parser.add_argument('--profile', choices=('public', 'author-evidence'), default='author-evidence')
    args = parser.parse_args()
    report = (ROOT/args.report).resolve()
    if not report.is_relative_to((ROOT/'reviews').resolve()) or report.exists():
        raise ValueError('Report must be a fresh path inside reviews')
    try:
        if args.profile == 'public':
            if args.author_environment_manifest is not None:
                raise ValueError('Public reproduction cannot provision private author inputs')
            from public_exports import reproduce
            result = reproduce(args.work_directory, report)
        else:
            result = run(args.work_directory,report,args.author_environment_manifest)
    except Exception as error:
        dump(report,dict(passed=False,accepted=False,promoted=False,error=type(error).__name__+': '+str(error)))
        raise
    dump(report,result)
    print('Candidate reproduction passed; release not accepted or promoted',flush=True)


if __name__=='__main__':
    main()
