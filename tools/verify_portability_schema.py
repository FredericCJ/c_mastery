"""Independent arithmetic wire oracle, without importing either generator."""
import argparse,hashlib,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();r=subprocess.run([a.driver,'oracle'],capture_output=True,text=True);assert r.returncode==0,r.stderr
 expected=[]
 for tag in range(8):
  for length in range(32):
   for flags in range(4):expected.append([tag,length,flags,tag+8*length+256*flags,tag,length,flags])
 actual=[list(map(int,line.split())) for line in r.stdout.splitlines()];assert actual==expected
 print(json.dumps(dict(passed=True,cases=len(expected),trace_sha256=hashlib.sha256(json.dumps(expected,separators=(',',':')).encode()).hexdigest())))
if __name__=='__main__':main()
