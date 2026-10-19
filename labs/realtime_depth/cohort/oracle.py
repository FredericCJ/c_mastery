"""Independent immutable unit-right histories and explicit published counterexample timeline."""
import argparse,itertools,random
from pathlib import Path
MAX=2**32-1
def initial(q,p,t,limit):return (q,p,t,limit,False,0,0,tuple((t,None) for _ in range(q)))
def advance(s,t):
    q,p,last,limit,active,served,debt,units=s
    units=tuple((due,max(due,last) if active and anchor is None and due<=t else anchor) for due,anchor in units)
    return q,p,t,limit,active,served,debt,units
def step(s,op,t,arg):
    q,p,last,limit,active,served,debt,units=s
    if t<last:return 4,s
    x=advance(s,t)
    if not op:
        if arg not in (0,1):return 4,s
        if bool(arg)!=active:
            units=tuple((t,t if arg else None) if due<=t else (due,anchor) for due,anchor in x[-1])
            x=q,p,t,limit,bool(arg),served,debt,units
        return 0,x
    if not arg or not active:return 4,s
    available=sum(due<=t for due,_ in x[-1])
    if not available or arg>available+limit or t+arg>MAX or served+arg>2**64-1:return 5,s
    for tick in range(t,t+arg):
        x=advance(x,tick);units=list(x[-1]);ready=[i for i,(_,a) in enumerate(units) if a is not None]
        if ready:
            i=min(ready,key=lambda i:(units[i][1],i));due=units[i][1]+p
            if due>MAX:return 5,s
            units[i]=max(due,tick+1),None
        else:
            i=min(range(q),key=lambda i:(units[i][0],i));due=units[i][0]+p
            if due>MAX:return 5,s
            units[i]=due,None;debt+=1
        served+=1
        x=q,p,tick,limit,active,served,debt,tuple(units)
    return 0,advance(x,t+arg)
def row(result,s):
    _,_,t,_,active,served,debt,units=s
    return [result,t,int(active),served,debt,sum(due<=t for due,_ in units),
            *(v for due,anchor in units for v in (due,0 if anchor is None else anchor,int(anchor is not None)))]
def cases():
    rng=random.Random(52051)
    for overrun in (0,1):
        for word in itertools.product(range(5),repeat=4):
            s=initial(2,3,0,overrun);ops=[];rows=[];t=0
            for a in word:
                op,arg=(0,0) if a==0 else (0,1) if a==1 else (1,1) if a==2 else (1,2) if a==3 else (0,1)
                t+=4 if a==4 else 1
                ops.extend((op,t,arg));result,s=step(s,op,t,arg);rows.append(row(result,s))
            yield ' '.join(map(str,[2,3,0,overrun,4,*ops])),rows
    for history in range(1000):
        q=rng.randint(1,16);p=rng.randint(q,40);limit=rng.randint(0,q);start=MAX-60 if history%10==0 else 0
        s=initial(q,p,start,limit);t=start;ops=[];rows=[]
        for index in range(18):
            t=min(MAX,t+rng.randrange(7));op=rng.randrange(2);arg=rng.randrange(q+limit+3) if op else rng.randrange(3)
            now=max(0,t-5) if index%11==0 else t
            result,s=step(s,op,now,arg);ops.extend((op,now,arg));rows.append(row(result,s))
        yield ' '.join(map(str,[q,p,start,limit,18,*ops])),rows
def check(output):
    lines=output.splitlines();expected=[r for _,rows in cases() for r in rows]
    if len(lines)!=len(expected):raise ValueError('Missing/extra unit history row')
    for index,(line,wanted) in enumerate(zip(lines,expected)):
        parts=line.split()
        if not parts or parts[0]!='U' or list(map(int,parts[1:]))!=wanted:raise ValueError('Unit history mismatch '+str(index))
    return {'rows':len(expected),'exhaustive_histories':1250,'seeded_histories':1000,'scope':'Finite eligibility/activation/debt histories; no arbitrary scheduler proof.'}
def check_trace(output):
    lines=[list(map(int,line.split()[1:])) for line in output.splitlines() if line.startswith('TRACE ')]
    expected=[]
    for flawed,end in ((0,99),(1,117)):
        low=49
        for t in range(end):
            choice=0 if t<18 else 1 if t==40 or 51<=t<70 else 3 if 41<=t<51 else 2 if 90<=t<(110 if flawed else 92) else 4
            available=20-t if t<18 else 2 if t<=40 else 1 if t<50 else 19 if t<=51 else 70-t if t<70 else 0 if t<90 else 110-t if flawed and t<110 else 92-t if not flawed and t<92 else 0
            expected.append([flawed,t,choice,available,low]);low-=choice==4
        if low:raise ValueError('Invalid independently specified low-priority witness')
    if lines!=expected:raise ValueError('Premature replenishment trace mismatch')
    return {'rows':len(expected),'corrected_finish':99,'flawed_finish':117,'scope':'Exact three-arrival original teaching witness; not universal equivalence.'}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path);args=parser.parse_args()
    if args.output:print(check(args.output.read_text(encoding='utf-8')))
    else:
        for line,_ in cases():print(line)
