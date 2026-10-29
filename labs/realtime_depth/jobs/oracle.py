"""Independent finite event ledger: Python integers, ready-list sorting, explicit timeline.

No C implementation or output supplies expected schedules. The random seed and
the exact finite domain are part of the experiment, not a schedulability proof.
"""
import itertools
import random
NONE=2**32-1
def expected(tasks,jobs,policy,overload,capacity,horizon,cost):
    # records: state,remaining,executed,start,finish,missed
    records=[[0,j[3],0,NONE,NONE,0] for j in jobs]
    ready=[];running=None;busy_until=0;ticks=[]
    metrics=dict(work=0,overhead=0,idle=0,preemptions=0,misses=0,rejected=0,dropped=0,completed=0,peak=0)
    ranks={task:rank for rank,task in enumerate(sorted(range(len(tasks)),key=lambda i:(tasks[i][2 if policy==1 else 1],i)))}
    deadlines=[j[2]+tasks[j[0]][2] for j in jobs]
    def protected(i):
        return i is not None and i in ready and jobs[i][4]<=records[i][2]<jobs[i][4]+jobs[i][5]
    def key(i):return ((deadlines[i] if policy==2 else ranks[jobs[i][0]]),jobs[i][2],i)
    for time in range(horizon+1):
        for i in ready:
            if deadlines[i]==time and not records[i][5]:records[i][5]=1;metrics['misses']+=1
        if time==horizon:break
        for i in [q for q,j in enumerate(jobs) if j[2]==time]:
            if jobs[i][3]==0:
                records[i][0]=2;records[i][3]=records[i][4]=time;metrics['completed']+=1;continue
            if len(ready)==capacity:
                victims=sorted((q for q in ready if not(q==running and protected(q))),key=lambda q:(jobs[q][2],q))
                if not overload or not victims:records[i][0]=3;metrics['rejected']+=1;continue
                q=victims[0];ready.remove(q);records[q][0]=4;metrics['dropped']+=1
                if running==q:running=None
            ready.append(i);records[i][0]=1;metrics['peak']=max(metrics['peak'],len(ready))
        if time<busy_until:
            metrics['overhead']+=1;ticks.append((NONE,2));continue
        selected=running if protected(running) else min(ready,key=key,default=None)
        if running is not None and running!=selected:
            metrics['preemptions']+=1;running=None
            if cost:
                busy_until=time+cost;metrics['overhead']+=1;ticks.append((NONE,2));continue
        if selected is None:metrics['idle']+=1;ticks.append((NONE,0));continue
        running=selected;row=records[selected];ticks.append((selected,1));metrics['work']+=1
        if row[3]==NONE:row[3]=time
        row[1]-=1;row[2]+=1
        if row[1]==0:
            row[0]=2;row[4]=time+1;ready.remove(selected);metrics['completed']+=1;running=None
    aggregate=[horizon]+[metrics[x] for x in ('work','overhead','idle','preemptions','misses','rejected','dropped','completed','peak')]
    assert metrics['work']+metrics['overhead']+metrics['idle']==horizon
    assert sum(r[2] for r in records)==metrics['work']
    return [0,*aggregate,*itertools.chain.from_iterable(records),*itertools.chain.from_iterable(ticks)]
def cases():
    randomizer=random.Random(520102)
    for _ in range(3000):
        n=randomizer.randrange(1,5);count=randomizer.randrange(0,13)
        tasks=[(randomizer.randrange(0,7),randomizer.randrange(1,9),randomizer.randrange(1,13),0) for _ in range(n)]
        jobs=[]
        for i in range(count):
            work=randomizer.randrange(0,9);start=randomizer.randrange(work+1);length=randomizer.randrange(work-start+1)
            jobs.append((randomizer.randrange(n),i,randomizer.randrange(0,21),work,start,length))
        options=(randomizer.randrange(3),randomizer.randrange(2),randomizer.randrange(1,8),randomizer.randrange(1,25),randomizer.randrange(4))
        values=[n,count,*options,*itertools.chain.from_iterable(tasks),*itertools.chain.from_iterable(jobs)]
        yield ' '.join(map(str,values)),expected(tasks,jobs,*options)
    # Exhaust all two-job unit-cost release/deadline/policy combinations here.
    for r,d1,d2,p,x in itertools.product(range(3),range(1,5),range(1,5),range(3),range(3)):
        tasks=[(2,3,d1,0),(1,2,d2,0)];jobs=[(0,0,0,2,0,0),(1,0,r,1,0,0)];options=(p,0,2,8,x)
        values=[2,2,*options,*itertools.chain.from_iterable(tasks),*itertools.chain.from_iterable(jobs)]
        yield ' '.join(map(str,values)),expected(tasks,jobs,*options)
def check(text):
    rows=text.splitlines();wanted=list(cases())
    if len(rows)!=len(wanted):raise ValueError(f'Oracle row count {len(rows)} != {len(wanted)}')
    for i,(line,(_,expected_row)) in enumerate(zip(rows,wanted)):
        actual=list(map(int,line.split()))
        if actual!=expected_row:raise ValueError(f'Oracle mismatch at {i}: actual={actual}, expected={expected_row}, input={wanted[i][0]}')
    return {'passed':True,'cases':len(rows),'seed':520102,'scope':'All aggregate counters, per-job retained states/results and every tick; finite one-struct lr_cpu model only.'}
def demand_checks():
    # Enumerate synchronous releases independently of the floor expression.
    checks=0
    for c,p,d,h in itertools.product(range(4),range(1,8),range(1,8),range(1,20)):
        explicit=sum(c for release in range(0,h+1,p) if release+d<=h)
        formula=max(0,(h+p-d)//p)*c
        if explicit!=formula:raise ValueError((c,p,d,h))
        checks+=1
    return {'passed':True,'cases':checks,'scope':'Finite synchronous demand counting; not all arrival intervals or a global feasibility claim.'}
if __name__=='__main__':
    import sys
    if len(sys.argv)>1 and sys.argv[1]=='input':print('\n'.join(x for x,_ in cases()))
    else:print(check(sys.stdin.read()))
