"""Scoped authoring gates and byte preservation for additive exercise batches."""
from __future__ import annotations

from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path

from common import ROOT, dump, read_json, sha


def require_research():
    edition = read_json(ROOT / 'curriculum/edition.json')
    if not edition.get('exercise_expansion'):
        return
    expansion = edition['exercise_expansion']
    batches = expansion.get('research_batches', [expansion['batch']])
    if not batches or len(batches) != len(set(batches)):
        raise ValueError('Exercise research needs distinct explicit batch gates')
    for batch in batches:
        require_batch_research(ROOT / 'research/v5/expansion' / batch.lower())


def require_batch_research(folder):
    folder = folder.resolve()
    if not folder.is_relative_to((ROOT / 'research/v5/expansion').resolve()):
        raise ValueError('Expansion research folder escapes its scope')
    gate = read_json(folder / 'research_gate.json')
    if gate.get('status') != 'passage-inspected':
        raise ValueError('Exercise research requires actual bounded passage inspections')
    pinned = gate.get('research_input_sha256', {})
    required = {p.relative_to(ROOT).as_posix() for p in folder.glob('*resources.json')}
    if not required or not required <= set(pinned):
        raise ValueError('Expansion research gate does not pin every source addition')
    shared = {}
    for name in required:
        for entry in read_json(ROOT/name).get('canonical_external_sources', []):
            path = (ROOT/entry['path']).resolve()
            if (not path.is_relative_to((ROOT/'research/v5/expansion').resolve())
                    or not path.name.endswith('resources.json') or entry['path'] not in pinned):
                raise ValueError('Invalid or unpinned shared source register')
            matches = [r for r in read_json(path)['resources'] if r['id'] == entry['id']]
            if len(matches) != 1 or not matches[0].get('inspection'):
                raise ValueError('Shared source lacks a unique inspected record: '+entry['id'])
            shared[entry['path']] = path
    for name, digest in pinned.items():
        path = (ROOT / name).resolve()
        if ((not path.is_relative_to(folder.resolve()) and shared.get(name) != path)
                or not path.is_file() or sha(path) != digest):
            raise ValueError('Expansion research changed after inspection gate: ' + name)


def historical_errors():
    path = ROOT / 'curriculum/v5/expansion/historical_receipts.json'
    if not path.is_file():
        return []
    return [name for name, digest in read_json(path)['files'].items()
            if not (ROOT / name).is_file() or sha(ROOT / name) != digest]


@contextmanager
def preserve_receipts(destination):
    """Run old check producers, retain their new output separately, restore historical bytes.

    This extends the existing v5 historical-receipt wrapper to every pre-expansion
    receipt. New outputs are kept under the current batch, never promoted to old
    acceptance. A unique run directory retains each attempted build's records.
    """
    destination = Path(destination).resolve()
    if not destination.is_relative_to((ROOT / 'reviews').resolve()):
        raise ValueError('Unsafe batch receipt destination')
    run = destination / 'runs' / datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run.mkdir(parents=True, exist_ok=False)
    before = {p: p.read_bytes() for p in (ROOT / 'reviews').rglob('*')
              if p.is_file() and not p.resolve().is_relative_to(destination)}
    try:
        yield run
    finally:
        updated = []
        for path in list((ROOT / 'reviews').rglob('*')):
            if not path.is_file() or path.resolve().is_relative_to(destination):
                continue
            old = before.get(path)
            if old is None or path.read_bytes() != old:
                relative = path.relative_to(ROOT / 'reviews')
                copy = run / 'legacy-producer-output' / relative
                copy.parent.mkdir(parents=True, exist_ok=True)
                copy.write_bytes(path.read_bytes())
                updated.append(relative.as_posix())
                if old is None:
                    path.unlink()
            if old is not None:
                path.write_bytes(old)
        for path, content in before.items():
            if not path.exists():
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(content)
        # Preserve this run's current scoped outputs as an immutable snapshot.
        for path in destination.iterdir():
            if path.is_file():
                copy = run / 'current' / path.name
                copy.parent.mkdir(parents=True, exist_ok=True)
                copy.write_bytes(path.read_bytes())
        errors = historical_errors()
        from v2_common import source_identity
        pin_path = ROOT / 'curriculum/v5/expansion/historical_receipts.json'
        report = {'schema_version': 1, 'passed': not errors, 'status': 'failed' if errors else 'passed',
                  'input_sha256': source_identity()['sha256'],
                  'inspected_paths': read_json(pin_path)['files'] if pin_path.is_file() else {},
                  'changed_historical_paths': errors, 'new_legacy_outputs_retained': updated,
                  'scope': 'Byte preservation only; historical acceptance does not accept current source changes.'}
        dump(run / 'preservation.json', report)
        dump(destination / 'historical_preservation.json', report)
        if errors:
            raise ValueError('Historical receipts changed: ' + str(errors))
