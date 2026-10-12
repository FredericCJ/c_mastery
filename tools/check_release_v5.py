"""Fifth-edition release guards: the v4 guard suite bound to reviews/v5 and the v5 entry points.

The sandboxed cases exercise accept_v5.pinned_receipt, package_v5 promotion and
the v5 campaign wrappers; no real receipt, archive or historical file changes.
"""
import unittest

import accept_v4
import accept_v5
import package_v4
import package_v5
import run_legacy_matrix_v4
import run_legacy_matrix_v5
import run_targets_v4
import run_targets_v5
from check_release_v4 import ReleaseGuards


class FifthEditionGuards(ReleaseGuards):
    RECEIPTS = 'reviews/v5'
    MODULES = (accept_v4, package_v4, run_targets_v4, run_legacy_matrix_v4,
               accept_v5, package_v5, run_targets_v5, run_legacy_matrix_v5)
    ACCEPT = accept_v5
    PACKAGE = package_v5
    TARGETS = run_targets_v5
    LEGACY = run_legacy_matrix_v5

    def test_fifth_edition_archives_are_named_by_the_manifest(self):
        self.assertEqual(len(package_v5.ARCHIVES), 4)
        self.assertTrue(all(name.startswith('c_mastery_v5_') for name in package_v5.ARCHIVES))


def main():
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(FifthEditionGuards))
    if not result.wasSuccessful():
        raise SystemExit(1)
    print(f"{result.testsRun} isolated fifth-edition release guards passed; {len(result.skipped)} platform skips.")


if __name__ == '__main__':
    main()
