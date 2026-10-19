"""Integer and byte-extent oracles for diagnostics versus enforced admissibility."""
import argparse
import hashlib
import itertools
import json
import subprocess
MAX=(1<<64)-1
def expected(row):
    kind,mode,n,index,counter,amount,reports=row
    if kind==0:
        if mode>2 or not 0<=counter<=100 or not -100<=amount<=100:return [2,counter,reports]
        candidate=counter+amount;bad=not 0<=candidate<=100
        if bad and mode:
            if reports==MAX:return [3,counter,reports]
            reports+=1
            if mode==2:return [7,counter,reports]
        return [0,max(0,min(100,candidate)),reports]
    result=0;data=[0]*8
    if mode>2:result=2
    elif index>=n:
        result=7
        if mode:
            if reports==MAX:result=3
            else:reports+=1
    else:data[index]=17
    return [result,reports,*data]
def run(driver):
    rows=[(0,m,0,0,c,a,r) for c,a,m,r in itertools.product(range(101),range(-100,101),range(3),[0,MAX-1,MAX])];ordinary=len(rows)
    rows += [(0,m,0,0,c,a,r) for c,a,m,r in itertools.product([-1,0,100,101],[-101,-100,0,100,101],range(4),[0,MAX])];counter=len(rows)
    rows += [(1,m,n,i,0,0,r) for n,i,m,r in itertools.product(range(9),range(11),range(4),[0,MAX-1,MAX])];buffers=len(rows)-counter
    encoded=[' '.join(map(str,r)) for r in rows];answers=[' '.join(map(str,expected(r))) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(encoded)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,ordinary_cases=ordinary,counter_cases=counter,buffer_cases=buffers,size_t_bits=64,trace_sha256=hashlib.sha256(('\n'.join(encoded+answers)+'\n').encode()).hexdigest(),model='Unbounded signed candidate with separately admitted report increment; independent byte-array write only inside advertised extent. Native64bit size_t profile only.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
