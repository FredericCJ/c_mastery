"""Execute the complete original and successor T65 test contract."""
import argparse
from run_errors_current import run
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True)
 raise SystemExit(run(p.parse_args().report))
