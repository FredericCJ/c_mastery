"""Independent selected evaluation-count and control-flow expectation."""
import argparse,hashlib,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
trace=[(0,0,1,0,''),(1,0,1,0,'3'),(1,1,1,0,'3:7'),(0,0,1,0,'literal'),(1,0,1,0,'3'),(0,0,0,0,'')]
minimum=[(0,0,0,4,''),(2,1,0,3,''),(1,1,0,3,''),(1,1,0,3,'')]
statement=[(0,0,1,3,''),(0,0,0,7,''),(0,0,3,0,''),(0,0,3,0,''),(0,0,0,9,''),(0,0,2,2,'')]
expected=[' '.join(map(str,(j,i,*row))) for j,group in enumerate([trace,minimum,statement]) for i,row in enumerate(group)]
r=subprocess.run([a.driver,'oracle'],text=True,capture_output=True);assert r.returncode==0 and r.stdout.splitlines()==expected,(r.stdout,expected)
print(json.dumps(dict(passed=True,cases=16,trace_sha256=hashlib.sha256(('\n'.join(expected)+'\n').encode()).hexdigest())))
