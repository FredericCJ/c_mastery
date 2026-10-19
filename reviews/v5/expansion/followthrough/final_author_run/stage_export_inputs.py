"""Stage the established source export selection, including required review builds.

Run only after all writers and candidate reproduction have finished. This is a
Git preparation record, not curriculum acceptance or release promotion.
"""
from pathlib import Path
import hashlib
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[5]
sys.path.insert(0, str(ROOT / 'tools'))
from package_v4 import source_files
from v2_common import source_identity
from v5_common import RECEIPTS


def git(*arguments, **kwargs):
    return subprocess.run(['git', *arguments], cwd=ROOT, check=True, **kwargs)


def main():
    publication = json.loads((ROOT / RECEIPTS / 'publication.json').read_text(encoding='utf8'))
    identity = source_identity()
    if not publication.get('passed') or publication.get('input_sha256') != identity['sha256']:
        raise ValueError('Current successful publication required before staging')
    files = source_files(identity, RECEIPTS)
    names = [p.relative_to(ROOT).as_posix() for p in files]
    forbidden = {'private', '__pycache__'}
    if any(forbidden.intersection(Path(name).parts) or
           Path(name).parts[0] in {'seed', '.frama-c'} or name == 'a-header.su' for name in names):
        raise ValueError('Unrelated or excluded path in export selection')
    if any(p.resolve() != p or not p.is_file() for p in files):
        raise ValueError('Redirected or non-regular export input')
    temporary = ROOT / 'build/final_git_staging'
    temporary.mkdir(parents=True, exist_ok=True)
    paths = temporary / 'export-inputs.nul'
    paths.write_bytes(b'\0'.join(name.encode('utf8') for name in names) + b'\0')
    # The checkout began with no tracked changes. Stage authorized tracked
    # deletions too, then force only the exact eligible export inputs: build/
    # also ignores required nested reviews/**/build evidence by default.
    git('add', '-u')
    git('--literal-pathspecs', 'add', '-f', '--pathspec-from-file=' + str(paths), '--pathspec-file-nul')
    tracked = set(git('ls-files', '-z', capture_output=True).stdout.decode('utf8').split('\0'))
    missing = sorted(set(names) - tracked)
    unrelated = sorted(name for name in tracked if name and (
        Path(name).parts[0] in {'seed', '.frama-c'} or name == 'a-header.su'))
    if missing or unrelated:
        raise ValueError({'missing_export_inputs': missing, 'unrelated_staged_paths': unrelated})
    git('diff', '--quiet')
    report = dict(source_identity=identity['sha256'], selected_files=len(names),
                  selected_paths_sha256=hashlib.sha256(paths.read_bytes()).hexdigest(),
                  missing_export_inputs=missing, unrelated_staged_paths=unrelated,
                  working_tree_matches_index=True, accepted=False, promoted=False,
                  scope='Established export-input selection staged; private captures, caches and unrelated files excluded. No commit performed by this script.')
    destination = ROOT / RECEIPTS / 'final_author_run/staging.json'
    destination.write_text(json.dumps(report, indent=2) + '\n', encoding='utf8')
    git('--literal-pathspecs', 'add', '-f', '--', str(destination.relative_to(ROOT)))
    print(json.dumps(report), flush=True)


if __name__ == '__main__':
    main()
