"""Focused real-consumer dependency fixture; no POSIX experiment is executed."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

SUPPLIER = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
ISOLATED = HERE / 'root'
EXPECTED_SOURCE = '1fd8b84957dfaa2a6119269aa81cf5a3ea5434cbdebbf8e7176fbe73adea1f27'
PROOF = 'reviews/v5/expansion/followthrough/posix_current_successor_verification.json'
EXPORT = 'research/v5/expansion/legacy_posix_context/exports.json'

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def read(name):
    return json.loads((SUPPLIER / name).read_text(encoding='utf8'))

def write(name, value):
    target = HERE / name
    assert not target.exists(), target
    target.write_bytes((json.dumps(value, indent=2, ensure_ascii=False) + '\n').encode())

def main():
    assert not ISOLATED.exists(), 'Never reuse an altered negative fixture'
    sys.path.insert(0, str(SUPPLIER / 'tools'))
    from v2_common import source_identity
    before = source_identity()
    assert before['sha256'] == EXPECTED_SOURCE
    print('source ready', before['sha256'], len(before['files']), flush=True)
    documents = {}
    pins = {}

    def maps(name, keys):
        data = read(name)
        documents[name] = digest(SUPPLIER / name)
        for key in keys:
            for path, sha256 in data.get(key, {}).items():
                assert not Path(path).is_absolute() and '..' not in Path(path).parts
                pins.setdefault(path, []).append(dict(document=name, field='/' + key,
                                                     expected_sha256=sha256))
        return data

    maps('research/v5/expansion/legacy_posix_context/launcher_binding_successor.json', ['source_sha256'])
    export = maps(EXPORT, ['source_sha256', 'outputs_sha256'])
    maps('reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json', ['inspected_paths', 'log_sha256'])
    maps('research/v5/expansion/legacy_posix_binding_current/completion_dispositions.json', ['source_sha256'])
    maps('reviews/v5/expansion/posix_binding_current/data_exports.json', ['source_sha256'])
    for name, report_name in [('io', 'io_validation.json'), ('namespace', 'namespace_validation.json'), ('retry', 'retry_validation.json')]:
        maps('research/v5/expansion/legacy_posix_' + name + '/support.json', ['source_sha256'])
        campaign = maps('reviews/v5/expansion/legacy_posix_' + name + '/' + report_name, ['inspected_paths', 'log_sha256'])
        maps('reviews/v5/expansion/legacy_posix_' + name + '/current_export_guard.json', ['inspected_paths'])
        if name == 'retry':
            adapter = maps(campaign['reused_evidence']['adapter']['path'], ['input_sha256'])
            for row in adapter['runs']:
                maps(row['path'], ['input_sha256'])
    assert len(documents) == 35 and len(pins) == 1270
    assert len(set(documents) | set(pins)) == 1290
    proof = read(PROOF)
    withheld = export['outputs_sha256']
    assert len(withheld) == 60
    assert sum((SUPPLIER / p).stat().st_size for p in withheld) == 323580
    assert not set(withheld) & set(before['files'])
    for name, sha256 in withheld.items():
        assert digest(SUPPLIER / name) == sha256
    selected = set(before['files']) | set(documents) | set(pins) | {PROOF}
    current = {name: digest(SUPPLIER / name) for name in sorted(selected)}
    closure = dict(schema_version=1, recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                   source_identity=before, proof=dict(path=PROOF, sha256=digest(SUPPLIER / PROOF), checks=proof['checks']),
                   documents=documents, selected_pin_edges=pins, selected_current_files=current,
                   withheld_outputs=dict(document=EXPORT, document_sha256=documents[EXPORT], field='/outputs_sha256',
                                         files=withheld, count=60, bytes=323580),
                   limits=['Explicit two-supersession consumer closure only; no arbitrary JSON traversal.',
                           'Original predecessor pins remain historical and are interpreted by their unchanged guards.',
                           'Copies author evidence locally; no archive, publication, POSIX experiment or release claim.'])
    write('closure.json', closure)
    ISOLATED.mkdir()
    for index, name in enumerate(sorted(selected - set(withheld)), 1):
        source = SUPPLIER / name
        target = ISOLATED / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target)
        assert digest(target) == current[name]
        if index % 4000 == 0:
            print('copied', index, flush=True)
    assert all(not (ISOLATED / name).exists() for name in withheld)
    code = """import json,pathlib,sys
