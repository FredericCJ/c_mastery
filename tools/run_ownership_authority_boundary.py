"""Run the declared six-profile C17 authority campaign."""
import argparse
from run_ownership_module import run
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report');a=p.parse_args();raise SystemExit(0 if run('authority_boundary',a.report) else 1)
