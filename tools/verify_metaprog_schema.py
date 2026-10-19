"""Independent byte/record oracle. No schema C source or descriptors are consulted."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess

def encode(v,p,c,x,n,cap):
    old='a5'*64
    if v not in [1,2] or p&3!=3 or p&~7:return [1,999,old]
    if p&4 and v==1:return [2,999,old]
    fields=[(1,c.to_bytes(2,'big')),(2,x.to_bytes(4,'big'))]
    if p&4:
        b=n.to_bytes(4,'big',signed=True);fields.append((3,b+bytes([b[0]^b[1]^b[2]^b[3]])))
    wire=bytes([v,len(fields)])+b''.join(bytes([tag,len(b)])+b for tag,b in fields)
    if len(wire)>cap:return [3,999,old]
    return [0,len(wire),wire.hex()+'a5'*(64-len(wire))]

def decode(b):
    bad=[1,3,99,88,77]
    if not 2<=len(b)<=64 or b[0] not in [1,2] or b[1]>3:return bad
    at=2;last=0;values={}
    for _ in range(b[1]):
        if at+2>len(b):return bad
        tag,n=b[at:at+2];at+=2
        if tag<=last or tag not in [1,2,3] or n!={1:2,2:4,3:5}[tag] or at+n>len(b) or tag==3 and b[0]==1:return bad
        last=tag;payload=b[at:at+n];at+=n
        if tag==3:
            if payload[4]!=payload[0]^payload[1]^payload[2]^payload[3]:return bad
            values[tag]=int.from_bytes(payload[:4],'big',signed=True)
        else:values[tag]=int.from_bytes(payload,'big')
    if at!=len(b) or not {1,2}<=values.keys():return bad
    return [0,7 if 3 in values else 3,values[1],values[2],values.get(3,0)]

def run(driver):
    cases=[];expected=[];kinds=[]
    def add(line,out,kind):cases.append(line);expected.append(out);kinds.append(kind)
    wires=set()
    for v,p,c,x,n,cap in itertools.product([0,1,2,3],[0,1,2,3,4,7,11],[0,0x1234,65535],[0,0x89abcdef,4294967295],[-2147483648,-1,0,2147483647],[0,11,12,18,19,64]):
        out=encode(v,p,c,x,n,cap);add(f'E {v} {p} {c} {x} {n} {cap}',out,'encode')
        if out[0]==0:wires.add(bytes.fromhex(out[2])[:out[1]])
    bad=set([b'',bytes([2]),bytes(65)])
    for wire in wires:
        for i in range(len(wire)):bad.add(wire[:i])
        bad.add(wire+b'\x00')
        for i in range(len(wire)):
            for delta in [1,128,255]:b=bytearray(wire);b[i]^=delta;bad.add(bytes(b))
    randomizer=random.Random(68006)
    for _ in range(500):bad.add(randomizer.randbytes(randomizer.randrange(66)))
    for b in sorted(wires|bad):add('D '+(b.hex() or '-'),decode(b),'decode')
    for p,c,x,n,stop in itertools.product([0,1,2,3,7,11],[0,65535],[0,4294967295],[-2147483648,2147483647],[0,1,2,3,4]):
        if p&3!=3 or p&~7:out=[1,999,0,0,0]
        else:
            vals=[c,x]+([n] if p&4 else []);count=stop if 0<stop<=len(vals) else len(vals);out=[4 if 0<stop<=len(vals) else 0,count,count,int('123'[:count]),sum(vals[:count])]
        add(f'V {p} {c} {x} {n} {stop}',out,'visit')
    p=subprocess.run([str(driver),'oracle'],input='\n'.join(cases)+'\n',text=True,capture_output=True,check=True)
    lines=p.stdout.splitlines();assert len(lines)==len(cases),(len(lines),len(cases))
    for line,exp,case,kind in zip(lines,expected,cases,kinds):
        words=line.split();got=[int(x) for x in words[:2]]+[words[2]] if kind=='encode' else list(map(int,words));assert got==exp,(case,got,exp)
    return dict(passed=True,cases=len(cases),encode_cases=kinds.count('encode'),decode_cases=kinds.count('decode'),visit_cases=kinds.count('visit'),accepted=sum(e[0]==0 for e in expected),stopped=sum(e[0]==4 for e in expected),trace_sha256=hashlib.sha256(json.dumps(list(zip(cases,expected)),separators=(',',':')).encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
