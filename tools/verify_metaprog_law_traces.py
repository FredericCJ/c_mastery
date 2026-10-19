"""Independent immutable-list model for owners, first divergence and trace reduction."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
WORDS=[b'',b'A',b'a',b' A ',b'B']
def canonical(b,mode):
    if mode>1:return 1,None
    if len(b)>8:return 3,None
    if mode and any(x<32 or x>126 for x in b):return 3,None
    return 0,b' '.join(x for x in b.lower().split(b' ') if x) if mode else b
def fnv(b):
    h=14695981039346656037
    for c in bytes([len(b)])+b:h=((h^c)*1099511628211)&((1<<64)-1)
    return h
def admitted(steps,fail):
    if len(steps)>12 or fail>12:return False
    live=[False,False];allocations=0
    for op,slot,value in steps:
        if op>2 or slot>1 or value>4 or op==1 and value or op==2 and (slot or value):return False
        if op==0:
            allocations+=1
            if allocations!=fail:live[slot]=True
        elif op==1:
            if not live[slot]:return False
            live[slot]=False
        elif not all(live):return False
    return True
def replay(steps,fail,variant):
    if not admitted(steps,fail):return [1,0,0,0,0,0,0]
    owners=[None,None];calls=refused=freed=0;failure=0;first=len(steps)
    for i,(op,slot,value) in enumerate(steps):
        if op==0:
            calls+=1
            if calls==fail:refused+=1
            else:
                if owners[slot] is not None:freed+=1
                owners[slot]=WORDS[value]
        elif op==1:owners[slot]=None;freed+=1
        else:
            a,b=owners
            if variant==2 and (not a or not b):failure=3
            else:
                equal=True if variant==3 else canonical(a,1)[1]==canonical(b,1)[1]
                def hashed(x):return int(x in [b'A',b'B']) if variant==3 else fnv(canonical(x,0 if variant else 1)[1])
                if equal and hashed(a)!=hashed(b):failure=1
            if failure:first=i;break
    freed+=sum(x is not None for x in owners)
    return [0,failure,first,calls,refused,freed,0]
def shrink(steps,fail,variant):
    result=replay(steps,fail,variant)
    if result[0]:return [1,99,0,0]
    if not result[1]:return [4,99,0,0]
    wanted=result[1];current=list(steps);attempts=0
    while True:
        candidates=[(current[:i]+current[i+1:],fail) for i in range(len(current))]
        for i,(op,slot,val) in enumerate(current):
            if op==0 and val:candidates.append((current[:i]+[(op,slot,val-1)]+current[i+1:],fail))
        if fail>1:candidates.append((current,fail-1))
        for candidate,ordinal in candidates:
            attempts+=1;r=replay(candidate,ordinal,variant)
            if r[0]==0 and r[1]==wanted:current,fail=list(candidate),ordinal;break
        else:break
    return [0,attempts,len(current),fail]+[v for s in current for v in s]
def run(driver):
    lines=[];expected=[];kinds=[]
    def add(line,out,kind):lines.append(line);expected.append(out);kinds.append(kind)
    words=WORDS+[b'a b',b'A  B',b'\0',b'a\0b',b'\xff',b'12345678',b'123456789',b'       ']+[bytes([x]) for x in range(256)]
    for b,mode,cache in itertools.product(words,[0,1,2],[0,99]):
        status,value=canonical(b,mode);add(f'H {mode} {cache} '+(b.hex() or '-'),[status,99 if status else fnv(value)],'hash')
    ops=[(0,0,1),(0,1,2),(1,0,0),(1,1,0),(2,0,0),(0,0,3),(0,1,4)]
    traces=[list(s) for n in range(4) for s in itertools.product(ops,repeat=n)]
    rnd=random.Random(68007)
    traces+=[[rnd.choice(ops) for _ in range(rnd.randrange(4,13))] for _ in range(1000)]
    shrunk=0
    for index,steps in enumerate(traces):
        for fail,variant in itertools.product([0,1,2,4],[0,1,2,3]):
            prefix=f'T {len(steps)} {fail} {variant} ';tail=' '.join(str(v) for s in steps for v in s);result=replay(steps,fail,variant);add(prefix+'0 '+tail,result,'replay')
            if result[0]==0 and result[1] and (index%3==0 or len(steps)>3):add(prefix+'1 '+tail,shrink(steps,fail,variant),'shrink');shrunk+=1
    for steps,fail,variant in [([(0,0,4),(0,1,4),(1,1,0),(0,0,3),(0,1,2),(2,0,0)],0,2), ([(0,0,4),(0,1,2),(2,0,0)],0,3), ([(2,0,0)],0,2),([],0,0)]:
        add(f'T {len(steps)} {fail} {variant} 1 '+' '.join(str(v) for s in steps for v in s),shrink(steps,fail,variant),'shrink');shrunk+=1
    p=subprocess.run([str(driver),'oracle'],input='\n'.join(lines)+'\n',text=True,capture_output=True,check=True);actual=p.stdout.splitlines();assert len(actual)==len(expected)
    for line,out,got in zip(lines,expected,actual):assert list(map(int,got.split()))==out,(line,out,got)
    return dict(passed=True,cases=len(lines),hash_cases=kinds.count('hash'),replay_cases=kinds.count('replay'),shrink_cases=shrunk,admitted_replays=sum(k=='replay' and e[0]==0 for k,e in zip(kinds,expected)),failure_witnesses=sum(k=='replay' and e[0]==0 and e[1]!=0 for k,e in zip(kinds,expected)),trace_sha256=hashlib.sha256(json.dumps(list(zip(lines,expected)),separators=(',',':')).encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
