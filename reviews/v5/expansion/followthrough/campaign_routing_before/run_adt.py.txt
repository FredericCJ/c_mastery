"""Tracked hosted producer for the adt family."""
import argparse
from run_expansion_family import run
if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    raise SystemExit(run('adt',args.report))
