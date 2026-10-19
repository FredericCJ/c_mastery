import argparse
from run_adt_module import run
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report');a=p.parse_args();raise SystemExit(0 if run('owning_sum',a.report) else 1)
