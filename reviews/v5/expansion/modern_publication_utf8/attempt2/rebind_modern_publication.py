"""Closed publication-only repair; never relabel a prior whole-source execution.

The allowlist is exact. No arbitrary CP1252 decoding, code normalization, receipt
rewriting, or new execution claim is permitted. The canonical Markdown files are
the authoring inputs; this idempotent producer preserves their prior byte images.
"""
import argparse
import copy
import hashlib
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASE = 'research/v5/modern/publication_utf8'
HISTORY = 'research/v5/modern/history/publication_utf8_20261018'
MANIFEST = HISTORY + '/correction.json'
MANIFEST_SHA256 = 'dec9e7a6312029f2793d4a453011f96c6a9ee0b3493ce887fa52fbea718b23d3'
TOOL = 'tools/rebind_modern_publication.py'
PROOF = BASE + '/current_binding.json'
EXERCISES = [*(f'X-T69-{i:03}' for i in range(1,7)),
             *(f'X-T70-{i:03}' for i in range(1,9)), 'X-T66-010']
ORIGINALS = {
 'curriculum/v5/exercises/X-T69-001.md':'aa8039d003275163ba374be44f71a31e3d64b56606b4234542bf67f298e6c819',
 'curriculum/v5/exercises/X-T69-002.md':'bd1d8cf95f1cf579f69c651e9e3bbe3d607e840addab0777c356612461d857e5',
 'curriculum/v5/exercises/X-T69-003.md':'67751cf82cfdecd91158fe26512e7122c997fa05892a78923ee329a90d0f98a5',
 'curriculum/v5/exercises/X-T69-004.md':'4254b1565946c4a8eddc12d1630a80d8f4129117e9b371f2747dce114e16bae6',
 'curriculum/v5/exercises/X-T69-005.md':'2859ca27a76adf9e267a7dfd3a6f7b4831ee4e1f22400bb9cec6977355c5f7de',
 'curriculum/v5/exercises/X-T69-006.md':'7d3f1b5a45e80f12116f165960cc1a170b97f4c2cdfaded461abe624656d6eec',
 'curriculum/v5/exercises/X-T70-001.md':'86c81ec38e16d36c91a30e05f5a52f288be9b08d14d3e99b9b36c27fd236f8ff',
 'curriculum/v5/exercises/X-T70-002.md':'64d48bb0b0c389f90db0c29edf4c3458e5f9d943c564d912a2e33ab7364c38db',
 'curriculum/v5/exercises/X-T70-003.md':'6357cf71277a64f30d3d07f107ea35c179db44a18345dcf0803e6ba6fc3fb75c',
 'curriculum/v5/exercises/X-T70-004.md':'9125511767f36f5596920d8cb9d44d4a0a6bf0f3ce2820cebf3baf9fa2bdc8e9',
 'curriculum/v5/exercises/X-T70-005.md':'edacf4c13a41e3b48d3794d6a835fb4cc26840a5840ca7a72c1df4a90b618e01',
 'curriculum/v5/exercises/X-T70-006.md':'9a1682fd99eab02e32756a073521a8d76479ae5249847cbdc7cec3592207f9b5',
 'curriculum/v5/exercises/X-T70-007.md':'fa8ce34a29b2469e552a3e4b417d6208584a367464693a55f4b08aa84f310e33',
 'curriculum/v5/exercises/X-T70-008.md':'f09c59bbccc5962731516df5ff22840838017d5c0243453b9341dd4b2535cd98',
 'labs/dod/CONTRACTS.md':'7f25cd5132885eb38de570b244951b323896e56cdc48d3cfe28ffabe4083c7ab',
 'assessor/dispatch/analyses/X-T69-006.md':'ad5c5745e2f4479b54b507e74c24848e259af304ccfb4f6c23ec480e1466cde7',
 'assessor/dod/analyses/X-T70-008.md':'b36ccef43044e2a5a452326f4b8ad23cba2fff91b34747d8c516e937f43a8eb7',
 'research/v5/modern/dod_structure_measurement_analysis.md':'3751b32b792118d9d315ab736b764268ce1ca1ff5b7c7100378cb4959a46e6a2',
 'labs/ownership/authority_boundary/AUTHORITY.md':'4df303065432ac3409665d81b7e3ad5e8d76ccbe53a6999e23bb9ecb2ec0f63a',
}


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def sha(path):
    return digest((ROOT/path).read_bytes())


