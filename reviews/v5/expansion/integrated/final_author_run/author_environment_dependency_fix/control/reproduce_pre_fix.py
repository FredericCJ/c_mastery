"""Reproduce the exact old discovery omission using synthetic capture fixtures.

This script never invokes a repository build, provisions a real primary source,
accepts a release or edits canonical files. Each run needs fresh output names.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
import zipfile

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
ROOT = next(p for p in HERE.parents if (p/'tools/candidate_author_environment.py').is_file())
EXPECTED = {
    'candidate_author_environment.py': 'aa379cd03cde17ffffbdead9009db31159a2985c40154ae0ef86c03a04f0a0b2',
    'check_candidate_author_environment.py': '435b2468eccaa129f604c2ab8808db4ae13d9b59274e0186ba7205f84eefd820',
}
sys.path.insert(0, str(ROOT/'tools'))


def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, help='Fresh JSON basename within this control directory')
    args = parser.parse_args()
    if Path(args.output).name != args.output or not args.output.endswith('.json'):
        raise ValueError('Use a JSON basename without path components')
    output = HERE/args.output
    raw = output.with_suffix('.log')
    if any(p.exists() or p.is_symlink() or p.resolve() != p for p in (output, raw)):
        raise ValueError('Fresh unredirected output files required')
    preimage = HERE.parent/'preimage'
    for name, digest in EXPECTED.items():
        if sha(preimage/name) != digest:
            raise ValueError('Changed pre-fix source: '+name)
    historical = {name: sha(HERE.parent/name) for name in ('pre_fix_rejection.json', 'pre_fix_rejection.log')}
    import candidate_author_environment as current
    import check_candidate_author_environment as guards
    from resource_identity_v5 import corrections
    consumed = [
        'tools/candidate_author_environment.py', 'tools/check_candidate_author_environment.py',
        'tools/resource_identity_v5.py', 'tools/resource_inputs_v5.py',
    ]
    source_hashes = {name: sha(ROOT/name) for name in consumed}
    spec = importlib.util.spec_from_file_location('candidate_author_environment_pre_fix', preimage/'candidate_author_environment.py')
    old = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(old)
    log = io.StringIO()
    # Keep the actual current test fixture/constants/consumer, replacing only
    # discover with the executable retained predecessor implementation.
    with patch.object(current, 'discover', old.discover):
        name = 'test_four_correction_captures_stage_for_actual_consumer_without_zip_payloads'
        suite = unittest.TestSuite([guards.CandidateEnvironmentGuards(name)])
        result = unittest.TextTestRunner(stream=log, verbosity=2).run(suite)
        assert result.testsRun == 1 and len(result.failures) == 1 and not result.errors and not result.skipped
        failure = result.failures[0][1]
        assert 'Items in the second set but not the first' in failure
        assert all(name in failure for name in ('hlc.pdf', 'crdt.pdf', 'delta.pdf', 'ot.pdf'))
        # Continue past that deliberately failing assertion in a separate
        # synthetic case: stage the old manifest and call the real reader.
        case = guards.CandidateEnvironmentGuards()
        case.setUp()
        try:
            manifest = case.correction_fixture()
            omitted = sorted(set(case.captures)-{r['destination'] for r in manifest['inputs']})
            assert omitted == sorted(case.captures)
            archive = Path(case.temp.name)/'old-discovery.zip'
            with zipfile.ZipFile(archive, 'w') as z:
                for name in sorted(case.names):
                    z.write(case.root/name, name)
            with zipfile.ZipFile(archive) as z:
                z.extractall(case.out)
            staged = current.stage(case.root, case.out, manifest, manifest, case.names)
            assert staged['files'] == 1
            try:
                corrections(case.out)
            except ValueError as error:
                consumer_error = str(error)
                assert consumer_error == 'Missing or escaping resource path: build/replicated_sources/hlc.pdf'
            else:
                raise AssertionError('Old discovery unexpectedly supplied correction captures')
        finally:
            case.doCleanups()
    assert source_hashes == {name: sha(ROOT/name) for name in consumed}
    assert historical == {name: sha(HERE.parent/name) for name in historical}
    raw.write_text(log.getvalue()+'\nActual staged correction reader: '+consumer_error+'\n', encoding='utf8', newline='\n')
    report = dict(
        schema_version=1, passed=True, recorded_at=datetime.now(timezone.utc).isoformat(),
        command=list(sys.orig_argv), working_directory=str(Path.cwd()), interpreter=sys.executable,
        control='Exact pre-fix discover() from retained source; current synthetic fixture/constants and real correction consumer retained.',
        source_sha256=source_hashes,
        predecessor_sha256={(preimage/name).relative_to(ROOT).as_posix():digest for name,digest in EXPECTED.items()},
        script_sha256=sha(Path(__file__).resolve()),
        tests_run=result.testsRun, intended_assertion_failures=len(result.failures), unexpected_errors=len(result.errors),
        omitted_capture_paths=omitted, old_manifest_staged_files=staged['files'],
        actual_correction_consumer_error=consumer_error,
        historical_results_unchanged=historical, source_unchanged=True,
        raw=dict(path=raw.name, sha256=sha(raw)),
        limits='Synthetic captures only. Intentional negative control, not a whole extracted build, new source inspection, release acceptance or independent review.',
    )
    output.write_text(json.dumps(report, indent=2)+'\n', encoding='utf8', newline='\n')
    print(json.dumps(dict(passed=True, report=str(output), report_sha256=sha(output), raw_sha256=sha(raw))))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
