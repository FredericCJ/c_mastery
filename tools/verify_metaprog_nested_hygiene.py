"""Independent explicit-work-stack context/repetition expansion model."""
import argparse
import collections
import hashlib
import itertools
import json
import random
import subprocess
MAX=2**32-1
def instruction(op,target=0,mask=0,symbol=7,context=0):return (op,target,mask,0,symbol,context)
def model(program,entry,args,caller,budget,next_context):
    depth_limit,token_limit,step_limit,allow=budget;start=next_context
    def fail(code):return f'{code} {start} 777 0 0 0'
    if not 1<=len(program)<=4 or entry>=len(program) or not 1<=depth_limit<=8 or token_limit>64 or not 1<=step_limit<=256 or allow>1 or not start or caller>=start:return fail(1)
    for arg in args:
        if len(arg)>4 or any(kind>2 or context>=start for kind,symbol,context in arg):return fail(1)
    for definition in program:
        if len(definition)>16:return fail(1)
        for op,target,mask,kind,symbol,context in definition:
            if op>5 or (op==1 and target>1) or (op in [3,4] and target>=len(program)) or (op==4 and not 1<=mask<=3) or (op==0 and context>=start):return fail(1)
    work=[('call',entry,caller,1,0,(0,0))];out=[];steps=calls=peak=0
    while work:
        task=work.pop()
        if task[0]=='call':
            _,definition,parent,depth,bound,indices=task
            if depth>depth_limit or steps==step_limit:return fail(3)
            if next_context==MAX:return fail(4)
            current=next_context;next_context+=1;steps+=1;calls+=1;peak=max(peak,depth)
            work.extend(('instruction',ins,current,parent,depth,bound,indices) for ins in reversed(program[definition]))
        else:
            _,ins,current,parent,depth,bound,indices=task;op,target,mask,kind,symbol,context=ins
            if steps==step_limit:return fail(3)
            steps+=1
            if op==3:work.append(('call',target,current,depth+1,bound,indices))
            elif op==4:
                if mask&bound:return fail(2)
                lengths=[len(args[j]) for j in range(2) if mask&(1<<j)]
                if len(set(lengths))!=1:return fail(2)
                for k in reversed(range(lengths[0])):work.append(('call',target,current,depth+1,bound|mask,tuple(k if mask&(1<<j) else indices[j] for j in range(2))))
            else:
                if len(out)==token_limit:return fail(3)
                if op==1:
                    if not bound&(1<<target) and len(args[target])!=1:return fail(2)
                    index=indices[target] if bound&(1<<target) else 0
                    if index>=len(args[target]):return fail(2)
                    out.append(args[target][index])
                elif op==2:out.append((2,symbol,current))
                elif op==5:
                    if not allow:return fail(5)
                    out.append((0,symbol,parent))
                else:out.append((0,symbol,context))
    return f'0 {next_context} {len(out)} {calls} {steps} {peak}'+''.join(f' {k} {s} {c}' for k,s,c in out)
def cases():
    rows=[];expected=[]
    def add(p,args,caller,budget,start,entry=0):
        line=[len(p),entry,start,caller,*budget]
        for arg in args:line.extend([len(arg),*(v for token in arg for v in token)])
        for definition in p:line.extend([len(definition),*(v for ins in definition for v in ins)])
        rows.append(' '.join(map(str,line)));expected.append(model(p,entry,args,caller,budget,start))
    programs=[[],[[]],[[instruction(2),instruction(1)]],[[instruction(3,1)],[]],[[instruction(4,1,3)],[instruction(2),instruction(1),instruction(1,1)]],[[instruction(4,1,1)],[instruction(4,2,2)],[instruction(1),instruction(1,1)]],[[instruction(3,0)]],[[instruction(5)]],[[instruction(4,1,1)],[instruction(4,1,1)]],[[instruction(0,context=12)]]]
    for p,la,lb,depth,tokens,steps,start,allow in itertools.product(programs,range(3),range(3),[1,3,8],[0,6,64],[1,10,256],[12,MAX-1],[0,1]):
        args=[[(0,7+j,5+i) for j in range(n)] for i,n in enumerate([la,lb])];add(p,args,5,(depth,tokens,steps,allow),start)
    rng=random.Random(68009)
    for _ in range(4000):
        n=rng.randrange(1,5);p=[]
        for i in range(n):
            definition=[]
            for j in range(rng.randrange(5)):
                op=rng.randrange(7);target=rng.randrange(3) if op==1 else rng.randrange(n+1);definition.append(instruction(op,target,rng.randrange(5),rng.randrange(10),rng.randrange(14)))
            p.append(definition)
        args=[[(rng.randrange(4),rng.randrange(10),rng.randrange(14)) for j in range(rng.randrange(5))] for i in range(2)];add(p,args,rng.randrange(14),(rng.randrange(10),rng.randrange(66),rng.randrange(258),rng.randrange(3)),rng.choice([0,1,12,MAX-1,MAX]),rng.randrange(n+1))
    return rows,expected
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();rows,expected=cases();r=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True);assert r.returncode==0,r.stderr;actual=r.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(i,rows[i],x,y)
    counts=dict(sorted(collections.Counter(int(x.split()[0]) for x in expected).items()));trace=hashlib.sha256(('\n'.join(rows)+'\n'+'\n'.join(expected)+'\n').encode()).hexdigest();print(json.dumps(dict(passed=True,cases=len(rows),outcomes=counts,trace_sha256=trace)))
if __name__=='__main__':main()