def read(path):
    return json.loads((ROOT/path).read_text(encoding='utf8'))


def write(path, data):
    p = ROOT/path
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(data, indent=2, ensure_ascii=True)+'\n', encoding='utf8', newline='\n')


def tokens():
    text = '\u2013'
    values = []
    for _ in range(3):
        text = text.encode('utf8').decode('cp1252')
        values.append((text, '\u2013'))
    return list(reversed(values)) + [('fa\u00c3\u00a7ade', 'fa\u00e7ade')]


def correct_text(path, text):
    """Only these canonical sources may receive these four literal repairs."""
    if path not in ORIGINALS:
        return text
    for before, after in tokens():
        text = text.replace(before, after)
    return text


def criterion_text(ident, text):
    """One retained criterion has either one or two historical encoding passes."""
    if ident != 'X-T70-007-R3':
        return text
    canonical = 'Exhaust keys0\u201331 and insertion/deletion permutations for collision pairs and differing bitmap positions; compare each version with an independent dictionary and inject every clone failure.'
    allowed = {canonical}
    for before, _ in tokens()[:3]:
        allowed.add(canonical.replace('\u2013', before))
    if text not in allowed:
        raise ValueError('Unrelated criterion change, not a punctuation successor')
    return canonical


def structure(raw):
    text = raw.decode('utf8')
    if not text.startswith('```json\n'):
        # Preserve literal CRLF rather than rewriting all document line endings.
        text = text.replace('\r\n', '\n')
    return json.loads(text.split('```json\n', 1)[1].split('\n```', 1)[0])


def mechanism_inputs():
    result = {}
    for family in ('dispatch', 'dod', 'ownership'):
        for area in ('labs', 'assessor'):
            for p in (ROOT/area/family).rglob('*'):
                if not p.is_file() or any(n in ('build', '__pycache__', 'evidence', '.git') for n in p.parts):
                    continue
                name = p.relative_to(ROOT).as_posix()
                if name not in ORIGINALS and p.suffix != '.pyc':
                    result[name] = sha(name)
    return result


def expected_record(path, old):
    before = old.decode('utf8')
    after = correct_text(path, before)
    assert before != after
    changes = [dict(line=i, before=a, after=b) for i, (a, b) in enumerate(
        zip(before.splitlines(), after.splitlines()), 1) if a != b]
    assert len(before.splitlines()) == len(after.splitlines())
    if path.startswith('curriculum/'):
        a, b = structure(old), structure(after.encode('utf8'))
        if path.endswith('X-T70-007.md'):
            for row in a['rubric']:
                row['criterion'] = criterion_text(row['id'], row['criterion'])
        assert a == b, 'Metadata change beyond the one exact criterion punctuation'
    return dict(path=path, before_sha256=digest(old), after_sha256=digest(after.encode('utf8')),
                preserved=HISTORY+'/'+path, changes=changes)


def validate_document_delta(manifest=None):
    errors = []
    d = read(MANIFEST) if manifest is None else manifest
    if (sha(MANIFEST) != MANIFEST_SHA256 or
            digest((json.dumps(d, indent=2, ensure_ascii=True)+'\n').encode('utf8')) != MANIFEST_SHA256):
        errors.append('changed immutable correction manifest')
    if d.get('operation') != 'closed-literal-publication-repair' or d.get('accepted') is not False:
        errors.append('publication operation/acceptance')
    rows = d.get('documents', [])
    if len(rows) != len(ORIGINALS) or {r.get('path') for r in rows} != set(ORIGINALS):
        errors.append('exact nineteen-file closed allowlist')
    for row in rows:
        path = row.get('path')
        if path not in ORIGINALS:
            continue
        try:
            old = (ROOT/HISTORY/path).read_bytes()
            if digest(old) != ORIGINALS[path] or row != expected_record(path, old):
                errors.append('changed preserved bytes/delta '+path)
            if (ROOT/path).read_bytes() != correct_text(path, old.decode('utf8')).encode('utf8'):
                errors.append('unrelated current byte change '+path)
        except (OSError, ValueError, AssertionError) as e:
            errors.append(path+': '+str(e))
    if d.get('unchanged_mechanism_inputs') != mechanism_inputs():
        errors.append('C/header/build/test/other family inputs changed')
    if d.get('claims') != claims():
        errors.append('changed evidence boundary')
    return errors


