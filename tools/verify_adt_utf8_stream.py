"""Strict Python decoder comparison with transactional chunks and all small bytes."""
import argparse
import codecs
import hashlib
import itertools
import json
import random
import subprocess
PREFIXES={b''}
for scalar in range(0x110000):
    if 0xd800<=scalar<=0xdfff:continue
    encoded=chr(scalar).encode('utf-8','strict')
    for cut in range(1,len(encoded)):PREFIXES.add(encoded[:cut])
def expected(data,mask):
    try:decoded=data.decode('utf-8','strict');base=(1,len(data),len(decoded))
    except UnicodeDecodeError:base=(0,77,78)
    decoder=codecs.getincrementaldecoder('utf-8')('strict');last=0;chunks=[]
    for i in range(len(data)+1):
        if mask&(1<<i):chunks.append(data[last:i]);last=i
    chunks.append(data[last:]);count=0;size=0;statuses=[]
    for chunk in chunks:
        saved=decoder.getstate()
        try:text=decoder.decode(chunk,final=False)
        except UnicodeDecodeError:decoder.setstate(saved);statuses.append(2)
        else:
            # CPython can defer ED A0's inevitable error. This API rejects an
            # impossible prefix immediately; derive all possible prefixes from
            # the independent encoder over every Unicode scalar.
            if decoder.getstate()[0] not in PREFIXES:decoder.setstate(saved);statuses.append(2)
            else:size+=len(chunk);count+=len(text);statuses.append(0)
    saved=decoder.getstate();pending=saved[0]
    try:decoder.decode(b'',final=True)
    except UnicodeDecodeError:decoder.setstate(saved);final=2;phase=0;out=(77,78)
    else:final=0;phase=1;out=(size,count)
    return ' '.join(map(str,[*base,''.join(map(str,statuses)),phase,size,count,len(pending),pending.hex() or '-',final,*out]))
def cases():
    rows=[]
    for n in range(3):
        for values in itertools.product(range(256),repeat=n):
            data=bytes(values)
            for split in range(n+1):rows.append((data,1<<split))
    boundary=[b'',bytes([0x00]),bytes([0x7f]),bytes([0xc2,0x80]),bytes([0xdf,0xbf]),bytes([0xe0,0xa0,0x80]),bytes([0xed,0x9f,0xbf]),bytes([0xee,0x80,0x80]),bytes([0xef,0xbf,0xbf]),bytes([0xf0,0x90,0x80,0x80]),bytes([0xf4,0x8f,0xbf,0xbf]),bytes([0xc0,0x80]),bytes([0xed,0xa0,0x80]),bytes([0xf4,0x90,0x80,0x80]),bytes([0xf0,0x8f,0xbf,0xbf]),bytes([0xe0,0x9f,0xbf]),bytes([0xf5,0x80,0x80,0x80]),bytes([0x41,0xe2,0x82,0xac,0xf4,0x8f,0xbf,0xbf])]
    for data in boundary:
        for n in range(len(data)+1):
            prefix=data[:n]
            for mask in range(1<<(n+1)):rows.append((prefix,mask))
    for lead in [0xe0,0xed,0xf0,0xf4]:
        for second in range(256):
            data=bytes([lead,second,0x80]+([0x80] if lead>=0xf0 else []))
            for mask in range(1<<(len(data)+1)):rows.append((data,mask))
    rng=random.Random(6704)
    for _ in range(10000):
        data=bytes(rng.randrange(256) for _ in range(rng.randrange(9)));rows.append((data,rng.randrange(1<<(len(data)+1))))
    return rows
def verify(driver):
    rows=cases();wanted=[expected(data,mask) for data,mask in rows];text='\n'.join(wanted)+'\n'
    p=subprocess.run([driver,'oracle'],input='\n'.join((data.hex() or '-')+' '+str(mask) for data,mask in rows)+'\n',text=True,capture_output=True,timeout=120)
    assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert actual==wanted,next(((rows[i][0].hex(),rows[i][1],a,b) for i,(a,b) in enumerate(zip(actual,wanted)) if a!=b),(len(actual),len(wanted)))
    return dict(passed=True,cases=len(rows),complete_small_strings=65793,small_string_splits=197121,possible_prefixes=len(PREFIXES),scalar_encodings=1112064,trace_sha256=hashlib.sha256(text.encode()).hexdigest(),limits='Python strict UTF-8 decode plus valid-prefix language derived by encoding every scalar; all0/1/2-byte strings and stated boundary partitions, not every long string or normalization/grapheme semantics.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
