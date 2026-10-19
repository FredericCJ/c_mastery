"""Scoped authoring gates and byte preservation for additive exercise batches."""
from __future__ import annotations

from contextlib import contextmanager
from datetime import datetime, timezone
import hashlib
import os
from pathlib import Path
import shutil
import sqlite3
import tempfile

from common import ROOT, dump, read_json, sha


def require_research():
    edition = read_json(ROOT / 'curriculum/edition.json')
    if not edition.get('exercise_expansion'):
        return
    expansion = edition['exercise_expansion']
    batches = expansion.get('research_batches', [expansion['batch']])
    if not batches or len(batches) != len(set(batches)):
        raise ValueError('Exercise research needs distinct explicit batch gates')
    from research_batches_v5 import discover
    expected, author_evidence = discover()
    if batches != expected or expansion.get('author_evidence_batches') != author_evidence:
        raise ValueError('Research/author evidence manifest differs from canonical classified inputs')
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
            if not (ROOT / name).is_file() or _stream_digest(ROOT / name) != digest]


_COPY_CHUNK = 1024 * 1024


def _stream_digest(path):
    digest = hashlib.sha256()
    with path.open('rb') as source:
        while block := source.read(_COPY_CHUNK):
            digest.update(block)
    return digest.hexdigest()


def _copy_stream(source, destination):
    """Copy regular-file bytes with a fixed payload buffer; return digest/size."""
    destination.parent.mkdir(parents=True, exist_ok=True)
    digest, size = hashlib.sha256(), 0
    with source.open('rb') as incoming, destination.open('xb') as outgoing:
        while block := incoming.read(_COPY_CHUNK):
            outgoing.write(block)
            digest.update(block)
            size += len(block)
    return digest.hexdigest(), size


def _safe_review_path(path, reviews):
    """Never read, replace or remove a redirected receipt or parent directory."""
    if not path.is_relative_to(reviews) or path.resolve() != path:
        raise ValueError('Redirected receipt path: ' + str(path))
    if path.is_symlink() or (hasattr(path, 'is_junction') and path.is_junction()):
        raise ValueError('Linked receipt path: ' + str(path))


def _receipt_files(reviews, destination):
    """Walk without following links or materializing the entire file tree."""
    def walk_error(error):
        raise error
    for folder, directories, names in os.walk(reviews, followlinks=False, onerror=walk_error):
        base = Path(folder)
        _safe_review_path(base, reviews)
        kept = []
        for name in directories:
            path = base / name
            _safe_review_path(path, reviews)
            if not path.is_relative_to(destination):
                kept.append(name)
        directories[:] = kept
        for name in names:
            path = base / name
            _safe_review_path(path, reviews)
            if not path.is_relative_to(destination):
                if not path.is_file():
                    raise ValueError('Non-regular receipt: ' + str(path))
                yield path


class _ReceiptSnapshot:
    """Disk-backed bytes and index, with bounded copy buffers and SQLite cache.

    TemporaryDirectory is deliberately not used: failed restoration must retain
    the only complete backup for diagnosis/recovery. Successful restoration
    removes only this exact private temporary directory. No payload is loaded
    into a Python mapping; only the eventual report's changed-name list grows
    with output count.
    """
    def __init__(self, reviews, destination, snapshot_parent=None):
        self.reviews, self.destination = reviews, destination
        parent = Path(snapshot_parent or tempfile.gettempdir()).resolve()
        if parent.is_relative_to(reviews):
            raise ValueError('Receipt snapshot must be outside reviews')
        parent.mkdir(parents=True, exist_ok=True)
        self.parent = parent
        self.directory = Path(tempfile.mkdtemp(prefix='cmastery-receipts-', dir=parent)).resolve()
        self.db = None
        try:
            self.db = sqlite3.connect(self.directory / 'index.sqlite')
            self.db.execute('PRAGMA journal_mode=OFF')
            self.db.execute('PRAGMA cache_size=-2048')
            self.db.execute('PRAGMA temp_store=FILE')
            self.db.execute('PRAGMA mmap_size=0')
            self.db.execute('CREATE TABLE receipts (relative TEXT PRIMARY KEY, saved TEXT, digest TEXT, size INTEGER)')
        except BaseException:
            self.close(remove=True)
            raise
        self.files = self.bytes = self.rewritten = self.unchanged = 0

    def capture(self):
        for path in _receipt_files(self.reviews, self.destination):
            before = path.stat()
            saved = Path('bytes') / f'{self.files // 1024:06d}' / f'{self.files:09d}'
            digest, size = _copy_stream(path, self.directory / saved)
            after = path.stat()
            if (before.st_size, before.st_mtime_ns, before.st_ino) != (after.st_size, after.st_mtime_ns, after.st_ino) or size != after.st_size:
                raise ValueError('Receipt changed during snapshot: ' + str(path))
            self.db.execute('INSERT INTO receipts VALUES (?,?,?,?)',
                            (path.relative_to(self.reviews).as_posix(), saved.as_posix(), digest, size))
            self.files += 1
            self.bytes += size
        self.db.commit()

    def _restore(self, path, saved, digest):
        _safe_review_path(path, self.reviews)
        if path.is_dir():
            # A producer may replace an old file with a directory of new files.
            # Those files have already been archived/removed. Remove only empty
            # directories within that exact original-file position.
            for folder, directories, names in os.walk(path, topdown=False, followlinks=False):
                current = Path(folder)
                _safe_review_path(current, self.reviews)
                if names:
                    raise ValueError('Unarchived output blocks receipt restoration: ' + str(current))
                for name in directories:
                    child = current / name
                    _safe_review_path(child, self.reviews)
                    child.rmdir()
            path.rmdir()
        path.parent.mkdir(parents=True, exist_ok=True)
        _safe_review_path(path, self.reviews)
        descriptor, temporary = tempfile.mkstemp(prefix='.receipt-restore-', dir=path.parent)
        os.close(descriptor)
        temporary = Path(temporary)
        temporary.unlink()  # _copy_stream creates exclusively, under this parent.
        try:
            actual, _ = _copy_stream(self.directory / saved, temporary)
            if actual != digest:
                raise ValueError('Receipt backup changed: ' + str(saved))
            os.replace(temporary, path)
        finally:
            if temporary.exists():
                _safe_review_path(temporary, self.reviews)
                temporary.unlink()
        self.rewritten += 1

    def restore(self, run):
        _safe_review_path(run, self.reviews)
        updated = []
        for path in _receipt_files(self.reviews, self.destination):
            relative = path.relative_to(self.reviews).as_posix()
            old = self.db.execute('SELECT saved,digest,size FROM receipts WHERE relative=?', (relative,)).fetchone()
            if old is not None and path.stat().st_size == old[2] and _stream_digest(path) == old[1]:
                self.unchanged += 1
                continue
            archive = run / 'legacy-producer-output' / relative
            _safe_review_path(archive, self.reviews)
            _copy_stream(path, archive)
            updated.append(relative)
            if old is None:
                _safe_review_path(path, self.reviews)
                path.unlink()
            else:
                self._restore(path, old[0], old[1])
        # Stream the index: a deleted receipt is absent from the post-run walk.
        for relative, saved, digest in self.db.execute('SELECT relative,saved,digest FROM receipts ORDER BY relative'):
            path = self.reviews / relative
            _safe_review_path(path, self.reviews)
            if not path.is_file():
                self._restore(path, saved, digest)
        return updated

    def close(self, *, remove):
        if self.db is not None:
            self.db.close()
        if remove:
            # Windows: verify the final absolute target before recursive delete.
            if self.directory.parent != self.parent or self.directory.resolve() != self.directory or not self.directory.name.startswith('cmastery-receipts-'):
                raise ValueError('Unsafe snapshot cleanup: ' + str(self.directory))
            shutil.rmtree(self.directory)


