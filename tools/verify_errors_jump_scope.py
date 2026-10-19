"""Independent symbolic resource-set and reverse-lifetime oracle."""
import argparse
import hashlib
import itertools
import json
import subprocess
MAX=4294967295
def expected(fail,cut,code,initial):
    if initial>MAX-3 or cut>7 or (cut==7 and code!=0) or (cut<7 and code not in range(1,9)):
        return [1,initial,0,0,0,0,0,0,0,99,0,0,0,0,0,0,0]+[0]*10
    if fail==1:return [0,initial,1,0,0,5,20,1,0,20,0,0,1,1,0,0,0]+[0]*10
    desired=3 if cut==7 else (cut+1)//2
    inner_failed=fail in range(2,2+desired)
    acquired=fail-2 if inner_failed else desired
    stage=8+acquired if inner_failed else cut
    inner=(5,0) if inner_failed else (0,37) if cut==7 else (code,0)
    attempts=1+(acquired+1 if inner_failed else desired)+1
    last_failed=fail==attempts
    result=(5,21) if last_failed else (0,99) if inner[0]==0 else (7,stage)
    owned={1,*range(2,2+acquired)};trace=[]
    for resource in reversed(range(2,2+acquired)):
        owned.remove(resource);trace.extend([resource,sum(1<<r for r in owned)])
    if not last_failed:trace.extend([5,2])
    trace.extend([1,0]);released=len(trace)//2
    return [0,initial+acquired,attempts,released,0,*result,*inner,stage,0,1,1,1,1,1,released]+trace+[0]*(10-len(trace))
def run(driver):
    cases=list(itertools.product(range(8),range(9),range(10),[0,MAX-3,MAX-2]));rows=[' '.join(map(str,r)) for r in cases];answers=[' '.join(map(str,expected(*r))) for r in cases]
    p=subprocess.run([driver,'oracle'],input='\n'.join(rows)+'\n',capture_output=True,text=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,nested_cases=len(cases),trace_sha256=hashlib.sha256(('\n'.join(rows+answers)+'\n').encode()).hexdigest(),model='symbolic acquired set, reverse release, per-boundary attempt index and separate published effects; no saved-context emulator')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
