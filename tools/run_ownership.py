import argparse
from run_ownership_current import run
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True)
 raise SystemExit(run(p.parse_args().report))