def _document_pin(path, old_hash, current_hash):
    """One exact historical document pin, never a blanket stale-pin exception.

    Internal literal comparison. The public predicate additionally checks the
    complete immutable closure and actual current publication-export proof.
    """
    if path not in ORIGINALS or old_hash != ORIGINALS[path]:
        return False
    old = (ROOT/HISTORY/path).read_bytes()
    if digest(old) != old_hash:
        return False
    corrected = correct_text(path, old.decode('utf8')).encode('utf8')
    return digest(corrected) == current_hash and (ROOT/path).read_bytes() == corrected


def approved_pin(path, old_hash, current_hash):
    """Fail closed without the complete current publication successor proof."""
    if path not in ORIGINALS or old_hash != ORIGINALS[path]:
        return False
    try:
        return not validate_publication_proof() and _document_pin(path, old_hash, current_hash)
    except (OSError, ValueError, KeyError, AssertionError):
        return False


def validate_publication_proof(proof=None):
    """Read-only current export/source guard; does not validate old C execution."""
    errors = validate_document_delta()
    d = read(PROOF) if proof is None else proof
    if d.get('passed') is not True or d.get('accepted') is not False or d.get('new_c_execution') is not False:
        errors.append('publication status or inflated execution claim')
    if d.get('claims') != claims() or d.get('selected_exercises') != EXERCISES:
        errors.append('exact publication scope/exercises')
    expected = set([TOOL, MANIFEST, *ORIGINALS, 'tools/selected_export_bindings.py',
                    'cmastery/cli.py', 'cmastery/exercises.py', 'cmastery/project_assets.py'])
    pins = d.get('input_sha256', {})
    if not expected <= set(pins):
        errors.append('missing publication source binding')
    for path, value in pins.items():
        p = (ROOT/path).resolve()
        if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(path) != value:
            errors.append('stale/missing publication input '+path)
    rows = d.get('exports', [])
    if len(rows) != len(EXERCISES) or [r.get('exercise') for r in rows] != EXERCISES:
        errors.append('missing/reordered/duplicate actual export')
    sys.path.insert(0, str(ROOT))
    from cmastery.exercises import parse_brief, learner_brief
    for row in rows:
        ident = row.get('exercise')
        if ident not in EXERCISES:
            continue
        meta, _ = parse_brief(ROOT/'curriculum/v5/exercises'/f'{ident}.md')
        dest = (ROOT/row['workspace']).resolve()
        cmd = row.get('command', [])
        if (len(cmd) != 8 or cmd[1:4] != ['-m','cmastery','init'] or cmd[4] != meta['project']
                or cmd[6:] != ['--exercise',ident] or row.get('returncode') != 0):
            errors.append('wrong export command/result '+ident)
        # Command paths record the actual WSL invocation; the artifact identity
        # remains repository-relative for Windows read-only verification.
        suffix = '/'+row['workspace']
        if len(cmd) > 5 and not str(cmd[5]).replace('\\','/').endswith(suffix):
            errors.append('export command/workspace mismatch '+ident)
        if not dest.is_relative_to((ROOT/'build').resolve()) or not dest.is_dir():
            errors.append('missing/escaped export '+ident); continue
        current = {p.relative_to(dest).as_posix():digest(p.read_bytes()) for p in dest.rglob('*') if p.is_file()}
        if current != row.get('files') or any('assessor' in Path(n).parts for n in current):
            errors.append('changed export or private leakage '+ident)
        source_contract = row.get('canonical_contract', {})
        if any(source_contract.get(k) != v for k,v in meta.items()) or source_contract.get('brief_sha256') != sha('curriculum/v5/exercises/'+ident+'.md'):
            errors.append('selected canonical metadata/rubric/variants changed '+ident)
        for name, value in source_contract.get('bound_files', {}).items():
            if pins.get(name) != value or sha(name) != value:
                errors.append('missing/changed selected bound file '+name)
        if (dest/'EXERCISE.md').read_text(encoding='utf8') != learner_brief(source_contract):
            errors.append('exported complete learner brief mismatch '+ident)
        for name in meta['shared_contracts']+meta['fixtures']:
            if digest((dest/name).read_bytes()) != sha(name):
                errors.append('changed supplied contract/fixture '+name)
    return errors


