"""Independent ownership ledger for two actual resource acquisitions."""
import argparse,json,hashlib,subprocess,itertools
p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();rows=[];expected=[]
for fail,tr in itertools.product(range(4),range(4)):
 rows.append(f'{fail} {tr}')
 if tr>2:r=[1,0,0,0,0,0,0,0]
 elif fail==1:r=[2,1,0,0,0,0,0,0]
 elif fail==2:r=[2,2,1,1,0,1,1,0]
 else:
  before=[x for x in [2,1] if x!=tr];after=before+([tr] if tr else []);number=lambda xs:sum(v*10**i for i,v in enumerate(reversed(xs)));r=[0,2,len(before),number(before),tr,2,number(after),0]
 expected.append(' '.join(map(str,r)))
out=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True,check=True).stdout.splitlines();assert out==expected,(out,expected)
trace=hashlib.sha256(('\n'.join(rows)+'\n'+'\n'.join(expected)+'\n').encode()).hexdigest();print(json.dumps(dict(passed=True,cases=len(rows),trace_sha256=trace)))
