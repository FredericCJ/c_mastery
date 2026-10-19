"""Independent integer timeline and policy models, never modular queue code."""
import argparse
import hashlib
import itertools
import json
import subprocess
MAX=(1<<64)-1

def serial(bits,a,b):
    m=1<<bits
    if a>=m or b>=m:return [2,9]
    if abs(a-b)==m//2:return [3,9]
    if a==b:return [0,0]
    forward=(a>b and a-b<m//2) or (a<b and b-a>m//2)
    return [0,1 if forward else -1]

def expected(row):
    kind,bits,start,a,b,elapsed=row
    if kind==0:
        if bits>2 or a>65535 or b>65535:return [2,123456]
        total=a+b
        if bits==0 and total>65535:return [3,123456]
        return [0,total%65536 if bits==1 else min(total,65535)]
    if kind==1:return serial(bits,a,b)
    bound=(1<<bits)//2 if kind==2 else MAX-start+1
    admitted=[d<bound for d in [a,b]]
    moved=elapsed<bound;actual=elapsed if moved else 0
    entries=[(d,i,identity) for i,(d,identity,ok) in enumerate(zip([a,b],[11,22],admitted)) if ok]
    ready=sorted(x for x in entries if x[0]<=actual)
    results=[]
    for sentinel in [99,98]:
        if ready:
            item=ready.pop(0);entries.remove(item);results.extend([0,item[2]])
        else:results.extend([4,sentinel])
    now=start+actual
    if kind==2:now%=1<<bits
    return [0,*[0 if x else 3 for x in admitted],0 if moved else 3,now,*results,len(entries)]

def run(driver):
    rows=[];counts={}
    values=[0,1,2,32767,32768,65534,65535,65536,(1<<32)-1]
    for policy,a,b in itertools.product(range(4),values,values):rows.append((0,policy,0,a,b,0))
    counts['policy_cases']=len(rows)
    for bits in [2,3,4]:
        for a,b in itertools.product(range(1<<bits),repeat=2):rows.append((1,bits,0,a,b,0))
    for a,b in itertools.product(values,repeat=2):rows.append((1,16,0,a,b,0))
    counts['serial_cases']=len(rows)-sum(counts.values())
    for bits in [2,3,4]:
        for start,a,b,elapsed in itertools.product(range(1<<bits),repeat=4):rows.append((2,bits,start,a,b,elapsed))
    for start,a,b,elapsed in itertools.product([0,65534,65535],*[ [0,1,32767,32768,65535] ]*3):rows.append((2,16,start,a,b,elapsed))
    counts['window_cases']=len(rows)-sum(counts.values())
    for bits,start,a,b,elapsed in itertools.product([2,4,16],[0,15,65535,MAX-1,MAX],[0,1,16,65536,MAX],[0,1,16,65536,MAX],[0,1,16,65536,MAX]):rows.append((3,bits,start,a,b,elapsed))
    counts['epoch_cases']=len(rows)-sum(counts.values())
    answers=[' '.join(map(str,expected(r))) for r in rows];encoded=[' '.join(map(str,r)) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(encoded)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    cycles=[]
    for a,b,c in itertools.product(range(16),repeat=3):
        ab,ba=serial(4,a,b),serial(4,b,a)
        assert ab[0]==ba[0] and (ab[0]!=0 or ab[1]==-ba[1])
        if ab==serial(4,b,c)==serial(4,c,a)==[0,1]:cycles.append([a,b,c])
    assert [0,11,6] in cycles
    return dict(passed=True,**counts,cycles=len(cycles),cycle_witness=[0,11,6],trace_sha256=hashlib.sha256(('\n'.join(encoded+answers)+'\n').encode()).hexdigest(),model='Unbounded integer due-time list with stable insertion ties; finite clock extent admitted separately. RFC pair relation is not used to order the oracle queue.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
