"""Run the declared clocks successor campaign."""
import argparse
from run_replicated_module import run
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report');a=p.parse_args();raise SystemExit(0 if run('clocks',a.report) else 1)
