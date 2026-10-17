"""Independent mathematical/tagged-set/hashlib oracles for bounded T76 inputs."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import random
import subprocess

ROOT=Path(__file__).resolve().parents[1]

def hlc(local,remote,wall):
    maximum=max(local[0],remote[0],wall)
    counters=[pair[1] for pair in [local,remote] if pair[0]==maximum]
    value=max(counters)+1 if counters else 0
    return None if value>2**32-1 else [maximum,value]

def merkle(values):
    if not values:return hashlib.sha256(b'').digest()
    if len(values)==1:return hashlib.sha256(b'\0'+values[0].to_bytes(4,'big')).digest()
    split=1<<((len(values)-1).bit_length()-1)
    return hashlib.sha256(b'\1'+merkle(values[:split])+merkle(values[split:])).digest()

def varint(value):
    out=[]
    while value>=128:out.append((value&127)|128);value>>=7
    return out+[value]

def parse_wire(data):
    position=0;fields={}
    def number():
        nonlocal position
        result=0
        for i in range(5):
            if position==len(data):raise ValueError(1)
            byte=data[position];position+=1
            if i==4 and byte>15:raise ValueError(3)
            result|=(byte&127)<<(7*i)
            if byte<128:
                if i and byte==0:raise ValueError(1)
                return result
        raise ValueError(1)
    try:
        while position<len(data):
            key=number()
            if key%8 or not 8<=key<=120 or key//8 in fields:raise ValueError(1)
            value=number()
            if len(fields)==8:raise ValueError(2)
            fields[key//8]=value
        if not {1,2}<=fields.keys():raise ValueError(1)
        return [0]+[byte for tag,value in sorted(fields.items()) for byte in varint(tag*8)+varint(value)]
    except ValueError as e:return [e.args[0]]

def replay(moves):
    position=score=0;random=1
    for move in moves:
        position=(position+move)%16;random=(random*1664525+1013904223)%2**32;score+=random%4
    return [0,len(moves),position,score,random]

def sequence(nodes,removed):
    children={}
    for ident,parent,value in nodes:children.setdefault(parent,[]).append((ident,value))
    def walk(parent):
        result=[]
        for ident,value in sorted(children.get(parent,[]),reverse=True):
            if ident not in removed:result.append(value)
            result+=walk(ident)
        return result
    return walk(0)

def fixture_check():
    cases=json.loads((ROOT/'fixtures/replicated/cases.json').read_text(encoding='utf-8'))['cases']
    assert len(cases)==9
    results={}
    for row in cases:
        kind=row['kind'];data=row['input']
        if kind=='log':results[kind]={'value':dict(data['events'][:data['prefix']])[data['key']],'nodes':3*len(data['events'])}
        elif kind=='projection':unique={event[0]:event for event in data['deliveries']};results[kind]={'count':len(unique),'total':sum(event[2] for event in unique.values())}
        elif kind=='clocks':results[kind]=hlc(data['local'],data['remote'],data['wall'])
        elif kind=='counters':results[kind]=sum(data['positive'])-sum(data['negative'])
        elif kind=='sets':results[kind]={'live':sorted(set(data['observed']+data['concurrent'])-set(data['removed']))}
        elif kind=='sequence':results[kind]=sequence(data['nodes'],data['removed'])
        elif kind=='merkle':results[kind]={'difference_mask':sum(1<<i for i,(a,b) in enumerate(zip(data['left'],data['right'])) if a!=b)}
        elif kind=='wire':results[kind]=parse_wire(data['bytes'])[1:]
        elif kind=='replay':values=replay(data['moves']);results[kind]=dict(zip(['tick','position','score','random'],values[1:]))
        assert results[kind]==row['expected'],(kind,results[kind],row['expected'])
    return len(cases)

def cases():
    rng=random.Random(76017)
    for n in range(66):
        for pattern in [bytes(n),bytes(range(n)),bytes(rng.randrange(256) for _ in range(n))]:
            yield 'hash',[n,*pattern],hashlib.sha256(pattern).hexdigest()
    yield 'hash',[3,97,98,99],hashlib.sha256(b'abc').hexdigest()
    for n in range(9):
        for _ in range(48):
            values=[rng.randrange(2**32) for _ in range(n)]
            yield 'merkle',[n,*values],merkle(values).hex()
    for lp,lc,rp,rc,wall in itertools.product(range(4),range(4),range(4),range(4),range(4)):
        result=hlc([lp,lc],[rp,rc],wall)
        yield 'hlc',[lp,lc,rp,rc,wall],' '.join(map(str,[0,*result]))
    for local,remote,wall in [([2**64-1,0],[0,0],0),([9,2**32-1],[9,0],8),([1,1],[1,2**32-1],2)]:
        result=hlc(local,remote,wall);yield 'hlc',[*local,*remote,wall],' '.join(map(str,[0,*result] if result else [3]))
    for n in range(17):
        events=[(rng.randrange(32),rng.randrange(-100,101)) for _ in range(n)]
        for prefix in range(n+1):
            for key in {0,31,*[k for k,_ in events[:prefix]]}:
                state=dict(events[:prefix]);expected=[0,state[key],n,3*n] if key in state else [5,777,n,3*n]
                yield 'log',[n,*[x for event in events for x in event],prefix,key],' '.join(map(str,expected))
    for _ in range(512):
        a=[rng.randrange(2**32) for _ in range(6)];b=[rng.randrange(2**32) for _ in range(6)];joined=list(map(max,a,b))
        yield 'pn',a+b,' '.join(map(str,joined+[sum(joined[:3])-sum(joined[3:])]))
    for _ in range(512):
        states=[dict(adds=[set() for _ in range(4)],removed=[set() for _ in range(4)],next=[0,0]) for _ in range(2)]
        deltas=[dict(adds=[set() for _ in range(4)],removed=[set() for _ in range(4)],next=[0,0]) for _ in range(2)]
        def join(state,other):
            for i in range(4):state['adds'][i]|=other['adds'][i];state['removed'][i]|=other['removed'][i]
            state['next']=list(map(max,state['next'],other['next']))
        commands=[];results=[]
        for _step in range(24):
            op=rng.randrange(4);replica=rng.randrange(2);arg=rng.randrange(4 if op<2 else 2);commands += [op,replica,arg];state=states[replica];status=0
            if op in [0,1]:
                delta=dict(adds=[set() for _ in range(4)],removed=[set() for _ in range(4)],next=[0,0])
                if op==0:
                    if state['next'][replica]==8:status=2
                    else:delta['adds'][arg].add(replica*8+state['next'][replica]);delta['next'][replica]=state['next'][replica]+1
                else:delta['removed'][arg]=set(state['adds'][arg])
                if not status:join(state,delta);deltas[replica]=delta
            else:join(state,states[arg] if op==2 else deltas[arg])
            results.append(status)
        for state in states:
            for i in range(4):results += [sum(1<<tag for tag in state['adds'][i]),sum(1<<tag for tag in state['removed'][i])]
            results+=state['next']
        yield 'set',[24,*commands],' '.join(map(str,results))
    for _ in range(400):
        n=rng.randrange(1,9);nodes=[(i,rng.randrange(i),65+i) for i in range(1,n+1)];pending=list(nodes);ordered=[];present={0}
        while pending:
            node=rng.choice([v for v in pending if v[1] in present]);pending.remove(node);ordered.append(node);present.add(node[0])
        removed={i for i in range(1,n+1) if rng.randrange(2)};rendered=sequence(nodes,removed)
        yield 'sequence',[n,*[x for node in ordered for x in node],sum(1<<(i-1) for i in removed)],' '.join(map(str,[len(rendered),*rendered]))
    for _ in range(400):
        tags=[1,2]+rng.sample(range(3,16),rng.randrange(7));rng.shuffle(tags)
        data=[byte for tag in tags for byte in varint(tag*8)+varint(rng.randrange(2**32))]
        yield 'wire',[len(data),*data],' '.join(map(str,parse_wire(data)))
        for n in {0,1,len(data)//2,len(data)-1}:
            yield 'wire',[n,*data[:n]],' '.join(map(str,parse_wire(data[:n])))
    invalid=[[8,128,0,16,0],[8,255,255,255,255,16,16,0],[8,1,8,2,16,0],[128,1,0],[9,0,16,0],[8,0,16,0,24,128]]
    for data in invalid:yield 'wire',[len(data),*data],' '.join(map(str,parse_wire(data)))
    for value in [-2**31,-2**31+1,-1,0,1,2**31-1,*range(-256,257)]:yield 'zigzag',[value],str(value*2 if value>=0 else -value*2-1)
    for n in range(9):
        for moves in itertools.product([-1,0,1],repeat=n):yield 'replay',[n,*moves],' '.join(map(str,replay(moves)))

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',type=Path);args=parser.parse_args();fixtures=fixture_check();count=0
    if args.driver:
        rows=list(cases());commands=''.join(name+' '+' '.join(map(str,values))+'\n' for name,values,_ in rows)
        process=subprocess.run([str(args.driver)],input=commands,text=True,capture_output=True,timeout=120)
        assert process.returncode==0,process.stderr
        actual=process.stdout.splitlines();assert len(actual)==len(rows),(len(actual),len(rows),process.stderr)
        for index,((name,values,expected),observed) in enumerate(zip(rows,actual)):
            assert observed.strip()==expected,(index,name,values,expected,observed)
        count=len(rows)
    print(json.dumps(dict(passed=True,fixtures=fixtures,driver_comparisons=count,scope='Finite independent models; not full distributed protocol or learner acceptance.')))
if __name__=='__main__':main()
