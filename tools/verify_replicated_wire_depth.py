"""Independent recursive semantic-map and Python codec comparison."""
import argparse
from collections import Counter
import hashlib
import itertools
import json
import math
import random
import struct
import subprocess

class Refusal(Exception):
    def __init__(self,status):self.status=status
def var(x):
    n=max(1,(x.bit_length()+6)//7)
    return bytes(((x>>(7*i))&127)|(128 if i+1<n else 0) for i in range(n))
def get(raw,at):
    stop=next((i for i in range(at,min(at+5,len(raw))) if raw[i]<128),None)
    if stop is None or stop-at==4 and raw[stop]>15:raise Refusal(1)
    value=sum((raw[i]&127)*128**(i-at) for i in range(at,stop+1))
    if raw[at:stop+1]!=var(value):raise Refusal(1)
    return value,stop+1
def encode(record,shuffle=None):
    tags=sorted(record)
    if shuffle:shuffle.shuffle(tags)
    out=b''
    for tag in tags:
        value=record[tag]
        if isinstance(value,dict):body=encode(value,shuffle)
        elif isinstance(value,bytes):body=value
        else:out+=var(tag*8)+var(value);continue
        out+=var(tag*8+2)+var(len(body))+body
    return out
def decode(raw,depth,byte_limit,field_limit,supported):
    if len(raw)>byte_limit or len(raw)>256:raise Refusal(2)
    fields=0;nodes=1
    def record(raw,level):
        nonlocal fields,nodes
        if level>depth:raise Refusal(2)
        out={};at=0
        while at<len(raw):
            key,at=get(raw,at);tag,kind=divmod(key,8)
            if tag>15:raise Refusal(3)
            if not tag:raise Refusal(1)
            if tag in (3,4,7):
                if kind!=2:raise Refusal(1)
            elif tag in (1,2,5,6):
                if kind!=0:raise Refusal(1)
            elif kind not in (0,2):raise Refusal(1)
            if tag in out:raise Refusal(1)
            if len(out)==8 or fields==field_limit:raise Refusal(2)
            fields+=1
            value,at=get(raw,at)
            if kind:
                if value>len(raw)-at:raise Refusal(1)
                body=raw[at:at+value];at+=value
                if tag in (3,4):
                    if nodes==16:raise Refusal(2)
                    nodes+=1;value=record(body,level+1)
                else:
                    if value>32:raise Refusal(2)
                    value=body
            elif tag==5 and value&~supported:raise Refusal(3)
            out[tag]=value
        if not {1,2}<=out.keys():raise Refusal(1)
        return out
    return record(raw,1)
def cases():
    rng=random.Random(0x760800);rows=[]
    values=[0,1,127,128,16383,16384,0x7fffffff,0x80000000,0xffffffff]
    for a,b in itertools.product(values,repeat=2):
        record={1:a,2:b,8:7,9:b'\x10\x01\x08\x01'}
        for _ in range(4):rows.append((4,256,32,3,encode(record,rng)))
    def make(level):
        r={1:rng.choice(values),2:rng.choice(values)}
        for tag in rng.sample([3,4,5,6,7,8,9,10,11,12,13,14,15],rng.randrange(7)):
            if tag in (3,4):
                if level<5:r[tag]=make(level+1)
            elif tag in (5,6):r[tag]=rng.randrange(8)
            elif tag==7 or rng.randrange(2):r[tag]=bytes(rng.randrange(256) for _ in range(rng.randrange(34)))
            else:r[tag]=rng.choice(values)
        return r
    for _ in range(1800):
        raw=encode(make(1),rng)
        if len(raw)>256:continue
        rows.append((rng.randrange(1,5),rng.choice([4,8,32,64,128,256]),rng.choice([2,3,8,16,31,32]),rng.choice([0,1,3,0xffffffff]),raw))
        if raw:
            cut=rng.randrange(len(raw));rows.append((4,256,32,3,raw[:cut]))
            changed=bytearray(raw);changed[rng.randrange(len(raw))]^=1<<rng.randrange(8);rows.append((4,256,32,3,bytes(changed)))
    seed=encode({1:1,2:1,3:{1:150,2:0xffffffff},8:bytes(range(32))})
    for end in range(len(seed)+1):rows.append((4,256,32,3,seed[:end]))
    for i in range(len(seed)):
        for bit in range(8):b=bytearray(seed);b[i]^=1<<bit;rows.append((4,256,32,3,bytes(b)))
    special=['0880001001','080110010802','0880808080101001','0880808080801001','080110801','08011001800100','080110013a21'+('00'*33),'080110011a00']
    # Deliberately malformed examples must still be whole octets.
    special[4]='08011080'
    for h in special:rows.append((4,256,32,3,bytes.fromhex(h)))
    lines=[];expected=[];counts=Counter();canonical_set=set()
    for depth,limit,fields,support,raw in rows:
        lines.append(f'{depth} {limit} {fields} {support} '+(raw.hex() or '-'))
        try:body=encode(decode(raw,depth,limit,fields,support));status=0;canonical_set.add(body);result=body.hex()
        except Refusal as e:status=e.status;result='-'
        counts[status]+=1;expected.append(f'{status} {result}')
    return lines,expected,counts,len(canonical_set)
def norm_cases():
    rng=random.Random(0x7608ff);bits=[0,1,0x8000000000000000,0x7ff0000000000000,0xfff0000000000000,0x7ff0000000000001,0xfff8000000000001,0x3ff0000000000000]
    texts=[b'',b'a',b'\0',b'\xc3\xa9',b'e\xcc\x81',b'\xe0\x80\x80',b'\xed\xa0\x80',b'\xf4\x90\x80\x80']
    rows=list(itertools.product(bits,texts))
    for _ in range(2048):
        value=rng.getrandbits(64)
        if rng.randrange(2):
            scalar=rng.randrange(0x110000)
            if 0xd800<=scalar<=0xdfff:scalar=0
            raw=chr(scalar).encode('utf8')
            if rng.randrange(2):raw=raw[:rng.randrange(len(raw))]
        else:raw=bytes(rng.randrange(256) for _ in range(rng.randrange(10)))
        rows.append((value,raw))
    lines=[];expected=[];counts=Counter()
    for bits,raw in rows:
        number=struct.unpack('>d',bits.to_bytes(8,'big'))[0]
        canon=0x7ff8000000000000 if math.isnan(number) else 0 if number==0 else bits
        try:raw.decode('utf8');status=0
        except UnicodeDecodeError:status=1
        counts[status]+=1;lines.append(f'{bits:016x} '+(raw.hex() or '-'));expected.append(f'{canon:016x} {status}')
    return lines,expected,counts
def execute(driver,mode,lines,expected):
    p=subprocess.run([driver,mode],input='\n'.join(lines)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr
    actual=p.stdout.splitlines();assert len(actual)==len(expected)
    for i,(a,b) in enumerate(zip(actual,expected)):assert a==b,(mode,i,lines[i],a,b)
    return hashlib.sha256(p.stdout.encode()).hexdigest()
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);args=ap.parse_args();a,b,c,n=cases();x,y,z=norm_cases();h=execute(args.driver,'oracle',a,b);nh=execute(args.driver,'normalization_oracle',x,y)
    print(json.dumps(dict(passed=True,wire_rows=len(a),wire_status_counts=dict(c),distinct_canonical_values=n,wire_output_sha256=h,normalization_rows=len(x),utf8_status_counts=dict(z),normalization_output_sha256=nh,limits=['Python integer/dictionary/UTF8/binary64 classifications are host oracles, not target dependencies.','The specified opaque-byte equality is intentionally narrower than future schema semantic equality.'])))
