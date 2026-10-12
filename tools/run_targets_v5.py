"""Run the retained target/comparison campaigns for the fifth edition, publishing receipts under reviews/v5."""
import argparse
from pathlib import Path
import run_targets_v4
from common import ROOT  # noqa: F401  (patched by the sandboxed release guards)
from run_targets_v4 import OUTPUTS, TARGET_RECEIPTS, historical_files  # noqa: F401  (re-exported)
from v5_common import RECEIPTS


def run(freertos_root=None, native=False, receipts=RECEIPTS):
    return run_targets_v4.run(freertos_root, native, receipts)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--freertos-root', type=Path)
    args = parser.parse_args()
    return run(args.freertos_root, args.native)


if __name__ == '__main__':
    raise SystemExit(main())
