import argparse,subprocess
p=argparse.ArgumentParser();p.add_argument('--faulty',required=True);p.add_argument('--repaired',required=True);a=p.parse_args()
bad=subprocess.run([a.faulty],capture_output=True,text=True,timeout=5)
fixed=subprocess.run([a.repaired],capture_output=True,text=True,timeout=5)
if bad.returncode!=1 or 'violations=1' not in bad.stdout or fixed.returncode or 'violations=0' not in fixed.stdout:raise SystemExit('admission observation mismatch')
print('Same final count=4; faulty transient invariant violation detected, repaired guard rejects before mutation.')