def export_current():
    assert not (ROOT/PROOF).exists(), 'Preserve an earlier export proof before producing a successor'
    assert not validate_document_delta()
    sys.path[:0] = [str(ROOT), str(ROOT/'tools')]
    from cmastery import exercises
    from selected_export_bindings import bind_completed_exports
    sources = [TOOL,MANIFEST,*ORIGINALS,'curriculum/v5/exercises.json',
               *[p.relative_to(ROOT).as_posix() for p in (ROOT/'cmastery').glob('*.py')]]
    before = {p:sha(p) for p in sources}
    work = ROOT/'build/modern-publication-exports'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    rows = []
    for ident in EXERCISES:
        item = exercises.exercise(ident)
        dest = work/ident
        cmd = [sys.executable,'-m','cmastery','init',item['project'],str(dest),'--exercise',ident]
        p = subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180)
        row = dict(exercise=ident,command=cmd,returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,
                   workspace=dest.relative_to(ROOT).as_posix())
        rows.append(row); write(work.relative_to(ROOT).as_posix()+'/commands.json', rows)
        assert p.returncode == 0, (ident,p.stdout,p.stderr)
        exercises.workspace_contract(dest,ident)
        row['canonical_contract'] = item
        row['files'] = {q.relative_to(dest).as_posix():digest(q.read_bytes()) for q in dest.rglob('*') if q.is_file()}
        assert not any('assessor' in Path(n).parts for n in row['files'])
    selected = bind_completed_exports(ROOT,before,rows)
    assert selected == EXERCISES
    result = dict(passed=True,accepted=False,new_c_execution=False,claims=claims(),
                  selected_exercises=EXERCISES,exports=rows,input_sha256=before,
                  observed_utc=datetime.now(timezone.utc).isoformat(),
                  whole_catalog_policy='Full catalog checked before/after actual exports; retained pins bind every selected canonical contract and supplied asset.')
    errors = validate_publication_proof(result)
    if errors:
        result['passed'] = False; result['errors'] = errors
        write(work.relative_to(ROOT).as_posix()+'/failed_binding.json',result)
        raise ValueError(errors)
    write(PROOF,result)


def claims():
    return ['Publication-only correction of nineteen exact documents; no C, header, build, test or fixture change.',
            'Original execution receipts and original document bytes remain preserved; their old whole-source identities are not called current.',
            'Only exact old-document pins may be identified as publication successors; all other source and command checks remain required.',
            'No new C execution, independent review, learner mastery, physical observation or release acceptance follows.']


def apply():
    if (ROOT/MANIFEST).exists():
        errors = validate_document_delta()
        if errors:
            raise ValueError(errors)
        return
    original = {p: (ROOT/p).read_bytes() for p in ORIGINALS}
    assert all(digest(b) == ORIGINALS[p] for p, b in original.items())
    records = [expected_record(p, b) for p, b in original.items()]
    unchanged = mechanism_inputs()
    # Every byte image is preserved before the first canonical file is changed.
    for p, b in original.items():
        dest = ROOT/HISTORY/p
        dest.parent.mkdir(parents=True, exist_ok=True)
        assert not dest.exists()
        dest.write_bytes(b)
    for p, b in original.items():
        (ROOT/p).write_bytes(correct_text(p, b.decode('utf8')).encode('utf8'))
    write(MANIFEST, dict(schema_version=1, operation='closed-literal-publication-repair', accepted=False,
          documents=records, unchanged_mechanism_inputs=unchanged, claims=claims(),
          codepoints=[dict(before=a, before_codepoints=['U+%04X'%ord(c) for c in a], after=b,
                          after_codepoints=['U+%04X'%ord(c) for c in b]) for a, b in tokens()]))
    assert not validate_document_delta()


