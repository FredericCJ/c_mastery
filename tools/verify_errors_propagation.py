"""Independent grammar/whole-integer oracle for two propagation representations."""
import argparse
import hashlib
import itertools
import json
import re
import subprocess

def parse(text,signed=False,final=True):
    n=len(text);pos=1 if signed and text.startswith('-') else 0;negative=pos==1;values=[]
    for field in range(2):
        match=re.match('[0-9]*',text[pos:]);digits=match[0]
        for k in range(1,len(digits)+1):
            if int(digits[:k])>65535:return 3,pos+k-1,None
        pos+=len(digits)
        if not digits:
            if not final and pos==n:return 4,n,None
            return 2,pos,None
        values.append(int(digits))
        if field==0:
            if pos==n:return (2 if final else 4),pos,None
            if text[pos]!=':':return 2,pos,None
            pos+=1
        elif pos!=n:return 2,pos,None
    if not final:return 4,n,None
    total=(-values[0] if negative else values[0])+values[1]
    return (0,n,total) if 0<=total<=65535 else (3,n,None)

def run(driver):
    ordinary=[''.join(s) for n in range(7) for s in itertools.product('019:x',repeat=n)]
    special=['1:65534','65536:0','1:65535','4294967295:0','4294967296:0','0'*400+':0','0:'+'0'*400,'-0:0','-65535:65535','-65536:0','-1:0','-1:1','-1:2','--1:2','1:-2','+1:2','1:2\x00']
    rows=[];expected=[]
    for text in ordinary+special:
        code,at,total=parse(text);part=f'{code} {0 if code==0 else at} {total if code==0 else 91}'
        rows.append(f'P 0 0 {len(text)} '+' '.join(str(ord(c)) for c in text));expected.append(' '.join([part]*3))
    ordinary_count=len(rows);stream_count=0
    signed=[''.join(s) for n in range(6) for s in itertools.product('019:-x',repeat=n)]
    for mode,strings in [(1,signed+special),(0,special)]:
        for text in strings:
            for split in range(len(text)+1):
                a,at,_=parse(text[:split],bool(mode),False)
                if a!=4:b,end,total=a,at,None;consumed=0
                else:b,end,total=parse(text,bool(mode));consumed=end-split
                rows.append(f'S {mode} {split} {len(text)} '+' '.join(str(ord(c)) for c in text))
                expected.append(f'{a} {at} {at} {at if a!=4 else 0} {b} {consumed} {end} {end if b!=0 else 0} {total if b==0 else 91}');stream_count+=1
    p=subprocess.run([driver,'oracle'],input='\n'.join(rows)+'\n',capture_output=True,text=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(expected)
    for i,(a,b) in enumerate(zip(actual,expected)):assert a==b,(i,rows[i],a,b)
    trace=hashlib.sha256(('\n'.join(rows+expected)+'\n').encode()).hexdigest();return dict(passed=True,ordinary_cases=ordinary_count,segmented_cases=stream_count,trace_sha256=trace)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
