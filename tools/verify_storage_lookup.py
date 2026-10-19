"""Independent unsigned byte ordering and exact ASCII normalization oracle."""
import argparse,hashlib,itertools,json,subprocess
def check(driver):
 inputs=[];expected=[]
 for n in [0,1,2]:
  pairs=itertools.product(range(256),repeat=2) if n==2 else ((a,0) for a in range(256)) if n==1 else [(0,0)]
  for a,b in pairs:
   inputs.append(f'{a} {b} {n}');data=bytes([a,b])[:n]
   if n and all(32<=x<=126 for x in data):
    normalized=bytes(x+32 if 65<=x<=90 else x for x in data)+bytes(16-n);expected.append(f'0 {n} {normalized[0]} {normalized[1]} {normalized[15]}')
   else:expected.append('2 777 165 165 165')
 p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,check=True);rows=p.stdout.splitlines();assert rows==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(rows,expected)) if a!=b),('length',len(rows),len(expected)))
 records={b'\x00':0,b'\x00\x00':1}
 for i in range(90):records[bytes([[0,1,128,255][i//23],0,i%23])]=i+2
 want=[f'{len(k)} {k.hex()} {records[k]}' for k in sorted(records)]
 q=subprocess.run([driver,'records'],text=True,capture_output=True,check=True);assert q.stdout.splitlines()==want
 return dict(passed=True,normalization_cases=len(inputs),binary_records=len(records),trace_sha256=hashlib.sha256((p.stdout+q.stdout).encode()).hexdigest(),limits='All one/two-octet inputs under exact printable-ASCII contract;92 independent unsigned byte records. Not Unicode normalization or arbitrary input lengths.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
