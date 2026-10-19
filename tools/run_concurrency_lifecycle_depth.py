"""Source-pinned copied-queue, reclamation protocol and actual pthread campaign."""
import argparse
from run_expansion_family import run

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    raise SystemExit(run('concurrency_lifecycle_depth', args.report))
