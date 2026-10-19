"""Independent occupancy/identity transition model, not a C implementation clone."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
MAX=(1<<64)-1
def expected(word):
    gen=MAX-1 if word[0]=='x' else 1
    slots=[[gen,0,0] for _ in range(4)];handles=[[9,1],[9,1]]
    closed=moved=False;calls=1;frees=0;live=1;rows=[]
    for pos,op in enumerate(word[1:],1):
        status=1;out=999
        if op=='b':handles[1]=handles[0][:];status=0
        elif op=='8':
            status=6 if moved else 4 if closed else 0
            if status==0:moved=True
        elif op=='9':
            status=0
            if not closed:frees+=1;live=0;closed=True
        elif closed:status=4
        elif op in '01':
            free=[i for i,s in enumerate(slots) if s[1]==0]
            status=0 if free else 2
            if free:
                i=free[0];slots[i][1:]=[1,10+pos];handles[int(op)]=[i,slots[i][0]]
        elif op in '2345a':
            h=handles[int(op)%2] if op!='a' else handles[0]
            valid=op!='a' and h[0]<4 and slots[h[0]][1]==1 and slots[h[0]][0]==h[1]
            status=0 if valid else 3
            if valid:
                s=slots[h[0]]
                if op in '23':out=s[2]
                else:s[2]=0;s[1]=2 if s[0]==MAX else 0;s[0]+=int(s[0]!=MAX)
        elif op in '67':
            calls+=1;status=5 if op=='7' else 0
            if op=='6':frees+=1
        row=[status,out,int(closed),calls,frees,live,*handles[0],*handles[1]]
        if not closed:row.extend(x for s in slots for x in s)
        rows.append(' '.join(map(str,row))+';')
    return '\n'.join(rows)+'\n.'
def base_expected(word):
    slots=[[1,0,0,0] for _ in range(4)];handles=[[9,1],[9,1]];rows=[]
    for pos,op in enumerate(word):
        status=0;out=999
        if op=='6':handles[1]=handles[0][:];status=1
        elif op in '01':
            free=[i for i,s in enumerate(slots) if not s[1] and not s[2]]
            if free:
                i=free[0];slots[i][1]=1;slots[i][3]=10+pos;handles[int(op)]=[i,slots[i][0]];status=1
        else:
            h=handles[int(op)%2]
            if h[0]<4 and slots[h[0]][1] and slots[h[0]][0]==h[1]:
                status=1;s=slots[h[0]]
                if op in '23':out=s[3]
                else:s[1]=0;s[0]+=1
        rows.append(' '.join(map(str,[status,out,*handles[0],*handles[1],*(x for s in slots for x in s)]))+';')
    return '\n'.join(rows)+'\n.'
def verify(driver):
    rng=random.Random(6705)
    words=[m+''.join(w) for m in 'nx' for w in itertools.product('0123456789ab',repeat=3)]
    words += [rng.choice('nx')+''.join(rng.choice('0123456789ab') for _ in range(40)) for _ in range(512)]
    base=[''.join(w) for w in itertools.product('0123456',repeat=4)]
    base += [''.join(rng.choice('0123456') for _ in range(40)) for _ in range(256)]
    hashes=[]
    for mode,seq,model in [('oracle',words,expected),('baseoracle',base,base_expected)]:
        p=subprocess.run([driver,mode],input='\n'.join(seq)+'\n',text=True,capture_output=True,timeout=120)
        assert p.returncode==0,p.stderr
        want='\n'.join(model(w) for w in seq)+'\n'
        if p.stdout!=want:
            a=p.stdout.splitlines();b=want.splitlines();raise AssertionError(next(((i,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y),(len(a),len(b))))
        hashes.append(hashlib.sha256(want.encode()).hexdigest())
    return dict(passed=True,successor_histories=len(words),successor_transitions=sum(len(w)-1 for w in words),baseline_histories=len(base),baseline_transitions=sum(map(len,base)),trace_sha256=hashes,limits='Finite sequential transition enumeration complements actual pthread executions; neither establishes all concurrent histories.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
