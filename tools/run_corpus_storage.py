import argparse
from run_expansion_family import run
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args()
 raise SystemExit(run('corpus_storage',a.report))
