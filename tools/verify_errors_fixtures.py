"""Independent integer, grammar and transition expectations for the T65 pilot."""
import argparse
from fractions import Fraction
import json
from pathlib import Path
import random
import re
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def expected(row):
    op=row['operation']; a=row['input']
    if op=='optional':
        value,present,old=a
        return ['INVALID',old] if present not in (0,1) else ['RANGE',old] if present and value==0 else ['OK',value if present else 0]
    if op=='parse':
        text=a; match=re.fullmatch(r'([0-9]+):([0-9]+)',text,flags=re.ASCII)
        # Detect the earliest overflowing digit independently with unbounded ints.
        for digits in re.finditer(r'[0-9]+',text,flags=re.ASCII):
            if digits.start()!=0 and (text[digits.start()-1]!=':' or text[:digits.start()-1].count(':') or not text[:digits.start()-1].isdigit()): break
            for size in range(1,len(digits[0])+1):
                if int(digits[0][:size])>65535:return ['RANGE',digits.start()+size-1]
        if match:
            total=sum(map(int,match.groups()))
            return ['OK',total] if total<=65535 else ['RANGE',len(text)]
        prefix=re.match(r'[0-9]+',text,flags=re.ASCII)
        if prefix is None:return ['INVALID',0]
        end=prefix.end()
        if end==len(text) or text[end]!=':':return ['INVALID',end]
        tail=re.match(r'[0-9]+',text[end+1:],flags=re.ASCII)
        return ['INVALID',end+1+(tail.end() if tail else 0)]
    if op=='progress':
        length,start,count,code=a
        if start>length:return ['INVALID',start]
        if start==length:return ['OK',start]
        if count>length-start or code not in ('OK','AGAIN','IO') or (count==0 and code=='OK'):return ['CONTRACT',start]
        return [code,start+count]
    if op=='composition':
        tag,payload,stage=a
        return [tag,payload,0] if tag!='OK' else ['OK',payload+1,1] if stage=='map' else ['RANGE',payload,1]
    if op=='validation':
        age,quota,denominator,capacity=a
        ids=[i for i,bad in enumerate([not 0<=age<=150,not 0<=quota<=1024,not 1<=denominator<=4096],1) if bad]
        return [len(ids),ids[:capacity],len(ids)>capacity]
    if op=='boundary':
        registrations,tag=a
        return [tag,list(reversed(registrations)),0]
    if op=='arithmetic':
        x,y,policy,old=a; total=x+y
        if max(x,y)>65535:return ['INVALID',old]
        if policy=='checked' and total>65535:return ['RANGE',old]
        return ['OK',total%65536 if policy=='wrapping' else min(total,65535) if policy=='saturating' else total]
    if op=='contracts':
        value,amount,mode,reports=a; candidate=value+amount;bad=not 0<=candidate<=100
        if bad and mode!='off':reports+=1
        if bad and mode=='enforce':return ['CONTRACT',value,reports]
        return ['OK',max(0,min(100,candidate)),reports]
    if op=='refinement':
        n,d=a
        if d==0:return ['INVALID',91,92]
        rational=Fraction(n,d)
        return ['OK',rational.numerator,rational.denominator]
    raise ValueError('Unknown fixture operation '+op)


def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');args=p.parse_args()
    cases=json.loads((ROOT/'fixtures/errors/cases.json').read_text())['cases']
    failures=[]
    for row in cases:
        if expected(row)!=row['expected']:failures.append(row['id'])
    for path in sorted((ROOT/'fixtures/errors').glob('PR-*.json')):
        data=json.loads(path.read_text())
        for row in data['cases']:
            if row not in cases or expected(row)!=row['expected']:failures.append(path.name+':'+row['id'])
    comparisons=0
    if args.driver:
        rng=random.Random(650017); commands=[]; answers=[]
        for i in range(1536):
            if i%2:
                a=rng.randrange(2**32); b=rng.randrange(2**32)
                commands.append(f'F {a} {b} 0')
                r=expected(dict(operation='refinement',input=[a,b]));answers.append(('2' if r[0]=='INVALID' else '0')+' '+str(r[1])+' '+str(r[2]))
            else:
                a=rng.randrange(65538);b=rng.randrange(65538);policy=rng.randrange(3)
                commands.append(f'A {a} {b} {policy}')
                r=expected(dict(operation='arithmetic',input=[a,b,['checked','wrapping','saturating'][policy],77]));answers.append(str({'OK':0,'INVALID':2,'RANGE':3}[r[0]])+' '+str(r[1]))
        result=subprocess.run([args.driver],input='\n'.join(commands)+'\n',capture_output=True,text=True,timeout=30)
        lines=result.stdout.splitlines()
        if result.returncode or lines!=answers:
            failures.append('C differential stream differs')
            mismatch=next((i for i,(x,y) in enumerate(zip(lines,answers)) if x!=y),min(len(lines),len(answers)))
            print('First divergent index:',mismatch)
        else:comparisons=len(commands)
    print(json.dumps(dict(fixtures=len(cases),differential_cases=comparisons,failures=failures)))
    return 1 if failures else 0
if __name__=='__main__':raise SystemExit(main())
