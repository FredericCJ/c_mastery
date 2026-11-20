"""Exercise the complete backup/trial/restore transaction in memory."""
import contextlib
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import Mock, patch

import f439_board as fb


class BoardFixture:
    serial = "0669FF485753667187124334"
    dry_run = False
    identity = dict(uid=[1, 2, 3], idcode=0x1234, flash_kib=2048,
                    optcr=0x810, optcr1=0)

    def __init__(self, root):
        self.root = root
        self.recovery_dir = root / "recovery"
        self.original = struct.pack("<II", 0x20001000, 0x08000021) + bytes(
            i % 251 for i in range(fb.SECTOR_SIZE - 8))
        self.sector = self.original
        course = self.original[:120] + bytes(40) + self.original[160:]
        self.course = fb.sector_envelope(course)
        self.digest = hashlib.sha256(self.course).hexdigest()
        self.manifest = dict(image=fb.TRIAL_IMAGE, elf_sha256=self.digest,
                             sources={"lesson.c": "test"})
        (root / "image.elf").write_bytes(self.course)
        (root / "manifest.json").write_text(json.dumps(self.manifest))
        self.writes = []
        self.session_count = 0
        self.alter_read = None
        self.fail_rollback = False

    @contextlib.contextmanager
    def session(self, action):
        self.session_count += 1
        yield Mock(dir=self.root / str(self.session_count), details={})

    def rebuilder(self, name):
        return self.manifest

    def _session_copy(self, session, path, manifest):
        return path

    def _identity(self, session, extra=(), **kwargs):
        if extra:
            path = Path(extra[0].split("{")[1].split("}")[0])
            raw = self.sector
            if path.stem == self.alter_read:
                raw = raw[:120] + bytes([raw[120] ^ 1]) + raw[121:]
            path.write_bytes(raw)
        return dict(self.identity), {}

    def _check_baseline(self, identity, session):
        pass

    def optcr1_baseline(self):
        return {"optcr1": 0}

    def _program(self, session, elf):
        raw = fb.parse_elf(elf.read_bytes()).read_flash(fb.SECTOR_BASE, fb.SECTOR_SIZE)
        if raw == self.original and self.fail_rollback:
            raise fb.PolicyError("injected failure restoring")
        self.writes.append(raw)
        self.sector = raw


class RestoreTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)
        self.board = BoardFixture(self.root)
        for target, value in [("TRIAL_SHA256", self.board.digest),
                              ("serial_device_for", lambda _: Path("/dev/ttyACM99")),
                              ("tty_holders", lambda _: []),
                              ("observe_trial", Mock(return_value={"ok": True}))]:
            patcher = patch.object(fb, target, value)
            patcher.start()
            self.addCleanup(patcher.stop)

    def trial(self, resume=False):
        return fb.board_roundtrip(self.board, self.root, resume=resume)

    def test_complete_sector_recovery(self):
        out = self.trial()
        self.assertEqual(len(self.board.writes), 2)
        self.assertEqual(self.board.sector, self.board.original)
        self.assertEqual(out["restoration"]["restored_bytes"], fb.SECTOR_SIZE)
        backup = Path(out["backup_dir"])
        self.assertEqual((backup / "original.bin").read_bytes(), self.board.original)
        self.assertEqual(backup.stat().st_mode & 0o777, 0o700)
        self.assertFalse((self.board.recovery_dir / "pending.json").exists())

    def test_inconsistent_reads_never_program(self):
        self.board.alter_read = "original-second"
        with self.assertRaisesRegex(fb.PolicyError, "reads differ"):
            self.trial()
        self.assertEqual(self.board.writes, [])

    def test_different_trial_digest_never_programs(self):
        (self.root / "image.elf").write_bytes(self.board.course[:-1])
        with self.assertRaisesRegex(fb.PolicyError, "reviewed report"):
            self.trial()
        self.assertEqual(self.board.writes, [])

    def test_second_sector_even_one_byte_refused(self):
        data = bytearray(self.board.course + b"x")
        struct.pack_into("<II", data, 52 + 16, fb.SECTOR_SIZE + 1, fb.SECTOR_SIZE + 1)
        self.assertTrue(fb.check_elf(data)["ok"])
        with self.assertRaisesRegex(fb.PolicyError, "sector 0 only"):
            fb.sector_zero_check(data)

    def test_interruption_restores(self):
        fb.observe_trial.side_effect = KeyboardInterrupt
        with self.assertRaises(KeyboardInterrupt):
            self.trial()
        self.assertEqual(self.board.sector, self.board.original)
        self.assertFalse((self.board.recovery_dir / "pending.json").exists())

    def test_failed_verification_requires_retry(self):
        self.board.alter_read = "after-restore"
        with self.assertRaisesRegex(fb.PolicyError, "after restoration"):
            self.trial()
        self.board.alter_read = None
        self.assertFalse(self.trial(resume=True)["restoration"]["reprogrammed"])
        self.assertEqual(self.board.sector, self.board.original)

    def test_recovery_from_interruption_binds_target_and_payload(self):
        self.board.fail_rollback = True
        with self.assertRaisesRegex(fb.PolicyError, "injected failure"):
            self.trial()
        self.board.fail_rollback = False
        self.board.identity = dict(self.board.identity, uid=[1, 2, 0])
        with self.assertRaisesRegex(fb.PolicyError, "identity"):
            self.trial(resume=True)
        self.board.identity = BoardFixture.identity
        pending = self.board.recovery_dir / "pending.json"
        record = json.loads(pending.read_text())
        path = self.board.recovery_dir / record["transaction"] / "original.bin"
        path.write_bytes(bytes(fb.SECTOR_SIZE))
        with self.assertRaisesRegex(fb.PolicyError, "disagree"):
            self.trial(resume=True)
        self.assertEqual(len(self.board.writes), 1)
        path.write_bytes(self.board.original)
        self.trial(resume=True)
        self.assertEqual(self.board.sector, self.board.original)
        self.assertFalse(pending.exists())

    def test_pending_journal_blocks_other_sessions(self):
        board = fb.Board(state_dir=self.root / "state", recovery_dir=self.root,
                         lock_path=self.root / "local.lock",
                         host_lock=lambda: self.root / "host.lock",
                         verify_tools=lambda: [], process_scan=lambda: [])
        (self.root / "pending.json").write_text("{}")
        for action in ("flash", "recover", "roundtrip"):
            with self.assertRaisesRegex(fb.PolicyError, "restoration pending"):
                with board.session(action):
                    self.fail("unsafe session started")


if __name__ == "__main__":
    unittest.main()
