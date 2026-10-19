"""Independent byte grammar and list-based ownership transition model."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
MAX=2**32-1
def packet(wire):
    if len(wire)<3:return 8
    if wire[0] not in [1,2]:return 9
    if int.from_bytes(wire[1:3],'big')!=len(wire)-3:return 10
    return 11 if wire[0]==2 and len(wire)>3 else 0
def model(row):
    origin,generation,cancel,requested,wire=row
    feed=2 if len(wire)>32 else 0;stored=b'' if feed else wire
    acquired=7 if requested>len(stored) else packet(stored[:requested]);kind=99;length=999;first=second=retired=0;valid=release=stale=1;head=origin;count=len(stored);nxt=generation;copy=b'\xa5'*32
    if acquired==0:
        kind=stored[0];payload=stored[3:requested];length=len(payload);first=min(length,32-(origin+3)%32);second=length-first;copy=payload+copy[length:]
        valid=5 if cancel else 0;release=0;stale=4;head=(origin+requested)%32;count=32-requested;retired=int(generation==MAX);nxt=generation if retired else generation+1
    return f'{feed} {acquired} {kind} {length} {first} {second} {valid} {release} {stale} {head} {count} {nxt} {retired} {packet(wire)} 1 {copy.hex()}'
def history(ops):
    head=30;count=0;active=cancel=closed=retired=0;nxt=MAX-1;generation=0;current=old=None;results=[]
    def valid(t):return 4 if not active or t!=generation else 5 if cancel else 0
    for op in ops:
        if op=='0':
            s=5 if closed else 2 if count+3>32 else 0
            if s==0:count+=3
        elif op=='1':
            s=5 if closed else 3 if active else 6 if retired else 7 if count<3 else 0
            if s==0:active=1;cancel=0;generation=nxt;current=generation;retired=int(nxt==MAX);nxt+=int(not retired)
        elif op=='2':
            s=valid(current)
            if s in [0,5]:cancel=1;s=0
        elif op in ['3','5']:
            s=valid(current if op=='3' else old)
            if s in [0,5]:head=(head+3)%32;count-=3;active=cancel=0;s=0
        elif op=='4':old=current;s=valid(old)
        elif op=='6':
            s=3 if active else 7 if count<3 else 0
            if s==0:head=(head+3)%32;count-=3
        else:closed=1;cancel=1 if active else cancel;s=0
        results.append(f'{s},{head},{count},{active},{cancel},{closed},{retired},{nxt},{generation}')
    return ';'.join(results)
def execute(driver,arg,inputs,want):
    p=subprocess.run([driver,arg],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;got=p.stdout.splitlines()
    assert got==want,next(((i,a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    return hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest()
def verify(driver):
    rows=[];rng=random.Random(6710)
    for origin in range(32):
        for length in range(30):
            for kind in [1,2,3]:
                for delta in [-1,0,1]:
                    declared=max(0,length+delta);wire=bytes([kind])+declared.to_bytes(2,'big')+bytes((i*17+length)%256 for i in range(length))
                    for cancel in [0,1]:rows.append((origin,MAX if cancel else 1,cancel,len(wire),wire))
    for _ in range(8192):
        wire=bytes(rng.randrange(256) for _ in range(rng.randrange(41)));rows.append((rng.randrange(32),rng.choice([1,MAX-1,MAX]),rng.randrange(2),rng.randrange(41),wire))
    for wire in [b'\x01\x00\x02\x07\x08',b'\x02\0\0',b'\1\0\0',b'']:
        for n in range(len(wire)+2):
            for h in range(32):rows.append((h,1,0,n,wire))
    inputs=[f'{h} {g} {c} {n} '+(w.hex() if w else '-') for h,g,c,n,w in rows];wanted=[model(row) for row in rows];trace=execute(driver,'oracle',inputs,wanted)
    histories=[''.join(p) for p in itertools.product('01234567',repeat=5)]
    histories+=['014301536','01230135','010101030','00000000000','0143014301','0172356']
    history_trace=execute(driver,'history',histories,[history(p) for p in histories])
    return dict(passed=True,cases=len(rows),admitted=sum(line.split()[1]=='0' for line in wanted),histories=len(histories),transitions=sum(map(len,histories)),trace_sha256=trace,history_sha256=history_trace,limits='Finite list/byte model with serialized callers. No arbitrary forged native views, data-race executions, scheduler or physical-network claim.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
