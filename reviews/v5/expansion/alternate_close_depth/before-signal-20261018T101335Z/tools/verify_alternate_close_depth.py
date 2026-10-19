"""Independent finite result/ownership table, not reference-code replay."""
import argparse,errno,hashlib,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 pairs=[(rc,error) for rc in [-2,-1,0,1,2] for error in range(-2,259)]
 expected=[]
 for rc,error in pairs:
  if rc==0:expected.append((0,2,0,1,4,99))
  elif rc==-1 and error==errno.EINTR:expected.append((1,1,error,1,0,7))
  else:expected.append((2,3,error,1,4,99))
 proc=subprocess.run([a.driver],input=''.join(f'{r} {e}\n' for r,e in pairs),text=True,capture_output=True,timeout=30)
 actual=[tuple(map(int,line.split())) for line in proc.stdout.splitlines()]
 passed=proc.returncode==0 and actual==expected
 print(json.dumps(dict(passed=passed,cases=len(pairs),eintr=errno.EINTR,trace_sha256=hashlib.sha256(proc.stdout.encode()).hexdigest(),scope='Five result classes times261 errno values; not target OS execution')))
 return int(not passed)
if __name__=='__main__':raise SystemExit(main())