@contextmanager
def preserve_receipts(destination, *, snapshot_parent=None):
    """Run old check producers, retain their new output separately, restore historical bytes.

    This extends the existing v5 historical-receipt wrapper to every pre-expansion
    receipt. New outputs are kept under the current batch, never promoted to old
    acceptance. A unique run directory retains each attempted build's records.
    All outside-destination regular files are copied to an OS-temporary disk
    snapshot (Windows defaults to the user's C: temporary drive). File payloads
    use fixed-size buffers, index metadata uses a 2MiB SQLite cache, and unchanged
    files are never rewritten. No concurrent producer or redirected review tree
    is supported. On restoration failure the disk backup is retained and named
    in the exception instead of being silently discarded.
    """
    destination = Path(destination).resolve()
    reviews = (ROOT / 'reviews').resolve()
    if reviews != ROOT.resolve() / 'reviews':
        raise ValueError('Redirected reviews directory')
    if not destination.is_relative_to(reviews):
        raise ValueError('Unsafe batch receipt destination')
    run = destination / 'runs' / datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run.mkdir(parents=True, exist_ok=False)
    snapshot = _ReceiptSnapshot(reviews, destination, snapshot_parent)
    try:
        snapshot.capture()
    except BaseException:
        snapshot.close(remove=True)  # Producer has not run; original bytes untouched.
        raise
    try:
        yield run
    finally:
        try:
            updated = snapshot.restore(run)
        except BaseException as error:
            snapshot.close(remove=False)
            raise RuntimeError('Receipt restoration failed; disk backup retained at ' + str(snapshot.directory)) from error
        try:
            # Preserve top-level scoped outputs exactly as before; nested current
            # directories remain current and are excluded from legacy restoration.
            for path in destination.iterdir():
                _safe_review_path(path, reviews)
                if path.is_file():
                    archive = run / 'current' / path.name
                    _safe_review_path(archive, reviews)
                    _copy_stream(path, archive)
            errors = historical_errors()
            from v2_common import source_identity
            pin_path = ROOT / 'curriculum/v5/expansion/historical_receipts.json'
            report = {'schema_version': 1, 'passed': not errors, 'status': 'failed' if errors else 'passed',
                      'input_sha256': source_identity()['sha256'],
                      'inspected_paths': read_json(pin_path)['files'] if pin_path.is_file() else {},
                      'changed_historical_paths': errors, 'new_legacy_outputs_retained': updated,
                      'snapshot_files': snapshot.files, 'snapshot_bytes': snapshot.bytes,
                      'restored_files': snapshot.rewritten, 'unchanged_files_not_rewritten': snapshot.unchanged,
                      'copy_buffer_bytes': _COPY_CHUNK, 'snapshot_storage': 'temporary disk; bounded SQLite index cache',
                      'scope': 'Byte preservation only; historical acceptance does not accept current source changes.'}
            dump(run / 'preservation.json', report)
            dump(destination / 'historical_preservation.json', report)
            if errors:
                raise ValueError('Historical receipts changed: ' + str(errors))
        finally:
            snapshot.close(remove=True)
