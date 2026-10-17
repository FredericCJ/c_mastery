import argparse
from run_expansion_family import run
if __name__ == '__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    raise SystemExit(run('api_depth',args.report))