def controls():
    d = read(MANIFEST)
    edits = [lambda x:x['documents'].pop(), lambda x:x['documents'].append(x['documents'][0]),
             lambda x:x['documents'][0].update(before_sha256='0'*64),
             lambda x:x['documents'][0].update(after_sha256='0'*64),
             lambda x:x['documents'][0]['changes'][0].update(after='changed semantic requirement'),
             lambda x:x['unchanged_mechanism_inputs'].pop(next(iter(x['unchanged_mechanism_inputs']))),
             lambda x:x['unchanged_mechanism_inputs'].update({'assessor/dod/dod.c':'0'*64}),
             lambda x:x.update(accepted=True), lambda x:x.update(operation='whole-source-current'),
             lambda x:x['claims'].append('new execution')]
    for edit in edits:
        bad = copy.deepcopy(d); edit(bad)
        assert validate_document_delta(bad), 'Invalid publication successor accepted'
    sample = next(iter(ORIGINALS)); current = sha(sample)
    assert _document_pin(sample, ORIGINALS[sample], current)
    assert not _document_pin(sample, '0'*64, current)
    assert not _document_pin(sample, ORIGINALS[sample], '0'*64)
    assert not _document_pin('assessor/dod/dod.c', sha('assessor/dod/dod.c'), sha('assessor/dod/dod.c'))
    assert correct_text('other.md', tokens()[0][0]) == tokens()[0][0]
    try:
        criterion_text('X-T70-007-R3', 'Exhaust keys0 through 63')
    except ValueError:
        pass
    else:
        raise AssertionError('Changed criterion admitted')
    # Even a coordinated map rewrite is not an independently preserved baseline.
    bad=copy.deepcopy(d)
    bad['unchanged_mechanism_inputs']['assessor/dod/dod.c']='1'*64
    bad['documents'][0]['before_sha256']='2'*64
    assert 'changed immutable correction manifest' in validate_document_delta(bad)
    return len(edits)+6


def export_controls():
    d = read(PROOF)
    edits = [lambda x:x['exports'].pop(), lambda x:x.update(passed=False),
             lambda x:x.update(accepted=True), lambda x:x.update(new_c_execution=True),
             lambda x:x['input_sha256'].pop(TOOL),
             lambda x:x['exports'][0].update(returncode=1),
             lambda x:x['exports'][0]['command'].__setitem__(-1,'X-T69-012'),
             lambda x:x['exports'][0]['files'].pop('EXERCISE.md'),
             lambda x:x['exports'][0]['canonical_contract']['rubric'].pop(),
             lambda x:x['exports'][0]['canonical_contract'].pop('delayed_transfer',None)]
    # The final edit must remove whichever accepted schema spelling is present.
    def lose_variant(x):
        c = x['exports'][0]['canonical_contract']
        c.pop('delayed_transfer',None);c.pop('delayed_assessment',None)
    edits[-1] = lose_variant
    for edit in edits:
        bad = copy.deepcopy(d); edit(bad)
        assert validate_publication_proof(bad), 'Invalid export successor accepted'
    return len(edits)


def main():
    p = argparse.ArgumentParser(); p.add_argument('--apply', action='store_true')
    p.add_argument('--exports', action='store_true')
    p.add_argument('--self-test', action='store_true'); p.add_argument('--output'); a = p.parse_args()
    if a.apply:
        apply()
    if a.exports:
        export_current()
    errors = validate_document_delta()
    count = controls() if a.self_test and not errors else 0
    if (ROOT/PROOF).exists():
        errors += validate_publication_proof()
        if a.self_test and not errors:
            count += export_controls()
    result = dict(passed=not errors, accepted=False, documents=len(ORIGINALS), negative_controls=count,
                  errors=errors, claims=claims(), input_sha256={n:sha(n) for n in [TOOL,MANIFEST,*ORIGINALS]},
                  exports_validated=(ROOT/PROOF).exists() and not errors)
    if a.output:
        out = (ROOT/a.output).resolve()
        assert out.is_relative_to((ROOT/'reviews/v5/expansion').resolve()) and not out.exists()
        write(out.relative_to(ROOT).as_posix(), result)
    print(json.dumps(result, indent=2)); return bool(errors)


if __name__ == '__main__':
    raise SystemExit(main())