root=pathlib.Path.cwd().resolve()
sys.path.insert(0,str(root/'tools'))
import audit_active_disposition_pins as audit
assert audit.ROOT.resolve()==root
result=audit.supersessions()
assert len(result)==7
print(json.dumps({'passed':True,'consumer':'audit_active_disposition_pins.supersessions','verified_superseded':len(result),'paths':sorted(result),'root':str(root)},sort_keys=True))
"""
    argv = [sys.executable, '-B', '-X', 'utf8', '-c', code]
    env = dict(os.environ, PYTHONDONTWRITEBYTECODE='1')
    env.pop('PYTHONPATH', None)

    def execute(label):
        start = time.monotonic()
        result = subprocess.run(argv, cwd=ISOLATED, env=env, capture_output=True, timeout=600)
        elapsed = time.monotonic() - start
        stdout = HERE / (label + '.stdout')
        stderr = HERE / (label + '.stderr')
        assert not stdout.exists() and not stderr.exists()
        stdout.write_bytes(result.stdout)
        stderr.write_bytes(result.stderr)
        row = dict(argv=argv, cwd=str(ISOLATED), exit_code=result.returncode, elapsed_seconds=elapsed,
                   stdout=dict(path=stdout.relative_to(SUPPLIER).as_posix(), sha256=digest(stdout), bytes=len(result.stdout)),
                   stderr=dict(path=stderr.relative_to(SUPPLIER).as_posix(), sha256=digest(stderr), bytes=len(result.stderr)),
                   python_version=sys.version, interpreter=sys.executable,
                   source_identity_sha256=before['sha256'], proof_sha256=digest(ISOLATED / PROOF))
        write(label + '.json', row)
        print(label, result.returncode, round(elapsed, 3), flush=True)
        return result, row

    negative, negative_record = execute('negative')
    error = negative.stderr.decode('utf8', errors='replace')
    assert negative.returncode != 0 and 'FileNotFoundError' in error
    assert 'rebind_posix_launcher_metadata.py' in error and 'PR-T23-01.md' in error
    assert 'ModuleNotFoundError' not in error and 'ImportError' not in error
    write('negative_state.json', dict(absent_outputs=sorted(withheld), selected_present_files={
        name: digest(ISOLATED / name) for name in sorted(selected - set(withheld))},
        actual_error='FileNotFoundError at real launcher export outputs_sha256 read',
        no_import_failure=True, consumer_result=negative_record))
    supplied = {}
    for name, sha256 in withheld.items():
        source = SUPPLIER / name
        target = ISOLATED / name
        assert digest(source) == sha256 and not target.exists()
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target)
        assert digest(target) == sha256
        supplied[name] = sha256
    write('supplied_outputs.json', dict(source_report=EXPORT, source_report_sha256=documents[EXPORT], files=supplied,
                                       bytes=323580, copied_only_declared_outputs=True, new_execution=False))
    positive, positive_record = execute('positive')
    assert positive.returncode == 0, positive.stderr.decode('utf8', errors='replace')
    assert json.loads(positive.stdout)['verified_superseded'] == 7
    assert not positive.stderr
    after = source_identity()
    assert before == after
    assert all(digest(ISOLATED / name) == sha256 for name, sha256 in current.items())
    write('result.json', dict(schema_version=1, passed=True, source_identity_before=before['sha256'],
        source_identity_after=after['sha256'], canonical_files=len(before['files']), copied_files=len(current),
        copied_bytes=sum((SUPPLIER / name).stat().st_size for name in current),
        unchanged_original_proof_sha256=digest(SUPPLIER / PROOF),
        unchanged_original_export_sha256=digest(SUPPLIER / EXPORT),
        negative=negative_record, positive=positive_record, provisioned_outputs=60, provisioned_bytes=323580,
        closure_sha256=digest(HERE/'closure.json'), original_root_modified=False,
        limits=['Focused dependency-consumer proof, not POSIX experiment execution or a complete candidate rebuild.',
                'Same unchanged real supersessions consumer executed before and after exactly 60 declared file additions.',
                'Negative state hashes and raw exception preserved before addition; no failed candidate altered.']))
    print('passed', digest(HERE / 'result.json'), flush=True)

if __name__ == '__main__':
    main()
