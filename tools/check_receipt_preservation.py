"""Real temporary-tree preservation tests; never snapshot the repository reviews."""
import hashlib
from contextlib import closing
import json
import os
from pathlib import Path
import sqlite3
import tempfile
import tracemalloc
import unittest
from unittest.mock import patch

import expansion_evidence as evidence
import v2_common


class ReceiptPreservationGuards(unittest.TestCase):
    def setUp(self):
        self.sandbox = tempfile.TemporaryDirectory(prefix='cmastery preservation guard ')
        self.root = Path(self.sandbox.name).resolve()
        self.reviews = self.root / 'reviews'
        self.destination = self.reviews / 'v5' / 'current'
        self.destination.mkdir(parents=True)
        self.backups = self.root / 'temporary disk'
        self.backups.mkdir()
        self.patch_root = patch.object(evidence, 'ROOT', self.root)
        self.patch_identity = patch.object(v2_common, 'source_identity', return_value={'sha256': 'synthetic-current-source'})
        self.patch_root.start()
        self.patch_identity.start()
        self.addCleanup(self.patch_root.stop)
        self.addCleanup(self.patch_identity.stop)
        self.addCleanup(self.sandbox.cleanup)

    def put(self, relative, data):
        p = self.root / relative
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(data)
        return p

    def guard(self):
        return evidence.preserve_receipts(self.destination, snapshot_parent=self.backups)

    def report(self, run):
        return json.loads((run/'preservation.json').read_text(encoding='utf8'))

    def test_changed_new_deleted_and_unchanged_receipts(self):
        changed = self.put('reviews/v4/result.bin', b'original\x00\xff')
        deleted = self.put('reviews/other/deleted.json', b'old deleted')
        unchanged = self.put('reviews/current-old/unchanged.txt', b'untouched')
        os.utime(unchanged, ns=(1_000_000_000, 1_000_000_000))
        before = unchanged.stat().st_mtime_ns
        with self.guard() as run:
            changed.write_bytes(b'new result\x00\xfe')
            deleted.unlink()
            added = self.put('reviews/new/nested/new.bin', b'new bytes')
        self.assertEqual(changed.read_bytes(), b'original\x00\xff')
        self.assertEqual(deleted.read_bytes(), b'old deleted')
        self.assertEqual(unchanged.stat().st_mtime_ns, before)
        self.assertFalse(added.exists())
        self.assertEqual((run/'legacy-producer-output/v4/result.bin').read_bytes(), b'new result\x00\xfe')
        self.assertEqual((run/'legacy-producer-output/new/nested/new.bin').read_bytes(), b'new bytes')
        report = self.report(run)
        self.assertTrue(report['passed'])
        self.assertEqual(report['snapshot_files'], 3)
        self.assertEqual(report['restored_files'], 2)
        self.assertEqual(report['unchanged_files_not_rewritten'], 1)
        self.assertEqual(set(report['new_legacy_outputs_retained']), {'v4/result.bin','new/nested/new.bin'})
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_current_destination_and_nested_outputs_remain_current(self):
        current = self.put('reviews/v5/current/current.bin', b'old current')
        nested = self.put('reviews/v5/current/nested/detail.txt', b'old nested')
        sibling = self.put('reviews/v5/current-other/receipt.txt', b'old sibling')
        with self.guard() as run:
            current.write_bytes(b'new current')
            nested.write_bytes(b'new nested')
            sibling.write_bytes(b'new sibling')
            created = self.put('reviews/v5/current/new.txt', b'new scoped')
        self.assertEqual(current.read_bytes(), b'new current')
        self.assertEqual(nested.read_bytes(), b'new nested')
        self.assertEqual(created.read_bytes(), b'new scoped')
        self.assertEqual(sibling.read_bytes(), b'old sibling')
        self.assertEqual((run/'current/current.bin').read_bytes(), b'new current')
        self.assertEqual((run/'current/new.txt').read_bytes(), b'new scoped')
        self.assertFalse((run/'current/nested').exists())  # Original top-level snapshot contract.

    def test_exception_restores_and_archives_without_swallowing_exception(self):
        p = self.put('reviews/v3/result.txt', b'before')
        with self.assertRaisesRegex(LookupError, 'producer failed'):
            with self.guard() as run:
                p.write_bytes(b'failed output')
                self.put('reviews/v5/current/attempt.txt', b'current failure')
                raise LookupError('producer failed')
        self.assertEqual(p.read_bytes(), b'before')
        self.assertEqual((run/'legacy-producer-output/v3/result.txt').read_bytes(), b'failed output')
        self.assertEqual((run/'current/attempt.txt').read_bytes(), b'current failure')
        self.assertTrue(self.report(run)['passed'])
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_all242_historical_digests_are_checked_after_restoration(self):
        pins = {}
        for n in range(242):
            relative = f'reviews/v4/receipt-{n:03d}.json'
            data = f'historical-{n}'.encode()
            self.put(relative, data)
            pins[relative] = hashlib.sha256(data).hexdigest()
        self.put('curriculum/v5/expansion/historical_receipts.json', json.dumps({'files':pins}).encode())
        with self.guard() as run:
            (self.root/'reviews/v4/receipt-000.json').write_bytes(b'changed')
            (self.root/'reviews/v4/receipt-241.json').unlink()
        self.assertEqual(evidence.historical_errors(), [])
        self.assertEqual(self.report(run)['inspected_paths'], pins)
        self.assertEqual(self.report(run)['snapshot_files'], 242)

    def test_preexisting_historical_mismatch_still_fails(self):
        p = self.put('reviews/v4/result.txt', b'preexisting mismatch')
        self.put('curriculum/v5/expansion/historical_receipts.json', json.dumps({'files':{'reviews/v4/result.txt':hashlib.sha256(b'accepted').hexdigest()}}).encode())
        with self.assertRaisesRegex(ValueError, 'Historical receipts changed'):
            with self.guard() as run:
                p.write_bytes(b'new attempted result')
        self.assertEqual(p.read_bytes(), b'preexisting mismatch')
        self.assertFalse(self.report(run)['passed'])
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_binary_multichunk_files_use_bounded_payload_memory(self):
        p = self.root/'reviews/v4/large.bin'
        p.parent.mkdir(parents=True)
        block = bytes(range(256))*4096
        with p.open('wb') as f:
            for _ in range(9):
                f.write(block)
            f.write(b'\x00tail\xff')
        original = evidence._stream_digest(p)
        old_read = Path.read_bytes
        def no_receipt_read_bytes(path):
            if path.is_relative_to(self.reviews) or path.is_relative_to(self.backups):
                raise AssertionError('Whole-file receipt read forbidden')
            return old_read(path)
        tracemalloc.start()
        try:
            with patch.object(Path, 'read_bytes', no_receipt_read_bytes):
                with self.guard() as run:
                    with p.open('wb') as f:
                        for _ in range(7):
                            f.write(block[::-1])
                self.assertEqual(evidence._stream_digest(p), original)
            _, peak = tracemalloc.get_traced_memory()
        finally:
            tracemalloc.stop()
        self.assertLess(peak, 6*evidence._COPY_CHUNK)
        self.assertEqual((run/'legacy-producer-output/v4/large.bin').stat().st_size, 7*len(block))
        self.assertEqual(self.report(run)['copy_buffer_bytes'], evidence._COPY_CHUNK)

    def test_file_directory_replacements_restore_original_file_set(self):
        file = self.put('reviews/v4/becomes-directory', b'original file')
        child = self.put('reviews/v3/becomes-file/original.txt', b'original child')
        with self.guard() as run:
            file.unlink()
            self.put('reviews/v4/becomes-directory/a/b/new.txt', b'nested producer output')
            child.unlink()
            child.parent.rmdir()
            child.parent.write_bytes(b'directory replaced')
        self.assertEqual(file.read_bytes(), b'original file')
        self.assertEqual(child.read_bytes(), b'original child')
        self.assertEqual((run/'legacy-producer-output/v4/becomes-directory/a/b/new.txt').read_bytes(), b'nested producer output')
        self.assertEqual((run/'legacy-producer-output/v3/becomes-file').read_bytes(), b'directory replaced')

    def test_snapshot_failure_never_enters_producer_and_removes_partial_backup(self):
        p = self.put('reviews/v4/receipt.txt', b'original')
        entered = False
        with patch.object(evidence, '_copy_stream', side_effect=OSError('snapshot full')):
            with self.assertRaisesRegex(OSError, 'snapshot full'):
                with self.guard():
                    entered = True
        self.assertFalse(entered)
        self.assertEqual(p.read_bytes(), b'original')
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_index_initialization_failure_cleans_private_temporary_directory(self):
        with patch.object(evidence.sqlite3, 'connect', side_effect=sqlite3.OperationalError('index unavailable')):
            with self.assertRaisesRegex(sqlite3.OperationalError, 'index unavailable'):
                with self.guard():
                    self.fail('producer ran without snapshot index')
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_unreadable_snapshot_subtree_does_not_silently_omit_receipts(self):
        original_walk = evidence.os.walk
        def broken_walk(*args, **kwargs):
            if Path(args[0]) == self.reviews:
                kwargs['onerror'](PermissionError('unreadable receipt directory'))
                return iter(())
            return original_walk(*args, **kwargs)
        with patch.object(evidence.os, 'walk', side_effect=broken_walk):
            with self.assertRaisesRegex(PermissionError, 'unreadable receipt directory'):
                with self.guard():
                    self.fail('producer ran with incomplete snapshot')
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_restore_failure_retains_complete_disk_backup(self):
        p = self.put('reviews/v4/receipt.txt', b'original complete bytes')
        real = evidence._copy_stream
        def fail_restore(source, destination):
            if source.is_relative_to(self.backups) and 'bytes' in source.parts:
                raise OSError('restoration unavailable')
            return real(source,destination)
        with patch.object(evidence, '_copy_stream', side_effect=fail_restore):
            with self.assertRaisesRegex(RuntimeError, 'disk backup retained'):
                with self.guard():
                    p.write_bytes(b'changed')
        directories = list(self.backups.iterdir())
        self.assertEqual(len(directories), 1)
        with closing(sqlite3.connect(directories[0]/'index.sqlite')) as db:
            saved, digest = db.execute('SELECT saved,digest FROM receipts').fetchone()
        self.assertEqual((directories[0]/saved).read_bytes(), b'original complete bytes')
        self.assertEqual(digest, hashlib.sha256(b'original complete bytes').hexdigest())

    def test_destination_and_snapshot_containment(self):
        outside = self.put('outside.txt', b'untouched')
        with self.assertRaisesRegex(ValueError, 'Unsafe batch receipt'):
            with evidence.preserve_receipts(self.root/'outside'):
                self.fail('unsafe producer ran')
        with self.assertRaisesRegex(ValueError, 'snapshot must be outside'):
            with evidence.preserve_receipts(self.destination, snapshot_parent=self.reviews/'backup'):
                self.fail('recursive snapshot ran')
        self.assertEqual(outside.read_bytes(), b'untouched')

    def test_receipt_symlink_is_rejected_without_touching_external_target(self):
        outside = self.put('outside.txt', b'untouched')
        link = self.reviews/'linked.txt'
        try:
            link.symlink_to(outside)
        except OSError as ex:
            self.skipTest('Host does not permit symlink fixture: '+str(ex))
        with self.assertRaisesRegex(ValueError, 'receipt path'):
            with self.guard():
                self.fail('linked producer ran')
        self.assertEqual(outside.read_bytes(), b'untouched')
        self.assertEqual(list(self.backups.iterdir()), [])

    def test_redirected_producer_archive_is_rejected_with_backup_retained(self):
        p = self.put('reviews/v4/receipt.txt', b'original')
        outside = self.root/'outside archive'
        outside.mkdir()
        probe = self.destination/'symlink-probe'
        try:
            probe.symlink_to(outside, target_is_directory=True)
            probe.unlink()
        except OSError as ex:
            self.skipTest('Host does not permit symlink fixture: '+str(ex))
        with self.assertRaisesRegex(RuntimeError, 'disk backup retained'):
            with self.guard() as run:
                p.write_bytes(b'changed')
                (run/'legacy-producer-output').symlink_to(outside, target_is_directory=True)
        self.assertEqual(list(outside.iterdir()), [])
        self.assertEqual(len(list(self.backups.iterdir())), 1)
        # Restoration correctly refused unsafe routing; it did not discard bytes.
        self.assertEqual(p.read_bytes(), b'changed')


if __name__ == '__main__':
    unittest.main(verbosity=2)
