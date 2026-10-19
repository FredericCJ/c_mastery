"""Run source-bound streaming hash and HMAC author evidence."""
import argparse
from run_replicated_module import run
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report');a=p.parse_args();raise SystemExit(0 if run('hashing',a.report) else 1)
