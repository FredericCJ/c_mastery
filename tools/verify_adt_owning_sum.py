"""Independent value/owner-count transition model for fallible sum operations."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess

def expected(word):
    values=[(0,0),(0,0)];calls=drops=0;rows=[]
    def remove(i):
        nonlocal drops
        if values[i][0]==3:drops+=1
        values[i]=(0,0)
    for op in word:
        status=0
        if op in '01':remove(0);values[0]=(1,0) if op=='0' else (2,4294967300)
        elif op in '23':calls+=1;remove(0);values[0]=(3,3 if op=='2' else 0)
        elif op in '489':
            source,destination=(1,0) if op=='9' else (0,1)
            if values[source][0]==3:calls+=1
            if op=='8' and values[source][0]==3:status=2
            else:remove(destination);values[destination]=values[source]
        elif op=='5':remove(1);values[1]=values[0];values[0]=(0,0)
        else:remove(0 if op=='6' else 1)
        live=sum(v[0]==3 for v in values)
        rows.append(' '.join(map(str,(status,*values[0],*values[1],calls,drops,live)))+';')
    return ''.join(rows)

def verify(driver):
    words=[''.join(x) for x in itertools.product('0123456789',repeat=4)]
    rng=random.Random(6701);words += [''.join(rng.choice('0123456789') for _ in range(32)) for _ in range(512)]
    p=subprocess.run([driver,'oracle'],input='\n'.join(words)+'\n',text=True,capture_output=True,timeout=90)
    assert p.returncode==0,p.stderr
    got=p.stdout.splitlines();want=[expected(w) for w in words]
    assert got==want,next(((i,words[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    return dict(passed=True,histories=len(words),transitions=sum(map(len,words)),trace_sha256=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest(),limits='Finite serialized ownership/clone-failure traces; no forged-pointer or concurrent lifetime inference.')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);args=p.parse_args();print(json.dumps(verify(args.driver)))
