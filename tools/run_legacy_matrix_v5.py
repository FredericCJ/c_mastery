"""Revalidate the retained families for the fifth edition; historical v3 receipts stay byte-identical."""
import run_legacy_matrix_v4
from common import ROOT  # noqa: F401  (patched by the sandboxed release guards)
from v5_common import RECEIPTS, manifest

FAMILIES = list(manifest()['retained_families'])
RECEIPT = RECEIPTS + '/legacy_matrix.json'


def main(families=None, receipt=RECEIPT):
    run_legacy_matrix_v4.main(list(families or FAMILIES), receipt)


if __name__ == '__main__':
    main()
