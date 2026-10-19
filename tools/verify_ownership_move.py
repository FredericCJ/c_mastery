"""Independent finite token authority and width-boundary model."""
import argparse
import hashlib
import itertools
import json
import subprocess
MAX=(1<<32)-1
def expected(row):
    n,seed,moves=row
    valid=1<=n<=8;opened=valid and seed<MAX;generation=seed+1 if opened else 0;original=generation
    result=[0 if opened else 1 if not valid else 2,generation]
    for _ in range(moves):
        if not opened:result.append(3)
        elif generation==MAX:result.append(7)
        else:generation+=1;result.append(0)
    same=opened and original==generation
    return result+[0 if same else 3,1 if same else 91,generation,int(opened and n>4),int(opened and n>4)]
def run(driver):
    rows=list(itertools.product(range(10),[0,1,2,MAX-3,MAX-2,MAX-1,MAX],range(9)));answers=[' '.join(map(str,expected(r))) for r in rows];inputs=[' '.join(map(str,r)) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,cases=len(rows),lengths=10,generation_seeds=7,move_depths=9,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),model='Integer generation transition and captured old token comparison; no private C representation reused. Pool ownership counts independently derive from inline threshold4.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
