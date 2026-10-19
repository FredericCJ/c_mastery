"""Run the higher_order source-bound hosted mechanisms and mutation controls."""
import argparse
from run_expansion_family import run

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(run('higher_order', parser.parse_args().report))
