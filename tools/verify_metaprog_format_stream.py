"""Independent bounded grammar substitution and accepted-prefix state model."""
import argparse
import hashlib
import itertools
import json
import random
import re
import subprocess

def hx(b):return b.hex() or '-'
def parse(fmt):
    if len(fmt)>32:return 1,None
    tokens=[];requirements={};pos=0
    while pos<len(fmt):
        match=re.match(rb'(\{\{|\}\}|\{([us])([0-3])\}|[^{}])',fmt[pos:],re.DOTALL)
        if not match:return 2,None
        token=match[0];pos+=len(token)
        if match[2]:
            kind=1 if match[2]==b'u' else 2;index=int(match[3])
            if index in requirements and requirements[index]!=kind:return 3,None
            requirements[index]=kind;tokens.append((kind,index))
        else:tokens.append(token[:1])
    return 0,(tokens,requirements)
def formatted(fmt,args,cap,used):
    ps,program=parse(fmt);ms=rs=-1;length=777;output=bytes([165])*160
    if ps==0:
        tokens,req=program;ms=0
        if any(a[0] not in [1,2] for a in args):ms=3
        elif any(a[0]==2 and len(a[2])>16 for a in args):ms=1
        elif any(i>=len(args) or args[i][0]!=k for i,k in req.items()):ms=3
        if ms==0:
            result=b''.join(t if isinstance(t,bytes) else (str(args[t[1]][1]).encode() if t[0]==1 else args[t[1]][2]) for t in tokens)
            if len(result)>128:ms=4
            else:length=len(result)
        rs=1 if used>cap else ms
        if rs==0:
            if length>cap-used:rs=4
            else:output=output[:used]+result+output[used+length:];used+=length
    return f'{ps} {ms} {rs} {length} {used} {output.hex()}'
def streamed(msg,steps):
    state=1;offset=0;uncertain=0;calls=0;external=b'';failure=0;results=[]
    for accepted,status,cancel in steps:
        if cancel and state==1:state=4
        if state==2:r=0
        elif state==3:r=failure
        elif state==4:r=9
        elif offset==len(msg):state=2;r=0
        else:
            left=len(msg)-offset;calls+=1;external+=msg[offset:offset+min(left,accepted)]
            if accepted>left or status>2 or (status==0 and accepted==0):state=3;uncertain=1;failure=8;r=8
            else:
                offset+=accepted
                if status==2:state=3;failure=7;r=7
                elif offset==len(msg):state=2;r=0
                else:r=6 if status==1 else 5
        results.append(str(r))
    return ','.join(results)+f' {state} {offset} {uncertain} {calls} {hx(external)}'
def cases():
    rows=[];expected=[];format_count=stream_count=accepted=0
    def addf(fmt,args,cap,used):
        nonlocal format_count,accepted
        rows.append(f'F {hx(fmt)} {len(args)} {cap} {used}'+''.join(f' {k} {n} {hx(s)}' for k,n,s in args));value=formatted(fmt,args,cap,used);expected.append(value);format_count+=1;accepted+=value.split()[2]=='0'
    argsets=[[],[(1,0,b'')],[(1,4294967295,b'')],[(2,0,b'%n\0')],[(1,42,b''),(2,0,b'A')],[(2,0,b'x'*17)],[(9,0,b'')],[(2,7,b'x'),(1,9,b'')]]
    formats=[b'',b'plain%n',b'{u0}',b'{s0}',b'{{{u0}}}:{s1}',b'{u0}{s0}',b'{u4}',b'{n}',b'{',b'}',b'{{',b'}}',b'\0{s0}\0',b'{s0}'*8,b'x'*33]
    for fmt,args,cap,used in itertools.product(formats,argsets,[0,1,2,9,10,11,12,128,140],[0,1,3]):addf(fmt,args,cap,used)
    rng=random.Random(68008);pieces=[b'x',b'%n',b'\0',b'{{',b'}}',b'{u0}',b'{s1}',b'{u2}',b'{s3}',b'{bad}',b'}']
    for _ in range(4000):
        fmt=b''.join(rng.choice(pieces) for _ in range(rng.randrange(9)));args=[(rng.choice([1,2]),rng.randrange(2**32),bytes(rng.randrange(256) for _ in range(rng.randrange(18)))) for _ in range(rng.randrange(5))];cap=rng.randrange(141);used=rng.randrange(cap+2);addf(fmt,args,cap,used)
    choices=list(itertools.product([0,1,2,4],[0,1,2,3],[0,1]))
    for msg in [b'',b'a',b'a\0c']:
        for steps in itertools.product(choices,repeat=2):
            rows.append(f'S {hx(msg)} 2'+''.join(f' {a} {s} {c}' for a,s,c in steps));expected.append(streamed(msg,steps));stream_count+=1
    for _ in range(2500):
        msg=bytes(rng.randrange(256) for _ in range(rng.randrange(9)));steps=[(rng.randrange(10),rng.randrange(4),int(rng.randrange(8)==0)) for _ in range(6)];rows.append(f'S {hx(msg)} 6'+''.join(f' {a} {s} {c}' for a,s,c in steps));expected.append(streamed(msg,steps));stream_count+=1
    return rows,expected,format_count,stream_count,accepted
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);a=ap.parse_args();rows,expected,nf,ns,accepted=cases();p=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(i,rows[i],x,y)
    digest=hashlib.sha256(('\n'.join(rows)+'\n'+'\n'.join(expected)+'\n').encode()).hexdigest();print(json.dumps(dict(passed=True,cases=len(rows),format_cases=nf,stream_cases=ns,accepted_formats=accepted,trace_sha256=digest)))
if __name__=='__main__':main()
