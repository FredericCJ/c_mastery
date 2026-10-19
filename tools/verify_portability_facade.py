"""Independent finite typed operation arithmetic,without importing C headers."""
import argparse,hashlib,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();r=subprocess.run([a.driver,'oracle'],capture_output=True,text=True);assert r.returncode==0,r.stderr
 expected=[[a,b,100*a+b,100*a+b] for a in range(100) for b in range(100)];actual=[list(map(int,s.split())) for s in r.stdout.splitlines()];assert actual==expected
 print(json.dumps(dict(passed=True,cases=len(expected),trace_sha256=hashlib.sha256(json.dumps(expected,separators=(',',':')).encode()).hexdigest())))
if __name__=='__main__':main()
