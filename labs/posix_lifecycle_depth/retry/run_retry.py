"""Check explicit learner receipt/store imports using an owned finite device model."""
import argparse,subprocess,tempfile
from pathlib import Path
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('receipt-source','receipt-include','store-source','store-include'):p.add_argument('--'+name,required=True,type=Path)
 p.add_argument('--compiler',default='cc');a=p.parse_args();here=Path(__file__).resolve().parent
 for path in (a.receipt_source,a.store_source,a.receipt_include/'receipt.h',a.store_include/'store.h'):
  if not path.is_file():p.error('Missing explicit learner input: '+str(path))
 with tempfile.TemporaryDirectory(prefix='request-retry-') as directory:
  exe=Path(directory)/'retry';subprocess.run([a.compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I'+str(a.receipt_include.resolve()),'-I'+str(a.store_include.resolve()),str(here/'tests.c'),str(a.receipt_source.resolve()),str(a.store_source.resolve()),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True,timeout=30)
 return 0
if __name__=='__main__':raise SystemExit(main())
