"""Independent original TLV grammar, stable multimap order and HMAC transcript."""
import argparse
import hashlib
import hmac
import itertools
import json
import random
import subprocess
KEY=bytes(range(32));DOMAIN=b'CMASTERY-ADT-RECORD-v1\0';MAX=2**32-1
def parse(wire):
    if not wire or len(wire)>145 or wire[0]>8:return None
    fields=[];at=1;seen=set()
    for _ in range(wire[0]):
        if at+2>len(wire):return None
        tag,n=wire[at:at+2];at+=2
        if n>16 or at+n>len(wire) or tag==0 or tag>=128 or tag==1 and n!=2 or tag==2 and n!=0:return None
        if tag<=2 and tag in seen:return None
        seen.add(tag);fields.append((tag,wire[at:at+n]));at+=n
    return fields if at==len(wire) else None
def encode(fields):return bytes([len(fields)])+b''.join(bytes([tag,len(value)])+value for tag,value in fields)
def model(row):
    canonical,cap,total,tamper,wire=row;fields=parse(wire);out=b'\xa5'*145;tag=b'\xa5'*32
    read=1;write=1;used=999;auth=1;apply=1;privileged=7
    if fields is not None:
        read=0;ordered=sorted(fields,key=lambda item:item[0]);encoded=encode(ordered if canonical else fields);write=0 if cap>=len(encoded) else 2
        if write==0:out=encoded+out[len(encoded):];used=len(encoded)
        tag=hmac.digest(KEY,DOMAIN+encode(ordered),'sha256');auth=0
        if tamper:tag=tag[:-1]+bytes([tag[-1]^1]);apply=3
        elif any(t>2 for t,_ in fields) or not any(t==1 for t,_ in fields):apply=4
        else:
            amount=next(int.from_bytes(v,'big') for t,v in fields if t==1)
            if total+amount>MAX:apply=5
            else:total+=amount;apply=0
    return f'{read} 1 {write} {used} {auth} {apply} {total} {privileged} {out.hex()} {tag.hex()}'
def verify(driver):
    rng=random.Random(6709);rows=[];choices=[(1,b'\0\0'),(1,b'\0\7'),(1,b'\xff\xff'),(2,b''),(3,b''),(3,b'\0'),(9,b'\0'),(9,b'\1'),(127,b'\xff')]
    for n in range(4):
        for fields in itertools.product(choices,repeat=n):
            wire=encode(fields)
            for canonical in [0,1]:rows.append((canonical,145,MAX-6 if n%2 else 10,0,wire))
    for _ in range(4096):
        fields=[]
        for _ in range(rng.randrange(9)):
            t=rng.choice([1,2,3,9,127]);length=2 if t==1 else 0 if t==2 else rng.randrange(17)
            fields.append((t,bytes(rng.randrange(256) for _ in range(length))))
        wire=encode(fields);rows.append((rng.randrange(2),rng.randrange(146),rng.getrandbits(32),rng.randrange(2),wire))
    examples=[encode([(1,b'\0\7'),(9,b'\0'),(9,b'\1')]),encode([(127,bytes(range(16)))]*8),b'\0']
    for wire in examples:
        rows.extend((c,cap,0,0,wire) for c in [0,1] for cap in range(146))
        rows.extend((1,145,0,0,wire[:cut]) for cut in range(len(wire)))
        rows.append((1,145,0,0,wire+b'\0'))
    for tag in range(256):
        for n in [0,1,2,16,17]:rows.append((1,145,10,0,bytes([1,tag,n])+bytes(range(n))))
    rows.extend((1,145,10,0,bytes(rng.randrange(256) for _ in range(rng.randrange(160)))) for _ in range(2048))
    text='\n'.join(f'{c} {cap} {total} {tamper} '+(wire.hex() if wire else '-') for c,cap,total,tamper,wire in rows)+'\n'
    p=subprocess.run([driver,'oracle'],input=text,text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;got=p.stdout.splitlines();want=[model(row) for row in rows]
    assert got==want,next(((i,rows[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    histogram={str(i):sum(int(line.split()[5])==i for line in want) for i in range(6)}
    return dict(passed=True,cases=len(rows),admitted=sum(parse(row[4]) is not None for row in rows),apply_statuses=histogram,trace_sha256=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest(),limits='Independent TLV grammar/stable tuple sorting and Python standard HMAC verify the original bounded format. No Protobuf compatibility, digital-signature implementation, cryptographic proof or arbitrary deployment claim.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
