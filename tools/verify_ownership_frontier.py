"""Independent dependency-set and event-sequence model, not C cleanup reuse."""
import argparse
import hashlib
import itertools
import json
import struct
import subprocess
LIMIT=((1<<(8*struct.calcsize('P')))-1)//4
DEPS=[set(),{0},{0},{1}]
def expected(row):
    order=row[:4];ft,fi=row[4:6];length=row[6:];seen=set()
    for i in order:
        if i not in range(4) or i in seen or not DEPS[i]<=seen or not 0<length[i]<=LIMIT:return [1,0,0,0,0,0,0,0,0]
        seen.add(i)
    acquired=[];initialized=set();events=[];attempts=0;code=0
    for i in order:
        events.append(10+i);attempts+=1
        if attempts==ft or length[i]>16:code=2;break
        acquired.append(i);events.append(20+i)
        if fi==i+1:code=3;break
        initialized.add(i)
    count,mask,init=(4,15,15) if code==0 else (0,0,0)
    for i in reversed(acquired):
        if i in initialized:events.append(30+i)
        events.append(40+i)
    return [code,count,mask,init,0,attempts,len(acquired),0,len(events),*events]
def run(driver):
    rows=[(*order,ft,fi,1,2,3,4) for order in itertools.permutations(range(4)) for ft,fi in itertools.product(range(5),repeat=2)]
    valid=[p for p in itertools.permutations(range(4)) if expected((*p,0,0,1,2,3,4))[0]==0]
    for p,ft,fi,lengths in itertools.product(valid,range(5),range(5),[(16,16,16,16),(1,1,17,1),(1,0,1,1),(1,1,1,LIMIT+1)]):rows.append((*p,ft,fi,*lengths))
    rows += [(0,1,1,3,0,0,1,1,1,1),(0,1,2,4,0,0,1,1,1,1)]
    answers=[' '.join(map(str,expected(r))) for r in rows];inputs=[' '.join(map(str,r)) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,cases=len(rows),topological_orders=len(valid),permutations=24,allocation_failure_positions=4,initialization_failure_nodes=4,size_bits=8*struct.calcsize('P'),trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),model='Dependency sets, initialized set and reversed acquired list; exact take/init/destroy/release events, with typed pool capacity16 separately admitted.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
